#ifndef __EGIS_SAVE_TO_BMP_H__
#define __EGIS_SAVE_TO_BMP_H__

#include <stdint.h>

#define BANDS 1
typedef struct
{
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} __attribute__((packed)) bitmap_info_header_t;

typedef struct
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} __attribute__((packed)) bitmap_file_header_t;
typedef struct
{
	uint8_t B;
	uint8_t G;
	uint8_t R;
} __attribute__((packed)) rgb_t;

int save_16bit_to_bitmap(int sensor_width, int sensor_height, unsigned short *in_data, char *filename);
int save_8bit_to_bitmap(int sensor_width, int sensor_height, unsigned char *in_data, char *filename);

#endif
