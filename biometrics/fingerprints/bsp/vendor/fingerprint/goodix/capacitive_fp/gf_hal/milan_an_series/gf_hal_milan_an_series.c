/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL layer implementation for milan an series
 * History:
 * Version: 1.0
 */

#include <endian.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "gf_common.h"
#include "gf_dump_data.h"
#include "gf_hal.h"
#include "gf_hal_device.h"
#include "gf_hal_milan_an_series.h"
#include "gf_type_define.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_hal_dump.h"
#include "gf_ca_entry.h"
#include "gf_hal_timer.h"
#include "gf_hal_test.h"
#include "gf_hal_milan_an_series_test.h"
#include "gf_hal_frr_database.h"
#include "gf_milan_an_series_dump.h"

#define LOG_TAG "[GF_HAL][gf_hal_milan_an_series]"
#define GF_ESD_TIME_SPAN_MS (2000)

int64_t g_irq_time_milan_an = 0;  // irq time of milan_a
static uint32_t g_match_fail_and_retry = 0;  // g_match_fail_and_retry
static uint32_t g_preprocess_fail_and_retry = 0;  // g_preprocess_fail_and_retry
static uint32_t g_crc_check_fail_and_retry = 0;  // g_crc_check_fail_and_retry

static gf_error_t irq_pre_process(gf_irq_t *cmd);
static gf_error_t irq_polling();
static void irq_process(gf_irq_t *cmd, gf_error_t *err_code);
static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code);
static void irq_enroll_or_authenticate(gf_irq_t *cmd, gf_error_t *err_code);
static void irq_down_enroll_or_authenticate(gf_irq_t *cmd,
                                            gf_error_t *err_code);
static void irq_image_enroll_or_authenticate(gf_irq_t *cmd,
                                             gf_error_t *err_code);
static void irq_up_enroll_or_authenticate(gf_irq_t *cmd);
static void gf_hal_milan_an_authenticate_success(gf_irq_t *cmd, gf_error_t *err_code);
static void gf_hal_milan_an_authenticate_fido_success(gf_irq_t *cmd,
                                             gf_error_t *err_code);
static void sensor_is_broken();
static gf_error_t hal_milan_an_series_reset_fwcfg();

/*
 * Function: hal_milan_an_series_download_fwcfg
 * Description: download the firmware config
 * Input:
 * Output: error status
 * Return:
 * Others:
 */
static gf_error_t hal_milan_an_series_download_fwcfg()
{
    gf_error_t err = GF_SUCCESS;
    uint8_t i = 0;

    gf_hal_destroy_timer(&g_esd_timer_id);

    do
    {
        for (i = 0; i < 3; i++)
        {
            gf_hal_reset_chip();
            err = gf_hal_download_fw();
            LOG_E(LOG_TAG, "[%s] download fw_cfg trial time=%d, code=%s", __func__, i, gf_strerror(err) );
            if (GF_SUCCESS == err)
            {
                break;
            }
        }
    }
    while (0);

    if (MODE_SLEEP != g_mode && MODE_FF != g_mode)
    {
        gf_hal_create_and_set_esd_timer();
    }
    return err;
}

/*
 * Function: hal_milan_an_series_init
 * Description: the init function
 * Input:
 * Output: error status
 * Return:
 * Others:
 */
static gf_error_t hal_milan_an_series_init(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_init_t *cmd = NULL;
    uint32_t size = sizeof(gf_init_t);
    gf_ioc_chip_info_t info = { 0 };
    uint8_t download_fw_flag = 0;
    uint8_t otp_info_tmp[GF_SENSOR_OTP_BUFFER_LEN] = { 0 };

    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        gf_hal_get_fw_info(&download_fw_flag);
        cmd = (gf_init_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_common_load_otp_info_from_sdcard(cmd->otp_info, &(cmd->otp_info_len));
        if (GF_SUCCESS == err)
        {
            memcpy(otp_info_tmp, cmd->otp_info, cmd->otp_info_len);
        }

        cmd->download_fw_flag = download_fw_flag;
        err = gf_hal_invoke_command(GF_CMD_INIT, cmd, size);

        if (GF_ERROR_SPI_FW_DOWNLOAD_FAILED == err)
        {
            LOG_E(LOG_TAG, "[%s] download fw_cfg failed first trial", __func__);
            err = hal_milan_an_series_download_fwcfg();
        }

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

        if (0 != memcmp(otp_info_tmp, cmd->otp_info, cmd->otp_info_len))
        {
            LOG_D(LOG_TAG, "[%s] init finished , save otp_info", __func__);
            gf_hal_common_save_otp_info_into_sdcard(cmd->otp_info, cmd->otp_info_len);
        }

        g_esd_check_flag = cmd->esd_check_flag;
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
        err = irq_polling();  // bn boot start, and then update base should poll irq
    }  // do init
    while (0);

    if (NULL != cmd)
    {
        free(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: gf_error_t hal_milan_an_series_close
 * Description: the funtion to close
 * Input: the pointer to the dev struct
 * Output: the error status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_close(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_destroy_timer(&g_esd_timer_id);
    err = gf_hal_common_close(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_cancel
 * Description: the function to cancel some specific operation
 * Input: the pointer to the dev struct
 * Output: the error status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        err = gf_hal_common_cancel(dev);
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
 * Function: hal_milan_an_series_test_cancel
 * Description: the function to test cancelling operation
 * Input: the pointer to the dev struct
 * Output: the error status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_test_cancel(void *dev)
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
 * Function: hal_milan_an_series_test_prior_cancel
 * Description: the function to test prior cancel
 * Input: the pointer to dev struct
 * Output: the error status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_test_prior_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
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
 * Function: hal_milan_an_series_pre_enroll
 * Description: the function to execute pre enroll operations
 * Input: the pointer to the dev struct
 * Output: the error status
 * Return:
 * Others:
 */

static uint64_t hal_milan_an_series_pre_enroll(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_pre_enroll(dev);
    VOID_FUNC_EXIT();
    return ret;
}

/*
 * Function: hal_milan_an_series_enroll
 * Description: the function to enroll
 * Input: the pointer to dev struct, the pointer to hat, the group_id and the timeout
 * Output: the error status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_enroll(void *dev, const void *hat,
                                             uint32_t group_id,
                                             uint32_t timeout_sec)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enroll(dev, hat, group_id, timeout_sec);
    FUNC_EXIT(err);
    return err;
}


/*
 * Function: hal_milan_an_series_post_enroll
 * Description: the function to execute post enroll operation
 * Input: the pointer to dev struct
 * Output: the error status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_post_enroll(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_post_enroll(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_authenticate
 * Description: the function to let the chip to preparing for authenticating
 * Input: the pointer to dev struct, the operation id, and the fingerprint group id
 * Output: the errot status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_authenticate(void *dev,
                                                   uint64_t operation_id,
                                                   uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_authenticate(dev, operation_id, group_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_get_auth_id
 * Description: the function to get authentication id
 * Input: the pointer to dev struct
 * Output: the authentication id
 * Return:
 * Others:
 */

static uint64_t hal_milan_an_series_get_auth_id(void *dev)
{
    uint64_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_get_auth_id(dev);
    VOID_FUNC_EXIT();
    return ret;
}

/*
 * Function: hal_milan_an_series_remove
 * Description: the function to remove a specific fingerprint
 * Input: the pointer to dev struc, the fingerprint group id, and the fingerprint id
 * Output: the error status
 * Return:
 * Others:
*/

static gf_error_t hal_milan_an_series_remove(void *dev, uint32_t group_id,
                                             uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_remove(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_set_active_group
 * Description: the function to set active group
 * Input: the pointer to dev and fingerprint group id
 * Output: the error status
 * Return:
 * Others:
 */

static gf_error_t hal_milan_an_series_set_active_group(void *dev,
                                                       uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_active_group(dev, group_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_enumerate
 * Description: the function to enumerate fingerprint templates
 * Input: the pointer to dev
 * Output: the result set and set size
 * Return: the error status
 * Others:
 */

static gf_error_t hal_milan_an_series_enumerate(void *dev, void *results,
                                                uint32_t *max_size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enumerate(dev, results, max_size);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_enumerate_with_callback
 * Description: the function to enumerate fingerprint templates in synchronous mode
 * Input: the pointer to dev struct
 * Output:
 * Return: the error status
 * Others:
 */

static gf_error_t hal_milan_an_series_enumerate_with_callback(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enumerate_with_callback(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: irq_enroll_or_authenticate
 * Description: the callback for receiving irq of enroll or authenticate
 * Input: the cmd and err code
 * Output:
 * Return: void
 * Others:
 */

static void irq_enroll_or_authenticate(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            irq_down_enroll_or_authenticate(cmd, err_code);
        }

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))
        {
            irq_image_enroll_or_authenticate(cmd, err_code);
            g_match_fail_and_retry = GF_ERROR_MATCH_FAIL_AND_RETRY == *err_code ? 1 : 0;
            g_preprocess_fail_and_retry =  GF_ERROR_PREPROCESS_FAIL_AND_RETRY == *err_code ? 1 : 0;
            g_crc_check_fail_and_retry = GF_ERROR_CRC_CHECK_FAILED_AND_RETRY == *err_code ? 1 : 0;
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
        {
            irq_up_enroll_or_authenticate(cmd);
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_an_series_authenticate_finish
 * Description: callback when authetication is finished
 * Input: pointer to struct containing irq info
 * Output:
 * Return: error status
 * Others:
 */

static gf_error_t hal_milan_an_series_authenticate_finish(gf_irq_t *buffer)
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

        buffer->save_flag = cmd->save_flag;
        buffer->update_stitch_flag = cmd->update_stitch_flag;
    }
    while (0);

    if (NULL != cmd)
    {
        free(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);

    return err;
}

/*
 * Function: gf_hal_milan_an_authenticate_success
 * Description: callback for authentication success
 * Input: the pointer to the struct containing irq info and error code
 * Output:
 * Return:
 * Others:
 */

static void gf_hal_milan_an_authenticate_success(gf_irq_t *cmd, gf_error_t *err_code)
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

        gf_hal_notify_authentication_succeeded(cmd->group_id, cmd->finger_id,
                                               &cmd->auth_token);
        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
              cmd->finger_id);

        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
        *err_code = hal_milan_an_series_authenticate_finish(cmd);

        LOG_D(LOG_TAG, "[%s] save_flag=%u, authenticate_update_flag = %u",
                __func__, cmd->save_flag, cmd->update_stitch_flag);

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(*err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }

        g_failed_attempts = 0;
    }  // do the main procedure on authenticate success
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: gf_hal_milan_an_authenticate_fido_success
 * Description: callback for authentication success of fido
 * Input: the pointer to the struct containing irq info and error code
 * Output:
 * Return:
 * Others:
 */

static void gf_hal_milan_an_authenticate_fido_success(gf_irq_t *cmd,
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
        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
        *err_code = hal_milan_an_series_authenticate_finish(cmd);

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(*err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }

        g_failed_attempts = 0;
    }  // do the main procedure on authenticate fido success
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_down_enroll_or_authenticate
 * Description: the irq callback for down event of enroll or authenticate
 * Input: the struct containing irq info and error code
 * Output:
 * Return:
 * Others:
 */

static void irq_down_enroll_or_authenticate(gf_irq_t *cmd,
                                            gf_error_t *err_code)
{
    uint32_t size = sizeof(gf_irq_t);
    UNUSED_VAR(size);  // used for polling C0
    VOID_FUNC_ENTER();

    do
    {
        if (0 == g_hal_config.report_key_event_only_enroll_authenticate
            || OPERATION_ENROLL == cmd->operation
            || OPERATION_AUTHENTICATE_FF == cmd->operation
            || OPERATION_AUTHENTICATE_IMAGE == cmd->operation
            || OPERATION_AUTHENTICATE_FIDO == cmd->operation)
        {
            g_match_fail_and_retry = 0;
            g_preprocess_fail_and_retry = 0;
            g_crc_check_fail_and_retry = 0;
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
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                /* FALL THROUGH */
            }

            case OPERATION_AUTHENTICATE_IMAGE:
            {
                /* FALL THROUGH */
            }

            case OPERATION_AUTHENTICATE_FIDO:
            {
                //  do not polling C0 when recive home_key_down ,because the firmware do not support for now
                /*
                if (g_hal_config.support_set_spi_speed_in_tee > 0)
                {
                    LOG_D(LOG_TAG, "[%s] start polling", __func__);
                    cmd->step = GF_IRQ_STEP_POLLING;
                    gf_hal_disable_irq();
                    *err_code = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);
                    gf_hal_enable_irq();
                }
                */
                break;
            }

            default:
            {
                break;
            }
        }  // switch (cmd->operation)
    }  // do the main procedure for irq_down_enroll or authenticate
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_image_enroll_or_authenticate
 * Description: irq callback to hanel image irq of enroll or authenticate
 * Input:  the struct containing irq info and error code
 * Output:
 * Return:
 * Others:
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
                gf_hal_milan_an_authenticate_success(cmd, err_code);
            }
            else if (OPERATION_AUTHENTICATE_FIDO == cmd->operation)
            {
                gf_hal_milan_an_authenticate_fido_success(cmd, err_code);
            }

            break;
        }

        case GF_ERROR_SENSOR_IS_BROKEN:
        {
            sensor_is_broken();
            break;
        }

        case GF_ERROR_ALGO_DIRTY_FINGER:
        {
            /* FALL THROUGH */
        }

        case GF_ERROR_ACQUIRED_IMAGER_DIRTY:
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK image quality is too low", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
            break;
        }

        case GF_ERROR_SENSOR_BROKEN_CHECK_ALGO_ERROR:
        {
            /* FALL THROUGH */
        }

        case GF_ERROR_ACQUIRED_PARTIAL:
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK valid area is too low", __func__);
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_PARTIAL);
            break;
        }

        case GF_ERROR_INVALID_FINGER_PRESS:
        {
            /* FALL THROUGH */
        }

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

        case GF_ERROR_TOO_FAST:
        {
            gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
            LOG_D(LOG_TAG, "[%s] finger lift too fast", __func__);
            break;
        }

        case GF_ERROR_NOT_MATCH:
        {
            /* FALL THROUGH */
        }

        case GF_ERROR_BIO_ASSAY_FAIL:
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

        case GF_ERROR_PREPROCESS_FAIL_AND_RETRY:
        {
            LOG_I(LOG_TAG, "[%s] authenticate preprocess fail and retry", __func__);
            break;
        }

        case GF_ERROR_CRC_CHECK_FAILED_AND_RETRY:
        {
            if (OPERATION_AUTHENTICATE_IMAGE == cmd->operation
                || OPERATION_AUTHENTICATE_FF == cmd->operation)
            {
                LOG_I(LOG_TAG, "[%s] crc check failed and retry, err=%s, errno=%d", __func__,
                    gf_strerror(*err_code), *err_code);
            }
            break;
        }

        case GF_ERROR_INVALID_DATA:
        {
            if (OPERATION_AUTHENTICATE_IMAGE == cmd->operation
                || OPERATION_AUTHENTICATE_FF == cmd->operation
                ||OPERATION_AUTHENTICATE_FIDO == cmd->operation)
            {
                gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_INVALID_DATA);
            }
            break;
        }

        case GF_ERROR_DYNAMIC_ERNOLL_INCOMPLETE_TEMPLATE:
        {
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_INCOMPLETE_TEMPLATE);
            LOG_D(LOG_TAG, "[%s] incomplete_template", __func__);
            gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            break;
        }

        case GF_ERROR_DYNAMIC_ENROLL_INVALID_PRESS_TOO_MUCH:
        {
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_INVALID_PRESS_TOO_MUCH);
            LOG_D(LOG_TAG, "[%s] continous invalid press", __func__);
            gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
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
 * Function: sensor_is_broken
 * Description: the function to handle the situation when sensor is broken
 * Input:
 * Output:
 * Return:
 * Others:
*/

static void sensor_is_broken()
{
    VOID_FUNC_ENTER();

    do
    {
        g_enroll_invalid_template_num++;
        LOG_D(LOG_TAG, "[%s] broken checked, invalid template count=%u", __func__,
              g_enroll_invalid_template_num);

        if (g_enroll_invalid_template_num >= (g_hal_config.enrolling_min_templates >> 1))
        {
            LOG_D(LOG_TAG, "[%s] broken checked, discard this enrolling", __func__);
            g_enroll_invalid_template_num = 0;
            gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_HW_UNAVAILABLE);
            hal_milan_an_series_cancel(g_fingerprint_device);
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
 * Function: irq_up_enroll_or_authenticate
 * Description: the irq callback for up event of enroll or authenticate
 * Input: the struct containing irq info
 * Output:
 * Return:
 * Others:
 */

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

            if (1 == g_match_fail_and_retry || 1 ==  g_preprocess_fail_and_retry)
            {
                g_match_fail_and_retry = 0;
                g_preprocess_fail_and_retry = 0;
            }
            if (1 == g_crc_check_fail_and_retry)
            {
                g_crc_check_fail_and_retry = 0;
                LOG_D(LOG_TAG, "[%s] add a notify of crc failed in retry when finger up", __func__);
                gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_INVALID_DATA);
            }

            gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
        }  // else if (cmd->report_authenticate_fail_flag > 0)

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
    }  // do the main procedure for irq_up_enroll or authenticate
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_polling
 * Description: poll a valid irq in TA
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t irq_polling()
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
        if (g_hal_config.support_set_spi_speed_in_tee > 0)
        {
            LOG_D(LOG_TAG, "[%s] start polling", __func__);
            cmd->step = GF_IRQ_STEP_POLLING;
            gf_hal_disable_irq();
            err = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);
            gf_hal_enable_irq();
        }
        else
        {
            LOG_I(LOG_TAG, "[%s] boot start and wait for image irq on debug mode", __func__);
        }
    }
    while (0);

    if (cmd != NULL)
    {
        free(cmd);
        cmd = NULL;
    }


    FUNC_EXIT(err);
    return err;
}

/*
 * Function: irq_pre_process
 * Description: the procedure to be executed prior to process irq
 * Input: the struct containing irq info
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t irq_pre_process(gf_irq_t *cmd)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t size = sizeof(gf_irq_t);
    FUNC_ENTER();

    do
    {
        g_irq_time_milan_an = gf_hal_current_time_microsecond();
        cmd->auth_token.version = GF_HW_AUTH_TOKEN_VERSION;
        cmd->auth_token.authenticator_type = htobe32(GF_HW_AUTH_FINGERPRINT);
        cmd->auth_token.timestamp = 0;
        cmd->uvt.uvt_len = MAX_UVT_LEN;
        cmd->step = GF_IRQ_STEP_GET_IRQ_TYPE;
        err = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);

        if (OPERATION_NAV == cmd->operation && GF_IRQ_FINGER_DOWN_MASK == cmd->irq_type)
        {
            g_nav_times++;
            LOG_D(LOG_TAG, "[%s] nav result : %d", __func__, cmd->nav_result.nav_code);
            gf_hal_nav_listener(cmd->nav_result.nav_code);
        }

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

        LOG_I(LOG_TAG, "[%s] irq_type=%s, irq_type=0x%08X, operation=%s", __func__,
              gf_strirq(cmd->irq_type), cmd->irq_type, gf_stroperation(cmd->operation));

        if ((cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_DOWN_MASK", __func__);
            g_down_irq_time = g_irq_time_milan_an;
            if (OPERATION_TEST_FPC_KEY_DETECT == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_RING_FINGER_DOWN", __func__);
                hal_notify_test_fpc_detected(err, GF_KEY_HOME, GF_KEY_STATUS_DOWN);
            }
        }

        if ((cmd->irq_type & GF_IRQ_IMAGE_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK", __func__);
            g_image_irq_time = g_irq_time_milan_an;
        }

        if ((cmd->irq_type & GF_IRQ_FINGER_UP_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_UP_MASK", __func__);
            if (OPERATION_TEST_FPC_KEY_DETECT == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_RING_FINGER_UP", __func__);
                hal_notify_test_fpc_detected(err, GF_KEY_HOME, GF_KEY_STATUS_UP);
            }
        }

        if ((cmd->irq_type & GF_IRQ_RESET_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_RESET_MASK", __func__);
            if (OPERATION_TEST_RESET_PIN != cmd->operation && OPERATION_TEST_FPC_KEY_DETECT != cmd->operation)
            {
                uint8_t i = 0;
                for (i = 0; i < 4; i++)
                {
                    err = hal_milan_an_series_download_fwcfg();
                    LOG_E(LOG_TAG, "[%s] download fw_cfg trial time=%d, code=%s",
                                    __func__, i, gf_strerror(err) );
                    if (GF_SUCCESS == err)
                    {
                        break;
                    }
                }

                if (err != GF_SUCCESS)
                {
                    LOG_E(LOG_TAG, "[%s] fw download failed", __func__);
                    break;
                }
            }
        }

        if ((cmd->irq_type & GF_IRQ_MENUKEY_DOWN_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_MENUKEY_DOWN_MASK", __func__);
            if (OPERATION_TEST_FPC_KEY_DETECT == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_MENUKEY_DOWN", __func__);
                hal_notify_test_fpc_detected(err, GF_KEY_MENU, GF_KEY_STATUS_DOWN);
            } else {
                gf_hal_send_key_event(GF_KEY_MENU, GF_KEY_STATUS_DOWN);
            }
        }
        else if ((cmd->irq_type & GF_IRQ_MENUKEY_UP_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_MENUKEY_UP_MASK", __func__);
            if (OPERATION_TEST_FPC_KEY_DETECT == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_MENUKEY_UP", __func__);
                hal_notify_test_fpc_detected(err, GF_KEY_MENU, GF_KEY_STATUS_UP);
            } else {
                gf_hal_send_key_event(GF_KEY_MENU, GF_KEY_STATUS_UP);
            }
        }

        if ((cmd->irq_type & GF_IRQ_BACKKEY_DOWN_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_BACKKEY_DOWN_MASK", __func__);
            if (OPERATION_TEST_FPC_KEY_DETECT == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BACKKEY_DOWN", __func__);
                hal_notify_test_fpc_detected(err, GF_KEY_BACK, GF_KEY_STATUS_DOWN);
            } else {
                gf_hal_send_key_event(GF_KEY_BACK, GF_KEY_STATUS_DOWN);
            }
        }
        else if ((cmd->irq_type & GF_IRQ_BACKKEY_UP_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_BACKKEY_UP_MASK", __func__);
            if (OPERATION_TEST_FPC_KEY_DETECT == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BACKKEY_UP", __func__);
                hal_notify_test_fpc_detected(err, GF_KEY_BACK, GF_KEY_STATUS_UP);
            } else {
                gf_hal_send_key_event(GF_KEY_BACK, GF_KEY_STATUS_UP);
            }
        }

        if ((cmd->irq_type & GF_IRQ_GSC_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_GSC_MASK", __func__);
        }

        if ((cmd->irq_type & GF_IRQ_FW_ERR_MASK)
            || (cmd->irq_type & GF_IRQ_CFG_ERR_MASK)
            || (cmd->irq_type & GF_IRQ_ESD_ERR_MASK))
        {
            LOG_D(LOG_TAG, "[%s] fw, cfg or esd error, need to download fw & cfg",
                  __func__);

            err = hal_milan_an_series_download_fwcfg();

            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] fw download failed", __func__);
                break;
            }
        }

        if ((cmd->irq_type & GF_IRQ_UPDATE_BASE_MASK) != 0)
        {
            LOG_D(LOG_TAG, "[%s] GF_IRQ_UPDATE_BASE_MASK, need update base", __func__);
        }
    }  // do the main procedure for irq_pre_process
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: irq_process
 * Description: the procedure to be executed to handle irq
 * Input: the struct containing irq info and error code
 * Output:
 * Return:
 * Others:
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
            {
                /* FALL THROUGH */
            }

            case OPERATION_CAMERA_KEY:
            {
                /* FALL THROUGH */
            }

            case OPERATION_POWER_KEY:
            {
                gf_hal_common_irq_key(cmd);
                break;
            }

            case OPERATION_ENROLL:
            {
                /* FALL THROUGH */
            }

            case OPERATION_AUTHENTICATE_IMAGE:
            {
                /* FALL THROUGH */
            }

            case OPERATION_AUTHENTICATE_FF:
            {
                /* FALL THROUGH */
            }

            case OPERATION_AUTHENTICATE_FIDO:
            {
                irq_enroll_or_authenticate(cmd, err_code);
                break;
            }

            case OPERATION_NAV:
            {
                // irq_nav(cmd);
                break;
            }

            case OPERATION_FINGER_BASE:
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_FINGER_BASE", __func__);
                break;
            }

            default:
            {
                hal_milan_an_series_irq_test(cmd, err_code);
                break;
            }
        }  // switch (cmd->operation)
    }  // do the procedure for irq_process
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: irq_post_process
 * Description: the procedure to be executed after irq processing
 * Input: the struct containing irq info and error code
 * Output:
 * Return:
 * Others:
 */
static void irq_post_process(gf_irq_t *cmd, gf_error_t error_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL != cmd)
        {
            if (cmd->irq_type & GF_IRQ_IMAGE_MASK ||
                (OPERATION_NAV == cmd->operation && GF_IRQ_FINGER_DOWN_MASK == cmd->irq_type))
            {
                gf_hal_dump_data_by_operation(cmd->operation, error_code);
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: hal_milan_an_series_irq
 * Description: function to handle irq
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */
gf_error_t hal_milan_an_series_irq()
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
    }
    while (0);

    if (cmd != NULL)
    {
        free(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_screen_on
 * Description: the function on situation of screen on
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */

static gf_error_t hal_milan_an_series_screen_on()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_screen_on();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_screen_off
 * Description: the function on situation of screen off
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_screen_off()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_screen_off();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_set_safe_class
 * Description: the function to set safe class
 * Input: the pointer to dev struct and safe class
 * Output:
 * Return: the error code
 * Others:
 */
static gf_error_t hal_milan_an_series_set_safe_class(void *dev,
                                                     gf_safe_class_t safe_class)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_set_safe_class(dev, safe_class);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_navigate
 * Description: the function to enter navigate mode
 * Input: the pointer to dev struct and navigating mode
 * Output:
 * Return: the error code
 * Others:
 */
static gf_error_t hal_milan_an_series_navigate(void *dev,
                                               gf_nav_mode_t nav_mode)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_navigate(dev, nav_mode);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_enable_fingerprint_module
 * Description: the function to enable fingerprint module
 * Input: the pointer to dev struct and navigating mode
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_enable_fingerprint_module(void *dev,
        uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_fingerprint_module(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_camera_capture
 * Description: the function to do camera capture
 * Input: the pointer to dev struct
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_camera_capture(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_camera_capture(dev);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_enable_ff_feature
 * Description: the function to enable/disable ff feature
 * Input: the pointer to dev struct and the flag for enabling or not
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_enable_ff_feature(void *dev,
                                                        uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_enable_ff_feature(dev, enable_flag);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_enable_bio_assay_feature
 * Description: the function to enable/disable bio assay feature
 * Input: the pointer to dev struct and the flag for enabling or not
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_enable_bio_assay_feature(void *dev,
        uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_set_config_t *cmd = NULL;
    uint32_t size = sizeof(gf_test_set_config_t);
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        cmd = (gf_test_set_config_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        g_hal_config.support_bio_assay = enable_flag;
        memcpy(&cmd->config, &g_hal_config, sizeof(gf_config_t));
        err = gf_hal_test_invoke_command(GF_CMD_TEST_SET_CONFIG, cmd, size);
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
 * Function: hal_milan_an_series_sync_finger_list
 * Description: the function to sync fingerprint list between app level and ta
 * Input: the pointer to dev and fingerprint list and its count from app level
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_sync_finger_list(void *dev,
                                                       uint32_t *list,
                                                       int32_t count)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_sync_finger_list(dev, list, count);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: gf_hal_check_esd
 * Description: the function to check esd
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t gf_hal_check_esd(void)
{
    gf_error_t err = GF_SUCCESS;

    gf_esd_check_t *esd_cmd = NULL;
    uint32_t size = sizeof(gf_esd_check_t);
    uint64_t delta = 0;
    uint64_t current_time = 0;
    static uint64_t last_esd_time = 0;

    FUNC_ENTER();

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        esd_cmd = (gf_esd_check_t *) malloc(size);
        if (NULL == esd_cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        // keep esd check time span when switch mode
        current_time = (uint64_t) gf_hal_current_time_microsecond();
        delta = (uint64_t) (current_time - last_esd_time) / 1000;
        if (delta < GF_ESD_TIME_SPAN_MS)
        {
            LOG_D(LOG_TAG, "[%s] keep esd check time span greater than %u ms",
                __func__, GF_ESD_TIME_SPAN_MS);
            break;
        }
        last_esd_time = current_time;

        memset(esd_cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_ESD_CHECK, esd_cmd, size);
        if (err != GF_SUCCESS)
        {
            break;
        }

        if (esd_cmd->download_fw_flag > 0)
        {
            err = gf_hal_download_fw();

            if (err != GF_SUCCESS || GF_MILAN_A_SERIES == g_hal_config.chip_series
                || GF_MILAN_AN_SERIES == g_hal_config.chip_series)
            {
                break;
            }
        }

        if (esd_cmd->cmd_header.reset_flag > 0)
        {
            LOG_D(LOG_TAG, "[%s] failed", __func__);
            err = gf_hal_download_cfg();
            if (err != GF_SUCCESS)
            {
                break;
            }
        }
    }  // do the main procedure for gf_hal_check_esd
    while (0);

    if (NULL != esd_cmd)
    {
        free(esd_cmd);
        esd_cmd = NULL;
    }

    FUNC_EXIT(err);

    return err;
}

/*
 * Function: hal_milan_an_series_invoke_command
 * Description: the function to invoke command into TA
 * Input: the operation id, command id, extra info and length
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_invoke_command(uint32_t operation_id,
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

        if (1 == g_fpc_config_had_downloaded && \
            cmd_id != GF_CMD_TEST_FPC_KEY_DETECT && \
            cmd_id != GF_CMD_TEST_DOWNLOAD_CFG && \
            cmd_id != GF_CMD_IRQ && \
            cmd_id != GF_CMD_DOWNLOAD_FW && \
            cmd_id != GF_CMD_DOWNLOAD_CFG && \
            cmd_id != GF_CMD_ESD_CHECK)
        {
            gf_cancel_t *cmd_temp = NULL;
            uint32_t size_temp = sizeof(gf_cancel_t);

            LOG_E(LOG_TAG, "[%s] need reset fw/cfg, cmd_id = %d", __func__, cmd_id);
            err = hal_milan_an_series_reset_fwcfg();
            if (GF_SUCCESS != err)
            {
                LOG_D(LOG_TAG, "[%s] reset fail when fpc config has downloaded", __func__);
                break;
            }
            g_fpc_config_had_downloaded = 0;

            cmd_temp = (gf_cancel_t *) malloc(size_temp);
            if (NULL == cmd_temp)
            {
                LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(cmd_temp, 0, size_temp);
            gf_hal_control_spi_clock(1);
            err = gf_ca_invoke_command(operation_id, GF_CMD_TEST_CANCEL, cmd_temp, size_temp);
            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] ca invoke command GF_CMD_TEST_CANCEL failed, errno=%d", __func__, err);
            }
            gf_hal_control_spi_clock(0);

            if (NULL != cmd_temp)
            {
                free(cmd_temp);
                cmd_temp = NULL;
            }
        }  // if sentence tail

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
            if (MODE_SLEEP != cmd->mode && MODE_FF != cmd->mode)
            {
                gf_hal_create_and_set_esd_timer();
            }
        }

        if (cmd->reset_flag > 0)
        {
            LOG_D(LOG_TAG, "[%s] reset_flag > 0, reset chip", __func__);
            gf_hal_destroy_timer(&g_esd_timer_id);
            gf_hal_reset_chip();
            if (MODE_SLEEP != cmd->mode && MODE_FF != cmd->mode)
            {
                gf_hal_create_and_set_esd_timer();
            }
        }

        pthread_mutex_unlock(&g_sensor_mutex);

        if (cmd->reset_flag <= 0)
        {
            if ((g_mode != MODE_SLEEP && g_mode != MODE_FF)
                && (MODE_SLEEP == cmd->mode || MODE_FF == cmd->mode))
            {
                gf_hal_destroy_timer(&g_esd_timer_id);
            }
            else if ((MODE_SLEEP == g_mode || MODE_FF == g_mode)
                     && (cmd->mode != MODE_SLEEP && cmd->mode != MODE_FF))
            {
                gf_hal_create_and_set_esd_timer();
            }
        }

        if (GF_CMD_IRQ != cmd_id && GF_CHECK_MODE_SWITCH == g_esd_check_flag)
        {
            if (GF_CMD_ESD_CHECK != cmd_id && g_mode != cmd->mode
                && (MODE_KEY == cmd->mode || MODE_IMAGE == cmd->mode
                    || MODE_NAV == cmd->mode || MODE_FF == cmd->mode))
            {
                gf_hal_check_esd();
            }
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
    }  // do the main procedure when invoke command
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_pause_enroll
 * Description: the function to pause enroll
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_pause_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_pause_enroll();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_resume_enroll
 * Description: the function to resume enroll
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_resume_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_resume_enroll();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_user_invoke_command
 * Description: the function to do user invoke command
 * Input: the command id, the extra info buffer and its length
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_user_invoke_command(uint32_t cmd_id,
                                                          void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_user_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_dump_invoke_command
 * Description: the function to invoke command with dump on
 * Input: the command id, extra info buffer and its length
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_dump_invoke_command(uint32_t cmd_id,
                                                          void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_dump_invoke_command(cmd_id, buffer, len);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_authenticate_fido
 * Description: the function to do authenticate of fido
 * Input:
 * Output: the error status
 * Return:
 * Others:
 */
static gf_error_t hal_milan_an_series_authenticate_fido(void *dev,
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
 * Function: hal_milan_an_series_is_id_valid
 * Description: check whether the pair of group id and finger id is valid
 * Input: the pointer to dev, the group id and finger id
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_is_id_valid(void *dev, uint32_t group_id,
                                                  uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_is_id_valid(dev, group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_get_id_list
 * Description: get the fingerprint id list
 * Input: the pointer to dev, the group id, the pointer to the list used to collect
       fingerprint and the count
 * Output:
 * Return: the return value
 * Others:
 */
static int32_t hal_milan_an_series_get_id_list(void *dev, uint32_t group_id,
                                               uint32_t *list, int32_t *count)
{
    int32_t ret = 0;
    VOID_FUNC_ENTER();
    ret = gf_hal_common_get_id_list(dev, group_id, list, count);
    VOID_FUNC_EXIT();
    return ret;
}

/*
 * Function: hal_milan_an_series_reset_lockout
 * Description: the function to get the device out from the lockout status
 * Input:
 * Output:
 * Return: the error status
 * Others:
 */
static gf_error_t hal_milan_an_series_reset_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_reset_lockout();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_lockout
 * Description: the function to lock the device
 * Input:
 * Output:
 * Return: error status
 * Others:
 */
static gf_error_t hal_milan_an_series_lockout()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_common_lockout();
    FUNC_EXIT(err);
    return err;
}

/*
 * Function: hal_milan_an_series_reset_fwcfg
 * Description: the function to reset fireware config
 * Input:
 * Output:
 * Return: error status
 * Others:
 */
static gf_error_t hal_milan_an_series_reset_fwcfg()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        gf_hal_reset_chip();
        err = gf_hal_download_fw();

        if (GF_SUCCESS != err)
        {
            break;
        }
    }
    while (0);

    hal_notify_test_fpc_reset_fwcfg(err);

    FUNC_EXIT(err);
    return err;
}

/*
 * Function: gf_hal_milan_an_series_function_customize
 * Description: configure the hal functions with milan an series ones
 * Input: the pointer to the struct of hal functions
 * Output:
 * Return: error status
 * Others:
 */
gf_error_t gf_hal_milan_an_series_function_customize(gf_hal_function_t
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

        hal_function->init = hal_milan_an_series_init;
        hal_function->close = hal_milan_an_series_close;
        hal_function->cancel = hal_milan_an_series_cancel;
        hal_function->test_cancel = hal_milan_an_series_test_cancel;
        hal_function->test_prior_cancel = hal_milan_an_series_test_prior_cancel;
        hal_function->pre_enroll = hal_milan_an_series_pre_enroll;
        hal_function->enroll = hal_milan_an_series_enroll;
        hal_function->post_enroll = hal_milan_an_series_post_enroll;
        hal_function->authenticate = hal_milan_an_series_authenticate;
        hal_function->get_auth_id = hal_milan_an_series_get_auth_id;
        hal_function->remove = hal_milan_an_series_remove;
        hal_function->set_active_group = hal_milan_an_series_set_active_group;
        hal_function->enumerate = hal_milan_an_series_enumerate;
        hal_function->enumerate_with_callback =
            hal_milan_an_series_enumerate_with_callback;
        hal_function->irq = hal_milan_an_series_irq;
        hal_function->screen_on = hal_milan_an_series_screen_on;
        hal_function->screen_off = hal_milan_an_series_screen_off;
        hal_function->set_safe_class = hal_milan_an_series_set_safe_class;
        hal_function->navigate = hal_milan_an_series_navigate;
        hal_function->enable_fingerprint_module =
            hal_milan_an_series_enable_fingerprint_module;
        hal_function->camera_capture = hal_milan_an_series_camera_capture;
        hal_function->enable_ff_feature = hal_milan_an_series_enable_ff_feature;
        hal_function->enable_bio_assay_feature =
            hal_milan_an_series_enable_bio_assay_feature;
        hal_function->reset_lockout = hal_milan_an_series_reset_lockout;
        hal_function->sync_finger_list = hal_milan_an_series_sync_finger_list;
        hal_function->invoke_command = hal_milan_an_series_invoke_command;
        hal_function->user_invoke_command = hal_milan_an_series_user_invoke_command;
        hal_function->dump_invoke_command = hal_milan_an_series_dump_invoke_command;
        hal_function->authenticate_fido = hal_milan_an_series_authenticate_fido;
        hal_function->is_id_valid = hal_milan_an_series_is_id_valid;
        hal_function->get_id_list = hal_milan_an_series_get_id_list;
        hal_function->lockout = hal_milan_an_series_lockout;
        hal_function->pause_enroll = hal_milan_an_series_pause_enroll;
        hal_function->resume_enroll = hal_milan_an_series_resume_enroll;
        hal_function->dump_chip_operation_data = hal_milan_an_dump_chip_operation_data;
    }  // do block for regitering callbacks
    while (0);

    FUNC_EXIT(err);
    return err;
}

