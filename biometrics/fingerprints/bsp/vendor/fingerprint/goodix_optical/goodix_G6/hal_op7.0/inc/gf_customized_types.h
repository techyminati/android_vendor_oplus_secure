/************************************************************************************
 ** File: - gf_customized_types.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      file for customized feature, add OPLUS feature cmd id to this file!
 **
 ** Version: 1.0
 ** Date created: 15:09:11,15/08/2019
 ** Author: Bangxiong.Wu@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Bangxiong.Wu       2019/10/12       create file for adding OPLUS cmd feature
 **  Bangxiong.Wu       2019/10/12       move ALGO_GET_ALGOVERSION to this file
 ************************************************************************************/

#ifndef _GF_CUSTOMIZED_TYPES_H_
#define _GF_CUSTOMIZED_TYPES_H_

#include "gf_delmar_types.h"

typedef struct gf_customized_config {
    uint8_t customized_optical_type;
} gf_customized_config_t;

typedef struct {
    gf_delmar_sensor_init_t parent;
    int32_t i_product_screen_id;
    gf_customized_config_t o_customized_config;
} gf_customized_sensor_init_t;

typedef enum {
    SCREEN_TYPE_AD097_SAMSUNG_ID = 0,
    SCREEN_TYPE_AD097_BOE_ID,
    SCREEN_TYPE_BD187_SAMSUNG_ID,
    SCREEN_TYPE_CC151_SAMSUNG_ID,
    SCREEN_TYPE_CC161_SAMSUNG_ID,
    SCREEN_TYPE_DD306_SAMSUNG_ID,
    SCREEN_TYPE_AE009_SAMSUNG_ID,
    SCREEN_TYPE_AE009_BOE_ID,
    SCREEN_TYPE_AD119_SAMSUNG_ID,
    SCREEN_TYPE_RA352_SAMSUNG_ID
} gf_customized_screen_id_t;

typedef enum {
    GF_CUSTOMIZED_CMD_GET_AUTH_DETAIL = GF_DELMAR_CMD_ALGO_MAX + 1,
    GF_CUSTOMIZED_CMD_GET_ALGOVERSION,
    GF_CUSTOMIZED_CMD_ALGO_MAX
} gf_customized_algo_cmd_id_t;

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

#endif  // #endif
