/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <cutils/properties.h>

#include "fingerprint_type.h"

#include "gf_ca_entry.h"
#include "gf_common.h"
#include "gf_hal_common.h"
#include "gf_hal_log.h"
#include "gf_hal.h"

#include "gf_hal_device.h"
#include "gf_queue.h"
#include "gf_hal_timer.h"
#include "gf_sec_ptrace.h"
#include "gf_hal_mem.h"
#ifdef SUPPORT_FUNCTIONAL_TEST
#include "gf_test_service.h"
#endif  // SUPPORT_FUNCTIONAL_TEST

#include "gf_hal_test.h"

#define LOG_TAG "[GF_HAL][gf_hal]"

#define ALGO_VERSION    "Milan_v_3.02.00.35"

volatile gf_work_state_t g_work_state = STATE_IDLE;  // work state
volatile gf_custom_state_t g_custom_state = STATE_TEST_IDLE;  // for oplus
gf_enroll_state_t g_enroll_state = GF_ENROLL_STATE_NORMALL; // for oplus

gf_fingerprint_device_t *g_fingerprint_device = NULL;  // fingerprint device

gf_hal_function_t g_hal_function = { 0 };  // hal function

timer_t g_irq_timer_id = 0;  // id of irq timer

gf_config_t g_hal_config = { 0 };  // hal config

/**
 * Function: gf_hal_user_invoke_command
 * Description: This function is invoking interface for user.
 * Input: cmd_id, buffer, len
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_user_invoke_command(uint32_t cmd_id, void *buffer, uint32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        // check params
        // exit loop if fail
        if (NULL == buffer)
        {
            LOG_E(LOG_TAG, "[%s] buffer is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.user_invoke_command(cmd_id, buffer, len);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_open
 * Description: open gf_hal, when service starting.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_open(void *dev)
{
    gf_error_t err = GF_SUCCESS;

#ifdef __SUPPORT_GF_MEMMGR
    GF_HAL_START_MEMORY_CHECK();
#endif  // #ifdef __SUPPORT_GF_MEMMGR

    gf_detect_sensor_t *cmd = NULL;
    uint8_t download_fw_flag = 0;
    uint8_t is_device_opened = 0;
    uint8_t is_session_opened = 0;
    uint8_t is_power_enabled = 0;
    uint32_t size = sizeof(gf_detect_sensor_t);
    uint8_t retry_time = 4;
    char algo_version[ALGO_VERSION_INFO_LEN] = { 0 };
    FUNC_ENTER();

    // check params
    // return if fail
    if (NULL == dev)
    {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    LOG_I(LOG_TAG, "[%s] SOFT VERSION INFO PRINT", __func__);
    LOG_I(LOG_TAG, "[%s] TARGET_MODE=%s", __func__, GF_TARGET_MODE);
    LOG_I(LOG_TAG, "[%s] PACKAGE_VERSION_CODE=%s", __func__,
          GF_PACKAGE_VERSION_CODE);
    LOG_I(LOG_TAG, "[%s] PACKAGE_VERSION_NAME=%s", __func__,
          GF_PACKAGE_VERSION_NAME);
    LOG_I(LOG_TAG, "[%s] GIT_BRANCH=%s", __func__, GF_GIT_BRANCH);
    LOG_I(LOG_TAG, "[%s] COMMIT_ID=%s", __func__, GF_COMMIT_ID);
    LOG_I(LOG_TAG, "[%s] BUILD_TIME=%s", __func__, GF_BUILD_TIME);

    gf_sec_ptrace_self();
    g_fingerprint_device = (gf_fingerprint_device_t *) dev;
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        err = gf_hal_device_open();

        if (err != GF_SUCCESS)
        {
            break;
        }

#ifdef SUPPORT_FUNCTIONAL_TEST
        err = gf_test_service_init();
#endif  // SUPPORT_FUNCTIONAL_TEST

        is_device_opened = 1;

        err = gf_ca_open_session();
        if (err != GF_SUCCESS)
        {
            break;
        }
        is_session_opened = 1;

        err = gf_hal_enable_power();

        if (err != GF_SUCCESS)
        {
            break;
        }
        is_power_enabled = 1;

        err = gf_hal_get_fw_info(&download_fw_flag);

        // SystemServer reboot, chip maybe in MODE_SLEEP, need reset
        // download_fw_flag == 1, need force update fw, can't reset chip.
        // Only Oswego-M & Trustonic tee download_fw_flag maybe is 1.
        if (!download_fw_flag)
        {
            gf_hal_reset_chip();
        }

        cmd = (gf_detect_sensor_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        // try 5 times when detect sensor failed
        do
        {
            memset(cmd, 0, size);
            err = gf_hal_control_spi_clock(1);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] spi clock enable failed", __func__);
                break;
            }

            cmd->retry_time = retry_time;
            // send cmd GF_CMD_DETECT_SENSOR to detect sensor
            err = gf_ca_invoke_command(GF_OPERATION_ID, GF_CMD_DETECT_SENSOR, cmd, size);
            gf_hal_control_spi_clock(0);
            if (err == GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] detect sensor success", __func__);
                break;
            }
            if (--retry_time > 0)
            {
                // detect sensor failed to reset chip and retry
                gf_hal_reset_chip();
                LOG_E(LOG_TAG, "[%s] detect sensor failed , need to retry, retry_time = %d",
                    __func__, retry_time);
                usleep(5000);
            }
        }  // do retry_time > 0
        while (retry_time > 0);

        if (err != GF_SUCCESS)
        {
            break;
        }
        else
        {
            memcpy(&g_hal_config, &cmd->config, sizeof(gf_config_t));
            // init hal function pointers
            err = gf_hal_function_init(&g_hal_function, g_hal_config.chip_series);

            if (err != GF_SUCCESS)
            {
                break;
            }

            err = g_hal_function.init((void *) g_fingerprint_device);

            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] hal init failed. err=%s, errno=%d", __func__, gf_strerror(err),
                        err);
                break;
            }
            else
            {
                // hal open sucess to set g_hal_inited = 1
                g_hal_inited = 1;
                if (hal_test_get_version(algo_version))
                {
                    LOG_E(LOG_TAG, "[%s] hal_test_get_version failed.", __func__);
                }
                if (strlen(algo_version) == 0)
                {
                    memcpy(algo_version, ALGO_VERSION, strlen(ALGO_VERSION));
                }
                LOG_E(LOG_TAG, "[%s], algo_version:%s", __func__,algo_version);
                property_set(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, algo_version);
            }
        }
    }  // do gf_hal_open
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (GF_SUCCESS != err)
    {
        if (is_power_enabled)
        {
            gf_hal_disable_power();
        }

        gf_hal_device_remove();
        // MTK_drivers gf_hal_device_remove() will disable power, QUALCOMM_drivers will not

        if (is_session_opened)
        {
            gf_ca_close_session();
        }
        if (is_device_opened)
        {
            gf_hal_device_close();
        }
    }

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_close
 * Description: close gf_hal, when service stopping.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_close(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    // check params
    // return if fail
    if (NULL == dev)
    {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        g_hal_inited = 0;
        err = g_hal_function.close(dev);
    }
    while (0);

    g_fingerprint_device = NULL;
    pthread_mutex_unlock(&g_hal_mutex);

#ifdef __SUPPORT_GF_MEMMGR
    GF_HAL_CLOSE_MEMORY_CHECK();
#endif  // #ifdef __SUPPORT_GF_MEMMGR

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_stop_camera_capture(void *dev);
int gf_keymode_enable(void *dev, int enable) {
    gf_error_t err = GF_SUCCESS;
    const int KEY_MODE_ENABLE = 1;
    if (dev == NULL) {
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }
    LOG_I(LOG_TAG, "[%s]", __func__);

    if (enable == KEY_MODE_ENABLE)
        err = gf_hal_camera_capture(dev);
    else
        err = gf_hal_stop_camera_capture(dev);

    return err;
}

/**
 * Function: gf_hal_cancel
 * Description: Cancel operation and stop timer.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s]", __func__);

    // check params
    // return if fail
    if (NULL == dev)
    {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    // It's a single thread in java service,
    // we need sync work state with irq thread & java service
    if (g_work_state != STATE_IDLE)
    {
#if defined(__ANDROID_N) || defined(__ANDROID_O) || defined(__ANDROID_P)
        //gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_CANCELED);
#endif  // __ANDROID_N || __ANDROID_O
        g_work_state = STATE_IDLE;
    }

    /*
    if (STATE_TOUCH_WORKING == g_custom_state) {
        gf_hal_set_fdt_sensitivity(FDT_TRIGGER_BLOCK_LOW, FDT_DOWN_DELTA_LOW);
    }
    */

    if (GF_ENROLL_STATE_PAUSED == g_enroll_state) {
        gf_hal_continue_enroll();
    }

    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        if (GF_ENROLL_STATE_PAUSED == g_enroll_state) {
            LOG_I(LOG_TAG, "[%s] cancel, so g_enroll_state should be GF_ENROLL_STATE_NORMALL", __func__);
            err = g_hal_function.cancel(dev);
            g_enroll_state = GF_ENROLL_STATE_NORMALL;
        }

        if (STATE_TEST_WORKING == g_custom_state) {
            LOG_I(LOG_TAG, "[%s] g_custom_state is STATE_TEST_WORKING", __func__);
            err = g_hal_function.test_cancel(dev);
            g_custom_state = STATE_TEST_IDLE;
        } else if (STATE_TOUCH_WORKING == g_custom_state) {
            err = gf_hal_invoke_command_ex(GF_CMD_USER_SET_CANCEL);
            g_custom_state = STATE_TEST_IDLE;
        } else {
            err = g_hal_function.cancel(dev);
        }
    }
    while (0);

    gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_CANCELED);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_pre_enroll
 * Description: Prepare for enrollment.
 * Input: dev
 * Output: None
 * Return: uint64_t
 */
uint64_t gf_hal_pre_enroll(void *dev)
{
    uint64_t ret = 0;
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    // check params
    // return if fail
    if (NULL == dev)
    {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        return 0;
    }

    LOG_I(LOG_TAG, "[%s]", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        ret = g_hal_function.pre_enroll(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return ret;
}

/**
 * Function: gf_hal_enroll
 * Description: Send command to TA, and start enrollment.
 * Input: dev, hat, group_id, timeout_sec
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enroll(void *dev, const void *hat, uint32_t group_id,
                         uint32_t timeout_sec)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] group_id=%u, timeout_sec=%u", __func__, group_id,
          timeout_sec);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if ((NULL == hat) || (NULL == dev))
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] bad paramters", __func__);
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.enroll(dev, hat, group_id, timeout_sec);
        g_work_state = STATE_ENROLL;
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_post_enroll
 * Description: After enroll finger, reset g_challenge in TA.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_post_enroll(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s]", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.post_enroll(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_authenticate_fido
 * Description: Authentication for fido.
 * Input: dev, group_id, aaid, aaid_len, challenge, challenge_len
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_authenticate_fido(void *dev, uint32_t group_id, uint8_t *aaid,
                                    uint32_t aaid_len,
                                    uint8_t *challenge, uint32_t challenge_len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        // check params
        // exit loop if fail
        if (NULL == dev || NULL == aaid || NULL == challenge)
        {
            LOG_E(LOG_TAG, "[%s] dev or aaid or challenge is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        pthread_mutex_lock(&g_hal_mutex);
        err = g_hal_function.authenticate_fido(dev, group_id, aaid, aaid_len, challenge,
                                           challenge_len);
        pthread_mutex_unlock(&g_hal_mutex);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_authenticate
 * Description: Start authentication operation, and wait finger down.
 * Input: dev, operation_id, group_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_authenticate(void *dev, uint64_t operation_id,
                               uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] group_id=%u", __func__, group_id);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.authenticate(dev, operation_id, group_id);
        g_work_state = STATE_AUTHENTICATE;
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_get_auth_id
 * Description: get auth_id from the authentication results.
 * Input: dev
 * Output: None
 * Return: uint64_t
 */
uint64_t gf_hal_get_auth_id(void *dev)
{
    uint64_t ret = 0;
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s]", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        ret = g_hal_function.get_auth_id(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return ret;
}

/**
 * Function: gf_hal_remove
 * Description: Find and remove a finger by finger_id in fingerlist.
 * Input: dev, group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_remove(void *dev, uint32_t group_id, uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.remove(dev, group_id, finger_id);
        g_work_state = STATE_REMOVE;
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_remove_for_sync_list
 * Description: Find and remove a finger by finger_id for sync list.
 * Input: dev, group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_remove_for_sync_list(void *dev, uint32_t group_id,
                                       uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = gf_hal_common_remove_for_sync_list(dev, group_id, finger_id);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_set_active_group
 * Description: activate a user group.
 * Input: dev, group_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_set_active_group(void *dev, uint32_t group_id, const char *store_path)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] group_id=%u", __func__, group_id);

    (void)store_path;

    if (NULL == dev)
    {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    // maybe called gf_hal_set_active_group() before gf_hal_function_init()
    if (NULL == g_hal_function.set_active_group)
    {
        LOG_I(LOG_TAG, "[%s] g_hal_function not inited", __func__);
        err = gf_hal_common_set_active_group(dev, group_id);
    }
    else
    {
        err = g_hal_function.set_active_group(dev, group_id);
    }

    //do_enumerate(dev, group_id, 0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_set_safe_class
 * Description: set safe class for algorithm.
 * Input: dev, safe_class
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_set_safe_class(void *dev, gf_safe_class_t safe_class)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] safe_class=%d", __func__, safe_class);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.set_safe_class(dev, safe_class);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_camera_capture
 * Description: Execute OPERATION_CAMERA_KEY, wait for finger down.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_camera_capture(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.camera_capture(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_stop_camera_capture(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.test_cancel(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_navigate
 * Description: Enter navigation mode, detect user's operation.
 * Input: dev, nav_mode
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_navigate(void *dev, gf_nav_mode_t nav_mode)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] nav_mode=%s, nav_mode_code=%d", __func__,
          gf_strnavmode(nav_mode), nav_mode);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.navigate(dev, nav_mode);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_enable_fingerprint_module
 * Description: Enable or disable fingerprint module.
 * Input: dev, enable_flag
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enable_fingerprint_module(void *dev, uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] enable_flag=%u", __func__, enable_flag);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.enable_fingerprint_module(dev, enable_flag);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_enable_ff_feature
 * Description: Enable or disable fingerflash feature.
 * Input: dev, enable_flag
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enable_ff_feature(void *dev, uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] enable_flag=%u", __func__, enable_flag);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.enable_ff_feature(dev, enable_flag);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_enable_bio_assay_feature
 * Description: Enable or disable "Live body detecting" feature.
 * Input: dev, enable_flag
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enable_bio_assay_feature(void *dev, uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (NULL == dev)
    {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        err = g_hal_function.enable_bio_assay_feature(dev, enable_flag);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_irq
 * Description: handle irq request that from linux kernel.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_irq()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.irq();
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_is_id_valid
 * Description: Return hal status by group id.
 * Input: dev, group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_is_id_valid(void *dev, uint32_t group_id, uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (NULL == dev)
    {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        err = g_hal_function.is_id_valid(dev, group_id, finger_id);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_get_id_list
 * Description: return a id list by group id.
 * Input: dev, group_id, finger_id
 * Output: list, count
 * Return: gf_error_t
 */
gf_error_t gf_hal_get_id_list(void *dev, uint32_t group_id, uint32_t *list,
                              int32_t *count)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (NULL == dev || NULL == list || NULL == count)
    {
        LOG_E(LOG_TAG, "[%s] dev or list or count is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        err = g_hal_function.get_id_list(dev, group_id, list, count);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_enumerate_with_callback
 * Description: enumerate finger list with callback.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enumerate_with_callback(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.enumerate_with_callback(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_enumerate
 * Description: enumerate finger list.
 * Input: dev
 * Output: results, max_size
 * Return: gf_error_t
 */
gf_error_t gf_hal_enumerate(void *dev, void *results, uint32_t *max_size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev || NULL == results || NULL == max_size)
        {
            LOG_E(LOG_TAG, "[%s] dev or results or max_size is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.enumerate(dev, results, max_size);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_screen_on
 * Description: Notify screen status to TA.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_screen_on(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s]", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.screen_on();
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_screen_off
 * Description: Notify screen status to TA.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_screen_off(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s]", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.screen_off();
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_pause_enroll
 * Description: Pause the current enrollment.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_pause_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);

    if(STATE_ENROLL != g_work_state) {
        LOG_E(LOG_TAG, "[%s] state err, g_work_state = %d", __func__, g_work_state);
        return GF_ERROR_SET_MODE_FAILED;
    }

    pthread_mutex_lock(&g_hal_mutex);

    err = g_hal_function.pause_enroll();
    if (GF_SUCCESS == err) {
        g_enroll_state = GF_ENROLL_STATE_PAUSED;
    }

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_continue_enroll
 * Description: Resume the current enrollment.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_continue_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);

    if (GF_ENROLL_STATE_PAUSED != g_enroll_state) {
        LOG_E(LOG_TAG, "[%s] state err, g_enroll_state = %d", __func__, g_enroll_state);
        return GF_ERROR_SET_MODE_FAILED;
    }

    pthread_mutex_lock(&g_hal_mutex);

    err = g_hal_function.resume_enroll();
    if (GF_SUCCESS == err) {
        g_enroll_state = GF_ENROLL_STATE_NORMALL;
    }

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_start_hbd
 * Description: Start detect "heart beat detecting" operation.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_start_hbd(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.start_hbd(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_reset_lockout
 * Description: Reset priority lock.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_reset_lockout(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    err = g_hal_function.reset_lockout();

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_lockout
 * Description: Lock the priority of the current operation.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_lockout(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    err = g_hal_function.lockout();

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_sync_finger_list
 * Description: Synchronize finger list.
 * Input: dev, list, count
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_sync_finger_list(void *dev, uint32_t *list, int32_t count)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t retry_times = 0;
    const uint32_t max_retry_times = 20;
    const uint32_t micro_seconds = 1000;
    FUNC_ENTER();

    while (retry_times < max_retry_times)
    {
        LOG_D(LOG_TAG, "[%s] retry %u times, g_set_active_group_done = %u", __func__,
              retry_times, g_set_active_group_done);

        if (1 == g_set_active_group_done)
        {
            break;
        }

        usleep(micro_seconds);
        retry_times++;
    }

    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev || NULL == list)
        {
            LOG_E(LOG_TAG, "[%s] dev or list is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.sync_finger_list(dev, list, count);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_is_inited
 * Description: Return initialized status.
 * Input: None
 * Output: None
 * Return: uint32_t
 */
uint32_t gf_hal_is_inited(void)
{
    return g_hal_inited;
}


/*for oplus*/

/**
 * Function: gf_hal_wait_touch_down
 * Description: Wait touch down.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_wait_touch_down(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = gf_hal_invoke_command_ex(GF_CMD_USER_SET_KEY_MODE);
        if (GF_SUCCESS == err) {
            g_custom_state = STATE_TOUCH_WORKING;
        }
    } while(0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_set_finger_screen
 * Description: Set finger screen state.
 * Input: screen_state
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_set_finger_screen(int32_t screen_state)
{
    gf_error_t err = GF_SUCCESS;
    UNUSED_VAR(screen_state);
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }
        err = g_hal_function.set_finger_screen(screen_state);
    } while(0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_open_debug
 * Description: Open debug.
 * Input: on
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_open_debug(uint32_t on)
{
    gf_error_t err = GF_SUCCESS;
    UNUSED_VAR(on);
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    //err = g_hal_function.pause_enroll();

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_get_image_quality
 * Description: Get image quality.
 * Input: None
 * Output: None
 * Return: None
 */
void gf_hal_get_image_quality(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        g_custom_state = STATE_TEST_WORKING;
        err = hal_test_performance();
    } while(0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
}

/**
 * Function: gf_hal_module_test
 * Description: Module test.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_module_test(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    do {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = hal_test_spi();
        if (GF_SUCCESS != err) {
            LOG_E(LOG_TAG, "[%s] hal_test_spi error", __func__);
            break;
        }

        err = hal_test_synchronous_pixel_open();
        if (GF_SUCCESS != err) {
            LOG_E(LOG_TAG, "[%s] hal_test_synchronous_pixel_open error", __func__);
            break;
        }

        err = hal_test_sensor_validity();
        if (GF_SUCCESS != err) {
            LOG_E(LOG_TAG, "[%s] hal_test_sensor_validity error", __func__);
            break;
        }

    } while(0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_test_bad_point
 * Description: Test bad point.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_test_bad_point(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    //err = g_hal_function.test_bad_point();

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_set_enroll_state
 * Description: Set enroll state.
 * Input: new_state
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_set_enroll_state(uint8_t new_state)
{
    gf_error_t err = GF_SUCCESS;
    UNUSED_VAR(new_state);
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    //err = g_hal_function.set_enroll_state();

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_check_data_noise
 * Description: Check data noise.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_check_data_noise(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enter", __func__);
    pthread_mutex_lock(&g_hal_mutex);

    //err = g_hal_function.set_enroll_state();

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

int gf_get_enrollment_total_times(void) {
    LOG_I("%s\n", __func__);
    int total_times = fp_config_info_init.total_enroll_times;
    return total_times;
}

typedef struct OplusQrCodeInfo{
    uint32_t token1;
    uint32_t errcode;
    uint32_t token2;
    uint32_t qrcode_length;
    char qrcode[32];
}OplusQrCodeInfo_t;

gf_error_t gf_hal_notify_qrcode(int32_t cmdId)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    OplusQrCodeInfo_t qrcodeinfo;
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    uint8_t qr_code_len = sizeof(qrcodeinfo.qrcode);
    err = gf_hal_common_get_qr_code(qrcodeinfo.qrcode, qr_code_len);
    qrcodeinfo.token1 = 1;
    qrcodeinfo.errcode= 0;
    qrcodeinfo.token2 = qrcodeinfo.token1;
    qrcodeinfo.qrcode_length = (uint32_t)qr_code_len;
    message.type = FINGERPRINT_OPTICAL_SENDCMD;
    message.data.test.cmd_id = cmdId;
    message.data.test.result = (int8_t *) &qrcodeinfo;
    message.data.test.result_len = sizeof(qrcodeinfo);
    LOG_D(LOG_TAG, "[%s] notify cmd, cmdId = %d, result = %s, resultLen = %d", __func__, cmdId, qrcodeinfo.qrcode, qrcodeinfo.qrcode_length);
    if (g_fingerprint_device->notify != NULL)
    {
        g_fingerprint_device->notify(&message);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_detect_sensor_broken
 * Description: detect sensor broken.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_detect_sensor_broken(void)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }
        err = gf_test_sensor_broken();
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);

    return err;
}
