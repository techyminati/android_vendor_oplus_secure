/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL layer simulator chip
 * History:
 * Version: 1.0
 */
#include <endian.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "gf_dump_data.h"
#include "gf_hal.h"
#include "gf_hal_device.h"
#include "gf_hal_simulator.h"
#include "gf_type_define.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_hal_dump.h"
#include "gf_ca_entry.h"
#include "gf_hal_timer.h"
#include "gf_hal_frr_database.h"
#include "gf_hal_test.h"
#include "gf_dump_bigdata.h"
#include "gf_hal_simulator_dump.h"

#define LOG_TAG "[GF_HAL][gf_hal_fps]"

timer_t g_key_long_pressed_timer_id_fps = 0;  // id of long pressed timer id

int64_t g_irq_time_fps = 0;  // irq time of fps

static gf_error_t irq_pre_process(gf_irq_t *cmd);
static void irq_process(gf_irq_t *cmd, gf_error_t *pre_process_result);
static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code);
static void irq_nav(gf_irq_t *cmd);
static void irq_key(gf_irq_t *cmd);
static void authenticate_not_match(gf_irq_t *cmd, gf_error_t err_code);
static void irq_enroll_or_authenticate(gf_irq_t *cmd, gf_error_t *err_code);
static void irq_down_enroll_or_authenticate(gf_irq_t *cmd,
                                            gf_error_t *err_code);
static void irq_image_enroll_or_authenticate(gf_irq_t *cmd,
                                             gf_error_t *err_code);
static void irq_up_enroll_or_authenticate(gf_irq_t *cmd);
static void sensor_is_broken();

/*
Function: hal_key_long_pressed_timer_thread
Description: call when key long pressed
Input: sigval
Output:
Return: none
Others:
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
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);
    VOID_FUNC_EXIT();
}

/*
Function: hal_fps_init
Description: init
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_init(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_init_t *cmd = NULL;
    uint32_t size = sizeof(gf_init_t);
    gf_ioc_chip_info_t info = { 0 };
    uint8_t download_fw_flag = 0;
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        err = gf_hal_get_fw_info(&download_fw_flag);
        cmd = (gf_init_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        gf_hal_common_load_otp_info_from_sdcard(cmd->otp_info, &(cmd->otp_info_len));
        cmd->download_fw_flag = download_fw_flag;
        err = gf_hal_invoke_command(GF_CMD_INIT, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        g_sensor_row = cmd->row;
        g_sensor_col = cmd->col;
        g_sensor_nav_row = cmd->nav_row;
        g_sensor_nav_col = cmd->nav_col;
        gf_dump_init(g_sensor_row, g_sensor_col, g_sensor_nav_row, g_sensor_nav_col,
                     g_hal_config.chip_type, g_hal_config.chip_series);
        gf_hal_judge_delete_frr_database(g_hal_config.chip_type,
                                         g_hal_config.chip_series);
        LOG_D(LOG_TAG, "[%s] init finished , save otp_info", __func__);
        gf_hal_common_save_otp_info_into_sdcard(cmd->otp_info, cmd->otp_info_len);
        err = gf_hal_device_enable();

        if (err != GF_SUCCESS)
        {
            break;
        }

        info.vendor_id = cmd->vendor_id[0];
        info.mode = g_mode;
        info.operation = g_operation;
        gf_hal_chip_info(info);
        err = gf_hal_init_finished();
    }  //  end do
    while (0);

    if (NULL != cmd)
    {
        free(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_close
Description: close
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_close(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);
    err = gf_hal_common_close(dev);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_cancel
Description: cancel operation and clear environment
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_nav_reset();
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) malloc(size);

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
        free(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_test_cancel
Description: test cancel operation and clear environment
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_test_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_nav_reset();
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);
        gf_hal_destroy_timer(&g_irq_timer_id);
        g_test_interrupt_pin_flag = 0;
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) malloc(size);

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
        free(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_test_prior_cancel
Description: test prior cancel
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_test_prior_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) malloc(size);

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
        free(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_pre_enroll
Description: pre enroll
Input: dev
Output:
Return: g_challenge
Others:
*/
static uint64_t hal_fps_pre_enroll(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_pre_enroll(dev);
    VOID_FUNC_EXIT();
    return ret;
}

/*
Function: hal_fps_enroll
Description: enroll
Input: dev, hat, group_id, timer time
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_enroll(void *dev, const void *hat,
                                            uint32_t group_id,
                                            uint32_t timeout_sec)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);

    err = gf_hal_common_enroll(dev, hat, group_id, timeout_sec);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_post_enroll
Description: post enroll
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_post_enroll(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_post_enroll(dev);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_authenticate
Description: authenticate
Input: dev, operation_id, group_id
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_authenticate(void *dev,
                                                  uint64_t operation_id,
                                                  uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);

    err = gf_hal_common_authenticate(dev, operation_id, group_id);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_get_auth_id
Description: get g_authenticator_id
Input: dev
Output:
Return: g_authenticator_id
Others:
*/
static uint64_t hal_fps_get_auth_id(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_get_auth_id(dev);
    VOID_FUNC_EXIT();
    return ret;
}

/*
Function: hal_fps_remove
Description: remove finger
Input: dev, group_id, finger_id
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_remove(void *dev, uint32_t group_id,
                                            uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    err = gf_hal_common_remove(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_set_active_group
Description: set active group
Input: dev, group_id
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_set_active_group(void *dev,
                                                      uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_active_group(dev, group_id);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_enumerate
Description: get finger list and the number of finger
Input: dev
Output: results, max_size
Return: error code
Others:
*/
static gf_error_t hal_fps_enumerate(void *dev, void *results,
                                               uint32_t *max_size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    err = gf_hal_common_enumerate(dev, results, max_size);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_enumerate_with_callback
Description: get finger list, the number of finger and notify this message by callback
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_enumerate_with_callback(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enumerate_with_callback(dev);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_on_nav_up
Description: navigation
Input:
Output:
Return: none
Others:
*/
static void hal_fps_on_nav_up()
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
Function: hal_fps_detect_nav_event
Description: detect navigation event
Input:
Output:
Return: none
Others:
*/
static gf_error_t hal_fps_detect_nav_event(gf_irq_t *cmd)
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

        if (detect_nav_event->nav_result.finger_up_flag > 0)
        {
            hal_fps_on_nav_up();
        }
    }
    while (0);

    gf_hal_dump_data_by_operation(OPERATION_NAV, err);

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_on_nav_down
Description: navigation down
Input: the pointer to comand
Output:
Return: none
Others:
*/
static void hal_fps_on_nav_down(gf_irq_t *cmd)
{
    int32_t time_delta = 0;
    int32_t nav_double_click_interval = 0;
    int32_t nav_long_press_interval = 0;
    VOID_FUNC_ENTER();
    gf_hal_send_nav_event(GF_NAV_FINGER_DOWN);
    g_key_down_flag = 1;
    g_nav_frame_index = 0;
    g_nav_times++;

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
                                           g_irq_time_fps) / 1000);

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
                    hal_fps_detect_nav_event(cmd);
                }

                break;
            }  // end case  GF_NAV_CLICK_STATUS_NONE

            case GF_NAV_CLICK_STATUS_DOWN_UP:
            {
                LOG_D(LOG_TAG, "[%s] get a double click", __func__);
                gf_hal_nav_reset();
                gf_hal_send_nav_event(GF_NAV_DOUBLE_CLICK);
                gf_hal_nav_complete();
                break;
            }

            default:
            {
                break;
            }
        }  // end switch
    }  // end if
    else
    {
        g_nav_click_status = GF_NAV_CLICK_STATUS_DOWN;

        if (GF_NAV_EVENT_DETECT_METHOD_POLLING ==
            g_hal_config.support_nav_get_data_method)
        {
            hal_fps_detect_nav_event(cmd);
        }
    }

    VOID_FUNC_EXIT();
}

/*
Function: hal_fps_authenticate_finish
Description: authenticate finish
Input: authenticate group_id and finger_id in buffer
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_authenticate_finish(gf_irq_t *buffer)
{
    gf_error_t err = GF_SUCCESS;
    gf_authenticate_finish_t *cmd = NULL;
    uint32_t size = sizeof(gf_authenticate_finish_t);

    FUNC_ENTER();

    do
    {
        cmd = (gf_authenticate_finish_t *) malloc(size);
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
        free(cmd);
    }

    FUNC_EXIT(err);

    return err;
}

/*
Function: gf_hal_fps_authenticate_success
Description: deal the result of authenticate success
Input: cmd, err_code
Output:
Return: none
Others:
*/
void gf_hal_fps_authenticate_success(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == cmd || NULL == err_code)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        if (OPERATION_AUTHENTICATE_FF != cmd->operation)
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
        }

        gf_hal_notify_authentication_succeeded(cmd->group_id, cmd->finger_id,
                                               &cmd->auth_token);
        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
              cmd->finger_id);
        LOG_D(LOG_TAG, "[%s] save_flag=%u", __func__, cmd->save_flag);
        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);

        hal_fps_authenticate_finish(cmd);

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
Function: gf_hal_fps_authenticate_fido_success
Description: deal the result of authenticate fido success
Input: cmd, err_code
Output:
Return: none
Others:
*/
void gf_hal_fps_authenticate_fido_success(gf_irq_t *cmd,
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

        hal_fps_authenticate_finish(cmd);

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
Function: irq_down_enroll_or_authenticate
Description: deal down irq when enroll and authenticate
Input: cmd, err_code
Output:
Return: none
Others:
*/
static void irq_down_enroll_or_authenticate(gf_irq_t *cmd,
                                            gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    if (0 == g_hal_config.report_key_event_only_enroll_authenticate
        || OPERATION_ENROLL == cmd->operation
        || OPERATION_AUTHENTICATE_FF == cmd->operation
        || OPERATION_AUTHENTICATE_IMAGE == cmd->operation
        || OPERATION_AUTHENTICATE_FIDO == cmd->operation)
    {
        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_FINGER_DOWN);
    }

    switch (cmd->operation)
    {
        case OPERATION_ENROLL:
        {
            gf_hal_create_timer(&g_long_pressed_timer_id, gf_hal_long_pressed_timer_thread);
            gf_hal_set_timer(&g_long_pressed_timer_id, 2, 2, 0);
            LOG_D(LOG_TAG, "[%s] start polling", __func__);
            cmd->step = GF_IRQ_STEP_POLLING;
            gf_hal_disable_irq();
            *err_code = gf_hal_invoke_command(GF_CMD_IRQ, cmd, sizeof(gf_irq_t));
            gf_hal_enable_irq();
            break;
        }

        case OPERATION_AUTHENTICATE_FF:
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
            /*fall-through*/
        }

        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_AUTHENTICATE_FIDO:
        {
            if (g_hal_config.support_set_spi_speed_in_tee > 0)
            {
                LOG_D(LOG_TAG, "[%s] start polling", __func__);
                cmd->step = GF_IRQ_STEP_POLLING;
                gf_hal_disable_irq();
                *err_code = gf_hal_invoke_command(GF_CMD_IRQ, cmd, sizeof(gf_irq_t));
                gf_hal_enable_irq();
            }

            break;
        }

        default:
        {
            break;
        }
    }  // end switch

    VOID_FUNC_EXIT();
}

/*
Function: sensor_is_broken
Description: sensor is broken checked
Input:
Output:
Return: none
Others:
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
                                              1))
        {
            LOG_D(LOG_TAG, "[%s] broken checked, discard this enrolling", __func__);
            g_enroll_invalid_template_num = 0;
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_HW_UNAVAILABLE);
            hal_fps_cancel(g_fingerprint_device);
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
Function: hal_fps_authenticate_retry
Description: retry to authenticate when authenticate failure last
Input: cmd
Output: authenticate  error code in result
Return:  error code
Others:
*/
static gf_error_t hal_fps_authenticate_retry(gf_irq_t *cmd, gf_error_t *result)
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
            gf_hal_dump_data_by_operation(cmd->operation, *result);
        }

        LOG_D(LOG_TAG, "[%s] start polling", __func__);
        cmd->step = GF_IRQ_STEP_POLLING;
        *result = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))
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
Function: irq_image_enroll_or_authenticate
Description: deal the result of enroll and authenticate
Input: cmd, error code
Output:
Return: none
Others:
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
                gf_hal_fps_authenticate_success(cmd, err_code);
            }
            else if (OPERATION_AUTHENTICATE_FIDO == cmd->operation)
            {
                gf_hal_fps_authenticate_fido_success(cmd, err_code);
            }

            break;
        }

        case GF_ERROR_SENSOR_IS_BROKEN:
        {
            sensor_is_broken();
            break;
        }

        case GF_ERROR_ACQUIRED_IMAGER_DIRTY:
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK image quality is too low", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
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
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER);
            LOG_D(LOG_TAG, "[%s] duplicate finger id=%u", __func__,
                  cmd->duplicate_finger_id);
            gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            break;
        }

        case GF_ERROR_DUPLICATE_AREA:
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA);
            LOG_D(LOG_TAG, "[%s] duplicate area", __func__);
            gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            break;
        }

        case GF_ERROR_MATCH_FAIL_AND_RETRY:
        {
            LOG_I(LOG_TAG, "[%s] match fail and retry", __func__);
            while (GF_ERROR_MATCH_FAIL_AND_RETRY == *err_code)
            {
                hal_fps_authenticate_retry(cmd, err_code);
            }
            break;
        }

        case GF_ERROR_NOT_MATCH:
        {
            authenticate_not_match(cmd, *err_code);
            break;
        }

        case GF_ERROR_SPI_RAW_DATA_CRC_FAILED:
        {
            LOG_I(LOG_TAG, "[%s] crc check error, err=%s, errno=%d", __func__,
                  gf_strerror(*err_code),
                  *err_code);
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_SPI_COMMUNICATION);
            break;
        }

        default:
        {
            LOG_I(LOG_TAG, "[%s] won't handle this error code, err=%s, errno=%d", __func__,
                  gf_strerror(*err_code), *err_code);
            break;
        }
    }  // end switch

    VOID_FUNC_EXIT();
}

/*
Function: irq_up_enroll_or_authenticate
Description: deal up irq
Input: cmd
Output:
Return: none
Others:
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
            || OPERATION_AUTHENTICATE_FIDO == cmd->operation)
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_FINGER_UP);
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
Function: irq_enroll_or_authenticate
Description: deal down, image, up irq in enroll and authenticate
Input: cmd, err_code
Output:
Return: none
Others:
*/
static void irq_enroll_or_authenticate(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
    {
        irq_down_enroll_or_authenticate(cmd, err_code);
    }

    if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))
    {
        irq_image_enroll_or_authenticate(cmd, err_code);
    }

    if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
    {
        irq_up_enroll_or_authenticate(cmd);
    }

    VOID_FUNC_EXIT();
}

/*
Function: authenticate_not_match
Description: deal authenticate not match
Input: cmd, err_code
Output:
Return: none
Others:
*/
static void authenticate_not_match(gf_irq_t *cmd, gf_error_t err_code)
{
    VOID_FUNC_ENTER()

    do
    {
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        if (cmd->too_fast_flag > 0)
        {
            LOG_I(LOG_TAG, "[%s] press too fast", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
        }
        else if (cmd->mistake_touch_flag > 0)
        {
            LOG_I(LOG_TAG, "[%s] touch by mistake", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE);
        }
        else
        {
            if (OPERATION_AUTHENTICATE_FF == cmd->operation
                || OPERATION_AUTHENTICATE_IMAGE == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] no finger match", __func__);
                gf_hal_notify_authentication_failed();
            }
            else if (OPERATION_AUTHENTICATE_FIDO == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] authenticator fido, no finger match", __func__);
                gf_hal_notify_authentication_fido_failed();
            }
        }

        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }
    }  // end do
    while (0);

    VOID_FUNC_EXIT();
}

/*
Function: irq_key
Description: key irq
Input: cmd
Output:
Return: none
Others:
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
                gf_hal_create_timer(&g_key_long_pressed_timer_id_fps,
                                    hal_key_long_pressed_timer_thread);
                gf_hal_set_timer(&g_key_long_pressed_timer_id_fps, 5, 5, 0);
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
Function: irq_nav
Description: navigation irq
Input: cmd
Output:
Return: none
Others:
*/
static void irq_nav(gf_irq_t *cmd)
{
    VOID_FUNC_ENTER();

    do
    {
        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            pthread_mutex_lock(&g_nav_click_status_mutex);
            hal_fps_on_nav_down(cmd);
            pthread_mutex_unlock(&g_nav_click_status_mutex);
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
        {
            if (g_key_down_flag > 0)
            {
                pthread_mutex_lock(&g_nav_click_status_mutex);
                hal_fps_on_nav_up();
                pthread_mutex_unlock(&g_nav_click_status_mutex);
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
Function: irq_process
Description: deal irq
Input: cmd
Output:
Return: none
Others:
*/
static void irq_process(gf_irq_t *cmd, gf_error_t *pre_process_result)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(pre_process_result);

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
            gf_hal_common_irq_finger_long_press(cmd, *pre_process_result);
            break;
        }

        case OPERATION_ENROLL:
        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_AUTHENTICATE_FF:
        case OPERATION_AUTHENTICATE_FIDO:
        {
            irq_enroll_or_authenticate(cmd, pre_process_result);
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
            LOG_D(LOG_TAG, "[%s] Unkown operation", __func__);
            break;
        }
    }  // end switch

    VOID_FUNC_EXIT();
}

/*
Function: irq_pre_process
Description: get irq type
Input: cmd
Output:
Return: error code
Others:
*/
static gf_error_t irq_pre_process(gf_irq_t *cmd)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t size = sizeof(gf_irq_t);
    FUNC_ENTER();

    do
    {
        g_irq_time_fps = gf_hal_current_time_microsecond();
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

        if (GF_IRQ_RESET_MASK == (cmd->irq_type & GF_IRQ_RESET_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_RESET_MASK", __func__);
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
            g_down_irq_time = g_irq_time_fps;
        }

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK", __func__);
            g_image_irq_time = g_irq_time_fps;
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_UP_MASK", __func__);
            gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);
        }

        if (0 != (cmd->irq_type & GF_IRQ_MENUKEY_DOWN_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_MENUKEY_DOWN_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_MENU, GF_KEY_STATUS_DOWN);
        }
        else if (0 != (cmd->irq_type & GF_IRQ_MENUKEY_UP_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_MENUKEY_UP_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_MENU, GF_KEY_STATUS_UP);
        }

        if (0 != (cmd->irq_type & GF_IRQ_BACKKEY_DOWN_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_BACKKEY_DOWN_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_BACK, GF_KEY_STATUS_DOWN);
        }
        else if (0 != (cmd->irq_type & GF_IRQ_BACKKEY_UP_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_BACKKEY_UP_MASK", __func__);
            gf_hal_send_key_event(GF_KEY_BACK, GF_KEY_STATUS_UP);
        }

        if (0 != (cmd->irq_type & GF_IRQ_PRESS_HEAVY_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_PRESS_HEAVY_MASK", __func__);
        }
        else if (0 != (cmd->irq_type & GF_IRQ_PRESS_LIGHT_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_PRESS_LIGHT_MASK", __func__);
        }
    }  // end do
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
Function: irq_post_process
Description: dump data after deal irq
Input: cmd, error code
Output:
Return: none
Others:
*/
static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL != cmd)
        {
            uint32_t is_image_or_nav = cmd->irq_type & (GF_IRQ_IMAGE_MASK | GF_IRQ_NAV_MASK);
            if (is_image_or_nav)
            {
                LOG_D(LOG_TAG, "dump the authenticate and enroll data ");
                gf_hal_dump_data_by_operation(cmd->operation, error_code);
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
        }  // end if
    }  // end do
    while (0);

    VOID_FUNC_EXIT();
}

/*
Function: hal_fps_irq
Description: deal irq
Input:
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_irq()
{
    gf_error_t err = GF_SUCCESS;
    gf_irq_t *cmd = NULL;
    uint32_t size = sizeof(gf_irq_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_irq_t *) malloc(size);

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
        free(cmd);
        cmd = NULL;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_screen_on
Description: screen on
Input:
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_screen_on(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_screen_on();
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_screen_off
Description: screen off
Input:
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_screen_off(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_screen_off();
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_set_safe_class
Description: set safe class
Input: dev, safe_class
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_set_safe_class(void *dev,
                                                    gf_safe_class_t safe_class)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_safe_class(dev, safe_class);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_navigate
Description: navigate
Input: dev, nav_mode
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_navigate(void *dev,
                                              gf_nav_mode_t nav_mode)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_navigate(dev, nav_mode);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_enable_fingerprint_module
Description: enable fingerprint module
Input: dev, enable_flag
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_enable_fingerprint_module(void *dev,
        uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_fingerprint_module(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_camera_capture
Description: camera capture
Input: dev
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_camera_capture(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_camera_capture(dev);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_enable_ff_feature
Description: enable ff feature
Input: dev, enable_flag
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_enable_ff_feature(void *dev,
                                                       uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_ff_feature(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_reset_lockout
Description: reset lockout
Input:
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_reset_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_reset_lockout();
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_lockout
Description: lockout
Input:
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_lockout();
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_sync_finger_list
Description: synchronous finger list
Input: dev, list, count
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_sync_finger_list(void *dev, uint32_t *list,
                                                      int32_t count)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_sync_finger_list(dev, list, count);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_pause_enroll
Description: pause enroll
Input:
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_pause_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_pause_enroll();
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_resume_enroll
Description: resume enroll
Input:
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_resume_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_resume_enroll();
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_invoke_command
Description: send message from ca to ta
Input: operation_id, cmd_id, buffer, len
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_invoke_command(uint32_t operation_id,
                                                    gf_cmd_id_t cmd_id,
                                                    void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = buffer;
    int32_t i = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, cmd is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (len < (int32_t)sizeof(gf_cmd_header_t))
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, len=%d, sizeof(gf_cmd_header_t)=%d",
                  __func__,
                  len, (int32_t)sizeof(gf_cmd_header_t));
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

        pthread_mutex_lock(&g_sensor_mutex);
        gf_hal_control_spi_clock(1);
        err = gf_ca_invoke_command(operation_id, cmd_id, cmd, len);
        gf_hal_control_spi_clock(0);

        if (GF_ERROR_TA_DEAD == err)
        {
            gf_hal_destroy_timer(&g_enroll_timer_id);
            gf_hal_destroy_timer(&g_key_long_pressed_timer_id_fps);
            gf_hal_destroy_timer(&g_long_pressed_timer_id);
            gf_hal_reset_chip();
            err = gf_hal_reinit();
        }

        if (cmd->reset_flag > 0)
        {
            LOG_D(LOG_TAG, "[%s] reset_flag > 0, reset chip", __func__);
            gf_hal_reset_chip();
        }

        pthread_mutex_unlock(&g_sensor_mutex);

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
    }  // end do
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_user_invoke_command
Description: send message from ca to ta
Input: cmd_id, buffer, len
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_user_invoke_command(uint32_t cmd_id,
                                                         void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_user_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_dump_invoke_command
Description: send message from ca to ta
Input: cmd_id, buffer, len
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_dump_invoke_command(uint32_t cmd_id,
                                                         void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_dump_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_authenticate_fido
Description: authenticate fido
Input: dev, group_id, aaid, aaid_len, challenge, challenge_len
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_authenticate_fido(void *dev,
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
Function: hal_fps_is_id_valid
Description: check finger_id is valid
Input: dev, group_id, finger_id
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_is_id_valid(void *dev, uint32_t group_id,
                                                 uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_is_id_valid(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_get_id_list
Description: get finger_id list and count in group_id
Input: dev, group_id
Output: list, count
Return: error code
Others:
*/
static int32_t hal_fps_get_id_list(void *dev, uint32_t group_id,
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
Function: gf_hal_simulator_function_customize
Description: init hal_function
Input: hal_function
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_simulator_function_customize(gf_hal_function_t
                                                    *hal_function)
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

        hal_function->init = hal_fps_init;
        hal_function->close = hal_fps_close;
        hal_function->cancel = hal_fps_cancel;
        hal_function->test_cancel = hal_fps_test_cancel;
        hal_function->test_prior_cancel = hal_fps_test_prior_cancel;
        hal_function->pre_enroll = hal_fps_pre_enroll;
        hal_function->enroll = hal_fps_enroll;
        hal_function->post_enroll = hal_fps_post_enroll;
        hal_function->authenticate = hal_fps_authenticate;
        hal_function->get_auth_id = hal_fps_get_auth_id;
        hal_function->remove = hal_fps_remove;
        hal_function->set_active_group = hal_fps_set_active_group;
        hal_function->enumerate = hal_fps_enumerate;
        hal_function->enumerate_with_callback =
            hal_fps_enumerate_with_callback;
        hal_function->irq = hal_fps_irq;
        hal_function->screen_on = hal_fps_screen_on;
        hal_function->screen_off = hal_fps_screen_off;
        hal_function->set_safe_class = hal_fps_set_safe_class;
        hal_function->navigate = hal_fps_navigate;
        hal_function->enable_fingerprint_module =
            hal_fps_enable_fingerprint_module;
        hal_function->camera_capture = hal_fps_camera_capture;
        hal_function->enable_ff_feature = hal_fps_enable_ff_feature;
        hal_function->reset_lockout = hal_fps_reset_lockout;
        hal_function->sync_finger_list = hal_fps_sync_finger_list;
        hal_function->invoke_command = hal_fps_invoke_command;
        hal_function->user_invoke_command = hal_fps_user_invoke_command;
        hal_function->dump_invoke_command = hal_fps_dump_invoke_command;
        hal_function->authenticate_fido = hal_fps_authenticate_fido;
        hal_function->is_id_valid = hal_fps_is_id_valid;
        hal_function->get_id_list = hal_fps_get_id_list;
        hal_function->lockout = hal_fps_lockout;
        hal_function->pause_enroll = hal_fps_pause_enroll;
        hal_function->resume_enroll = hal_fps_resume_enroll;
        hal_function->dump_chip_init_data = hal_fps_dump_chip_init_data;
    }  // endo do
    while (0);

    FUNC_EXIT(err);
    return err;
}

