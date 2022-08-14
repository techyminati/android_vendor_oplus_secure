#include <stdio.h>
#include <sys/stat.h>
#include "plat_file.h"
#include "plat_mem.h"

void save_img16_BigEndian(char* path, uint16_t* raw_img16, int width, int height) {
    int i;
    uint8_t* output_img = NULL;

    output_img = (uint8_t*)plat_alloc(width * height * sizeof(uint16_t));

    for (i = 0; i < (width * height); i++) {
        output_img[2 * i] = (raw_img16[i] >> 8) & 0xFF;
        output_img[2 * i + 1] = (raw_img16[i]) & 0xFF;
    }
    plat_save_file(path, output_img, width * height * sizeof(uint16_t));
    plat_free(output_img);
}

int plat_save_raw_image(char* path, unsigned char* pImage, unsigned int width,
			unsigned int height)
{
	if (path == NULL) return -1;
	if (pImage == NULL) return -2;

	FILE* fn = fopen(path, "wb");
	if (fn != NULL) {
		unsigned int size = fwrite(pImage, 1, width * height, fn);
		fclose(fn);
		return size;
	}
	return -3;
}

int plat_load_raw_image(char* path, unsigned char* pImage, unsigned int width,
			unsigned int height)
{
	if (pImage == NULL) {
		return -1;
	}
	int ret;
	long image_size = width * height;
	FILE* fn = fopen(path, "r");
	ret = -2;
	if (fn != NULL) {
		ret = fread(pImage, 1, image_size, fn);
		// egislog_d("load size = %d", ret);
		fclose(fn);
		if (ret < image_size) ret = -3;
	}
	return ret;
}

static long get_file_size(char* file)
{
	struct stat filestat;
	if (stat(file, &filestat) == 0) {
		return filestat.st_size;
	}
	return 0;
}

int plat_save_file(char* path, unsigned char* buf, unsigned int len)
{
	return plat_save_raw_image(path, buf, len, 1);
}

int plat_load_file(char* path, unsigned char* buf, unsigned int len,
		   unsigned int* real_size)
{
	if (!buf || len == 0 || !real_size) {
		return -1;
	}
	unsigned int file_size = get_file_size(path);
	*real_size = 0;
	if (file_size == 0) {
		return PLAT_FILE_NOT_EXIST;
	}
	int read_size = len > file_size ? file_size : len;
	int ret;
	FILE* fn = fopen(path, "r");
	ret = -2;
	if (fn != NULL) {
		ret = fread(buf, 1, read_size, fn);
		*real_size = read_size;
		// egislog_d("load size = %d", ret);
		fclose(fn);
		if (ret < read_size) ret = -3;
	}
	return ret;
}

int plat_remove_file(char* path)
{
	if (path) {
		return remove(path);
	}
	return -1;
}
