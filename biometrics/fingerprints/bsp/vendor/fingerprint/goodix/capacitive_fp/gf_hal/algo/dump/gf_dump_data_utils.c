/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: util funtions for dump module
 * History:
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Long.Liu     2019/02/11        modify for coverity
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <cutils/properties.h>

#include "gf_dump_data_utils.h"
#include "gf_error.h"
#include "gf_aes.h"
#include "gf_dump_data_encoder.h"
#include "gf_dump_data_decoder.h"
#include "gf_dump_config.h"
#include "gf_hal.h"
#include "gf_hal_log.h"
#include "gf_fingerprint.h"

#define LOG_TAG "[GF_HAL][gf_dump_data_utils]"

#define PROPERTY_DUMP_DATA "gf.debug.dump_data"
#define PROPERTY_DUMP_BIGDATA_DATA "gf.debug.dump_bigdata_data"
#define VALUE_NOT_SET (-1)

static uint32_t g_width = 0;  // sensor width
static uint32_t g_height = 0;  // sensor height
static uint32_t g_nav_width = 0;  // sensor width when navigate
static uint32_t g_nav_height = 0;  // sensor height when navigate
static gf_chip_type_t g_chip_type = GF_CHIP_UNKNOWN;  // chip type
static gf_chip_series_t g_chip_series = GF_UNKNOWN_SERIES;  // chip series
static uint8_t g_boot_time_str[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };  // bn calibration time_str
#ifdef __SIMULATOR
int32_t g_apk_enabled_dump = VALUE_NOT_SET;  // if apk called enable/disable dump
gf_dump_config_t g_dump_config;  // a copy of dump config
#else  // __SIMULATOR
static int32_t g_apk_enabled_dump = VALUE_NOT_SET;  // if apk called enable/disable dump
static gf_dump_config_t g_dump_config;  // a copy of dump config
#endif  // __SIMULATOR

/**
 *Function: gf_init_dump_config
 *Description: initialize dump config
 *Input: sensor_width, sensor width
 *       sensor_height, sensor height
 *       nav_width, nav width
 *       nav_height, nav height
 *       chip_type, chip type
 *       chip_series, chip series
 *Output: none
 *Return: none
 */
void gf_init_dump_config(uint32_t sensor_width, uint32_t sensor_height,
                                   uint32_t nav_width, uint32_t nav_height,
                                   gf_chip_type_t chip_type, gf_chip_series_t chip_series)
{
    g_width = sensor_width;
    g_height = sensor_height;
    g_nav_width = nav_width;
    g_nav_height = nav_height;
    g_chip_type = chip_type;
    g_chip_series = chip_series;

    struct timeval tv = { 0 };
    gettimeofday(&tv, NULL);
    gf_get_time_string(&tv, g_boot_time_str, 0);

    memset(&g_dump_config, 0, sizeof(gf_dump_config_t));
    gf_dump_config_t* cfg = gf_get_dump_config();
    if (cfg != NULL)
    {
        memcpy(&g_dump_config, cfg, sizeof(gf_dump_config_t));
        free(cfg);
        cfg = NULL;
        LOG_D(LOG_TAG, "dump config:dump_version<%s>, dump_bigdata_version<%s>"
                        "dump_enabled<%d>, encrypt_enabled<%d>, bigdata_enabled<%d>, dump_path<%d>",
                        g_dump_config.dump_version, g_dump_config.dump_bigdata_version,
                        g_dump_config.dump_enabled, g_dump_config.dump_encrypt_enabled,
                        g_dump_config.dump_big_data_enabled, g_dump_config.dump_path);
    }
}

/**
 *Function: gf_get_sensor_width
 *Description: get sensor width
 *Input: none
 *Output: none
 *Return: sensor width
 */
uint32_t gf_get_sensor_width()
{
    return g_width;
}

/**
 *Function: gf_get_sensor_height
 *Description: get sensor height
 *Input: none
 *Output: none
 *Return: sensor height
 */
uint32_t gf_get_sensor_height()
{
    return g_height;
}

/**
 *Function: gf_get_nav_width
 *Description: get nav width
 *Input: none
 *Output: none
 *Return: nav width
 */
uint32_t gf_get_nav_width()
{
    return g_nav_width;
}

/**
 *Function: gf_get_nav_height
 *Description: get nav height
 *Input: none
 *Output: none
 *Return: nav height
 */
uint32_t gf_get_nav_height()
{
    return g_nav_height;
}

/**
 *Function: gf_get_boot_time_str
 *Description: get boot time string
 *Input: none
 *Output: none
 *Return: boot time string
 */
uint8_t* gf_get_boot_time_str()
{
    return g_boot_time_str;
}

/**
 *Function: gf_get_chip_type
 *Description: get chip type
 *Input: none
 *Output: none
 *Return: chip type
 */
gf_chip_type_t gf_get_chip_type()
{
    return g_chip_type;
}

/**
 *Function: gf_get_chip_series
 *Description: get chip series
 *Input: none
 *Output: none
 *Return: chip series
 */
gf_chip_series_t gf_get_chip_series()
{
    return g_chip_series;
}

/**
 *Function: gf_get_dump_version
 *Description: get dump version
 *Input: none
 *Output: none
 *Return: dump version code
 */
uint8_t* gf_get_dump_version()
{
    return g_dump_config.dump_version;
}

/**
 *Function: gf_get_dump_bigdata_version
 *Description: get dump bigdata version
 *Input: none
 *Output: none
 *Return: dump bigdata version code
 */
uint8_t* gf_get_dump_bigdata_version()
{
    return g_dump_config.dump_bigdata_version;
}

/**
 *Function: gf_enable_dump
 *Description: enabe dump
 *Input: none
 *Output: none
 *Return: none
 */
void gf_enable_dump()
{
    if (g_dump_config.dump_enabled)
    {
        g_apk_enabled_dump = 1;
    }

    return;
}

/**
 *Function: gf_disable_dump
 *Description: disable dump
 *Input: none
 *Output: none
 *Return: none
 */
void gf_disable_dump()
{
    if (g_dump_config.dump_enabled)
    {
        g_apk_enabled_dump = 0;
        gf_set_dump_path(DUMP_PATH_DATA);
    }

    return;
}

/**
 *Function: gf_is_dump_enabled
 *Description: get if dump is enabled
 *Input: none
 *Output: none
 *Return: 1, enabled; 0, disabled.
 */
uint8_t gf_is_dump_enabled()
{
    uint8_t enabled = 0;
    int8_t property_value = VALUE_NOT_SET;
    do
    {
        // check config first
        if (0 == g_dump_config.dump_enabled)
        {
            // if config is disabled then dump cannot be switched on by apk or setprop
            break;
        }

        // if dump is switched off by apk, then dump is disabled
        if (0 == g_apk_enabled_dump)
        {
            break;
        }

        // if dump is switched off by property, then dump is disabled
#ifdef GOODIX_CONFIG_DUMP
        property_value = 1;
        //dump is enabled
        enabled = 1;
#else
        property_value = 0;
#endif
        if (0 == property_value)
        {
            break;
        }
    }  // do...
    while (0);

    return enabled;
}

/**
 *Function: gf_is_dump_encrypt_enabled
 *Description: get if dump encrypt is enabled
 *Input: none
 *Output: none
 *Return: 1, enabled; 0, disabled.
 */
uint8_t gf_is_dump_encrypt_enabled()
{
    return g_dump_config.dump_encrypt_enabled;
}

/**
 *Function: gf_get_dump_encryptor_version
 *Description: get dump encryptor version
 *Input: none
 *Output: none
 *Return: dump encryptor version
 */
uint16_t gf_get_dump_encryptor_version()
{
    return 0;
}

/**
 *Function: gf_is_dump_bigdata_enabled
 *Description: get if dump bigdata is enabled
 *Input: none
 *Output: none
 *Return: 1, enabled; 0, disabled.
 */
uint8_t gf_is_dump_bigdata_enabled()
{
    uint8_t enabled = 0;
    do
    {
        // check config first
        if (0 == g_dump_config.dump_big_data_enabled)
        {
            break;
        }
        // if config is enabled, then check property
        enabled = property_get_bool(PROPERTY_DUMP_BIGDATA_DATA, g_dump_config.dump_big_data_enabled);
    }
    while (0);

    return enabled;
}

/**
 *Function: gf_set_dump_path
 *Description: set dump path
 *Input: path, DUMP_PATH_SDCARD or DUMP_PATH_DATA
 *Output: none
 *Return: none
 */
void gf_set_dump_path(gf_dump_path_t path)
{
    if (path != g_dump_config.dump_path)
    {
        g_dump_config.dump_path = path;
    }
    return;
}

/**
 *Function: gf_get_dump_path
 *Description: get dump path
 *Input: none
 *Output: none
 *Return: DUMP_PATH_SDCARD or DUMP_PATH_DATA
 */
gf_dump_path_t gf_get_dump_path()
{
    return g_dump_config.dump_path;
}

/**
 *Function: gf_get_dump_root_dir
 *Description: get dump root dir
 *Input: none
 *Output: none
 *Return: dump root dir
 */
uint8_t* gf_get_dump_root_dir()
{
    if (DUMP_PATH_DATA == g_dump_config.dump_path)
    {
        return (uint8_t*) GF_DUMP_DATA_ROOT_PATH;
    }
    else
    {
        return (uint8_t*) "";
    }
}

/**
 *Function: gf_is_operation_result_dump_allowed
 *Description: check if the given operation and err code dump is allowed
 *Input: operation,
 *       op_err_code,
 *Output: none
 *Return: 1, allowed; 0, not allowed
 */
uint8_t gf_is_operation_result_dump_allowed(gf_operation_type_t operation,
                                                gf_error_t op_err_code)
{
    uint32_t allow = 0;
    gf_dump_operation_type_t dump_op = DUMP_OP_NOT_ALLOWED;
    gf_dump_operation_result_t result = OP_RESULT_ALL;

    do
    {
        dump_op = gf_get_dump_op_by_irq_op(operation);
        if (DUMP_OP_NOT_ALLOWED == dump_op
            || OP_DATA_NONE == g_dump_config.dump_operation[dump_op].data_type)
        {
            allow = 0;
            break;
        }
        result = g_dump_config.dump_operation[dump_op].result;
        switch (result)
        {
            case OP_RESULT_ALL:
            {
                allow = 1;
                break;
            }

            case OP_RESULT_SUCCESS:
            {
                allow = (GF_SUCCESS == op_err_code ? 1 : 0);
                break;
            }

            case OP_RESULT_FAIL:
            {
                allow = (GF_SUCCESS == op_err_code ? 0 : 1);
                break;
            }

            default:
            {
                allow = 0;
                break;
            }
        }  // switch (result)
    }  // do...
    while (0);

    return allow;
}

/**
 *Function: gf_is_operation_data_dump_allowed
 *Description: check if the data for given operation dump is allowed
 *Input: operation, operation type
 *       op_data, data type
 *Output: none
 *Return: 1, allowed; 0, not allowed
 */
uint8_t gf_is_operation_data_dump_allowed(gf_operation_type_t operation,
                                                gf_dump_operation_data_type_t op_data)
{
    gf_dump_operation_type_t dump_op = DUMP_OP_NOT_ALLOWED;
    dump_op = gf_get_dump_op_by_irq_op(operation);

    if (DUMP_OP_NOT_ALLOWED == dump_op)
    {
        return 0;
    }

    return op_data & g_dump_config.dump_operation[dump_op].data_type ? 1 : 0;
}

/**
 *Function: gf_is_dump_template_allowed
 *Description: check if dump template is allowed
 *Input: none
 *Output: none
 *Return: 1, allowed; 0, not allowed
 */
uint8_t gf_is_dump_template_allowed()
{
    return (OP_DATA_TEMPLATE_DATA & g_dump_config.dump_operation[DUMP_OP_TEMPLATE].data_type) ? 1 : 0;
}

/**
 *Function: gf_is_dump_device_info_allowed
 *Description: check if dump device info is allowed
 *Input: none
 *Output: none
 *Return: 1, allowed; 0, not allowed
 */
uint8_t gf_is_dump_device_info_allowed()
{
    return (OP_DATA_DEVICE_INFO & g_dump_config.dump_operation[DUMP_OP_DEVICE_INFO].data_type) ? 1 : 0;
}

/**
 *Function: gf_get_time_string
 *Description: get time string YYYY-MM-DD-HH-MM-SS-MICROS or YYYY-MM-DD-HH-MM-SS
 *Input: tv, time value
 *       str_buf, string buffer
 *       with_micro_sec, 1 with, 0 without
 *Output: time string
 *Return: none
 */
void gf_get_time_string(struct timeval* tv, uint8_t* str_buf, uint8_t with_micro_sec)
{
    struct tm current_tm = { 0 };

    if (NULL == tv || NULL == str_buf)
    {
        return;
    }
    memset(str_buf, 0, GF_DUMP_FILE_PATH_MAX_LEN);

    localtime_r(&tv->tv_sec, &current_tm);
    if (with_micro_sec)
    {
        snprintf((char*)str_buf, GF_DUMP_FILE_PATH_MAX_LEN,
             "%04d-%02d-%02d-%02d-%02d-%02d-%06ld",
             current_tm.tm_year + 1900,
             current_tm.tm_mon + 1, current_tm.tm_mday, current_tm.tm_hour,
             current_tm.tm_min, current_tm.tm_sec, tv->tv_usec);
    }
    else
    {
        snprintf((char*)str_buf, GF_DUMP_FILE_PATH_MAX_LEN,
             "%04d-%02d-%02d-%02d-%02d-%02d",
             current_tm.tm_year + 1900,
             current_tm.tm_mon + 1, current_tm.tm_mday, current_tm.tm_hour,
             current_tm.tm_min, current_tm.tm_sec);
    }

    return;
}

/**
 *Function: gf_get_time_stamp
 *Description: get time stamp
 *Input: tv, time value
 *Output: none
 *Return: time stamp
 */
int64_t gf_get_time_stamp(struct timeval* tv)
{
    int64_t timestamp = 0;
    struct tm time = { 0 };

    if (NULL == tv)
    {
        return timestamp;
    }
    localtime_r(&tv->tv_sec, &time);
    timestamp = (int64_t)tv->tv_sec * 1000000L + tv->tv_usec;
    return timestamp;
}

/**
 *Function: gf_get_operation_result_str
 *Description: get operation result string
 *Input: dump_data, dump data buffer got from TA
 *       error_code, error code
 *       result, result string buffer
 *       result_buf_len, result string buf len
 *Output: result string
 *Return: none
 */
void gf_get_operation_result_str(gf_dump_data_t *dump_data,
                                     gf_error_t error_code,
                                     uint8_t* result,
                                     uint32_t result_buf_len)
{
    VOID_FUNC_ENTER();
    if (NULL == dump_data || NULL == result)
    {
        return;
    }

    switch (error_code)
    {
        case GF_SUCCESS:
        {
            snprintf((char*)result, result_buf_len, "retry_%d_success",
                    dump_data->data.image.authenticate_retry_count);
            dump_data->data.image.authenticate_retry_count = 0;
            break;
        }

        case GF_ERROR_SENSOR_IS_BROKEN:
        {
            snprintf((char*)result, result_buf_len, "%s", "sensor_is_damaged");
            break;
        }

        case GF_ERROR_INVALID_BASE:
        {
            snprintf((char*)result, result_buf_len, "%s", "bad_base");
            break;
        }

        case GF_ERROR_INVALID_FINGER_PRESS:
        {
            snprintf((char*)result, result_buf_len, "%s", "invalid_finger_press");
            break;
        }

        default:
        {
            snprintf((char*)result, result_buf_len, "retry_%d_code_%u_fail",
                     dump_data->data.image.authenticate_retry_count, error_code);
            break;
        }
    }  // switch (error_code)

    VOID_FUNC_EXIT();
    return;
}

/**
 *Function: gf_dump_raw_data
 *Description: dump raw data
 *Input: data_encoder, encoder buffer
 *       file_path, file path
 *       data, path to dump
 *       data_num, how many data elements in data
 *       data_type, raw data type
 *Output: encoded data into encoder buffer
 *Return: gf_error_t
 */
gf_error_t gf_dump_raw_data(dump_data_encoder_t* data_encoder,
                         const uint8_t* file_path,
                         const uint16_t* data,
                         uint32_t data_num,
                         data_type_t data_type)
{
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == data_encoder || NULL == file_path
            || NULL == data || 0 == data_num)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_dump_encoder_fopen(data_encoder, file_path, data_type);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, file_path);
            break;
        }
        err = gf_dump_encoder_fwrite(data, 2 * data_num, data_encoder);
    }
    while (0);

    gf_dump_encoder_fclose(data_encoder);
    return err;
}

/**
 *Function: gf_dump_raw_data
 *Description: dump raw data
 *Input: data_encoder, encoder buffer
 *       file_path, file path
 *       data, path to dump
 *       data_num, how many data elements in data
 *       data_type, raw data type
 *Output: encoded data into encoder buffer
 *Return: gf_error_t
 */
gf_error_t gf_dump_raw_data_minus(dump_data_encoder_t* data_encoder,
                         const uint8_t* file_path,
                         const uint16_t* data,
                         uint32_t data_num,
                         data_type_t data_type)
{
    gf_error_t err = GF_SUCCESS;
    uint16_t data_temp[IMAGE_BUFFER_LEN];
    uint16_t i = 0;

    do
    {
        if (NULL == data_encoder || NULL == file_path
            || NULL == data || 0 == data_num)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_E(LOG_TAG, "data base= %d", data[100]);
        for (i = 0; i < gf_get_sensor_width() * gf_get_sensor_height(); i++)
        {
            if (data[i] > 7000)
            {
                data_temp[i] = data[i] - 7095;
            }
            else
            {
                data_temp[i] = data[i];
            }
        }

        err = gf_dump_encoder_fopen(data_encoder, file_path, data_type);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, file_path);
            break;
        }
        err = gf_dump_encoder_fwrite(data_temp, 2 * data_num, data_encoder);
    }  // do ...
    while (0);

    gf_dump_encoder_fclose(data_encoder);
    return err;
}
/**
 *Function: gf_dump_encrypt
 *Description: encrypt encoder buffer
 *Input: encoder, encoder buffer
 *Output: encrypted encoder buffer
 *Return: gf_error_t
 */
static gf_error_t gf_dump_encrypt(dump_data_encoder_t* encoder)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_buffer_header_t* buf = NULL;
    uint8_t* crypto_buf = NULL;
    FUNC_ENTER();

    do
    {
        if (NULL == encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        buf = (dump_data_buffer_header_t*)encoder->buf;
        buf->encrypt_info = (int32_t)gf_get_dump_encryptor_version();

        crypto_buf = (uint8_t*)buf + sizeof(dump_data_buffer_header_t);
        gf_hal_aes_encrypt(crypto_buf, crypto_buf, encoder->cur_write_offset - sizeof(dump_data_buffer_header_t));
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: gf_handle_dump_buf
 *Description: handle dump buffer, call this function after dump finished to
 *             write data into file system.
 *Input: data_encoder, encoder buffer
 *Output: files in file system
 *Return: gf_error_t
 */
gf_error_t gf_handle_dump_buf(dump_data_encoder_t* data_encoder)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_buffer_header_t* buf = NULL;
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == data_encoder->buf
            || sizeof(dump_data_buffer_header_t) == data_encoder->cur_write_offset)
        {
            break;
        }

        gf_dump_encoder_fclose(data_encoder);

        buf = (dump_data_buffer_header_t*)data_encoder->buf;

        // encrypt if needed
        if (gf_is_dump_encrypt_enabled())
        {
            err = gf_dump_encrypt(data_encoder);
            if (err != GF_SUCCESS)
            {
                break;
            }
        }

        // dump to external storage, sd card
        if (DUMP_PATH_SDCARD == gf_get_dump_path())
        {
            gf_fingerprint_dump_msg_t message = { 0 };
            message.cmd_id = 0;  // not use
            message.data = (int8_t *) buf;
            message.data_len = data_encoder->cur_write_offset;
            g_fingerprint_device->dump_notify(&message);
            break;
        }

        // dump to internal storage
        err = (gf_error_t)gf_decode_and_write_file(buf,
                data_encoder->cur_write_offset, gf_get_dump_root_dir());
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: gf_set_dump_config
 *Description: set dump config
 *Input: cfg, dump cfg
 *Output: none
 *Return: none
 */
void gf_set_dump_config(gf_dump_config_t* cfg)
{
    int32_t i = 0;
    if (NULL == cfg)
    {
        return;
    }
    g_dump_config.dump_enabled = cfg->dump_enabled;
    g_dump_config.dump_encrypt_enabled = cfg->dump_encrypt_enabled;
    g_dump_config.dump_big_data_enabled = cfg->dump_big_data_enabled;
    g_dump_config.dump_path = cfg->dump_path;
    LOG_D(LOG_TAG, "dump config:dump_version<%s>, dump_bigdata_version<%s>"
                    "dump_enabled<%d>, encrypt_enabled<%d>, bigdata_enabled<%d>, dump_path<%d>",
                    g_dump_config.dump_version, g_dump_config.dump_bigdata_version,
                    g_dump_config.dump_enabled, g_dump_config.dump_encrypt_enabled,
                    g_dump_config.dump_big_data_enabled, g_dump_config.dump_path);
    for (i = 0; i < DUMP_OP_NUM; i++)
    {
        g_dump_config.dump_operation[i].result = cfg->dump_operation[i].result;
        g_dump_config.dump_operation[i].data_type = cfg->dump_operation[i].data_type;
        LOG_D(LOG_TAG, "dump config:operation<%d>, result<%d>, data_type<%d>",
                    g_dump_config.dump_operation[i].operation,
                    g_dump_config.dump_operation[i].result,
                    g_dump_config.dump_operation[i].data_type);
    }
}


