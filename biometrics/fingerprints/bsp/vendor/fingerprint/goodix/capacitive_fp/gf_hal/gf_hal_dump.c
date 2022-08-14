/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "gf_ca_entry.h"
#include "gf_hal_device.h"
#include "gf_hal_common.h"
#include "gf_hal.h"
#include "gf_hal_log.h"
#include "gf_hal_timer.h"
#include "gf_hal_dump.h"
#include "gf_hal_test_utils.h"
#include "gf_dump_data.h"
#include "gf_common.h"
#include "gf_aes.h"
#include "gf_fingerprint.h"
#include "gf_dump_data_utils.h"
#include "gf_dump_data_encoder.h"

#define LOG_TAG "[GF_HAL][gf_hal_dump]"


/**
 * Function: hal_dump_nav_base
 * Description: Dump navigation base data.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_dump_nav_base()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_dump_data_by_operation(OPERATION_NAV_BASE, err);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_dump_finger_base
 * Description: Dump finger base data.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_dump_finger_base()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_hal_dump_data_by_operation(OPERATION_FINGER_BASE, err);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_enable_dump_data
 * Description: Enable dump feature.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_enable_dump_data()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_enable_dump();
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: get_int32_value
 * Description: parse int32 value got from test cmd.
 * Input: buf, cmd data
 * Output: None
 * Return: int32_t value
 */
static int32_t get_int32_value(const uint8_t* buf)
{
    return buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
}

/**
 * Function: hal_set_dump_config
 * Description: set dump config
 * Input: param, parameter data; param_len, param len
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_set_dump_config(const uint8_t *param,
                           uint32_t param_len)
{
    gf_error_t err = GF_SUCCESS;
    const uint8_t* buf = param;
    gf_dump_config_t cfg;
    uint32_t size = sizeof(int32_t);
    int32_t value = 0;
    int32_t i = 0;
    FUNC_ENTER();
    do
    {
        if (NULL == buf)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        memset(&cfg, 0, sizeof(gf_dump_config_t));
        cfg.dump_enabled = get_int32_value(buf) ? 1 : 0;
        buf += size;
        cfg.dump_encrypt_enabled = get_int32_value(buf) ? 1 : 0;
        buf += size;
        cfg.dump_big_data_enabled = get_int32_value(buf) ? 1 : 0;
        buf += size;
        value = get_int32_value(buf);
        if (value < DUMP_PATH_SDCARD || value > DUMP_PATH_DATA)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        cfg.dump_path = value;
        for (i = 0; i < DUMP_OP_NUM; i++)
        {
            buf += size;
            cfg.dump_operation[i].result = get_int32_value(buf);
            buf += size;
            cfg.dump_operation[i].data_type = get_int32_value(buf);
        }
        gf_set_dump_config(&cfg);
    }  // do...
    while (0);
    FUNC_EXIT(err);
    return err;
}


/**
 * Function: hal_enable_dump_data
 * Description: Disable dump feature.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_disable_dump_data()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_disable_dump();
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_dump_templates
 * Description: Dump all of the templates.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_dump_templates()
{
    gf_error_t err = GF_SUCCESS;
    dump_data_encoder_t* data_encoder = NULL;
    gf_user_enumerate_t *enumerate = NULL;
    gf_dump_template_t *template = NULL;
    uint32_t enumerate_size = sizeof(gf_user_enumerate_t);
    uint32_t i = 0;
    uint32_t template_size = sizeof(gf_dump_template_t);
    struct timeval tv = { 0 };
    int64_t timestamp = 0;
    FUNC_ENTER();

    do
    {
        // check dump template config
        if (gf_is_dump_enabled() == 0 || gf_is_dump_template_allowed() == 0)
        {
            LOG_D(LOG_TAG, "[%s] dump template not allowed", __func__);
            break;
        }
        enumerate = (gf_user_enumerate_t *) malloc(enumerate_size);

        if (NULL == enumerate)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, enumerate", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        err = g_hal_function.user_invoke_command(GF_USER_CMD_ENUMERATE, enumerate,
                                                 enumerate_size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        LOG_D(LOG_TAG, "[%s] finger count=%u", __func__, enumerate->size);
        template = (gf_dump_template_t *) malloc(template_size);

        if (NULL == template)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, template", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        gettimeofday(&tv, NULL);
        timestamp = gf_get_time_stamp(&tv);
        for (i = 0; i < enumerate->size; i++)
        {
            LOG_D(LOG_TAG, "[%s] finger_id[%d]=%u", __func__, i, enumerate->finger_ids[i]);
            memset(template, 0, template_size);
            template->group_id = enumerate->group_ids[i];
            template->finger_id = enumerate->finger_ids[i];
            err = g_hal_function.dump_invoke_command(GF_CMD_DUMP_TEMPLATE, template,
                                                     template_size);
            if (err != GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] get finger_id[%d] data err", __func__, i);
                continue;
            }

            if (data_encoder != NULL)
            {
                gf_dump_encoder_reset(data_encoder);
            }
            else
            {
                err = gf_dump_encoder_create(&data_encoder, timestamp);
                if (err != GF_SUCCESS)
                {
                    break;
                }
            }
            err = gf_dump_template(data_encoder, template, &tv);
            if (err != GF_SUCCESS)
            {
                break;
            }
            err = gf_handle_dump_buf(data_encoder);
            if (err != GF_SUCCESS)
            {
                break;
            }
        }  // for enumerate
    }  // do hal_dump_templates
    while (0);

    gf_dump_encoder_destroy(data_encoder);
    data_encoder = NULL;

    if (NULL != enumerate)
    {
        free(enumerate);
    }

    if (NULL != template)
    {
        free(template);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_set_dump_path
 * Description: Set storage path.
 * Input: buf
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_set_dump_path(const uint8_t* buf)
{
    gf_error_t err = GF_SUCCESS;
    int32_t path = DUMP_PATH_DATA;
    FUNC_ENTER();

    do
    {
        if (NULL == buf)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        path = get_int32_value(buf);
        if (path < DUMP_PATH_SDCARD || path > DUMP_PATH_DATA)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        gf_set_dump_path(path);
        LOG_I(LOG_TAG, "[%s] dump_path=%u", __func__, path);
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_dump_template
 * Description: Dump finger template by finger id.
 * Input: group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_dump_template(uint32_t group_id, uint32_t finger_id)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    gf_dump_template_t *template = NULL;
    dump_data_encoder_t* data_encoder = NULL;
    struct timeval tv = { 0 };
    uint32_t template_size = sizeof(gf_dump_template_t);

    do
    {
        // check dump template config
        if (gf_is_dump_enabled() == 0 || gf_is_dump_template_allowed() == 0)
        {
            LOG_D(LOG_TAG, "[%s] dump template not allowed", __func__);
            break;
        }
        LOG_D(LOG_TAG, "[%s] group_id = %u, finger_id = %u", __func__, group_id,
              finger_id);
        template = (gf_dump_template_t *) malloc(template_size);

        if (NULL == template)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, template", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(template, 0, template_size);
        template->group_id = group_id;
        template->finger_id = finger_id;
        err = g_hal_function.dump_invoke_command(GF_CMD_DUMP_TEMPLATE, template,
                                                 template_size);

        if (err != GF_SUCCESS)
        {
            LOG_D(LOG_TAG, "[%s] get finger_id = %u data err", __func__, finger_id);
            break;
        }

        gettimeofday(&tv, NULL);
        err = gf_dump_encoder_create(&data_encoder, gf_get_time_stamp(&tv));
        if (err != GF_SUCCESS)
        {
            break;
        }
        err = gf_dump_template(data_encoder, template, &tv);
        if (err != GF_SUCCESS)
        {
            break;
        }
        err = gf_handle_dump_buf(data_encoder);
    }  // do gf_hal_dump_template
    while (0);

    gf_dump_encoder_destroy(data_encoder);
    data_encoder = NULL;

    if (NULL != template)
    {
        free(template);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_dump_auth_retry_data
 * Description: 
 * Input: 
 * Output: None
 * Return: gf_error_t
 */
static void gf_hal_dump_auth_retry_data(gf_operation_type_t operation, uint8_t auth_retry_cache_index)
{
    uint8_t i = 0;
    gf_error_t err = GF_SUCCESS;
    gf_dump_data_t *cmd = NULL;
    dump_data_encoder_t* data_encoder = NULL;
    struct timeval tv = { 0 };
    uint32_t size = sizeof(gf_dump_data_t);

    VOID_FUNC_ENTER();

    do
    {
        cmd = (gf_dump_data_t *) malloc(size);
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        err = gf_dump_encoder_create(&data_encoder, gf_get_time_stamp(&tv));
        if (err != GF_SUCCESS)
        {
            break;
        }

        for (i = 0; i < auth_retry_cache_index; i++)
        {
            memset(cmd, 0, size);
            cmd->operation = operation;
            cmd->data.image.image_quality = i + 1;  // cache flag
            err = g_hal_function.dump_invoke_command(GF_CMD_DUMP_DATA, cmd, size);

            if (err != GF_SUCCESS)
            {
                break;
            }
            err = gf_dump_auth_retry_data(data_encoder, cmd, operation, i);

            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] dump data err<%d>", __func__, err);
                break;
            }
        }
        err = gf_handle_dump_buf(data_encoder);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] dump buf err<%d>", __func__, err);
        }
    }  // do gf_hal_dump_auth_data
    while (0);

    gf_dump_encoder_destroy(data_encoder);

    if (cmd != NULL)
    {
        free(cmd);
        cmd = NULL;
    }
    VOID_FUNC_EXIT();
}

/**
 * Function: hal_dump_reserve_data_by_operation
 * Description: Dump reserve data by operation.
 * Input: operation, error_code
 * Output: None
 * Return: None
 */
static void hal_dump_reserve_data_by_operation(gf_operation_type_t operation, gf_error_t error_code)
{
    gf_error_t err = GF_SUCCESS;
    gf_dump_reserve_t *cmd = NULL;
    dump_data_encoder_t* data_encoder = NULL;
    struct timeval tv = { 0 };
    uint32_t size = sizeof(gf_dump_reserve_t);

    VOID_FUNC_ENTER();

    do
    {
        // check operation and operation result dump config
        if (gf_is_dump_enabled() == 0 || gf_is_operation_result_dump_allowed(operation, error_code) == 0)
        {
            LOG_D(LOG_TAG, "[%s] dump operation<%d> for err_code<%d> not allowed",
                __func__, operation, error_code);
            break;
        }

        cmd = (gf_dump_reserve_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->operation = operation;
        cmd->is_only_dump_broken_check = g_is_only_dump_broken_check;

        err = g_hal_function.dump_invoke_command(GF_CMD_DUMP_DATA_RESERVE, cmd, size);
        if (err != GF_SUCCESS)
        {
            break;
        }
        err = gf_dump_encoder_create(&data_encoder, gf_get_time_stamp(&tv));
        if (err != GF_SUCCESS)
        {
            break;
        }
        err = gf_dump_data_by_operation_reserve(data_encoder, cmd, operation, &tv, error_code, g_hal_config.chip_type);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] dump data err<%d>", __func__, err);
            break;
        }
        err = gf_handle_dump_buf(data_encoder);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] dump buf err<%d>", __func__, err);
        }
    }  // do hal_dump_reserve_data_by_operation
    while (0);

    gf_dump_encoder_destroy(data_encoder);

    if (cmd != NULL)
    {
        free(cmd);
        cmd = NULL;
    }

    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_dump_data_by_operation
 * Description: Dump data by operation.
 * Input: operation, error_code
 * Output: None
 * Return: None
 */
#ifdef GOODIX_CONFIG_DUMP
void gf_hal_dump_data_by_operation(gf_operation_type_t operation, gf_error_t error_code)
{
    gf_error_t err = GF_SUCCESS;
    gf_dump_data_t *cmd = NULL;
    dump_data_encoder_t* data_encoder = NULL;
    uint32_t size = sizeof(gf_dump_data_t);
    struct timeval tv = { 0 };
    uint8_t auth_retry_cache_index = 0;

    VOID_FUNC_ENTER();

    do
    {
        // check operation and operation result dump config
        if (gf_is_dump_enabled() == 0 || gf_is_operation_result_dump_allowed(operation, error_code) == 0)
        {
            LOG_D(LOG_TAG, "[%s] dump operation<%d> for err_code<%d> not allowed",
                __func__, operation, error_code);
            break;
        }

        cmd = (gf_dump_data_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->operation = operation;
        err = g_hal_function.dump_invoke_command(GF_CMD_DUMP_DATA, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        if (OPERATION_AUTHENTICATE_FF == operation ||
            OPERATION_AUTHENTICATE_SLEEP == operation ||
            OPERATION_AUTHENTICATE_IMAGE == operation ||
            OPERATION_AUTHENTICATE_FIDO == operation ||
            OPERATION_TEST_UNTRUSTED_AUTHENTICATE == operation)
        {
            auth_retry_cache_index = cmd->data.image.image_quality >> 24;
            cmd->data.image.image_quality &= 0x000000FF;
            LOG_E(LOG_TAG, "[%s] link:auth_retry_cache_index=%d...", __func__, auth_retry_cache_index);
            if (cmd->data.image.authenticate_retry_count >= 0xFF)
            {
                LOG_D(LOG_TAG, "[%s] skip dump authdata if FDT INT!= finger .", __func__);
                break;
            }
        }
        cmd->down_irq_time = g_down_irq_time;
        cmd->screen_flag = g_screen_status;
        cmd->is_only_dump_broken_check = g_is_only_dump_broken_check;
        gettimeofday(&tv, NULL);

        if (OPERATION_NAV == operation)
        {
            cmd->data.nav.nav_times = g_nav_times;
            cmd->data.nav.nav_frame_index = g_nav_frame_index;
        }

        if (g_hal_function.dump_chip_operation_data(cmd, operation,
                            &tv, error_code, g_hal_config.chip_type))
        {
            // return true, no need common dump for this operation,
            // return false, chip need common dump for this operation
            break;
        }

        err = gf_dump_encoder_create(&data_encoder, gf_get_time_stamp(&tv));
        if (err != GF_SUCCESS)
        {
            break;
        }
        err = gf_dump_data_by_operation(data_encoder, cmd, operation, &tv, error_code, g_hal_config.chip_type);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] dump data err<%d>", __func__, err);
            break;
        }

        err = gf_handle_dump_buf(data_encoder);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] dump buf err<%d>", __func__, err);
        }

        if (OPERATION_NAV == operation)
        {
            g_nav_frame_index = cmd->data.nav.nav_frame_index;
        }

        // dump reserve data
        hal_dump_reserve_data_by_operation(operation, error_code);
    }  // do gf_hal_dump_data_by_operation
    while (0);

    gf_dump_encoder_destroy(data_encoder);

    if (cmd != NULL)
    {
        free(cmd);
        cmd = NULL;
    }
    if (gf_is_dump_enabled() && auth_retry_cache_index && g_hal_config.support_authenticate_once_dump
        && gf_is_operation_result_dump_allowed(operation, GF_ERROR_MATCH_FAIL_AND_RETRY))
    {
        LOG_E(LOG_TAG, "[%s] link:enter gf_hal_dump_auth_retry_data", __func__);
        gf_hal_dump_auth_retry_data(operation, auth_retry_cache_index);
    }
    VOID_FUNC_EXIT();
}
#else
void gf_hal_dump_data_by_operation(gf_operation_type_t operation, gf_error_t error_code)
{
    UNUSED_VAR(operation);
    UNUSED_VAR(error_code);
    LOG_E(LOG_TAG, "[%s] GOODIX_CONFIG_DUMP is not defined, just do nothing", __func__);
}
#endif
/**
 * Function: gf_hal_common_dump_invoke_command
 * Description: Dump invoking interface.
 * Input: cmd_id, buffer, len
 * Output: buffer
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_dump_invoke_command(uint32_t cmd_id, void *buffer,
                                             int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_sensor_mutex);

    do
    {
        if (NULL == buffer)
        {
            LOG_E(LOG_TAG, "[%s] buffer is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_hal_control_spi_clock(1);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] spi clock enable failed", __func__);
            break;
        }

        err = gf_ca_invoke_command(GF_DUMP_OPERATION_ID, cmd_id, buffer, len);
        gf_hal_control_spi_clock(0);
    }
    while (0);

    pthread_mutex_unlock(&g_sensor_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_dump_cmd
 * Description: Parse dump command.
 * Input: dev, cmd_id, param, param_len
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_dump_cmd(void *dev, uint32_t cmd_id, const uint8_t *param,
                           uint32_t param_len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    UNUSED_VAR(dev);
    UNUSED_VAR(param);
    UNUSED_VAR(param_len);
    LOG_D(LOG_TAG, "[%s] cmd id=%d", __func__, cmd_id);

    if (0 == g_hal_inited)
    {
        err = GF_ERROR_GENERIC;
        LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
        return err;
    }

    pthread_mutex_lock(&g_hal_mutex);

    switch (cmd_id)
    {
        case CMD_ENABLE_DUMP_DATA:
        {
            err = hal_enable_dump_data();
            break;
        }

        case CMD_DISABLE_DUMP_DATA:
        {
            err = hal_disable_dump_data();
            break;
        }

        case CMD_DUMP_TEMPLATES:
        {
            err = hal_dump_templates();
            break;
        }

        case CMD_SET_DUMP_PATH:
        {
            err = hal_set_dump_path(param);
            break;
        }

        case CMD_DUMP_NAV_BASE:
        {
            err = hal_dump_nav_base();
            break;
        }

        case CMD_DUMP_FINGER_BASE:
        {
            err = hal_dump_finger_base();
            break;
        }

        case CMD_SET_DUMP_CONFIG:
        {
            err = hal_set_dump_config(param, param_len);
            break;
        }

        default:
        {
            break;
        }
    }  // switch dump type

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_dump_save_cur_time
 * Description: gf_hal_dump_save_cur_time
 * Input: index
 * Output: None
 * Return: None
 */
void gf_hal_dump_save_cur_time(uint8_t index)
{
    gf_dump_save_cur_auth_time(index);
}

