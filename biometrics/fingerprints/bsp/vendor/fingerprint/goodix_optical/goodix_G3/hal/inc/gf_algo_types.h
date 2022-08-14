/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_ALGO_TYPES_H_
#define _GF_ALGO_TYPES_H_

#include "gf_base_types.h"
typedef enum
{
    GF_CMD_ALGO_INIT,
    GF_CMD_ALGO_ENROLL,
    GF_CMD_ALGO_AUTHENTICATE,
    GF_CMD_ALGO_POST_AUTHENTICATE,
    GF_CMD_ALGO_KPI,
    GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_THREE,
    GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_FIVE,
    GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_OLD,
    GF_CMD_ALGO_DSP_MEM_INIT,
    GF_CMD_ALGO_BIG_DATA,
    GF_CMD_ALGO_MAX
} gf_algo_cmd_id_t;

typedef struct
{
    gf_cmd_header_t header;
    uint32_t preprocess_col;
    uint32_t preprocess_row;
    uint8_t reserve[32];
} gf_algo_init_t;

typedef struct
{
    gf_cmd_header_t header;
    uint32_t o_gid;
    uint32_t o_finger_id;
    uint32_t o_duplicate_finger_id;
    uint16_t o_samples_remaining;
    uint8_t o_antipeep_screen_struct_flag;
    uint8_t reserve[32];
} gf_algo_enroll_image_t;

typedef enum
{
    AUTH_TYPE_UNKNOWN,
    AUTH_TYPE_LOCK,
    AUTH_TYPE_SETTING,
    AUTH_TYPE_PAYMENT,
    AUTH_TYPE_OTHER_UNKOWN
} gf_algo_auth_type_t;

typedef struct
{
    gf_cmd_header_t header;
    uint32_t i_retry_count;
    uint32_t i_finger_up;  // 1:finger up; 0:finger down
    uint32_t i_recog_round;
    uint32_t i_dsp_getfeature_step;
    gf_algo_auth_type_t i_auth_type;
    uint32_t o_study_flag;
    uint32_t o_gid;
    uint32_t o_finger_id;
    uint8_t o_save_flag;
    uint32_t o_dismatch_reason;
    gf_hw_auth_token_t io_auth_token;
    uint8_t o_antipeep_screen_struct_flag;
    int32_t o_image_quality;
    int32_t o_valid_area;
    int32_t o_match_score;
    int32_t o_touch_diff;
    int32_t o_sig_val;
    int32_t o_p_touch;
    int32_t o_slight_index;
    int32_t o_fake_cover;
    int32_t o_bright_flag;
    uint32_t o_is_fake_finger;
    uint32_t o_high_light;
    uint32_t o_rawdata_mean;  // use 4 bytes
    uint8_t o_retry_flag;
    uint8_t o_round_two;
    uint8_t i_residual_flag;
    uint8_t dummy1;  // use 4 bytes
    int32_t o_fake_para_info[5];
    uint8_t reserve[4];  // total 32 bytes
} gf_algo_auth_image_t;

typedef struct
{
    gf_cmd_header_t header;
    uint32_t i_retry_count;
    uint32_t i_study_flag;
    uint32_t i_gid;
    uint32_t i_finger_id;
    uint8_t o_save_flag;
    uint8_t reserve[32];
} gf_algo_post_auth_t;

typedef struct
{
    int32_t image_quality;
    int32_t valid_area;
    int32_t match_score;
    uint32_t authenticate_study_flag;
    int32_t key_point_num;
    uint32_t increase_rate;
    uint32_t overlay;
    uint32_t get_raw_data_time;
    uint32_t broken_check_time;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t enroll_time;
    uint32_t authenticate_time;
    uint32_t total_time;
    uint32_t study_flag;
    int32_t update;
    uint32_t weak_update;
    int32_t fake_result;
    uint32_t fake_detect_time;  //use 4 bytes reserve
    uint8_t reserve[28];  //total 32 bytes
} gf_algo_performance_t;


typedef struct
{
    gf_cmd_header_t header;
    gf_algo_performance_t o_performance;
} gf_algo_kpi_t;

typedef struct
{
    gf_cmd_header_t header;
} gf_algo_dsp_mem_init_t;

#define STRING_LENGTH 128
#define DATA_INT_COUNT 32
#define DATA_STRING_COUNT 32
typedef struct
{
    gf_cmd_header_t header;
    int is_ta_valid;
    char algo_version[STRING_LENGTH];
    char data_string[DATA_STRING_COUNT][STRING_LENGTH];
    int data_int[DATA_INT_COUNT];
} oplus_report_data_t;

#endif  /* _GF_ALGO_TYPES_H_ */
