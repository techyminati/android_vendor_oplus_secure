/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Long.Liu     2019/02/11        modify for coverity
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "gf_hal.h"
#include "gf_hal_log.h"
#include "gf_dump_data.h"
#include "gf_dump_data_utils.h"
#include "gf_dump_bigdata.h"

#define LOG_TAG "[GF_HAL][gf_dump_data]"

static uint8_t g_cur_authtime_str[5][GF_DUMP_FILE_PATH_MAX_LEN];
static uint8_t g_last_cur_time_str[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
static void dump_empty_dir(const uint8_t *dir)
{
    int32_t ret = 0;
    DIR *dp = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf = { 0 };

    if (NULL == (dp = opendir((char*)dir)))
    {
        LOG_D(LOG_TAG, "[%s] cannot open directory=%s", __func__, dir);
        return;
    }

    chdir((char*)dir);

    while (NULL != (entry = readdir(dp)))
    {
        ret = lstat(entry->d_name, &statbuf);

        if (0 != ret)
        {
            break;
        }

        if (S_ISREG(statbuf.st_mode))
        {
            ret = remove(entry->d_name);

            if (0 != ret)
            {
                break;
            }
        }
    }

    closedir(dp);
}

static gf_error_t dump_calibration_params(dump_data_encoder_t* data_encoder,
                                             gf_dump_data_t *dump_data,
                                             gf_error_t error_code,
                                             const uint8_t *dir,
                                             const uint8_t* result_str,
                                             const uint8_t *cur_time,
                                             uint32_t width,
                                             uint32_t height)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data
            || NULL == dir || NULL == result_str || NULL == cur_time)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
        // dump kr to ".csv" file
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_kr.csv", (char*)dir, (char*)cur_time, (char*)result_str);
        err = gf_dump_raw_data(data_encoder, filepath, (uint16_t*)dump_data->data.image.kr,
                    gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);

        // dump b to ".csv" file
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_b.csv", (char*)dir, (char*)cur_time,(char*)result_str);
        err = gf_dump_raw_data(data_encoder, filepath, (uint16_t*)dump_data->data.image.b,
                    gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);
        // dump basic information for base frame
        {
            uint32_t i = 0;
            uint8_t line[1024] = { 0 };
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_base_info.csv", (char*)dir,
                     (char*)cur_time,(char*)result_str);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_NORMAL_FILE_DATA);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] open file (%s) fail", __func__, filepath);
                break;
            }

            gf_dump_encoder_fwrite("preprocess version, ", strlen("preprocess version, "), data_encoder);
            gf_dump_encoder_fwrite(dump_data->data.image.preprocess_version,
                    strlen((char *) dump_data->data.image.preprocess_version), data_encoder);
            gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
            // sensor id
            gf_dump_encoder_fwrite("sensor id, ", strlen("sensor id, "), data_encoder);

            for (i = 0; i < GF_SENSOR_ID_LEN; i++)
            {
                snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->data.image.sensor_id[i]);
                gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
            }

            gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
            // chip id
            gf_dump_encoder_fwrite("chip id, ", strlen("chip id, "), data_encoder);

            for (i = 0; i < GF_CHIP_ID_LEN; i++)
            {
                snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->data.image.chip_id[i]);
                gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
            }

            gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
            // vendor id
            gf_dump_encoder_fwrite("vendor id, ", strlen("vendor id, "), data_encoder);

            for (i = 0; i < GF_VENDOR_ID_LEN; i++)
            {
                snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->data.image.vendor_id[i]);
                gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
            }

            gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
            gf_dump_encoder_fwrite("frame num, ", strlen("frame num, "), data_encoder);
            snprintf((char*)line, sizeof(line), "%d", dump_data->data.image.frame_num);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
            err = gf_dump_encoder_fclose(data_encoder);
        }
    }
    while (0);


    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_bmp_for_ee_image(gf_dump_data_t *dump_data, const uint8_t* dir,
    const uint8_t* result_str, const uint8_t* cur_time_str, const uint8_t* select_bmp_path)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    dump_data_encoder_t* data_encoder_ee = NULL;
    dump_data_encoder_t* data_encoder_before_ee = NULL;
#ifdef SUPPORT_D_PALM
    dump_data_encoder_t* data_encoder_palm = NULL;
#endif
    struct timeval tv = { 0 };
    FUNC_ENTER();

    do
    {
        err = gf_dump_encoder_create(&data_encoder_before_ee, gf_get_time_stamp(&tv));
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp_before_ee.bmp", (char*)dir,
            (char*)cur_time_str, (char*)result_str);
        err = gf_dump_encoder_fopen(data_encoder_before_ee, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp_before_ee.csv", (char*)dir,
            (char*)cur_time_str, (char*)result_str);
        gf_dump_encoder_add_path(data_encoder_before_ee, (uint8_t*)filepath);

        err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp_before_ee,
            gf_get_sensor_width() * gf_get_sensor_height(), data_encoder_before_ee);
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder_before_ee);
        err = gf_handle_dump_buf(data_encoder_before_ee);
        BREAK_IF_ERROR(err);  // break if error

        err = gf_dump_encoder_create(&data_encoder_ee, gf_get_time_stamp(&tv));
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp_ee.bmp", (char*)dir,
            (char*)cur_time_str, (char*)result_str);
        err = gf_dump_encoder_fopen(data_encoder_ee, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp_ee.csv", (char*)dir,
            (char*)cur_time_str, (char*)result_str);
        gf_dump_encoder_add_path(data_encoder_ee, (uint8_t*)filepath);

        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) data_encoder_ee->buf;
        buf->sensor_width = dump_data->data.image.ee_w;
        buf->sensor_height = dump_data->data.image.ee_h;
        LOG_E(LOG_TAG, "[%s]lishun ee_w<%d>", __func__,  dump_data->data.image.ee_w);
        LOG_E(LOG_TAG, "[%s]lishun ee_h<%d>", __func__,  dump_data->data.image.ee_h);
        err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp_ee,
                dump_data->data.image.ee_w * dump_data->data.image.ee_h, data_encoder_ee);

        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder_ee);
        err = gf_handle_dump_buf(data_encoder_ee);
        BREAK_IF_ERROR(err);  // break if error
#ifdef SUPPORT_D_PALM
        err = gf_dump_encoder_create(&data_encoder_palm, gf_get_time_stamp(&tv));
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp_palm.bmp", (char*)dir,
            (char*)cur_time_str, (char*)result_str);
        err = gf_dump_encoder_fopen(data_encoder_palm, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp_palm.csv", (char*)dir,
            (char*)cur_time_str, (char*)result_str);
        gf_dump_encoder_add_path(data_encoder_palm, (uint8_t*)filepath);

        buf = (dump_data_buffer_header_t*) data_encoder_palm->buf;
        buf->sensor_width = dump_data->data.image.pl_w;
        buf->sensor_height = dump_data->data.image.pl_h;
        LOG_E(LOG_TAG, "[%s]pl_w<%d>", __func__,  dump_data->data.image.pl_w);
        LOG_E(LOG_TAG, "[%s]pl_h<%d>", __func__,  dump_data->data.image.pl_h);

        err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp_temp,
            dump_data->data.image.pl_w * dump_data->data.image.pl_h, data_encoder_palm);
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder_palm);
        err = gf_handle_dump_buf(data_encoder_palm);
        BREAK_IF_ERROR(err);  // break if error
#endif
    } while (0);

    gf_dump_encoder_destroy(data_encoder_before_ee);
    gf_dump_encoder_destroy(data_encoder_ee);
#ifdef SUPPORT_D_PALM
    gf_dump_encoder_destroy(data_encoder_palm);
#endif

    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_bmp_for_ee_image_restore(gf_dump_data_t *dump_data, const uint8_t* dir, uint8_t index)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    dump_data_encoder_t* data_encoder_ee = NULL;
    dump_data_encoder_t* data_encoder_before_ee = NULL;
#ifdef SUPPORT_D_PALM
    dump_data_encoder_t* data_encoder_palm = NULL;
#endif
    struct timeval tv = { 0 };
    FUNC_ENTER();

    do
    {
        err = gf_dump_encoder_create(&data_encoder_before_ee, gf_get_time_stamp(&tv));
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp_before_ee.bmp",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        err = gf_dump_encoder_fopen(data_encoder_before_ee, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp_before_ee.csv",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        gf_dump_encoder_add_path(data_encoder_before_ee, (uint8_t*)filepath);

        err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp_before_ee,
            gf_get_sensor_width() * gf_get_sensor_height(), data_encoder_before_ee);
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder_before_ee);
        err = gf_handle_dump_buf(data_encoder_before_ee);
        BREAK_IF_ERROR(err);  // break if error

        err = gf_dump_encoder_create(&data_encoder_ee, gf_get_time_stamp(&tv));
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp_ee.bmp",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        err = gf_dump_encoder_fopen(data_encoder_ee, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp_ee.csv",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        gf_dump_encoder_add_path(data_encoder_ee, (uint8_t*)filepath);

        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) data_encoder_ee->buf;
        buf->sensor_width = dump_data->data.image.ee_w;
        buf->sensor_height = dump_data->data.image.ee_h;
        LOG_E(LOG_TAG, "[%s]lishun ee_w<%d>", __func__,  dump_data->data.image.ee_w);
        LOG_E(LOG_TAG, "[%s]lishun ee_h<%d>", __func__,  dump_data->data.image.ee_h);
        err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp_ee,
                dump_data->data.image.ee_w * dump_data->data.image.ee_h, data_encoder_ee);

        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder_ee);
        err = gf_handle_dump_buf(data_encoder_ee);
        BREAK_IF_ERROR(err);  // break if error
#ifdef SUPPORT_D_PALM
        err = gf_dump_encoder_create(&data_encoder_palm, gf_get_time_stamp(&tv));
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp_palm.bmp",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        err = gf_dump_encoder_fopen(data_encoder_palm, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp_palm.csv",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        gf_dump_encoder_add_path(data_encoder_palm, (uint8_t*)filepath);

        buf = (dump_data_buffer_header_t*) data_encoder_palm->buf;
        buf->sensor_width = dump_data->data.image.pl_w;
        buf->sensor_height = dump_data->data.image.pl_h;
        LOG_E(LOG_TAG, "[%s]pl_w<%d>", __func__,  dump_data->data.image.pl_w);
        LOG_E(LOG_TAG, "[%s]pl_h<%d>", __func__,  dump_data->data.image.pl_h);

        err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp_temp,
            dump_data->data.image.pl_w * dump_data->data.image.pl_h, data_encoder_palm);
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder_palm);
        err = gf_handle_dump_buf(data_encoder_palm);
        BREAK_IF_ERROR(err);  // break if error
#endif
    } while (0);

    gf_dump_encoder_destroy(data_encoder_before_ee);
    gf_dump_encoder_destroy(data_encoder_ee);
#ifdef SUPPORT_D_PALM
    gf_dump_encoder_destroy(data_encoder_palm);
#endif

    FUNC_EXIT(err);
    return err;
}
static gf_error_t dump_cali_and_bmp(dump_data_encoder_t* data_encoder,
                                    gf_dump_data_t *dump_data,
                                    gf_operation_type_t operation,
                                    gf_error_t error_code,
                                    const uint8_t* dir,
                                    const uint8_t* result_str,
                                    const uint8_t* cur_time_str,
                                    const uint8_t* select_bmp_path)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data
            || NULL == dir || NULL == result_str || NULL == cur_time_str)
        {
            err = GF_ERROR_CANCELED;
            break;
        }
        if (GF_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS == error_code
            || GF_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS == error_code)
        {
            err = GF_ERROR_CANCELED;
            break;
        }

        // dump bigdata info
        if (gf_is_dump_bigdata_enabled() == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_info.json", (char*)dir,
                    (char*)cur_time_str);
            err = gf_bigdata_dump_image_data(data_encoder, filepath, dump_data, operation, error_code);
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump calibration params
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_PARAMS) == 1)
        {
            err = dump_calibration_params(data_encoder, dump_data, error_code, dir, result_str,
                                       cur_time_str, gf_get_sensor_width(), gf_get_sensor_height());
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump caliRes
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_RES) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_calires.csv", (char*)dir,
                    (char*)cur_time_str, (char*)result_str);
            err = gf_dump_raw_data(data_encoder, (uint8_t*)filepath, dump_data->data.image.cali_res,
                        gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump dataBmp
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_DATA_BMP) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp.bmp", (char*)dir,
                    (char*)cur_time_str, (char*)result_str);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_databmp.csv", (char*)dir,
                    (char*)cur_time_str, (char*)result_str);
            gf_dump_encoder_add_path(data_encoder, (uint8_t*)filepath);
            if (dump_data->data.image.select_index == 0)
            {
                // dump select bmp
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
                strncpy(strstr((char*)select_bmp_path, ".bmp"), ".csv", strlen(".csv") + 1);
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
            }
            err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
        }

        // dump sitoBmp
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_SITO_BMP) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_sitobmp.bmp", (char*)dir,
                    (char*)cur_time_str, (char*)result_str);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_sitobmp.csv", (char*)dir,
                    (char*)cur_time_str, (char*)result_str);
            gf_dump_encoder_add_path(data_encoder, (uint8_t*)filepath);
            if (dump_data->data.image.select_index != 0)
            {
                // dump select bmp
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
                strncpy(strstr((char*)select_bmp_path, ".bmp"), ".csv", strlen(".csv") + 1);
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
            }
            err = gf_dump_encoder_fwrite(dump_data->data.image.sito_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
        }

        if (SUPPORT_IMAGE_EE_CONDITION(g_hal_config)
            && g_hal_config.support_dump_image_ee_bmp
                && (gf_is_operation_data_dump_allowed(operation, OP_DATA_DATA_BMP) == 1))
        {
            err = dump_bmp_for_ee_image(dump_data, dir,
                result_str, cur_time_str, select_bmp_path);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_cali_and_bmp2(dump_data_encoder_t* data_encoder,
                                    gf_dump_data_t *dump_data,
                                    gf_operation_type_t operation,
                                    gf_error_t error_code,
                                    const uint8_t* dir,
                                    const uint8_t* result_str,
                                    const uint8_t* cur_time_str,
                                    const uint8_t* select_bmp_path,
                                    const uint8_t* select_bmp_path2,
                                    const uint8_t* select_bmp_path3)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data
            || NULL == dir || NULL == result_str || NULL == cur_time_str)
        {
            err = GF_ERROR_CANCELED;
            break;
        }
        if (GF_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS == error_code
            || GF_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS == error_code)
        {
            err = GF_ERROR_CANCELED;
            break;
        }

        // dump bigdata info
        if (gf_is_dump_bigdata_enabled() == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_info.json", (char*)dir,
                    (char*)cur_time_str);
            err = gf_bigdata_dump_image_data(data_encoder, filepath, dump_data, operation, error_code);
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump calibration params
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_PARAMS) == 1)
        {
            err = dump_calibration_params(data_encoder, dump_data, error_code, dir, result_str,
                                       cur_time_str, gf_get_sensor_width(), gf_get_sensor_height());
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump dataBmp
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_DATA_BMP) == 1)
        {
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)select_bmp_path, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)select_bmp_path2, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            err = gf_dump_encoder_fwrite(dump_data->data.image.data2_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)select_bmp_path3, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            err = gf_dump_encoder_fwrite(dump_data->data.image.share.reserve.dataf_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_broken_check_data(dump_data_encoder_t* data_encoder,
                                         gf_dump_data_t *dump_data,
                                         gf_operation_type_t operation,
                                         gf_error_t error_code,
                                         const uint8_t* dir,
                                         const uint8_t* result_str,
                                         const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    int32_t i = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == data_encoder
            || NULL == dir || NULL == result_str || NULL == cur_time_str)
        {
            err = GF_ERROR_CANCELED;
            break;
        }

        if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == error_code)
        {
            err = GF_ERROR_CANCELED;
            break;
        }

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_BROKEN_CHECK_RAW_DATA) == 0)
        {
            break;
        }

        // dump broken check data
        for (i = 0; i < dump_data->data.image.broken_check_frame_num; i++)
        {
            // dump caliRes
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_sensor_broken_%d.csv",
                     (char*)dir, (char*)cur_time_str, (char*)result_str, i);
            err = gf_dump_raw_data(data_encoder,
                                filepath,
                                dump_data->data.image.broken_check_raw_data[i],
                                gf_get_sensor_width() * gf_get_sensor_height(),
                                DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }

        if (dump_data->data.image.broken_check_base_len > 0)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_sensor_broken_base.csv",
                     (char*)dir, (char*)cur_time_str, (char*)result_str);
            err = gf_dump_raw_data(data_encoder,
                                filepath,
                                dump_data->data.image.broken_check_base,
                                gf_get_sensor_width() * gf_get_sensor_height(),
                                DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }

        if (GF_ERROR_SENSOR_IS_BROKEN == error_code
            || GF_ERROR_PREPROCESS_FAILED == error_code)
        {
            rmdir((char*)dir);  // TODO remove empty dir here
            err = GF_ERROR_CANCELED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_asp_data_info(gf_dump_reserve_t *dump_data,
                                     gf_operation_type_t operation,
                                     gf_error_t error_code,
                                     const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t result_str[GF_DUMP_RESULT_STR_MAX_LEN] = { 0 };
    uint32_t i = 0;
    uint8_t line[1024] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    dump_data_encoder_t* data_encoder = NULL;
    gf_dump_data_t *dump_data_tmp = NULL;
    struct timeval tv = { 0 };
    uint32_t feature_size = 0;
    FUNC_ENTER();

    do
    {
        dump_data_tmp = (gf_dump_data_t *) malloc(sizeof(gf_dump_data_t));
        if (NULL == dump_data || NULL == cur_time_str || NULL == dump_data_tmp)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        dump_data_tmp->data.image.authenticate_retry_count = dump_data->authenticate_retry_count;
        gf_get_operation_result_str(dump_data_tmp, error_code, result_str, GF_DUMP_RESULT_STR_MAX_LEN);

        if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == operation)
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_UNTRUSTED_AUTHENTICATE_DIR);
        }
        else
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_AUTHENTICATE_DIR);
        }

        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            feature_size = 60;
        }
        else
        {
            feature_size = 80;
        }
        if (1 == dump_data->is_only_dump_broken_check)
        {
            break;
        }
        err = gf_dump_encoder_create(&data_encoder, gf_get_time_stamp(&tv));
            BREAK_IF_ERROR(err);  // break if error
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                 "%s%s_%s_q%d_c%d_s%d_fid%u_fl%d_p%.6f_sti%d_up%d_st%d_stf%d_tf%d_sflist.csv",
                 (char*)dir, (char*)cur_time_str, (char*)result_str,
                 dump_data->image_quality,
                 dump_data->valid_area,
                 dump_data->match_score,
                 dump_data->match_finger_id,
                 dump_data->asp_level,
                 dump_data->asp_probability,
                 dump_data->asp_timeinterval,
                 dump_data->asp_updateflag,
                 dump_data->asp_sensortype,
                 dump_data->asp_stesting_flag,
                 dump_data->asp_testing_flag);
        err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fwrite("asp_version", strlen("asp_version"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite(dump_data->asp_version,
                    strlen((char *) dump_data->asp_version), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("algo_version", strlen("algo_version"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite(dump_data->algo_version,
                    strlen((char *) dump_data->algo_version), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("diff_use", strlen("diff_use"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        snprintf((char*)line, sizeof(line), "%d", dump_data->diff_use);
        gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("tcode", strlen("tcode"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        snprintf((char*)line, sizeof(line), "%d", dump_data->asp_tcode);
        gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("dac", strlen("dac"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        for (i = 0; i < 4; i++)
        {
            snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->default_dac[i]);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        }
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("chip id", strlen("chip id"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        for (i = 0; i < GF_CHIP_ID_LEN; i++)
        {
            snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->chip_id[i]);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        }
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("asp_template", strlen("asp_template"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        for (i = 0; i < feature_size; i++)
        {
            snprintf((char*)line, sizeof(line), "%.6f\n", dump_data->asp_template_feature[i]);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        }
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("asp_sample", strlen("asp_sample"), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        for (i = 0; i < feature_size; i++)
        {
            snprintf((char*)line, sizeof(line), "%.6f\n", dump_data->asp_sample_feature[i]);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        }
        // LOG_E(LOG_TAG, "[%s] linzy asp_sample_feature[0] %d", __func__, (int32_t)(dump_data->data.image.asp_sample_feature[0] * 1000));
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder);
        err = gf_handle_dump_buf(data_encoder);
        BREAK_IF_ERROR(err);  // break if error
    } while (0);

    if (dump_data_tmp != NULL) {
        free(dump_data_tmp);
        dump_data_tmp = NULL;
    }

    gf_dump_encoder_destroy(data_encoder);
    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_ENROLL, OPERATION_TEST_UNTRUSTED_ENROLL
static gf_error_t dump_enroll(dump_data_encoder_t* data_encoder,
                              gf_dump_data_t *dump_data,
                              gf_operation_type_t operation,
                              gf_error_t error_code,
                              const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path2[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path3[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t result_str[GF_DUMP_RESULT_STR_MAX_LEN] = { 0 };
    uint32_t members = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == data_encoder || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        members = gf_get_sensor_width() * gf_get_sensor_height();
        gf_get_operation_result_str(dump_data, error_code, result_str, GF_DUMP_RESULT_STR_MAX_LEN);

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

        // TODO(goodix): dump origin data

        err = dump_broken_check_data(data_encoder, dump_data, operation,
                                     error_code, dir, result_str, cur_time_str);
        BREAK_IF_ERROR(err);  // break if error
        if (1 == dump_data->is_only_dump_broken_check)
        {
            break;
        }

        // dump raw data
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            if (g_hal_config.support_enroll_select_better_image)
            {
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_0_se%d_rawdata.csv",
                         (char*)dir, (char*)cur_time_str, (char*)result_str,
                         dump_data->data.image.enroll_select_index);
                err = gf_dump_raw_data(data_encoder, filepath, dump_data->data.image.raw_data,
                                        members, DATA_TYPE_RAW_DATA);
                BREAK_IF_ERROR(err);  // break if error
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_1_se%d_rawdata.csv",
                         (char*)dir, (char*)cur_time_str, (char*)result_str,
                         dump_data->data.image.enroll_select_index);
                err = gf_dump_raw_data(data_encoder, filepath,
                                        dump_data->data.image.raw_data + members,
                                        members, DATA_TYPE_RAW_DATA);
                BREAK_IF_ERROR(err);  // break if error
            }
            else
            {
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_rawdata.csv",
                         (char*)dir, (char*)cur_time_str, (char*)result_str);
                err = gf_dump_raw_data(data_encoder, filepath, dump_data->data.image.raw_data,
                                        members, DATA_TYPE_RAW_DATA);
                BREAK_IF_ERROR(err);  // break if error
                /*
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_rawdata1.csv",
                         (char*)dir, (char*)cur_time_str, (char*)result_str);
                err = gf_dump_raw_data(data_encoder, filepath, &(dump_data->data.image.raw_data[160*32]),
                                        members, DATA_TYPE_RAW_DATA);
                BREAK_IF_ERROR(err);  // break if error
                */
            }
        }

        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            // dump Touchmask
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
	             "%s%s_%s_selectbmp_%d_%d_%d_%d_%u_TouMask.csv", dir, cur_time_str,
	             result_str, dump_data->data.image.image_quality,
	             dump_data->data.image.valid_area,
	             dump_data->data.image.overlap_rate_between_last_template,
	             dump_data->data.image.increase_rate_between_stitch_info,
	             dump_data->data.image.duplicated_finger_id);
            err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, filepath);
                break;
            }
            err = gf_dump_encoder_fwrite(dump_data->data.image.share.reserve.touch_mask, members, data_encoder);
            gf_dump_encoder_fclose(data_encoder);

        }
        // dump Brokenmask
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
	             "%s%s_%s_selectbmp_%d_%d_%d_%d_%u_BroMask.csv", dir, cur_time_str,
	             result_str, dump_data->data.image.image_quality,
	             dump_data->data.image.valid_area,
	             dump_data->data.image.overlap_rate_between_last_template,
	             dump_data->data.image.increase_rate_between_stitch_info,
	             dump_data->data.image.duplicated_finger_id);

        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, filepath);
            break;
        }
        err = gf_dump_encoder_fwrite(dump_data->data.image.broken_mask, members, data_encoder);
        gf_dump_encoder_fclose(data_encoder);

        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            if (OPERATION_ENROLL == operation)
            {
                snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                         "%s%s_%s_selectbmp_%d_%d_%d_%d_%u_enc.bmp", dir, cur_time_str,
                         result_str, dump_data->data.image.image_quality,
                         dump_data->data.image.valid_area,
                         dump_data->data.image.overlap_rate_between_last_template,
                         dump_data->data.image.increase_rate_between_stitch_info,
                         dump_data->data.image.duplicated_finger_id);
                snprintf((char*)select_bmp_path2, GF_DUMP_FILE_PATH_MAX_LEN,
                         "%s%s_%s_selectdecbmp_%d_%d_%d_%d_%u.bmp", dir, cur_time_str,
                         result_str, dump_data->data.image.image_quality,
                         dump_data->data.image.valid_area,
                         dump_data->data.image.overlap_rate_between_last_template,
                         dump_data->data.image.increase_rate_between_stitch_info,
                         dump_data->data.image.duplicated_finger_id);
                snprintf((char*)select_bmp_path3, GF_DUMP_FILE_PATH_MAX_LEN,
                         "%s%s_%s_selectfbmp_%d_%d_%d_%d_%u.bmp", dir, cur_time_str,
                         result_str, dump_data->data.image.image_quality,
                         dump_data->data.image.valid_area,
                         dump_data->data.image.overlap_rate_between_last_template,
                         dump_data->data.image.increase_rate_between_stitch_info,
                         dump_data->data.image.duplicated_finger_id);
            }
            else
            {
                snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                        "%s%s_%s_selectbmp_%d_%d_%d_%u_%d_enc.bmp", dir, cur_time_str,
                        result_str, dump_data->data.image.image_quality,
                        dump_data->data.image.valid_area,
                        dump_data->data.image.match_score,
                        dump_data->data.image.match_finger_id,
                        dump_data->data.image.study_flag);
                snprintf((char*)select_bmp_path2, GF_DUMP_FILE_PATH_MAX_LEN,
                        "%s%s_%s_selectbmp_%d_%d_%d_%u_%d.bmp", dir, cur_time_str,
                        result_str, dump_data->data.image.image_quality,
                        dump_data->data.image.valid_area,
                        dump_data->data.image.match_score,
                        dump_data->data.image.match_finger_id,
                        dump_data->data.image.study_flag);
                snprintf((char*)select_bmp_path3, GF_DUMP_FILE_PATH_MAX_LEN,
                        "%s%s_%s_selectfbmp_%d_%d_%d_%u_%d.bmp", dir, cur_time_str,
                        result_str, dump_data->data.image.image_quality,
                        dump_data->data.image.valid_area,
                        dump_data->data.image.match_score,
                        dump_data->data.image.match_finger_id,
                        dump_data->data.image.study_flag);
            }
            err = dump_cali_and_bmp2(data_encoder, dump_data, operation,
                                     error_code, dir, result_str, cur_time_str,
                                     select_bmp_path, select_bmp_path2, select_bmp_path3);
        }
        else
        {
            UNUSED_VAR(select_bmp_path2);
            UNUSED_VAR(select_bmp_path3);
            if (OPERATION_ENROLL == operation)
            {
                snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                         "%s%s_%s_selectbmp_%d_%d_%d_%d_%u.bmp", dir, cur_time_str,
                         result_str, dump_data->data.image.image_quality,
                         dump_data->data.image.valid_area,
                         dump_data->data.image.overlap_rate_between_last_template,
                         dump_data->data.image.increase_rate_between_stitch_info,
                         dump_data->data.image.duplicated_finger_id);
            }
            else
            {
                snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                        "%s%s_%s_selectbmp_%d_%d_%d_%u_%d.bmp", dir, cur_time_str,
                        result_str, dump_data->data.image.image_quality,
                        dump_data->data.image.valid_area,
                        dump_data->data.image.match_score,
                        dump_data->data.image.match_finger_id,
                        dump_data->data.image.study_flag);
            }
            err = dump_cali_and_bmp(data_encoder, dump_data, operation,
                        error_code, dir, result_str, cur_time_str, select_bmp_path);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_PERFORMANCE
static gf_error_t dump_test_performance(dump_data_encoder_t* data_encoder,
                                        gf_dump_data_t *dump_data,
                                        gf_operation_type_t operation,
                                        gf_error_t error_code,
                                        const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path2[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path3[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t result_str[GF_DUMP_RESULT_STR_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_get_operation_result_str(dump_data, error_code, result_str, GF_DUMP_RESULT_STR_MAX_LEN);

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_PERFORMANCE_DIR);

        // TODO(goodix): dump origin data

        err = dump_broken_check_data(data_encoder, dump_data, operation,
                error_code, dir, result_str, cur_time_str);
        BREAK_IF_ERROR(err);  // break if error

        // dump raw data
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_rawdata.csv",
                    (char*)dir, (char*)cur_time_str, (char*)result_str);
            err = gf_dump_raw_data(data_encoder,
                            filepath,
                            dump_data->data.image.raw_data,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }

        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_%s_selectbmp_%d_%d_enc.bmp", (char*)dir, (char*)cur_time_str,
                     result_str, dump_data->data.image.image_quality,
                     dump_data->data.image.valid_area);
            snprintf((char*)select_bmp_path2, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_%s_selectbmp_%d_%d.bmp", (char*)dir, (char*)cur_time_str,
                     result_str, dump_data->data.image.image_quality,
                     dump_data->data.image.valid_area);
            snprintf((char*)select_bmp_path3, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_%s_selectfbmp_%d_%d.bmp", (char*)dir, (char*)cur_time_str,
                     result_str, dump_data->data.image.image_quality,
                     dump_data->data.image.valid_area);
            err = dump_cali_and_bmp2(data_encoder, dump_data, operation,
                                     error_code, dir, result_str, cur_time_str,
                                     select_bmp_path, select_bmp_path2, select_bmp_path3);
        }
        else
        {
            UNUSED_VAR(select_bmp_path2);
            UNUSED_VAR(select_bmp_path3);
            snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_%s_selectbmp_%d_%d.bmp", (char*)dir, (char*)cur_time_str,
                     result_str, dump_data->data.image.image_quality,
                     dump_data->data.image.valid_area);
            err = dump_cali_and_bmp(data_encoder, dump_data, operation,
                        error_code, dir, result_str, cur_time_str, select_bmp_path);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_calibration_params_restore(dump_data_encoder_t* data_encoder,
                                    gf_dump_data_t *dump_data, const uint8_t* dir, uint8_t index)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t line[1024] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data || NULL == dir)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        // dump kr to ".csv" file
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_kr.csv",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        err = gf_dump_raw_data(data_encoder, filepath, (uint16_t*)(dump_data->data.image.kr),
                    gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);
        BREAK_IF_ERROR(err);  // break if error
        // dump b to ".csv" file
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_b.csv",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        err = gf_dump_raw_data(data_encoder, filepath, (uint16_t*)(dump_data->data.image.b),
                    gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);
        BREAK_IF_ERROR(err);  // break if error

        // dump basic information for base frame
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_base_info.csv",
                 (char*)dir, (char*)g_cur_authtime_str[index], index);
        err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_NORMAL_FILE_DATA);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] open file (%s) fail", __func__, filepath);
            break;
        }

        gf_dump_encoder_fwrite("preprocess version, ", strlen("preprocess version, "), data_encoder);
        gf_dump_encoder_fwrite(dump_data->data.image.preprocess_version,
                strlen((char *) dump_data->data.image.preprocess_version), data_encoder);
        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        // sensor id
        gf_dump_encoder_fwrite("sensor id, ", strlen("sensor id, "), data_encoder);

        for (i = 0; i < GF_SENSOR_ID_LEN; i++)
        {
            snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->data.image.sensor_id[i]);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        }

        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        // chip id
        gf_dump_encoder_fwrite("chip id, ", strlen("chip id, "), data_encoder);

        for (i = 0; i < GF_CHIP_ID_LEN; i++)
        {
            snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->data.image.chip_id[i]);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        }

        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        // vendor id
        gf_dump_encoder_fwrite("vendor id, ", strlen("vendor id, "), data_encoder);

        for (i = 0; i < GF_VENDOR_ID_LEN; i++)
        {
            snprintf((char*)line, sizeof(line), "0x%02X, ", dump_data->data.image.vendor_id[i]);
            gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        }

        gf_dump_encoder_fwrite("\n", strlen("\n"), data_encoder);
        gf_dump_encoder_fwrite("frame num, ", strlen("frame num, "), data_encoder);
        snprintf((char*)line, sizeof(line), "%d", dump_data->data.image.frame_num);
        gf_dump_encoder_fwrite(line, strlen((char*)line), data_encoder);
        err = gf_dump_encoder_fclose(data_encoder);
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_cali_and_bmp_restore(dump_data_encoder_t* data_encoder,
                                             gf_dump_data_t *dump_data,
                                             gf_operation_type_t operation,
                                             const uint8_t* dir,
                                             const uint8_t* select_bmp_path,
                                             uint8_t index)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data || NULL == dir)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        // dump calibration params
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_PARAMS) == 1)
        {
            err = dump_calibration_params_restore(data_encoder, dump_data, dir, index);
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump caliRes
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_RES) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_calires.csv", (char*)dir,
                    (char*)g_cur_authtime_str[index], index);
            err = gf_dump_raw_data(data_encoder, (uint8_t*)filepath, dump_data->data.image.cali_res,
                        gf_get_sensor_width() * gf_get_sensor_height(), DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump dataBmp
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_DATA_BMP) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp.bmp", (char*)dir,
                    (char*)g_cur_authtime_str[index], index);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_databmp.csv", (char*)dir,
                    (char*)g_cur_authtime_str[index], index);
            gf_dump_encoder_add_path(data_encoder, (uint8_t*)filepath);
            if (dump_data->data.image.select_index == 0)
            {
                // dump select bmp
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
                strncpy(strstr((char*)select_bmp_path, ".bmp"), ".csv", strlen(".csv") + 1);
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
            }
            err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
        }

        // dump sitoBmp
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_SITO_BMP) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_sitobmp.bmp", (char*)dir,
                    (char*)g_cur_authtime_str[index], index);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_retry_%d_code_1064_fail_sitobmp.csv", (char*)dir,
                    (char*)g_cur_authtime_str[index], index);
            gf_dump_encoder_add_path(data_encoder, (uint8_t*)filepath);
            if (dump_data->data.image.select_index != 0)
            {
                // dump select bmp
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
                strncpy(strstr((char*)select_bmp_path, ".bmp"), ".csv", strlen(".csv") + 1);
                gf_dump_encoder_add_path(data_encoder, (uint8_t*)select_bmp_path);
            }
            err = gf_dump_encoder_fwrite(dump_data->data.image.sito_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
        }

        if (SUPPORT_IMAGE_EE_CONDITION(g_hal_config)
            && g_hal_config.support_dump_image_ee_bmp
                && (gf_is_operation_data_dump_allowed(operation, OP_DATA_DATA_BMP) == 1))
        {
            err = dump_bmp_for_ee_image_restore(dump_data, dir, index);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t dump_cali_and_bmp2_restore(dump_data_encoder_t* data_encoder,
                                             gf_dump_data_t *dump_data,
                                             gf_operation_type_t operation,
                                             const uint8_t* dir,
                                             uint8_t index)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data || NULL == dir)
        {
            err = GF_ERROR_CANCELED;
            break;
        }

        // dump calibration params
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_PARAMS) == 1)
        {
            err = dump_calibration_params_restore(data_encoder, dump_data, dir, index);
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump dataBmp
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_DATA_BMP) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                      "%s%s_retry_%d_code_1064_fail_selectbmp_%d_%d_0_0_0_enc.bmp", (char*)dir,
                      (char*)g_cur_authtime_str[index], index,
                       dump_data->data.image.image_quality,
                       dump_data->data.image.valid_area);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            err = gf_dump_encoder_fwrite(dump_data->data.image.data_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                      "%s%s_retry_%d_code_1064_fail_selectbmp_%d_%d_0_0_0.bmp", (char*)dir,
                      (char*)g_cur_authtime_str[index], index,
                       dump_data->data.image.image_quality,
                       dump_data->data.image.valid_area);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            err = gf_dump_encoder_fwrite(dump_data->data.image.data2_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                      "%s%s_retry_%d_code_1064_fail_selectfbmp_%d_%d_0_0_0.bmp", (char*)dir,
                      (char*)g_cur_authtime_str[index], index,
                       dump_data->data.image.image_quality,
                       dump_data->data.image.valid_area);
            err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            err = gf_dump_encoder_fwrite(dump_data->data.image.share.reserve.dataf_bmp,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_dump_auth_retry_data(dump_data_encoder_t* data_encoder,
                                   gf_dump_data_t *dump_data,
                                   gf_operation_type_t operation,
                                   uint8_t index)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };

    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == operation)
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_UNTRUSTED_AUTHENTICATE_DIR);
        }
        else
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_AUTHENTICATE_DIR);
        }
        // restore rawdata
        LOG_E(LOG_TAG, "[%s] link:index=%d", __func__, index);
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_retry_%d_code_1064_fail_rawdata.csv",
                     (char*)dir, (char*)g_cur_authtime_str[index], index);
            err = gf_dump_raw_data(data_encoder,
                                   filepath,
                                   dump_data->data.image.raw_data,
                                   gf_get_sensor_width() * gf_get_sensor_height(),
                                   DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }
        // restore touchmask&&broken_mask
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            // dump Touchmask
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_retry_%d_code_1064_fail_selectbmp_%d_%d_0_0_0_TouMask.csv", dir,
                     (char*)g_cur_authtime_str[index], index,
                     dump_data->data.image.image_quality,
                     dump_data->data.image.valid_area);
            err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
            BREAK_IF_ERROR(err);  // break if error
            err = gf_dump_encoder_fwrite(dump_data->data.image.share.reserve.touch_mask,
                        gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
            gf_dump_encoder_fclose(data_encoder);
        }
        // dump Brokenmask
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                 "%s%s_retry_%d_code_1064_fail_selectbmp_%d_%d_0_0_0_BroMask.csv", dir,
                 (char*)g_cur_authtime_str[index], index,
                 dump_data->data.image.image_quality,
                 dump_data->data.image.valid_area);
        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
        BREAK_IF_ERROR(err);  // break if error
        err = gf_dump_encoder_fwrite(dump_data->data.image.broken_mask,
                    gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder);
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            dump_cali_and_bmp2_restore(data_encoder, dump_data, operation, dir, index);
        }
        else
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                    "%s%s_retry_%d_code_1064_fail_selectbmp_%d_%d_%d_%u_%d.bmp", (char*)dir,
                    (char*)g_cur_authtime_str[index], index,
                    dump_data->data.image.image_quality,
                    dump_data->data.image.valid_area,
                    dump_data->data.image.match_score,
                    dump_data->data.image.match_finger_id,
                    dump_data->data.image.study_flag);
            dump_cali_and_bmp_restore(data_encoder, dump_data, operation, dir, filepath, index);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}
// dump OPERATION_AUTHENTICATE_IMAGE,
//      OPERATION_AUTHENTICATE_SLEEP,
//      OPERATION_AUTHENTICATE_FF,
//      OPERATION_TEST_UNTRUSTED_AUTHENTICATE
static gf_error_t dump_authenticate(dump_data_encoder_t* data_encoder,
                                    gf_dump_data_t *dump_data,
                                    gf_operation_type_t operation,
                                    gf_error_t error_code,
                                    const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path2[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t select_bmp_path3[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t result_str[GF_DUMP_RESULT_STR_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_get_operation_result_str(dump_data, error_code, result_str, GF_DUMP_RESULT_STR_MAX_LEN);

        if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == operation)
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_UNTRUSTED_AUTHENTICATE_DIR);
        }
        else
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_AUTHENTICATE_DIR);
        }

        // TODO(goodix): dump origin data

        err = dump_broken_check_data(data_encoder, dump_data, operation,
                error_code, dir, result_str, cur_time_str);
        BREAK_IF_ERROR(err);  // break if error
        if (1 == dump_data->is_only_dump_broken_check)
        {
            break;
        }

        // dump raw data
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_rawdata.csv",
                    (char*)dir, (char*)cur_time_str, (char*)result_str);

            err = gf_dump_raw_data(data_encoder,
                            filepath,
                            dump_data->data.image.raw_data,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }

        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            // dump Touchmask
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_%s_selectbmp_%d_%d_%d_%u_%d_TouMask.csv", dir, cur_time_str,
                     result_str, dump_data->data.image.image_quality,
                     dump_data->data.image.valid_area,
                     dump_data->data.image.match_score,
                     dump_data->data.image.match_finger_id,
                     dump_data->data.image.study_flag);
            err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, filepath);
                break;
            }
            err = gf_dump_encoder_fwrite(dump_data->data.image.share.reserve.touch_mask,
                        gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
            gf_dump_encoder_fclose(data_encoder);
        }
        // dump Brokenmask
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%s%s_%s_selectbmp_%d_%d_%d_%u_%d_BroMask.csv", dir, cur_time_str,
                     result_str, dump_data->data.image.image_quality,
                     dump_data->data.image.valid_area,
                     dump_data->data.image.match_score,
                     dump_data->data.image.match_finger_id,
                     dump_data->data.image.study_flag);

        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_IMAGE_DATA);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] open encoder file failed,<%s>", __func__, filepath);
            break;
        }
        err = gf_dump_encoder_fwrite(dump_data->data.image.broken_mask,
                        gf_get_sensor_width() * gf_get_sensor_height(), data_encoder);
        gf_dump_encoder_fclose(data_encoder);

        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                    "%s%s_%s_selectbmp_%d_%d_%d_%u_%d_enc.bmp", (char*)dir, (char*)cur_time_str,
                    result_str, dump_data->data.image.image_quality,
                    dump_data->data.image.valid_area,
                    dump_data->data.image.match_score,
                    dump_data->data.image.match_finger_id,
                    dump_data->data.image.study_flag);
            snprintf((char*)select_bmp_path2, GF_DUMP_FILE_PATH_MAX_LEN,
                    "%s%s_%s_selectbmp_%d_%d_%d_%u_%d.bmp", (char*)dir, (char*)cur_time_str,
                    result_str, dump_data->data.image.image_quality,
                    dump_data->data.image.valid_area,
                    dump_data->data.image.match_score,
                    dump_data->data.image.match_finger_id,
                    dump_data->data.image.study_flag);
            snprintf((char*)select_bmp_path3, GF_DUMP_FILE_PATH_MAX_LEN,
                    "%s%s_%s_selectfbmp_%d_%d_%d_%u_%d.bmp", (char*)dir, (char*)cur_time_str,
                    result_str, dump_data->data.image.image_quality,
                    dump_data->data.image.valid_area,
                    dump_data->data.image.match_score,
                    dump_data->data.image.match_finger_id,
                    dump_data->data.image.study_flag);

            err = dump_cali_and_bmp2(data_encoder, dump_data, operation,
                                     error_code, dir, result_str, cur_time_str,
                                     select_bmp_path, select_bmp_path2, select_bmp_path3);
        }
        else
        {
            UNUSED_VAR(select_bmp_path2);
            UNUSED_VAR(select_bmp_path3);
            snprintf((char*)select_bmp_path, GF_DUMP_FILE_PATH_MAX_LEN,
                    "%s%s_%s_selectbmp_%d_%d_%d_%u_%d.bmp", (char*)dir, (char*)cur_time_str,
                    result_str, dump_data->data.image.image_quality,
                    dump_data->data.image.valid_area,
                    dump_data->data.image.match_score,
                    dump_data->data.image.match_finger_id,
                    dump_data->data.image.study_flag);

            err = dump_cali_and_bmp(data_encoder, dump_data, operation,
                        error_code, dir, result_str, cur_time_str, select_bmp_path);
        }
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] dump cali_bmp error, break", __func__);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_FINGER_BASE
static gf_error_t dump_finger_base(dump_data_encoder_t* data_encoder,
                                   gf_dump_data_t *dump_data,
                                   gf_operation_type_t operation,
                                   gf_error_t error_code,
                                   const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t result_str[GF_DUMP_RESULT_STR_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_get_operation_result_str(dump_data, error_code, result_str, GF_DUMP_RESULT_STR_MAX_LEN);

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_FINGER_BASE_DIR);
        // TODO(goodix): dump origin data

        if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == error_code)
        {
            break;
        }

        // dump bigdata info
        if (gf_is_dump_bigdata_enabled() == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_info.json", (char*)dir,
                    (char*)cur_time_str);
            err = gf_bigdata_dump_image_data(data_encoder, filepath, dump_data, operation, error_code);
            BREAK_IF_ERROR(err);  // break if error
        }

        // dump raw data
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%s_rawdata.csv", (char*)dir,
                    (char*)cur_time_str, (char*)result_str);
            err = gf_dump_raw_data(data_encoder,
                            (uint8_t*)filepath,
                            dump_data->data.image.raw_data,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
            BREAK_IF_ERROR(err);  // break if error
        }

        if (GF_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS == error_code
                || GF_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS == error_code)
        {
            break;
        }

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_PARAMS) == 1)
        {
            err = dump_calibration_params(data_encoder, dump_data, error_code, dir, result_str,
                                 cur_time_str, gf_get_sensor_width(), gf_get_sensor_height());
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_PIXEL_OPEN_STEP1, OPERATION_TEST_PIXEL_OPEN_STEP2
static gf_error_t dump_test_pixel_open_step(dump_data_encoder_t* data_encoder,
                                            gf_dump_data_t *dump_data,
                                            gf_operation_type_t operation,
                                            uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t image_index[4] = {0};
    uint32_t i = 0, cur_time_len = strlen((char *)cur_time_str);
    uint32_t frame_num = dump_data->data.image.frame_num;

    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            cur_time_len = strlen((char *)cur_time_str);
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_PIXEL_OPEN_DIR);

            // OPERATION_TEST_PIXEL_OPEN_STEP2 : CHICAGO-H-HV,CHICAGO-HS-HV frame_num=4
            // other frame_num =2
            if (0 == frame_num)
            {
                frame_num = 1;
            }

            for (i = 0; i < frame_num; i++)
            {
                if (OPERATION_TEST_PIXEL_OPEN_STEP1 == operation)
                {
                    memset(image_index, 0, sizeof(image_index));
                    sprintf((char*)image_index, "%d", i);
                    strncat((char *) cur_time_str, (const char*)"_", strlen("_"));
                    strncat((char *) cur_time_str, (char*)image_index, strlen((char*)image_index));
                }
                else
                {
                    memset(image_index, 0, sizeof(image_index));
                    memset(cur_time_str + cur_time_len, 0, GF_DUMP_FILE_PATH_MAX_LEN - cur_time_len);
                    sprintf((char*)image_index, "%d", i + 1);
                    strncat((char *) cur_time_str, (const char*)"_", strlen("_"));
                    strncat((char *) cur_time_str, (char*)image_index, strlen((char*)image_index));
                }

                strncpy((char*)filepath, (char*)dir, strlen((char*)dir) + 1);
                strncat((char*)filepath, (char*)cur_time_str, strlen((char*)cur_time_str));
                strncat((char*)filepath, ".csv", strlen(".csv"));
                err = gf_dump_raw_data(data_encoder,
                            filepath,
                            dump_data->data.image.raw_data + i * gf_get_sensor_width() * gf_get_sensor_height(),
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
            }
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_PIXEL_SHORT_STREAK
static gf_error_t dump_test_pixel_short_streak(dump_data_encoder_t* data_encoder,
                                            gf_dump_data_t *dump_data,
                                            gf_operation_type_t operation,
                                            uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };

    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_PIXEL_SHORT_STREAK_DIR);

            strncpy((char*)filepath, (char*)dir, strlen((char*)dir) + 1);
            strncat((char*)filepath, (char*)cur_time_str, strlen((char*)cur_time_str));
            strncat((char*)filepath, ".csv", strlen(".csv"));
            err = gf_dump_raw_data(data_encoder,
                        filepath,
                        dump_data->data.image.raw_data,
                        gf_get_sensor_width() * gf_get_sensor_height(),
                        DATA_TYPE_RAW_DATA);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_SENSOR_FINE_STEP1, OPERATION_TEST_SENSOR_FINE_STEP2
static gf_error_t dump_test_sensor_fine_step(dump_data_encoder_t* data_encoder,
                                             gf_dump_data_t *dump_data,
                                             gf_operation_type_t operation,
                                             uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_TEST_SENSOR_FINE_DIR);

            if (OPERATION_TEST_SENSOR_FINE_STEP1 == operation)
            {
                strncat((char *) cur_time_str, (const char *) "_debug", strlen("_debug"));
            }
            else
            {
                strncat((char*)cur_time_str, "_debug_no_tx", strlen("_debug_no_tx"));
            }

            strncpy((char*)filepath, (char*)dir, strlen((char*)dir) + 1);
            strncat((char*)filepath, (char*)cur_time_str, strlen((char*)cur_time_str));
            strncat((char*)filepath, ".csv", strlen(".csv"));
            err = gf_dump_raw_data(data_encoder,
                            filepath,
                            dump_data->data.image.raw_data,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_NAV_BASE
static gf_error_t dump_nav_base(dump_data_encoder_t* data_encoder,
                                gf_dump_data_t *dump_data,
                                gf_operation_type_t operation,
                                const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_NAV_BASE_DIR);
        // dump bigdata info
        if (gf_is_dump_bigdata_enabled() == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_info.json", (char*)dir,
                    (char*)cur_time_str);
            err = gf_bigdata_dump_nav_data(data_encoder, filepath, dump_data, operation);
            BREAK_IF_ERROR(err);  // break if error
        }

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_rawdata.csv",
                    (char*)dir, (char*)cur_time_str);
            err = gf_dump_raw_data(data_encoder,
                            filepath,
                            &dump_data->data.nav.raw_data[0],
                            gf_get_nav_width() * gf_get_nav_height(),
                            DATA_TYPE_NAV_RAW_DATA);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_NAV
static gf_error_t dump_nav(dump_data_encoder_t* data_encoder,
                           gf_dump_data_t *dump_data,
                           gf_operation_type_t operation,
                           gf_chip_type_t chip_type,
                           const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_NAV_DIR);
        // dump bigdata info
        if (gf_is_dump_bigdata_enabled() == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_info.json", (char*)dir, (char*)cur_time_str);
            err = gf_bigdata_dump_nav_data(data_encoder, filepath, dump_data, operation);
            BREAK_IF_ERROR(err);  // break if error
        }

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            for (i = 0; i < dump_data->data.nav.nav_frame_count; i++)
            {
                for (j = 0; j < dump_data->data.nav.frame_num[i]; j++)
                {
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                            "%s%snavigation_rawdata_%u_%02u.csv", (char*)dir, (char*)cur_time_str,
                            dump_data->data.nav.nav_times,
                            dump_data->data.nav.nav_frame_index);
                    err = gf_dump_raw_data(data_encoder,
                                    filepath,
                                    &dump_data->data.nav.raw_data[(i + j) * gf_get_nav_width() * gf_get_nav_height()],
                                    gf_get_nav_width() * gf_get_nav_height(),
                                    DATA_TYPE_NAV_RAW_DATA);
                    BREAK_IF_ERROR(err);  // break if error
                    dump_data->data.nav.nav_frame_index++;
                }
            }
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_BAD_POINT_RECODE_BASE, OPERATION_TEST_BAD_POINT
static gf_error_t dump_test_bad_point(dump_data_encoder_t* data_encoder,
                                      gf_dump_data_t *dump_data,
                                      gf_operation_type_t operation,
                                      const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_CONSISTENCY_TEST_DIR);

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            if (OPERATION_TEST_BAD_POINT_RECODE_BASE == operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT save bad point basedata",
                          __func__);
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_base_rawdata.csv", (char*)dir,
                         (char*)cur_time_str, dump_data->data.image.frame_num);
            }
            else
            {
                if (GF_MILAN_AN_SERIES == g_hal_config.chip_series)
                {
                    if (1 == dump_data->data.image.frame_num)
                    {
                        LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT save base", __func__);
                        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_base.csv", (char*)dir, (char*)cur_time_str);
                        err = gf_dump_raw_data(data_encoder,
                            filepath,
                            (uint16_t *)dump_data->data.image.b,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
                        break;
                    }
                    else
                    {
                        LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT save data", __func__);
                        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_rawdata.csv", (char*)dir, (char*)cur_time_str, dump_data->data.image.frame_num - 1);
                    }
                }
                else
                {
                    LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT save data", __func__);
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_rawdata.csv", (char*)dir,
                            (char*)cur_time_str, dump_data->data.image.frame_num);
                }
            }
            err = gf_dump_raw_data(data_encoder,
                            filepath,
                            dump_data->data.image.raw_data,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_CALIBRATION_PARA_RETEST_RECODE_BASE, OPERATION_TEST_CALIBRATION_PARA_RETEST
static gf_error_t dump_calibration_param_retest(dump_data_encoder_t* data_encoder,
                                      gf_dump_data_t *dump_data,
                                      gf_operation_type_t operation,
                                      const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_CALIBRATION_PARA_RETEST_DIR);

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            if (OPERATION_TEST_CALIBRATION_PARA_RETEST_RECODE_BASE == operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_CALIBRATION_PARA_RETEST_RECODE_BASE save basedata",
                          __func__);
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_base_rawdata.csv", (char*)dir,
                         (char*)cur_time_str, dump_data->data.image.frame_num);
            }
            else
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_CALIBRATION_PARA_RETEST save touchdata", __func__);
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_touch_rawdata.csv", (char*)dir,
                        (char*)cur_time_str, dump_data->data.image.frame_num);
            }
            err = gf_dump_raw_data(data_encoder,
                            filepath,
                            dump_data->data.image.raw_data,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_DATA_NOISE, OPERATION_TEST_DATA_NOISE_BASE
static gf_error_t dump_test_data_noise(dump_data_encoder_t* data_encoder,
                                      gf_dump_data_t *dump_data,
                                      gf_operation_type_t operation,
                                      const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == cur_time_str)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_SNR_TEST_PATH);

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_RAW_DATA) == 1)
        {
            if (OPERATION_TEST_DATA_NOISE_BASE == operation)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_DATA_NOISE_BASE save snr test basedata",
                          __func__);
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_base_rawdata.csv", (char*)dir,
                         (char*)cur_time_str, dump_data->data.image.frame_num);
            }
            else
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_DATA_NOISE save snr test touchdata", __func__);
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_%d_rawdata.csv", (char*)dir,
                        (char*)cur_time_str, dump_data->data.image.frame_num);
            }
            err = gf_dump_raw_data(data_encoder,
                            filepath,
                            dump_data->data.image.raw_data,
                            gf_get_sensor_width() * gf_get_sensor_height(),
                            DATA_TYPE_RAW_DATA);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

// dump OPERATION_TEST_BOOT_CALIBRATION
static gf_error_t hal_dump_boot_cali(dump_data_encoder_t* data_encoder,
                                     gf_dump_data_t *dump_data,
                                     gf_operation_type_t operation,
                                     const uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t boot_str[GF_DUMP_FILE_PATH_MAX_LEN]  = "boot";

    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == dump_data
            || NULL == cur_time_str)
        {
            err = GF_ERROR_CANCELED;
            break;
        }

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", GF_DUMP_BOOT_CALI_BASE_DIR);

        // dump calibration params
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_CALIBRATION_PARAMS) == 1)
        {
            err = dump_calibration_params(data_encoder, dump_data, err, dir, boot_str,
                                          cur_time_str, gf_get_sensor_width(), gf_get_sensor_height());
            BREAK_IF_ERROR(err);  // break if error
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_dump_data_by_operation(dump_data_encoder_t* data_encoder,
                                     gf_dump_data_t *dump_data,
                                     gf_operation_type_t operation,
                                     struct timeval* tv,
                                     gf_error_t error_code,
                                     gf_chip_type_t chip_type)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t cur_time_str[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t i = 0;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] operation=%d, error_code=%d", __func__, operation,
          error_code);

    do
    {
        if (NULL == dump_data || NULL == data_encoder || NULL == tv)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }
        for (i = 0; i < (sizeof(dump_data->data.image.b) / sizeof(uint16_t)); i++)
        {
            if (dump_data->data.image.b[i] >= CHICAGO_B_OFFSET)
            {
                dump_data->data.image.b[i] -= CHICAGO_B_OFFSET;
            }
        }

        gf_get_time_string(tv, cur_time_str, 1);

        switch (operation)
        {
            case OPERATION_ENROLL:
            case OPERATION_TEST_UNTRUSTED_ENROLL:
            {
                err = dump_enroll(data_encoder, dump_data,
                                  operation, error_code, cur_time_str);
                break;
            }

            case OPERATION_AUTHENTICATE_IMAGE:
            case OPERATION_AUTHENTICATE_SLEEP:
            case OPERATION_AUTHENTICATE_FF:
            case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
            {
                if (dump_data->data.image.authenticate_retry_count > 0) {
                    memcpy(cur_time_str, g_last_cur_time_str, GF_DUMP_FILE_PATH_MAX_LEN);
                } else {
                    memcpy(g_last_cur_time_str, cur_time_str, GF_DUMP_FILE_PATH_MAX_LEN);
                }
                err = dump_authenticate(data_encoder, dump_data, operation, error_code, cur_time_str);
                break;
            }

            case OPERATION_TEST_PERFORMANCE:
            {
                err = dump_test_performance(data_encoder, dump_data, operation, error_code, cur_time_str);
                break;
            }

            case OPERATION_FINGER_BASE:
            {
                err = dump_finger_base(data_encoder, dump_data, operation, error_code, cur_time_str);
                break;
            }

            case OPERATION_TEST_PIXEL_OPEN_STEP1:
            case OPERATION_TEST_PIXEL_OPEN_STEP2:
            {
                err = dump_test_pixel_open_step(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            case OPERATION_TEST_PIXEL_SHORT_STREAK:
            {
                err = dump_test_pixel_short_streak(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            case OPERATION_TEST_SENSOR_FINE_STEP1:
            case OPERATION_TEST_SENSOR_FINE_STEP2:
            {
                err = dump_test_sensor_fine_step(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            case OPERATION_NAV_BASE:
            {
                err = dump_nav_base(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            case OPERATION_NAV:
            {
                err = dump_nav(data_encoder, dump_data, operation, chip_type, cur_time_str);
                break;
            }

            case OPERATION_TEST_BAD_POINT_RECODE_BASE:
            case OPERATION_TEST_BAD_POINT:
            {
                err = dump_test_bad_point(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            case OPERATION_TEST_CALIBRATION_PARA_RETEST_RECODE_BASE:
            case OPERATION_TEST_CALIBRATION_PARA_RETEST:
            {
                err = dump_calibration_param_retest(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            case OPERATION_TEST_DATA_NOISE_BASE:
            case OPERATION_TEST_DATA_NOISE:
            {
                err = dump_test_data_noise(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            case OPERATION_TEST_BOOT_CALIBRATION:
            {
                err = hal_dump_boot_cali(data_encoder, dump_data, operation, cur_time_str);
                break;
            }

            default:
            {
                break;
            }
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}
gf_error_t gf_dump_data_by_operation_reserve(dump_data_encoder_t* data_encoder,
                                  gf_dump_reserve_t *dump_data,
                                  gf_operation_type_t operation,
                                  struct timeval* tv,
                                  gf_error_t error_code,
                                  gf_chip_type_t chip_type)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] operation=%d, error_code=%d", __func__, operation,
       error_code);

    do
    {
     if (NULL == dump_data || NULL == data_encoder || NULL == tv)
     {
         err = GF_ERROR_BAD_PARAMS;
         LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
         break;
     }


     switch (operation)
     {
         case OPERATION_ENROLL:
         case OPERATION_TEST_UNTRUSTED_ENROLL:
         {
             LOG_D(LOG_TAG, "[%s] enroll don't dump reserve", __func__);
             break;
         }

         case OPERATION_AUTHENTICATE_IMAGE:
         case OPERATION_AUTHENTICATE_SLEEP:
         case OPERATION_AUTHENTICATE_FF:
         case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
         {
             err = dump_asp_data_info(dump_data, operation, error_code, g_last_cur_time_str);
             break;
         }

            default:
            {
                break;
            }
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_dump_template(dump_data_encoder_t* data_encoder,
                    gf_dump_template_t *template,
                    struct timeval* tv)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    uint8_t file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t cur_time[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };

    do
    {
        if (NULL == data_encoder || NULL == template || NULL == tv)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        gf_get_time_string(tv, cur_time, 1);
        snprintf((char*)file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_template_%u.dat",
            GF_DUMP_TEMPLATE_DIR, (char*)cur_time, template->finger_id);
        err = gf_dump_encoder_fopen(data_encoder, file_path, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        err = gf_dump_encoder_fwrite(template->template_data, template->template_len, data_encoder);
        BREAK_IF_ERROR(err);  // break if error
        gf_dump_encoder_fclose(data_encoder);

        // dump bigdata info
        if (gf_is_dump_bigdata_enabled() == 1)
        {
            snprintf((char*)file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_info.json",
                GF_DUMP_TEMPLATE_DIR, (char*)cur_time);
            err = gf_bigdata_dump_template_info(data_encoder, file_path);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

void gf_dump_init(uint32_t row, uint32_t col, uint32_t nav_row,
                  uint32_t nav_col,
                  gf_chip_type_t chip_type, gf_chip_series_t chip_series)
{
    uint32_t sensor_width = col;
    uint32_t sensor_height = row;
    uint32_t nav_width = 0;
    uint32_t nav_height = 0;

    if (GF_MILAN_F_SERIES == chip_series || GF_DUBAI_A_SERIES == chip_series)
    {
        if (GF_CHIP_3208 == chip_type)
        {
            nav_width = nav_col;
            nav_height = nav_row;
        }
        else
        {
            nav_width = nav_row;
            nav_height = nav_col;
        }
    }
    else if (GF_MILAN_HV == chip_series)
    {
        nav_width = nav_row;
        nav_height = nav_col;
    }
    else
    {
        nav_width = nav_col;
        nav_height = nav_row;
    }

    gf_init_dump_config(sensor_width, sensor_height, nav_width, nav_height,
        chip_type, chip_series);
}

void gf_dump_prepare_for_test_cmd(uint32_t cmd_id)
{
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    VOID_FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s]cmd_id=%u", __func__, cmd_id);

    do
    {
        if (gf_is_dump_enabled())
        {
            break;
        }

        switch (cmd_id)
        {
            case CMD_TEST_BAD_POINT:
            {
                snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s", (char*)gf_get_dump_root_dir(),
                         GF_DUMP_CONSISTENCY_TEST_DIR);
                break;
            }

            case CMD_TEST_PERFORMANCE:
            {
                snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s", (char*)gf_get_dump_root_dir(),
                         GF_DUMP_TEST_PERFORMANCE_DIR);
                break;
            }

            case CMD_TEST_PIXEL_OPEN:
            {
                snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s", (char*)gf_get_dump_root_dir(),
                         GF_DUMP_TEST_PIXEL_OPEN_DIR);
                break;
            }

            default:
            {
                break;
            }
        }

        if ('/' == dir[0])
        {
            dump_empty_dir(dir);
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

gf_error_t gf_dump_memmgr_pool(uint8_t *data, uint32_t len, void *time_str)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_encoder_t* data_encoder = NULL;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    struct timeval tv = { 0 };
    int64_t timestamp = 0;

    FUNC_ENTER();

    do
    {
        if (gf_is_dump_enabled() == 0)
        {
            LOG_D(LOG_TAG, "[%s] dump flag is disable", __func__);
            break;
        }

        if (NULL == data || NULL == time_str)
        {
            LOG_E(LOG_TAG, "[%s] invalid params: data or time_str is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (len == 0)
        {
            LOG_E(LOG_TAG, "[%s] invalid params: len=%d", __func__, len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gettimeofday(&tv, NULL);
        timestamp = gf_get_time_stamp(&tv);

        err = gf_dump_encoder_create(&data_encoder, timestamp);
        if (err != GF_SUCCESS)
        {
            break;
        }
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_mem_pool.csv", GF_DUMP_MEM_MANAGER_POOL_DIR,
                 (char *) time_str);
        err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)filepath, DATA_TYPE_NORMAL_FILE_DATA);
        if (err != GF_SUCCESS)
        {
            break;
        }

        err = gf_dump_encoder_fwrite(data, len, data_encoder);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fclose(data_encoder);
        err = gf_handle_dump_buf(data_encoder);
    }
    while (0);

    gf_dump_encoder_destroy(data_encoder);

    FUNC_EXIT(err);
    return err;
}

void gf_dump_save_cur_auth_time(uint8_t index)
{
    struct timeval tv = { 0 };
    gettimeofday(&tv, NULL);
    if (index <= 4)
    {
        gf_get_time_string(&tv, g_cur_authtime_str[index], 1);
    }
}

