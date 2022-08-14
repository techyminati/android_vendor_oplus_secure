/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include <endian.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "gf_dump_data.h"
#include "gf_hal.h"
#include "gf_hal_device.h"
#include "gf_hal_oswego_m.h"
#include "gf_type_define.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_hal_dump.h"
#include "gf_ca_entry.h"
#include "gf_hal_timer.h"
#include "gf_queue.h"
#include "gf_hal_test.h"
#include "gf_hal_oswego_m_test.h"
#include "gf_hal_frr_database.h"
#include "gf_hal_mem.h"

#define LOG_TAG "[GF_HAL][gf_hal_oswego_m_test]"


int64_t g_irq_time = 0;  // irq time of oswego

static gf_error_t irq_pre_process(gf_irq_t *cmd);
static void irq_process(gf_irq_t *cmd, gf_error_t *pre_process_result);
static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code);
static void irq_nav(gf_irq_t *cmd);

static void irq_enroll_or_authenticate(gf_irq_t *cmd, gf_error_t *err_code);
static void irq_down_enroll_or_authenticate(gf_irq_t *cmd,
                                            gf_error_t *err_code);
static void irq_image_enroll_or_authenticate(gf_irq_t *cmd,
                                             gf_error_t *err_code);
static void irq_up_enroll_or_authenticate(gf_irq_t *cmd);
static void sensor_is_broken();

static gf_error_t hal_oswego_m_init(void *dev)
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
        gf_hal_get_fw_info(&download_fw_flag);
        cmd = (gf_init_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
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
        g_esd_check_flag = cmd->esd_check_flag;
        err = gf_hal_download_cfg();

        if (err != GF_SUCCESS)
        {
            break;
        }

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

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_close(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_esd_timer_id);
    err = gf_hal_common_close(dev);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) GF_MEM_MALLOC(size);

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

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_test_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_irq_timer_id);
        g_test_interrupt_pin_flag = 0;
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) GF_MEM_MALLOC(size);

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

static gf_error_t hal_oswego_m_test_prior_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        gf_hal_common_cancel(dev);
        cmd = (gf_cancel_t *) GF_MEM_MALLOC(size);

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

static uint64_t hal_oswego_m_pre_enroll(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_pre_enroll(dev);
    VOID_FUNC_EXIT();
    return ret;
}

static gf_error_t hal_oswego_m_enroll(void *dev, const void *hat,
                                      uint32_t group_id,
                                      uint32_t timeout_sec)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enroll(dev, hat, group_id, timeout_sec);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_post_enroll(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_post_enroll(dev);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_authenticate(void *dev, uint64_t operation_id,
                                            uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_authenticate(dev, operation_id, group_id);
    FUNC_EXIT(err);
    return err;
}

static uint64_t hal_oswego_m_get_auth_id(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_get_auth_id(dev);
    VOID_FUNC_EXIT();
    return ret;
}

static gf_error_t hal_oswego_m_remove(void *dev, uint32_t group_id,
                                      uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_remove(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_set_active_group(void *dev, uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_active_group(dev, group_id);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_enumerate(void *dev, void *results,
                                         uint32_t *max_size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enumerate(dev, results, max_size);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_enumerate_with_callback(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enumerate_with_callback(dev);
    FUNC_EXIT(err);
    return err;
}

static void hal_oswego_m_on_nav_down()
{
    VOID_FUNC_ENTER();
    gf_hal_send_nav_event(GF_NAV_FINGER_DOWN);
    g_key_down_flag = 1;

    if (g_hal_config.nav_double_click_interval_in_ms > 0
        || g_hal_config.nav_long_press_interval_in_ms > 0)
    {
        gf_hal_nav_assert_config_interval();

        switch (g_nav_click_status)
        {
            case GF_NAV_CLICK_STATUS_NONE:
            {
                LOG_D(LOG_TAG, "[%s] get the first down", __func__);

                if (g_hal_config.nav_double_click_interval_in_ms > 0)
                {
                    gf_hal_create_timer(&g_nav_double_click_timer_id,
                                        gf_hal_nav_double_click_timer_thread);
                    gf_hal_set_timer(&g_nav_double_click_timer_id, 0,
                                     g_hal_config.nav_double_click_interval_in_ms / 1000,
                                     1000 * 1000 * (g_hal_config.nav_double_click_interval_in_ms % 1000));
                }

                if (g_hal_config.nav_long_press_interval_in_ms > 0)
                {
                    gf_hal_create_timer(&g_nav_long_press_timer_id,
                                        gf_hal_nav_long_press_timer_thread);
                    gf_hal_set_timer(&g_nav_long_press_timer_id, 0,
                                     g_hal_config.nav_long_press_interval_in_ms / 1000,
                                     1000 * 1000 * (g_hal_config.nav_long_press_interval_in_ms % 1000));
                }

                g_nav_click_status = GF_NAV_CLICK_STATUS_DOWN;
                break;
            }

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
        }
    }
    else
    {
        g_nav_click_status = GF_NAV_CLICK_STATUS_DOWN;
    }

    VOID_FUNC_EXIT();
}

static void hal_oswego_m_on_nav_up()
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
            break;
        }

        case OPERATION_AUTHENTICATE_FF:
        {
#if defined(__ANDROID_O) || defined(__ANDROID_P)
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_DETECTED);
#endif  // _ANDROID_O
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);  // fall-through
        }  // no break here!

        case OPERATION_AUTHENTICATE_IMAGE:  // NOLINT
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
    }

    VOID_FUNC_EXIT();
}

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
                gf_hal_common_authenticate_success(cmd, err_code);
            }
            else if (OPERATION_AUTHENTICATE_FIDO == cmd->operation)
            {
                gf_hal_common_authenticate_fido_success(cmd, err_code);
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

        case GF_ERROR_SENSOR_BROKEN_CHECK_ALGO_ERROR:  // NOLINT
        case GF_ERROR_ACQUIRED_PARTIAL:
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK valid area is too low", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_PARTIAL);
            break;
        }

        case GF_ERROR_INVALID_FINGER_PRESS:  // NOLINT
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
            break;
        }

        case GF_ERROR_NOT_MATCH:
        {
            gf_hal_common_authenticate_not_match(cmd, *err_code);
            break;
        }

        case GF_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS:
        {
            LOG_I(LOG_TAG, "[%s] too much under saturated pixels, err=%s, errno=%d",
                  __func__, gf_strerror(*err_code), *err_code);
            gf_hal_notify_error_info(
                GF_FINGERPRINT_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS);
            break;
        }

        case GF_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS:
        {
            LOG_I(LOG_TAG, "[%s] too much over saturated pixels, err=%s, errno=%d",
                  __func__, gf_strerror(*err_code), *err_code);
            gf_hal_notify_error_info(
                GF_FINGERPRINT_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS);
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
    }

    VOID_FUNC_EXIT();
}

static void irq_up_enroll_or_authenticate(gf_irq_t *cmd)
{
    VOID_FUNC_ENTER();

    do
    {
        if (cmd->too_fast_flag > 0)
        {
            LOG_I(LOG_TAG, "[%s] press too fast", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
        }
        else if (cmd->mistake_touch_flag > 0)
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE);
        }
        else if (cmd->report_authenticate_fail_flag > 0)
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

            gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
        }

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
            hal_oswego_m_cancel(g_fingerprint_device);
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

static void irq_nav(gf_irq_t *cmd)
{
    VOID_FUNC_ENTER();

    do
    {
        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            pthread_mutex_lock(&g_nav_click_status_mutex);
            hal_oswego_m_on_nav_down();
            pthread_mutex_unlock(&g_nav_click_status_mutex);
            g_nav_frame_index = 0;
            g_nav_times++;
        }

        if (0 != (cmd->irq_type & GF_IRQ_NAV_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_NAV_MASK", __func__);
            pthread_mutex_lock(&g_nav_click_status_mutex);

            if (GF_NAV_NONE != cmd->nav_result.nav_code
                && GF_NAV_CLICK != cmd->nav_result.nav_code
                && GF_NAV_HEAVY != cmd->nav_result.nav_code
                && GF_NAV_CLICK_STATUS_NONE != g_nav_click_status)
            {
                gf_hal_nav_listener(cmd->nav_result.nav_code);
            }

            pthread_mutex_unlock(&g_nav_click_status_mutex);
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
        {
            if (g_key_down_flag > 0)
            {
                pthread_mutex_lock(&g_nav_click_status_mutex);
                hal_oswego_m_on_nav_up();
                pthread_mutex_unlock(&g_nav_click_status_mutex);
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

static gf_error_t irq_pre_process(gf_irq_t *cmd)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t size = sizeof(gf_irq_t);
    FUNC_ENTER();

    do
    {
        g_irq_time = gf_hal_current_time_microsecond();
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

        LOG_I(LOG_TAG, "[%s] irq_type=%s, irq_type=0x%08X", __func__,
              gf_strirq(cmd->irq_type),
              cmd->irq_type);

        if (GF_IRQ_RESET_MASK == (cmd->irq_type & GF_IRQ_RESET_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_RESET_MASK", __func__);
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_DOWN_MASK", __func__);
            g_down_irq_time = g_irq_time;
        }

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK", __func__);
            g_image_irq_time = g_irq_time;
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_UP_MASK", __func__);
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
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static void irq_process(gf_irq_t *cmd, gf_error_t *pre_process_result)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(pre_process_result);

    switch (cmd->operation)
    {
        case OPERATION_HOME_KEY:  // NOLINT
        case OPERATION_CAMERA_KEY:  // NOLINT
        case OPERATION_POWER_KEY:
        {
            gf_hal_common_irq_key(cmd);
            break;
        }

        case OPERATION_ENROLL:  // NOLINT
        case OPERATION_AUTHENTICATE_IMAGE:  // NOLINT
        case OPERATION_AUTHENTICATE_FF:  // NOLINT
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
            hal_oswego_irq_test(cmd, pre_process_result);
            break;
        }
    }

    VOID_FUNC_EXIT();
}

static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL != cmd)
        {
            if (cmd->irq_type & (GF_IRQ_IMAGE_MASK | GF_IRQ_NAV_MASK))
            {
                gf_hal_dump_data_by_operation(cmd->operation, error_code);
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

static gf_error_t hal_oswego_m_irq()
{
    gf_error_t err = GF_SUCCESS;
    gf_irq_t *cmd = NULL;
    uint32_t size = sizeof(gf_irq_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_irq_t *) GF_MEM_MALLOC(size);

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
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}  // NOLINT

static gf_error_t hal_oswego_m_screen_on(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_screen_on();
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_screen_off(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_screen_off();
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_set_safe_class(void *dev,
                                              gf_safe_class_t safe_class)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_safe_class(dev, safe_class);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_navigate(void *dev, gf_nav_mode_t nav_mode)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_navigate(dev, nav_mode);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_enable_fingerprint_module(void *dev,
                                                         uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_fingerprint_module(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_camera_capture(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_camera_capture(dev);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_enable_ff_feature(void *dev,
                                                 uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_ff_feature(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_reset_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_reset_lockout();
    FUNC_EXIT(err);
    return err;
}


static gf_error_t hal_oswego_m_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_lockout();
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_sync_finger_list(void *dev, uint32_t *list,
                                                int32_t count)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_sync_finger_list(dev, list, count);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_pause_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_pause_enroll();
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_resume_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_resume_enroll();
    FUNC_EXIT(err);
    return err;
}


static gf_error_t hal_oswego_m_invoke_command(uint32_t operation_id,
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

        pthread_mutex_lock(&g_sensor_mutex);
        gf_hal_control_spi_clock(1);
        err = gf_ca_invoke_command(operation_id, cmd_id, cmd, len);
        gf_hal_control_spi_clock(0);

        if (GF_ERROR_TA_DEAD == err)
        {
            gf_hal_destroy_timer(&g_enroll_timer_id);
            gf_hal_destroy_timer(&g_esd_timer_id);
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

        if ((g_mode != MODE_SLEEP && g_mode != MODE_DEBUG && g_mode != MODE_FF)
            && (MODE_SLEEP == cmd->mode || MODE_DEBUG == cmd->mode
            || MODE_FF == cmd->mode))
        {
            gf_hal_destroy_timer(&g_esd_timer_id);
        }
        else if ((MODE_SLEEP == g_mode || MODE_DEBUG == g_mode || MODE_FF == g_mode)
                 && (cmd->mode != MODE_SLEEP && cmd->mode != MODE_DEBUG
                 && cmd->mode != MODE_FF))
        {
            gf_hal_create_and_set_esd_timer();
        }

        if (MODE_KEY == g_mode && cmd->mode != MODE_KEY)
        {
            g_key_down_flag = 0;
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
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_user_invoke_command(uint32_t cmd_id,
                                                   void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_user_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_dump_invoke_command(uint32_t cmd_id,
                                                   void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_dump_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_authenticate_fido(void *dev, uint32_t group_id,
                                                 uint8_t *aaid,
                                                 uint32_t aaid_len, uint8_t *challenge, uint32_t challenge_len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_authenticate_fido(dev, group_id, aaid, aaid_len, challenge,
                                          challenge_len);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_oswego_m_is_id_valid(void *dev, uint32_t group_id,
                                           uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_is_id_valid(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

static int32_t hal_oswego_m_get_id_list(void *dev, uint32_t group_id,
                                        uint32_t *list,
                                        int32_t *count)
{
    int32_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_get_id_list(dev, group_id, list, count);
    VOID_FUNC_EXIT();
    return ret;
}

gf_error_t gf_hal_oswego_m_function_customize(gf_hal_function_t *hal_function)
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

        hal_function->init = hal_oswego_m_init;
        hal_function->close = hal_oswego_m_close;
        hal_function->cancel = hal_oswego_m_cancel;
        hal_function->test_cancel = hal_oswego_m_test_cancel;
        hal_function->test_prior_cancel = hal_oswego_m_test_prior_cancel;
        hal_function->pre_enroll = hal_oswego_m_pre_enroll;
        hal_function->enroll = hal_oswego_m_enroll;
        hal_function->post_enroll = hal_oswego_m_post_enroll;
        hal_function->authenticate = hal_oswego_m_authenticate;
        hal_function->get_auth_id = hal_oswego_m_get_auth_id;
        hal_function->remove = hal_oswego_m_remove;
        hal_function->set_active_group = hal_oswego_m_set_active_group;
        hal_function->enumerate = hal_oswego_m_enumerate;
        hal_function->enumerate_with_callback = hal_oswego_m_enumerate_with_callback;
        hal_function->irq = hal_oswego_m_irq;
        hal_function->screen_on = hal_oswego_m_screen_on;
        hal_function->screen_off = hal_oswego_m_screen_off;
        hal_function->set_safe_class = hal_oswego_m_set_safe_class;
        hal_function->navigate = hal_oswego_m_navigate;
        hal_function->enable_fingerprint_module =
            hal_oswego_m_enable_fingerprint_module;
        hal_function->camera_capture = hal_oswego_m_camera_capture;
        hal_function->enable_ff_feature = hal_oswego_m_enable_ff_feature;
        hal_function->reset_lockout = hal_oswego_m_reset_lockout;
        hal_function->sync_finger_list = hal_oswego_m_sync_finger_list;
        hal_function->invoke_command = hal_oswego_m_invoke_command;
        hal_function->user_invoke_command = hal_oswego_m_user_invoke_command;
        hal_function->dump_invoke_command = hal_oswego_m_dump_invoke_command;
        hal_function->authenticate_fido = hal_oswego_m_authenticate_fido;
        hal_function->is_id_valid = hal_oswego_m_is_id_valid;
        hal_function->get_id_list = hal_oswego_m_get_id_list;
        hal_function->lockout = hal_oswego_m_lockout;
        hal_function->pause_enroll = hal_oswego_m_pause_enroll;
        hal_function->resume_enroll = hal_oswego_m_resume_enroll;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

