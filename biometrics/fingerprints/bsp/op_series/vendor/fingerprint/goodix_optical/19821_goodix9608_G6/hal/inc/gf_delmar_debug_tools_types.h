/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _GF_DELMAR_DEBUG_TOOLS_TYPES_H_
#define _GF_DELMAR_DEBUG_TOOLS_TYPES_H_

#include <stdint.h>
#include "gf_debug_tools_types.h"
#include "gf_delmar_types.h"

#define FUSION_IMG_SG_INDEX                        (0x10)

enum {
    GF_DEBUG_TOOLS_CMD_RETRIEVE_BMP = GF_DEBUG_TOOLS_CMD_MAX + 1,
    GF_DEBUG_TOOLS_CMD_FINDSENSOR,
    GF_DEBUG_TOOLS_CMD_UNTRUST_ENROLL_ENABLE,
    GF_DEBUG_TOOLS_CMD_DATA_COLLECT,
    GF_DEBUG_TOOLS_CMD_DATA_SAMPLING,
    GF_DEBUG_TOOLS_CMD_READ_FLASH,
    GF_DEBUG_TOOLS_CMD_WRITE_FLASH,
    GF_DELMAR_DEBUG_TOOLS_CMD_MAX,
};

typedef enum {
    OPERATION_STEP_COLLECT_RGB_NONE = 0,
    OPERATION_STEP_RED_DATA_COLLECT,
    OPERATION_STEP_BLUE_DATA_COLLECT,
    OPERATION_STEP_GREEN_DATA_COLLECT,
    OPERATION_STEP_WHITE_DATA_COLLECT,
    OPERATION_STEP_COLLECT_RGB_MAX = 99
} gf_test_collect_color_data_operation_step_t;

enum {
    BRIGHTNESS_LEVEL_H = 0,
    BRIGHTNESS_LEVEL_M = 1,
    BRIGHTNESS_LEVEL_L = 2,
};

enum {
    SAMPLING_OBJECT_FINGER = 0,
    SAMPLING_OBJECT_FLESH_RUBBER = 1,
    SAMPLING_OBJECT_DARK_RUBBER = 2,
};

typedef struct {
    uint8_t o_is_valid;
    uint8_t bmp_data[DELMAR_RAW_BUFFER_LEN];
    uint8_t bmp_decrypt_data[DELMAR_RAW_BUFFER_LEN];
    uint8_t bmp_data_cali[DELMAR_RAW_BUFFER_LEN];
    uint32_t width;
    uint32_t height;
    int32_t rm;
    int32_t sg;
} gf_delmar_bmp_data_t;

typedef struct {
    gf_cmd_header_t header;
    gf_delmar_bmp_data_t o_bmp_data[];
} gf_delmar_retrieve_bmp_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint8_t index;
    gf_delmar_bmp_data_t o_bmp_data[];
} gf_delmar_find_sensor_cmd_t;

typedef struct {
    uint8_t sensor_index;
    uint8_t sg_index;
    uint8_t fusion_best_sg_id;
    uint8_t fusion_second_best_sg_id;

    uint16_t raw_data[DELMAR_RAW_BUFFER_LEN];
    uint8_t raw_data_cf[DELMAR_RAW_CF_BUFFER_LEN];
#ifdef SUPPORT_DUMP_ORIGIN_DATA
    uint8_t origin_data[DELMAR_ORIGIN_BUFFER_LEN];
    uint32_t origin_col;
    uint32_t origin_row;
#endif  //  SUPPORT_DUMP_ORIGIN_DATA
    uint8_t black_set[DELMAR_RAW_BUFFER_LEN];
    uint32_t black_set_len;
    gf_delmar_bmp_data_t test_bmp_data;
    uint8_t raw_feature_old_arr[DELMAR_RAW_FEATURE_OLD_LEN];
    uint32_t raw_feature_old_arr_len;
    int16_t diff_image[DELMAR_BMP_BUFFER_LEN];
    uint8_t raw_image[DELMAR_BMP_BUFFER_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    int32_t dark_preprocess;
    uint32_t bad_rawdata_flag;
    uint32_t n_add_one_frame_flag;
    int32_t raw_data_mean;
    int32_t sig_rate;
    int32_t broken_level;
    int32_t spoof_level;
    int32_t bio_assay_ret;
    int32_t moire_detect;
    int32_t partial_touch;
    int32_t compensation_mode;
    int32_t reverse_recog;
    int32_t safe_level;
    int32_t weak_level;
    int32_t motion_x;
    int32_t motion_y;
    int32_t motion_cost;
    uint8_t mask_data[DELMAR_BMP_BUFFER_LEN];
    uint32_t mask_data_len;
    uint8_t is_highlight;
} gf_delmar_collect_rawdata_piece_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_retry_count;
    uint64_t valid_sensor_ids;
    gf_delmar_coordinate_info_t cd_info;
    gf_delmar_preprocess_moire_info_t moire_info[MAX_SENSOR_NUM];
    gf_delmar_sensor_temperature_t sensor_temp;
    uint8_t sensor_temp_valid;
    uint8_t o_exposure_time;

    uint32_t piece_count;
    // keep it as the last item
    gf_delmar_collect_rawdata_piece_t pieces[];
} gf_delmar_collect_rawdata_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t flash_addr;
    uint32_t data_len;
    uint8_t  data[DELMAR_RW_FLASH_BUFFER_LEN];
} gf_test_rw_flash_t;

#define TEST_MAX_SAMPLING_FRAME_NUM (3)
#define IF_FREE(X) {if ((X) != NULL) { CPL_MEM_FREE(X); (X) = NULL;}}

typedef struct {
    gf_cmd_header_t header;
    int32_t i_collect_phase;
    uint8_t i_sensor_index;
    int32_t i_frame_num;
    uint32_t i_brightness_level;
} gf_delmar_test_cmd_base_t;

typedef struct {
    uint16_t o_raw_data[DELMAR_RAW_BUFFER_LEN];
    uint16_t o_raw_data_cf[DELMAR_RAW_BUFFER_LEN];
#ifdef SUPPORT_MULTI_BRIGHTNESS_LEVEL
    uint16_t o_lb_raw_data_cf[DELMAR_RAW_BUFFER_LEN];
#endif  // SUPPORT_MULTI_BRIGHTNESS_LEVEL
    uint8_t o_origin_data[DELMAR_ORIGIN_BUFFER_LEN];
    uint32_t o_origin_col;
    uint32_t o_origin_row;
    gf_delmar_bmp_data_t o_bmp_data;
} gf_delmar_data_sampling_piece_t;

typedef struct {
    gf_delmar_test_cmd_base_t cmd_base;

    uint32_t piece_count;
    gf_delmar_data_sampling_piece_t pieces[];
} gf_delmar_data_sampling_cmd_t;

#define COLLECT_FILE_PATH_MAX_LEN (256)
typedef struct {
    char dir_content[COLLECT_FILE_PATH_MAX_LEN];
    uint8_t is_sample;  // template or sample
    uint32_t frm_count;
    uint32_t retry_times;
} gf_test_collect_rawdata_pending_info_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t i_enable;
} gf_delmar_untrust_enroll_t;

#endif /* _GF_DELMAR_DEBUG_TOOLS_TYPES_H_ */
