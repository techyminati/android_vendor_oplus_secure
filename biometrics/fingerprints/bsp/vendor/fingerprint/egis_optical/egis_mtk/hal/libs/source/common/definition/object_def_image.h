#ifndef __OBJECT_DEFINITION_IMAGE_H__
#define __OBJECT_DEFINITION_IMAGE_H__

#include "object_def.h"
#include "plat_mem.h"

// #define ONLY_REQUEST_IMGTYPE_BKG  // temporary use define to switch
#define IMGTYPE_BIN 0
#define IMGTYPE_RAW 1
#define IMGTYPE_BKG 2
#define IMGTYPE_DEBUG 3

#define G4_BPP_BKG 160
#define G4_BKG_SIZE 800040

#ifdef _WINDOWS
#define PACKED_STRUCT(__Declaration__) \
    __pragma(pack(push, 1)) typedef struct __Declaration__ __pragma(pack(pop))
#else
#define PACKED_STRUCT(__Declaration__) typedef struct __Declaration__ __attribute__((packed))
#endif

PACKED_STRUCT({
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
    uint16_t algo_flag;
    uint16_t reject_reason;
    int16_t match_score;
    uint16_t try_match_result;
    uint16_t qty;
    uint16_t partial;
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
    uint16_t match_type;  // VERIFY_TYPE_NORMAL_MATCH, VERIFY_TYPE_QUICK_MATCH, ..
    int16_t g2_partial;
    int16_t black_edge;
    uint16_t is_in_algo;
    uint16_t is_fake;
    uint16_t is_residual;
    uint64_t ta_time_ms;
    uint16_t is_reject_retry;
    uint16_t is_duplicate_finger;
    uint16_t is_dqe_enroll;
    uint16_t is_get_image_vibration;
    uint32_t matched_id;
    uint16_t is_scratch;
    uint16_t is_last_match_failed;
    uint16_t is_last_try;
    uint16_t min_moire_index_cnt;
    uint16_t is_update_debase;
    uint16_t is_finger_off;
    uint16_t inline_qty;
    uint16_t seg;
    uint32_t cls_score;
    uint32_t status_score;
    uint32_t strange_score;
})
rbs_obj_image_v1_0_t;

#define RBSOBJ_set_IMAGE_v1_0(img_obj, the_imgtype, the_width, the_height, the_bpp) \
    OBJ_DESC_init((&(img_obj->obj_desc)), RBSOBJ_ID_IMAGE, "IMAGE", 1, 0, 0);       \
    img_obj->obj_desc.header_size = sizeof(*img_obj) - sizeof(img_obj->obj_desc);   \
    img_obj->obj_desc.payload_size = the_width * the_height * the_bpp / 8;          \
    if (the_bpp == G4_BPP_BKG) img_obj->obj_desc.payload_size = G4_BKG_SIZE;        \
    img_obj->imgtype = the_imgtype;                                                 \
    img_obj->width = the_width;                                                     \
    img_obj->height = the_height;                                                   \
    img_obj->bpp = the_bpp;

#define RBSOBJ_set_IMAGE_param(img_obj, param, value) \
    if (img_obj != NULL) img_obj->param = value;

#define RBSOBJ_get_IMAGE_param(img_obj, param) img_obj->param

#define RBSOBJ_copy_IMAGE_params(src_img_obj, dest_img_obj)                                     \
    {                                                                                           \
        int offset = (uint8_t*)(&src_img_obj->param_type) - (uint8_t*)(&src_img_obj->obj_desc); \
        if (dest_img_obj == NULL)                                                               \
            mem_set(((uint8_t*)(src_img_obj) + offset), 0, sizeof(*src_img_obj) - offset);      \
        else                                                                                    \
            mem_move(((uint8_t*)(dest_img_obj) + offset), ((uint8_t*)(src_img_obj) + offset),   \
                     sizeof(*src_img_obj) - offset);                                            \
    }

#endif
