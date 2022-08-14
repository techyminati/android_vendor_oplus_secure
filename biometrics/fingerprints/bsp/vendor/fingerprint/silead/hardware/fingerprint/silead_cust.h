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
 * <author>    <date>   <version>     <desc>
 * David Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_CUST_H__
#define __SILEAD_CUST_H__

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
#define SILFP_CHECK_RESULT 999

typedef enum silfp_cust_error {
    FINGERPRINT_ERROR_READ_CHIP_ID = 7,/*chip id fail*/
    FINGERPRINT_ERROR_CHECK_REG = 8,//reg check fail
    FINGERPRINT_ERROR_CHECK_PIXEL = 9,//pixel check fail
    FINGERPRINT_ERROR_CHECK_SPI_TIMEOUT = 1026,//spi timeout
    FINGERPRINT_ERROR_NO_FINGER_DEADPX_CHECK_FAIL = 1081,//no finger deadpx check  fail
    FINGERPRINT_ERROR_BADLINE_CHECK_FAIL = 1082,//badline check fail
    FINGERPRINT_ERROR_GREY_DIFF_CHECK_FAIL = 1083,//grey_diff check fail
    FINGERPRINT_ERROR_HAVE_FINGER_DEADPX_CHECK_FAIL = 1084,//have finger deadpx check fail
} silfp_cust_error_t;

typedef struct silfp_cust_broken_info {
    uint32_t grey1;
    uint32_t grey2;
    uint32_t grey_diff;
    uint32_t deadpx;
    uint32_t badline;
} silfp_broken_info;

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
void silfp_cust_auth_mismatch(void);
void silfp_cust_get_chip_id(int32_t chip_id);
void silfp_cust_get_algo_version(int32_t algo_version, char *algo_version_char);
void silfp_notify_send_dcsmsg(int32_t auth_result, int32_t fail_reason, int32_t retry_times, silfp_broken_info* binfo);

#endif // __SILEAD_CUST_H__
