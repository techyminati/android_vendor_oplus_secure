#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "plat_file.h"
#include "plat_mem.h"
#include "plat_log.h"
#include "egis_save_to_bmp.h"

#define _Max(a, b) ((b) > (a) ? (b) : (a))
#define _Min(a, b) ((b) < (a) ? (b) : (a))

static void change_endian(uint16_t *in_data, int nSize)
{
	if (in_data == NULL)
		return;

	for (int i = 0; i < nSize; ++i)
		in_data[i] = (((in_data[i] & 0xFF) << 8) | ((in_data[i] & 0xFF00) >> 8));
}

static void image_16bitsTo8bits(unsigned short *raw_16bits, unsigned char *raw_8bits, int width, int height)
{
	int min = 256 * 256;
	int max = 0;
	int i, ratio;
	unsigned short value;

	const int number_ignore_line = 3;
	for (int i = number_ignore_line; i < height - number_ignore_line; i++) {
		for (int j = number_ignore_line; j < width - number_ignore_line; j++) {
			value = raw_16bits[i * width + j];
			if (min > value)
				min = value;
			if (max < value)
				max = value;
		}
	}
	if (max == min) {
		ratio = 1;
	} else {
		ratio = max - min;
	}
	for (i = 0; i < width * height; i++) {
		raw_8bits[i] = _Min(255, _Max(0, 256 * (raw_16bits[i] - min) / ratio));
	}
}

int save_16bit_to_bitmap(int sensor_width, int sensor_height, unsigned short *in_data, char *filename)
{
	if (filename == NULL || NULL == in_data) {
		ex_log(LOG_ERROR, "%s file name can't be empty!", __func__);
		return -1;
	}

	uint8_t *img_tmp = malloc(sensor_height * sensor_width * sizeof(uint8_t));
	if (NULL == img_tmp) {
		ex_log(LOG_ERROR, "inData");
		return -1;
	}

	change_endian(in_data, sensor_height * sensor_width);
	image_16bitsTo8bits(in_data, img_tmp, sensor_width, sensor_height);
	save_8bit_to_bitmap(sensor_width, sensor_height, img_tmp, filename);

	if (NULL != img_tmp) {
		free(img_tmp);
		img_tmp = NULL;
	}

	return 0;
}

int save_8bit_to_bitmap(int sensor_width, int sensor_height, unsigned char *in_data, char *filename)
{
	if (filename == NULL || NULL == in_data) {
		ex_log(LOG_ERROR, "%s file name can't be empty!", __func__);
		return -1;
	}
	const int height = sensor_height;
	const int width = sensor_width - (sensor_width % 4);
	uint32_t size = height * width * 3;

	ex_log(LOG_DEBUG, "%s height = %d, width = %d", __func__, height, width);
	char *pPath = strstr(filename, ".bin");
	if(pPath == NULL) {
		ex_log(LOG_DEBUG, "path is error.");
		return -1;
	}
	mem_move(pPath, ".bmp", 4);
	rgb_t pRGB[height][width];

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			uint8_t a = in_data[i * BANDS * height + j];
			pRGB[i][j].R = pRGB[i][j].G = pRGB[i][j].B = a;
		}
	}

	bitmap_file_header_t fileHeader;
	fileHeader.bfType = 0X4d42;  //'MB' magic number
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfSize = sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t) + size;
	fileHeader.bfOffBits = sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t);

	bitmap_info_header_t bitmapHeader = {0};
	bitmapHeader.biSize = sizeof(bitmap_info_header_t);
	bitmapHeader.biHeight = -height;
	bitmapHeader.biWidth = width;
	bitmapHeader.biPlanes = 1;
	bitmapHeader.biBitCount = 24;
	bitmapHeader.biSizeImage = size;
	bitmapHeader.biCompression = 0;

	char *bmp_data = malloc(fileHeader.bfSize * sizeof(uint8_t));

	if (NULL == bmp_data) {
		ex_log(LOG_DEBUG, "%s bmp_data malloc fail!", __func__);
		return -1;
	}

	char *p_offset = bmp_data;
	mem_move(p_offset, &fileHeader, sizeof(bitmap_file_header_t));
	p_offset += sizeof(bitmap_file_header_t);
	mem_move(p_offset, &bitmapHeader, sizeof(bitmap_info_header_t));
	p_offset += sizeof(bitmap_info_header_t);
	mem_move(p_offset, pRGB, size);

	ex_log(LOG_DEBUG, "bmp path = %s", filename);
	plat_save_file(filename, bmp_data, fileHeader.bfSize);

	if (NULL != bmp_data) {
		free(bmp_data);
		bmp_data = NULL;
	}

	return 0;
}