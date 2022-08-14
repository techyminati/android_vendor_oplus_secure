/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: bigdata related dump
 * History:
 */

#include <stdlib.h>
#include <stdio.h>
#include <cutils/properties.h>
#include <time.h>
#include <cutils/fs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "gf_error.h"
#include "gf_common.h"
#include "gf_hal_log.h"
#include "gf_dump_bigdata.h"
#include "gf_dump_data_utils.h"
#include "gf_hal_mem.h"

#define LOG_TAG "[GF_HAL][gf_dump_bigdata]"

/**
 *Function: upper
 *Description: to upper string
 *Input: string, original string
 *Output: upper string
 *Return: none
 */
static void upper(char *string)
{
    if (NULL == string)
    {
        return;
    }

    while (*string)
    {
        if (*string >= 'a' && *string <= 'z')
        {
            *string &= 0xDF;
        }

        ++string;
    }

    return;
}

/**
 *Function: gf_bigdata_dump_template_info
 *Description: bigdata dump template info
 *Input: data_encoder, encoder buffer
 *       filepath, dump file name
 *Output: encoded template info to encoder buffer
 *Return: gf_error_t
 */
gf_error_t gf_bigdata_dump_template_info(dump_data_encoder_t* data_encoder, uint8_t* filepath)
{
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == filepath || NULL == data_encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fprintf(data_encoder, "{\n");
        gf_dump_encoder_fprintf(data_encoder, "    \"version\":    \"%s\",\n",
                gf_get_dump_bigdata_version());
        gf_dump_encoder_fprintf(data_encoder, "    \"type\":    %d,\n", TEMPLATE);
        gf_dump_encoder_fprintf(data_encoder, "}\n");
        gf_dump_encoder_fclose(data_encoder);
    }
    while (0);

    return err;
}

/**
 *Function: gf_bigdata_dump_device_info
 *Description: bigdata dump device info
 *Input: data_encoder, encoder buffer
 *       dir, dir which file will be dump at
 *       dev_info, device info to be dump
 *       cur_time_str, current time string
 *Output: encoded device info to encoder buffer
 *Return: gf_error_t
 */
gf_error_t gf_bigdata_dump_device_info(dump_data_encoder_t* data_encoder,
                                       uint8_t* dir,
                                       gf_dev_info_t *dev_info,
                                       uint8_t* cur_time_str)
{
    gf_error_t err = GF_SUCCESS;
    char file_name[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    gf_bigdata_device_info_t *device_info = NULL;
    int32_t i = 0;
    uint32_t len = 0;

    do
    {
        if (NULL == dir || NULL == dev_info || NULL == data_encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        device_info = (gf_bigdata_device_info_t *)GF_MEM_MALLOC(sizeof(gf_bigdata_device_info_t));

        if (NULL == device_info)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(device_info, 0, sizeof(gf_bigdata_device_info_t));
        device_info->scene_info.type = DEVICE;
        memcpy(device_info->android_version, dev_info->android_version,
               sizeof(device_info->android_version));
        memcpy(device_info->service_version, dev_info->service_version,
               sizeof(device_info->service_version));
        memcpy(device_info->platform, dev_info->platform,
               sizeof(device_info->platform));
        upper((char *)device_info->platform);
        memcpy(device_info->algo_version, dev_info->algo_version,
               sizeof(device_info->algo_version));
        memcpy(device_info->preprocess_version, dev_info->preprocess_version,
               sizeof(device_info->preprocess_version));
        snprintf(device_info->fw_version, GF_BIGDATA_FW_VERSION_LEN, "NULL");
        memcpy(device_info->tee_version, dev_info->tee_version,
               sizeof(device_info->tee_version));
        memcpy(device_info->ta_version, dev_info->ta_version,
               sizeof(device_info->ta_version));
        snprintf((char *)device_info->chip_id, GF_BIGDATA_CHIP_ID_LEN,
                 "0x%02X%02X%02X%02X",
                 dev_info->chip_id[3], dev_info->chip_id[2],
                 dev_info->chip_id[1], dev_info->chip_id[0]);
        snprintf((char *)device_info->vendor_id, GF_BIGDATA_VENDOR_ID_LEN, "0x%02X%02X",
                 dev_info->vendor_id[0], dev_info->vendor_id[1]);
        snprintf((char *)device_info->sensor_id, GF_BIGDATA_SENSOR_ID_LEN * 2,
                 "%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X%X", dev_info->sensor_id[0]
                 , dev_info->sensor_id[1], dev_info->sensor_id[2], dev_info->sensor_id[3]
                 , dev_info->sensor_id[4], dev_info->sensor_id[5], dev_info->sensor_id[6]
                 , dev_info->sensor_id[7], dev_info->sensor_id[8], dev_info->sensor_id[9]
                 , dev_info->sensor_id[10], dev_info->sensor_id[11], dev_info->sensor_id[12]
                 , dev_info->sensor_id[13], dev_info->sensor_id[14], dev_info->sensor_id[15]);
        snprintf((char *)device_info->otp_chipid, GF_BIGDATA_SENSOR_OTP_CHIPID_LEN,
                 "NULL");
        snprintf((char *)device_info->otp_version, GF_BIGDATA_SENSOR_OTP_VERSION_LEN,
                 "NULL");
        memcpy(device_info->otp_info, dev_info->otp_info,
               sizeof(device_info->otp_info));
        memcpy(device_info->production_date, dev_info->production_date,
               sizeof(device_info->production_date));
        memcpy(device_info->module_version, dev_info->module_version,
               sizeof(device_info->module_version));
        device_info->row = dev_info->row;
        device_info->col = dev_info->col;
        device_info->nav_row = dev_info->nav_row;
        device_info->nav_col = dev_info->nav_col;
        device_info->orientation = dev_info->orientation;
        device_info->facing = dev_info->facing;
        snprintf(file_name, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s_info.json", (char *)dir,
                 (char*)cur_time_str);
        err = gf_dump_encoder_fopen(data_encoder, (uint8_t*)file_name, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        len = gf_get_err_table_len();
        gf_dump_encoder_fprintf(data_encoder, "{\n");
        gf_dump_encoder_fprintf(data_encoder, "    \"version\":    \"%s\",\n",
                gf_get_dump_bigdata_version());
        gf_dump_encoder_fprintf(data_encoder, "    \"type\":    %d,\n", device_info->scene_info.type);
        gf_dump_encoder_fprintf(data_encoder, "    \"scene\":    %d,\n", device_info->scene_info.scene);
        gf_dump_encoder_fprintf(data_encoder, "    \"charge\":    %d,\n", device_info->scene_info.charge);
        gf_dump_encoder_fprintf(data_encoder, "    \"screen_status\":    %d,\n",
                device_info->scene_info.screen_status);
        gf_dump_encoder_fprintf(data_encoder, "    \"lock_status\":    %d,\n",
                device_info->scene_info.lock_status);
        gf_dump_encoder_fprintf(data_encoder, "    \"android_version\":    \"%s\",\n",
                device_info->android_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"service_version\":    \"%s\",\n",
                device_info->service_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"platform\":    \"%s\",\n", device_info->platform);
        gf_dump_encoder_fprintf(data_encoder, "    \"algo_version\":    \"%s\",\n", device_info->algo_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"preprocess_version\":    \"%s\",\n",
                device_info->preprocess_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"fw_version\":    \"%s\",\n", device_info->fw_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"ta_version\":    \"%s\",\n", device_info->ta_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"chip_id\":    \"%s\",\n", device_info->chip_id);
        gf_dump_encoder_fprintf(data_encoder, "    \"vendor_id\":    \"%s\",\n", device_info->vendor_id);
        gf_dump_encoder_fprintf(data_encoder, "    \"sensor_id\":    \"%s\",\n", device_info->sensor_id);
        gf_dump_encoder_fprintf(data_encoder, "    \"otp_version\":    \"%s\",\n", device_info->otp_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"otp_chipid\":    \"%s\",\n", device_info->otp_chipid);
        gf_dump_encoder_fprintf(data_encoder, "    \"production_date\":    \"%s\",\n",
                device_info->production_date);
        gf_dump_encoder_fprintf(data_encoder, "    \"module_version\":    \"%s\",\n",
                device_info->module_version);
        gf_dump_encoder_fprintf(data_encoder, "    \"row\":    %d,\n", device_info->row);
        gf_dump_encoder_fprintf(data_encoder, "    \"col\":    %d,\n", device_info->col);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_row\":    %d,\n", device_info->nav_row);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_col\":    %d,\n", device_info->nav_col);
        gf_dump_encoder_fprintf(data_encoder, "    \"orientation\":    %d,\n", device_info->orientation);
        gf_dump_encoder_fprintf(data_encoder, "    \"facing\":    %d,\n", device_info->facing);
        gf_dump_encoder_fprintf(data_encoder, "    \"error_string\":    \"");

        for (i = 0; i < len - 1; i++)
        {
            gf_dump_encoder_fprintf(data_encoder, " %s,", err_table[i].strerror);
        }

        gf_dump_encoder_fprintf(data_encoder, " %s\"\n", err_table[i].strerror);
        gf_dump_encoder_fprintf(data_encoder, "}\n");
        gf_dump_encoder_fclose(data_encoder);
    }  // do...
    while (0);

    if (NULL != device_info)
    {
        GF_MEM_FREE(device_info);
        device_info = NULL;
    }

    return err;
}

/**
 *Function: gf_bigdata_dump_image_data
 *Description: bigdata dump image data
 *Input: data_encoder, encoder buffer
 *       filepath, dump file name
 *       dump_data, data to be dump
 *       operation, operation
 *       error_code, error code
 *Output: encoded image data to encoder buffer
 *Return: gf_error_t
 */
gf_error_t gf_bigdata_dump_image_data(dump_data_encoder_t* data_encoder, const uint8_t* filepath,
                                      gf_dump_data_t* dump_data,
                                      gf_operation_type_t operation, gf_error_t error_code)
{
    gf_error_t err = GF_SUCCESS;
    gf_bigdata_image_t *image_info = NULL;

    do
    {
        if (NULL == filepath || NULL == dump_data || NULL == data_encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (operation != OPERATION_ENROLL
            && operation != OPERATION_TEST_UNTRUSTED_ENROLL
            && operation != OPERATION_TEST_UNTRUSTED_AUTHENTICATE
            && operation != OPERATION_AUTHENTICATE_FF
            && operation != OPERATION_AUTHENTICATE_IMAGE
            && operation != OPERATION_AUTHENTICATE_FIDO
            && operation != OPERATION_FINGER_BASE)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (error_code < GF_SUCCESS || error_code > GF_ERROR_MAX)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        image_info = (gf_bigdata_image_t *)GF_MEM_MALLOC(sizeof(gf_bigdata_image_t));

        if (NULL == image_info)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(image_info, 0, sizeof(gf_bigdata_image_t));

        if ((operation == OPERATION_ENROLL)
            || (operation == OPERATION_TEST_UNTRUSTED_ENROLL))
        {
            image_info->scene_info.type = ENROLL;
        }
        else if (operation == OPERATION_FINGER_BASE)
        {
            image_info->scene_info.type = FINGER_BASE;
        }
        else
        {
            image_info->scene_info.type = AUTHENTICATE;
        }

        image_info->scene_info.index = dump_data->down_irq_time;
        image_info->scene_info.screen_status = dump_data->screen_flag;
        image_info->scene_info.scene = operation;
        image_info->timestamp_keydown = dump_data->down_irq_time / 1000;
        image_info->get_gsc_data_time =
            dump_data->data.image.dump_performance.get_gsc_data_time / 1000;
        image_info->get_raw_data_time =
            dump_data->data.image.dump_performance.get_raw_data_time / 1000;
        image_info->broken_check_time =
            dump_data->data.image.dump_performance.broken_check_time / 1000;
        image_info->preprocess_time =
            dump_data->data.image.dump_performance.preprocess_time / 1000;
        image_info->get_feature_time =
            dump_data->data.image.dump_performance.get_feature_time / 1000;
        image_info->enroll_time =
            dump_data->data.image.dump_performance.enroll_time / 1000;
        image_info->authenticate_time =
            dump_data->data.image.dump_performance.authenticate_time / 1000;
        image_info->bio_assay_time =
            dump_data->data.image.dump_performance.bio_assay_time / 1000;

        if (error_code >= GF_ERROR_BASE)
        {
            image_info->error_code = error_code - GF_ERROR_BASE + 1;
        }
        else
        {
            image_info->error_code = error_code;
        }

        image_info->image_quality = dump_data->data.image.image_quality;
        image_info->valid_area = dump_data->data.image.valid_area;
        image_info->template_count =
            dump_data->data.image.dump_performance.template_count;
        image_info->key_point_num =
            dump_data->data.image.dump_performance.key_point_num;
        image_info->enrolling_finger_id = dump_data->data.image.enrolling_finger_id;
        image_info->overlay = dump_data->data.image.dump_performance.overlay;
        image_info->increase_rate_between_stitch_info =
            dump_data->data.image.increase_rate_between_stitch_info;
        image_info->overlap_rate_between_last_template =
            dump_data->data.image.overlap_rate_between_last_template;
        image_info->duplicated_finger_id = dump_data->data.image.duplicated_finger_id;
        image_info->increase_rate =
            dump_data->data.image.dump_performance.increase_rate;
        image_info->match_score = dump_data->data.image.match_score;
        image_info->match_finger_id = dump_data->data.image.match_finger_id;
        image_info->study_flag = dump_data->data.image.study_flag;
        image_info->authenticate_update_flag =
            dump_data->data.image.dump_performance.authenticate_update_flag;
        image_info->try_count = dump_data->data.image.dump_performance.try_count;
        image_info->is_final = dump_data->data.image.dump_performance.is_final;
        image_info->frame_num = dump_data->data.image.frame_num;
        image_info->select_index = dump_data->data.image.select_index;
        image_info->image_origin_data_len = dump_data->data.image.origin_data_len;
        image_info->image_raw_data_len = dump_data->data.image.raw_data_len;
        image_info->gsc_untouch_data_len = dump_data->data.image.gsc_untouch_data_len;
        image_info->gsc_touch_data_len = dump_data->data.image.gsc_touch_data_len;
        image_info->bad_point_num =
            dump_data->data.image.dump_performance.bad_point_num;
        image_info->fp_rawdata_max =
            dump_data->data.image.dump_performance.fp_rawdata_max;
        image_info->fp_rawdata_min =
            dump_data->data.image.dump_performance.fp_rawdata_min;
        image_info->fp_rawdata_average =
            dump_data->data.image.dump_performance.fp_rawdata_average;
        image_info->gsc_rawdata_max =
            dump_data->data.image.dump_performance.gsc_rawdata_max;
        image_info->gsc_rawdata_min =
            dump_data->data.image.dump_performance.gsc_rawdata_min;
        image_info->gsc_rawdata_average =
            dump_data->data.image.dump_performance.gsc_rawdata_average;

        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fprintf(data_encoder, "{\n");
        gf_dump_encoder_fprintf(data_encoder, "    \"version\":    \"%s\",\n",
                gf_get_dump_bigdata_version());
        gf_dump_encoder_fprintf(data_encoder, "    \"type\":    %d,\n", image_info->scene_info.type);
        gf_dump_encoder_fprintf(data_encoder, "    \"scene\":    %d,\n", image_info->scene_info.scene);
        gf_dump_encoder_fprintf(data_encoder, "    \"charge\":    %d,\n", image_info->scene_info.charge);
        gf_dump_encoder_fprintf(data_encoder, "    \"screen_status\":    %d,\n",
                image_info->scene_info.screen_status);
        gf_dump_encoder_fprintf(data_encoder, "    \"lock_status\":    %d,\n",
                image_info->scene_info.lock_status);
        gf_dump_encoder_fprintf(data_encoder, "    \"timestamp_keydown\":    %"PRIu64",\n",
                image_info->timestamp_keydown);
        gf_dump_encoder_fprintf(data_encoder, "    \"get_gsc_data_time\":    %d,\n",
                image_info->get_gsc_data_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"get_raw_data_time\":    %d,\n",
                image_info->get_raw_data_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"broken_check_time\":    %d,\n",
                image_info->broken_check_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"preprocess_time\":    %d,\n", image_info->preprocess_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"get_feature_time\":    %d,\n",
                image_info->get_feature_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"enroll_time\":    %d,\n", image_info->enroll_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"authenticate_time\":    %d,\n",
                image_info->authenticate_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"bio_assay_time\":    %d,\n", image_info->bio_assay_time);
        gf_dump_encoder_fprintf(data_encoder, "    \"error_code\":    %d,\n", image_info->error_code);
        gf_dump_encoder_fprintf(data_encoder, "    \"image_quality\":    %d,\n", image_info->image_quality);
        gf_dump_encoder_fprintf(data_encoder, "    \"valid_area\":    %d,\n", image_info->valid_area);
        gf_dump_encoder_fprintf(data_encoder, "    \"template_count\":    %d,\n", image_info->template_count);
        gf_dump_encoder_fprintf(data_encoder, "    \"key_point_num\":    %d,\n", image_info->key_point_num);
        gf_dump_encoder_fprintf(data_encoder, "    \"enrolling_finger_id\":    %u,\n",
                image_info->enrolling_finger_id);
        gf_dump_encoder_fprintf(data_encoder, "    \"overlay\":    %d,\n", image_info->overlay);
        gf_dump_encoder_fprintf(data_encoder, "    \"increase_rate_between_stitch_info\":    %d,\n",
                image_info->increase_rate_between_stitch_info);
        gf_dump_encoder_fprintf(data_encoder, "    \"overlap_rate_between_last_template\":    %d,\n",
                image_info->overlap_rate_between_last_template);
        gf_dump_encoder_fprintf(data_encoder, "    \"duplicated_finger_id\":    %d,\n",
                image_info->duplicated_finger_id);
        gf_dump_encoder_fprintf(data_encoder, "    \"increase_rate\":    %d,\n", image_info->increase_rate);
        gf_dump_encoder_fprintf(data_encoder, "    \"trycount\":    %d,\n", image_info->try_count);
        gf_dump_encoder_fprintf(data_encoder, "    \"isFinal\":    %d,\n", image_info->is_final);
        gf_dump_encoder_fprintf(data_encoder, "    \"match_score\":    %d,\n", image_info->match_score);
        gf_dump_encoder_fprintf(data_encoder, "    \"match_finger_id\":    %d,\n", image_info->match_finger_id);
        gf_dump_encoder_fprintf(data_encoder, "    \"study_flag\":    %d,\n", image_info->study_flag);
        gf_dump_encoder_fprintf(data_encoder, "    \"authenticate_update_flag\":    %d,\n",
                image_info->authenticate_update_flag);
        gf_dump_encoder_fprintf(data_encoder, "    \"frame_num\":    %d,\n", image_info->frame_num);
        gf_dump_encoder_fprintf(data_encoder, "    \"select_index\":    %d,\n", image_info->select_index);
        gf_dump_encoder_fprintf(data_encoder, "    \"image_origin_data_len\":    %d,\n",
                image_info->image_origin_data_len);
        gf_dump_encoder_fprintf(data_encoder, "    \"image_raw_data_len\":    %d,\n",
                image_info->image_raw_data_len);
        gf_dump_encoder_fprintf(data_encoder, "    \"gsc_untouch_data_len\":    %d,\n",
                image_info->gsc_untouch_data_len);
        gf_dump_encoder_fprintf(data_encoder, "    \"gsc_touch_data_len\":    %d,\n",
                image_info->gsc_touch_data_len);
        gf_dump_encoder_fprintf(data_encoder, "    \"badpointNum\":    %d,\n", image_info->bad_point_num);
        gf_dump_encoder_fprintf(data_encoder, "    \"fprawdata_max\":    %d,\n", image_info->fp_rawdata_max);
        gf_dump_encoder_fprintf(data_encoder, "    \"fprawdata_min\":    %d,\n", image_info->fp_rawdata_min);
        gf_dump_encoder_fprintf(data_encoder, "    \"fprawdata_average\":    %d,\n",
                image_info->fp_rawdata_average);
        gf_dump_encoder_fprintf(data_encoder, "    \"gscrawdata_max\":    %d,\n", image_info->gsc_rawdata_max);
        gf_dump_encoder_fprintf(data_encoder, "    \"gscrawdata_min\":    %d,\n", image_info->gsc_rawdata_min);
        gf_dump_encoder_fprintf(data_encoder, "    \"gscrawdata_average\":    %d\n",
                image_info->gsc_rawdata_average);
        gf_dump_encoder_fprintf(data_encoder, "}\n");
        gf_dump_encoder_fclose(data_encoder);
    }  // do...
    while (0);

    if (NULL != image_info)
    {
        GF_MEM_FREE(image_info);
        image_info = NULL;
    }

    return err;
}

/**
 *Function: gf_bigdata_dump_nav_data
 *Description: bigdata dump nav data
 *Input: data_encoder, encoder buffer
 *       filepath, dump file name
 *       dump_data, data to be dump
 *       operation, operation
 *Output: encoded nav data to encoder buffer
 *Return: gf_error_t
 */
gf_error_t gf_bigdata_dump_nav_data(dump_data_encoder_t* data_encoder,
                                    const uint8_t* filepath,
                                    gf_dump_data_t* dump_data,
                                    gf_operation_type_t operation)
{
    gf_error_t err = GF_SUCCESS;
    gf_bigdata_nav_t *nav_info = NULL;

    do
    {
        if (NULL == filepath || NULL == dump_data || NULL == data_encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "[%s] operation = %d", __func__, operation);
        nav_info = (gf_bigdata_nav_t *)GF_MEM_MALLOC(sizeof(gf_bigdata_nav_t));

        if (NULL == nav_info)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(nav_info, 0, sizeof(gf_bigdata_nav_t));

        if (operation == OPERATION_NAV_BASE)
        {
            nav_info->scene_info.type = NAV_BASE;
        }
        else if (operation == OPERATION_NAV)
        {
            nav_info->scene_info.type = NAVIGATION;
        }
        else
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        nav_info->timestamp_keydown = dump_data->down_irq_time / 1000;
        nav_info->nav_code = dump_data->data.nav.nav_code;
        nav_info->nav_times = dump_data->data.nav.nav_times;
        nav_info->nav_origin_data_len = dump_data->data.nav.origin_data_len;
        nav_info->nav_raw_data_len = dump_data->data.nav.raw_data_len;
        nav_info->nav_frame_index = dump_data->data.nav.nav_frame_index;
        nav_info->nav_frame_count = dump_data->data.nav.nav_frame_count;
        memcpy(&nav_info->nav_config, &dump_data->data.nav.nav_config,
               sizeof(nav_info->nav_config));
        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fprintf(data_encoder, "{\n");
        gf_dump_encoder_fprintf(data_encoder, "    \"version\":    \"%s\",\n",
                gf_get_dump_bigdata_version());
        gf_dump_encoder_fprintf(data_encoder, "    \"type\":    %d,\n", nav_info->scene_info.type);
        gf_dump_encoder_fprintf(data_encoder, "    \"scene\":    %d,\n", nav_info->scene_info.scene);
        gf_dump_encoder_fprintf(data_encoder, "    \"charge\":    %d,\n", nav_info->scene_info.charge);
        gf_dump_encoder_fprintf(data_encoder, "    \"screen_status\":    %d,\n",
                nav_info->scene_info.screen_status);
        gf_dump_encoder_fprintf(data_encoder, "    \"lock_status\":    %d,\n",
                nav_info->scene_info.lock_status);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_code\":    %d,\n", nav_info->nav_code);
        gf_dump_encoder_fprintf(data_encoder, "    \"cover_type\":    %d,\n", nav_info->nav_config.cover_type);
        gf_dump_encoder_fprintf(data_encoder, "    \"inertia_x\":    %d,\n", nav_info->nav_config.inertia_x);
        gf_dump_encoder_fprintf(data_encoder, "    \"inertia_y\":    %d,\n", nav_info->nav_config.inertia_y);
        gf_dump_encoder_fprintf(data_encoder, "    \"static_x\":    %d,\n", nav_info->nav_config.static_x);
        gf_dump_encoder_fprintf(data_encoder, "    \"static_y\":    %d,\n", nav_info->nav_config.static_y);
        gf_dump_encoder_fprintf(data_encoder, "    \"sad_x_off_thr\":    %d,\n", nav_info->nav_config.sad_x_off_thr);
        gf_dump_encoder_fprintf(data_encoder, "    \"sad_y_off_thr\":    %d,\n", nav_info->nav_config.sad_y_off_thr);
        gf_dump_encoder_fprintf(data_encoder, "    \"max_nvg_frame_num\":    %d,\n",
                nav_info->nav_config.max_nvg_frame_num);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_origin_data_len\":    %d,\n",
                nav_info->nav_origin_data_len);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_raw_data_len\":    %d,\n", nav_info->nav_raw_data_len);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_times\":    %d,\n", nav_info->nav_times);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_frame_index\":    %d,\n", nav_info->nav_frame_index);
        gf_dump_encoder_fprintf(data_encoder, "    \"nav_frame_count\":    %d\n", nav_info->nav_frame_count);
        gf_dump_encoder_fprintf(data_encoder, "}\n");
        gf_dump_encoder_fclose(data_encoder);
    }  // do...
    while (0);

    if (NULL != nav_info)
    {
        GF_MEM_FREE(nav_info);
        nav_info = NULL;
    }

    return err;
}

/**
 *Function: gf_bigdata_dump_basic_json_data
 *Description: bigdata dump nav data
 *Input: data_encoder, encoder buffer
 *       filepath, dump file name
 *       dump_data, data to be dump
 *       operation, operation
 *Output: encoded basic json to encoder buffer
 *Return: gf_error_t
 */
gf_error_t gf_bigdata_dump_basic_json_data(dump_data_encoder_t* data_encoder,
                                           const uint8_t* filepath,
                                           gf_dump_data_t* dump_data,
                                           gf_operation_type_t operation)
{
    gf_error_t err = GF_SUCCESS;
    gf_bigdata_basic_json_t *json_info = NULL;

    do
    {
        if (NULL == data_encoder || NULL == filepath || NULL == dump_data)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "[%s] operation = %d", __func__, operation);
        json_info = (gf_bigdata_basic_json_t *)GF_MEM_MALLOC(sizeof(gf_bigdata_basic_json_t));

        if (NULL == json_info)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(json_info, 0, sizeof(gf_bigdata_basic_json_t));

        if (operation < OPERATION_ENROLL || operation > OPERATION_MAX)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        json_info->scene_info.type = MAX;
        json_info->scene_info.screen_status = dump_data->screen_flag;
        err = gf_dump_encoder_fopen(data_encoder, filepath, DATA_TYPE_NORMAL_FILE_DATA);
        BREAK_IF_ERROR(err);  // break if error

        gf_dump_encoder_fprintf(data_encoder, "{\n");
        gf_dump_encoder_fprintf(data_encoder, "    \"version\":    \"%s\",\n",
                gf_get_dump_bigdata_version());
        gf_dump_encoder_fprintf(data_encoder, "    \"type\":    %d,\n", json_info->scene_info.type);
        gf_dump_encoder_fprintf(data_encoder, "    \"scene\":    %d,\n", json_info->scene_info.scene);
        gf_dump_encoder_fprintf(data_encoder, "    \"charge\":    %d,\n", json_info->scene_info.charge);
        gf_dump_encoder_fprintf(data_encoder, "    \"screen_status\":    %d,\n",
                json_info->scene_info.screen_status);
        gf_dump_encoder_fprintf(data_encoder, "    \"lock_status\":    %d\n",
                json_info->scene_info.lock_status);
        gf_dump_encoder_fprintf(data_encoder, "}\n");
        gf_dump_encoder_fclose(data_encoder);
    }  // do...
    while (0);

    if (NULL != json_info)
    {
        GF_MEM_FREE(json_info);
        json_info = NULL;
    }

    return err;
}
