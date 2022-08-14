#ifndef _GF_CUSTOMIZED_ALGO_TYPES_H_
#define _GF_CUSTOMIZED_ALGO_TYPES_H_

#include <stdint.h>
#include "gf_algo_types.h"
#include "gf_delmar_product_test_types.h"

#define ENROLL_TIME_FLAG_FILE_NAME "gf_algo_init.so"
#define ENROLL_TIME_FLAG_BAK_FILE_NAME "gf_algo_init_bak.so"
#define ENROLL_TIME_FLAG_UNLOCK_FILE_NAME "gf_algo_init_unlock.so"

typedef enum CUSTOMIZED_ALGOCMD_ID {
    GF_CMD_ALGO_INIT_PARAMS = GF_CMD_ALGO_MAX+ 6,
    GF_CMD_ALGO_SAVE_PARAMS = GF_CMD_ALGO_MAX+ 7,
} gf_customized_algo_cmd_id_t;

typedef struct GF_DELMAR_ENROLL_TIME_FLAG {
    gf_cmd_header_t header;
    uint64_t  enroll_time;
} gf_delmar_enroll_time_flag_t;

typedef struct GF_DELMAR_FAKE_INFO {
    uint32_t nfV2ParamCount;
    uint64_t enroll_time_flag;
} gf_delmar_fake_info_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_temperature;
    uint32_t i_sensor_index;
    uint32_t o_image_quality;
    uint32_t o_valid_area;
    uint8_t o_is_fake_finger;
    uint8_t mask_bw[DELMAR_BW_MASK_LEN];
    uint16_t raw_data[DELMAR_RAW_BUFFER_LEN];
    uint32_t raw_data_len;
    uint8_t raw_data_cf[DELMAR_RAW_CF_BUFFER_LEN];
    uint32_t raw_data_cf_len;
    uint8_t data_bmp[DELMAR_BMP_BUFFER_LEN];
    uint8_t data_decrypt_bmp[DELMAR_BMP_BUFFER_LEN];
    int16_t diff_image[DELMAR_BMP_BUFFER_LEN];
    uint8_t color_point_data[DELMAR_CF_POINT_BUFFER_LEN];
    uint32_t color_point_data_len;
    uint32_t preprocess_col;
    uint32_t preprocess_row;
    uint8_t mask_data[DELMAR_BMP_BUFFER_LEN];
    uint32_t mask_data_len;
    uint32_t mask_bw_len;
    int32_t nFakeParamV1Arr[2][20];
    int32_t nFakeParamV2Arr[2][20];
    int32_t nfFakeParamV1Arr[2][20];
    int32_t nfFakeParamV2Arr[2][20];
    uint32_t o_part;
    int32_t fake_result;
    gf_delmar_coordinate_info_t i_coordinate_info;
}gf_customized_image_quality_test_t;

enum {
    GF_CMD_TEST_GET_IMAGE_QUALITY = GF_CMD_TEST_MAX + 1,
}gf_customized_delmar_algo_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint8_t i_dsp_applied;
    int32_t o_image_quality;
    int32_t o_valid_area;
    int32_t o_match_score;
    int32_t o_sig_val;
    int32_t o_fake_result;
    uint16_t o_gain;
    uint32_t o_detect_fake_time;
    uint32_t o_preprocess_time;
    uint32_t o_get_feature_time;
    uint32_t o_authenticate_time;
} gf_customized_auth_detail_get_t;

typedef struct {
    gf_cmd_header_t header;
    char version_info[MAX_ALGO_VERSION_LEN];
} gf_customized_algo_version_info_t;

#endif  // _GF_CUSTOMIZED_ALGO_TYPES_H_