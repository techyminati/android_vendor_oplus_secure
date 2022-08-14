/******************************************************************************
 * @file   silead_ext_cb.c
 * @brief  Contains fingerprint extension operate functions.
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
 * Jack Zhang  2018/5/18    0.1.0      Init version
 * Sileadinc   2019/1/2     0.1.1      add cal & factory cust code
 * Bangxiong.Wu 2019/02/14  0.1.2      usleep 100ms before optical calibrate
 * Bangxiong.Wu 2019/02/19  0.1.3      usleep 500ms before optical calibrate
 * Bangxiong.Wu 2019/03/18  0.1.4      Change hypnusd calling method
 * Bangxiong.Wu 2019/04/02  0.1.5      usleep 50ms for hbm take effect instead of wait ui ready
 * Bangxiong.Wu 2019/04/10  1.0.0      Rename function
 * Bangxiong.Wu 2019/04/22  1.0.1      Pause image quality test when finger up
 *****************************************************************************/

#define FILE_TAG "silead_ext_cb"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_impl.h"
#include "silead_worker.h"
#include "silead_cust.h"
#include "silead_ext.h"
#include "silead_ext_cb.h"

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
#include "silead_notify_cust.h"
#include "silead_ext_cb_cust.h"
#endif

#define NOTIFY_QUALITY_ENABLE 1

typedef struct {
    int32_t cmd;
    int32_t (*dispatch) (int32_t cmd, int32_t subcmd);
} ext_cmd_info_t;

typedef struct {
    int32_t cmd;
    int32_t sub_cmd;
    func_cb cb;
} ext_cmd_data_t;

static ext_cmd_data_t m_cmd_data;

static pthread_mutex_t m_lock;
static pthread_cond_t m_worker_cond;
static int32_t m_self_test_result = 0;

static int32_t _ext_cb_self_test(int32_t cmd, int32_t subcmd);
static int32_t _ext_cb_image_quality_get(int32_t cmd, int32_t subcmd);
static int32_t _ext_cb_image_capture_aging(int32_t cmd, int32_t subcmd);
static int32_t _ext_cb_calibrate_step(int32_t cmd, int32_t subcmd);
static int32_t _ext_cb_optic_test_factory_quality(int32_t cmd, int32_t subcmd);
static const ext_cmd_info_t ext_cmd[] = {
    {EXT_CMD_SELF_TEST,                    _ext_cb_self_test},
    {EXT_CMD_IMAGE_QUALITY_GET,            _ext_cb_image_quality_get},
    {EXT_CMD_IMAGE_CAPTURE_AGING,          _ext_cb_image_capture_aging},
    {EXT_CMD_CALIBRATE_STEP,               _ext_cb_calibrate_step},
    {EXT_CMD_OPTIC_TEST_FACTORY_QUALITY,   _ext_cb_optic_test_factory_quality},
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
    {EXT_CMD_SPI_TEST,   silfp_cust_ext_cb_spi_test},
#endif
};

int32_t silfp_ext_cb_init(void)
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_worker_cond, NULL);

    pthread_mutex_lock(&m_lock);
    m_cmd_data.cmd = -1;
    m_cmd_data.sub_cmd = 0;
    m_cmd_data.cb = NULL;
    pthread_mutex_unlock(&m_lock);

    return 0;
}

int32_t silfp_ext_cb_deinit(void)
{
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_worker_cond);
    return 0;
}

int32_t silfp_ext_cb_request_sync(int32_t cmd, int32_t timeout_sec)
{
    int32_t ret = 0;
    struct timeval now;
    struct timespec outtime;

    if (timeout_sec <= 0) {
        timeout_sec = 1;
    }

    LOG_MSG_INFO("request %d, timeout %d", cmd, timeout_sec);

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + timeout_sec;
    outtime.tv_nsec = now.tv_usec * 1000;

    pthread_mutex_lock(&m_lock);

    m_self_test_result = EXT_RESULT_FAIL;
    m_cmd_data.cmd = cmd;
    m_cmd_data.sub_cmd = 0;
    m_cmd_data.cb = NULL;
    silfp_worker_set_state(STATE_EXT_CB);

    pthread_cond_timedwait(&m_worker_cond, &m_lock, &outtime);
    ret = m_self_test_result;
    pthread_mutex_unlock(&m_lock);

    LOG_MSG_INFO("cmd(%d) result(%d)", cmd, ret);
    return ret;
}

int32_t silfp_ext_cb_request(int32_t cmd, int32_t subcmd, func_cb cb)
{
    LOG_MSG_INFO("request %d: %d", cmd, subcmd);

    pthread_mutex_lock(&m_lock);
    m_cmd_data.cmd = cmd;
    m_cmd_data.sub_cmd = subcmd;
    m_cmd_data.cb = cb;
    pthread_mutex_unlock(&m_lock);

    silfp_worker_set_state(STATE_EXT_CB);

    return 0;
}

int32_t silfp_ext_cb_command(void)
{
    uint32_t i = 0;
    ext_cmd_data_t data;

    pthread_mutex_lock(&m_lock);
    memcpy(&data, &m_cmd_data, sizeof(data));
    pthread_mutex_unlock(&m_lock);

    if (data.cb != NULL) {
        LOG_MSG_INFO("run ext cb fun");
        data.cb(data.cmd, data.sub_cmd);
    } else {
        for (i = 0; i < NUM_ELEMS(ext_cmd); i++) {
            if (ext_cmd[i].cmd == data.cmd) {
                LOG_MSG_INFO("run cmd (0x%x, %d)", data.cmd, data.sub_cmd);
                ext_cmd[i].dispatch(data.cmd, data.sub_cmd);
                break;
            }
        }
        if (i >= NUM_ELEMS(ext_cmd)) {
            if (!silfp_worker_is_canceled()) {
                silfp_worker_set_state_no_signal(STATE_IDLE);
            }
        }
    }

    return 0;
}

static int32_t _ext_cb_self_test(int32_t __unused cmd, int32_t __unused subcmd)
{
    int32_t ret = 0;
    uint32_t result = 0;

    int32_t optic = silfp_impl_is_optic();

    ret = silfp_ext_test_self(optic, &result);
    if (ret < 0 || result != EXT_RESULT_PASS) {
        ret = EXT_RESULT_FAIL;
    } else {
        ret = EXT_RESULT_PASS;
    }

    pthread_mutex_lock(&m_lock);
    m_self_test_result = ret;
    pthread_cond_signal(&m_worker_cond);
    pthread_mutex_unlock(&m_lock);

    if (!silfp_worker_is_canceled()) {
        silfp_worker_set_state_no_signal(STATE_IDLE);
    }

    return 0;
}

static int32_t _ext_cb_image_quality_get(int32_t __unused cmd, int32_t __unused subcmd)
{
    int32_t ret = 0;
    uint8_t quality = 0;

    ret = silfp_impl_wait_finger_up_with_cancel();
    if (ret >= 0) {
        ret = silfp_impl_get_finger_down_with_cancel();
    }

    if (!silfp_impl_is_finger_down()) { //finger up, pause image quality test
         LOG_MSG_DEBUG("finger up, pause image quality test");
         return 0;
    }

    silfp_cust_set_hbm_mode(1);
    usleep(50 * 1000);//usleep 50ms for hbm take effect
    if (ret >= 0) {
        ret = silfp_impl_capture_image(IMG_CAPTURE_QUALITY_TEST, 0);
    }

    if (ret >= 0) {
        ret = silfp_ext_test_image_test(EXT_IMG_FEATURE_QAULITY_MASK, NULL, NULL, &quality, NULL, NULL, NULL, NULL);

        silfp_worker_set_state_no_signal(STATE_WAIT);
        if (ret < 0) {
            LOG_MSG_INFO("get image quality failed! ret = (%d)", ret);
            silfp_cust_send_quality_notice(NOTIFY_QUALITY_ENABLE, quality);
        } else {
            LOG_MSG_INFO("get image quality successed, quality = (%d)", quality);
            silfp_cust_send_quality_notice(NOTIFY_QUALITY_ENABLE, quality);
        }
    } else{
        LOG_MSG_INFO("capture image failed!");
        silfp_cust_send_quality_notice(NOTIFY_QUALITY_ENABLE, quality);
    }

    silfp_cust_restore_hbm();

    return 0;
}

static int32_t _ext_cb_image_capture_aging(int32_t __unused cmd, int32_t __unused subcmd)
{
    int32_t ret = 0;

    silfp_impl_download_normal();
    silfp_impl_capture_image_pre();
    do {
#ifdef FP_HYPNUSD_ENABLE
        silfp_cust_send_hypnusdsetaction();
#endif
        silfp_cust_set_hbm_mode(1);
        ret = silfp_impl_capture_image_raw(IMG_CAPTURE_AGING_TEST, 0);
        silfp_cust_restore_hbm();
        silfp_cust_send_aging_notice(EXT_RESULT_PASS);
        if (!silfp_worker_is_canceled()) {
            silfp_worker_wait_condition(1000);
        }
    } while (!silfp_worker_is_canceled());

    return 0;
}

static int32_t _ext_cb_calibrate_step(int32_t __unused cmd, int32_t subcmd)
{
    int32_t ret = 0;
    static uint32_t brightness_bak = 0;
    ret = silfp_cust_get_cur_brightness(&brightness_bak);

    if (!ret) {
        LOG_MSG_DEBUG("back up brightness value %d", brightness_bak);
    } else {
        LOG_MSG_ERROR("get current brightness fail,ret = %d", ret);
    }

    switch (subcmd) {
    case TEST_SUB_CMD_CAL_STEP_1: {
        ret = silfp_cust_set_hbm_mode(1);
        if (!ret) {
            usleep(500*1000);
            ret = silfp_impl_cal_step(1);
        } else {
            LOG_MSG_ERROR("set hbm mode in step 1 fail,ret = %d", ret);
        }
        silfp_cust_restore_hbm();
        break;
    }
    case TEST_SUB_CMD_CAL_STEP_2: {
        ret = silfp_cust_set_hbm_mode(1);
        if (!ret) {
            usleep(500*1000);
            ret = silfp_impl_cal_step(2);
        } else {
            LOG_MSG_ERROR("set hbm mode in step 2 fail,ret = %d", ret);
        }
        silfp_cust_restore_hbm();
        break;
    }
    case TEST_SUB_CMD_CAL_STEP_3: {
        silfp_cust_restore_hbm();
        ret = silfp_cust_set_brightness(BRIGHTNESS_ALL);
        if (!ret) {
            usleep(500*1000);
            ret = silfp_impl_cal_step(3);
        } else {
            LOG_MSG_ERROR("set brightness all in step 3 fail,ret = %d", ret);
        }
        silfp_cust_set_brightness(brightness_bak);
        break;
    }
    default: {
        LOG_MSG_ERROR("test %d not implement", subcmd);
        break;
    }
    }
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
    //get alg param
    LOG_MSG_DEBUG("subcmd:%d get alg param", subcmd);
    silfp_cust_notify_cal_test_result_notice(subcmd,0);
#endif

    silfp_cust_send_calibrate_notice(subcmd, ret);
    if (!silfp_worker_is_canceled()) {
        silfp_worker_set_state_no_signal(STATE_IDLE);
    }

    return 0;
}

static int32_t _ext_cb_optic_test_factory_quality(int32_t __unused cmd, int32_t __unused subcmd)
{
    int32_t ret = 0;
    uint32_t qa_result = 0;
    uint32_t quality  = 0;
    uint32_t length = 0;

    int32_t ret2 = 0;
    uint32_t snr_result = 0;
    uint32_t snr = 0;
    uint32_t noise = 0;
    uint32_t signal = 0;

    silfp_cust_set_hbm_mode(1);
    silfp_impl_download_normal();

    ret = silfp_ext_optic_test_factory_quality(&qa_result, &quality, &length);
    if (qa_result == EXT_RESULT_PASS && ret >= 0) {
        ret = EXT_RESULT_PASS;
    } else {
        ret = EXT_RESULT_FAIL;
        LOG_MSG_ERROR("ret: %d, result: %u, quality:%u, length:%u", ret, qa_result, quality, length);
    }

    ret2 = silfp_ext_optic_test_snr(&snr_result, &snr, &noise, &signal);
    if (ret == EXT_RESULT_PASS && (ret2 < 0)) { // quality test pass, snr fail, return fail
        ret = EXT_RESULT_FAIL;
    }
    silfp_cust_restore_hbm();
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
    silfp_cust_notify_send_fingerprint_cmd_3v_notice(FUN_FINGERPRINT_TEST1,ret, snr, noise, signal);
#else
    silfp_cust_send_optic_test_factory_quality_notice(ret, snr, noise, signal);
#endif
    if (!silfp_worker_is_canceled()) {
        silfp_worker_set_state_no_signal(STATE_IDLE);
    }

    return 0;
}
