/******************************************************************************
 * @file   silead_cust.h
 * @brief  Contains fingerprint operate functions header file.
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
 * David Wang      2018/7/2    0.1.0      Init version
 * Bangxiong.Wu    2019/03/18  1.0.0      Change hypnusd calling method
 * Bangxiong.Wu    2019/04/01  1.0.1      Add get reverse grey time function
 * Bangxiong.Wu    2019/04/10  1.0.2      Move this file to ca and compatible for HAL
 * Bangxiong.Wu    2019/04/14  1.0.3      add for powermonitor
 *
 *****************************************************************************/

#ifndef __SILEAD_CUST_H__
#define __SILEAD_CUST_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
#include "silead_cmd_cust.h"
#endif

#define UI_FLAG         0x80
#define UI_READY_SET    0x01
#define UI_READY_CLEAR  0x02
#define UI_DUMP_ENABLE  0x04
#define UI_DUMP_DISABLE 0x08
#define IS_UI_FLAG(a)           ((a) & UI_FLAG)
#define IS_UI_READY_FLAG(a)     ((a) & (UI_READY_SET | UI_READY_CLEAR))
#define IS_UI_DUMP_FLAG(a)      ((a) & (UI_DUMP_ENABLE | UI_DUMP_DISABLE))
#define GET_UI_READY_FLAG(a)    ((a) & 0x7F)
#define IS_UI_READY_SET(a)      ((a) & UI_READY_SET)
#define IS_UI_READY_CLEAR(a)    ((a) & UI_READY_CLEAR)
#define IS_UI_DUMP_ENABLE(a)    ((a) & UI_DUMP_ENABLE)
#define IS_UI_DUMP_DISABLE(a)   ((a) & UI_DUMP_DISABLE)

const char* silfp_cust_get_dump_path(void);
const char* silfp_cust_get_cal_path(void);
const char* silfp_cust_get_ta_name(void);
int32_t silfp_cust_is_capture_disable(void);

int32_t silfp_cust_need_cancel_notice(void);
int32_t silfp_cust_is_screen_from_drv(void);
int32_t silfp_cust_get_finger_status_mode(void);

void silfp_cust_finger_down_pre_action(void);
void silfp_cust_finger_down_after_action(void);
void silfp_cust_finger_up_pre_action(void);
void silfp_cust_finger_up_after_action(void);

int32_t silfp_cust_set_hbm_mode(uint32_t mode);
int32_t silfp_cust_set_brightness(uint32_t mode);
int32_t silfp_cust_get_cur_brightness(uint32_t *value);
int32_t silfp_cust_restore_hbm(void);
int32_t silfp_cust_restore_brightness(void);

void silfp_cust_capture_get_tp_info(uint8_t mode);

int32_t silfp_cust_get_sys_charging_state(void);

int32_t silfp_cust_tpl_change_action(void);

int32_t silfp_cust_tp_irq_enable(int32_t enable);
int32_t silfp_cust_clear_ui_ready(void);
int32_t silfp_cust_wait_ui_ready(void);
int32_t silfp_cust_notify_ui_ready(uint32_t addition);

int32_t silfp_cust_auth_get_retry_times(void);
int32_t  silfp_cust_auth_get_reverse_grey_times(void);
int32_t silfp_cust_auth_mistouch_ignor_screen_off(void);

int32_t silfp_cust_enroll_timeout_sec(void);
int32_t silfp_cust_enroll_cont_err_times(void);
int32_t silfp_cust_enroll_report_remain_when_error(void);
int32_t silfp_cust_trans_notice_code(int32_t code);

int32_t silfp_cust_otp_parse(void *buf, uint32_t size, uint32_t offset, uint32_t *otp, uint32_t count);

int32_t silfp_cust_send_quality_notice(int32_t result, int32_t quality);
int32_t silfp_cust_send_aging_notice(int32_t result);
int32_t silfp_cust_send_calibrate_notice(int32_t sub_id, int32_t result);
int32_t silfp_cust_send_optic_test_factory_quality_notice(int32_t result, uint32_t snr, uint32_t noise, uint32_t signal);

int32_t silfp_cust_is_optic(void);
int32_t silfp_cust_esd_support(void);

#ifdef FP_HYPNUSD_ENABLE
void silfp_cust_send_hypnusdsetaction(void);
#endif

void silfp_cust_monitor_power_reset(void);
void silfp_cust_monitor_power_start(void);
void silfp_cust_monitor_power_end(void);

#ifdef __cplusplus
}
#endif

#endif // __SILEAD_CUST_H__
