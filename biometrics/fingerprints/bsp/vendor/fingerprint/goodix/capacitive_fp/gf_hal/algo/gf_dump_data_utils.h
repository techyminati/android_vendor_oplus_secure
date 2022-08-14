/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: Basic util functions for dump data
 * History: None
 * Version: 1.0
 */
#ifndef _GF_DUMP_DATA_UTILS_H_
#define _GF_DUMP_DATA_UTILS_H_

#include <sys/time.h>
#include "gf_common.h"
#include "gf_dump_config.h"
#include "gf_dump_data_encoder.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#define GF_DUMP_RESULT_STR_MAX_LEN (30)

#define GF_DUMP_DATA_ROOT_PATH "/data/vendor/fingerprint"
#define GF_DUMP_FINGER_BASE_DIR "/gf_data/base/finger_base/"
#define GF_DUMP_BOOT_CALI_BASE_DIR "/gf_data/base/"
#define GF_DUMP_CALIBRATION_DATA_DIR "/gf_data/calibration_data_frames/"
#define GF_DUMP_NAV_BASE_DIR "/gf_data/base/nav_base/"
#define GF_DUMP_NAV_DIR "/gf_data/navigation/"
#define GF_DUMP_NAV_ROWDATA "rawdata/"
#define GF_DUMP_NAV_ENHANCE "enhance/"
#define GF_DUMP_ENROLL_DIR "/gf_data/enroll/"
#define GF_DUMP_AUTHENTICATE_DIR "/gf_data/authenticate/"
#define GF_DUMP_TEST_PIXEL_OPEN_DIR "/gf_data/test_sensor/"
#define GF_DUMP_TEST_PIXEL_SHORT_STREAK_DIR "/gf_data/test_short/"
#define GF_DUMP_TEST_SENSOR_FINE_DIR "/gf_data/test_sensor_fine/"
#define GF_DUMP_CONSISTENCY_TEST_DIR "/gf_data/consistency/"
#define GF_DUMP_TEST_UNTRUSTED_ENROLL_DIR "/gf_data/untrusted_enroll/"
#define GF_DUMP_TEST_UNTRUSTED_AUTHENTICATE_DIR "/gf_data/untrusted_authenticate/"
#define GF_DUMP_GSC_DIR "/gf_data/gsc/"
#define GF_DUMP_TEMPLATE_DIR "/gf_data/templates/"
#define GF_DUMP_TEST_PERFORMANCE_DIR "/gf_data/test_performance/"
#define GF_DUMP_MEM_MANAGER_POOL_DIR "/gf_data/memmgr_pool/"
#define GF_DUMP_DEV_INFO "/gf_data/device/"
#define GF_DUMP_CALIBRATION_PARA_RETEST_DIR "/gf_data/calibration_para_retest/"
#define GF_DUMP_SNR_TEST_PATH "/gf_data/snr_test/"

#define BREAK_IF_ERROR(err) \
    {   \
        if ((err) != GF_SUCCESS)  \
        {                          \
            LOG_E(LOG_TAG, "[%s] break, err<%d>", __func__, (err));  \
            break;  \
        }  \
    }

void gf_init_dump_config(uint32_t sensor_width, uint32_t sensor_height,
                                   uint32_t nav_width, uint32_t nav_height,
                                   gf_chip_type_t chip_type, gf_chip_series_t chip_series);

uint32_t gf_get_sensor_width();
uint32_t gf_get_sensor_height();
uint32_t gf_get_nav_width();
uint32_t gf_get_nav_height();
uint8_t* gf_get_boot_time_str();
gf_chip_type_t gf_get_chip_type();
gf_chip_series_t gf_get_chip_series();
uint8_t* gf_get_dump_version();
uint8_t* gf_get_dump_bigdata_version();

/**
 * @brief: If dump is configured disabled, then there's no way to enable dump!
 *         If dump is configured enabled, then at runtime, we have two ways to switch dump on/off,
 *         GF_Test.apk and "adb shell setprop gf.debug.dump_data 1". If there's one way switched dump off
 *         then dump is disabled. Otherwise dump is enabled.
 */
void gf_enable_dump();
void gf_disable_dump();
uint8_t gf_is_dump_enabled();


uint8_t gf_is_dump_encrypt_enabled();
uint16_t gf_get_dump_encryptor_version();

uint8_t gf_is_dump_bigdata_enabled();


/**
 * @brief: set/get the dump root path
 */
void gf_set_dump_path(gf_dump_path_t path);
gf_dump_path_t gf_get_dump_path();
uint8_t* gf_get_dump_root_dir();

/**
 * @brief: check if the given op/op_result/op_data dump is allowed.
 * @return : 1, allowed
 *           0, not allowed
 */
uint8_t gf_is_operation_result_dump_allowed(gf_operation_type_t operation,
                                                gf_error_t op_err_code);
uint8_t gf_is_operation_data_dump_allowed(gf_operation_type_t operation,
                                                gf_dump_operation_data_type_t op_data);

uint8_t gf_is_dump_template_allowed();
uint8_t gf_is_dump_device_info_allowed();


/**
 * @brief: get_current_time_string yyyy-mm-dd-hh-MM-ss-uuuuuu or yyyy-mm-dd-hh-MM-ss
 * @param: str_buf, caller must make sure it's size is >= GF_DUMP_FILE_PATH_MAX_LEN
 * @param: with_micro_sec, return string with micro seconds, 1 with, 0 without
 *
 */
void gf_get_time_string(struct timeval* tv, uint8_t* str_buf, uint8_t with_micro_sec);
int64_t gf_get_time_stamp(struct timeval* tv);

void gf_get_operation_result_str(gf_dump_data_t *dump_data,
                                     gf_error_t error_code,
                                     uint8_t* result,
                                     uint32_t result_buf_len);

/**
 * @brief: tool function for dump 16bits raw data
 * @param: data_num, 16bits data num. the memory size for data is 2*data_num
 *
 */
gf_error_t gf_dump_raw_data(dump_data_encoder_t* data_encoder,
                         const uint8_t* file_path,
                         const uint16_t* data,
                         uint32_t data_num,
                         data_type_t data_type);

gf_error_t gf_dump_raw_data_minus(dump_data_encoder_t* data_encoder,
                         const uint8_t* file_path,
                         const uint16_t* data,
                         uint32_t data_num,
                         data_type_t data_type);
/**
 * @brief: handle dump buffer, write to internal storage or notify apk
 */
gf_error_t gf_handle_dump_buf(dump_data_encoder_t* data_encoder);

void gf_set_dump_config(gf_dump_config_t* cfg);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_DUMP_DATA_UTILS_H_

