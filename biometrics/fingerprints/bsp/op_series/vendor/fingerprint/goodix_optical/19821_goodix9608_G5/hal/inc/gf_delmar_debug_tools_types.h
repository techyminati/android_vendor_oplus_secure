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

enum
{
    GF_DEBUG_TOOLS_CMD_RETRIEVE_BMP = GF_DEBUG_TOOLS_CMD_MAX + 1,
    GF_DEBUG_TOOLS_CMD_FINDSENSOR,
    GF_DEBUG_TOOLS_CMD_GET_VERSION,
    GF_DEBUG_TOOLS_CMD_UNTRUST_ENROLL_ENABLE,
    GF_DEBUG_TOOLS_CMD_DATA_COLLECT,
    GF_DEBUG_TOOLS_CMD_DATA_SAMPLING,
    GF_DEBUG_TOOLS_CMD_SET_SAMPLING_CFG,
    GF_DEBUG_TOOLS_CMD_GET_SAMPLING_CFG,
    GF_DELMAR_DEBUG_TOOLS_CMD_MAX,
};

typedef enum
{
    OPERATION_STEP_COLLECT_RGB_NONE = 0,
    OPERATION_STEP_RED_DATA_COLLECT,
    OPERATION_STEP_BLUE_DATA_COLLECT,
    OPERATION_STEP_GREEN_DATA_COLLECT,
    OPERATION_STEP_WHITE_DATA_COLLECT,
    OPERATION_STEP_COLLECT_RGB_MAX = 99
} gf_test_collect_color_data_operation_step_t;

enum
{
    BRIGHTNESS_LEVEL_H = 0,
    BRIGHTNESS_LEVEL_M = 1,
    BRIGHTNESS_LEVEL_L = 2,
};

enum
{
    SAMPLING_OBJECT_FINGER = 0,
    SAMPLING_OBJECT_FLESH_RUBBER = 1,
    SAMPLING_OBJECT_DARK_RUBBER = 2,
};

typedef struct
{
    gf_cmd_header_t cmd_header;
    int8_t algo_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t preprocess_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t production_algo_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t fake_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t fw_version[FW_VERSION_INFO_LEN];
    int8_t tee_version[TEE_VERSION_INFO_LEN];
    int8_t ta_version[TA_VERSION_INFO_LEN];
    uint8_t chip_id[DELMAR_CHIP_ID_BUFFER_LEN];
    uint8_t vendor_id[DELMAR_VENDOR_ID_BUFFER_LEN];
    uint8_t sensor_id[DELMAR_SENSOR_ID_BUFFER_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    uint16_t pga_gain[MAX_SENSOR_NUM];
    uint16_t exposure_time[MAX_SENSOR_NUM];
} gf_test_get_version_t;

typedef struct
{
    uint8_t bmp_data[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint8_t bmp_decrypt_data[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint8_t bmp_data_cali[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint32_t single_bmp_size;
    uint32_t width;
    uint32_t height;
    int32_t rm[MAX_SENSOR_NUM];
    int32_t sg[MAX_SENSOR_NUM];
} gf_delmar_bmp_data_t;

typedef struct
{
    gf_cmd_header_t header;
    gf_delmar_bmp_data_t o_bmp_data;
} gf_delmar_retrieve_bmp_cmd_t;

typedef struct
{
    gf_cmd_header_t header;
    uint8_t sensor_index;
    gf_delmar_bmp_data_t o_bmp_data;
} gf_delmar_find_sensor_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint16_t raw_data[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
#ifdef SUPPORT_DUMP_ORIGIN_DATA
    uint8_t origin_data[MAX_SENSOR_NUM][DELMAR_ORIGIN_BUFFER_LEN];
    uint32_t origin_col;
    uint32_t origin_row;
#endif  //  SUPPORT_DUMP_ORIGIN_DATA
    uint8_t black_set[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint32_t black_set_len[MAX_SENSOR_NUM];
    uint64_t valid_sensor_ids;
    gf_delmar_bmp_data_t test_bmp_data;
    uint8_t raw_feature_old_arr[MAX_SENSOR_NUM][DELMAR_RAW_FEATURE_OLD_LEN];
    uint32_t raw_feature_old_arr_len[MAX_SENSOR_NUM];
    int16_t diff_image[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    int32_t dark_preprocess[MAX_SENSOR_NUM];
    uint32_t bad_rawdata_flag[MAX_SENSOR_NUM];
    uint32_t n_add_one_frame_flag;
    int32_t cali_state[MAX_SENSOR_NUM];
    int32_t raw_data_mean[MAX_SENSOR_NUM];
    int32_t sig_rate[MAX_SENSOR_NUM];
    int32_t broken_level[MAX_SENSOR_NUM];
    int32_t spoof_level[MAX_SENSOR_NUM];
    int32_t bio_assay_ret;
    int32_t moire_detect[MAX_SENSOR_NUM];
    int32_t partial_touch[MAX_SENSOR_NUM];
    int32_t compensation_mode[MAX_SENSOR_NUM];
    int32_t reverse_recog[MAX_SENSOR_NUM];
    int32_t safe_level[MAX_SENSOR_NUM];
    int32_t weak_level[MAX_SENSOR_NUM];
    int32_t motion_x[MAX_SENSOR_NUM];
    int32_t motion_y[MAX_SENSOR_NUM];
    int32_t motion_cost[MAX_SENSOR_NUM];
    uint8_t mask_data[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint32_t mask_data_len[MAX_SENSOR_NUM];
    uint32_t i_retry_count;
    gf_delmar_coordinate_info_t cd_info;
} gf_test_collect_rawdata_t;

#define TEST_MAX_SAMPLING_FRAME_NUM (3)
#define IF_FREE(X) {if ((X) != NULL) { CPL_MEM_FREE(X); (X) = NULL;}}

typedef struct
{
    gf_cmd_header_t header;
    int32_t i_collect_phase;
    uint8_t i_sensor_index;
    int32_t i_frame_num;
} gf_delmar_test_cmd_base_t;

typedef struct
{
    gf_delmar_test_cmd_base_t cmd_base;
    uint8_t o_sensor_id[DELMAR_SENSOR_ID_BUFFER_LEN];
    uint16_t o_raw_data[DELMAR_RAW_BUFFER_LEN * TEST_MAX_SAMPLING_FRAME_NUM * MAX_SENSOR_NUM];
    uint16_t o_raw_data_cf[DELMAR_RAW_BUFFER_LEN * TEST_MAX_SAMPLING_FRAME_NUM * MAX_SENSOR_NUM];
    uint8_t o_origin_data[DELMAR_ORIGIN_BUFFER_LEN * TEST_MAX_SAMPLING_FRAME_NUM * MAX_SENSOR_NUM];
    gf_delmar_bmp_data_t o_bmp_data[TEST_MAX_SAMPLING_FRAME_NUM];
    uint32_t o_origin_col;
    uint32_t o_origin_row;
} gf_delmar_data_sampling_cmd_t;

typedef struct
{
    gf_cmd_header_t header;
    gf_delmar_sampling_cfg_t io_cfg;
} gf_delmar_sampling_cfg_cmd_t;

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
