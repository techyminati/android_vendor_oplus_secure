/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: milan an chip particular dump
 * History:
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gf_milan_an_series_dump.h"
#include "gf_hal_log.h"
#include "gf_dump_data.h"
#include "gf_dump_data_utils.h"
#include "gf_dump_bigdata.h"

#define LOG_TAG "[GF_HAL][gf_milan_an_dump]"

/**
 *Function: encode_nav_enhance_data
 *Description: encode nav enhance data
 *Input: data_encoder, encoder buffer; dump_data, dump data buf
 *Output: encoded nav enhance data to data_encoder
 *Return: gf_error_t
 */
static gf_error_t encode_nav_enhance_data(dump_data_encoder_t* data_encoder,
                                            gf_dump_data_t *dump_data)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t nav_dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t i = 0;
    uint32_t j = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == data_encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        snprintf((char*)nav_dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%d/%s", GF_DUMP_NAV_DIR,
                dump_data->data.nav_enhance.nav_times,
                GF_DUMP_NAV_ENHANCE);

        for (i = 0; i < dump_data->data.nav_enhance.nav_frame_count; i++)
        {
            for (j = 0; j < dump_data->data.nav_enhance.frame_num[i]; j++)
            {
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%snavigation_rawdata_%u_%02u.csv", (char*)nav_dir,
                        dump_data->data.nav_enhance.nav_times,
                        dump_data->data.nav_enhance.nav_frame_index);
                err = gf_dump_raw_data(data_encoder,
                            (uint8_t*)filepath,
                            &dump_data->data.nav_enhance.nav_raw_data_enhance
                            [(i + j) * gf_get_nav_width() * gf_get_nav_height()],
                            gf_get_nav_width() * gf_get_nav_height(),
                            DATA_TYPE_NAV_RAW_DATA);
                BREAK_IF_ERROR(err);  // break if error
                dump_data->data.nav_enhance.nav_frame_index++;
            }
        }
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: dump_nav_enhance_data
 *Description: dump nav enhance data
 *Input: data_encoder, encoder buffer
 *Output: dump files
 *Return: gf_error_t
 */
static gf_error_t dump_nav_enhance_data(dump_data_encoder_t* data_encoder)
{
    gf_error_t err = GF_SUCCESS;
    gf_dump_data_t *cmd = NULL;
    uint32_t size = sizeof(gf_dump_data_t);
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
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
        err = gf_hal_invoke_command(GF_CMD_DUMP_NAV_ENHANCE_DATA, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] GF_CMD_DUMP_NAV_ENHANCE_DATA error:%d", __func__, err);
            break;
        }

        cmd->data.nav_enhance.nav_times = g_nav_times;

        err = encode_nav_enhance_data(data_encoder, cmd);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] dump nav enhanced data error:%d", __func__, err);
            break;
        }

        err = gf_handle_dump_buf(data_encoder);
    }  // do...
    while (0);

    if (cmd != NULL)
    {
        free(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: hal_milan_an_dump_nav
 *Description: milan_an nav dump
 *Input: dump_data, data to be dump;
 *       operation, current operation;
 *       tv, current system time
 *Output: dump files
 *Return: gf_error_t
 */
static gf_error_t hal_milan_an_dump_nav(gf_dump_data_t *dump_data,
                                       gf_operation_type_t operation,
                                       struct timeval* tv)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t cur_time_str[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    dump_data_encoder_t* data_encoder = NULL;
    int64_t time_stamp = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == tv)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_get_time_string(tv, cur_time_str, 1);
        time_stamp = gf_get_time_stamp(tv);

        err = gf_dump_encoder_create(&data_encoder, time_stamp);
        if (NULL == data_encoder)
        {
            break;
        }

        dump_data->data.nav.nav_frame_index = 0;
        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%d/%s", GF_DUMP_NAV_DIR,
                    dump_data->data.nav.nav_times, GF_DUMP_NAV_ROWDATA);  // bigdata modify

        /*bigdata begin*/
        if (gf_is_dump_bigdata_enabled() == 1)
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%sinfo.json", (char*)dir);
            err = gf_bigdata_dump_nav_data(data_encoder, filepath, dump_data, operation);
            BREAK_IF_ERROR(err);  // break if error
        }
        /*bigdata end*/

        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_NAV_ENHANCE) == 1)
        {
            for (i = 0; i < dump_data->data.nav.nav_frame_count; i++)
            {
                for (j = 0; j < dump_data->data.nav.frame_num[i]; j++)
                {
                    snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%snavigation_rawdata_%u_%02u.csv", (char*)dir,
                             dump_data->data.nav.nav_times, dump_data->data.nav.nav_frame_index);
                    err = gf_dump_raw_data(data_encoder,
                                    (uint8_t*)filepath,
                                    &dump_data->data.nav.raw_data[(i + j) * gf_get_nav_width() * gf_get_nav_height()],
                                    gf_get_nav_width() * gf_get_nav_height(),
                                    DATA_TYPE_NAV_RAW_DATA);
                    BREAK_IF_ERROR(err);  // break if error
                    dump_data->data.nav.nav_frame_index++;
                }
            }
        }
        err = gf_handle_dump_buf(data_encoder);
        BREAK_IF_ERROR(err);  // break if error

        // dump nav data finished, reuse dump buffer
        gf_dump_encoder_reset(data_encoder);
        // dump nav enhance data
        if (gf_is_operation_data_dump_allowed(operation, OP_DATA_NAV_ENHANCE) == 1)
        {
            err = dump_nav_enhance_data(data_encoder);
        }
    }  // do...
    while (0);

    gf_dump_encoder_destroy(data_encoder);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: dump_calibration_params_to_csv_file
 *Description: dump calibrations params to csv file
 *Input: data_encoder, encoder buffer;
 *       file_path, file name for csv file;
 *       data, data to be dump
 *Output: encoded data to data_encoder
 *Return: gf_error_t
 */
static gf_error_t dump_calibration_params_to_csv_file(dump_data_encoder_t* data_encoder,
                                                        const uint8_t* file_path, uint8_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;
    char line[1024] = { 0 };
    char *ptr = line;
    FUNC_ENTER();

    do
    {
        if (NULL == data_encoder || NULL == file_path || NULL == data)
        {
            LOG_E(LOG_TAG, "[%s] invalid param file_path=%p, data=%p", __func__, file_path, data);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_dump_encoder_fopen(data_encoder, file_path, DATA_TYPE_NORMAL_FILE_DATA);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] open file (%s) fail", __func__, file_path);
            err = GF_ERROR_FILE_OPEN_FAILED;
            break;
        }

        snprintf(ptr, sizeof(line), "Tcode, TempCode, KeyStatus, DacCalStep, CalFrmNum, FrmDataDiff,"
            "FrmDataMax, FrmDataMin, FrmDataAve, OsFailPixelNum, BadPixelNum, RefInfo\n");
        ptr = line + strlen(line);
        for (i = 0; i < 24; i = i + 2)
        {
            snprintf(ptr, sizeof(line), "%u,", *((uint16_t *) (data + i)));
            ptr = line + strlen(line);
        }
        data += 24;
        // Vcm R value
        snprintf(ptr, sizeof(line), "\nVcmRVal =, ");
        ptr = line + strlen(line);
        for (i = 0; i < 32; i++)
        {
            snprintf(ptr, sizeof(line), "%u,", *(data + i));
            ptr = line + strlen(line);
        }
        data += 32;
        // VcmREn
        data++;
        // Normal Dac value L
        snprintf(ptr, sizeof(line), "\nNorDacVal_L =, ");
        ptr = line + strlen(line);
        for (i = 0; i < 60; i++)
        {
            snprintf(ptr, sizeof(line), "%u,", *(data + i));
            ptr = line + strlen(line);
        }
        data += 60;

        // Dac value H
        snprintf(ptr, sizeof(line), "\nDacVal_H =, ");
        ptr = line + strlen(line);
        for (i = 0; i < 60; i++)
        {
            snprintf(ptr, sizeof(line), "%u,", *(data + i));
            ptr = line + strlen(line);
        }
        data += 60;

        // dummy dac value L
        snprintf(ptr, sizeof(line), "\nDmyDacVal_L =, ");
        ptr = line + strlen(line);
        for (i = 0; i < 60; i++)
        {
            snprintf(ptr, sizeof(line), "%u,", *(data + i));
            ptr = line + strlen(line);
        }
        err = gf_dump_encoder_fwrite(line, strlen(line), data_encoder);
        gf_dump_encoder_fclose(data_encoder);
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: dump_calibration_data_frames
 *Description: dump calibrations data frames
 *Input: dump_data, data to be dump;
 *       tv, current system time
 *Output: dump files
 *Return: gf_error_t
 */
static gf_error_t dump_calibration_data_frames(gf_dump_data_t *dump_data,
                                                struct timeval* tv)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t filepath[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t tmp_line[1024];
    dump_data_encoder_t* data_encoder = NULL;
    int64_t time_stamp = 0;
    uint32_t index = 0;
    uint32_t frame_num = 0;
    uint32_t i = 0;

    gf_calibration_data_frames_t* calibration_data_frames =
            &(dump_data->data.calibration_data_frames);
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == tv)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        time_stamp = gf_get_time_stamp(tv);
        err = gf_dump_encoder_create(&data_encoder, time_stamp);
        if (NULL == data_encoder)
        {
            break;
        }

        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s-calib_index_%u/", GF_DUMP_CALIBRATION_DATA_DIR,
                (char*)gf_get_boot_time_str(), calibration_data_frames->calibration_index);

        // dump base valid info
        snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%sbase_valid_info.csv", (char*)dir);
        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_NORMAL_FILE_DATA);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] open file (%s) fail", __func__, filepath);
            err = GF_ERROR_FILE_OPEN_FAILED;
            break;
        }

        gf_dump_encoder_fwrite("base_valid, base_chosen, key_status\n",
                strlen("base_valid, base_chosen, key_status "), data_encoder);
        index = calibration_data_frames->calibration_index < CALIBRATION_MAX_INDEX ?
                calibration_data_frames->calibration_index : CALIBRATION_MAX_INDEX;
        for (i = 0; i < index; i++)
        {
            snprintf((char*)tmp_line, sizeof(tmp_line), "%u, %u, %u\n", calibration_data_frames->is_base_valid[i],
                    calibration_data_frames->base_chosen[i],
                    calibration_data_frames->key_status[i]);
            err = gf_dump_encoder_fwrite(tmp_line, strlen((char*)tmp_line), data_encoder);
            BREAK_IF_ERROR(err);  // break if error
        }
        gf_dump_encoder_fclose(data_encoder);

        // dump base calibration params
        if (0 == calibration_data_frames->base_chosen[index - 1])
        {
            for (i = 0; i < (calibration_data_frames->calibration_frame_num / 3); i++)
            {
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%scal_params_%u.csv", (char*)dir, i);
                err = dump_calibration_params_to_csv_file(data_encoder, filepath,
                        calibration_data_frames->calibration_params[i]);
            }
            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] dump caliration param fail, error=%d", __func__, err);
                break;
            }
        }
        else
        {
            snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%scal_params.csv", (char*)dir);
            err = dump_calibration_params_to_csv_file(data_encoder, filepath,
                    calibration_data_frames->calibration_params[0]);
            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] dump caliration param fail, error=%d", __func__, err);
                break;
            }
        }

        frame_num =
                calibration_data_frames->calibration_frame_num < CALIBRATION_MAX_FRAMES ?
                        calibration_data_frames->calibration_frame_num : CALIBRATION_MAX_FRAMES;
        for (i = 0; i < frame_num; i++)
        {
            if (calibration_data_frames->key_status[index - 1] > 0 && i == frame_num - 1
                    && calibration_data_frames->base_chosen[index - 1] != 0)
            {
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%sfinger_base_%u_t.csv", (char*)dir, i);
            }
            else
            {
                snprintf((char*)filepath, GF_DUMP_FILE_PATH_MAX_LEN, "%sfinger_base_%u.csv", (char*)dir, i);
            }

            err = gf_dump_raw_data(data_encoder, filepath, calibration_data_frames->raw_data[i],
                                    gf_get_sensor_width()*gf_get_sensor_height(), DATA_TYPE_RAW_DATA);
            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] dump first finger base raw data fail, error=%d", __func__, err);
                break;
            }
        }
        err = gf_handle_dump_buf(data_encoder);
    }  // do...
    while (0);

    gf_dump_encoder_destroy(data_encoder);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: dump_calibration_data_frames
 *Description: interface for dump calibrations data frames
 *Input: none
 *       tv, current system time
 *Output: encoded data to encoder
 *Return: gf_error_t
 */
static gf_error_t hal_dump_calibration_data_frames(void)
{
    gf_error_t err = GF_SUCCESS;
    gf_dump_data_t *cmd = NULL;
    uint32_t size = sizeof(gf_dump_data_t);
    struct timeval tv = { 0 };

    FUNC_ENTER();

    do
    {
        cmd = (gf_dump_data_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd=%p", __func__, cmd);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_DUMP_CALIBRATION_DATA_FRAMES, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] GF_CMD_DUMP_CALIBRATION_DATA_FRAMES error:%d", __func__, err);
            break;
        }
        gettimeofday(&tv, NULL);

        err = dump_calibration_data_frames(cmd, &tv);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] dump calibration data frames error:%d", __func__, err);
            break;
        }
    }  // do...
    while (0);

    if (cmd != NULL)
    {
        free(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}


/**
 *Function: hal_milan_an_dump_chip_operation_data
 *Description: dump milan an chip particular operation data
 *Input: dump_data, data to dump
 *       operation, operation to be dump
 *       tv, current system time
 *       error_code, error code
 *       chip_type, chip type
 *Output: dump files
 *Return: gf_error_t
 */
bool hal_milan_an_dump_chip_operation_data(gf_dump_data_t *dump_data,
                                       gf_operation_type_t operation,
                                       struct timeval* tv,
                                       gf_error_t error_code,
                                       gf_chip_type_t chip_type)
{
    bool ret = false;
    gf_error_t err = GF_SUCCESS;
    UNUSED_VAR(error_code);
    UNUSED_VAR(chip_type);
    FUNC_ENTER();

    do
    {
        if (NULL == dump_data || NULL == tv)
        {
            LOG_E(LOG_TAG, "[%s] dump_data or tv is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        switch (operation)
        {
            case OPERATION_NAV:
            {
                err = hal_milan_an_dump_nav(dump_data, operation, tv);
                ret = true;  // nav dump finished, return true, no need common dump for nav
                break;
            }

            case OPERATION_FINGER_BASE:
            {
                LOG_D(LOG_TAG, "[%s] dump calibration data frames", __func__);
                err = hal_dump_calibration_data_frames();
                ret = false;  // return false, continue common dump for finger base
                break;
            }

            default:
            {
                break;
            }
        }
    } while (0);  // do...

    FUNC_EXIT(err);
    return ret;
}


