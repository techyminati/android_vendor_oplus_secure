#include <stdlib.h>
#include <string.h>
#include "image_cut.h"
#include "egis_definition.h"
#include "plat_mem.h"

int image_crop(unsigned char* src_img, int src_width, int src_height, int des_width, int des_height)
{
	if (src_img == NULL || des_width > src_width || des_height >src_height) {
		return EGIS_INCORRECT_PARAMETER;
	}

	int i;
	int width_point, height_point;
	width_point = (src_width - des_width) / 2;
	height_point = (src_height - des_height) / 2;
	unsigned char *des_img = (unsigned char *)plat_alloc(des_width * des_height);

	if (des_img == NULL) {
		return EGIS_OUT_OF_MEMORY;
	}

	unsigned char* src = src_img;
	unsigned char* des = des_img;

	for (i = height_point; i < (des_height + height_point); i++){
		memcpy(des, src + i * src_width + width_point, des_width);
		des += des_width;
	}

	memset(src_img, 0x00, src_width * src_height);
	memcpy(src_img, des_img, des_width * des_height);

	plat_free(des_img);

	return EGIS_OK;
}