/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "gf_fingerprint.h"
#include "gf_common.h"

#include "gf_ca_entry.h"
#include "gf_aes.h"
#include "gf_user_type_define.h"
#include "gf_dump_data.h"
#include "gf_hal.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_hal_device.h"
#include "gf_hal_frr_database.h"
#include "gf_hal_timer.h"
#include "gf_hal_test_utils.h"
#include "gf_hal_milan.h"
#include "gf_hal_milan_an_series.h"

#include "gf_hal_test.h"
#include "gf_hal_mem.h"
#include "SNR_Cal.h"

#include "fingerprint_type.h"

#define LOG_TAG "[GF_HAL][gf_hal_test]"

#define GF_DUMP_FILE_TIME_MAX_LEN 64
#define GF_SENSOR_BROKEN_CNT_MAX      20
static void hal_notify_image_quality(int32_t quality);

static timer_t g_test_spi_transfer_timer_id = 0;  // id of test spi transfer timer
pthread_mutex_t g_test_interrupt_pin_mutex = PTHREAD_MUTEX_INITIALIZER;  // test interrupt pin mutex
pthread_mutex_t g_sensor_broken_mutex = PTHREAD_MUTEX_INITIALIZER;  // broken mutex
uint32_t g_test_spi_transfer_times = 0;  // test spi trnasfer times
uint8_t g_test_interrupt_pin_flag = 0;  // test interrupt pin flags
uint32_t g_test_sensor_fine_count = 0;  // test sensor fine count
uint32_t g_fpc_config_had_downloaded = 0;  // test fpc key downLoad cfg flag

// for snr test
uint8_t g_stable_max_rawdata_framenum = 5;  // g_stable_max_rawdata_framenum
uint8_t g_stable_max_basedata_framenum = 10;  // g_stable_max_basedata_framenum
static timer_t g_stable_test_timeout_timer_id = 0;

static uint8_t g_sensor_broken_count = 0;
static int64_t g_last_detect_time = 0;
uint32_t g_sensor_broken_err = GF_SUCCESS;
uint8_t g_sensor_broken_check_mode = 0;

uint32_t GF_STABLE_TEST_TIME_OUT_THRESHOLD = 30;   // timeout = 30s

//for oppo
gf_error_t hal_test_synchronous_pixel_open()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_test_pixel_open *cmd = NULL;
    uint32_t retry=2;
    FUNC_ENTER();

    do {
        uint32_t size = sizeof(gf_cmd_test_pixel_open);
        cmd = (gf_cmd_test_pixel_open *) GF_MEM_MALLOC(size);
        if (NULL == cmd) {
            LOG_E(LOG_TAG, "[%s] cmd out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        do
        {
            memset(cmd, 0, size);
            err = gf_hal_test_invoke_command(GF_CMD_TEST_SYNCHRONOUS_PIXEL_OPEN, cmd, size);
            if (cmd->cmd_header.reset_flag)
            {
               gf_hal_test_invoke_command_ex(GF_CMD_TEST_RESET_CLEAR);
               continue;
            }
            break;
        } while(retry--);

        gf_hal_dump_data_by_operation(OPERATION_TEST_PIXEL_OPEN_STEP1, err);
        gf_hal_dump_data_by_operation(OPERATION_TEST_PIXEL_OPEN_STEP2, err);

    } while(0);

    if (cmd != NULL) {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static void hal_notify_image_quality(int32_t quality)
{
    VOID_FUNC_ENTER();

    fingerprint_msg_t message;
    LOG_I(LOG_TAG, "[%s] quality = %d", __func__, quality);
    if(NULL == g_fingerprint_device || NULL == g_fingerprint_device->notify || quality <= 0) {
        return;
    }
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = ENGINEERING_INFO;
    message.data.engineering.type = FINGERPRINT_IMAGE_QUALITY;
    message.data.engineering.quality.successed = 1;
    message.data.engineering.quality.image_quality = quality;
    if(quality >= fp_config_info_init.fp_image_quality_pass_score)
        message.data.engineering.quality.quality_pass = 1;
    else 
        message.data.engineering.quality.quality_pass = 0;
    g_fingerprint_device->notify(&message);

    VOID_FUNC_EXIT();
}

static void hal_notify_test_cmd(int32_t cmd_id, uint8_t *msg_info,
                                uint32_t len)
{
    VOID_FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] cmd_id=%d", __func__, cmd_id);

    if (NULL == g_fingerprint_device || NULL == g_fingerprint_device->test_notify
        || NULL == msg_info)
    {
        return;
    }

    gf_fingerprint_msg_t message = { 0 };
    message.type = GF_FINGERPRINT_TEST_CMD;
    message.data.test.cmd_id = cmd_id;
    message.data.test.result = (int8_t *) msg_info;
    message.data.test.result_len = len;
    g_fingerprint_device->test_notify(&message);
    VOID_FUNC_EXIT();
}

void hal_notify_test_spi_performance(gf_error_t err,
                                     gf_test_spi_performance_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    LOG_D(LOG_TAG, "test spi get_dr_timestamp_time=%uus",
          result->get_dr_timestamp_time);
    LOG_D(LOG_TAG, "test spi         get_mode_time=%uus", result->get_mode_time);
    LOG_D(LOG_TAG, "test spi   get_fw_version_time=%uus",
          result->get_fw_version_time);
    LOG_D(LOG_TAG, "test spi        get_image_time=%ums",
          result->get_image_time / 1000);
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_dr_timestamp_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_mode_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_chip_id_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_vendor_id_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_sensor_id_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_fw_version_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_image_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t raw_data_len;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t fw_version[FW_VERSION_INFO_LEN];
    len += HAL_TEST_SIZEOF_ARRAY(FW_VERSION_INFO_LEN);
    // uint8_t chip_id[GF_CHIP_ID_LEN];
    len += HAL_TEST_SIZEOF_ARRAY(GF_CHIP_ID_LEN);
    // uint8_t vendor_id[GF_VENDOR_ID_LEN];
    len += HAL_TEST_SIZEOF_ARRAY(GF_VENDOR_ID_LEN);
    // uint8_t sensor_id[GF_SENSOR_ID_LEN];
    len += HAL_TEST_SIZEOF_ARRAY(GF_SENSOR_ID_LEN);
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_DR_TIMESTAMP_TIME,
                                        result->get_dr_timestamp_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_MODE_TIME,
                                        result->get_mode_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_CHIP_ID_TIME,
                                        result->get_chip_id_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_VENDOR_ID_TIME,
                                        result->get_vendor_id_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_SENSOR_ID_TIME,
                                        result->get_sensor_id_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_FW_VERSION_TIME,
                                        result->get_fw_version_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_IMAGE_TIME,
                                        result->get_image_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_RAW_DATA_LEN,
                                        result->raw_data_len);
        current = hal_test_encode_array(current, TEST_TOKEN_FW_VERSION,
                                        result->fw_version,
                                        FW_VERSION_INFO_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_CHIP_ID, result->chip_id,
                                        GF_CHIP_ID_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_VENDOR_ID,
                                        result->vendor_id,
                                        GF_VENDOR_ID_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_SENSOR_ID,
                                        result->sensor_id,
                                        GF_SENSOR_ID_LEN);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_SPI_PERFORMANCE, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_spi_transfer(gf_error_t err, int32_t remainings)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] spi transfer remainings=%d", __func__, remainings);

    if (remainings == 0 || err != GF_SUCCESS)
    {
        gf_hal_destroy_timer(&g_test_spi_transfer_timer_id);
        g_hal_function.cancel((void *) g_fingerprint_device);
    }

    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // remainings
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        uint8_t *current = test_result;
        memset(test_result, 0, len);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_SPI_TRANSFER_REMAININGS,
                                        remainings);
        hal_notify_test_memory_check(__func__, test_result, current, len);
        hal_notify_test_cmd(CMD_TEST_SPI_TRANSFER, test_result, len);
        GF_MEM_FREE(test_result);
    }
    else
    {
        LOG_E(LOG_TAG, "[%s] malloc memory failed", __func__);
    }

    VOID_FUNC_EXIT();
}

static void hal_oswego_notify_test_bad_point(gf_error_t err,
                                             gf_bad_point_test_result_oswego_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    LOG_D(LOG_TAG, "test bad point             m_avg_diff_val=%u",
          result->m_avg_diff_val);
    LOG_D(LOG_TAG, "test bad point                  m_noise=%f", result->m_noise);
    LOG_D(LOG_TAG, "test bad point            m_bad_pixel_num=%u",
          result->m_bad_pixel_num);
    LOG_D(LOG_TAG, "test bad point  m_local_small_bad_pixel_num=%u",
          result->m_local_small_bad_pixel_num);
    LOG_D(LOG_TAG, "test bad point    m_local_big_bad_pixel_num=%u",
          result->m_local_big_bad_pixel_num);
    LOG_D(LOG_TAG, "test bad point    m_flatness_bad_pixel_num=%u",
          result->m_flatness_bad_pixel_num);
    LOG_D(LOG_TAG, "test bad point              m_is_bad_line=%u",
          result->m_is_bad_line);
    LOG_D(LOG_TAG, "test bad point           m_all_tilt_angle=%f",
          result->m_all_tilt_angle);
    LOG_D(LOG_TAG, "test bad point      m_block_tilt_angle_max=%f",
          result->m_block_tilt_angle_max);
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // unsigned short m_avg_diff_val;
    len += HAL_TEST_SIZEOF_INT16;
    // double m_noise;
    len += HAL_TEST_SIZEOF_DOUBLE;
    // unsigned int m_bad_pixel_num;
    len += HAL_TEST_SIZEOF_INT32;
    // unsigned int m_local_small_bad_pixel_num
    len += HAL_TEST_SIZEOF_INT32;
    // unsigned int m_local_big_bad_pixel_num;
    len += HAL_TEST_SIZEOF_INT32;
    // unsigned int m_flatness_bad_pixel_num;
    len += HAL_TEST_SIZEOF_INT32;
    // unsigned int m_is_bad_line
    len += HAL_TEST_SIZEOF_INT32;
    // float m_all_tilt_angle;
    len += HAL_TEST_SIZEOF_FLOAT;
    // float m_block_tilt_angle_max;
    len += HAL_TEST_SIZEOF_FLOAT;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int16(current, TEST_TOKEN_AVG_DIFF_VAL,
                                        result->m_avg_diff_val);
        current = hal_test_encode_double(current, TEST_TOKEN_NOISE, result->m_noise);
        current = hal_test_encode_int32(current, TEST_TOKEN_BAD_PIXEL_NUM,
                                        result->m_bad_pixel_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_LOCAL_SMALL_BAD_PIXEL_NUM,
                                        result->m_local_small_bad_pixel_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_LOCAL_BIG_BAD_PIXEL_NUM,
                                        result->m_local_big_bad_pixel_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_FLATNESS_BAD_PIXEL_NUM,
                                        result->m_flatness_bad_pixel_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_IS_BAD_LINE,
                                        result->m_is_bad_line);
        current = hal_test_encode_float(current, TEST_TOKEN_ALL_TILT_ANGLE,
                                        result->m_all_tilt_angle);
        current = hal_test_encode_float(current, TEST_TOKEN_BLOCK_TILT_ANGLE_MAX,
                                        result->m_block_tilt_angle_max);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_BAD_POINT, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

static void hal_milan_notify_test_bad_point(gf_error_t err,
                                            gf_bad_point_test_result_milan_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();
    LOG_D(LOG_TAG, "bad point test             total=%u", result->total);
    LOG_D(LOG_TAG, "bad point test             local=%u", result->local);
    LOG_D(LOG_TAG, "bad point test        localWorst=%u", result->local_worst);
    LOG_D(LOG_TAG, "bad point test          singular=%u", result->singular);
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // uint16_t total; TODO: use oswego size: unsigned int m_bad_pixel_num;
    len += HAL_TEST_SIZEOF_INT32;
    // uint16_t local; TODO: use oswego size: unsigned int m_localBadPixelNum;
    len += HAL_TEST_SIZEOF_INT32;
    // uint16_t local_worst;
    len += HAL_TEST_SIZEOF_INT16;
    // uint32_t singular;
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_BAD_PIXEL_NUM,
                                        result->total);
        current = hal_test_encode_int32(current, TEST_TOKEN_LOCAL_BAD_PIXEL_NUM,
                                        result->local);
        current = hal_test_encode_int16(current, TEST_TOKEN_LOCAL_WORST,
                                        result->local_worst);
        current = hal_test_encode_int32(current, TEST_TOKEN_SINGULAR, result->singular);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_BAD_POINT, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_performance(gf_error_t err, gf_test_performance_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER()

    if (NULL == result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    LOG_D(LOG_TAG, "test performance       img_quality=%d", result->image_quality);
    LOG_D(LOG_TAG, "test performance        valid_area=%d", result->valid_area);
    LOG_D(LOG_TAG, "test performance     key_point_num=%d", result->key_point_num);
    LOG_D(LOG_TAG, "test performance     increase_rate=%u%%",
          result->increase_rate);
    LOG_D(LOG_TAG, "test performance           overlay=%u%%", result->overlay);
    LOG_D(LOG_TAG, "test performance get_raw_data_time=%ums",
          result->get_raw_data_time / 1000);
    LOG_D(LOG_TAG, "test performance   preprocess_time=%ums",
          result->preprocess_time / 1000);
    LOG_D(LOG_TAG, "test performance  get_feature_time=%ums",
          result->get_feature_time / 1000);
    LOG_D(LOG_TAG, "test performance       enroll_time=%ums",
          result->enroll_time / 1000);
    LOG_D(LOG_TAG, "test performance authenticate_time=%ums",
          result->authenticate_time / 1000);
    LOG_D(LOG_TAG, "test performance          gsc_time=%uus",
          result->get_gsc_data_time);
    LOG_D(LOG_TAG, "test performance    bio_assay_time=%uus",
          result->bio_assay_time);

    if (result->authenticate_finger_count > 0)
    {
        LOG_D(LOG_TAG, "test performance authenticate total time=%ums, fingers=%u",
              result->authenticate_finger_time / 1000, result->authenticate_finger_count);
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // int32_t image_quality;
    len += HAL_TEST_SIZEOF_INT32;
    // int32_t valid_area;
    len += HAL_TEST_SIZEOF_INT32;
    // int32_t key_point_num;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t increase_rate;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t overlay;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_raw_data_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t preprocess_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t get_feature_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t enroll_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t authenticate_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t authenticate_update_flag;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t authenticate_finger_count;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t authenticate_finger_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t total_time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t gsc time;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t bio assay time
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_IMAGE_QUALITY,
                                        result->image_quality);
        current = hal_test_encode_int32(current, TEST_TOKEN_VALID_AREA,
                                        result->valid_area);
        current = hal_test_encode_int32(current, TEST_TOKEN_KEY_POINT_NUM,
                                        result->key_point_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_INCREATE_RATE,
                                        result->increase_rate);
        current = hal_test_encode_int32(current, TEST_TOKEN_OVERLAY, result->overlay);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_RAW_DATA_TIME,
                                        result->get_raw_data_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_PREPROCESS_TIME,
                                        result->preprocess_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_FEATURE_TIME,
                                        result->get_feature_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_ENROLL_TIME,
                                        result->enroll_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATE_TIME,
                                        result->authenticate_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATE_UPDATE_FLAG,
                                        result->authenticate_update_flag);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATE_FINGER_COUNT,
                                        result->authenticate_finger_count);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATE_FINGER_ITME,
                                        result->authenticate_finger_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_TOTAL_TIME,
                                        result->total_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_GSC_DATA_TIME,
                                        result->get_gsc_data_time / 1000);
        current = hal_test_encode_int32(current, TEST_TOKEN_BIO_ASSAY_TIME,
                                        result->bio_assay_time / 1000);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    // Factory test about PERFORMANCE
    if (NULL != g_fingerprint_device->factory_test_notify)
    {
        g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_PERFORMANCE,
                                                  err, (void *)result);
    }

    hal_notify_image_quality(result->image_quality); //for oplus
    hal_notify_test_cmd(CMD_TEST_PERFORMANCE, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_reset_pin(gf_error_t err)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // int32_t reset_flag;
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_RESET_FLAG, 1);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_RESET_PIN, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }
}

void hal_notify_test_interrupt_pin(gf_error_t err)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    // Factory test about INTERRUPT_PIN
    if (NULL != g_fingerprint_device->factory_test_notify)
    {
        g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_INTERRUPT_PIN, err,
                                                  NULL);
    }

    hal_notify_test_cmd(CMD_TEST_INTERRUPT_PIN, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }
}

void hal_notify_test_pixel_open(gf_error_t err, uint32_t bad_pixel_num, uint32_t local_bad_pixel_num)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t bad_pixel_num;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t local bad point
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_BAD_PIXEL_NUM,
                                        bad_pixel_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_LOCAL_BAD_PIXEL_NUM,
                                        local_bad_pixel_num);

        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    // Factory test result for PIXEL_OPEN
    if (NULL != g_fingerprint_device->factory_test_notify)
    {
        g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_PIXEL_OPEN, err,
                                                  (void *)&bad_pixel_num);
    }

    hal_notify_test_cmd(CMD_TEST_PIXEL_OPEN, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }
}

void hal_notify_test_pixel_short_streak(gf_error_t err, uint32_t bad_pixel_short_streak_num)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t bad_pixel_short_streak_num;
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_BAD_PIXEL_SHORT_STREAK_NUM,
                                        bad_pixel_short_streak_num);

        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    // Factory test result for PIXEL_SHORT_STREAK
    if (NULL != g_fingerprint_device->factory_test_notify)
    {
        g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_PIXEL_SHORT_STREAK, err,
                                                  (void *)&bad_pixel_short_streak_num);
    }

    hal_notify_test_cmd(CMD_TEST_PIXEL_SHORT_STREAK, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }
}

void hal_notify_test_sensor_fine(gf_error_t err, uint32_t average_pixel_diff)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t average_pixel_diff;
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_AVERAGE_PIXEL_DIFF,
                                        average_pixel_diff);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_SENSOR_FINE, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }
}

void hal_notify_test_real_time_data(gf_error_t err,
                                    gf_test_real_time_data_t *result)
{
    uint8_t *test_result = NULL;
    VOID_FUNC_ENTER();
    {
        uint32_t len = 0;
        uint32_t array_len = g_sensor_row * g_sensor_col;

        if (NULL == result)
        {
            LOG_E(LOG_TAG, "[%s] result is null", __func__);
            return;
        }

        // chip type
        len += HAL_TEST_SIZEOF_INT32;
        // chip series
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // rawdata
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(uint16_t) * array_len);
        // base
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(uint16_t) * array_len);
        // Kr
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(int16_t) * array_len);
        // B
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(int16_t) * array_len);
        // FPC_KEY_VALUE
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(uint8_t) * 4);
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL != test_result)
        {
            memset(test_result, 0, len);
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                            g_hal_config.chip_type);
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                            g_hal_config.chip_series);
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
            current = hal_test_encode_array(current, TEST_TOKEN_RAW_DATA,
                                            (uint8_t *) result->raw_data, sizeof(uint16_t) * array_len);
            current = hal_test_encode_array(current, TEST_TOKEN_BASE_DATA,
                                            (uint8_t *) result->base_data, sizeof(uint16_t) * array_len);
            current = hal_test_encode_array(current, TEST_TOKEN_KR_DATA,
                                            (uint8_t *) result->kr_data, sizeof(int16_t) * array_len);
            current = hal_test_encode_array(current, TEST_TOKEN_B_DATA,
                                            (uint8_t *) result->b_data,
                                            sizeof(int16_t) * array_len);
            current = hal_test_encode_array(current, TEST_TOKEN_FPC_KEY_DATA,
                                            (uint8_t *) result->fpc_key_data, sizeof(uint8_t) * 4);
            hal_notify_test_memory_check(__func__, test_result, current, len);
        }
        else
        {
            len = 0;
        }

        hal_notify_test_cmd(CMD_TEST_REAL_TIME_DATA, test_result, len);

        if (test_result != NULL)
        {
            GF_MEM_FREE(test_result);
            test_result = NULL;
        }
    }
    VOID_FUNC_EXIT();
}

void hal_notify_test_bmp_data(gf_error_t err, gf_test_bmp_data_t *result)
{
    uint8_t *test_result = NULL;
    VOID_FUNC_ENTER();
    {
        uint32_t len = 0;
        uint32_t array_len = g_sensor_row * g_sensor_col;

        if (NULL == result)
        {
            LOG_E(LOG_TAG, "[%s] result is null", __func__);
            return;
        }

        // chip type
        len += HAL_TEST_SIZEOF_INT32;
        // chip series
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // image quality
        len += HAL_TEST_SIZEOF_INT32;
        // valid area
        len += HAL_TEST_SIZEOF_INT32;
        // bmpdata
        len += HAL_TEST_SIZEOF_ARRAY(array_len);
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (test_result != NULL)
        {
            memset(test_result, 0, len);
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                            g_hal_config.chip_type);
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                            g_hal_config.chip_series);
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
            current = hal_test_encode_int32(current, TEST_TOKEN_IMAGE_QUALITY,
                                            result->image_quality);
            current = hal_test_encode_int32(current, TEST_TOKEN_VALID_AREA,
                                            result->valid_area);
            current = hal_test_encode_array(current, TEST_TOKEN_BMP_DATA,
                                            (uint8_t *) result->bmp_data, sizeof(uint8_t) * array_len);
            hal_notify_test_memory_check(__func__, test_result, current, len);
        }
        else
        {
            len = 0;
        }

        hal_notify_test_cmd(CMD_TEST_BMP_DATA, test_result, len);

        if (test_result != NULL)
        {
            GF_MEM_FREE(test_result);
            test_result = NULL;
        }
    }
    VOID_FUNC_EXIT();
}

void hal_notify_test_stable_factor(gf_error_t err, float result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // stable_factor
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) malloc(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        LOG_D(LOG_TAG, "[%s] test_stable_factor err = %d,", __func__, err);
        current = hal_test_encode_float32(current, TEST_TOKEN_STABLE_FACTOR_RESULT,
                                          result);
        LOG_D(LOG_TAG, "[%s] test_stable_factor result = %f,", __func__, result);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_STABLE_FACTOR, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_twill_badpoint(gf_error_t err, twill_badpoint_result_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    // uint32_t array_len = g_sensor_row * g_sensor_col;
    VOID_FUNC_ENTER();

    if (NULL == result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // total
    len += HAL_TEST_SIZEOF_INT32;
    // local
    len += HAL_TEST_SIZEOF_INT32;
    // numlocal
    len += HAL_TEST_SIZEOF_INT32;
    // line
    len += HAL_TEST_SIZEOF_INT32;
    // mat
    // len += HAL_TEST_SIZEOF_ARRAY(array_len);
    // local mat
    // len += HAL_TEST_SIZEOF_ARRAY(array_len);
    test_result = (uint8_t *) malloc(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        LOG_D(LOG_TAG, "[%s] twill badpoint algo result = %d,", __func__, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_TWILL_BADPOINT_TOTAL_RESULT,
                                          result->algo_badpoint_total_result);
        current = hal_test_encode_int32(current, TEST_TOKEN_TWILL_BADPOINT_LOCAL_RESULT,
                                          result->algo_badpoint_local_result);
        current = hal_test_encode_int32(current, TEST_TOKEN_TWILL_BADPOINT_NUMLOCAL_RESULT,
                                          result->algo_badpoint_numlocal_result);
        current = hal_test_encode_int32(current, TEST_TOKEN_TWILL_BADPOINT_LINE_RESULT,
                                          result->algo_badpoint_line_result);
        // current = hal_test_encode_array(current, TEST_TOKEN_TWILL_BADPOINT_MAT_RESULT,
        //                     (uint8_t *) result->algo_badpoint_mat_result, sizeof(uint8_t) * array_len);
        // current = hal_test_encode_array(current, TEST_TOKEN_TWILL_BADPOINT_LOCAL_MAT_RESULT,
        //                     (uint8_t *) result->algo_badpoint_localmat_result, sizeof(uint8_t) * array_len);
        LOG_D(LOG_TAG, "[%s] badpoint_total_result = %d,",
                __func__, result->algo_badpoint_total_result);
        LOG_D(LOG_TAG, "[%s] badpoint_local_result = %d,",
                __func__, result->algo_badpoint_local_result);
        LOG_D(LOG_TAG, "[%s] badpoint_numlocal_result = %d,",
                __func__, result->algo_badpoint_numlocal_result);
        LOG_D(LOG_TAG, "[%s] badpoint_line_result = %d,",
                __func__, result->algo_badpoint_line_result);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_TWILL_BADPOINT, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_data_noise(gf_error_t err, snr_result_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // product id
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // snr
    len += HAL_TEST_SIZEOF_INT32;
    // signal
    len += HAL_TEST_SIZEOF_INT32;
    // noise
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) malloc(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_PRODUCT_ID,
                                        result->product_id);
        LOG_D(LOG_TAG, "[%s] product_id = 0x%x,", __func__, result->product_id);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        LOG_D(LOG_TAG, "[%s] snr algo result = %d,", __func__, err);
        current = hal_test_encode_float32(current, TEST_TOKEN_DATA_NOISE_RESULT,
                                          result->algo_snr);
        current = hal_test_encode_float32(current, TEST_TOKEN_DATA_NOISE_SIGNAL,
                                          result->algo_signal);
        current = hal_test_encode_float32(current, TEST_TOKEN_DATA_NOISE_NOISE,
                                          result->algo_noise);
        LOG_D(LOG_TAG, "[%s] snr snr value = %f,", __func__, result->algo_snr);
        LOG_D(LOG_TAG, "[%s] snr signal = %f,", __func__, result->algo_signal);
        LOG_D(LOG_TAG, "[%s] snr noise = %f,", __func__, result->algo_noise);
        LOG_D(LOG_TAG, "[%s] snr selectPercentage = %f,", __func__,
              result->algo_select_percentage);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_NOISE, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_frr_far_record_calibration(gf_error_t err,
                                                gf_test_calibration_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    do
    {
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // uint32_t frameNum;
        len += HAL_TEST_SIZEOF_INT32;
        // uint16_t base_data[...];
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(result->base_data));
        // int16_t kr_data[...];
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(result->kr_data));
        // int16_t b_data[...];
        len += HAL_TEST_SIZEOF_ARRAY(sizeof(result->b_data));
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == test_result)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            len = 0;
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_FRAME_NUM,
                                        result->frame_num);
        current = hal_test_encode_array(current, TEST_TOKEN_BASE_DATA,
                                        (uint8_t *) result->base_data, sizeof(result->base_data));
        current = hal_test_encode_array(current, TEST_TOKEN_KR_DATA,
                                        (uint8_t *) result->kr_data, sizeof(result->kr_data));
        current = hal_test_encode_array(current, TEST_TOKEN_B_DATA,
                                        (uint8_t *) result->b_data, sizeof(result->b_data));
        hal_notify_test_cmd(CMD_TEST_FRR_FAR_RECORD_CALIBRATION, test_result, len);
    }
    while (0);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_frr_far_record_enroll(gf_error_t err,
                                           gf_test_frr_far_t *result,
                                           gf_test_frr_far_reserve_data_t *reserve_result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result || NULL == reserve_result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // rawdata
    len += HAL_TEST_SIZEOF_ARRAY(sizeof(uint16_t) * g_sensor_row * g_sensor_col);
    // base
    len += HAL_TEST_SIZEOF_ARRAY(sizeof(reserve_result->b_data));
    // kr
    len += HAL_TEST_SIZEOF_ARRAY(sizeof(reserve_result->kr_data));
    // image quality
    len += HAL_TEST_SIZEOF_INT32;
    // valid area
    len += HAL_TEST_SIZEOF_INT32;
    // bmpdata
    len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
    {
        // bmpdecdata
        len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    }
    // algo index
    len += HAL_TEST_SIZEOF_INT32;
    // gsc buffer
    len += HAL_TEST_SIZEOF_ARRAY(GF_LIVE_DATA_LEN * sizeof(int32_t));
    // broken mask
    len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
    {
        //  touch mask
        len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    }
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_array(current, TEST_TOKEN_RAW_DATA,
                                        (uint8_t *) result->raw_data,
                                        sizeof(uint16_t) * g_sensor_row * g_sensor_col);
        current = hal_test_encode_array(current, TEST_TOKEN_B_DATA,
                                        (uint8_t *) reserve_result->b_data, sizeof(reserve_result->b_data));
        current = hal_test_encode_array(current, TEST_TOKEN_KR_DATA,
                                        (uint8_t *) reserve_result->kr_data, sizeof(reserve_result->kr_data));
        current = hal_test_encode_int32(current, TEST_TOKEN_IMAGE_QUALITY,
                                        result->image_quality);
        current = hal_test_encode_int32(current, TEST_TOKEN_VALID_AREA,
                                        result->valid_area);
        current = hal_test_encode_array(current, TEST_TOKEN_BMP_DATA,
                                        (uint8_t *) result->bmp_data,
                                        g_sensor_row * g_sensor_col);
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            current = hal_test_encode_array(current, TEST_TOKEN_BMPDEC_DATA,
                                            (uint8_t *) reserve_result->bmp2_data,
                                            g_sensor_row * g_sensor_col);
        }
        current = hal_test_encode_int32(current, TEST_TOKEN_ALGO_INDEX,
                                        result->algo_index);
        current = hal_test_encode_array(current, TEST_TOKEN_GSC_DATA,
                                        (uint8_t *) result->gsc_data,
                                        GF_LIVE_DATA_LEN * sizeof(int32_t));
        current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_BROKEN_MASK_DATA,
                                        (uint8_t *) result->broken_mask,
                                        g_sensor_row * g_sensor_col);
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_TOUCH_MASK_DATA,
                                            (uint8_t *) reserve_result->touch_mask,
                                            g_sensor_row * g_sensor_col);
        }
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_FRR_FAR_RECORD_ENROLL, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_frr_far_record_authenticate(gf_error_t err,
                                                 gf_test_frr_far_t *result,
                                                 gf_test_frr_far_reserve_data_t *reserve_result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result || NULL == reserve_result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // rawdata
    len += HAL_TEST_SIZEOF_ARRAY(sizeof(uint16_t) * g_sensor_row * g_sensor_col);
    // base
    len += HAL_TEST_SIZEOF_ARRAY(sizeof(reserve_result->b_data));
    // kr
    len += HAL_TEST_SIZEOF_ARRAY(sizeof(reserve_result->kr_data));
    // image quality
    len += HAL_TEST_SIZEOF_INT32;
    // valid area
    len += HAL_TEST_SIZEOF_INT32;
    // bmpdata
    len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
    {
        // bmpdecdata
        len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    }
    // exchange index
    len += HAL_TEST_SIZEOF_INT32;
    // gsc flag
    len += HAL_TEST_SIZEOF_INT32;
    // gsc buffer
    len += HAL_TEST_SIZEOF_ARRAY(GF_LIVE_DATA_LEN * sizeof(int32_t));
    // broken mask
    len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
    {
        // touch mask
        len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    }
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_array(current, TEST_TOKEN_RAW_DATA,
                                        (uint8_t *) result->raw_data,
                                        sizeof(uint16_t) * g_sensor_row * g_sensor_col);
        current = hal_test_encode_array(current, TEST_TOKEN_B_DATA,
                                        (uint8_t *) reserve_result->b_data, sizeof(reserve_result->b_data));
        current = hal_test_encode_array(current, TEST_TOKEN_KR_DATA,
                                        (uint8_t *) reserve_result->kr_data, sizeof(reserve_result->kr_data));
        current = hal_test_encode_int32(current, TEST_TOKEN_IMAGE_QUALITY,
                                        result->image_quality);
        current = hal_test_encode_int32(current, TEST_TOKEN_VALID_AREA,
                                        result->valid_area);
        current = hal_test_encode_array(current, TEST_TOKEN_BMP_DATA,
                                        (uint8_t *) result->bmp_data,
                                        g_sensor_row * g_sensor_col);
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            current = hal_test_encode_array(current, TEST_TOKEN_BMPDEC_DATA,
                                            (uint8_t *) reserve_result->bmp2_data,
                                            g_sensor_row * g_sensor_col);
        }
        current = hal_test_encode_int32(current, TEST_TOKEN_ALGO_INDEX,
                                        result->algo_index);
        current = hal_test_encode_int32(current, TEST_TOKEN_GSC_FLAG, result->gsc_flag);
        current = hal_test_encode_array(current, TEST_TOKEN_GSC_DATA,
                                        (uint8_t *) result->gsc_data,
                                        GF_LIVE_DATA_LEN * sizeof(int32_t));
        current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_BROKEN_MASK_DATA,
                                        (uint8_t *) result->broken_mask,
                                        g_sensor_row * g_sensor_col);
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_TOUCH_MASK_DATA,
                                            (uint8_t *) reserve_result->touch_mask,
                                            g_sensor_row * g_sensor_col);
        }
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_sensor_validity(gf_error_t err, int32_t status)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // int32_t status
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_SENSOR_VALIDITY, status);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_SENSOR_VALIDITY, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

static void hal_notify_test_frr_far_play_calibration(gf_error_t err,
                                                     gf_test_calibration_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    UNUSED_VAR(result);
    VOID_FUNC_ENTER();

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_FRR_FAR_PLAY_CALIBRATION, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

static void hal_notify_test_frr_far_play_enroll(gf_error_t err,
                                                gf_test_frr_far_t *result,
                                                gf_test_frr_far_reserve_data_t *reserve_result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result || NULL == reserve_result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // image quality
    len += HAL_TEST_SIZEOF_INT32;
    // valid area
    len += HAL_TEST_SIZEOF_INT32;
    // bmpdata
    len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    // key point num
    len += HAL_TEST_SIZEOF_INT32;
    // enroll score
    len += HAL_TEST_SIZEOF_INT32;
    if ((result->data_type == TEST_TOKEN_RAW_DATA)
        || (result->data_type == TEST_TOKEN_PREPROCESS_RAW_DATA))
    {
        // broken mask
        len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            //  touch mask
            len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
        }
    }
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_IMAGE_QUALITY,
                                        result->image_quality);
        current = hal_test_encode_int32(current, TEST_TOKEN_VALID_AREA,
                                        result->valid_area);
        current = hal_test_encode_array(current, TEST_TOKEN_BMP_DATA,
                                        (uint8_t *) result->bmp_data,
                                        g_sensor_row * g_sensor_col);
        current = hal_test_encode_int32(current, TEST_TOKEN_KEY_POINT_NUM,
                                        result->key_point_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_SCORE,
                                        result->score);
        if ((result->data_type == TEST_TOKEN_RAW_DATA)
            || (result->data_type == TEST_TOKEN_PREPROCESS_RAW_DATA))
        {
            current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_BROKEN_MASK_DATA,
                                            (uint8_t *) result->broken_mask,
                                            g_sensor_row * g_sensor_col);
            if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
            {
                current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_TOUCH_MASK_DATA,
                                                (uint8_t *) reserve_result->touch_mask,
                                                g_sensor_row * g_sensor_col);
            }
        }
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_FRR_FAR_PLAY_ENROLL, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

static void hal_notify_test_frr_far_play_authenticate(gf_error_t err,
                                                      gf_test_frr_far_t *result,
                                                      gf_test_frr_far_reserve_data_t *reserve_result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result || NULL == reserve_result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // image quality
    len += HAL_TEST_SIZEOF_INT32;
    // valid area
    len += HAL_TEST_SIZEOF_INT32;

    if ((result->data_type == TEST_TOKEN_RAW_DATA)
        || (result->data_type == TEST_TOKEN_PREPROCESS_RAW_DATA))
    {
        // bmpdata
        len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
    }

    // preprocess time
    len += HAL_TEST_SIZEOF_INT32;
    // get feature time
    len += HAL_TEST_SIZEOF_INT32;
    // authenticate time
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t gsc_flag;
    len += HAL_TEST_SIZEOF_INT32;
    // key point num
    len += HAL_TEST_SIZEOF_INT32;
    // authenticate score
    len += HAL_TEST_SIZEOF_INT32;
    // update
    len += HAL_TEST_SIZEOF_INT32;
    // study replace index
    len += HAL_TEST_SIZEOF_INT32;
    // chche tmp num
    len += HAL_TEST_SIZEOF_INT32;
    // extra tmp num
    len += HAL_TEST_SIZEOF_INT32;
    if ((result->data_type == TEST_TOKEN_RAW_DATA)
        || (result->data_type == TEST_TOKEN_PREPROCESS_RAW_DATA))
    {
        // broken mask
        len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
        if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
        {
            //  touch mask
            len += HAL_TEST_SIZEOF_ARRAY(g_sensor_row * g_sensor_col);
        }
    }
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_IMAGE_QUALITY,
                                        result->image_quality);
        current = hal_test_encode_int32(current, TEST_TOKEN_VALID_AREA,
                                        result->valid_area);

        if ((result->data_type == TEST_TOKEN_RAW_DATA)
            || (result->data_type == TEST_TOKEN_PREPROCESS_RAW_DATA))
        {
            current = hal_test_encode_array(current, TEST_TOKEN_BMP_DATA,
                                            (uint8_t *) result->bmp_data, g_sensor_row * g_sensor_col);
        }

        current = hal_test_encode_int32(current, TEST_TOKEN_PREPROCESS_TIME,
                                        result->preprocess_time);
        current = hal_test_encode_int32(current, TEST_TOKEN_GET_FEATURE_TIME,
                                        result->get_feature_time);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATE_TIME,
                                        result->authenticate_time);
        current = hal_test_encode_int32(current, TEST_TOKEN_GSC_FLAG,
                                        result->gsc_flag);
        current = hal_test_encode_int32(current, TEST_TOKEN_KEY_POINT_NUM,
                                        result->key_point_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_SCORE, result->score);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATE_UPDATE_FLAG,
                                        result->update);
        current = hal_test_encode_int32(current, TEST_TOKEN_STUDY_REPLACE_INDEX,
                                        result->study_replase_index);
        current = hal_test_encode_int32(current, TEST_TOKEN_CACHE_TEMPLATE_NUM,
                                        result->cache_tmp_num);
        current = hal_test_encode_int32(current, TEST_TOKEN_EXTRA_TEMPLATE_NUM,
                                        result->extra_tmp_num);
        if ((result->data_type == TEST_TOKEN_RAW_DATA)
            || (result->data_type == TEST_TOKEN_PREPROCESS_RAW_DATA))
        {
            current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_BROKEN_MASK_DATA,
                                            (uint8_t *) result->broken_mask,
                                            g_sensor_row * g_sensor_col);
            if (GF_DUBAI_A_SERIES == g_hal_config.chip_series)
            {
                current = hal_test_encode_array(current, TEST_TOKEN_FRR_FAR_TOUCH_MASK_DATA,
                                                (uint8_t *) reserve_result->touch_mask,
                                                g_sensor_row * g_sensor_col);
            }
        }
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_finger_event(gf_error_t err, uint32_t finger_event)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // finger status
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_FINGER_EVENT, finger_event);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    LOG_D(LOG_TAG, "[%s] finger_event=%u", __func__, finger_event);
    hal_notify_test_cmd(CMD_TEST_CHECK_FINGER_EVENT, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_bio_hbd_calibration(gf_error_t err,
                                         gf_test_hbd_feature_t *hbd_feature,
                                         gf_cmd_test_id_t cmd_id)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == hbd_feature)
    {
        LOG_E(LOG_TAG, "[%s] hbd_feature is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // hbd base
    len += HAL_TEST_SIZEOF_INT16;
    // hdb_avg
    len += HAL_TEST_SIZEOF_INT16;
    // hbd buffer
    len += HAL_TEST_SIZEOF_ARRAY(hbd_feature->hbd_data_len);
    // electricity_value of LED0
    len += HAL_TEST_SIZEOF_INT32;
    // finger status
    // len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int16(current, TEST_TOKEN_HBD_BASE,
                                        hbd_feature->hbd_base);
        current = hal_test_encode_int16(current, TEST_TOKEN_HBD_AVG,
                                        hbd_feature->hbd_avg);
        current = hal_test_encode_array(current, TEST_TOKEN_HBD_RAW_DATA,
                                        hbd_feature->hdb_data,
                                        hbd_feature->hbd_data_len);
        current = hal_test_encode_int32(current, TEST_TOKEN_ELECTRICITY_VALUE,
                                        hbd_feature->electricity_value);
        /* current = hal_test_encode_int32(current, TEST_TOKEN_HBD_FINGER_STATUS,
         * hbd_feature->finger_status); */
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    LOG_D(LOG_TAG, "[%s] hbd_base=%u, hbd_avg=%u, electricity_value=%u",
          __func__, hbd_feature->hbd_base, hbd_feature->hbd_avg,
          hbd_feature->electricity_value);
    hal_notify_test_cmd(cmd_id, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

gf_error_t hal_notify_test_bad_point(gf_error_t err,
                                     gf_bad_point_test_result_t *result)
{
    FUNC_ENTER();

    do
    {
        if (NULL == result)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        switch (g_hal_config.chip_series)
        {
            case GF_OSWEGO_M:
            {
                hal_oswego_notify_test_bad_point(err,
                                                 (gf_bad_point_test_result_oswego_t *) result);
                break;
            }

            case GF_MILAN_F_SERIES:
            case GF_DUBAI_A_SERIES:
            case GF_MILAN_A_SERIES:
            case GF_MILAN_AN_SERIES:
            {
                /* FALL THROUGH */
            }

            case GF_MILAN_HV:
            {
                hal_milan_notify_test_bad_point(err,
                                                (gf_bad_point_test_result_milan_t *) result);
                break;
            }

            default:
            {
                err = GF_ERROR_UNSUPPORT_CHIP;
                break;
            }
        }
    }
    while (0);

    // Factory test about BAD_POINT
    if (NULL != g_fingerprint_device->factory_test_notify &&
        GF_ERROR_UNSUPPORT_CHIP != err)
    {
        g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_BAD_POINT,
                                                  err, (void *)result);
    }

    FUNC_EXIT(err);
    return err;
}

void hal_notify_test_rawdata_saturated(gf_error_t err,
                                       gf_test_rawdata_saturated_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    if (NULL == result)
    {
        LOG_E(LOG_TAG, "[%s] result is null", __func__);
        return;
    }

    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error code
    len += HAL_TEST_SIZEOF_INT32;
    // under_saturated_pixel_count
    len += HAL_TEST_SIZEOF_INT32;
    // over_saturated_pixel_count
    len += HAL_TEST_SIZEOF_INT32;
    // saturated_pixel_threshold
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_UNDER_SATURATED_PIXEL_COUNT,
                                        result->under_saturated_pixel_count);
        current = hal_test_encode_int32(current, TEST_TOKEN_OVER_SATURATED_PIXEL_COUNT,
                                        result->over_saturated_pixel_count);
        current = hal_test_encode_int32(current, TEST_TOKEN_SATURATED_PIXEL_THRESHOLD,
                                        result->saturated_pixel_threshold);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_RAWDATA_SATURATED, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_error(gf_error_t err, gf_cmd_test_id_t cmd_id)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();
    // chip type
    len += HAL_TEST_SIZEOF_INT32;
    // chip series
    len += HAL_TEST_SIZEOF_INT32;
    // error code
    len += HAL_TEST_SIZEOF_INT32;
    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (NULL != test_result)
    {
        uint8_t *current = test_result;
        memset(test_result, 0, len);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(cmd_id, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_test_spi_transfer_timer_thread(union sigval v)
{
    FUNC_ENTER();
    UNUSED_VAR(v);
    g_test_spi_transfer_timer_id = 0;
    hal_notify_test_spi_transfer(GF_ERROR_TEST_SPI_TRANSFER_TIMEOUT,
                                 g_test_spi_transfer_times);
    VOID_FUNC_EXIT();
}

void hal_irq_timer_thread(union sigval v)
{
    FUNC_ENTER();
    UNUSED_VAR(v);
    pthread_mutex_lock(&g_test_interrupt_pin_mutex);

    if (g_test_interrupt_pin_flag > 0)
    {
        hal_notify_test_interrupt_pin(GF_ERROR_TEST_INTERRUPT_PIN);
        g_test_interrupt_pin_flag = 0;
    }

    pthread_mutex_unlock(&g_test_interrupt_pin_mutex);
    gf_hal_destroy_timer(&g_irq_timer_id);
    VOID_FUNC_EXIT();
}

static gf_error_t hal_test_enumerate()
{
    gf_error_t err = GF_SUCCESS;
    gf_enumerate_t *cmd = NULL;
    uint32_t i = 0;
    FUNC_ENTER();

    do
    {
        uint32_t size = sizeof(gf_enumerate_t);
        cmd = (gf_enumerate_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_ENUMERATE, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        LOG_D(LOG_TAG, "[%s] size=%u", __func__, cmd->size);

        for (i = 0; i < g_hal_config.max_fingers_per_user; i++)
        {
            LOG_D(LOG_TAG, "[%s] group_id[%u]=%u, finger_id[%u]=%u", __func__, i,
                  cmd->group_ids[i],
                  i, cmd->finger_ids[i]);
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_bad_point()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        uint32_t size = sizeof(gf_cmd_header_t);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] cmd out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        if (GF_MILAN_AN_SERIES == g_hal_config.chip_series)
        {
            memset(cmd, 0, size);
            err = gf_hal_test_invoke_command(GF_CMD_TEST_BAD_POINT_PRE_GET_BASE, cmd, size);

            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] ca invoke command failed, errno=%d", __func__, err);
                break;
            }

            gf_hal_disable_irq();
            hal_milan_an_series_irq();
            gf_hal_enable_irq();
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_BAD_POINT, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_calibration_para_retest()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        uint32_t size = sizeof(gf_cmd_header_t);
        cmd = (gf_cmd_header_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] cmd out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_CALIBRATION_PARA_RETEST, cmd,
                                         size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_pixel_open()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (GF_MILAN_HV != g_hal_config.chip_series)
        {
            err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PIXEL_OPEN);

            if (err != GF_SUCCESS)
            {
                break;
            }
        }

        err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PIXEL_OPEN_STEP1);
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_pixel_short_streak()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (GF_MILAN_HV != g_hal_config.chip_series)
        {
            err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PIXEL_OPEN);

            if (err != GF_SUCCESS)
            {
                break;
            }
        }
        err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PIXEL_SHORT_STREAK);
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_performance(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PERFORMANCE);
    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_spi_performance(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_SPI_PERFORMANCE);
    FUNC_EXIT(err);
    return err;
}
gf_error_t hal_test_spi_transfer(const uint8_t *cmd_buf, uint8_t len)
{
    gf_error_t err = GF_SUCCESS;
    time_t timer_count = 0;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    memcpy(&g_test_spi_transfer_times, cmd_buf, len);
    timer_count = 2 + (time_t) g_test_spi_transfer_times / 10;
    LOG_D(LOG_TAG, "[%s] times=%u", __func__, g_test_spi_transfer_times);

    do
    {
        gf_hal_create_timer(&g_test_spi_transfer_timer_id,
                            hal_test_spi_transfer_timer_thread);
        gf_hal_set_timer(&g_test_spi_transfer_timer_id, 0, timer_count, 0);
        err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_SPI_TRANSFER);

        if (err != GF_SUCCESS)
        {
            gf_hal_destroy_timer(&g_test_spi_transfer_timer_id);
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_spi_rw(const uint8_t *cmd_buf, uint8_t cmd_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_spi_rw_t *cmd = NULL;
    uint32_t spi_rw_cmd = TEST_SPI_READ;
    uint32_t start_addr = 0;
    uint32_t rw_length = 0;
    uint8_t *rw_content = NULL;
    uint8_t *test_result = NULL;
    uint32_t encode_len = 0;
    uint32_t token = 0;
    const uint8_t *in_buf = cmd_buf;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        in_buf = hal_test_decode_uint32(&token, in_buf);

        switch (token)
        {
            case TEST_TOKEN_SPI_RW_CMD:
            {
                in_buf = hal_test_decode_uint32(&spi_rw_cmd, in_buf);
                break;
            }

            case TEST_TOKEN_SPI_RW_START_ADDR:
            {
                in_buf = hal_test_decode_uint32(&start_addr, in_buf);
                break;
            }

            case TEST_TOKEN_SPI_RW_LENGTH:
            {
                in_buf = hal_test_decode_uint32(&rw_length, in_buf);
                break;
            }

            case TEST_TOKEN_SPI_RW_CONTENT:
            {
                if (rw_content != NULL)
                {
                    GF_MEM_FREE(rw_content);
                }

                in_buf += 4;  // buf len
                rw_content = (uint8_t *) GF_MEM_MALLOC(rw_length);

                if (NULL != rw_content)
                {
                    memcpy(rw_content, in_buf, rw_length);
                    in_buf += rw_length;
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }
    while (in_buf < cmd_buf + cmd_len);

    LOG_D(LOG_TAG, "[%s] spi_rw_cmd=%u, start_addr=0x%X, rw_len=%u", __func__,
          spi_rw_cmd,
          start_addr, rw_length);

    do
    {
        uint32_t size = sizeof(gf_test_spi_rw_t);
        cmd = (gf_test_spi_rw_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_PRE_SPI, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        memset(cmd, 0, size);
        cmd->spi_cmd = spi_rw_cmd;
        cmd->start_addr = start_addr;
        cmd->rw_len = rw_length;
        memset(cmd->rw_content, 0, GF_MAX_SPI_RW_LEN);

        if (cmd->spi_cmd == TEST_SPI_WRITE && rw_content != NULL)
        {
            memcpy(cmd->rw_content, rw_content, rw_length);
        }

        err = gf_hal_test_invoke_command(GF_CMD_TEST_SPI_RW, cmd, size);
        LOG_D(LOG_TAG, "[%s] cmd=%u, start_addr=0x%X, len=%u", __func__, cmd->spi_cmd,
              cmd->start_addr, cmd->rw_len);
        /*
         LOG_D(LOG_TAG, "[%s] content:0x%x 0x%x 0x%x 0x%x", __func__,
         cmd->rw_content[0], cmd->rw_content[1], cmd->rw_content[2], cmd->rw_content[3]);
         */
        // error code
        encode_len += HAL_TEST_SIZEOF_INT32;
        // read or wirte cmd
        encode_len += HAL_TEST_SIZEOF_INT32;
        // start addr
        encode_len += HAL_TEST_SIZEOF_INT32;
        // read write len
        encode_len += HAL_TEST_SIZEOF_INT32;
        // read write content
        encode_len += HAL_TEST_SIZEOF_ARRAY(cmd->rw_len);
        test_result = (uint8_t *) GF_MEM_MALLOC(encode_len);

        if (NULL == test_result)
        {
            encode_len = 0;
            LOG_E(LOG_TAG, "[%s] test_result out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(test_result, 0, encode_len);
        {
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE,
                                            cmd->cmd_header.result);
            current = hal_test_encode_int32(current, TEST_TOKEN_SPI_RW_CMD, cmd->spi_cmd);
            current = hal_test_encode_int32(current, TEST_TOKEN_SPI_RW_START_ADDR,
                                            cmd->start_addr);
            current = hal_test_encode_int32(current, TEST_TOKEN_SPI_RW_LENGTH, cmd->rw_len);
            current = hal_test_encode_array(current, TEST_TOKEN_SPI_RW_CONTENT,
                                            cmd->rw_content,
                                            cmd->rw_len);
            hal_notify_test_memory_check(__func__, test_result, current, encode_len);
        }
        hal_notify_test_cmd(CMD_TEST_SPI_RW, test_result, encode_len);
    }
    while (0);

    if (NULL != rw_content)
    {
        GF_MEM_FREE(rw_content);
        rw_content = NULL;
    }

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_spi(void)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_spi_t *cmd = NULL;
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    uint32_t size = sizeof(gf_test_spi_t);
    uint32_t retry = 2;
    FUNC_ENTER();

    do
    {
        cmd = (gf_test_spi_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        do
        {
            memset(cmd, 0, size);
            err = gf_hal_test_invoke_command(GF_CMD_TEST_PRE_SPI, cmd, size);
            if (cmd->cmd_header.reset_flag)
            {
               gf_hal_test_invoke_command_ex(GF_CMD_TEST_RESET_CLEAR);
               continue;
            }
            break;
        } while(retry--);

        if (err != GF_SUCCESS)
        {
            break;
        }
        retry = 2;
        do
        {
            memset(cmd, 0, size);
            err = gf_hal_test_invoke_command(GF_CMD_TEST_SPI, cmd, size);
            if (cmd->cmd_header.reset_flag)
            {
               gf_hal_test_invoke_command_ex(GF_CMD_TEST_RESET_CLEAR);
               continue;
            }
            break;
        } while(retry--);
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // chip type
        len += HAL_TEST_SIZEOF_INT32;
        // chip series
        len += HAL_TEST_SIZEOF_INT32;

        if (GF_MILAN_F_SERIES == g_hal_config.chip_series ||
            GF_DUBAI_A_SERIES == g_hal_config.chip_series ||
            GF_MILAN_HV == g_hal_config.chip_series)
        {
            // uint8_t chip_id[GF_CHIP_ID_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(GF_CHIP_ID_LEN);
        }
        else
        {
            // uint8_t fw_version[FW_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(FW_VERSION_INFO_LEN);
            // uint8_t code_fw_version[FW_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(FW_VERSION_INFO_LEN);
        }

        // uint8_t sensor_otp_type;
        len += HAL_TEST_SIZEOF_INT8;
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == test_result)
        {
            len = 0;
            LOG_E(LOG_TAG, "[%s] test_result out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);

        if (GF_MILAN_F_SERIES == g_hal_config.chip_series ||
            GF_DUBAI_A_SERIES == g_hal_config.chip_series ||
            GF_MILAN_HV == g_hal_config.chip_series)
        {
            current = hal_test_encode_array(current, TEST_TOKEN_CHIP_ID, cmd->chip_id,
                                            GF_CHIP_ID_LEN);
            LOG_D(LOG_TAG, "[%s] chip_id=0x%02x%02x%02x%02x", __func__,
                  cmd->chip_id[3], cmd->chip_id[2], cmd->chip_id[1], cmd->chip_id[0]);
        }
        else
        {
            current = hal_test_encode_array(current, TEST_TOKEN_FW_VERSION, cmd->fw_version,
                                            FW_VERSION_INFO_LEN);
            LOG_D(LOG_TAG, "[%s] fw_version=%s", __func__, cmd->fw_version);
            current = hal_test_encode_array(current, TEST_TOKEN_CODE_FW_VERSION,
                                            cmd->code_fw_version, FW_VERSION_INFO_LEN);
        }

        current = hal_test_encode_int8(current, TEST_TOKEN_SENSOR_OTP_TYPE,
                                       cmd->sensor_otp_type);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    while (0);

    // Factory test for SPI_TEST
    if (NULL != g_fingerprint_device->factory_test_notify)
    {
        g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_SPI, err,
                                                  (void *)cmd);
    }

    hal_notify_test_cmd(CMD_TEST_SPI, test_result, len);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_get_version(char *buf)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_get_version_t *cmd = NULL;
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    uint32_t size = sizeof(gf_test_get_version_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_test_get_version_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_PRE_GET_VERSION, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_GET_VERSION, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
        memcpy(buf, cmd->algo_version, sizeof(cmd->algo_version));
        LOG_D(LOG_TAG, "[%s]       algo_version=%s", __func__, cmd->algo_version);
        LOG_D(LOG_TAG, "[%s] preprocess_version=%s", __func__, cmd->preprocess_version);
        LOG_D(LOG_TAG, "[%s]         fw_version=%s", __func__, cmd->fw_version);
        LOG_D(LOG_TAG, "[%s]        tee_version=%s", __func__, cmd->tee_version);
        LOG_D(LOG_TAG, "[%s]         ta_version=%s", __func__, cmd->ta_version);
        LOG_D(LOG_TAG, "[%s]            chip_id=0x%02X%02X%02X%02X", __func__,
              cmd->chip_id[0],
              cmd->chip_id[1], cmd->chip_id[2], cmd->chip_id[3]);
        LOG_D(LOG_TAG, "[%s]          vendor_id=0x%02X%02X%02X%02X", __func__,
              cmd->vendor_id[0],
              cmd->vendor_id[1], cmd->vendor_id[2], cmd->vendor_id[3]);
        LOG_D(LOG_TAG, "[%s] heart beat algo version   = %s", __func__,
              cmd->heart_beat_algo_version);
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // int8_t algo_version[ALGO_VERSION_INFO_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(ALGO_VERSION_INFO_LEN);
        // int8_t preprocess_version[ALGO_VERSION_INFO_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(ALGO_VERSION_INFO_LEN);
        // int8_t fw_version[FW_VERSION_INFO_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(FW_VERSION_INFO_LEN);
        // int8_t tee_version[TEE_VERSION_INFO_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(TEE_VERSION_INFO_LEN);
        // int8_t ta_version[TA_VERSION_INFO_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(TA_VERSION_INFO_LEN);
        // int8_t chip_id[GF_CHIP_ID_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(GF_CHIP_ID_LEN);
        // uint8_t vendor_id[GF_VENDOR_ID_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(GF_VENDOR_ID_LEN);
        // uint8_t sensor_id[GF_SENSOR_ID_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(GF_SENSOR_ID_LEN);
        // uint8_t production_date[PRODUCTION_DATE_LEN];
        len += HAL_TEST_SIZEOF_ARRAY(PRODUCTION_DATE_LEN);
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == test_result)
        {
            len = 0;
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_array(current, TEST_TOKEN_ALGO_VERSION,
                                        (uint8_t *) cmd->algo_version, ALGO_VERSION_INFO_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_PREPROCESS_VERSION,
                                        (uint8_t *) cmd->preprocess_version, ALGO_VERSION_INFO_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_FW_VERSION,
                                        (uint8_t *) cmd->fw_version,
                                        FW_VERSION_INFO_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_TEE_VERSION,
                                        (uint8_t *) cmd->tee_version, TEE_VERSION_INFO_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_TA_VERSION,
                                        (uint8_t *) cmd->ta_version,
                                        TA_VERSION_INFO_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_CHIP_ID, cmd->chip_id,
                                        GF_CHIP_ID_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_VENDOR_ID, cmd->vendor_id,
                                        GF_VENDOR_ID_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_SENSOR_ID, cmd->sensor_id,
                                        GF_SENSOR_ID_LEN);
        current = hal_test_encode_array(current, TEST_TOKEN_PRODUCTION_DATE,
                                        cmd->production_date,
                                        PRODUCTION_DATE_LEN);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    while (0);

    // Factory test about GET_VERSION
    if (NULL != g_fingerprint_device->factory_test_notify)
    {
        g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_GET_VERSION, err,
                                                  (void *)cmd);
    }

    hal_notify_test_cmd(CMD_TEST_GET_VERSION, test_result, len);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_get_chip_type()
{
    gf_error_t err = GF_SUCCESS;
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    FUNC_ENTER();

    do
    {
        // chip_type;
        len += HAL_TEST_SIZEOF_INT32;
        // chip_series
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == test_result)
        {
            len = 0;
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, GF_SUCCESS);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    while (0);

    hal_notify_test_cmd(CMD_TEST_FRR_FAR_GET_CHIP_TYPE, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_init(const uint8_t *cmd_buf, uint32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_frr_far_init_t *cmd = NULL;
    uint32_t safe_class = 0;
    uint32_t template_count = 0;
    uint32_t token = 0;
    const uint8_t *in_buf = cmd_buf;
    uint32_t size = sizeof(gf_test_frr_far_t);
    uint32_t support_bio_assay = 0;
    uint32_t forbidden_duplicate_finger = 0;
    uint32_t group = 0;
    uint32_t frr_far_para_flag = 0;
    uint32_t increase_rate = 0;
    uint32_t overlay = 0;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);

        do
        {
            in_buf = hal_test_decode_uint32(&token, in_buf);
            LOG_D(LOG_TAG, "token = %u", token);

            switch (token)
            {
                case TEST_TOKEN_SAFE_CLASS:
                {
                    in_buf = hal_test_decode_uint32(&safe_class, in_buf);
                    break;
                }

                case TEST_TOKEN_TEMPLATE_COUNT:
                {
                    in_buf = hal_test_decode_uint32(&template_count, in_buf);
                    break;
                }

                case TEST_TOKEN_SUPPORT_BIO_ASSAY:
                {
                    in_buf = hal_test_decode_uint32(&support_bio_assay, in_buf);
                    break;
                }

                case TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS:
                {
                    in_buf = hal_test_decode_uint32(&forbidden_duplicate_finger, in_buf);
                    break;
                }

                case TEST_TOKEN_FRR_FAR_GROUP_ID:
                {
                    in_buf = hal_test_decode_uint32(&group, in_buf);
                    break;
                }

                case TEST_TOKEN_FRR_FAR_PARA_FLAG:
                {
                    in_buf = hal_test_decode_uint32(&frr_far_para_flag, in_buf);
                    break;
                }

                case TEST_TOKEN_INCREATE_RATE:
                {
                    in_buf = hal_test_decode_uint32(&increase_rate, in_buf);
                    break;
                }

                case TEST_TOKEN_OVERLAY:
                {
                    in_buf = hal_test_decode_uint32(&overlay, in_buf);
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
        while (in_buf < cmd_buf + len);

        LOG_D(LOG_TAG, "[%s] group: %d, safe class = %u, template_count = %u,"
              "support_bio_assay: %u forbidden_duplicate_finger:%d",
              __func__, group, safe_class, template_count,
              support_bio_assay, forbidden_duplicate_finger);
        LOG_D(LOG_TAG, "[%s] frr_far_para_flag = %d, increase_rate thd = %u,"
              "overlay thd = %u,", __func__, frr_far_para_flag, increase_rate, overlay);
        cmd = (gf_test_frr_far_init_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        // means the chip don't support bio feature or it's not neccessary
        if (g_hal_config.support_bio_assay == 0)
        {
            support_bio_assay = 0;
        }

        memset(cmd, 0, size);
        cmd->safe_class = safe_class;
        cmd->template_count = template_count;
        cmd->support_bio_assay = support_bio_assay;
        cmd->forbidden_duplicate_finger = forbidden_duplicate_finger;
        cmd->finger_group_id = group;
        cmd->frr_far_para_flag = frr_far_para_flag;
        cmd->increase_rate = increase_rate;
        cmd->overlay = overlay;
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_INIT, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_record_calibration()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_RECORD_CALIBRATION, cmd,
                                         size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_record_enroll()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_RECORD_ENROLL, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_record_authenticate()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE, cmd,
                                         size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_record_authenticate_finish()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(
              GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_play_calibration(const uint8_t *buf,
                                                    uint32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_calibration_t *cmd = NULL;
    uint32_t size = sizeof(gf_test_calibration_t);
    uint32_t token = 0;
    uint32_t data_len = 0;
    const uint8_t *in_buf = buf;
    FUNC_ENTER();
    UNUSED_VAR(buf_len);

    if (NULL == buf)
    {
        LOG_E(LOG_TAG, "[%s] buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        cmd = (gf_test_calibration_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);

        do
        {
            in_buf = hal_test_decode_uint32(&token, in_buf);
            in_buf = hal_test_decode_uint32(&data_len, in_buf);

            switch (token)
            {
                case TEST_TOKEN_KR_DATA:
                {
                    memcpy(cmd->kr_data, in_buf, data_len);
                    break;
                }

                case TEST_TOKEN_B_DATA:
                {
                    memcpy(cmd->b_data, in_buf, data_len);
                    break;
                }

                default:
                {
                    break;
                }
            }

            in_buf += data_len;
        }
        while (in_buf < buf + buf_len);

        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_PLAY_CALIBRATION, cmd,
                                         size);
        hal_notify_test_frr_far_play_calibration(err, cmd);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_play_enroll(const uint8_t *cmd_buf,
                                               uint32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_frr_far_new_t *cmd = NULL;
    uint32_t token = 0;
    uint32_t data_len = 0;
    const uint8_t *in_buf = cmd_buf;
    uint32_t size = sizeof(gf_test_frr_far_new_t);
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        cmd = (gf_test_frr_far_new_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->frr_far.check_flag = 0;

        do
        {
            in_buf = hal_test_decode_uint32(&token, in_buf);

            switch (token)
            {
                case TEST_TOKEN_RAW_DATA:
                case TEST_TOKEN_PREPROCESS_RAW_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.raw_data, in_buf, data_len);
                    in_buf += data_len;
                    cmd->frr_far.data_type = token;
                    break;
                }

                case TEST_TOKEN_BMP_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.bmp_data, in_buf, data_len);
                    in_buf += data_len;
                    cmd->frr_far.data_type = token;
                    break;
                }

                case TEST_TOKEN_GSC_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.gsc_data, in_buf, data_len);
                    in_buf += data_len;
                    break;
                }

                case TEST_TOKEN_FRR_FAR_BROKEN_MASK_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.broken_mask, in_buf, data_len);
                    in_buf += data_len;
                    cmd->frr_far.have_broken_mask = 1;
                    break;
                }

                case TEST_TOKEN_FRR_FAR_TOUCH_MASK_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far_reserve.touch_mask, in_buf, data_len);
                    in_buf += data_len;
                    cmd->frr_far.have_broken_mask = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        while (in_buf < cmd_buf + len);

        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_PLAY_ENROLL, cmd, size);
        hal_notify_test_frr_far_play_enroll(err, &(cmd->frr_far), &(cmd->frr_far_reserve));
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_play_authenticate(const uint8_t *cmd_buf,
                                                     uint32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_frr_far_new_t *cmd = NULL;
    uint32_t data_type = 0;
    const uint8_t *in_buf = cmd_buf;
    uint32_t size = sizeof(gf_test_frr_far_new_t);
    uint32_t token = 0;
    uint32_t data_len = 0;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        cmd = (gf_test_frr_far_new_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);

        do
        {
            in_buf = hal_test_decode_uint32(&token, in_buf);

            switch (token)
            {
                case TEST_TOKEN_RAW_DATA:
                case TEST_TOKEN_PREPROCESS_RAW_DATA:
                {
                    data_type = token;
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.raw_data, in_buf, data_len);
                    in_buf += data_len;
                    break;
                }

                case TEST_TOKEN_BMP_DATA:
                {
                    data_type = token;
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.bmp_data, in_buf, data_len);
                    in_buf += data_len;
                    break;
                }

                case TEST_TOKEN_GSC_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.gsc_data, in_buf, data_len);
                    in_buf += data_len;
                    break;
                }

                case TEST_TOKEN_FRR_FAR_BROKEN_MASK_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far.broken_mask, in_buf, data_len);
                    in_buf += data_len;
                    cmd->frr_far.have_broken_mask = 1;
                    break;
                }

                case TEST_TOKEN_FRR_FAR_TOUCH_MASK_DATA:
                {
                    in_buf = hal_test_decode_uint32(&data_len, in_buf);
                    memcpy(cmd->frr_far_reserve.touch_mask, in_buf, data_len);
                    in_buf += data_len;
                    cmd->frr_far.have_broken_mask = 1;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        while (in_buf < cmd_buf + len);

        cmd->frr_far.data_type = data_type;
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE, cmd,
                                         size);
        hal_notify_test_frr_far_play_authenticate(err, &(cmd->frr_far), &(cmd->frr_far_reserve));
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_enroll_finish()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_FRR_FAR_ENROLL_FINISH);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_save_finger(const uint8_t *cmd_buf,
                                               uint8_t cmd_len)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t token = 0;
    uint32_t group_id = 0;
    uint32_t finger_id = 0;
    uint32_t length = 0;
    gf_test_frr_far_save_t *cmd = NULL;
    const uint8_t *in_buf = cmd_buf;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        uint32_t size = sizeof(gf_test_frr_far_save_t);
        cmd = (gf_test_frr_far_save_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        in_buf = hal_test_decode_uint32(&token, in_buf);

        if (token == TEST_TOKEN_FRR_FAR_GROUP_ID)
        {
            in_buf = hal_test_decode_uint32(&group_id, in_buf);
            cmd->group_id = group_id;
        }

        in_buf = hal_test_decode_uint32(&token, in_buf);

        if (token == TEST_TOKEN_FRR_FAR_FINGER_ID)
        {
            in_buf = hal_test_decode_uint32(&finger_id, in_buf);
            cmd->finger_id = finger_id;
        }

        in_buf = hal_test_decode_uint32(&token, in_buf);

        if (token == TEST_TOKEN_FRR_FAR_SAVE_FINGER_PATH)
        {
            in_buf = hal_test_decode_uint32(&length, in_buf);
            memcpy(cmd->finger_name, in_buf, length);
            in_buf += length;
        }

        LOG_D(LOG_TAG, "[%s] cmd_len=%u, length=%u, %s %u %u", __func__,
              cmd_len, length, cmd->finger_name, group_id, finger_id);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_SAVE_FINGER, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_del_finger(const uint8_t *cmd_buf,
                                              uint8_t cmd_len)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t token = 0;
    uint32_t group_id = 0;
    uint32_t finger_id = 0;
    uint32_t length = 0;
    gf_test_frr_far_remove_t *cmd = NULL;
    const uint8_t *in_buf = cmd_buf;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        uint32_t size = sizeof(gf_test_frr_far_save_t);
        cmd = (gf_test_frr_far_remove_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        in_buf = hal_test_decode_uint32(&token, in_buf);

        if (token == TEST_TOKEN_FRR_FAR_GROUP_ID)
        {
            in_buf = hal_test_decode_uint32(&group_id, in_buf);
            cmd->group_id = group_id;
        }

        in_buf = hal_test_decode_uint32(&token, in_buf);

        if (token == TEST_TOKEN_FRR_FAR_FINGER_ID)
        {
            in_buf = hal_test_decode_uint32(&finger_id, in_buf);
            cmd->finger_id = finger_id;
        }

        LOG_D(LOG_TAG, "[%s] cmd_len=%u, length=%u, %u %u", __func__,
              cmd_len, length, group_id, finger_id);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_DEL_FINGER, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_preprocess_init()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_FRR_FAR_PREPROCESS_INIT);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_frr_far_cancel()
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_cancel_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FRR_FAR_CANCEL, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_real_time_data()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_REAL_TIME_DATA, cmd, size);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] test real data failed. err = %d", __func__, err);
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_bmp_data()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_BMP_DATA, cmd, size);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] test bmp data failed. err = %d", __func__, err);
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_cancel()
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
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

static void gf_hal_test_stable_stop_test()
{
    VOID_FUNC_ENTER();

    do
    {
        hal_notify_test_stable_factor(-1, 0.0f);
    }
    while (0);

    gf_hal_destroy_timer(&g_stable_test_timeout_timer_id);
    hal_test_cancel();

    VOID_FUNC_EXIT();
}

static void gf_hal_test_twill_badpoint_stop_test()
{
    twill_badpoint_result_t *cmd = NULL;
    uint32_t size = sizeof(twill_badpoint_result_t);
    VOID_FUNC_ENTER();

    do
    {
        cmd = (twill_badpoint_result_t *)GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            break;
        }

        memset(cmd, 0, size);
        hal_notify_test_twill_badpoint(-1, cmd);
    }
    while (0);

    hal_test_cancel();

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    VOID_FUNC_EXIT();
}

static void gf_hal_test_snr_stop_test()
{
    snr_result_t *cmd = NULL;
    uint32_t size = sizeof(snr_result_t);
    VOID_FUNC_ENTER();

    do
    {
        cmd = (snr_result_t *)GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            break;
        }

        memset(cmd, 0, size);
        hal_notify_test_data_noise(-1, cmd);
    }
    while (0);

    hal_test_cancel();

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    VOID_FUNC_EXIT();
}

static void gf_test_stable_timer_thread(union sigval v)
{
    VOID_FUNC_ENTER();
    gf_hal_test_stable_stop_test();
    VOID_FUNC_EXIT();
}

static gf_error_t hal_test_twill_badpoint()
{
    gf_error_t err = GF_SUCCESS;
    twill_badpoint_result_t *notify_result = NULL;
    uint32_t size_notify = sizeof(twill_badpoint_result_t);
    FUNC_ENTER();

    do
    {
        notify_result = (twill_badpoint_result_t *) GF_MEM_MALLOC(size_notify);
        if (NULL == notify_result)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(notify_result, 0, size_notify);
        notify_result->algo_result = 1;
        notify_result->algo_badpoint_total_result = -1;
        notify_result->algo_badpoint_local_result = -1;
        notify_result->algo_badpoint_numlocal_result = -1;
        notify_result->algo_badpoint_line_result = -1;
        err = gf_hal_test_invoke_command(GF_CMD_TEST_TWILL_BADPOINT_GET_RESULT,
                    notify_result, size_notify);
        if (GF_SUCCESS == err)
        {
            LOG_D(LOG_TAG, "[%s] get result success", __func__);
        }
        else
        {
            LOG_E(LOG_TAG, "[%s] get result failed errno =%d", __func__, err);
            gf_hal_test_twill_badpoint_stop_test();
            break;
        }

        hal_notify_test_twill_badpoint((uint32_t)notify_result->algo_result, notify_result);
    }
    while (0);

    if (notify_result != NULL)
    {
        GF_MEM_FREE(notify_result);
        notify_result = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_data_noise()
{
    gf_error_t err = GF_SUCCESS;
    snr_result_t *snr_result = NULL;
    uint32_t size_snr_out = sizeof(snr_result_t);
    FUNC_ENTER();

    do
    {
        snr_result = (snr_result_t *) GF_MEM_MALLOC(size_snr_out);
        if (NULL == snr_result)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(snr_result, 0, size_snr_out);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_SNR_GET_RESULT, snr_result,
                    size_snr_out);
        if (GF_SUCCESS == err)
        {
            LOG_D(LOG_TAG, "[%s] get result success", __func__);
        }
        else
        {
            LOG_E(LOG_TAG, "[%s] get result failed errno =%d", __func__, err);
            gf_hal_test_snr_stop_test();
            break;
        }
        hal_notify_test_data_noise((uint32_t)snr_result->algo_result, snr_result);
    }
    while (0);

    if (snr_result != NULL)
    {
        GF_MEM_FREE(snr_result);
        snr_result = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_reset_pin()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        err = gf_hal_disable_irq();

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] disable_irq failed, err=%s, errno=%d", __func__,
                  gf_strerror(err), err);
        }

        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_RESET_PIN1, cmd, size);

        // because cmd->reset_flag > 0 reset in gf_hal_test_invoke_command()
        if (0 == cmd->reset_flag)
        {
            pthread_mutex_lock(&g_sensor_mutex);
            gf_hal_reset_chip();
            pthread_mutex_unlock(&g_sensor_mutex);
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_RESET_PIN2, cmd, size);

        if (GF_SUCCESS != gf_hal_enable_irq())
        {
            LOG_E(LOG_TAG, "[%s] enable_irq failed, err=%s, errno=%d", __func__,
                  gf_strerror(err), err);
        }

        // Factory test about RESET_PIN
        if (NULL != g_fingerprint_device->factory_test_notify)
        {
            g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_RESET_PIN, err,
                                                      NULL);
        }

        hal_notify_test_reset_pin(err);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_sensor_fine()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_SENSOR_FINE);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_interrupt_pin()
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    FUNC_ENTER();

    do
    {
        uint32_t size = sizeof(gf_cmd_header_t);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_INTERRUPT_PIN, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] test interrupt pin failed. err = %d", __func__, err);
            hal_notify_test_interrupt_pin(GF_ERROR_TEST_INTERRUPT_PIN);
            break;
        }

        g_test_interrupt_pin_flag = 1;
        gf_hal_create_timer(&g_irq_timer_id, hal_irq_timer_thread);
        gf_hal_set_timer(&g_irq_timer_id, 1, 1, 0);
    }
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_check_finger_touch_event()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_CHECK_FINGER_EVENT);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_bio_calibration()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_BIO_CALIBRATION);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_hbd_calibration()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_HBD_CALIBRATION);
    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_notify_test_calibration_para_retest(gf_error_t err,
                                                   gf_test_calibration_para_t *result)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == result)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "calibration_para_retest          avg_base_value=%u",
              result->avg_base_value);
        LOG_D(LOG_TAG, "calibration_para_retest          avg_touch_value=%u",
              result->avg_touch_value);
        LOG_D(LOG_TAG, "calibration_para_retest          min_touch_value=%u",
              result->min_touch_value);
        LOG_D(LOG_TAG, "calibration_para_retest          result=%u", result->result);
        // chip type
        len += HAL_TEST_SIZEOF_INT32;
        // chip series
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // result->avg_base_value
        len += HAL_TEST_SIZEOF_INT32;
        // result->avg_touch_value
        len += HAL_TEST_SIZEOF_INT32;
        // result->min_touch_value
        len += HAL_TEST_SIZEOF_INT32;
        // result->result
        len += HAL_TEST_SIZEOF_INT32;
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (test_result != NULL)
        {
            memset(test_result, 0, len);
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                            g_hal_config.chip_type);
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                            g_hal_config.chip_series);
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
            current = hal_test_encode_int32(current, TEST_TOKEN_AVG_BASE_RAWDATA,
                                            result->avg_base_value);
            current = hal_test_encode_int32(current, TEST_TOKEN_AVG_TOUCH_RAWDATA,
                                            result->avg_touch_value);
            current = hal_test_encode_int32(current, TEST_TOKEN_MIN_TOUCH_RAWDATA,
                                            result->min_touch_value);
            current = hal_test_encode_int32(current,
                                            TEST_TOKEN_CALIBRATION_PARA_RETEST_RESULT, result->result);
        }
        else
        {
            len = 0;
        }

        hal_notify_test_cmd(CMD_TEST_CALIBRATION_PARA_RETEST, test_result, len);
        hal_test_cancel();
    }  // do...

    while (0);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_prior_cancel()
{
    gf_error_t err = GF_SUCCESS;
    gf_cancel_t *cmd = NULL;
    uint32_t size = sizeof(gf_cancel_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_cancel_t *) malloc(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_PRIOR_CANCEL, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        free(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_get_config()
{
    gf_error_t err = GF_SUCCESS;
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    FUNC_ENTER();
    // gf_chip_type_t chip_type;
    len += HAL_TEST_SIZEOF_INT32;
    // gf_chip_series_t chip_series;
    len += HAL_TEST_SIZEOF_INT32;
    // error_code
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t max_fingers;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t max_fingers_per_user;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t support_key_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t support_ff_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t support_power_key_feature;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t forbidden_untrusted_enroll;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t forbidden_enroll_duplicate_fingers;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t support_bio_assay;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t support_performance_dump;
    len += HAL_TEST_SIZEOF_INT32;
    // gf_nav_mode_t support_nav_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t nav_double_click_interval_in_ms;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t nav_long_press_interval_in_ms;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t enrolling_min_templates;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t valid_image_quality_threshold;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t valid_image_area_threshold;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t duplicate_finger_overlay_score;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t increase_rate_between_stitch_info;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t screen_on_authenticate_fail_retry_count;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t screen_off_authenticate_fail_retry_count;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t screen_on_valid_touch_frame_threshold;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t screen_off_valid_touch_frame_threshold;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t image_quality_threshold_for_mistake_touch;
    len += HAL_TEST_SIZEOF_INT32;
    // gf_authenticate_order_t authenticate_order;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t reissue_key_down_when_entry_ff_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t reissue_key_down_when_entry_image_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint8_t support_sensor_broken_check;
    len += HAL_TEST_SIZEOF_INT32;
    // uint16_t broken_pixel_threshold_for_disable_sensor;
    len += HAL_TEST_SIZEOF_INT32;
    // uint16_t broken_pixel_threshold_for_disable_study;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t bad_point_test_max_frame_number;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t report_key_event_only_enroll_authenticate;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t require_down_and_up_in_pairs_for_image_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t require_down_and_up_in_pairs_for_ff_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t require_down_and_up_in_pairs_for_key_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t require_down_and_up_in_pairs_for_nav_mode;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t support_set_spi_speed_in_tee;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t support_frr_analysis;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t template_update_save_threshold;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t support_swipe_enroll;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t total_bad_point;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t local_bad_point;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t local_worst_bad_point;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t support_expand_image;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t expand_row;
    len += HAL_TEST_SIZEOF_INT32;
    // uint32_t expand_col;
    len += HAL_TEST_SIZEOF_INT32;

    if (GF_NAV_MODE_NONE == g_hal_config.support_nav_mode)
    {
        g_hal_config.nav_double_click_interval_in_ms = 0;
        g_hal_config.nav_long_press_interval_in_ms = 0;
    }
    else
    {
        gf_hal_nav_assert_config_interval();
    }

    test_result = (uint8_t *) GF_MEM_MALLOC(len);

    if (test_result != NULL)
    {
        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, TEST_TOKEN_MAX_FINGERS,
                                        g_hal_config.max_fingers);
        current = hal_test_encode_int32(current, TEST_TOKEN_MAX_FINGERS_PER_USER,
                                        g_hal_config.max_fingers_per_user);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_KEY_MODE,
                                        (int32_t) g_hal_config.support_key_mode);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_FF_MODE,
                                        (int32_t) g_hal_config.support_ff_mode);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_POWER_KEY_FEATURE,
                                        (int32_t) g_hal_config.support_power_key_feature);
        current = hal_test_encode_int32(current, TEST_TOKEN_FORBIDDEN_UNTRUSTED_ENROLL,
                                        (int32_t) g_hal_config.forbidden_untrusted_enroll);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS,
                                        (int32_t) g_hal_config.forbidden_enroll_duplicate_fingers);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_BIO_ASSAY,
                                        (int32_t) g_hal_config.support_bio_assay);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_PERFORMANCE_DUMP,
                                        (int32_t) g_hal_config.support_performance_dump);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_NAV_MODE,
                                        g_hal_config.support_nav_mode);
        current = hal_test_encode_int32(current, TEST_TOKEN_NAV_DOUBLE_CLICK_TIME,
                                        g_hal_config.nav_double_click_interval_in_ms);
        current = hal_test_encode_int32(current, TEST_TOKEN_NAV_LONG_PRESS_TIME,
                                        g_hal_config.nav_long_press_interval_in_ms);
        current = hal_test_encode_int32(current, TEST_TOKEN_ENROLLING_MIN_TEMPLATES,
                                        g_hal_config.enrolling_min_templates);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD,
                                        g_hal_config.valid_image_quality_threshold);
        current = hal_test_encode_int32(current, TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD,
                                        g_hal_config.valid_image_area_threshold);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE,
                                        g_hal_config.duplicate_finger_overlay_score);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO,
                                        g_hal_config.increase_rate_between_stitch_info);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT,
                                        g_hal_config.screen_on_authenticate_fail_retry_count);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT,
                                        g_hal_config.screen_off_authenticate_fail_retry_count);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SCREEN_ON_VALID_TOUCH_FRAME_THRESHOLD,
                                        g_hal_config.screen_on_valid_touch_frame_threshold);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SCREEN_OFF_VALID_TOUCH_FRAME_THRESHOLD,
                                        g_hal_config.screen_off_valid_touch_frame_threshold);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_IMAGE_QUALITY_THRESHOLD_FOR_MISTAKE_TOUCH,
                                        g_hal_config.image_quality_threshold_for_mistake_touch);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATE_ORDER,
                                        g_hal_config.authenticate_order);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE,
                                        g_hal_config.reissue_key_down_when_entry_ff_mode);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE,
                                        g_hal_config.reissue_key_down_when_entry_image_mode);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_SENSOR_BROKEN_CHECK,
                                        (int32_t) g_hal_config.support_sensor_broken_check);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_SENSOR,
                                        (int32_t) g_hal_config.broken_pixel_threshold_for_disable_sensor);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_STUDY,
                                        (int32_t) g_hal_config.broken_pixel_threshold_for_disable_study);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_BAD_POINT_TEST_MAX_FRAME_NUMBER,
                                        g_hal_config.bad_point_test_max_frame_number);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_REPORT_KEY_EVENT_ONLY_ENROLL_AUTHENTICATE,
                                        g_hal_config.report_key_event_only_enroll_authenticate);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_IMAGE_MODE,
                                        g_hal_config.require_down_and_up_in_pairs_for_image_mode);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_FF_MODE,
                                        g_hal_config.require_down_and_up_in_pairs_for_ff_mode);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_KEY_MODE,
                                        g_hal_config.require_down_and_up_in_pairs_for_key_mode);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_NAV_MODE,
                                        g_hal_config.require_down_and_up_in_pairs_for_nav_mode);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SUPPORT_SET_SPI_SPEED_IN_TEE,
                                        g_hal_config.support_set_spi_speed_in_tee);
        current = hal_test_encode_int32(current, TEST_TOKEN_SUPPORT_FRR_ANALYSIS,
                                        g_hal_config.support_frr_analysis);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_TEMPLATE_UPDATE_SAVE_THRESHOLD,
                                        g_hal_config.template_update_save_threshold);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SUPPORT_SWIPE_ENROLL,
                                        g_hal_config.support_swipe_enroll);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_BAD_POINT_TOTAL_GRADIENT,
                                        g_hal_config.total_bad_point);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_BAD_POINT_LOCAL_GRADIENT,
                                        g_hal_config.local_bad_point);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_BAD_POINT_LOCAL_WORST_GRADIENT,
                                        g_hal_config.local_worst_bad_point);
        hal_notify_test_memory_check(__func__, test_result, current, len);
    }
    else
    {
        len = 0;
    }

    hal_notify_test_cmd(CMD_TEST_GET_CONFIG, test_result, len);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    FUNC_EXIT(err);
    return err;
}

static void hal_test_share_memory_performance(uint32_t len)
{
    uint8_t *buffer = NULL;
    uint32_t retry_count = 1;
    uint32_t i = 0;
    VOID_FUNC_ENTER();

    do
    {
        int64_t start_time = 0;
        int64_t cost_time = 0;

        if (0 == len)
        {
            break;
        }

        buffer = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == buffer)
        {
            LOG_E(LOG_TAG, "[%s] out of memory", __func__);
            break;
        }

        start_time = gf_hal_current_time_microsecond();

        for (; i < retry_count; i++)
        {
            gf_error_t err = GF_SUCCESS;
            err = g_hal_function.user_invoke_command(
                      GF_USER_CMD_TEST_SHARE_MEMORY_PERFORMANCE, buffer, len);

            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG,
                      "[%s] test share memory performance fail, i=%u, err=%s, errno=%d",
                      __func__, i, gf_strerror(err), err);
            }
        }

        cost_time = gf_hal_current_time_microsecond() - start_time;
        LOG_D(LOG_TAG, "[%s] share %8d bytes, time=%lldms", __func__, len,
              (long long int)(cost_time / 1000 / retry_count));
    }
    while (0);

    if (NULL != buffer)
    {
        GF_MEM_FREE(buffer);
    }

    VOID_FUNC_EXIT();
}

static void hal_test_share_memory_check(void)
{
    uint8_t *buf = NULL;
    uint32_t i = 0;
    VOID_FUNC_ENTER();

    do
    {
        buf = (uint8_t *) GF_MEM_MALLOC(TEST_SHARE_MEMORY_CHECK_LEN + 6);

        if (NULL == buf)
        {
            LOG_E(LOG_TAG, "[%s] out of memory", __func__);
            break;
        }

        memset(buf, 0xAA, TEST_SHARE_MEMORY_CHECK_LEN + 6);
        g_hal_function.user_invoke_command(GF_USER_CMD_TEST_SHARE_MEMORY_CHECK, buf,
                                           TEST_SHARE_MEMORY_CHECK_LEN);

        for (; i < TEST_SHARE_MEMORY_CHECK_LEN; i++)
        {
            if (buf[i] != 0x55)
            {
                LOG_E(LOG_TAG, "[%s] ta to hal share memory failed", __func__);
                break;
            }
        }

        for (; i < TEST_SHARE_MEMORY_CHECK_LEN + 6; i++)
        {
            if (buf[i] != 0xAA)
            {
                LOG_E(LOG_TAG, "[%s] ta to hal share memory out of bounds, buf[%d] = 0x%02X",
                      __func__, i, buf[i]);
            }
        }

        LOG_I(LOG_TAG, "[%s] ta to hal share memory succeed", __func__);
    }
    while (0);

    if (NULL != buf)
    {
        GF_MEM_FREE(buf);
    }

    VOID_FUNC_EXIT();
}

static gf_error_t hal_test_set_config(const uint8_t *buf, uint32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_set_config_t *cmd = NULL;
    uint32_t token = 0;
    uint32_t value = 0;
    FUNC_ENTER();

    do
    {
        if (buf_len < 8 || buf == NULL)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, buf_len=%u", __func__, buf_len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        const uint8_t *in_buf = buf;
        in_buf = hal_test_decode_uint32(&token, in_buf);
        hal_test_decode_uint32(&value, in_buf);
        LOG_D(LOG_TAG, "[%s], token=%u, value=%u", __func__, token, value);
        uint32_t size = sizeof(gf_test_set_config_t);
        cmd = (gf_test_set_config_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->token = token;
        memcpy(&cmd->config, &g_hal_config, sizeof(gf_config_t));

        switch (token)
        {
            case TEST_TOKEN_MAX_FINGERS:
            {
                cmd->config.max_fingers = value;
                break;
            }

            case TEST_TOKEN_MAX_FINGERS_PER_USER:
            {
                cmd->config.max_fingers_per_user = value;
                break;
            }

            case TEST_TOKEN_SUPPORT_KEY_MODE:
            {
                cmd->config.support_key_mode = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_SUPPORT_FF_MODE:
            {
                cmd->config.support_ff_mode = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_SUPPORT_POWER_KEY_FEATURE:
            {
                cmd->config.support_power_key_feature = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_FORBIDDEN_UNTRUSTED_ENROLL:
            {
                cmd->config.forbidden_untrusted_enroll = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS:
            {
                cmd->config.forbidden_enroll_duplicate_fingers = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_SUPPORT_BIO_ASSAY:
            {
                cmd->config.support_bio_assay = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_SUPPORT_PERFORMANCE_DUMP:
            {
                cmd->config.support_performance_dump = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_SUPPORT_NAV_MODE:
            {
                cmd->config.support_nav_mode = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_NAV_DOUBLE_CLICK_TIME:
            {
                cmd->config.nav_double_click_interval_in_ms = value;
                break;
            }

            case TEST_TOKEN_NAV_LONG_PRESS_TIME:
            {
                cmd->config.nav_long_press_interval_in_ms = value;
                break;
            }

            case TEST_TOKEN_ENROLLING_MIN_TEMPLATES:
            {
                cmd->config.enrolling_min_templates = value;
                break;
            }

            case TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD:
            {
                cmd->config.valid_image_quality_threshold = value;
                break;
            }

            case TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD:
            {
                cmd->config.valid_image_area_threshold = value;
                break;
            }

            case TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE:
            {
                cmd->config.duplicate_finger_overlay_score = value;
                break;
            }

            case TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO:
            {
                cmd->config.increase_rate_between_stitch_info = value;
                break;
            }

            case TEST_TOKEN_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT:
            {
                cmd->config.screen_on_authenticate_fail_retry_count = value;
                break;
            }

            case TEST_TOKEN_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT:
            {
                cmd->config.screen_off_authenticate_fail_retry_count = value;
                break;
            }

            case TEST_TOKEN_SCREEN_ON_VALID_TOUCH_FRAME_THRESHOLD:
            {
                cmd->config.screen_on_valid_touch_frame_threshold = value;
                break;
            }

            case TEST_TOKEN_SCREEN_OFF_VALID_TOUCH_FRAME_THRESHOLD:
            {
                cmd->config.screen_off_valid_touch_frame_threshold = value;
                break;
            }

            case TEST_TOKEN_IMAGE_QUALITY_THRESHOLD_FOR_MISTAKE_TOUCH:
            {
                cmd->config.image_quality_threshold_for_mistake_touch = value;
                break;
            }

            case TEST_TOKEN_AUTHENTICATE_ORDER:
            {
                cmd->config.authenticate_order = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE:
            {
                cmd->config.reissue_key_down_when_entry_ff_mode = value;
                break;
            }

            case TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE:
            {
                cmd->config.reissue_key_down_when_entry_image_mode = value;
                break;
            }

            case TEST_TOKEN_SUPPORT_SENSOR_BROKEN_CHECK:
            {
                cmd->config.support_sensor_broken_check = (uint8_t) value;
                break;
            }

            case TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_SENSOR:
            {
                cmd->config.broken_pixel_threshold_for_disable_sensor = (uint16_t) value;
                break;
            }

            case TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_STUDY:
            {
                cmd->config.broken_pixel_threshold_for_disable_study = (uint16_t) value;
                break;
            }

            case TEST_TOKEN_BAD_POINT_TEST_MAX_FRAME_NUMBER:
            {
                cmd->config.bad_point_test_max_frame_number = value;
                break;
            }

            case TEST_TOKEN_REPORT_KEY_EVENT_ONLY_ENROLL_AUTHENTICATE:
            {
                cmd->config.report_key_event_only_enroll_authenticate = value;
                break;
            }

            case TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_IMAGE_MODE:
            {
                cmd->config.require_down_and_up_in_pairs_for_image_mode = value;
                break;
            }

            case TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_FF_MODE:
            {
                cmd->config.require_down_and_up_in_pairs_for_ff_mode = value;
                break;
            }

            case TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_KEY_MODE:
            {
                cmd->config.require_down_and_up_in_pairs_for_key_mode = value;
                break;
            }

            case TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_NAV_MODE:
            {
                cmd->config.require_down_and_up_in_pairs_for_nav_mode = value;
                break;
            }

            case TEST_TOKEN_SUPPORT_SET_SPI_SPEED_IN_TEE:
            {
                cmd->config.support_set_spi_speed_in_tee = value;
                break;
            }

            case TEST_TOKEN_SUPPORT_FRR_ANALYSIS:
            {
                cmd->config.support_frr_analysis = value;
                break;
            }

            case TEST_TOKEN_TEMPLATE_UPDATE_SAVE_THRESHOLD:
            {
                cmd->config.template_update_save_threshold = value;
                break;
            }

            case TEST_TOKEN_SUPPORT_SWIPE_ENROLL:
            {
                cmd->config.support_swipe_enroll= value;
                break;
            }

            case TEST_TOKEN_BAD_POINT_TOTAL_GRADIENT:
            {
                cmd->config.total_bad_point= value;
                break;
            }

            case TEST_TOKEN_BAD_POINT_LOCAL_GRADIENT:
            {
                cmd->config.local_bad_point= value;
                break;
            }

            case TEST_TOKEN_BAD_POINT_LOCAL_WORST_GRADIENT:
            {
                cmd->config.local_worst_bad_point= value;
                break;
            }

            default:
            {
                err = GF_ERROR_UNKNOWN_TEST_TOKEN;
                LOG_E(LOG_TAG, "[%s] invalid key value detected. err=%s, errno=%d", __func__,
                      gf_strerror(err), err);
                break;
            }
        }

        if (err != GF_SUCCESS)
        {
            break;
        }

        err = gf_hal_test_invoke_command(GF_CMD_TEST_SET_CONFIG, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        memcpy(&g_hal_config, &cmd->config, sizeof(gf_config_t));

        if (TEST_TOKEN_SUPPORT_NAV_MODE == token)
        {
            if (value > 0)
            {
                g_hal_function.navigate(g_fingerprint_device, (gf_nav_mode_t) value);
            }
            else
            {
                g_hal_function.test_cancel(g_fingerprint_device);
                g_hal_config.nav_double_click_interval_in_ms = 0;
                g_hal_config.nav_long_press_interval_in_ms = 0;
            }
        }

        uint32_t len = 0;
        // chip type
        len += HAL_TEST_SIZEOF_INT32;
        // chip series3909:17: err
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // changed config
        len += HAL_TEST_SIZEOF_INT32;
        uint8_t *result = (uint8_t *) GF_MEM_MALLOC(len);

        if (result == NULL)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(result, 0, len);
        uint8_t *current = result;
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                        g_hal_config.chip_type);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SERIES,
                                        g_hal_config.chip_series);
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        current = hal_test_encode_int32(current, token, value);
        hal_notify_test_memory_check(__func__, result, current, len);
        hal_notify_test_cmd(CMD_TEST_SET_CONFIG, result, len);
        GF_MEM_FREE(result);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_driver_cmd(const uint8_t *cmd_buf, uint32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_driver_cmd_t *cmd = NULL;
    FUNC_ENTER();

    do
    {
        if (NULL == cmd_buf || len == 0)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "device_enable",
                         strlen("device_enable")))
        {
            err = gf_hal_device_enable();
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "device_disable",
                         strlen("device_disable")))
        {
            err = gf_hal_device_disable();
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "spi_clock_enable",
                         strlen("spi_clock_enable")))
        {
            err = gf_hal_control_spi_clock(1);
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "spi_clock_disable",
                         strlen("spi_clock_disable")))
        {
            err = gf_hal_control_spi_clock(0);
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "hal_crash", strlen("hal_crash")))
        {
            uint32_t *addr = NULL;
            // cppcheck-suppress nullPointer
            *addr = 0x1234;
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "close_session",
                         strlen("close_session")))
        {
            gf_ca_close_session();
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "download_fw", strlen("download_fw")))
        {
            err = gf_hal_download_fw();

            if (err != GF_SUCCESS)
            {
                break;
            }

            err = gf_hal_download_cfg();
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "download_cfg",
                         strlen("download_cfg")))
        {
            err = gf_hal_download_cfg();
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "share_memory_performance",
                         strlen("share_memory_performance")))
        {
            hal_test_share_memory_performance(4);
            hal_test_share_memory_performance(32);
            hal_test_share_memory_performance(1024);
            hal_test_share_memory_performance(10240);
            hal_test_share_memory_performance(102400);
            hal_test_share_memory_performance(1024 * 1024);
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "share_memory_check",
                         strlen("share_memory_check")))
        {
            hal_test_share_memory_check();
            break;
        }

        uint32_t size = sizeof(gf_test_driver_cmd_t);
        cmd = (gf_test_driver_cmd_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        memcpy(cmd->cmd_buf, cmd_buf, len < 64 ? len : 64);

        if (0 == strncmp((const char *)cmd_buf, "set_mode", strlen("set_mode")))
        {
            uint32_t mode = atoi((char *) &cmd->cmd_buf[strlen("set_mode") + 1]);
            cmd->mode = mode;
            LOG_D(LOG_TAG, "[%s] set_mode mode=%s, mode_code=%d", __func__,
                  gf_strmode(mode), mode);
        }
        else if (0 == strncmp((const char *)cmd_buf, "write_product_info",
                              strlen("write_product_info")))
        {
            uint32_t product_cfg_idx = atoi((char *)
                                            &cmd->cmd_buf[strlen("write_product_info") + 1]);
            cmd->product_cfg_idx = product_cfg_idx;
            LOG_D(LOG_TAG, "[%s] product_cfg_idx=%u", __func__, product_cfg_idx);
        }
        else if (0 == strncmp((const char *)cmd_buf, "esd_exception",
                              strlen("esd_exception")))
        {
            uint32_t esd_exception_count = atoi((char *)
                                                &cmd->cmd_buf[strlen("esd_exception") + 1]);
            cmd->esd_exception_count = esd_exception_count;
            LOG_D(LOG_TAG, "[%s] esd_exception_count=%u", __func__, esd_exception_count);
        }
        else if (0 == strncmp((const char *)cmd_buf, "write", strlen("write")))
        {
            LOG_D(LOG_TAG, "[%s] drvier write", __func__);
            uint16_t address = 0;
            uint8_t value = 0;
            char temp[4] = { 0 };

            if (len - strlen("write") - 1 < 4 + 2)
            {
                LOG_E(LOG_TAG, "[%s] invalid address or value ", __func__);
                err = GF_ERROR_INVALID_DATA;
                break;
            }

            memcpy(temp, &cmd_buf[strlen("write") + 1], 4);
            address = (uint16_t) strtol(temp, NULL, 16);
            value = (uint8_t) strtol((const char *)&cmd_buf[strlen("write") + 1 + 4], NULL,
                                     16);
            LOG_D(LOG_TAG, "[%s] write address=0x%04X, val=0x%02X", __func__, address,
                  value);
            cmd->address = address;
            cmd->value = value;
        }
        else if (0 == strncmp((const char *)cmd_buf, "read", strlen("read")))
        {
            LOG_D(LOG_TAG, "[%s] drvier read", __func__);
            uint16_t address = 0;
            char temp[4] = { 0 };

            if (len - strlen("read") - 1 < 4)
            {
                LOG_E(LOG_TAG, "[%s] invalid address ", __func__);
                err = GF_ERROR_INVALID_DATA;
                break;
            }

            memcpy(temp, &cmd_buf[strlen("read") + 1], 4);
            address = (uint16_t) strtol(temp, NULL, 16);
            LOG_D(LOG_TAG, "[%s] read address=0x%04X", __func__, address);
            cmd->address = address;
        }
        else if (0 == strncmp((const char *)cmd_buf, "ignore", strlen("ignore")))
        {
            LOG_D(LOG_TAG, "[%s] set ignore irq", __func__);
            uint8_t ignore_irq = 0;

            if (len - strlen("ignore") < 2)
            {
                LOG_E(LOG_TAG, "[%s] invalid irq_type", __func__);
                err = GF_ERROR_INVALID_DATA;
                break;
            }

            ignore_irq = (uint8_t) strtol((const char *)&cmd_buf[strlen("ignore") + 1],
                                          NULL, 16);
            LOG_D(LOG_TAG, "[%s] ignore irq type=0x%X", __func__, ignore_irq);
            cmd->ignore_irq_type = ignore_irq;
        }

        err = gf_hal_test_invoke_command(GF_CMD_TEST_DRIVER_CMD, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        if (0 == strncmp((const char *)cmd_buf, "get_mode", strlen("get_mode")))
        {
            LOG_D(LOG_TAG, "[%s] get_mode mode=%s, mode_code=%d", __func__,
                  gf_strmode(cmd->mode), cmd->mode);
        }
        else if (0 == strncmp((const char *)cmd_buf, "secure_share_memory_performance",
                              strlen("secure_share_memory_performance")))
        {
            uint32_t i;

            for (i = 0; i < cmd->secure_share_memory_count; i++)
            {
                LOG_E(LOG_TAG, "[%s] secure_share_memory_performance %llu ms share %d bytes", \
                      __func__, (long long unsigned int)(cmd->secure_share_memory_time[i] / 1000), \
                      cmd->secure_share_memory_size[i]);
            }
        }
        else if (0 == strncmp((const char *)cmd_buf, "read", strlen("read")))
        {
            LOG_D(LOG_TAG, "[%s] read address=0x%X , val=0x%X", __func__,
                  cmd->address, cmd->value);
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_enroll()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        gf_hal_destroy_timer(&g_long_pressed_timer_id);
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
        err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_UNTRUSTED_ENROLL);

        if (err != GF_SUCCESS)
        {
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT);
        g_enroll_invalid_template_num = 0;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_pause_enroll()
{
    gf_error_t err = GF_SUCCESS;
    gf_pause_enroll_t *cmd = NULL;
    uint32_t size = sizeof(gf_pause_enroll_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();

    do
    {
        cmd = (gf_pause_enroll_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_TEST_UNTRUSTED_PAUSE_ENROLL;
        cmd->pause_enroll_flag = 1;
        err = gf_hal_test_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (err == GF_SUCCESS)
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
    }

    FUNC_EXIT(err);
    return err;
}


gf_error_t hal_test_resume_enroll()
{
    gf_error_t err = GF_SUCCESS;
    gf_pause_enroll_t *cmd = NULL;
    uint32_t size = sizeof(gf_pause_enroll_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();

    do
    {
        cmd = (gf_pause_enroll_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_TEST_UNTRUSTED_PAUSE_ENROLL;
        cmd->pause_enroll_flag = 0;
        err = gf_hal_test_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (err == GF_SUCCESS)
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
    }

    FUNC_EXIT(err);
    return err;
}


gf_error_t hal_test_authenticate(void *dev, uint64_t operation_id,
                                 uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_authenticate_t *cmd = NULL;
    uint32_t size = sizeof(gf_authenticate_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u", __func__, group_id);
    UNUSED_VAR(dev);

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        gf_hal_destroy_timer(&g_long_pressed_timer_id);
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
        g_failed_attempts = 0;
        cmd = (gf_authenticate_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_TEST_UNTRUSTED_AUTHENTICATE;
        cmd->group_id = group_id;
        cmd->operation_id = operation_id;
        err = gf_hal_test_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_delete_untrusted_enrolled_finger(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        err = gf_hal_test_invoke_command_ex(
                  GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER);

        if (err != GF_SUCCESS)
        {
            break;
        }

        gf_hal_notify_test_remove_succeeded(0, 0);
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_access_frr_database()
{
    gf_error_t err = GF_SUCCESS;
    FILE *fp = NULL;
    uint32_t len = 0;
    uint8_t *test_result = NULL;
    uint8_t *metadata_array = NULL;
    FUNC_ENTER();

    do
    {
        int32_t tmp_tag_data = 0;
        uint32_t tmp_offset = 0;
        int32_t buf_full_data = 0;
        uint32_t buf_full_offset = 0;
        uint32_t update_pos_offset = 0;
        int32_t update_pos_data = 0;
        int32_t metadata_cnt = 0;

        if (access(FRR_DATABASE_FILE, F_OK) != 0)
        {
            err = GF_ERROR_FILE_NOT_EXIST;
            LOG_E(LOG_TAG, "[%s] file not exist. err=%s, errno=%d", __func__,
                  gf_strerror(err), err);
            break;
        }

        fp = fopen(FRR_DATABASE_FILE, "r");

        if (fp == NULL)
        {
            err = GF_ERROR_FILE_OPEN_FAILED;
            LOG_E(LOG_TAG, "[%s] can't open file. err=%s, errno=%d", __func__,
                  gf_strerror(err), err);
            break;
        }

        // tag data with 7 width end with '\n'
        uint32_t per_metadata_len = strlen("XQXXXAXXX") + 1;
        update_pos_offset = gf_hal_get_tag_data_pos(TAG_UPDATE_POS);
        update_pos_data = gf_hal_get_tag_data(fp, update_pos_offset);
        buf_full_offset = gf_hal_get_tag_data_pos(TAG_BUF_FULL);
        buf_full_data = gf_hal_get_tag_data(fp, buf_full_offset);

        if (buf_full_data == 1)
        {
            metadata_cnt = FRR_DATABASE_METADATA_MAX;
        }
        else
        {
            metadata_cnt = update_pos_data;
        }

        // there are 13 tags in our database file
        len = HAL_TEST_SIZEOF_INT32 * 13;
        len += HAL_TEST_SIZEOF_ARRAY(per_metadata_len * metadata_cnt);
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == test_result)
        {
            LOG_E(LOG_TAG, "[%s] out of memory ", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            len = 0;
            break;
        }

        memset(test_result, 0, len);
        uint8_t *current = test_result;
        tmp_offset = gf_hal_get_tag_data_pos(TAG_PACKAGE_VERSION);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_PACKAGE_VERSION,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_PROTOCOL_VERSION);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_PROTOCOL_VERSION,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_CHIP_TYPE);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE, tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(
                         TAG_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(
                         TAG_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT, tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_CHIP_SUPPORT_BIO);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_SUPPORT_BIO,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_IS_BIO_ENABLE);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_IS_BIO_ENABLE,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_AUTHENTICATED_WITH_BIO_SUCCESS_COUNT);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_AUTHENTICATED_WITH_BIO_SUCCESS_COUNT,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_AUTHENTICATED_WITH_BIO_FAILED_COUNT);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current,
                                        TEST_TOKEN_AUTHENTICATED_WITH_BIO_FAILED_COUNT,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_AUTHENTICATED_SUCCESS_COUNT);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATED_SUCCESS_COUNT,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_AUTHENTICATED_FAILED_COUNT);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_AUTHENTICATED_FAILED_COUNT,
                                        tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_BUF_FULL);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_BUF_FULL, tmp_tag_data);
        tmp_offset = gf_hal_get_tag_data_pos(TAG_UPDATE_POS);
        tmp_tag_data = gf_hal_get_tag_data(fp, tmp_offset);
        current = hal_test_encode_int32(current, TEST_TOKEN_UPDATE_POS, tmp_tag_data);
        metadata_array = (uint8_t *) GF_MEM_MALLOC(per_metadata_len * metadata_cnt);

        if (NULL == metadata_array)
        {
            LOG_E(LOG_TAG, "[%s] GF_MEM_MALLOC failed ", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        int32_t cnt = 0;
        cnt = fread(metadata_array, per_metadata_len, metadata_cnt, fp);

        if (cnt != metadata_cnt)
        {
            LOG_E(LOG_TAG, "[%s] read file length incorrect", __func__);
            err = GF_ERROR_FILE_READ_FAILED;
            break;
        }

        current = hal_test_encode_array(current, TEST_TOKEN_METADATA, metadata_array,
                                        per_metadata_len * metadata_cnt);
        hal_notify_test_memory_check(__func__, test_result, current, len);
        hal_notify_test_cmd(CMD_TEST_FRR_DATABASE_ACCESS, test_result, len);
    }
    while (0);

    if (fp != NULL)
    {
        fclose(fp);
    }

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    if (metadata_array != NULL)
    {
        GF_MEM_FREE(metadata_array);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_access_authenticate_ratio_database()
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    uint8_t* buf = NULL;
    do
    {
        if (g_hal_config.support_authenticate_ratio <= 0)
        {
            LOG_E(LOG_TAG, "[%s] no support authenticate ratio", __func__);
            break;
        }
        uint32_t len = 0;
        len = AUTH_RATIO_BUF_LEN + sizeof(uint32_t) * 4;  // auth ratio data length and two tag length
        buf = (uint8_t *) GF_MEM_MALLOC(len);
        if (NULL == buf)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, test_result", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        err =gf_hal_get_auth_ratio_record(buf, &len);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] read authenticate ratio data error", __func__);
            break;
        }
        hal_notify_test_cmd(CMD_TEST_AUTH_RATIO, buf, len);
    }
    while(0);

    if (buf != NULL)
    {
        GF_MEM_FREE(buf);
    }

    FUNC_EXIT(err);
    return err;
}
static gf_error_t hal_test_download_fw(const uint8_t *cmd_buf,
                                       uint32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_download_fw_cfg_t *cmd = NULL;
    uint8_t *result = NULL;
    const uint8_t *in_buf = cmd_buf;
    uint32_t token = 0;
    uint32_t data_len = 0;
    FUNC_ENTER();
    UNUSED_VAR(buf_len);

    do
    {
        if (cmd_buf == NULL)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            break;
        }

        gf_hal_reset_chip();
        uint32_t size = sizeof(gf_test_download_fw_cfg_t);
        cmd = (gf_test_download_fw_cfg_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        in_buf = hal_test_decode_uint32(&token, in_buf);
        in_buf = hal_test_decode_uint32(&data_len, in_buf);
        cmd->fw_data_len = data_len;
        memcpy(cmd->fw_data, in_buf, data_len);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_DOWNLOAD_FW, cmd, size);
        uint32_t len = HAL_TEST_SIZEOF_INT32;
        result = (uint8_t *) GF_MEM_MALLOC(len);

        if (result == NULL)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(result, 0, len);
        uint8_t *current = result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, result, current, len);
        hal_notify_test_cmd(CMD_TEST_DOWNLOAD_FW, result, len);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (result != NULL)
    {
        GF_MEM_FREE(result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_download_cfg(const uint8_t *cmd_buf,
                                        uint32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_download_fw_cfg_t *cmd = NULL;
    uint8_t *result = NULL;
    const uint8_t *in_buf = cmd_buf;
    uint32_t token = 0;
    uint32_t data_len = 0;
    FUNC_ENTER();
    UNUSED_VAR(buf_len);

    do
    {
        if (cmd_buf == NULL)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            break;
        }

        gf_hal_reset_chip();
        uint32_t size = sizeof(gf_test_download_fw_cfg_t);
        cmd = (gf_test_download_fw_cfg_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        in_buf = hal_test_decode_uint32(&token, in_buf);
        in_buf = hal_test_decode_uint32(&data_len, in_buf);
        cmd->cfg_data_len = data_len;
        memcpy(cmd->cfg_data, in_buf, data_len);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_DOWNLOAD_CFG, cmd, size);
        uint32_t len = HAL_TEST_SIZEOF_INT32;
        result = (uint8_t *) GF_MEM_MALLOC(len);

        if (result == NULL)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(result, 0, len);
        uint8_t *current = result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, result, current, len);
        hal_notify_test_cmd(CMD_TEST_DOWNLOAD_CFG, result, len);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (result != NULL)
    {
        GF_MEM_FREE(result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_download_cfg_for_fpckey(const uint8_t *cmd_buf,
                                                   uint32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_download_fw_cfg_t *cmd = NULL;
    uint8_t *result = NULL;
    const uint8_t *in_buf = cmd_buf;
    uint32_t token = 0;
    uint32_t data_len = 0;
    uint32_t download_fpc_flag = 0;
    uint8_t decode_success_flag = 1;
    FUNC_ENTER();
    UNUSED_VAR(buf_len);

    do
    {
        if (cmd_buf == NULL)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            break;
        }

        uint32_t size = sizeof(gf_test_download_fw_cfg_t);
        cmd = (gf_test_download_fw_cfg_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);

        do
        {
            in_buf = hal_test_decode_uint32(&token, in_buf);

            if (token == TEST_TOKEN_FPC_DOWNLOAD_CFG)
            {
                in_buf = hal_test_decode_uint32(&download_fpc_flag, in_buf);
                cmd->fpc_key_dl_flag = download_fpc_flag;
            }
            else if (token == TEST_PARAM_TOKEN_CFG_DATA)
            {
                in_buf = hal_test_decode_uint32(&data_len, in_buf);
                cmd->cfg_data_len = data_len;
                memcpy(cmd->cfg_data, in_buf, data_len);
                in_buf += data_len;
            }
            else
            {
                LOG_E(LOG_TAG, "[%s] invalid token:%d.", __func__, token);
                decode_success_flag = 0;
                break;
            }
        }
        while (in_buf - cmd_buf < buf_len);

        if (0 == decode_success_flag)
        {
            LOG_E(LOG_TAG, "[%s] failed to decode cfg, data is valid.", __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        gf_hal_reset_chip();
        err = gf_hal_test_invoke_command(GF_CMD_TEST_DOWNLOAD_CFG, cmd, size);

        if (1 == download_fpc_flag)
        {
            g_fpc_config_had_downloaded = (err == GF_SUCCESS ? 1 : 0);
        }

        LOG_D(LOG_TAG, "[%s]  g_fpc_config_had_downloaded? (%d)", __func__,
              g_fpc_config_had_downloaded);
        uint32_t len = HAL_TEST_SIZEOF_INT32;
        result = (uint8_t *) GF_MEM_MALLOC(len);

        if (result == NULL)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(result, 0, len);
        uint8_t *current = result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, result, current, len);
        hal_notify_test_cmd(CMD_TEST_FPC_KEY_DOWNLOAD_CFG, result, len);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (result != NULL)
    {
        GF_MEM_FREE(result);
    }

    FUNC_EXIT(err);
    return err;
}


static gf_error_t hal_test_download_fwcfg(const uint8_t *cmd_buf,
                                          uint32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_download_fw_cfg_t *cmd = NULL;
    uint8_t *result = NULL;
    const uint8_t *in_buf = cmd_buf;
    uint32_t token = 0;
    uint32_t fw_data_len = 0;
    uint32_t cfg_data_len = 0;
    FUNC_ENTER();

    do
    {
        if (cmd_buf == NULL)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            break;
        }

        gf_hal_reset_chip();
        // this command is only for milan A series
        uint32_t size = sizeof(gf_test_download_fw_cfg_t);
        cmd = (gf_test_download_fw_cfg_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);

        do
        {
            in_buf = hal_test_decode_uint32(&token, in_buf);

            if (TEST_PARAM_TOKEN_FW_DATA == token)
            {
                in_buf = hal_test_decode_uint32(&fw_data_len, in_buf);
                cmd->fw_data_len = fw_data_len;
                memcpy(cmd->fw_data, in_buf, fw_data_len);
                in_buf += fw_data_len;
            }
            else if (TEST_PARAM_TOKEN_CFG_DATA == token)
            {
                in_buf = hal_test_decode_uint32(&cfg_data_len, in_buf);
                cmd->cfg_data_len = cfg_data_len;
                memcpy(cmd->cfg_data, in_buf, cfg_data_len);
                in_buf += cfg_data_len;
            }
        }
        while (in_buf < cmd_buf + buf_len);

        err = gf_hal_test_invoke_command(GF_CMD_TEST_DOWNLOAD_FWCFG, cmd, size);
        uint32_t len = HAL_TEST_SIZEOF_INT32;
        result = (uint8_t *) GF_MEM_MALLOC(len);

        if (result == NULL)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(result, 0, len);
        uint8_t *current = result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, result, current, len);
        hal_notify_test_cmd(CMD_TEST_DOWNLOAD_FWCFG, result, len);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (result != NULL)
    {
        GF_MEM_FREE(result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_read_cfg(const uint8_t *cmd_buf, uint32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_download_fw_cfg_t *cmd = NULL;
    uint32_t size = sizeof(gf_test_download_fw_cfg_t);
    uint8_t *result = NULL;
    uint32_t token = 0;
    uint32_t data_len = 0;
    const uint8_t *in_buf = cmd_buf;
    FUNC_ENTER();
    UNUSED_VAR(len);

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    in_buf = hal_test_decode_uint32(&token, in_buf);
    in_buf = hal_test_decode_uint32(&data_len, in_buf);

    do
    {
        gf_hal_reset_chip();
        cmd = (gf_test_download_fw_cfg_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->cfg_data_len = data_len;
        err = gf_hal_test_invoke_command(GF_CMD_TEST_READ_CFG, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] GF_CMD_TEST_READ_CFG command fail=%d", __func__, err);
            break;
        }

        {
            uint32_t len = HAL_TEST_SIZEOF_INT32;
            uint8_t *current = NULL;
            len += HAL_TEST_SIZEOF_ARRAY(cmd->cfg_data_len);
            result = (uint8_t *) GF_MEM_MALLOC(len);

            if (result == NULL)
            {
                LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(result, 0, len);
            current = result;
            current = hal_test_encode_array(current, TEST_TOKEN_CFG_DATA, cmd->cfg_data,
                                            cmd->cfg_data_len);
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
            hal_notify_test_memory_check(__func__, result, current, len);
            hal_notify_test_cmd(CMD_TEST_READ_CFG, result, len);
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (result != NULL)
    {
        GF_MEM_FREE(result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_read_fw(const uint8_t *cmd_buf, uint32_t len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_download_fw_cfg_t *cmd = NULL;
    uint32_t size = sizeof(gf_test_download_fw_cfg_t);
    uint8_t *result = NULL;
    uint32_t token = 0;
    uint32_t data_len = 0;
    const uint8_t *in_buf = cmd_buf;
    FUNC_ENTER();
    UNUSED_VAR(len);

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    in_buf = hal_test_decode_uint32(&token, in_buf);
    in_buf = hal_test_decode_uint32(&data_len, in_buf);

    do
    {
        gf_hal_reset_chip();
        cmd = (gf_test_download_fw_cfg_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->fw_data_len = data_len;
        err = gf_hal_test_invoke_command(GF_CMD_TEST_READ_FW, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] GF_CMD_TEST_READ_FW command fail=%d", __func__, err);
            break;
        }

        {
            uint32_t len = HAL_TEST_SIZEOF_INT32;
            uint8_t *current = NULL;
            len += HAL_TEST_SIZEOF_ARRAY(cmd->fw_data_len);
            result = (uint8_t *) GF_MEM_MALLOC(len);

            if (result == NULL)
            {
                LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(result, 0, len);
            current = result;
            current = hal_test_encode_array(current, TEST_TOKEN_FW_DATA, cmd->fw_data,
                                            cmd->fw_data_len);
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
            hal_notify_test_memory_check(__func__, result, current, len);
            hal_notify_test_cmd(CMD_TEST_READ_FW, result, len);
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (result != NULL)
    {
        GF_MEM_FREE(result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_reset_fwcfg()
{
    gf_error_t err = GF_SUCCESS;
    uint8_t *result = NULL;
    FUNC_ENTER();

    do
    {
        gf_hal_reset_chip();
        err = gf_hal_download_fw();

        if (GF_SUCCESS != err)
        {
            break;
        }

        // milan Bn won't download cfg again
        if (GF_MILAN_AN_SERIES != g_hal_config.chip_series)
        {
            err = gf_hal_download_cfg();

            if (GF_SUCCESS != err)
            {
                break;
            }
        }

        uint32_t len = HAL_TEST_SIZEOF_INT32;
        result = (uint8_t *) GF_MEM_MALLOC(len);

        if (result == NULL)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(result, 0, len);
        uint8_t *current = result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, result, current, len);

        if (1 == g_fpc_config_had_downloaded)
        {
            hal_notify_test_cmd(CMD_TEST_FPC_KEY_RESET_FWCFG, result, len);
            g_fpc_config_had_downloaded = 0;
        }
        else
        {
            hal_notify_test_cmd(CMD_TEST_RESET_FWCFG, result, len);
        }
    }
    while (0);

    if (result != NULL)
    {
        GF_MEM_FREE(result);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_rawdata_saturated()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_RAWDATA_SATURATED);
    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_fpc_key_detect(const uint8_t *cmd_buf,
                                          uint32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_fpc_key_data_t *data = NULL;
    uint32_t size = 0;
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    uint8_t fpc_key_type = 0;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        if (buf_len != 1)
        {
            LOG_E(LOG_TAG, "[%s] params length error, buf_len=%u", __func__, buf_len);
            break;
        }

        fpc_key_type = cmd_buf[0];
        LOG_E(LOG_TAG, "[%s] fpc key=0x%x", __func__, fpc_key_type);
        size = sizeof(gf_test_fpc_key_data_t);
        data = (gf_test_fpc_key_data_t *) GF_MEM_MALLOC(size);

        if (NULL == data)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(data, 0, size);
        data->key_code = fpc_key_type;
        err = gf_hal_test_invoke_command(GF_CMD_TEST_FPC_KEY_DETECT, data, size);
        LOG_I(LOG_TAG, "[%s] fpc key, en = 0x%x fail = %d", __func__, data->fpc_key_en,
              data->fail_stat);
        // gf_sensor_type_t sensor_type;
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // uint32_t fpc key enable flag;
        len += HAL_TEST_SIZEOF_INT8;
        // fail status
        len += HAL_TEST_SIZEOF_INT8;
        // if can test
        len += HAL_TEST_SIZEOF_INT8;
        // key rawdata
        len += HAL_TEST_SIZEOF_ARRAY(6);
        // cancelation data
        len += HAL_TEST_SIZEOF_ARRAY(6);
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (test_result != NULL)
        {
            memset(test_result, 0, len);
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                            g_hal_config.chip_type);
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
            current = hal_test_encode_int8(current, TEST_TOKEN_FPC_KEY_EN_FLAG,
                                           data->fpc_key_en);
            current = hal_test_encode_int8(current, TEST_TOKEN_FPC_KEY_FAIL_STATUS,
                                           data->fail_stat);
            current = hal_test_encode_int8(current, TEST_TOKEN_FPC_KEY_CAN_TEST,
                                           data->can_test);
            current = hal_test_encode_array(current, TEST_TOKEN_FPC_KEY_RAWDATA,
                                            data->key_rawdata, 6);
            current = hal_test_encode_array(current, TEST_TOKEN_FPC_KEY_CANCELDATA,
                                            data->fpc_key_cancel, 6);
            hal_notify_test_memory_check(__func__, test_result, current, len);
        }
        else
        {
            len = 0;
        }

        hal_notify_test_cmd(CMD_TEST_FPC_KEY, test_result, len);
    }
    while (0);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    if (data != NULL)
    {
        GF_MEM_FREE(data);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t hal_test_sensor_validity(void)
{
    gf_error_t err = GF_SUCCESS;
    gf_test_sensor_validity_t *cmd = NULL;
    uint32_t retry = 2;
    FUNC_ENTER();

    do
    {
        uint32_t size = sizeof(gf_test_sensor_validity_t);
        cmd = (gf_test_sensor_validity_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] buf out of memory  while malloc cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        do
        {
            memset(cmd, 0, size);
            err = gf_hal_test_invoke_command(GF_CMD_TEST_SENSOR_VALIDITY, cmd, size);
            if (cmd->cmd_header.reset_flag)
            {
               gf_hal_test_invoke_command_ex(GF_CMD_TEST_RESET_CLEAR);
               continue;
            }
            break;
        } while(retry--);

        // Factory test about SENSOR_VALIDITY
        if (NULL != g_fingerprint_device->factory_test_notify)
        {
            g_fingerprint_device->factory_test_notify((uint32_t)CMD_TEST_SENSOR_VALIDITY,
                                                      err, (void *)cmd);
        }

        if (0 == cmd->cmd_header.reset_flag)
        {
            hal_notify_test_sensor_validity(err, cmd->is_passed);
        }

        LOG_D(LOG_TAG, "[%s] chip check result=%d", __func__, cmd->is_passed);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_set_memmgr_config(const uint8_t *cmd_buf,
                                             uint8_t cmd_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_memmgr_config_t *cmd = NULL;
    uint32_t memmgr_enable = 0;
    uint32_t debug_enable = 0;
    uint32_t match_best_mem_pool_enable = 0;
    uint32_t erase_mem_pool_when_free = 0;
    uint32_t dump_time_enable = 0;
    uint32_t memmgr_pool_size = 0;
    uint32_t token = 0;
    const uint8_t *in_buf = cmd_buf;
    FUNC_ENTER();

    if (NULL == cmd_buf)
    {
        LOG_E(LOG_TAG, "[%s] cmd_buf is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    do
    {
        in_buf = hal_test_decode_uint32(&token, in_buf);

        switch (token)
        {
            case TEST_TOKEN_MEMMGR_ENABLE:
            {
                in_buf = hal_test_decode_uint32(&memmgr_enable, in_buf);
                break;
            }

            case TEST_TOKEN_MEMMGR_DEBUG_ENABLE:
            {
                in_buf = hal_test_decode_uint32(&debug_enable, in_buf);
                break;
            }

            case TEST_TOKEN_MEMMGR_BEST_MATCH_ENABLE:
            {
                in_buf = hal_test_decode_uint32(&match_best_mem_pool_enable, in_buf);
                break;
            }

            case TEST_TOKEN_MEMMGR_FREE_ERASE_ENABLE:
            {
                in_buf = hal_test_decode_uint32(&erase_mem_pool_when_free, in_buf);
                break;
            }

            case TEST_TOKEN_MEMMGR_DUMP_TIME_ENABLE:
            {
                in_buf = hal_test_decode_uint32(&dump_time_enable, in_buf);
                break;
            }

            case TEST_TOKEN_MEMMGR_POOL_SIZE:
            {
                in_buf = hal_test_decode_uint32(&memmgr_pool_size, in_buf);
                break;
            }

            default:
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
        }

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter, token=%u", __func__, token);
            break;
        }
    }
    while (in_buf < cmd_buf + cmd_len);

    do
    {
        uint32_t size = 0;

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] bad input config parameter", __func__);
            break;
        }

        size = sizeof(gf_memmgr_config_t);
        cmd = (gf_memmgr_config_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] buf out of memory  while GF_MEM_MALLOC cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->memmgr_enable = (uint8_t) memmgr_enable;
        cmd->debug_enable = (uint8_t) debug_enable;
        cmd->erase_mem_pool_when_free = (uint8_t) erase_mem_pool_when_free;
        cmd->match_best_mem_pool_enable = (uint8_t) match_best_mem_pool_enable;
        cmd->dump_time_enable = (uint8_t) dump_time_enable;
        cmd->memmgr_pool_size = memmgr_pool_size;
        err = g_hal_function.user_invoke_command(GF_USER_CMD_TEST_SET_MEMMGR_CONFIG,
                                                 cmd, size);
    }
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_get_memmgr_config()
{
    gf_error_t err = GF_SUCCESS;
    uint32_t len = 0;
    uint8_t *test_result = NULL;
    gf_memmgr_config_t *cmd = NULL;
    FUNC_ENTER();

    do
    {
        uint32_t size = sizeof(gf_memmgr_config_t);
        cmd = (gf_memmgr_config_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = g_hal_function.user_invoke_command(GF_USER_CMD_TEST_GET_MEMMGR_CONFIG,
                                                 cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        // error code
        len += HAL_TEST_SIZEOF_INT32;
        // memmgr_enable
        len += HAL_TEST_SIZEOF_INT32;
        // debug_enable
        len += HAL_TEST_SIZEOF_INT32;
        // match_best_mem_pool_enable
        len += HAL_TEST_SIZEOF_INT32;
        // erase_mem_pool_when_free
        len += HAL_TEST_SIZEOF_INT32;
        // dump_time_enable
        len += HAL_TEST_SIZEOF_INT32;
        // enable_memmgr_next_reboot
        len += HAL_TEST_SIZEOF_INT32;
        // pool_size
        len += HAL_TEST_SIZEOF_INT32;
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == test_result)
        {
            len = 0;
            LOG_E(LOG_TAG, "[%s] test_result out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        else
        {
            memset(test_result, 0, len);
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE,
                                            err);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_ENABLE,
                                            (uint32_t) cmd->memmgr_enable);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_DEBUG_ENABLE,
                                            (uint32_t) cmd->debug_enable);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_BEST_MATCH_ENABLE,
                                            (uint32_t) cmd->match_best_mem_pool_enable);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_FREE_ERASE_ENABLE,
                                            (uint32_t) cmd->erase_mem_pool_when_free);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_DUMP_TIME_ENABLE,
                                            (uint32_t) cmd->dump_time_enable);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_NEXT_REBOOT_ENABLE,
                                            (uint32_t) cmd->enable_memmgr_next_reboot);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_POOL_SIZE,
                                            cmd->memmgr_pool_size);
            hal_notify_test_memory_check(__func__, test_result, current, len);
        }

        hal_notify_test_cmd(CMD_TEST_MEMMGR_GET_CONFIG, test_result, len);
    }
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    if (NULL != test_result)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_get_memmgr_info()
{
    gf_error_t err = GF_SUCCESS;
    uint32_t len = 0;
    uint8_t *test_result = NULL;
    gf_memmgr_info_t *cmd = NULL;
    FUNC_ENTER();

    do
    {
        uint32_t node_count = 0;
        uint32_t size = 0;
        size = sizeof(gf_memmgr_info_t);
        cmd = (gf_memmgr_info_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = g_hal_function.user_invoke_command(GF_USER_CMD_TEST_GET_MEMMGR_INFO, cmd,
                                                 size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        node_count = cmd->transport_node_count;

        if (cmd->total_node_count != node_count)
        {
            LOG_E(LOG_TAG, "[%s] too much node, total_node_count=%u, node_count=%u",
                  __func__, cmd->total_node_count, node_count);
            err = GF_ERROR_MAX_NUM;
            break;
        }

        /* Notice:
         * So far, we just assume it could finish transport node info one time
         */
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // mem_pool_start_addr
        len += HAL_TEST_SIZEOF_INT64;
        // mem_pool_end_addr
        len += HAL_TEST_SIZEOF_INT64;
        // cur_used_block_count
        len += HAL_TEST_SIZEOF_INT32;
        // max_used_block_count
        len += HAL_TEST_SIZEOF_INT32;
        // cur_used_mem_size
        len += HAL_TEST_SIZEOF_INT32;
        // maxs_used_mem_size
        len += HAL_TEST_SIZEOF_INT32;
        // total_node_count
        len += HAL_TEST_SIZEOF_INT32;
        // node_info_arr
        len += HAL_TEST_SIZEOF_ARRAY(node_count * cmd->node_info_size);
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (NULL == test_result)
        {
            len = 0;
            LOG_E(LOG_TAG, "[%s] test_result out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        else
        {
            memset(test_result, 0, len);
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE,
                                            err);
            current = hal_test_encode_int64(current, TEST_TOKEN_MEMMGR_POOL_START_ADDR,
                                            (int64_t) cmd->mem_pool_start_addr);  // compatible with 64 bit pinter
            current = hal_test_encode_int64(current, TEST_TOKEN_MEMMGR_POOL_END_ADDR,
                                            (int64_t) cmd->mem_pool_end_addr);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_CUR_USED_BLOCK_COUNT,
                                            cmd->cur_used_block_count);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_MAX_USED_BLOCK_COUNT,
                                            cmd->max_used_block_count);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_CUR_USED_MEM_SIZE,
                                            cmd->cur_used_mem_size);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_MAX_USED_MEM_SIZE,
                                            cmd->maxs_used_mem_size);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_TOTAL_NODE_COUNT,
                                            cmd->total_node_count);
            current = hal_test_encode_array(current, TEST_TOKEN_MEMMGR_NODE_INFO,
                                            (uint8_t *) cmd->mem_pool_node_data, node_count * cmd->node_info_size);
            hal_notify_test_memory_check(__func__, test_result, current, len);
        }

        hal_notify_test_cmd(CMD_TEST_MEMMGR_GET_INFO, test_result, len);
    }
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    if (NULL != test_result)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_dump_memmgr_pool()
{
    gf_error_t err = GF_SUCCESS;
    struct timeval tv;
    int8_t cur_time_str[GF_DUMP_FILE_TIME_MAX_LEN] = { 0 };
    struct tm current_tm = { 0 };
    gf_memmgr_pool_dump_t *cmd = NULL;
    uint32_t offset = 0;
    uint32_t len = 0;
    uint8_t *test_result = NULL;
    uint8_t *current = NULL;  // use for notify
    FUNC_ENTER();

    do
    {
        uint32_t tmp_len = 0;
        uint32_t result_len = 0;
        uint32_t cur_time_str_len = 0;
        uint32_t size = 0;
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &current_tm);
        snprintf((char *) cur_time_str, GF_DUMP_FILE_TIME_MAX_LEN,
                 "%04d-%02d-%02d-%02d-%02d-%02d-%06ld", current_tm.tm_year + 1900,
                 current_tm.tm_mon + 1, current_tm.tm_mday, current_tm.tm_hour,
                 current_tm.tm_min, current_tm.tm_sec, tv.tv_usec);
        cur_time_str_len = strlen((char *) cur_time_str);
        size = sizeof(gf_memmgr_pool_dump_t);
        cmd = (gf_memmgr_pool_dump_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        // time_str
        len += HAL_TEST_SIZEOF_ARRAY(cur_time_str_len);
        // offset
        len += HAL_TEST_SIZEOF_INT32;
        // finished
        len += HAL_TEST_SIZEOF_INT32;
        tmp_len = len;
        // pool_arr
        len += HAL_TEST_SIZEOF_ARRAY(DUMP_MEM_POOL_BUF_LEN);
        result_len = len;
        test_result = (uint8_t *) GF_MEM_MALLOC(result_len);

        if (NULL == test_result)
        {
            len = 0;
            LOG_E(LOG_TAG, "[%s] test_result out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        do
        {
            cmd->offset = offset;
            err = g_hal_function.user_invoke_command(GF_USER_CMD_TEST_DUMP_MEMMGR_POOL, cmd,
                                                     size);

            if (err != GF_SUCCESS)
            {
                break;
            }

            err = gf_dump_memmgr_pool((uint8_t *) cmd->pool_arr, cmd->dump_len,
                                      cur_time_str);

            if (err != GF_SUCCESS)
            {
                break;
            }

            current = test_result;
            memset(test_result, 0, result_len);
            current = hal_test_encode_array(current, TEST_TOKEN_MEMMGR_DUMP_TIME,
                                            (uint8_t *) cur_time_str, cur_time_str_len);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_DUMP_OFFSET,
                                            cmd->offset);
            current = hal_test_encode_int32(current, TEST_TOKEN_MEMMGR_DUMP_FINISHED,
                                            (uint32_t) cmd->finished);
            current = hal_test_encode_array(current, TEST_TOKEN_MEMMGR_DUMP_POOL,
                                            (uint8_t *) cmd->pool_arr, cmd->dump_len);
            len = tmp_len + HAL_TEST_SIZEOF_ARRAY(cmd->dump_len);
            hal_notify_test_memory_check(__func__, test_result, current, len);
            hal_notify_test_cmd(CMD_TEST_MEMMGR_DUMP_POOL, test_result, len);
            offset += cmd->dump_len;
        }
        while (0 == cmd->finished);

        /*
         * the break below is just remind the extra do...while,
         * in consideration of maybe need add code after this break in the future
         */
        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    if (NULL != test_result)
    {
        GF_MEM_FREE(test_result);
        test_result = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hal_test_stable_factor(void)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    gf_hal_create_timer(&g_stable_test_timeout_timer_id, gf_test_stable_timer_thread);
    gf_hal_set_timer(&g_stable_test_timeout_timer_id, 0,
                     GF_STABLE_TEST_TIME_OUT_THRESHOLD, 0);
    do
    {
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_STABLE_FACTOR, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] execute GF_CMD_TEST_STABLE_FACTOR failed", __func__);
            break;
        }
    }
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

void gf_hal_notify_test_acquired_info(gf_fingerprint_acquired_info_t
                                      acquired_info)
{
    gf_fingerprint_msg_t message = { 0 };

    if (NULL == g_fingerprint_device || NULL == g_fingerprint_device->test_notify)
    {
        LOG_E(LOG_TAG, "[%s] NULL device or notify", __func__);
        return;
    }

    message.type = GF_FINGERPRINT_ACQUIRED;
    message.data.acquired.acquired_info = acquired_info;
    g_fingerprint_device->test_notify(&message);
}

void gf_hal_notify_test_error_info(gf_fingerprint_error_t err_code)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
    {
        gf_fingerprint_msg_t message = { 0 };
        message.type = GF_FINGERPRINT_ERROR;
        message.data.error = err_code;
        g_fingerprint_device->test_notify(&message);
    }
}

void gf_hal_notify_test_enrollment_progress(uint32_t group_id,
                                            uint32_t finger_id,
                                            uint32_t samples_remaining)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
    {
        gf_fingerprint_msg_t message = { 0 };
        message.type = GF_FINGERPRINT_TEMPLATE_ENROLLING;
        message.data.enroll.finger.gid = group_id;
        message.data.enroll.finger.fid = finger_id;
        message.data.enroll.samples_remaining = samples_remaining;
        g_fingerprint_device->test_notify(&message);
    }
}

void gf_hal_notify_test_authentication_failed()
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
    {
        gf_fingerprint_msg_t message = { 0 };
        message.type = GF_FINGERPRINT_AUTHENTICATED;
        message.data.authenticated.finger.gid = 0;
        message.data.authenticated.finger.fid = 0;  // 0 is authenticate failed
        g_fingerprint_device->test_notify(&message);
    }
}

void gf_hal_notify_test_authentication_succeeded(uint32_t group_id,
                                                 uint32_t finger_id,
                                                 gf_hw_auth_token_t *auth_token)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
    {
        gf_fingerprint_msg_t message = { 0 };
        message.type = GF_FINGERPRINT_AUTHENTICATED;
        message.data.authenticated.finger.gid = group_id;
        message.data.authenticated.finger.fid = finger_id;
        memcpy(&message.data.authenticated.hat, auth_token, sizeof(gf_hw_auth_token_t));
        g_fingerprint_device->test_notify(&message);
    }
}

void gf_hal_notify_test_remove_succeeded(uint32_t group_id, uint32_t finger_id)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
    {
        gf_fingerprint_msg_t message = { 0 };
        message.type = GF_FINGERPRINT_TEMPLATE_REMOVED;
        message.data.removed.finger.gid = group_id;
        message.data.removed.finger.fid = finger_id;
        g_fingerprint_device->test_notify(&message);
    }
}

gf_error_t gf_hal_test_invoke_command(gf_cmd_id_t cmd_id, void *buffer,
                                      int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (NULL == buffer)
    {
        LOG_E(LOG_TAG, "[%s] buffer is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        err = g_hal_function.invoke_command(GF_TEST_OPERATION_ID, cmd_id, buffer, len);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_test_invoke_command_ex(gf_cmd_id_t cmd_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(cmd_id, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_test_cmd(void *dev, uint32_t cmd_id, const uint8_t *param,
                           uint32_t param_len)
{
    char algo_version[ALGO_VERSION_INFO_LEN] = {0};
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] cmd_id=%u", __func__, cmd_id);

    if (0 == g_hal_inited)
    {
        err = GF_ERROR_GENERIC;
        LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
        return err;
    }

    pthread_mutex_lock(&g_hal_mutex);
    // check dump data flag and delete directory
    gf_dump_prepare_for_test_cmd(cmd_id);

    switch (cmd_id)
    {
        case CMD_TEST_ENUMERATE:
        {
            err = hal_test_enumerate();
            break;
        }

        case CMD_TEST_DRIVER:
        {
            err = hal_test_driver_cmd(param, param_len);
            break;
        }

        case CMD_TEST_PIXEL_OPEN:
        {
            err = hal_test_pixel_open();
            break;
        }

        case CMD_TEST_PIXEL_SHORT_STREAK:
        {
            err = hal_test_pixel_short_streak();
            break;
        }

        case CMD_TEST_BAD_POINT:
        {
            err = hal_test_bad_point();
            break;
        }

        case CMD_TEST_CALIBRATION_PARA_RETEST:
        {
            err = hal_test_calibration_para_retest();
            break;
        }

        case CMD_TEST_PERFORMANCE:
        {
            g_down_irq_time = gf_hal_current_time_microsecond();
            err = hal_test_performance();
            break;
        }

        case CMD_TEST_SPI_PERFORMANCE:
        {
            err = hal_test_spi_performance();
            break;
        }

        case CMD_TEST_SPI_TRANSFER:
        {
            err = hal_test_spi_transfer(param, param_len);
            break;
        }

        case CMD_TEST_SPI:
        {
            err = hal_test_spi();
            break;
        }

        case CMD_TEST_GET_VERSION:
        {
            err = hal_test_get_version(algo_version);
            break;
        }

        case CMD_TEST_FRR_FAR_GET_CHIP_TYPE:
        {
            err = hal_test_frr_far_get_chip_type();
            break;
        }

        case CMD_TEST_FRR_FAR_INIT:
        {
            err = hal_test_frr_far_init(param, param_len);
            break;
        }

        case CMD_TEST_FRR_FAR_RECORD_CALIBRATION:
        {
            err = hal_test_frr_far_record_calibration();
            break;
        }

        case CMD_TEST_FRR_FAR_RECORD_ENROLL:
        {
            err = hal_test_frr_far_record_enroll();
            break;
        }

        case CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE:
        {
            err = hal_test_frr_far_record_authenticate();
            break;
        }

        case CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH:
        {
            err = hal_test_frr_far_record_authenticate_finish();
            break;
        }

        case CMD_TEST_FRR_FAR_PLAY_CALIBRATION:
        {
            err = hal_test_frr_far_play_calibration(param, param_len);
            break;
        }

        case CMD_TEST_FRR_FAR_PLAY_ENROLL:
        {
            err = hal_test_frr_far_play_enroll(param, param_len);
            break;
        }

        case CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE:
        {
            err = hal_test_frr_far_play_authenticate(param, param_len);
            break;
        }

        case CMD_TEST_FRR_FAR_ENROLL_FINISH:
        {
            err = hal_test_frr_far_enroll_finish();
            break;
        }

        case CMD_TEST_FRR_FAR_SAVE_FINGER:
        {
            err = hal_test_frr_far_save_finger(param, param_len);
            break;
        }

        case CMD_TEST_FRR_FAR_DEL_FINGER:
        {
            err = hal_test_frr_far_del_finger(param, param_len);
            break;
        }

        case CMD_TEST_FRR_FAR_PREPROCESS_INIT:
        {
            err = hal_test_frr_far_preprocess_init();
            break;
        }

        case CMD_TEST_CANCEL_FRR_FAR:
        {
            err = hal_test_frr_far_cancel();
            break;
        }

        case CMD_TEST_REAL_TIME_DATA:
        {
            err = hal_test_real_time_data();
            break;
        }

        case CMD_TEST_BMP_DATA:
        {
            err = hal_test_bmp_data();
            break;
        }

        case CMD_TEST_CANCEL:
        {
            err = hal_test_cancel();
            break;
        }

        case CMD_TEST_PRIOR_CANCEL:
        {
            err = hal_test_prior_cancel();
            break;
        }

        case CMD_TEST_RESET_PIN:
        {
            err = hal_test_reset_pin();
            break;
        }

        case CMD_TEST_SENSOR_FINE:
        {
            g_test_sensor_fine_count = 0;
            err = hal_test_sensor_fine();
            break;
        }

        case CMD_TEST_INTERRUPT_PIN:
        {
            err = hal_test_interrupt_pin();
            break;
        }

        case CMD_TEST_UNTRUSTED_ENROLL:
        {
            err = hal_test_enroll();
            break;
        }

        case CMD_TEST_UNTRUSTED_PAUSE_ENROLL:
        {
            err = hal_test_pause_enroll();
            break;
        }

        case CMD_TEST_UNTRUSTED_RESUME_ENROLL:
        {
            err = hal_test_resume_enroll();
            break;
        }

        case CMD_TEST_UNTRUSTED_AUTHENTICATE:
        {
            err = hal_test_authenticate(dev, 0, 0);
            break;
        }

        case CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER:
        {
            err = hal_test_delete_untrusted_enrolled_finger();
            break;
        }

        case CMD_TEST_CHECK_FINGER_EVENT:
        {
            err = hal_test_check_finger_touch_event();
            break;
        }

        case CMD_TEST_BIO_CALIBRATION:
        {
            err = hal_test_bio_calibration();
            break;
        }

        case CMD_TEST_HBD_CALIBRATION:
        {
            err = hal_test_hbd_calibration();
            break;
        }

        case CMD_TEST_GET_CONFIG:
        {
            err = hal_test_get_config();
            break;
        }

        case CMD_TEST_DOWNLOAD_FW:
        {
            err = hal_test_download_fw(param, param_len);
            break;
        }

        case CMD_TEST_FPC_KEY_DOWNLOAD_CFG:
        {
            err = hal_test_download_cfg_for_fpckey(param, param_len);
            break;
        }

        case CMD_TEST_DOWNLOAD_CFG:
        {
            err = hal_test_download_cfg(param, param_len);
            break;
        }

        case CMD_TEST_DOWNLOAD_FWCFG:
        {
            err = hal_test_download_fwcfg(param, param_len);
            break;
        }

        case CMD_TEST_READ_CFG:
        {
            err = hal_test_read_cfg(param, param_len);
            break;
        }

        case CMD_TEST_READ_FW:
        {
            err = hal_test_read_fw(param, param_len);
            break;
        }

        case CMD_TEST_NOISE:
        {
            err = hal_test_data_noise();
            break;
        }

        case CMD_TEST_FPC_KEY_RESET_FWCFG:
        case CMD_TEST_RESET_FWCFG:
        {
            uint32_t retry_times = 0;

            for (retry_times = 0; retry_times < 3; retry_times++)
            {
                err = hal_test_reset_fwcfg();

                if (GF_SUCCESS == err)
                {
                    LOG_D(LOG_TAG, "[%s] success to reset fw and cfg.", __func__);
                    break;
                }

                LOG_E(LOG_TAG, "[%s] retry reset fw and cfg times:%d.", __func__, retry_times);
            }

            break;
        }

        case CMD_TEST_SENSOR_VALIDITY:
        {
            err = hal_test_sensor_validity();
            break;
        }

        case CMD_TEST_SET_CONFIG:
        {
            err = hal_test_set_config(param, param_len);
            break;
        }

        case CMD_TEST_RESET_CHIP:
        {
            pthread_mutex_lock(&g_sensor_mutex);
            gf_hal_reset_chip();
            pthread_mutex_unlock(&g_sensor_mutex);
            LOG_I(LOG_TAG, "[%s] CMD_TEST_RESET_CHIP", __func__);
            break;
        }

        case CMD_TEST_SPI_RW:
        {
            err = hal_test_spi_rw(param, param_len);
            break;
        }

        case CMD_TEST_FRR_DATABASE_ACCESS:
        {
            err = hal_test_access_frr_database();
            break;
        }

        case CMD_TEST_AUTH_RATIO:
        {
            err = hal_test_access_authenticate_ratio_database();
            break;
        }

        case CMD_TEST_RAWDATA_SATURATED:
        {
            err = hal_test_rawdata_saturated();
            break;
        }

        case CMD_TEST_FPC_KEY:
        {
            err = hal_test_fpc_key_detect(param, param_len);
            break;
        }

        case CMD_TEST_MEMMGR_SET_CONFIG:
        {
            err = hal_test_set_memmgr_config(param, param_len);
            break;
        }

        case CMD_TEST_MEMMGR_GET_CONFIG:
        {
            err = hal_test_get_memmgr_config();
            break;
        }

        case CMD_TEST_MEMMGR_GET_INFO:
        {
            err = hal_test_get_memmgr_info();
            break;
        }

        case CMD_TEST_MEMMGR_DUMP_POOL:
        {
            err = hal_test_dump_memmgr_pool();
            break;
        }

        case CMD_TEST_DISABLE_POWER:
        {
            err = gf_hal_disable_power();
            break;
        }

        case CMD_TEST_ENABLE_POWER:
        {
            err = gf_hal_enable_power();
            break;
        }

        case CMD_TEST_DEVICE_CLOSE:
        {
            gf_hal_device_close();
            break;
        }

        case CMD_TEST_DEVICE_OPEN:
        {
            gf_hal_device_open();
            break;
        }

        case CMD_TEST_STABLE_FACTOR:
        {
            hal_test_stable_factor();
            break;
        }

        case CMD_TEST_TWILL_BADPOINT:
        {
            hal_test_twill_badpoint();
            break;
        }

        default:
        {
            break;
        }
    }

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_test_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.test_cancel(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_test_prior_cancel(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        err = g_hal_function.test_prior_cancel(dev);
    }
    while (0);

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
    return err;
}

void hal_notify_test_fpc_detected(gf_error_t err, uint32_t finger_event,
                                  uint32_t status)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    do
    {
        // sensor type
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // finger event
        len += HAL_TEST_SIZEOF_INT32;
        // finger status
        len += HAL_TEST_SIZEOF_INT32;
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (test_result != NULL)
        {
            memset(test_result, 0, len);
            uint8_t *current = test_result;
            current = hal_test_encode_int32(current, TEST_TOKEN_CHIP_TYPE,
                                            g_hal_config.chip_type);
            current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
            current = hal_test_encode_int32(current, TEST_TOKEN_FPC_KEY_EVENT,
                                            finger_event);
            current = hal_test_encode_int32(current, TEST_TOKEN_FPC_KEY_STATUS, status);
        }
        else
        {
            len = 0;
        }

        LOG_D(LOG_TAG, "[%s] finger event: %d status: %d", __func__, finger_event,
              status);
        hal_notify_test_cmd(CMD_TEST_FPC_KEY, test_result, len);
    }
    while (0);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

void hal_notify_test_fpc_reset_fwcfg(gf_error_t err)
{
    uint8_t *test_result = NULL;
    uint32_t len = 0;
    VOID_FUNC_ENTER();

    do
    {
        len += HAL_TEST_SIZEOF_INT32;
        test_result = (uint8_t *) GF_MEM_MALLOC(len);

        if (test_result == NULL)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, result", __func__);
            break;
        }

        memset(test_result, 0, len);
        uint8_t *current = test_result;
        current = hal_test_encode_int32(current, TEST_TOKEN_ERROR_CODE, err);
        hal_notify_test_memory_check(__func__, test_result, current, len);
        hal_notify_test_cmd(CMD_TEST_FPC_KEY_RESET_FWCFG, test_result, len);
    }
    while (0);

    if (test_result != NULL)
    {
        GF_MEM_FREE(test_result);
    }

    VOID_FUNC_EXIT();
}

gf_error_t gf_hal_calculate_stable(void)
{
    gf_error_t err = GF_SUCCESS;
    stable_result_t *stable_in = NULL;
    uint32_t size_stable = sizeof(stable_result_t);
    FUNC_ENTER();

    do
    {
        stable_in = (stable_result_t *) GF_MEM_MALLOC(size_stable);

        if (NULL == stable_in)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            break;
        }

        memset(stable_in, 0, size_stable);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_STABLE_FACTOR_RESULT,
                    stable_in, size_stable);
        if (GF_SUCCESS == err)
        {
            LOG_D(LOG_TAG, "[%s] get stable result success", __func__);
        }
        else
        {
            LOG_E(LOG_TAG, "[%s] get stable result failed errno =%d", __func__, err);
            gf_hal_test_stable_stop_test();
            break;
        }
        hal_notify_test_stable_factor(0, stable_in->algo_stable_result);
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_test_stable_get_touch_data(void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t index = 0;
    clock_t start, finish;
    double duration;
    gf_irq_t *irq = (gf_irq_t *)buffer;
    FUNC_ENTER()

    do
    {
        if (NULL == buffer || len == 0)
        {
            LOG_E(LOG_TAG, "[%s] bad params, cmd", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        start = clock();

        for (index = 0; index < g_stable_max_rawdata_framenum;)
        {
            irq->test_data_noise.data_type = TEST_SNR_RAW_DATE;
            irq->test_data_noise.frame_num = index;
            irq->test_data_noise.max_frame_num = g_stable_max_rawdata_framenum;
            if (irq->step == GF_IRQ_STEP_PROCESS)
            {
                irq->step = GF_IRQ_STEP_POLLING;   // make sure polling 5 rawdata
            }
            err = gf_hal_test_invoke_command(GF_CMD_IRQ, irq, len);
            if (GF_SUCCESS == err)
            {
                LOG_D(LOG_TAG, "[%s] get rawdata success and index =%d", __func__, index);
                // dump touch data
                LOG_D(LOG_TAG, "[%s] dump snr touch %d data", __func__, index);
                gf_hal_dump_data_by_operation(OPERATION_TEST_DATA_NOISE, err);
                index++;
            }
            else
            {
                LOG_E(LOG_TAG, "[%s] get rawdata failed errno =%d", __func__, err);
                gf_hal_test_stable_stop_test();
                break;
            }
        }

        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        LOG_D(LOG_TAG, "[%s] get touch cost time =%05f", __func__, duration);
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t gf_hal_test_stable_get_base(void)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint8_t index = 0;
    uint32_t size = sizeof(gf_cmd_header_t);
    clock_t start, finish;
    double duration;
    FUNC_ENTER();

    do
    {
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        start = clock();
        memset(cmd, 0, size);
        err = gf_hal_test_invoke_command(GF_CMD_TEST_STABLE_FACTOR_BASE, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] execute GF_CMD_TEST_DATA_NOISE_BASE failed", __func__);
            break;
        }
        for (index = 0; index < g_stable_max_basedata_framenum;)
        {
            err = gf_hal_test_invoke_command(GF_CMD_TEST_STABLE_FACTOR_GET_BASE, cmd, size);

            if (GF_SUCCESS == err)
            {
                LOG_D(LOG_TAG, "[%s] get basedata success and index =%d", __func__, index);
                // dump touch data
                gf_hal_dump_data_by_operation(OPERATION_TEST_DATA_NOISE_BASE, err);
                index++;
            }
            else
            {
                LOG_E(LOG_TAG, "[%s] get basedata failed errno =%d", __func__, err);
                gf_hal_test_stable_stop_test();
                break;
            }
        }
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        LOG_D(LOG_TAG, "[%s] get base cost time  =%05f", __func__, duration);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
        cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hal_test_stable_get_result(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER()

    do
    {
        err = gf_hal_test_stable_get_base();

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] get base failed", __func__);
            break;
        }

        err = gf_hal_calculate_stable();
        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] get result failed", __func__);
            break;
        }
    }
    while (0);

    gf_hal_destroy_timer(&g_stable_test_timeout_timer_id);
    hal_test_cancel();
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_test_sensor_broken
 * Description: test id & fdt & base check if sensor broken.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t hal_test_sensor_broken(void)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);

        if (g_enum_fingers == 0 || g_sensor_broken_check_mode == 1) {
            LOG_E(LOG_TAG, "[%s] g_enum_fingers = 0 or g_sensor_broken_check_mode =1,full check", __func__);
            cmd->temp_check_level = FULL_TEMPERATURE_CHECK;
        }
        err = gf_hal_test_invoke_command(GF_CMD_TEST_SENSOR_BROKEN, cmd, size);
        g_sensor_broken_err = err;
    } while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}
gf_error_t gf_test_sensor_broken(void)
{
    gf_error_t err = GF_SUCCESS;
	int64_t cur_time = gf_hal_current_time_second();
    int32_t time_delta = 0;
    int retry_times = 2;
    int broken_error_count = 0;

    do
    {
        time_delta = (int32_t)llabs(cur_time - g_last_detect_time);
        if (time_delta < 2)
        {
            LOG_E(LOG_TAG, "[%s] continue sensor broken check return", __func__);
            break;
        }

        if (g_sensor_disable)
        {
            LOG_E(LOG_TAG, "[%s] sensor is broken and disabled already!", __func__);
            break;
        }

        if (g_sensor_power_down)
        {
            gf_hal_power_reset();
        }

        for (int i = 0; i < retry_times; i++)
        {
            err = hal_test_sensor_broken();
            g_last_detect_time = gf_hal_current_time_second();

            LOG_D(LOG_TAG, "[%s] hal_test_sensor_broken err %d!", __func__, err);
            if (err == GF_SUCCESS) {
                if((g_sensor_broken_count > 0 && broken_error_count == 0) ||
                    (broken_error_count > 0)){
                    fingerprint_auth_dcsmsg_t auth_context;
                    memset(&auth_context, 0, sizeof(fingerprint_auth_dcsmsg_t));
                    auth_context.auth_result = 999;
                    auth_context.fail_reason = 2;
                    auth_context.retry_times = broken_error_count;
                    auth_context.preprocess_time= g_sensor_broken_count*retry_times+i-broken_error_count;
                    gf_hal_notify_send_auth_dcsmsg(auth_context);

                }else{
                    fingerprint_auth_dcsmsg_t auth_context;
                    memset(&auth_context, 0, sizeof(fingerprint_auth_dcsmsg_t));
                    auth_context.auth_result = 999;
                    auth_context.fail_reason = 0;
                    auth_context.retry_times = 0;
                    auth_context.preprocess_time= 0;
                    gf_hal_notify_send_auth_dcsmsg(auth_context);
                }
                g_sensor_broken_count = 0;
                g_sensor_disable = 0;
                break;
            }
            if (err == GF_ERROR_SENSOR_IS_BROKEN)
                broken_error_count ++;
            if (i < retry_times - 1) {
                usleep(30000);
                gf_hal_power_reset();
            }
        }
        if ((err == GF_ERROR_SENSOR_IS_BROKEN) || (err == GF_ERROR_SENSOR_NOT_AVAILABLE)||(err == GF_ERROR_SPI_RAW_DATA_CRC_FAILED)) {
            LOG_E(LOG_TAG, "[%s] sensor's temp is rising, not available!", __func__);
            gf_hal_disable_power();
            if (err == GF_ERROR_SENSOR_NOT_AVAILABLE || err == GF_ERROR_SPI_RAW_DATA_CRC_FAILED)
                g_sensor_broken_count ++;
            if ((err == GF_ERROR_SENSOR_IS_BROKEN) || (g_sensor_broken_count >= GF_SENSOR_BROKEN_CNT_MAX)){
                g_sensor_disable = 1;

                fingerprint_auth_dcsmsg_t auth_context;
                memset(&auth_context, 0, sizeof(fingerprint_auth_dcsmsg_t));
                auth_context.auth_result = 999;
                auth_context.fail_reason = 1;
                if (err == GF_ERROR_SENSOR_IS_BROKEN){
                auth_context.retry_times = retry_times;
                auth_context.preprocess_time= g_sensor_broken_count*retry_times;
                }
                if (err == GF_ERROR_SENSOR_NOT_AVAILABLE){
                auth_context.retry_times = 0;
                auth_context.preprocess_time = GF_SENSOR_BROKEN_CNT_MAX*retry_times;
                }
                gf_hal_notify_send_auth_dcsmsg(auth_context);
            }
        }

    } while (0);

    FUNC_EXIT(err);
    return err;
}
