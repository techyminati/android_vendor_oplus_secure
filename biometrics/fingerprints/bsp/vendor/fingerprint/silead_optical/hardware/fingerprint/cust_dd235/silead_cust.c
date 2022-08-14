/**********************************************************************************************************
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
 * ------------------- Revision History -----------------------------------------------------------------
 * <author>        <date>   <version>     <desc>
 * Sileadinc      2019/1/2    0.1.0      Init version
 * Bangxiong.Wu   2019/2/24   0.1.1      Fix rapid lifting bug
 * Bangxiong.Wu   2019/2/24   0.1.2      Correct calibrate step 3
 * Bangxiong.Wu   2019/3/22   0.1.3      Enable enroll report remain when error and error max enroll times
 * Bangxiong.Wu   2019/03/18  1.0.0      Move hypnusd call to hidl
 * Bangxiong.Wu   2019/03/28  1.0.1      If wait ui ready timeout,no longer capture image and return canceled
 * Bangxiong.Wu   2019/04/01  1.0.2      Add reverse grey time function
 * Bangxiong.Wu   2019/04/10  1.0.3      Compatible for HAL
 * Bangxiong.Wu   2019/04/14  1.0.4      Add for algo and power monitor
 * Bangxiong.Wu   2019/05/07  1.0.5      Add log for enable&disable tp node
 *
 *********************************************************************************************************/
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
#define FILE_TAG "silead_cust"
#include "log/logmsg.h"

#include "silead_cust.h"
#include "silead_screen.h"

#include "fingerprint.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "silead_error.h"
#include "silead_fp.h"
#include "silead_const.h"
#include "silead_worker.h"
#include "silead_const.h"
#include "silead_impl.h"
#include "silead_enroll.h"
#include "silead_ext_cb.h"
#include "silead_error.h"
#include "silead_notify.h"
#include "silead_finger.h"
#include "fingerprint.h"
#include "silead_notify_cust.h"

/*CONSTANT PATH*/
#define FP_CONFIG_ENABLE_TPIRQ_PATH "/proc/touchpanel/fp_enable"
#define FP_CONFIG_SCREEN_HBM_PATH "/sys/devices/virtual/mtk_disp_mgr/mtk_disp_mgr/LCM_HBM"
#define FP_CONFIG_SCREEN_HBM_PATH2 "/sys/kernel/oplus_display/hbm"
#define FP_CONFIG_SCREEN_BRIGHTNESS_PATH "/sys/kernel/oplus_display/oplus_brightness"
#define FP_CONFIG_SCREEN_BRIGHTNESS_PATH2 "XXXXXXXXXX4"

/*CONSTANT NUMBER*/
#define MAX_CONTINUOUS_ENROLL_EROLL_COUNT    10
#define ENABLE_REPORT_WHEN_ENROLL_ERROR    1
#define MAX_RETRY_TIMES 3
#define REVERSE_GREY_TIME 0 //0:disable reverse grey;1~3:enable reverse grey in step retry0 or retry1 or retry2
#define MAX_ENROLL_TIMEOUT_SEC    600
#define ACTION_TYPE 12
#define ACTION_TIMEOUT 1000
#define POWER_LEAK_PER_MISTOUCH  3.14159

extern fingerprint_notify_t m_notify_func;
volatile static int32_t m_finger_ready = 0;  //flag for ui ready infact
int32_t m_is_mistouch_mode = 0;

const char* silfp_cust_get_dump_path(void)
{
    return NULL;
}

const char* silfp_cust_get_cal_path(void)
{
    return NULL;
}

const char* silfp_cust_get_ta_name(void)
{
    return NULL;
}

inline int32_t silfp_is_finger_ready(void)
{
    return m_finger_ready;
}

int32_t silfp_cust_is_capture_disable(void)
{
    return 0;
}

int32_t silfp_cust_need_cancel_notice(void)
{
    return 0;
}

int32_t silfp_cust_is_screen_from_drv(void)
{
    return 0;
}

int32_t silfp_cust_get_finger_status_mode(void)
{
    return SIFP_FINGER_STATUS_TP;
}

void silfp_cust_finger_down_pre_action(void)
{

}

void silfp_cust_finger_down_after_action(void)
{
    silfp_cust_notify_send_finger_press_status(FINGERPRINT_TOUCH_DOWN);
}

void silfp_cust_finger_up_pre_action(void)
{
}

void silfp_cust_finger_up_after_action(void)
{
    silfp_cust_notify_send_finger_press_status(FINGERPRINT_TOUCH_UP);
}

#ifdef FP_HYPNUSD_ENABLE
void silfp_cust_send_hypnusdsetaction(void)
{
    silfp_cust_notify_send_hypnusdsetaction(ACTION_TYPE, ACTION_TIMEOUT);
}
#endif

int32_t silfp_cust_set_hbm_mode(uint32_t  mode)
{
    char buff[50];
    int fd = open(FP_CONFIG_SCREEN_HBM_PATH,O_WRONLY);
    if(fd<0) {
        LOG_MSG_ERROR("open hbm node1 failed,try node2!");
        fd = open(FP_CONFIG_SCREEN_HBM_PATH2,O_WRONLY);
        if(fd<0) {
            LOG_MSG_ERROR("open failed,hbm node not exist!");
            return fd;
        }
    }
    sprintf(buff,"%d",(int)mode);
    write(fd,buff,50);
    close(fd);
    return 0;
}

int32_t silfp_cust_set_brightness(uint32_t  mode)
{
    char buff[50];
    int fd = open(FP_CONFIG_SCREEN_BRIGHTNESS_PATH,O_WRONLY);
    if(fd<0) {
        LOG_MSG_ERROR("open brightness node1 failed,try node2!");
        fd = open(FP_CONFIG_SCREEN_BRIGHTNESS_PATH2,O_WRONLY);
        if(fd<0) {
            LOG_MSG_ERROR("open failed,brightness node not exist!");
            return fd;
        }
    }
    sprintf(buff,"%d",(int)mode);
    write(fd,buff,50);
    close(fd);
    return 0;
}

int32_t silfp_cust_get_cur_brightness(uint32_t *value)
{
    char buff[16];
    uint32_t bright_value = 0;
    int length;
    int fd = open(FP_CONFIG_SCREEN_BRIGHTNESS_PATH,O_RDWR);
    if(fd<0) {
        LOG_MSG_ERROR("open brightness node1 failed,try node2!");
        fd = open(FP_CONFIG_SCREEN_BRIGHTNESS_PATH2,O_RDWR);
        if(fd<0) {
            LOG_MSG_ERROR("open failed,brightness node not exist!");
            return fd;
        }
    }
    length = read(fd,buff,10);
    if(length > 0){
        bright_value = atoi(buff);
    }else{
        LOG_MSG_ERROR("read failed.");
    }
    *value = bright_value;
    LOG_MSG_DEBUG("bright_value=%d--.",bright_value);
    close(fd);
    return 0;
}

int32_t silfp_cust_restore_hbm(void)
{
    int32_t ret = 0;
    ret = silfp_cust_set_hbm_mode(0);
    if (ret) {
        LOG_MSG_ERROR("restore hbm failed!");
    }
    return 0;
}

int32_t silfp_cust_restore_brightness(void)
{
    return 0;
}

void silfp_cust_capture_get_tp_info(uint8_t __unused mode)
{
	int ret = -1;

    ret = sl_get_tp_touch_info(mode);
}

int32_t silfp_cust_get_sys_charging_state(void)
{
    return 0;
}

int32_t silfp_cust_tpl_change_action(void)
{
    // [tpl change] action after tpl change (remove, add, load)
    silfp_cust_notify_fingerprint_enumerate();
    return 0;
}

int32_t silfp_cust_tp_irq_enable(int32_t  enable)
{
    char buff[2];
    int fd = open(FP_CONFIG_ENABLE_TPIRQ_PATH,O_WRONLY);
    LOG_MSG_INFO("enable_tp mode = %d.", enable);
    if(fd<0) {
        LOG_MSG_ERROR("sl_fp_set_touch_irq_enable fail!");
        return fd;
    }
    sprintf(buff,"%d",(int)enable);
    write(fd,buff,2);
    close(fd);
    return fd;
}

int32_t silfp_cust_clear_ui_ready(void)
{
    LOG_MSG_VERBOSE("clear ui ready flag");
    m_finger_ready = 0;
    return 0;
}
int32_t silfp_cust_wait_ui_ready(void)
{
    int iRetry = 200;
    while((iRetry > 0) && (!silfp_worker_is_canceled ())) {
        if(silfp_is_finger_ready() && silfp_impl_is_finger_down()) {
            LOG_MSG_VERBOSE("ui is ready");
            silfp_cust_clear_ui_ready();
            return SL_SUCCESS;
        }
        if(!silfp_impl_is_finger_down()){
            LOG_MSG_INFO("finger up, move to fast.");
            return -SL_ERROR_MOVE_TOO_FAST;
        }
        iRetry--;
        usleep(100 * 25); // 2.5ms
    }
    silfp_cust_clear_ui_ready();

    if (silfp_worker_is_canceled()) {
        return -SL_ERROR_CANCELED;
    }

    LOG_MSG_DEBUG("wait ui ready timeout, retry 200 times and wait 500ms!");
    return -SL_ERROR_CHECK_ICON_FAILED;
}

int32_t silfp_cust_notify_ui_ready(uint32_t __unused addition)
{
    LOG_MSG_VERBOSE("notify ui ready");
    m_finger_ready = 1;
    return 0;
}

int32_t silfp_cust_auth_get_retry_times(void)
{
    return MAX_RETRY_TIMES;
}

int32_t  silfp_cust_auth_get_reverse_grey_times(void)
{
    return REVERSE_GREY_TIME;
}

int32_t silfp_cust_auth_mistouch_ignor_screen_off(void)
{
    return 0;
}

int32_t silfp_cust_enroll_timeout_sec(void)
{
    return MAX_ENROLL_TIMEOUT_SEC;//enroll timeout sec
}

int32_t silfp_cust_enroll_cont_err_times(void)
{
    return MAX_CONTINUOUS_ENROLL_EROLL_COUNT;
}

int32_t silfp_cust_enroll_report_remain_when_error(void)
{
    return ENABLE_REPORT_WHEN_ENROLL_ERROR;
}
int32_t silfp_cust_trans_notice_code(int32_t code)
{
    if (code < 0) {
        code = -code;
    }
    return code;
}

void silfp_cust_monitor_power_reset(void) {
    m_is_mistouch_mode = 0;
}

void silfp_cust_monitor_power_start(void) {
    m_is_mistouch_mode = 1;
}

void silfp_cust_monitor_power_end(void) {
    if (m_is_mistouch_mode) {
        silfp_cust_notify_send_monitor_power_notice(POWER_LEAK_PER_MISTOUCH);
    }
}

int32_t silfp_cust_otp_parse(void __unused *buf, uint32_t __unused size, uint32_t __unused offset, uint32_t __unused *otp, uint32_t __unused count)
{
    return offset;
}

int32_t silfp_cust_send_quality_notice(int32_t __unused result, int32_t __unused quality)
{
    silfp_cust_notify_send_fingerprint_img_quality_notice(result,quality);
    return 0;
}

int32_t silfp_cust_send_aging_notice(int32_t __unused result)
{
    // result: 0 pass, 1 failed
    //LOG_MSG_VERBOSE("default, not impliment");
    uint32_t result_len = 8;
    int32_t send_result[2] = {0};

    send_result[0] = 100;
    send_result[1] = result;

    silfp_cust_notify_send_fingerprint_cmd_notice(FUN_FINGERPRINT_TEST2, (int8_t *)send_result, result_len);
    return 0;
}

int32_t silfp_cust_send_calibrate_notice(int32_t __unused sub_id, int32_t __unused result)
{
    //LOG_MSG_VERBOSE("default, not impliment");
    uint32_t result_len = 8;
    int32_t send_result[2] = {0};

    send_result[0] = 100;
    send_result[1] = result;

    silfp_cust_notify_send_fingerprint_cmd_notice(sub_id, (int8_t *)send_result, result_len);
    return 0;
}

int32_t silfp_cust_send_optic_test_factory_quality_notice(int32_t __unused result, uint32_t __unused snr, uint32_t __unused noise, uint32_t __unused signal)
{
    // [factory test] optic factory quality test, result: 0 pass, 1 failed
    //LOG_MSG_VERBOSE("default, not impliment");
    uint32_t result_len = 32;
    int32_t send_result[8] = {0};

    send_result[0] = 100;
    send_result[1] = result;
    send_result[2] = 101;
    send_result[3] = snr;
    send_result[4] = 102;
    send_result[5] = noise;
    send_result[6] = 103;
    send_result[7] = signal;

    silfp_cust_notify_send_fingerprint_cmd_notice(FUN_FINGERPRINT_TEST1, (int8_t *)send_result, result_len);
    return 0;
}

int32_t silfp_cust_is_optic(void)
{
    return 1;
}

int32_t silfp_cust_esd_support(void)
{
    return -1;
}

int32_t silfp_get_enroll_total_times(void)
{
    uint32_t num;
    if (silfp_impl_get_enroll_num(&num) < 0) {
        num = 0;
    }
    LOG_MSG_DEBUG("enroll count: %d", num);
    return num;
}

int32_t silfp_pause_enroll(void)
{
    LOG_MSG_DEBUG("enroll pause");
    return silfp_enroll_pause(1);
}

int32_t silfp_continue_enroll(void)
{
    LOG_MSG_DEBUG("enroll continue");
    return silfp_enroll_pause(0);
}

int32_t silfp_pause_identify(void)
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int32_t silfp_continue_identify(void)
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int32_t silfp_get_alikey_status(void)
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int32_t silfp_set_touch_event_listener(void)
{
    LOG_MSG_DEBUG("set touch event");
    silfp_worker_set_state(STATE_LOCK);
    return 0;
}

int32_t silfp_set_screen_state(uint32_t sreenstate)
{
    LOG_MSG_DEBUG("set screen state %d", sreenstate);
    if (FINGERPRINT_SCREEN_OFF == sreenstate) {
        silfp_worker_screen_state_callback(0, NULL);
    } else {
        silfp_worker_screen_state_callback(1, NULL);
    }
    return 0;
}

int32_t silfp_dynamically_config_log(uint32_t __unused on)
{
    LOG_MSG_DEBUG("set dynamically config log not implement");
    return 0;
}

int32_t silfp_get_engineering_info(uint32_t type)
{
    int ret = 0;
    switch (type) {
    case FINGERPRINT_GET_IMAGE_QUALITYS: {
        LOG_MSG_DEBUG("image quality get");
        ret = silfp_ext_cb_request(EXT_CMD_IMAGE_QUALITY_GET, 0, NULL);
        break;
    }
    case FINGERPRINT_SELF_TEST: {
        ret = silfp_ext_cb_request_sync(EXT_CMD_SELF_TEST, 0);
        break;
    }
    default: {
        LOG_MSG_DEBUG("test %d not implement", type);
        break;
    }
    }
    return ret;
}

int32_t silfp_touch_down(void)
{
    LOG_MSG_DEBUG("touch down");
    return 0;
}

int32_t silfp_touch_up(void)
{
    LOG_MSG_DEBUG("touch up");
    return 0;
}

int silfp_send_fingerprint_cmd(int32_t cmd_id, int8_t __unused *in_buf, uint32_t __unused size)
{
    int32_t ret = 0;
    LOG_MSG_DEBUG("enter cmd= %d ", cmd_id);
    switch (cmd_id) {
        case FUN_FINGERPRINT_TEST_START:
            LOG_MSG_DEBUG("test %d not implement", cmd_id);
            break;
        case FUN_FINGERPRINT_CAL_STEP_1:
        case FUN_FINGERPRINT_CAL_STEP_2:
        case FUN_FINGERPRINT_CAL_STEP_3:
        case FUN_FINGERPRINT_CAL_STEP_4:
        case FUN_FINGERPRINT_CAL_STEP_5:
        case FUN_FINGERPRINT_CAL_STEP_6:
        case FUN_FINGERPRINT_CAL_STEP_7:
        case FUN_FINGERPRINT_CAL_STEP_8:
        case FUN_FINGERPRINT_CAL_STEP_9:
        case FUN_FINGERPRINT_CAL_STEP_10:
            silfp_ext_cb_request(EXT_CMD_CALIBRATE_STEP, cmd_id, NULL);
            break;
        case FUN_FINGERPRINT_TEST1:
            silfp_ext_cb_request_sync(EXT_CMD_OPTIC_TEST_FACTORY_QUALITY, 1);
            break;
        case FUN_FINGERPRINT_TEST2:
            LOG_MSG_DEBUG("test %d not implement", cmd_id);
            silfp_cust_notify_send_fp_cmd_with_1v_notice(cmd_id, 0);
            break;
        case FUN_FINGERPRINT_TEST_FINISH:
            LOG_MSG_DEBUG("test %d not implement", cmd_id);
            break;
        case FUN_AGING_TEST:
            silfp_ext_cb_request_sync(EXT_CMD_IMAGE_CAPTURE_AGING,1);
            break;
        case FUN_AGING_TEST_FINISH: {
            silfp_finger_cancel();
            break;
        }
        default: {
            LOG_MSG_DEBUG("default, test %d not implement", cmd_id);
            silfp_cust_notify_send_fp_cmd_with_1v_notice(cmd_id, 0);
            break;
        }
    }

    return ret;
}
#endif
