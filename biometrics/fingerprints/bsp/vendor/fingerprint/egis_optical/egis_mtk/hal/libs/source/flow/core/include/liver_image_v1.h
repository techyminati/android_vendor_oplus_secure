#ifndef __LIVER_IMAGE_T_V1__
#define __LIVER_IMAGE_T_V1__

#include "common_definition.h"
#define LIVE_IMG_VER (0xA1)
typedef struct {
    uint32_t live_image_header_ver;
	int framecount;
	int	img_width;
	int img_height;
	int raw_width;
	int raw_height;
	int raw_bpp;
	int live_image_type;
	union image_par{
		param_imageqmlib_t qm_parameter;
		param_imageprocessing_t process_parameter;
		param_image_statistics_data_t image_statistics_data;
		param_cali_image_data_t cali_image_data;
	}image_par_t;
} liver_image_out_header_t;

#define get_image_bpp(header) (8)
#endif
