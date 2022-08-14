#ifndef _GF_CUSTOMIZED_ALGO_TYPES_H_
#define _GF_CUSTOMIZED_ALGO_TYPES_H_

#include <stdint.h>
#include "gf_algo_types.h"

#define ENROLL_TIME_FLAG_FILE_NAME "gf_algo_init.so"
#define ENROLL_TIME_FLAG_BAK_FILE_NAME "gf_algo_init_bak.so"
#define ENROLL_TIME_FLAG_UNLOCK_FILE_NAME "gf_algo_init_unlock.so"

typedef enum CUSTOMIZED_ALGOCMD_ID {
    GF_CMD_ALGO_INIT_PARAMS = GF_CMD_ALGO_MAX+ 6,
    GF_CMD_ALGO_SAVE_PARAMS = GF_CMD_ALGO_MAX+ 7,
    GF_CUSTOMIZED_CMD_GET_AUTH_DETAIL = GF_CMD_ALGO_MAX + 8,
    GF_CUSTOMIZED_CMD_GET_ALGOVERSION = GF_CMD_ALGO_MAX + 9,
    GF_CUSTOMIZED_CMD_ALGO_MAX
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