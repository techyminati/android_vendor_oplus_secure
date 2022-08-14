/******************************************************************************
 * @file   silead_cust.c
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
 * <author>    <date>   <version>     <desc>
 * David Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_cust"
#include "log/logmsg.h"

#include "silead_cust.h"
#include "silead_screen.h"

#define __WEAK __attribute__((weak))

const char* __WEAK silfp_cust_get_dump_path(void)
{
    // return default dump path
    return NULL;
}

const char* __WEAK silfp_cust_get_cal_path(void)
{
    // return default cal path
    return NULL;
}

const char* __WEAK silfp_cust_get_ta_name(void)
{
    // return default ta name
    return NULL;
}

int32_t __WEAK silfp_cust_is_capture_disable(void)
{
    // capture disabled
    return 0;
}

int32_t __WEAK silfp_cust_need_cancel_notice(void)
{
    // need notify when cancel, if return -1, use default
    return -1;
}

int32_t __WEAK silfp_cust_is_screen_from_drv(void)
{
    // get screen status from driver
    return 1;
}

int32_t __WEAK silfp_cust_get_finger_status_mode(void)
{
    return -1;
}

void __WEAK silfp_cust_finger_down_pre_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_finger_down_after_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_finger_up_pre_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_finger_up_after_action(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

int32_t __WEAK silfp_cust_set_hbm_mode(uint32_t __unused mode)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_set_brightness(uint32_t __unused mode)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_restore_hbm(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_restore_brightness(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

void __WEAK silfp_cust_capture_get_tp_info(uint8_t __unused mode)
{
    // [capture image] get tp info and send to ta, for optic
    LOG_MSG_VERBOSE("default, not impliment");
}

int32_t __WEAK silfp_cust_get_sys_charging_state(void)
{
    // [all] for charging interference
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_tpl_change_action(void)
{
    // [tpl change] action after tpl change (remove, add, load)
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_tp_irq_enable(int32_t __unused enable)
{
    // enable/disable tp irq
    return 0;
}

int32_t __WEAK silfp_cust_clear_ui_ready(void)
{
    // [auth | enroll] clear finger ready
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_wait_ui_ready(void)
{
    // [auth | enroll] wait finger ready when touch down
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_notify_ui_ready(uint32_t __unused addition)
{
    // [auth | enroll] notify finger ready
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_auth_get_retry_times(void)
{
    // [auth] retry times for each auth
    return 2;
}

int32_t __WEAK silfp_cust_auth_mistouch_ignor_screen_off(void)
{
    // [auth] mistouch and screen off, ignor notify
    return 0;
}

int32_t __WEAK silfp_cust_enroll_timeout_sec(void)
{
    // [enroll] enroll timeout (seconds)
    // if return 0, disable timeout check
    return 0;
}

int32_t __WEAK silfp_cust_enroll_cont_err_times(void)
{
    // [enroll] when cont error, report FINGERPRINT_ERROR_UNABLE_TO_PROCESS error
    // if return 0, disabled
    return 0;
}

int32_t __WEAK silfp_cust_enroll_report_remain_when_error(void)
{
    // [enroll] when error, report remain times to UI
    // 1: enable, 0: disabled
    return 0;
}

int32_t __WEAK silfp_cust_trans_notice_code(int32_t code)
{
    // [enroll] notice code trans, such as same area/same finger ect.
    if (code < 0) {
        code = -code;
    }
    return code;
}

int32_t __WEAK silfp_cust_otp_parse(void __unused *buf, uint32_t __unused size, uint32_t __unused offset, uint32_t __unused *otp, uint32_t __unused count)
{
    // otp parse
    return offset;
}

int32_t __WEAK silfp_cust_send_quality_notice(int32_t __unused result, int32_t __unused quality)
{
    // result: 0 pass, 1 failed
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_send_aging_notice(int32_t __unused result)
{
    // result: 0 pass, 1 failed
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_send_calibrate_notice(int32_t __unused sub_id, int32_t __unused result)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_send_optic_test_factory_quality_notice(int32_t __unused result, uint32_t __unused snr, uint32_t __unused noise, uint32_t __unused signal)
{
    // [factory test] optic factory quality test, result: 0 pass, 1 failed
    LOG_MSG_VERBOSE("default, not impliment");
    return 0;
}

int32_t __WEAK silfp_cust_is_optic(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
    return -1;
}

int32_t __WEAK silfp_cust_esd_support(void)
{
    return -1;
}

void __WEAK silfp_cust_auth_mismatch(void)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_get_chip_id(int32_t __unused chip_id)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_cust_get_algo_version(int32_t __unused algo_version, char __unused *algo_version_char)
{
    LOG_MSG_VERBOSE("default, not impliment");
}

void __WEAK silfp_notify_send_dcsmsg(int32_t __unused auth_result, int32_t __unused fail_reason, int32_t __unused retry_times, silfp_broken_info __unused *binfo)
{
    LOG_MSG_VERBOSE("default, not impliment");
}
