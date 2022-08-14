/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: HAL function initialization for milan chip
 * History: None
 * Version: 1.0
 */
#include <endian.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <cutils/properties.h>


#include "gf_dump_data.h"
#include "gf_hal.h"
#include "gf_hal_device.h"
#include "gf_hal_milan.h"
#include "gf_type_define.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_hal_dump.h"
#include "gf_ca_entry.h"
#include "gf_hal_timer.h"
#include "gf_hal_frr_database.h"
#include "gf_hal_test.h"
#include "gf_hal_milan_test.h"
#include "gf_hal_mem.h"
#include "gf_milan_dump.h"

#define LOG_TAG "[GF_HAL][gf_hal_milan]"

#define PROPERTY_FINGERPRINT_QRCODE "oplus.fingerprint.qrcode.support"

#define PROPERTY_FINGERPRINT_QRCODE_VALUE "oplus.fingerprint.qrcode.value"


timer_t g_key_long_pressed_timer_id = 0;  // id of long pressed timer id

int64_t g_irq_time_milan = 0;  // irq time of milan chip

static  bool g_is_down_detected = false;  // flag of detect finger down
uint8_t g_hal_over_current_count = 0;  // over current count
static uint8_t g_auth_retry_index = 0;  // g_auth_retry_index
static gf_error_t irq_pre_process(gf_irq_t *cmd);
static void irq_process(gf_irq_t *cmd, gf_error_t *err_code);
static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code);
static void irq_nav(gf_irq_t *cmd);
static void irq_key(gf_irq_t *cmd);
static void irq_enroll_or_authenticate(gf_irq_t *cmd, gf_error_t *err_code);
static void irq_down_enroll_or_authenticate(gf_irq_t *cmd,
                                            gf_error_t *err_code);
static void irq_image_enroll_or_authenticate(gf_irq_t *cmd,
                                             gf_error_t *err_code);
static void irq_up_enroll_or_authenticate(gf_irq_t *cmd);
static void hal_milan_authenticate_fido_success(gf_irq_t *cmd, gf_error_t *err_code);
static void hal_milan_authenticate_success(gf_irq_t *cmd, gf_error_t *err_code);
static gf_error_t hal_milan_authenticate_retry(gf_irq_t *cmd, gf_error_t *result);
static void sensor_is_broken();
static void hal_milan_on_nav_down(gf_irq_t *cmd);
static void hal_milan_on_nav_up();

/**
 * Function: hal_key_long_pressed_timer_thread
 * Description: Key long pressed timer thread
 * Input: None
 * Output: None
 * Return: void
 */
static void hal_key_long_pressed_timer_thread(union sigval v)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(v);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (g_key_down_flag == 0)
        {
            break;
        }

        gf_hal_invoke_command_ex(GF_CMD_CHECK_FINGER_LONG_PRESS);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_init
 * Description: Do initialization for milan chip.
 * Input: dev
 * Output: dev
 * Return: Error code
 */
static gf_error_t hal_milan_init(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_init_t *cmd = NULL;
    uint32_t size = sizeof(gf_init_t);
    gf_ioc_chip_info_t info = { 0 };
    uint8_t download_fw_flag = 0;
    uint8_t hal_otp_buf[GF_SENSOR_OTP_BUFFER_LEN] = { 0 };
    uint8_t goodix_qr_code[20] = {0};
    uint8_t qr_code_len = sizeof(goodix_qr_code);
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        err = gf_hal_get_fw_info(&download_fw_flag);

        if (err != GF_SUCCESS)
        {
            break;
        }

        cmd = (gf_init_t *) GF_MEM_MALLOC(size);  // NOLINT(542)

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_common_load_otp_info_from_sdcard(cmd->otp_info,
                                                      &(cmd->otp_info_len));

        if (err == GF_SUCCESS)
        {
            memcpy(hal_otp_buf, cmd->otp_info, cmd->otp_info_len);
        }

        cmd->download_fw_flag = download_fw_flag;
        err = gf_hal_invoke_command(GF_CMD_INIT, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        if (memcmp(cmd->otp_info, hal_otp_buf, cmd->otp_info_len) != 0)  // NOLINT(705)
        {
            LOG_I(LOG_TAG, "[%s] update new otp data to HAL storage", __func__);
            gf_hal_common_save_otp_info_into_sdcard(cmd->otp_info, cmd->otp_info_len);
        }

        g_sensor_row = cmd->row;
        g_sensor_col = cmd->col;
        g_sensor_nav_row = cmd->nav_row;
        g_sensor_nav_col = cmd->nav_col;
        // milan HV chip_type maybe change in GF_CMD_INT according to product id
        if (cmd->chip_type != 0)
        {
            g_hal_config.chip_type = cmd->chip_type;
        }
        if (cmd->chip_series != 0)
        {
            g_hal_config.chip_series = cmd->chip_series;
        }
        gf_dump_init(g_sensor_row, g_sensor_col, g_sensor_nav_row, g_sensor_nav_col,
                     g_hal_config.chip_type, g_hal_config.chip_series);
        gf_hal_judge_delete_frr_database(g_hal_config.chip_type,
                                         g_hal_config.chip_series);
        LOG_D(LOG_TAG, "[%s] init finished , save otp_info", __func__);
        err = gf_hal_device_enable();

        if (err != GF_SUCCESS)
        {
            break;
        }

        if (SUPPORT_IMAGE_EE_CONDITION(g_hal_config))
        {
            g_sensor_row = cmd->row_ee;
            g_sensor_col = cmd->col_ee;
        }

        info.vendor_id = cmd->vendor_id[0];
        info.mode = g_mode;
        info.operation = g_operation;
        err = gf_hal_chip_info(info);

        err = gf_hal_common_get_qr_code(goodix_qr_code, qr_code_len);
        LOG_D(LOG_TAG, "[%s] @@@@@ mQRCode=%s,len=%d", __func__, goodix_qr_code, strlen((const char *)goodix_qr_code));
        property_set(PROPERTY_FINGERPRINT_QRCODE_VALUE, (const char *)goodix_qr_code);
        if(qr_code_len != 0)
        property_set(PROPERTY_FINGERPRINT_QRCODE, "1");

        if (err != GF_SUCCESS)
        {
            break;
        }

        err = gf_hal_init_finished();

        if (err != GF_SUCCESS)
        {
            break;
        }
    }  // do...
    while (0);

    gf_hal_dump_data_by_operation(OPERATION_TEST_BOOT_CALIBRATION, err);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_close
 * Description: Destroy timer and other common resources.
 * Input: Device object
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_close(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
    err = gf_hal_common_close(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_cancel
 * Description: Send command to cancel the current operation
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        if (true == g_is_down_detected) {
            gf_hal_notify_msg_info(FINGERPRINT_TOUCH_UP);
            g_is_down_detected = false;
            LOG_E(LOG_TAG, "[%s] set g_is_down_detected=%d", __func__, g_is_down_detected);
        }
        gf_hal_nav_reset();

        gf_hal_common_cancel(dev);
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
        cmd = (gf_cancel_t *) GF_MEM_MALLOC(size);  // NOLINT(542)

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_CANCEL, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (g_hal_config.support_detect_sensor_temperature)
    {
        gf_hal_post_sem_detect_broken();
    }

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_test_cancel
 * Description: Test cancel operation
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_test_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_nav_reset();

        gf_hal_destroy_timer(&g_irq_timer_id);
        g_test_interrupt_pin_flag = 0;
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) GF_MEM_MALLOC(size);  // NOLINT(542)

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_CANCEL, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_test_prior_cancel
 * Description: Test prior cancel operation
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_test_prior_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) GF_MEM_MALLOC(size);  // NOLINT(542)

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_TEST_PRIOR_CANCEL, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_pre_enroll
 * Description: Pre-enroll
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static uint64_t hal_milan_pre_enroll(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_pre_enroll(dev);
    VOID_FUNC_EXIT();
    return ret;
}

/*
 * Function: hal_milan_enroll
 * Description: Enroll
 * Input: Device object, hat, group id, timeout
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_enroll(void *dev, const void *hat,
                                            uint32_t group_id,
                                            uint32_t timeout_sec)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
    err = gf_hal_common_enroll(dev, hat, group_id, timeout_sec);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_post_enroll
 * Description: Post enroll
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_post_enroll(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_post_enroll(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_authenticate
 * Description: Authenticate
 * Input: Device object, operation id, group id
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_authenticate(void *dev,
                                                  uint64_t operation_id,
                                                  uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
    err = gf_hal_common_authenticate(dev, operation_id, group_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_get_auth_id
 * Description: Get authenticate id
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static uint64_t hal_milan_get_auth_id(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_get_auth_id(dev);
    VOID_FUNC_EXIT();
    return ret;
}

/*
 * Function: hal_milan_remove
 * Description: Remove finger template
 * Input: Device object, group id, finger id
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_remove(void *dev, uint32_t group_id,
                                            uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_remove(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_set_active_group
 * Description: Set active group
 * Input: Device object, group id
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_set_active_group(void *dev,
                                                      uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_active_group(dev, group_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_enumerate
 * Description: Fetch the finger list registered
 * Input: Device object, max size
 * Output: The finger list and finger count
 * Return: Error code
 */
static gf_error_t hal_milan_enumerate(void *dev, void *results,
                                               uint32_t *max_size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enumerate(dev, results, max_size);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_enumerate_with_callback
 * Description: Fetch the finger list registered, return the result by callback
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_enumerate_with_callback(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enumerate_with_callback(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_on_nav_up
 * Description: Send nav-up event
 * Input: None
 * Output: None
 * Return: void
 */
static void hal_milan_on_nav_up()
{
    VOID_FUNC_ENTER();
    gf_hal_send_nav_event(GF_NAV_FINGER_UP);
    g_key_down_flag = 0;

    if (g_nav_click_status == GF_NAV_CLICK_STATUS_DOWN)
    {
        if (g_nav_double_click_timer_id != 0)
        {
            LOG_D(LOG_TAG, "[%s] set to down_up", __func__);
            g_nav_click_status = GF_NAV_CLICK_STATUS_DOWN_UP;
        }
        else
        {
            LOG_D(LOG_TAG, "[%s] get a click", __func__);
            gf_hal_send_nav_event(GF_NAV_CLICK);
            gf_hal_nav_reset();
        }
    }

    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_detect_nav_event
 * Description: Detect nav event
 * Input: IRQ command
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_detect_nav_event(gf_irq_t *cmd)
{
    gf_error_t err = GF_SUCCESS;
    gf_irq_t *detect_nav_event = cmd;
    FUNC_ENTER();

    do
    {
        if (NULL == detect_nav_event)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        if (GF_NAV_NONE != detect_nav_event->nav_result.nav_code
            && GF_NAV_CLICK != detect_nav_event->nav_result.nav_code
            && GF_NAV_HEAVY != detect_nav_event->nav_result.nav_code
            && GF_NAV_CLICK_STATUS_NONE != g_nav_click_status)
        {
            gf_hal_nav_listener(detect_nav_event->nav_result.nav_code);
        }

        if (GF_NAV_NONE == detect_nav_event->nav_result.nav_code)
        {
            gf_hal_nav_reset();
        }

        if (detect_nav_event->nav_result.finger_up_flag > 0)
        {
            hal_milan_on_nav_up();
        }
    }  // do...
    while (0);

    gf_hal_dump_data_by_operation(OPERATION_NAV, err);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_on_nav_down
 * Description: Send finger down event
 * Input: IRQ command
 * Output: None
 * Return: void
 */
static void hal_milan_on_nav_down(gf_irq_t *cmd)
{
    int32_t time_delta = 0;
    int32_t nav_double_click_interval = 0;
    int32_t nav_long_press_interval = 0;
    g_key_down_flag = 1;
    g_nav_frame_index = 0;
    g_nav_times++;
    VOID_FUNC_ENTER();
    gf_hal_send_nav_event(GF_NAV_FINGER_DOWN);

    if (g_hal_config.nav_double_click_interval_in_ms > 0
        || g_hal_config.nav_long_press_interval_in_ms > 0)
    {
        gf_hal_nav_assert_config_interval();

        switch (g_nav_click_status)
        {
            case GF_NAV_CLICK_STATUS_NONE:
            {
                LOG_D(LOG_TAG, "[%s] get the first down", __func__);
                time_delta = (int32_t)(llabs(gf_hal_current_time_microsecond() -
                                           g_irq_time_milan) / 1000);

                if (g_hal_config.nav_double_click_interval_in_ms > 0)
                {
                    nav_double_click_interval = g_hal_config.nav_double_click_interval_in_ms -
                                                time_delta;
                    nav_double_click_interval = (nav_double_click_interval > 0) ?
                                                nav_double_click_interval : 1;
                    gf_hal_create_timer(&g_nav_double_click_timer_id,
                                        gf_hal_nav_double_click_timer_thread);
                    gf_hal_set_timer(&g_nav_double_click_timer_id, 0,
                                     nav_double_click_interval / 1000,
                                     1000 * 1000 * (nav_double_click_interval % 1000));
                }

                if (g_hal_config.nav_long_press_interval_in_ms > 0)
                {
                    nav_long_press_interval = g_hal_config.nav_long_press_interval_in_ms -
                                              time_delta;
                    nav_long_press_interval = (nav_long_press_interval > 0) ?
                                              nav_long_press_interval : 1;
                    gf_hal_create_timer(&g_nav_long_press_timer_id,
                                        gf_hal_nav_long_press_timer_thread);
                    gf_hal_set_timer(&g_nav_long_press_timer_id, 0,
                                     nav_long_press_interval / 1000,
                                     1000 * 1000 * (nav_long_press_interval % 1000));
                }

                g_nav_click_status = GF_NAV_CLICK_STATUS_DOWN;

                if (GF_NAV_EVENT_DETECT_METHOD_POLLING ==
                    g_hal_config.support_nav_get_data_method)
                {
                    hal_milan_detect_nav_event(cmd);
                }

                break;
            }  // case GF_NAV_CLICK_STATUS_NONE

            case GF_NAV_CLICK_STATUS_DOWN_UP:
            {
                if (g_nav_double_click_timer_id != 0)
                {
                    LOG_D(LOG_TAG, "[%s] get a double click", __func__);
                    gf_hal_nav_reset();
                    gf_hal_send_nav_event(GF_NAV_DOUBLE_CLICK);
                    if (GF_NAV_EVENT_DETECT_METHOD_POLLING ==
                        g_hal_config.support_nav_get_data_method)
                    {
                        gf_hal_nav_complete();
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }  // switch (g_nav_click_status)
    }  // if...
    else
    {
        g_nav_click_status = GF_NAV_CLICK_STATUS_DOWN;

        if (GF_NAV_EVENT_DETECT_METHOD_POLLING ==
            g_hal_config.support_nav_get_data_method)
        {
            hal_milan_detect_nav_event(cmd);
        }
    }

    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_authenticate_finish
 * Description: Send authenticate finish command
 * Input: IRQ command
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_authenticate_finish(gf_irq_t *buffer)
{
    gf_error_t err = GF_SUCCESS;
    gf_authenticate_finish_t *cmd = NULL;
    uint32_t size = sizeof(gf_authenticate_finish_t);

    FUNC_ENTER();

    do
    {
        cmd = (gf_authenticate_finish_t *) GF_MEM_MALLOC(size);  // NOLINT(542)

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->group_id = buffer->group_id;
        cmd->finger_id = buffer->finger_id;
        cmd->operation = buffer->operation;
        cmd->irq_type = buffer->irq_type;
        err = gf_hal_invoke_command(GF_CMD_AUTHENTICATE_FINISH, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);

    return err;
}

/*
 * Function: hal_milan_authenticate_success
 * Description: Authenticate success handling
 * Input: IRQ command and error code
 * Output: None
 * Return: Error code
 */
static void hal_milan_authenticate_success(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    fingerprint_auth_dcsmsg_t auth_context;
    memset(&auth_context, 0, sizeof(auth_context));
    auth_context.fail_reason = GF_FINGERPRINT_ACQUIRED_GOOD;

    do
    {
        if (NULL == cmd || NULL == err_code)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);

        gf_hal_notify_authentication_succeeded(cmd->group_id, cmd->finger_id,
                                               &cmd->auth_token);
        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
              cmd->finger_id);
        LOG_D(LOG_TAG, "[%s] save_flag=%u", __func__, cmd->save_flag);
        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
        *err_code = hal_milan_authenticate_finish(cmd);

        auth_context.auth_result = 1;//1 means success
        //auth_context.fail_reason = cmd->o_dismatch_reason;
        auth_context.quality_score = cmd->dump_performance.image_quality;
        auth_context.match_score = cmd->dump_performance.match_score;
        //auth_context.signal_value = cmd->o_sig_val;
        auth_context.img_area = cmd->dump_performance.valid_area;
        auth_context.retry_times = g_auth_retry_times;//context->retry;
        //auth_context.module_type = mContext->mSensor->mModuleType;
        //auth_context.lense_type = mContext->mSensor->mLenseType;
        //auth_context.dsp_availalbe = mDSPAvailable;
        gf_hal_notify_send_auth_dcsmsg(auth_context);

        if (g_hal_config.support_authenticate_ratio > 0)
        {
            gf_hal_save_auth_ratio_record(cmd);
        }

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(*err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }

        g_failed_attempts = 0;
    }  // end do
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_authenticate_fido_success
 * Description: Authenticate fido success handling
 * Input: IRQ command and error code
 * Output: None
 * Return: void
 */
static void hal_milan_authenticate_fido_success(gf_irq_t *cmd,
                                                gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == cmd || NULL == err_code)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);

        if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
        {
            gf_fingerprint_msg_t message = { 0 };
            message.type = GF_FINGERPRINT_AUTHENTICATED_FIDO;
            message.data.authenticated_fido.finger_id = cmd->finger_id;
            message.data.authenticated_fido.uvt_len = cmd->uvt.uvt_len;
            memcpy(&message.data.authenticated_fido.uvt_data, &cmd->uvt.uvt_buf,
                   sizeof(message.data.authenticated_fido.uvt_data));
            g_fingerprint_device->test_notify(&message);
        }

        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
              cmd->finger_id);
        LOG_D(LOG_TAG, "[%s] save_flag=%u", __func__, cmd->save_flag);
        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);

        *err_code = hal_milan_authenticate_finish(cmd);

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(*err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }

        g_failed_attempts = 0;
    }  // do...
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_down_enroll_or_authenticate
 * Description: Handle finger down event, send command to start enroll or authenticate
 * Input: IRQ command and error code
 * Output: error code
 * Return: void
 */
static void irq_down_enroll_or_authenticate(gf_irq_t *cmd,
                                            gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (0 == g_hal_config.report_key_event_only_enroll_authenticate
            || OPERATION_ENROLL == cmd->operation
            || OPERATION_AUTHENTICATE_FF == cmd->operation
            || OPERATION_AUTHENTICATE_IMAGE == cmd->operation
            || OPERATION_AUTHENTICATE_FIDO == cmd->operation
            || OPERATION_USER_KEY == cmd->operation)
        {
            gf_hal_notify_msg_info(FINGERPRINT_TOUCH_DOWN);

        gf_bind_bigcore_bytid();
#ifdef FP_HYPNUSD_ENABLE
            gf_set_hypnus(ACTION_TYPE, ACTION_TIMEOUT_500);
#endif
        }

        if (OPERATION_AUTHENTICATE_FF == cmd->operation)
        {
#if defined(__ANDROID_O) || defined(__ANDROID_P)
            //gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_DETECTED);
#endif  // _ANDROID_O
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
            /*fall-through*/
        }

        if (OPERATION_ENROLL == cmd->operation)
        {
            gf_hal_create_timer(&g_long_pressed_timer_id, gf_hal_long_pressed_timer_thread);
            gf_hal_set_timer(&g_long_pressed_timer_id, 2, 2, 0);
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: sensor_is_broken
 * Description: Sensor broken handling
 * Input: None
 * Output: None
 * Return: void
 */
static void sensor_is_broken()
{
    VOID_FUNC_ENTER();

    do
    {
        g_enroll_invalid_template_num++;
        LOG_D(LOG_TAG, "[%s] broken checked, invalid template count=%u", __func__,
              g_enroll_invalid_template_num);

        if (g_enroll_invalid_template_num >= (g_hal_config.enrolling_min_templates >>
                                              1))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] broken checked, discard this enrolling", __func__);
            g_enroll_invalid_template_num = 0;
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_HW_UNAVAILABLE);
            hal_milan_cancel(g_fingerprint_device);
        }
        else
        {
            LOG_D(LOG_TAG, "[%s] broken checked, skip this invalid template", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_PARTIAL);
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_authenticate_retry
 * Description: Retry authenticate operation
 * Input: IRQ command and error code
 * Output: Command invoke result
 * Return: Error code
 */
static gf_error_t hal_milan_authenticate_retry(gf_irq_t *cmd, gf_error_t *result)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t size = sizeof(gf_irq_t);

    FUNC_ENTER();

    do
    {
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, cmd is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if ((cmd->irq_type & (GF_IRQ_IMAGE_MASK | GF_IRQ_NAV_MASK)) != 0)
        {
            if ((OPERATION_AUTHENTICATE_FF == cmd->operation ||
                OPERATION_AUTHENTICATE_SLEEP == cmd->operation ||
                OPERATION_AUTHENTICATE_IMAGE == cmd->operation ||
                OPERATION_AUTHENTICATE_FIDO == cmd->operation) &&
                g_hal_config.support_authenticate_once_dump)
            {
                // save time
                gf_hal_dump_save_cur_time(g_auth_retry_index);
                g_auth_retry_index++;
            }
            else
            {
                gf_hal_dump_data_by_operation(cmd->operation, *result);
            }
        }

        LOG_D(LOG_TAG, "[%s] start polling", __func__);
        cmd->step = GF_IRQ_STEP_POLLING;
        *result = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK", __func__);
            irq_image_enroll_or_authenticate(cmd, result);
        }
        else
        {
            LOG_D(LOG_TAG, "[%s] polling image fail", __func__);
        }
    }
    while (0);

    FUNC_EXIT(err);

    return err;
}

/*
 * Function: irq_image_enroll_or_authenticate
 * Description: Handle enroll or authenticate result
 * Input: IRQ command and error code
 * Output: Error code
 * Return: void
 */
static void irq_image_enroll_or_authenticate(gf_irq_t *cmd,
                                             gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    switch (*err_code)
    {
        case GF_SUCCESS:
        {
            if (OPERATION_ENROLL == cmd->operation)
            {
                gf_hal_common_enroll_success(cmd, err_code);
            }
            else if (OPERATION_AUTHENTICATE_FF == cmd->operation
                     || OPERATION_AUTHENTICATE_IMAGE == cmd->operation)
            {
                hal_milan_authenticate_success(cmd, err_code);
            }
            else if (OPERATION_AUTHENTICATE_FIDO == cmd->operation)
            {
                hal_milan_authenticate_fido_success(cmd, err_code);
            }

            break;
        }

        case GF_ERROR_SENSOR_IS_BROKEN:
        {
            sensor_is_broken();
            break;
        }

        case GF_ERROR_ALGO_DIRTY_FINGER:  // fall through
        case GF_ERROR_ALGO_COVER_BROKEN:
        case GF_ERROR_ACQUIRED_IMAGER_DIRTY:
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK image quality is too low", __func__);
            if (OPERATION_TEST_UNTRUSTED_ENROLL == cmd->operation
                || OPERATION_TEST_UNTRUSTED_AUTHENTICATE == cmd->operation
                || OPERATION_ENROLL == cmd->operation) {
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
               // gf_hal_notify_enrollment_progress(cmd->group_id, cmd->finger_id, cmd->samples_remaining);
            } else {
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_image_info(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                gf_hal_notify_authentication_failed();
            }
            break;
        }

        case GF_ERROR_SENSOR_BROKEN_CHECK_ALGO_ERROR:
        case GF_ERROR_ACQUIRED_PARTIAL:
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK valid area is too low", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_PARTIAL);
            break;
        }

        case GF_ERROR_INVALID_FINGER_PRESS:
        case GF_ERROR_INVALID_BASE:
        {
            LOG_I(LOG_TAG, "[%s] bad base or temperature rise, err=%s, errno=%d", __func__,
                  gf_strerror(*err_code), *err_code);
            break;
        }

        case GF_ERROR_DUPLICATE_FINGER:
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_ALREAD_ENROLLED);
            LOG_D(LOG_TAG, "[%s] duplicate finger id=%u", __func__,
                  cmd->duplicate_finger_id);
            if (0 == g_hal_config.support_swipe_enroll || OPERATION_ENROLL != cmd->operation)
            {
                gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            }
            break;
        }

        case GF_ERROR_DUPLICATE_AREA:
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_SIMLAR);
            LOG_D(LOG_TAG, "[%s] duplicate area", __func__);
            if (0 == g_hal_config.support_swipe_enroll || OPERATION_ENROLL != cmd->operation)
            {
                gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            }
            break;
        }

        case GF_ERROR_MATCH_FAIL_AND_RETRY:
        {
            LOG_I(LOG_TAG, "[%s] match fail and retry", __func__);
            break;
        }

        case GF_ERROR_TOO_FAST:
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
            LOG_D(LOG_TAG, "[%s] finger lift too fast", __func__);
            break;
        }

        case GF_ERROR_NOT_MATCH:
        case GF_ERROR_ALGO_FAKE:
        {
            gf_hal_common_authenticate_not_match(cmd, *err_code);
            break;
        }

        case GF_ERROR_SPI_RAW_DATA_CRC_FAILED:
        {
            LOG_I(LOG_TAG, "[%s] crc check error, err=%s, errno=%d", __func__,
                  gf_strerror(*err_code), *err_code);
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_SPI_COMMUNICATION);
            break;
        }

        case GF_ERROR_DYNAMIC_ENROLL_INVALID_PRESS_TOO_MUCH:
        {
            LOG_D(LOG_TAG, "[%s] invalid press too much", __func__);
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_INVALID_PRESS_TOO_MUCH);
            break;
        }

        case GF_ERROR_DYNAMIC_ERNOLL_INCOMPLETE_TEMPLATE:
        {
            LOG_D(LOG_TAG, "[%s] incomplete template", __func__);
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_INCOMPLETE_TEMPLATE);
            break;
        }

        case GF_ERROR_TOO_SLOW:
        {
            LOG_D(LOG_TAG, "[%s] swipe too slow", __func__);
            if (OPERATION_ENROLL == cmd->operation)
            {
                gf_hal_notify_enrollment_progress(cmd->group_id, cmd->finger_id,
                                    cmd->samples_remaining);
            }
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_SLOW);
            break;
        }

        case GF_ERROR_INVALID_DATA:
        {
            if (g_hal_config.support_swipe_enroll > 0 && OPERATION_ENROLL == cmd->operation)
            {
                sensor_is_broken();
            }
            break;
        }

        case GF_ERROR_ALGO_PALM_DETECT:
        {
            LOG_D(LOG_TAG, "[%s] palm detect", __func__);
            break;
        }

        default:
        {
            LOG_I(LOG_TAG, "[%s] won't handle this error code, err=%s, errno=%d", __func__,
                  gf_strerror(*err_code), *err_code);
            break;
        }
    }  // switch (*err_code)

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_up_enroll_or_authenticate
 * Description: Handle finger up event when enroll or authenticate
 * Input: IRQ command
 * Output: None
 * Return: void
 */
static void irq_up_enroll_or_authenticate(gf_irq_t *cmd)
{
    VOID_FUNC_ENTER();

    do
    {
        if (0 == g_hal_config.report_key_event_only_enroll_authenticate
            || OPERATION_ENROLL == cmd->operation
            || OPERATION_AUTHENTICATE_FF == cmd->operation
            || OPERATION_AUTHENTICATE_IMAGE == cmd->operation
            || OPERATION_AUTHENTICATE_FIDO == cmd->operation
            || OPERATION_USER_KEY == cmd->operation)
        {
            gf_hal_notify_msg_info(FINGERPRINT_TOUCH_UP);
        }

        if (OPERATION_ENROLL == cmd->operation)
        {
            gf_hal_destroy_timer(&g_long_pressed_timer_id);
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_enroll_or_authenticate
 * Description: Enroll or authenticate entry
 * Input: IRQ command and error code
 * Output: Error code
 * Return: void
 */
static void irq_enroll_or_authenticate(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))  // NOLINT(705)
        {
            g_auth_retry_times = 0;
            irq_down_enroll_or_authenticate(cmd, err_code);
        }

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))  // NOLINT(705)
        {
            irq_image_enroll_or_authenticate(cmd, err_code);

            while (GF_ERROR_MATCH_FAIL_AND_RETRY == *err_code)
            {
                hal_milan_authenticate_retry(cmd, err_code);
            }
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))  // NOLINT(705)
        {
            irq_up_enroll_or_authenticate(cmd);
            if (0 == g_hal_config.support_swipe_enroll
                || OPERATION_ENROLL != cmd->operation)
            {
                g_is_only_dump_broken_check = 1;
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_nav
 * Description: IRQ nav event handling
 * Input: IRQ command
 * Output: None
 * Return: void
 */
static void irq_nav(gf_irq_t *cmd)
{
    VOID_FUNC_ENTER();

    do
    {
        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            pthread_mutex_lock(&g_nav_click_status_mutex);
            hal_milan_on_nav_down(cmd);
            pthread_mutex_unlock(&g_nav_click_status_mutex);
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))  // NOLINT(705)
        {
            if (g_key_down_flag > 0)
            {
                pthread_mutex_lock(&g_nav_click_status_mutex);
                hal_milan_on_nav_up();
                pthread_mutex_unlock(&g_nav_click_status_mutex);
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_key
 * Description: IRQ key event handling
 * Input: IRQ command
 * Output: None
 * Return: void
 */
static void irq_key(gf_irq_t *cmd)
{
    VOID_FUNC_ENTER();

    do
    {
        gf_hal_common_irq_key(cmd);

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            if (g_key_down_flag > 0)
            {
                gf_hal_create_timer(&g_key_long_pressed_timer_id,
                                    hal_key_long_pressed_timer_thread);
                gf_hal_set_timer(&g_key_long_pressed_timer_id, 5, 5, 0);
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}
/*
 * Function: irq_process
 * Description: Process IRQ command
 * Input: IRQ command
 * Output: Pre process result
 * Return: void
 */
static void irq_process(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(err_code);

    do
    {
        if (0 == cmd->irq_type)
        {
            LOG_I(LOG_TAG, "[%s] invalid irq type 0", __func__);
            break;
        }

        switch (cmd->operation)
        {
            case OPERATION_HOME_KEY:
            case OPERATION_CAMERA_KEY:
            case OPERATION_POWER_KEY:
            {
                irq_key(cmd);
                break;
            }

            case OPERATION_CHECK_FINGER_LONG_PRESS:
            {
                gf_hal_common_irq_finger_long_press(cmd, *err_code);
                break;
            }

            case OPERATION_ENROLL:
            case OPERATION_AUTHENTICATE_IMAGE:
            case OPERATION_AUTHENTICATE_FF:
            case OPERATION_AUTHENTICATE_FIDO:
            case OPERATION_USER_KEY:
            {
                irq_enroll_or_authenticate(cmd, err_code);
                break;
            }

            case OPERATION_NAV:
            {
                irq_nav(cmd);
                break;
            }

            case OPERATION_FINGER_BASE:
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_FINGER_BASE", __func__);
                break;
            }

            default:
            {
                gf_hal_milan_irq_test(cmd, err_code);
                break;
            }
        }  // switch (cmd->operation)
    }  // do...
    while (0);

    VOID_FUNC_EXIT();
}
/*
 * Function: irq_pre_process
 * Description: Pre-process IRQ command
 * Input: IRQ command
 * Output: None
 * Return: void
 */
static gf_error_t irq_pre_process(gf_irq_t *cmd)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t size = sizeof(gf_irq_t);
    FUNC_ENTER();

    do
    {
        g_irq_time_milan = gf_hal_current_time_microsecond();
        g_is_only_dump_broken_check = 0;
        cmd->auth_token.version = GF_HW_AUTH_TOKEN_VERSION;
        cmd->auth_token.authenticator_type = htobe32(GF_HW_AUTH_FINGERPRINT);
        cmd->auth_token.timestamp = 0;
        cmd->uvt.uvt_len = MAX_UVT_LEN;
        cmd->step = GF_IRQ_STEP_GET_IRQ_TYPE;
        err = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);

        if (GF_SUCCESS != err)
        {
            g_spi_speed = GF_SPI_LOW_SPEED;
            cmd->step = GF_IRQ_STEP_IDLE;
            // no break here, fall through handle error code by irq_type & operation
        }
        else
        {
            g_spi_speed = cmd->speed;
        }

        /* handle invalid irq type */
        LOG_I(LOG_TAG, "[%s] irq_type=%s, irq_type=0x%04X", __func__,
              gf_strirq(cmd->irq_type), cmd->irq_type);

        if (0 == cmd->irq_type)
        {
            break;
        }
        LOG_D(LOG_TAG, "[%s] g_is_down_detected=%d", __func__, g_is_down_detected);
        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK) && !g_is_down_detected
            && (cmd->operation !=  OPERATION_HOME_KEY))
        {
            LOG_D(LOG_TAG, "[%s] detect finger up first but no finger down", __func__);
            cmd->irq_type = 0;
            break;
        }

        if (GF_IRQ_STEP_GET_IMAGE == cmd->step)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_STEP_GET_IMAGE", __func__);
            err = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);

            if (GF_SUCCESS != err)
            {
                g_spi_speed = GF_SPI_LOW_SPEED;
                cmd->step = GF_IRQ_STEP_IDLE;
                // no break here, fall through handle error code by irq_type & operation
            }
            else
            {
                g_spi_speed = cmd->speed;
            }
        }

        if (GF_IRQ_STEP_POST_GET_IMAGE == cmd->step)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_STEP_POST_GET_IMAGE", __func__);
            err = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);

            if (GF_SUCCESS != err)
            {
                g_spi_speed = GF_SPI_LOW_SPEED;
                cmd->step = GF_IRQ_STEP_IDLE;
                // no break here, fall through handle error code by irq_type & operation
            }
            else
            {
                g_spi_speed = cmd->speed;
            }
        }

        if (0 != (cmd->irq_type & GF_IRQ_FDT_REVERSE_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FDT_REVERSE_MASK", __func__);
            /**
             * g_hal_config.support_nav_mode > GF_NAV_MODE_NONE: Navigation feature is enable through chip configuration.
             * OPERATION_NAV == cmd->operation: Enable navigation feature by application.
             */
            LOG_I(LOG_TAG, "[%s] received FDT irq, so reset dump_base_frame_flag",
                  __func__);
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_DOWN_MASK", __func__);
            g_down_irq_time = g_irq_time_milan;
            g_is_down_detected = true;
            LOG_D(LOG_TAG, "[%s] set g_is_down_detected=%d", __func__, g_is_down_detected);
            if (GF_IRQ_STEP_POLLING == cmd->step)
            {
                // snr start in test file
                if (OPERATION_TEST_STABLE_FACTOR == cmd->operation)
                {
                    LOG_D(LOG_TAG, "[%s] do not polling here", __func__);
                }
                else
                {
                    irq_process(cmd, &err);
                    LOG_D(LOG_TAG, "[%s] start polling", __func__);
                    if (g_hal_config.support_swipe_enroll > 0 && OPERATION_ENROLL == cmd->operation)
                    {
                        while (GF_IRQ_STEP_POLLING == cmd->step)
                        {
                            err = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);
                            // avoid call irq_image_enroll_or_authenticate twice
                            if (GF_IRQ_STEP_POLLING == cmd->step)
                            {
                                irq_image_enroll_or_authenticate(cmd, &err);
                            }
                        }
                    }
                    else
                    {
                        err = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);
                    }
                }  // end else
            }  // end if
        }  // end if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))

        if (0 != (cmd->irq_type & GF_IRQ_RESET_FIRST_MASK))
        {
            LOG_E(LOG_TAG, "[%s] receive reset irq first time, need to reset chip", __func__);
            gf_hal_reset_chip();
        }

        if (0 != (cmd->irq_type & GF_IRQ_FARGO_ERR_MASK))
        {
            LOG_E(LOG_TAG, "[%s] receive fargo error irq, need to reset chip", __func__);
            gf_hal_reset_chip();
        }

        if (0 != (cmd->irq_type & GF_IRQ_FARGO_ERR_NOT_RESOLVED_MASK))
        {
            LOG_E(LOG_TAG, "[%s] fargo error can not resolved,chip power off.", __func__);
            gf_hal_disable_power();
        }

        if (0 != (cmd->irq_type & GF_IRQ_RESET_FAILED_MASK))
        {
            LOG_E(LOG_TAG, "[%s] chip reset err & power off.", __func__);
            gf_hal_disable_power();
        }

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK", __func__);
            g_image_irq_time = g_irq_time_milan;
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_UP_MASK", __func__);
            g_is_down_detected = false;
            LOG_D(LOG_TAG, "[%s] set g_is_down_detected=%d", __func__, g_is_down_detected);
            gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
        }
        if (0 != (cmd->irq_type & GF_IRQ_RESET_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_RESET_MASK", __func__);
        }

        if (0 != (cmd->irq_type & GF_IRQ_ESD_IRQ_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_ESD_IRQ_MASK", __func__);
            gf_hal_reset_chip();
        }

        if (0 != (cmd->irq_type & GF_IRQ_MENUKEY_DOWN_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_MENUKEY_DOWN_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_MENU, GF_KEY_STATUS_DOWN);
        }
        else if (0 != (cmd->irq_type & GF_IRQ_MENUKEY_UP_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_MENUKEY_UP_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_MENU, GF_KEY_STATUS_UP);
        }

        if (0 != (cmd->irq_type & GF_IRQ_BACKKEY_DOWN_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_BACKKEY_DOWN_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_BACK, GF_KEY_STATUS_DOWN);
        }
        else if (0 != (cmd->irq_type & GF_IRQ_BACKKEY_UP_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_BACKKEY_UP_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_BACK, GF_KEY_STATUS_UP);
        }

        if (0 != (cmd->irq_type & GF_IRQ_PRESS_HEAVY_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_PRESS_HEAVY_MASK", __func__);
        }
        else if (0 != (cmd->irq_type & GF_IRQ_PRESS_LIGHT_MASK))  // NOLINT(705)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_PRESS_LIGHT_MASK", __func__);
        }
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: irq_post_process
 * Description: Post process IRQ command
 * Input: IRQ command and error code
 * Output: None
 * Return: void
 */
static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL != cmd)
        {
            uint32_t is_image_or_nav = cmd->irq_type & (GF_IRQ_IMAGE_MASK | GF_IRQ_NAV_MASK);

            if (OPERATION_TEST_STABLE_FACTOR == cmd->operation)
            {
                // do not dump here
                break;
            }

            if (is_image_or_nav || g_is_only_dump_broken_check)
            {
                if (OPERATION_TEST_BAD_POINT == cmd->operation)
                {
                    if (cmd->test_bad_point.algo_processed_flag > 0)
                    {
                        LOG_D(LOG_TAG, "dump bad point test data ");
                        gf_hal_dump_data_by_operation(cmd->operation, error_code);
                    }
                }
                else
                {
                    LOG_D(LOG_TAG, "dump the authenticate and enroll data ");
                    gf_hal_dump_data_by_operation(cmd->operation, error_code);
                    g_auth_retry_index = 0;
                }
                g_is_only_dump_broken_check = 0;
            }

            if (((OPERATION_AUTHENTICATE_IMAGE == cmd->operation)
                 || (OPERATION_AUTHENTICATE_FF == cmd->operation))
                 && (cmd->save_flag > 0))
            {
                LOG_D(LOG_TAG, "templates update dump the new template");
                gf_hal_dump_template(cmd->group_id, cmd->finger_id);
            }

            if ((OPERATION_ENROLL == cmd->operation)
                && (cmd->samples_remaining == 0)
                && (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK)))
            {
                // when enroll success, dump the templates
                LOG_D(LOG_TAG, "enroll success and dump templates");
                gf_hal_dump_template(cmd->group_id, cmd->finger_id);
            }
        }  // if end
    }  // do...
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_irq
 * Description: IRQ main entry
 * Input: None
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_irq()
{
    gf_error_t err = GF_SUCCESS;
    gf_irq_t *cmd = NULL;
    uint32_t size = sizeof(gf_irq_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_irq_t *) GF_MEM_MALLOC(size);   // NOLINT(542)

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = irq_pre_process(cmd);
        irq_process(cmd, &err);
        irq_post_process(cmd, err);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_screen_on
 * Description: Screen on handling
 * Input: None
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_screen_on(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_screen_on();
    if (g_hal_config.support_detect_sensor_temperature)
    {
        gf_hal_post_sem_detect_broken();
    }
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_screen_off
 * Description: Screen off handling
 * Input: None
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_screen_off(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    if (g_hal_config.support_detect_sensor_temperature)
    {
        gf_hal_post_sem_detect_broken();
    }
    err = gf_hal_common_screen_off();

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_set_safe_class
 * Description: Set safe class
 * Input: Device object, safe class
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_set_safe_class(void *dev,
                                                    gf_safe_class_t safe_class)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_safe_class(dev, safe_class);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_navigate
 * Description: Start to navigate
 * Input: Device object and navigation mode
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_navigate(void *dev,
                                              gf_nav_mode_t nav_mode)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_navigate(dev, nav_mode);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_enable_fingerprint_module
 * Description: Enable fingerprint module
 * Input: Device object and enable flag
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_enable_fingerprint_module(void *dev,
        uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_fingerprint_module(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_camera_capture
 * Description: Capture camera
 * Input: Device object
 * Output: None
 * Return: Error code
 */
static gf_error_t hal_milan_camera_capture(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_camera_capture(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_enable_ff_feature
 * Description: Enable ff feature
 * Input: Device object and enable flag
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_enable_ff_feature(void *dev,
                                                       uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_ff_feature(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_reset_lockout
 * Description: Reset lock out status
 * Input: None
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_reset_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_reset_lockout();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_lockout
 * Description: Lock out
 * Input: None
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_lockout();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_sync_finger_list
 * Description: Synchronize finger list
 * Input: Device object, finger list and finger count
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_sync_finger_list(void *dev, uint32_t *list,
                                                      int32_t count)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_sync_finger_list(dev, list, count);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_pause_enroll
 * Description: Pause enroll operation
 * Input: None
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_pause_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_pause_enroll();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_resume_enroll
 * Description: Resume enroll operation
 * Input: None
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_resume_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_resume_enroll();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_invoke_command
 * Description: Wrapper to invoke command
 * Input: Operation id, command id, command object and object size
 * Output: Command result data
 * Return: Error Code
 */
static gf_error_t hal_milan_invoke_command(uint32_t operation_id,
                                                    gf_cmd_id_t cmd_id,
                                                    void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = buffer;
    int32_t i = 0;
    uint8_t reset_chip_flag = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, cmd is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (len < (int32_t)sizeof(gf_cmd_header_t))  // NOLINT(705)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, len=%d, sizeof(gf_cmd_header_t)=%d",
                  __func__, len, (int32_t)sizeof(gf_cmd_header_t));
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (NULL == g_fingerprint_device)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, g_fingerprint_device is NULL",
                  __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if ((0 == g_enable_fingerprint_module) && (cmd_id != GF_CMD_INIT)
            && (cmd_id != GF_CMD_SET_ACTIVE_GROUP)
            && (cmd_id != GF_CMD_ENUMERATE)
            && (cmd_id != GF_CMD_DETECT_SENSOR))
        {
            LOG_D(LOG_TAG, "[%s] g_enable_fingerprint_module=0", __func__);
            break;
        }

        if(g_sensor_power_down == 1) {
            LOG_E(LOG_TAG, "[%s] power-off, exit", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        pthread_mutex_lock(&g_sensor_mutex);
        err = gf_hal_control_spi_clock(1);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] spi clock enable failed", __func__);
            pthread_mutex_unlock(&g_sensor_mutex);
            break;
        }

        err = gf_ca_invoke_command(operation_id, cmd_id, cmd, len);
        gf_hal_control_spi_clock(0);

        if (GF_ERROR_TA_DEAD == err)
        {
            gf_hal_destroy_timer(&g_enroll_timer_id);
            gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
            gf_hal_destroy_timer(&g_long_pressed_timer_id);
            gf_hal_reset_chip();
            err = gf_hal_reinit();
        }

        if (cmd->reset_flag > 0)
        {
            reset_chip_flag = 1;
            LOG_D(LOG_TAG, "[%s] reset_flag > 0, err = %d, reset chip", __func__, err);
            if (cmd->reset_flag == 2)
                gf_hal_power_reset();
            else
                gf_hal_reset_chip();
        }
        if (GF_ERROR_OVER_CURRENT == err)
        {
            g_hal_over_current_count++;
            if (g_hal_over_current_count < 6)
            {
                LOG_D(LOG_TAG, "[%s] g_hal_over_current_count is %d, power reset chip",
                            __func__, g_hal_over_current_count);
                gf_hal_disable_irq();
                usleep(5000);
                gf_hal_power_reset();
                reset_chip_flag = 1;
            }
            else
            {
                LOG_E(LOG_TAG, "[%s] err=%s, errno=%d. sensor is broken, disable power!",
                            __func__, gf_strerror(err), err);
                gf_hal_disable_power();
                gf_hal_device_remove();
                gf_hal_device_close();
            }
        }  // if ...
        else
        {
            LOG_E(LOG_TAG, "[%s] err=%s, errno=%d. reset g_hal_over_current_count to 0!",
                            __func__, gf_strerror(err), err);
            g_hal_over_current_count = 0;

        }

        pthread_mutex_unlock(&g_sensor_mutex);

        if (reset_chip_flag)
        {
            LOG_D(LOG_TAG, "[%s] after reset do irq process", __func__);
            reset_chip_flag = 0;
            hal_milan_irq();
            gf_hal_enable_irq();
        }

        if (MODE_KEY == g_mode && cmd->mode != MODE_KEY)
        {
            g_key_down_flag = 0;
        }

        if (OPERATION_CHECK_FINGER_LONG_PRESS == cmd->operation)
        {
            g_key_down_flag = 1;
        }

        if (g_mode != cmd->mode || g_operation != cmd->operation)
        {
            g_mode = cmd->mode;
            g_operation = cmd->operation;
            LOG_D(LOG_TAG, "[%s] cmd_id=%s, cmd_id=%d", __func__, gf_strcmd(cmd_id),
                  cmd_id);
            LOG_D(LOG_TAG, "[%s] g_mode=%s, g_mode=%d", __func__, gf_strmode(g_mode),
                  g_mode);
            LOG_D(LOG_TAG, "[%s] g_operation=%s, g_operation=%d", __func__,
                  gf_stroperation(g_operation), g_operation);
        }

        for (i = 0; i < MAX_OPERATION_ARRAY_SIZE; i++)
        {
            if (cmd->operation_array[i] != OPERATION_INVAILD)
            {
                LOG_V(LOG_TAG, "[%s] operation[%d]=%s, operation[%d]=%d", __func__,
                      i, gf_stroperation(cmd->operation_array[i]), i, cmd->operation_array[i]);
            }
        }
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_user_invoke_command
 * Description: Wrapper to invoke user command
 * Input: Command id, command object and object length
 * Output: Command result data
 * Return: Error code
 */
static gf_error_t hal_milan_user_invoke_command(uint32_t cmd_id,
                                                         void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_user_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_dump_invoke_command
 * Description: Wrapper to invoke dump command
 * Input: Command id, command object and object len
 * Output: Command result data
 * Return: Error Code
 */
static gf_error_t hal_milan_dump_invoke_command(uint32_t cmd_id,
                                                         void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_dump_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_authenticate_fido
 * Description: Authenticate for fido
 * Input: Device object, group id, aaid, aaid length, challenge and challenge length
 * Output: None
 * Return: Error Code
 */
static gf_error_t hal_milan_authenticate_fido(void *dev,
                                                       uint32_t group_id, uint8_t *aaid,
                                                       uint32_t aaid_len, uint8_t *challenge, uint32_t challenge_len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_authenticate_fido(dev, group_id, aaid, aaid_len, challenge,
                                          challenge_len);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_is_id_valid
 * Description: Check finger id is valid or not
 * Input: Device object, group id and finger id
 * Output: None
 * Return: Valid status
 */
static gf_error_t hal_milan_is_id_valid(void *dev, uint32_t group_id,
                                                 uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_is_id_valid(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_get_id_list
 * Description: Get finger id list
 * Input: Device object, group id
 * Output: Finger id list and item count
 * Return: Error Code
 */
static int32_t hal_milan_get_id_list(void *dev, uint32_t group_id,
                                              uint32_t *list,
                                              int32_t *count)
{
    int32_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_get_id_list(dev, group_id, list, count);
    VOID_FUNC_EXIT();
    return ret;
}

/*
 * Function: gf_hal_milan_function_customize
 * Description: Init hal function struct for milan chip
 * Input: None
 * Output: hal function struct
 * Return: Error Code
 */
gf_error_t gf_hal_milan_function_customize(gf_hal_function_t *hal_function)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (NULL == hal_function)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG "[%s] invalid param", __func__);
            break;
        }

        hal_function->init = hal_milan_init;
        hal_function->close = hal_milan_close;
        hal_function->cancel = hal_milan_cancel;
        hal_function->test_cancel = hal_milan_test_cancel;
        hal_function->test_prior_cancel = hal_milan_test_prior_cancel;
        hal_function->pre_enroll = hal_milan_pre_enroll;
        hal_function->enroll = hal_milan_enroll;
        hal_function->post_enroll = hal_milan_post_enroll;
        hal_function->authenticate = hal_milan_authenticate;
        hal_function->get_auth_id = hal_milan_get_auth_id;
        hal_function->remove = hal_milan_remove;
        hal_function->set_active_group = hal_milan_set_active_group;
        hal_function->enumerate = hal_milan_enumerate;
        hal_function->enumerate_with_callback =
            hal_milan_enumerate_with_callback;
        hal_function->irq = hal_milan_irq;
        hal_function->screen_on = hal_milan_screen_on;
        hal_function->screen_off = hal_milan_screen_off;
        hal_function->set_safe_class = hal_milan_set_safe_class;
        hal_function->navigate = hal_milan_navigate;
        hal_function->enable_fingerprint_module =
            hal_milan_enable_fingerprint_module;
        hal_function->camera_capture = hal_milan_camera_capture;
        hal_function->enable_ff_feature = hal_milan_enable_ff_feature;
        hal_function->reset_lockout = hal_milan_reset_lockout;
        hal_function->sync_finger_list = hal_milan_sync_finger_list;
        hal_function->invoke_command = hal_milan_invoke_command;
        hal_function->user_invoke_command = hal_milan_user_invoke_command;
        hal_function->dump_invoke_command = hal_milan_dump_invoke_command;
        hal_function->authenticate_fido = hal_milan_authenticate_fido;
        hal_function->is_id_valid = hal_milan_is_id_valid;
        hal_function->get_id_list = hal_milan_get_id_list;
        hal_function->lockout = hal_milan_lockout;
        hal_function->pause_enroll = hal_milan_pause_enroll;
        hal_function->resume_enroll = hal_milan_resume_enroll;
        hal_function->dump_chip_init_data = gf_hal_milan_dump_chip_init_data;
        hal_function->dump_chip_operation_data = gf_hal_milan_dump_chip_operation_data;
    }  // do block for regitering callbacks
    while (0);

    FUNC_EXIT(err);
    return err;
}

