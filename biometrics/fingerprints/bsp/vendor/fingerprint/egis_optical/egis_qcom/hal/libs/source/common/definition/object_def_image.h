#ifndef __OBJECT_DEFINITION_IMAGE_H__
#define __OBJECT_DEFINITION_IMAGE_H__

#include "object_def.h"

// #define ONLY_REQUEST_IMGTYPE_BKG  // temporary use define to switch
#define IMGTYPE_8BIT 0
#define IMGTYPE_RAW 1
#define IMGTYPE_BKG 2

#ifdef _WINDOWS
#define PACKED_STRUCT(__Declaration__) __pragma(pack(push, 1)) typedef struct __Declaration__ __pragma(pack(pop))
#else
#define PACKED_STRUCT(__Declaration__) typedef struct __Declaration__ __attribute__((packed)) 
#endif

#define VERIFY_TRY_MATCH_RESULT_LAST_NOT_MATCH -1
#define VERIFY_TRY_MATCH_RESULT_NOT_MATCH 0
#define VERIFY_TRY_MATCH_RESULT_MATCH 1

PACKED_STRUCT( {
	rbs_obj_desc_t obj_desc;
	uint16_t imgtype;
	uint16_t width;
	uint16_t height;
	uint16_t bpp;
	uint16_t param_type;  // the name should not be changed for param offset
	uint16_t index_fingeron;
	uint16_t index_try_match;
	uint16_t index_series;
	uint16_t is_bad;
	int16_t algo_flag;
	uint16_t reject_reason;
	uint16_t match_score;
	int16_t try_match_result;
	int16_t qty;      // g2_qty for G2
	uint16_t partial;  // ML Partial for optical projects
	int16_t fake_score;
	uint16_t is_light;
	uint16_t stat_min;
	uint16_t stat_max;
	uint16_t stat_avg;
	int16_t temperature;
	uint16_t exposure_x10;
	uint16_t hw_integrate_count;
	uint16_t sw_integrate_count;
	uint16_t extract_qty;
	uint16_t bds_debug_path;
	uint16_t bds_pool_add;
	uint16_t is_learning;
	uint16_t match_type; // VERIFY_TYPE_NORMAL_MATCH, VERIFY_TYPE_QUICK_MATCH, ..
	int16_t g2_partial;
	uint16_t is_last_image;
	int16_t black_edge;
}) rbs_obj_image_v1_0_t;

#define RBSOBJ_set_IMAGE_v1_0(img_obj, the_imgtype, the_width, the_height, the_bpp)   \
	OBJ_DESC_init((&(img_obj->obj_desc)), RBSOBJ_ID_IMAGE, "IMAGE", 1, 0, 0);     \
	img_obj->obj_desc.header_size = sizeof(*img_obj) - sizeof(img_obj->obj_desc); \
	img_obj->obj_desc.payload_size = the_width * the_height * the_bpp / 8;        \
	img_obj->imgtype = the_imgtype;                                               \
	img_obj->width = the_width;                                                   \
	img_obj->height = the_height;                                                 \
	img_obj->bpp = the_bpp;

#define RBSOBJ_set_IMAGE_param(img_obj, param, value) \
	if (img_obj != NULL) img_obj->param = value;

#define RBSOBJ_get_IMAGE_param(img_obj, param) \
	img_obj->param

#define RBSOBJ_copy_IMAGE_params(src_img_obj, dest_img_obj)                                     \
	{ int offset = (uint8_t*)(&src_img_obj->param_type) - (uint8_t*)(&src_img_obj->obj_desc); \
	if (dest_img_obj == NULL) memset(((uint8_t*)(src_img_obj) + offset), 0, sizeof(*src_img_obj) - offset); \
	else memcpy(((uint8_t*)(dest_img_obj) + offset), ((uint8_t*)(src_img_obj) + offset), sizeof(*src_img_obj) - offset); }

#endif