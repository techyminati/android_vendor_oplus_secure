/******************************************************************************
 * @file   silead_notify_cust.c
 * @brief  Contains fingerprint operate functions.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
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
 * Bangxiong.Wu   2019/3/10   1.0.0      Add for saving calibrate data
 * Bangxiong.Wu   2019/03/18  1.0.1      Add for hypnusd
 * Bangxiong.Wu   2019/04/10  1.0.2      Rename function
 * Bangxiong.Wu   2019/04/14  1.0.3      Add for algo and power monitor
 *****************************************************************************/
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE

#define FILE_TAG "silead_notify_cust"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include "silead_finger.h"
#include "silead_cmd_cust.h"
#include "silead_notify_cust.h"
#include "silead_dump.h"

static fingerprint_notify_t m_notify_func_cust = NULL;
#define QUALITY_THR 45
#define QUALITY_PASS 1
#define QUALITY_FAIL 0
#define OPLUS_ALGO_VERSION "v388-1-5"    //Need Update with algo update
#define CHIP_IC 7011
#define LENS_TYPE 2

void silfp_notify_set_cust_notify_callback(fingerprint_notify_t notify)
{
    LOG_MSG_VERBOSE("set notify");

    if(NULL != notify) {
        m_notify_func_cust = notify;
    }
}

void silfp_cust_notify_send_fingerprint_cmd_notice(int32_t sub_id, int8_t *result, uint32_t len)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    LOG_MSG_DEBUG("len = %d", len);

    msg.type = FINGERPRINT_OPTICAL_SENDCMD;
    msg.data.test.cmd_id = sub_id;
    msg.data.test.result = result;
    msg.data.test.result_len = len;
    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}

void silfp_cust_notify_send_fingerprint_img_quality_notice(int32_t result, int32_t quality)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_ENGINEERING_INFO;
    msg.data.engineering.type=FINGERPRINT_IMAGE_QUALITY;
    msg.data.engineering.quality.successed = result;
    msg.data.engineering.quality.image_quality = quality;
    msg.data.engineering.quality.quality_pass = (quality >= QUALITY_THR ? QUALITY_PASS : QUALITY_FAIL);
    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}

void silfp_cust_notify_fingerprint_enumerate()
{
    fingerprint_msg_t msg;
    int ret;
    uint32_t count = MAX_ID_LIST_SIZE;
    memset(&msg, 0, sizeof(msg));

    ret = silfp_finger_enumerate6(msg.data.enumerated_oplus.finger, &count);
    if(ret >= 0) {
        msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        msg.data.enumerated_oplus.samples_remaining = count;
        //msg.data.enumerated_oplus.gid = m_gid;
        if (m_notify_func_cust != NULL) {
            m_notify_func_cust(&msg);
        }
    }
}

void silfp_cust_notify_send_finger_press_status(int32_t status)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = (fingerprint_msg_type_t) status;
    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}

#ifdef FP_HYPNUSD_ENABLE
void silfp_cust_notify_send_hypnusdsetaction(int32_t action_type, int32_t action_timeout)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_HYPNUSDSETACION;
    msg.data.hypnusd_setting.action_type = action_type;
    msg.data.hypnusd_setting.action_timeout = action_timeout;
    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}
#endif

void silfp_cust_notify_send_fingerprint_cmd_3v_notice(int32_t sub_id, int32_t ret, int32_t v, int32_t v2, int32_t v3)
{
    uint32_t result_len = 32;
    int32_t result[8] = {0};

    result[0] = 100;
    result[1] = ret;
    result[2] = 101;
    result[3] = v;
    result[4] = 102;
    result[5] = v2;
    result[6] = 103;
    result[7] = v3;

    silfp_cust_notify_send_fingerprint_cmd_notice(sub_id,(int8_t *)result, result_len);
}

void silfp_cust_notify_send_fp_cmd_with_1v_notice(int32_t cmd_id, int32_t ret)
{
    uint32_t result_len = 8;
    int32_t result[2] = {0};

    result[0] = 100;
    result[1] = ret;

    silfp_cust_notify_send_fingerprint_cmd_notice(cmd_id, (int8_t *)result, result_len);
}

static void __silfp_cust_notify_send_calib_cmd1_notice(int32_t dead_pixels_t, int32_t circle_t, int32_t diameter_t, int32_t mean_w_t,
    int32_t shading_t, int32_t shading_unit_t)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_CALIBRATE_PARAMATER;
    msg.data.algo_parm.type = FINGERPRINT_CALIB_CMD1;
    msg.data.algo_parm.data.ft_algo.dead_pixels = dead_pixels_t;
    msg.data.algo_parm.data.ft_algo.circle = circle_t;
    msg.data.algo_parm.data.ft_algo.diameter = diameter_t;
    msg.data.algo_parm.data.ft_algo.mean_w = mean_w_t;
    msg.data.algo_parm.data.ft_algo.shading = shading_t;
    msg.data.algo_parm.data.ft_algo.shading_unit = shading_unit_t;

    LOG_MSG_DEBUG("calibrate step1:[dead_pixels]=%d, [circle]=%d, [diameter]=%d, [mean]=%d, [shading]=%d, [shading_unit]=%d",
        dead_pixels_t, circle_t, diameter_t, mean_w_t, shading_t, shading_unit_t);

    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}

static void __silfp_cust_nofity_send_calib_cmd2_notice(int32_t mean_b_t, int32_t p_percent_t, int32_t p_wb_percent_t, int32_t noise_t,
    int32_t blot_t, int32_t blot_glass_t, int32_t status_t)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_CALIBRATE_PARAMATER;
    msg.data.algo_parm.type = FINGERPRINT_CALIB_CMD2;
    msg.data.algo_parm.data.ft_algo.mean_b = mean_b_t;
    msg.data.algo_parm.data.ft_algo.p_percent = p_percent_t;
    msg.data.algo_parm.data.ft_algo.p_wb_percent = p_wb_percent_t;
    msg.data.algo_parm.data.ft_algo.noise = noise_t;
    msg.data.algo_parm.data.ft_algo.blot = blot_t;
    msg.data.algo_parm.data.ft_algo.blot_glass = blot_glass_t;
    msg.data.algo_parm.data.ft_algo.status = status_t;

    LOG_MSG_DEBUG("calibrate step2:[mean_b]=%d, [p_percent]=%d, [p_wb_percent]=%d, [noise_t]=%d, [blot_t]=%d, [blot_glass_t]=%d,  [status_t]=%d",
        mean_b_t, p_percent_t, p_wb_percent_t, noise_t, blot_t, blot_glass_t, status_t);

    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}

int32_t silfp_cust_notify_cal_test_result_notice(int32_t subcmd,int32_t __unused reserve)
{
    algotirhm_paramater_t alg_ptr;
    memset(&alg_ptr, 0, sizeof(alg_ptr));
    sl_fp_query_algorithm_parameter(&alg_ptr);
    if (subcmd == FUN_FINGERPRINT_CAL_STEP_1) {
        __silfp_cust_notify_send_calib_cmd1_notice(alg_ptr.ft.dead_pixels, alg_ptr.ft.circle, alg_ptr.ft.diameter,
        alg_ptr.ft.mean_w, alg_ptr.ft.shading, alg_ptr.ft.shading_unit);
    } else if (subcmd == FUN_FINGERPRINT_CAL_STEP_2) {
            __silfp_cust_nofity_send_calib_cmd2_notice(alg_ptr.ft.mean_b, alg_ptr.ft.p_percent, alg_ptr.ft.p_wb_percent,
                alg_ptr.ft.noise, alg_ptr.ft.blot, alg_ptr.ft.blot_glass, alg_ptr.ft.status);
      }

#if ((defined SIL_DUMP_IMAGE) && (defined SL_FP_FEATURE_OPLUS_CUSTOMIZE))
    if (subcmd == FUN_FINGERPRINT_CAL_STEP_3) {
        silfp_dump_algorithm_parameter(subcmd, &alg_ptr, reserve);
    }
#endif

    return 0;
}

void silfp_cust_notify_send_auth_dcsmsg(fingerprint_auth_dcsmsg_t auth_context) {
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_AUTHENTICATED_DCSSTATUS;
    msg.data.auth_dcsmsg.auth_result = auth_context.auth_result;
    msg.data.auth_dcsmsg.fail_reason = auth_context.fail_reason;
    msg.data.auth_dcsmsg.quality_score = 0;    //UNUSED
    msg.data.auth_dcsmsg.match_score = 0;    //UNUSED
    msg.data.auth_dcsmsg.signal_value = 0;    //UNUSED
    msg.data.auth_dcsmsg.img_area = 0;    //UNUSED
    msg.data.auth_dcsmsg.retry_times = auth_context.retry_times;
    memcpy(msg.data.auth_dcsmsg.algo_version, OPLUS_ALGO_VERSION, sizeof(OPLUS_ALGO_VERSION));
    msg.data.auth_dcsmsg.chip_ic = CHIP_IC;
    msg.data.auth_dcsmsg.module_type = 0;    //UNUSED
    msg.data.auth_dcsmsg.lense_type = LENS_TYPE;
    msg.data.auth_dcsmsg.dsp_availalbe = 0;    //UNUSED

    LOG_MSG_DEBUG("authenticate, result = %d", msg.data.auth_dcsmsg.auth_result);
    LOG_MSG_DEBUG("authenticate, fail_reason = %d", msg.data.auth_dcsmsg.fail_reason);
    LOG_MSG_DEBUG("authenticate, retry_times = %d", msg.data.auth_dcsmsg.retry_times);
    LOG_MSG_DEBUG("authenticate, algo_version = %s", msg.data.auth_dcsmsg.algo_version);
    LOG_MSG_DEBUG("authenticate, chip_ic = %d", msg.data.auth_dcsmsg.chip_ic);
    LOG_MSG_DEBUG("authenticate, lens_type = %d", msg.data.auth_dcsmsg.lense_type);
    //...reversed
    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}

void silfp_cust_notify_send_monitor_power_notice(double battery) {
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_MONITOR;
    msg.data.monitor.type = FINGERPRINT_POWER_MONITOR;
    msg.data.monitor.data.power.battery = battery;

    LOG_MSG_DEBUG("power leak because mistouch!!!");
    if (m_notify_func_cust != NULL) {
        m_notify_func_cust(&msg);
    }
}

#endif
