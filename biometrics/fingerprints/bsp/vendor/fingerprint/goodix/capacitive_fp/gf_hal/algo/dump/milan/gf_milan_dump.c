/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: milan chip particular dump
 * History:
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gf_milan_dump.h"
#include "gf_hal_log.h"
#include "gf_dump_data.h"
#include "gf_dump_data_utils.h"
#include "gf_dump_bigdata.h"
#include "gf_hal_mem.h"
#include "gf_hal_common.h"
#include "gf_hal.h"

#define LOG_TAG "[GF_HAL][gf_milan_dump]"

/**
 *Function: hal_dump_reg_config
 *Description: dump reg config
 *Input: data_encoder, encoder buffer
 *       dir, dump dir
 *       data_milan_config, milan config
 *       cur_time_str, current time string
 *Output: encoded reg config data to data_encoder
 *Return: gf_error_t
 */
static gf_error_t hal_dump_reg_config(dump_data_encoder_t* data_encoder,
                                     uint8_t* dir,
                                     milan_chip_config_t* data_milan_config,
                                     uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    milan_chip_config_t milan_config;
    FUNC_ENTER();

    do
    {
        if (NULL == dir || NULL == data_encoder || NULL == data_milan_config)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }  // end if
        memcpy(&milan_config, data_milan_config, sizeof(milan_chip_config_t));

        snprintf((char*)file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_config", (char *)dir,
                 (char*)cur_time_str);
        err = gf_dump_encoder_fopen(data_encoder, file_path, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fprintf(data_encoder, "ff: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.ff.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.ff.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump ff config
        for (i = 0; i <= milan_config.fdt_manual_down.data_len / 16; i++)
        {
            err = gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.ff.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "fdt_manual_down: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.fdt_manual_down.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.fdt_manual_down.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump fdt_manual_down config
        for (i = 0; i <= milan_config.fdt_manual_down.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.fdt_manual_down.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "fdt_down: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.fdt_down.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.fdt_down.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump fdt_down config
        for (i = 0; i <= milan_config.fdt_down.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.fdt_down.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "fdt_up: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.fdt_up.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.fdt_up.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump fdt up config
        for (i = 0; i <= milan_config.fdt_up.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.fdt_up.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "image: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.image.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.image.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump image config
        for (i = 0; i <= milan_config.image.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.image.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav_fdt_down: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.nav_fdt_down.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav_fdt_down.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump nav fdt down config
        for (i = 0; i <= milan_config.nav_fdt_down.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav_fdt_down.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav_fdt_up: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.nav_fdt_up.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav_fdt_up.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump nav fdt up config
        for (i = 0; i <= milan_config.nav_fdt_up.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav_fdt_up.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.nav.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump nav config
        for (i = 0; i <= milan_config.nav.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav_base: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.nav_base.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav_base.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        // dump nav base config
        for (i = 0; i <= milan_config.nav_base.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav_base.data[16 * i + j]);
            }  // end for

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }  // end for

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fclose(data_encoder);
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_dump_device_info
 *Description: dump device info
 *Input: dev_info, device info to be dump
 *Output: files in file system
 *Return: gf_error_t
 */
static gf_error_t hal_dump_device_info(gf_dev_info_t *dev_info)
{
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t cur_time[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    gf_error_t err = GF_SUCCESS;
    dump_data_encoder_t* data_encoder = NULL;
    struct timeval tv = { 0 };
    int64_t timestamp = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dev_info)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }  // end if

        gettimeofday(&tv, NULL);
        timestamp = gf_get_time_stamp(&tv);
        gf_get_time_string(&tv, cur_time, 1);

        err = gf_dump_encoder_create(&data_encoder, timestamp);
        if (NULL == data_encoder)
        {
            break;
        }  // end if

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_DEV_INFO);
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_otp_info", (char*)dir, (char*)cur_time);
        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fwrite((uint8_t *)dev_info->otp_info, GF_BIGDATA_SENSOR_OTP_INFO_LEN, data_encoder);
        gf_dump_encoder_fclose(data_encoder);

        err = hal_dump_reg_config(data_encoder, dir, &dev_info->g_milan_config, cur_time);
        BREAK_IF_ERROR(err);  // break if error

        err = gf_bigdata_dump_device_info(data_encoder, dir, dev_info, cur_time);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] write device info fail", __func__);
        }  // end if

        err = gf_handle_dump_buf(data_encoder);
    }  // do...
    while (0);

    gf_dump_encoder_destroy(data_encoder);
    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_milan_dump_chip_init_data
 *Description: dump milan f chip particular init data
 *Input: none
 *Output: files in file system
 *Return: gf_error_t
 */
bool gf_hal_milan_dump_chip_init_data()
{
    bool ret = true;
    gf_error_t err = GF_SUCCESS;
    gf_dev_info_t *cmd_dev_info = NULL;
    int32_t cmd_size = sizeof(gf_dev_info_t);
    FUNC_ENTER();

    do
    {
        if (gf_is_dump_enabled() == 0 ||
            gf_is_dump_bigdata_enabled() == 0 ||
            gf_is_dump_device_info_allowed() == 0)
        {
            break;
        }  // end if
        cmd_dev_info = (gf_dev_info_t *) GF_MEM_MALLOC(cmd_size);

        if (NULL == cmd_dev_info)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd_dev_info", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }  // end if

        memset(cmd_dev_info, 0, cmd_size);
        err = gf_hal_invoke_command(GF_CMD_GET_DEV_INFO, cmd_dev_info, cmd_size);
        BREAK_IF_ERROR(err);
        // dump bigdata if success
        err = hal_dump_device_info(cmd_dev_info);
    }  // do...
    while (0);

    if (NULL != cmd_dev_info)
    {
        GF_MEM_FREE(cmd_dev_info);
    }  // end if

    FUNC_EXIT(err);
    return ret;
}

/**
 *Function: hal_milan_dump_bad_point
 *Description: dump milan f chip bad point data
 *Input: dump_data, operation, timestamp
 *Output: files in file system
 *Return: gf_error_t
 */
static gf_error_t hal_milan_dump_bad_point(dump_data_encoder_t* data_encoder,
                                       gf_dump_data_t *dump_data,
                                       gf_operation_type_t operation,
                                      const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t index = 0;
    uint32_t frame_num = MAX_BAD_POINT_TEST_FRAME_NUM;
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }  // end if

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_CONSISTENCY_TEST_DIR);

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            if (OPERATION_TEST_BAD_POINT_RECODE_BASE == operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT save bad point basedata",
                          __func__);
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_base_rawdata.csv", (char*)dir,
                         (char*)cur_time_str, dump_data->data.image.frame_num);
                err = gf_dump_raw_data(data_encoder,
                                filepath,
                                dump_data->data.image.raw_data,
                                gf_get_sensor_width() * gf_get_sensor_height(),
                                DATA_TYPE_RAW_DATA);
            }  // bad point base dump
            else
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT save data", __func__);
                // dump bad point data
                for (index = 0; index < frame_num; index++)
                {
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_rawdata.csv",
                                (char*)dir, (char*)cur_time_str, index);
                    err = gf_dump_raw_data(data_encoder,
                                filepath,
                                dump_data->data.image.share.bad_point.bad_point_test_raw_data[index],
                                gf_get_sensor_width() * gf_get_sensor_height(),
                                DATA_TYPE_RAW_DATA);
                    if (err != GF_SUCCESS)
                    {
                        break;
                    }  // end ir
                }  // end of for
            }  // end of else
        }  // end of if gf_is_operation_data_dump_allowed
    }  // end of do while
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_milan_dump_mask_by_opreation
 *Description: set dir and dump broken mask for milan f/hv chip
 *Input: dump_data, operation, timestamp
 *Output: files in file system
 *Return: gf_error_t
 */
static gf_error_t hal_milan_dump_mask_by_opreation(dump_data_encoder_t* data_encoder,
                                             gf_dump_data_t *dump_data,
                                             gf_error_t error_code,
                                             gf_operation_type_t operation,
                                             const uint8_t *cur_time)
{
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        // check if dump is allowed
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_PARAMS) == 1)
        {
            // break if only dump broken check
            if (1 == dump_data->is_only_dump_broken_check)
            {
                break;
            }

            switch (operation)
            {
                case OPERATION_ENROLL:
                case OPERATION_TEST_UNTRUSTED_ENROLL:
                {
                    // GF_DUMP_ENROLL_DIR & GF_DUMP_TEST_UNTRUSTED_ENROLL_DIR
                    if (OPERATION_ENROLL == operation)
                    {
                        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%u/",
                                 GF_DUMP_ENROLL_DIR, dump_data->data.image.enrolling_finger_id);
                    }
                    else
                    {
                        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%u/",
                                GF_DUMP_TEST_UNTRUSTED_ENROLL_DIR, dump_data->data.image.enrolling_finger_id);
                    }
                    break;
                }

                case OPERATION_TEST_PERFORMANCE:
                {
                    // GF_DUMP_TEST_PERFORMANCE_DIR
                    snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_PERFORMANCE_DIR);
                    break;
                }

                case OPERATION_AUTHENTICATE_IMAGE:
                case OPERATION_AUTHENTICATE_SLEEP:
                case OPERATION_AUTHENTICATE_FF:
                case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
                {
                    // GF_DUMP_AUTHENTICATE_DIR & GF_DUMP_TEST_UNTRUSTED_AUTHENTICATE_DIR
                    if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == operation)
                    {
                        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s",
                                GF_DUMP_TEST_UNTRUSTED_AUTHENTICATE_DIR);
                    }
                    else
                    {
                        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_AUTHENTICATE_DIR);
                    }
                    break;
                }

                default:
                {
                    // opreation not support
                    dir[0] = '\0';
                    break;
                }
            }  // switch ..
        }  // end if
    }  // end do
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_dump_swipe_enroll_raw_data
 *Description: dump swipe enroll raw data
 *Input: swipe_data, cur_time, error_code, dir
 *Output: encoded data in data_encoder
 *Return: gf_error_t
 */
static gf_error_t hal_dump_swipe_enroll_raw_data(dump_data_encoder_t* data_encoder,
                                             gf_dump_swipe_enroll_t *swipe_data,
                                             const uint8_t *cur_time,
                                             gf_error_t error_code,
                                             const uint8_t* dir)
{
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    gf_error_t err = GF_SUCCESS;
    int32_t frame_num = 0;
    int32_t i = 0;
    FUNC_ENTER();
    UNUSED_VAR(data_encoder);
    UNUSED_VAR(cur_time);

    do
    {
        LOG_D(LOG_TAG, "[%s] data_len = %d, data_type=%d, finish_flag=%d",
                __func__, swipe_data->data_len, swipe_data->data_type, swipe_data->finish_flag);
        if (swipe_data->data_len % sizeof(gf_dump_swipe_enroll_raw_t) != 0)
        {
            LOG_D(LOG_TAG, "[%s] got dirty data, data_len=%d, gf_dump_swipe_enroll_raw_t size=%d",
                __func__, swipe_data->data_len, (int32_t)sizeof(gf_dump_swipe_enroll_raw_t));
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        frame_num = swipe_data->data_len / sizeof(gf_dump_swipe_enroll_raw_t);
        LOG_D(LOG_TAG, "[%s] frame num=%d", __func__, frame_num);
        if (gf_is_operation_data_dump_allowed(OPERATION_ENROLL, OP_DATA_RAW_DATA) == 1)
        {
            for (; i < frame_num; i++)
            {
                gf_dump_swipe_enroll_raw_t* raw_data = swipe_data->swipe_data.swipe_raw + i;
                LOG_D(LOG_TAG, "[%s] polling_index=%d, overlap_error=%d, overlap_rate=%d", __func__,
                    raw_data->polling_index, raw_data->overlap_error, raw_data->overlap_rate);
                LOG_D(LOG_TAG, "[%s] register_flag=%d, raw_data_len=%d", __func__,
                    raw_data->register_flag, raw_data->raw_data_len);
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_%d_%d_%u_%u_rawdata.csv",
                    dir, cur_time, raw_data->polling_index, error_code, raw_data->overlap_error,
                    raw_data->overlap_rate, raw_data->register_flag);
                err = gf_dump_raw_data(data_encoder,
                                    filepath,
                                    raw_data->raw_data,
                                    raw_data->raw_data_len,
                                    DATA_TYPE_RAW_DATA);
            }
        }
    }  // end do
    while (0);
    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_dump_swipe_enroll_cali_data
 *Description: dump swipe enroll preprocess data
 *Input: swipe_data, cur_time, error_code, dir
 *Output: encoded data in data_encoder
 *Return: gf_error_t
 */
static gf_error_t hal_dump_swipe_enroll_cali_data(dump_data_encoder_t* data_encoder,
                                             gf_dump_swipe_enroll_t *swipe_data,
                                             const uint8_t *cur_time,
                                             gf_error_t error_code,
                                             const uint8_t* dir)
{
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    gf_error_t err = GF_SUCCESS;
    int32_t frame_num = 0;
    int32_t j = 0;
    FUNC_ENTER();
    UNUSED_VAR(data_encoder);
    UNUSED_VAR(swipe_data);
    UNUSED_VAR(cur_time);

    do
    {
        LOG_D(LOG_TAG, "[%s] data_len = %d, data_type=%d, finish_flag=%d",
                __func__, swipe_data->data_len, swipe_data->data_type, swipe_data->finish_flag);
        if (swipe_data->data_len % sizeof(gf_dump_swipe_enroll_cali_t) != 0)
        {
            LOG_D(LOG_TAG, "[%s] got dirty data, data_len=%d, gf_dump_swipe_enroll_cali_t size=%d",
                __func__, swipe_data->data_len, (int32_t)sizeof(gf_dump_swipe_enroll_cali_t));
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        frame_num = swipe_data->data_len / sizeof(gf_dump_swipe_enroll_cali_t);
        LOG_D(LOG_TAG, "[%s] frame num=%d", __func__, frame_num);
        for (; j < frame_num; j++)
        {
            gf_dump_swipe_enroll_cali_t* cali_data = swipe_data->swipe_data.swipe_cali + j;
            LOG_D(LOG_TAG, "[%s] polling_index=%d, image_quality=%d, valid_area=%d", __func__,
                cali_data->polling_index, cali_data->image_quality, cali_data->valid_area);
            LOG_D(LOG_TAG, "[%s] record_flag=%d", __func__,
                cali_data->record_flag);
            // dump calibration params
            if (gf_is_operation_data_dump_allowed(OPERATION_ENROLL, OP_DATA_CALIBRATION_PARAMS) == 1)
            {
                // dump kr to ".csv" file
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_kr.csv",
                            (char*)dir, (char*)cur_time, cali_data->polling_index);
                err = gf_dump_raw_data(data_encoder, filepath, (uint16_t*)cali_data->kr,
                            gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);

                // dump b to ".csv" file
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_b.csv",
                            (char*)dir, (char*)cur_time, cali_data->polling_index);
                err = gf_dump_raw_data(data_encoder, filepath, (uint16_t*)cali_data->b,
                            gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);

                // dump broken mask
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_broken_mask.csv",
                    (char*)dir, (char*)cur_time, cali_data->polling_index);
                err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
                if (err != GF_SUCCESS)
                {
                    LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, filepath);
                    break;
                }
                err = gf_dump_encoder_fwrite(cali_data->broken_mask,
                                gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
                gf_dump_encoder_fclose(data_encoder);

                if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
                {
                    // dump touch mask
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_touch_mask.csv",
                        (char*)dir, (char*)cur_time, cali_data->polling_index);
                    err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
                    if (err != GF_SUCCESS)
                    {
                        LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, filepath);
                        break;
                    }
                    gf_dump_encoder_fwrite(cali_data->touch_mask,
                                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
                    gf_dump_encoder_fclose(data_encoder);
                }

                // dump basic information for base frame
                {
                    uint32_t i = 0;
                    uint8_t line[1024] = { 0 };
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_base_info.csv",
                        (char*)dir, (char*)cur_time, cali_data->polling_index);
                    err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_NORMAL_FILE_DATA);
                    if (err != GF_SUCCESS)
                    {
                        LOG_E(LOG_TAG, "[%s] open file (%s) fail", __func__, filepath);
                        break;
                    }

                    gf_dump_encoder_fwrite("preprocess version, ", strlen("preprocess version, "), data_encoder);
                    gf_dump_encoder_fwrite(cali_data->preprocess_version,
                            strlen((char *) cali_data->preprocess_version), data_encoder);
                    gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
                    // sensor id
                    gf_dump_encoder_fwrite("sensor id, ", strlen("sensor id, "), data_encoder);

                    for (i = 0; i < GF_SENSOR_ID_LEN; i++)
                    {
                        snprintf((char*)line, sizeof(line), "0x%02X, ", cali_data->sensor_id[i]);
                        gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
                    }

                    gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
                    // chip id
                    gf_dump_encoder_fwrite("chip id, ", strlen("chip id, "), data_encoder);

                    for (i = 0; i < GF_CHIP_ID_LEN; i++)
                    {
                        snprintf((char*)line, sizeof(line), "0x%02X, ", cali_data->chip_id[i]);
                        gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
                    }

                    gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
                    // vendor id
                    gf_dump_encoder_fwrite("vendor id, ", strlen("vendor id, "), data_encoder);

                    for (i = 0; i < GF_VENDOR_ID_LEN; i++)
                    {
                        snprintf((char*)line, sizeof(line), "0x%02X, ", cali_data->vendor_id[i]);
                        gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
                    }

                    gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
                    gf_dump_encoder_fwrite("image quality, ", strlen("image quality, "), data_encoder);
                    snprintf((char*)line, sizeof(line), "%u", cali_data->image_quality);
                    gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);

                    gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
                    gf_dump_encoder_fwrite("valid area, ", strlen("valid area, "), data_encoder);
                    snprintf((char*)line, sizeof(line), "%u", cali_data->valid_area);
                    gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);

                    err = gf_dump_encoder_fclose(data_encoder);
                }
            }  // end if

            // dump caliRes
            if (gf_is_operation_data_dump_allowed(OPERATION_ENROLL, OP_DATA_CALIBRATION_RES) == 1)
            {
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_calires.csv",
                    (char*)dir, (char*)cur_time, cali_data->polling_index);
                err = gf_dump_raw_data(data_encoder, (uint8_t*)filepath, cali_data->cali_res,
                            gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);
            }

            if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
            {
                // dump dataBmp
                if (gf_is_operation_data_dump_allowed(OPERATION_ENROLL, OP_DATA_DATA_BMP) == 1)
                {
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_enc_swipebmp.bmp",
                            (char*)dir, (char*)cur_time, cali_data->polling_index);
                    err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
                    BREAK_IF_ERROR(err);  // break if error
                    err = gf_dump_encoder_fwrite(cali_data->data_bmp,
                            gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
                    gf_dump_encoder_fclose(data_encoder);
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_swipebmp.bmp",
                            (char*)dir, (char*)cur_time, cali_data->polling_index);
                    err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
                    BREAK_IF_ERROR(err);  // break if error
                    err = gf_dump_encoder_fwrite(cali_data->data2_bmp,
                            gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
                    gf_dump_encoder_fclose(data_encoder);
                }
            }
            else
            {
                // dump dataBmp
                if (gf_is_operation_data_dump_allowed(OPERATION_ENROLL, OP_DATA_DATA_BMP) == 1)
                {
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_databmp.bmp",
                        (char*)dir, (char*)cur_time, cali_data->polling_index);
                    err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
                    BREAK_IF_ERROR(err);  // break if error
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_databmp.csv",
                        (char*)dir, (char*)cur_time, cali_data->polling_index);
                    gf_dump_encoder_add_path(data_encoder, (uint8_t*)filepath);
                    err = gf_dump_encoder_fwrite(cali_data->data_bmp,
                            gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
                    gf_dump_encoder_fclose(data_encoder);
                }

                // dump sitoBmp
                if (gf_is_operation_data_dump_allowed(OPERATION_ENROLL, OP_DATA_SITO_BMP) == 1)
                {
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_sitobmp.bmp",
                        (char*)dir, (char*)cur_time, cali_data->polling_index);
                    err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%03u_sitobmp.csv",
                        (char*)dir, (char*)cur_time, cali_data->polling_index);
                    gf_dump_encoder_add_path(data_encoder, (uint8_t*)filepath);
                    err = gf_dump_encoder_fwrite(cali_data->sito_bmp,
                            gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
                    gf_dump_encoder_fclose(data_encoder);
                }
            }  // else ...
        }  // end for
    }  // end do
    while (0);
    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_dump_swipe_enroll
 *Description: dump swipe enroll data
 *Input: data_encoder, cmd, cur_time, error_code
 *Output: encoded data in data_encoder
 *Return: gf_error_t
 */
static gf_error_t hal_dump_swipe_enroll(dump_data_encoder_t* data_encoder,
                                             gf_dump_data_t *cmd,
                                             const uint8_t *cur_time,
                                             gf_error_t error_code)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t data_type = 0;
    uint8_t finish_flag = 0;
    gf_dump_swipe_enroll_t* swipe_data = (gf_dump_swipe_enroll_t*)&(cmd->data.swipe);
    FUNC_ENTER();

    do
    {
        do
        {
            data_type = swipe_data->data_type;
            finish_flag = swipe_data->finish_flag;
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%u/",
                     GF_DUMP_ENROLL_DIR, swipe_data->enrolling_finger_id);
            if (0 == data_type)
            {
                if (swipe_data->data_len > 0)
                {
                    err = hal_dump_swipe_enroll_raw_data(data_encoder, swipe_data, cur_time, error_code, dir);
                }
            }
            else
            {
                if (swipe_data->data_len > 0)
                {
                    err = hal_dump_swipe_enroll_cali_data(data_encoder, swipe_data, cur_time, error_code, dir);
                }
            }
            if (1 == finish_flag)
            {
                if (0 == data_type)
                {
                    data_type = 1;
                    finish_flag = 0;
                }
                else
                {
                    LOG_D(LOG_TAG, "[%s] dump swipe enroll finished", __func__);
                    break;
                }
            }
            memset(cmd, 0, sizeof(gf_dump_data_t));
            cmd->operation = OPERATION_ENROLL;
            cmd->swipe_enroll_flag = 1;
            swipe_data->data_type = data_type;
            swipe_data->finish_flag = finish_flag;
            err = g_hal_function.dump_invoke_command(GF_CMD_DUMP_DATA, cmd, sizeof(gf_dump_data_t));
        }  // end do
        while (GF_SUCCESS == err);
    }  // end do
    while (0);
    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_milan_dump_chip_operation_data
 *Description: dump milan chip data by operation
 *Input: dump_data, operation, timestamp, errcode, chip_type
 *Output: files in file system
 *Return: gf_error_t
 */
bool gf_hal_milan_dump_chip_operation_data(gf_dump_data_t *dump_data,
                                       gf_operation_type_t operation,
                                       struct timeval* tv,
                                       gf_error_t error_code,
                                       gf_chip_type_t chip_type)
{
    bool ret = false;
    gf_error_t err = GF_SUCCESS;
    dump_data_encoder_t* data_encoder = NULL;
    uint8_t cur_time_str[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    UNUSED_VAR(error_code);
    UNUSED_VAR(chip_type);
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }  // end if

        err = gf_dump_encoder_create(&data_encoder, gf_get_time_stamp(tv));
        if (err != GF_SUCCESS)
        {
            break;
        }  // end if

        gf_get_time_string(tv, cur_time_str, 1);

        switch (operation)
        {
            case OPERATION_TEST_BAD_POINT_RECODE_BASE:
            case OPERATION_TEST_BAD_POINT:
            {
                err = hal_milan_dump_bad_point(data_encoder, dump_data, operation, cur_time_str);
                ret = true;  // bad point dump finished, no need common dump for bad point
                break;
            }

            case OPERATION_ENROLL:
            case OPERATION_TEST_UNTRUSTED_ENROLL:
            case OPERATION_TEST_PERFORMANCE:
            case OPERATION_AUTHENTICATE_IMAGE:
            case OPERATION_AUTHENTICATE_SLEEP:
            case OPERATION_AUTHENTICATE_FF:
            case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
            {
                // dump for broken mask in milan f/hv chip
                if (dump_data->swipe_enroll_flag > 0)
                {
                    err = hal_dump_swipe_enroll(data_encoder, dump_data, cur_time_str, error_code);
                    ret = true;  // return true, no need common dump
                }
                else
                {
                    err = hal_milan_dump_mask_by_opreation(data_encoder, dump_data,
                              error_code, operation, cur_time_str);
                    ret = false;  // return false, continue common dump
                }
                break;
            }

            default:
            {
                break;
            }
        }  // end switch
        BREAK_IF_ERROR(err);  // if dump fail, should break out of while
        err = gf_handle_dump_buf(data_encoder);
    }  // end of do while
    while (0);

    gf_dump_encoder_destroy(data_encoder);

    FUNC_EXIT(err);
    return ret;
}
