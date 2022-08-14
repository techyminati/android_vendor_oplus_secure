/******************************************************************************
 * @file   silead_cmd_cust.h
 * @brief  Contains fingerprint operate functions header file.
 *
 *
 * Copyright (c) 2018-2019 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>        <date>   <version>     <desc>
 * Sileadinc      2019/1/2    0.1.0      Init version
 * Bangxiong.wu   2019/3/10   1.0.0      Modify for saving calibrate data
 * Bangxiong.Wu   2019/4/10   1.0.1      Compatible for HAL and delete static function declaration
 *****************************************************************************/

#ifndef __SILEAD_CMD_CUST_H__
#define __SILEAD_CMD_CUST_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
#include "silead_ext_cb_cust.h"

#define IDENTIFY_ENGINE_STR_LEN_MAX  16

typedef struct _ft_info {
    int32_t dead_pixels;
    int32_t circle;
    int32_t diameter;
    int32_t mean_w;
    int32_t mean_b;
    int32_t p_percent;
    int32_t p_wb_percent;
    int32_t noise;
    int32_t blot;
    int32_t blot_glass;
    int32_t status;
    int32_t shading;
    int32_t shading_unit;
} ft_info_t;

typedef struct _algorithm_paramater {
    uint32_t ver_identify_engine;
    char     str_identify_engine[IDENTIFY_ENGINE_STR_LEN_MAX];
    uint32_t ver_algorithm;
    uint32_t ver_ta;
    uint32_t postprocess_ctl;
    uint32_t aec;
    uint16_t mean;
    uint16_t time;
    uint16_t range_mean;
    uint16_t range_thr;
    uint16_t range_offset;
    ft_info_t ft;
    // ....
} algotirhm_paramater_t;

typedef enum _fp_cal_test_step {
    FUN_FINGERPRINT_TEST_START = 0,
    FUN_FINGERPRINT_CAL_STEP_1 = 1,
    FUN_FINGERPRINT_CAL_STEP_2,
    FUN_FINGERPRINT_CAL_STEP_3,
    FUN_FINGERPRINT_CAL_STEP_4,
    FUN_FINGERPRINT_CAL_STEP_5,
    FUN_FINGERPRINT_CAL_STEP_6,
    FUN_FINGERPRINT_CAL_STEP_7,
    FUN_FINGERPRINT_CAL_STEP_8,
    FUN_FINGERPRINT_CAL_STEP_9,
    FUN_FINGERPRINT_CAL_STEP_10,
    FUN_FINGERPRINT_TEST1 = 0x101,
    FUN_FINGERPRINT_TEST2 = 0x102,
    FUN_FINGERPRINT_TEST_FINISH = 0x110,
    FUN_AGING_TEST =  0x201,
    FUN_AGING_TEST_FINISH =  0x202,
} fp_cal_test_step_t;

int32_t silfp_get_enroll_total_times(void);
int32_t silfp_pause_enroll(void);
int32_t silfp_continue_enroll(void);
int32_t silfp_pause_identify(void);
int32_t silfp_continue_identify(void);
int32_t silfp_get_alikey_status(void);
int32_t silfp_set_touch_event_listener(void);
int32_t silfp_set_screen_state(uint32_t sreenstate);
int32_t silfp_dynamically_config_log(uint32_t on);
int32_t silfp_get_engineering_info(uint32_t type);
int32_t silfp_touch_down(void);
int32_t silfp_touch_up(void);
int silfp_send_fingerprint_cmd(int32_t cmd_id, int8_t __unused *in_buf, uint32_t __unused size);

int32_t sl_fp_query_algorithm_parameter(algotirhm_paramater_t *ptr);

int silfp_send_fingerprint_cmd(int32_t cmd_id, int8_t* in_buf, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif

#endif // __SILEAD_CUST_H__
