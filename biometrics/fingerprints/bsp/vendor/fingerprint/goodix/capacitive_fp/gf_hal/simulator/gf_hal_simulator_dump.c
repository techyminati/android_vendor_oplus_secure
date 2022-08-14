/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL layer simulator dump
 * History:
 * Version: 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gf_hal_simulator_dump.h"
#include "gf_hal_log.h"
#include "gf_dump_data.h"
#include "gf_dump_data_utils.h"
#include "gf_dump_bigdata.h"


#define LOG_TAG "[GF_HAL][gf_milan_f_dump]"

/*
Function: hal_fps_dump_reg_config
Description: dump config data
Input: data_encoder, dump data buffer.
           dir, the sub-dir for dump data.
           milan_config, milan chip config
           cur_time_str, current time string
Output:
Return: error code
Others:
*/
static gf_error_t hal_fps_dump_reg_config(dump_data_encoder_t* data_encoder,
                                     uint8_t* dir,
                                     milan_chip_config_t milan_config,
                                     uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dir || NULL == data_encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        snprintf((char*)file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_config", (char *)dir,
                 (char*)cur_time_str);
        err = gf_dump_encoder_fopen(data_encoder, file_path, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fprintf(data_encoder, "ff: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.ff.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.ff.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.fdt_manual_down.data_len / 16; i++)
        {
            err = gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.ff.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "fdt_manual_down: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.fdt_manual_down.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.fdt_manual_down.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.fdt_manual_down.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.fdt_manual_down.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "fdt_down: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.fdt_down.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.fdt_down.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.fdt_down.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.fdt_down.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "fdt_up: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.fdt_up.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.fdt_up.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.fdt_up.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.fdt_up.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "image: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.image.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.image.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.image.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.image.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav_fdt_down: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.nav_fdt_down.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav_fdt_down.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.nav_fdt_down.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav_fdt_down.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav_fdt_up: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.nav_fdt_up.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav_fdt_up.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.nav_fdt_up.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav_fdt_up.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n", milan_config.nav.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.nav.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fprintf(data_encoder, "nav_base: { \n");
        gf_dump_encoder_fprintf(data_encoder, "    start_address: 0x%x \n",
                milan_config.nav_base.start_address);
        gf_dump_encoder_fprintf(data_encoder, "    data_len: %d \n", milan_config.nav_base.data_len);
        gf_dump_encoder_fprintf(data_encoder, "    data: { \n");

        for (i = 0; i <= milan_config.nav_base.data_len / 16; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, "            ");

            for (j = 0; j < 16; j++)
            {
                gf_dump_encoder_fprintf(data_encoder, "0x%02X,", milan_config.nav_base.data[16 * i + j]);
            }

            gf_dump_encoder_fprintf(data_encoder, "\n");
        }

        gf_dump_encoder_fprintf(data_encoder, "    } \n");
        gf_dump_encoder_fprintf(data_encoder, "} \n");
        gf_dump_encoder_fclose(data_encoder);
    }  // end do
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_dump_device_info
Description: dump device info
Input: dev_info, the device info to be dump
Output: device info dump files
Return: error code
Others:
*/
static gf_error_t hal_fps_dump_device_info(gf_dev_info_t *dev_info)
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
        }

        gettimeofday(&tv, NULL);
        timestamp = gf_get_time_stamp(&tv);
        gf_get_time_string(&tv, cur_time, 1);

        err = gf_dump_encoder_create(&data_encoder, timestamp);
        if (NULL == data_encoder)
        {
            break;
        }

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_DEV_INFO);
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_otp_info", (char*)dir, (char*)cur_time);
        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fwrite((uint8_t *)dev_info->otp_info, GF_BIGDATA_SENSOR_OTP_INFO_LEN, data_encoder);
        gf_dump_encoder_fclose(data_encoder);

        err = hal_fps_dump_reg_config(data_encoder, dir, dev_info->g_milan_config, cur_time);
        BREAK_IF_ERROR(err);  // break if error

        err = gf_bigdata_dump_device_info(data_encoder, dir, dev_info, cur_time);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] write device info fail", __func__);
        }

        err = gf_handle_dump_buf(data_encoder);
    }  // end do
    while (0);

    gf_dump_encoder_destroy(data_encoder);
    FUNC_EXIT(err);
    return err;
}

/*
Function: hal_fps_dump_chip_init_data
Description: dump chip init data after HAL init finshed, actually only device info to be dump
Input: none
Output: device info dump files
Return: error code
Others:
*/
bool hal_fps_dump_chip_init_data()
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
        }
        cmd_dev_info = (gf_dev_info_t *) malloc(cmd_size);

        if (NULL == cmd_dev_info)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd_dev_info", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd_dev_info, 0, cmd_size);
        err = gf_hal_invoke_command(GF_CMD_GET_DEV_INFO, cmd_dev_info, cmd_size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        err = hal_fps_dump_device_info(cmd_dev_info);
    }
    while (0);

    if (NULL != cmd_dev_info)
    {
        free(cmd_dev_info);
    }

    FUNC_EXIT(err);
    return ret;
}


