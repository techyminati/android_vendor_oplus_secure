/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: Dump big data
 * History: None
 * Version: 1.0
 */


#ifndef _GF_DUMP_BIGDATA_H_
#define _GF_DUMP_BIGDATA_H_

#include "gf_error.h"
#include "gf_common.h"
#include "gf_user_type_define.h"
#include "gf_dump_data_encoder.h"

/**
 * milan_f_series bigdata dump device info
 */

#define MILAN_CONFIG_CONTENT_MAX_LEN 524

typedef struct
{
    uint16_t start_address;  // the start register address of this mode's configuration
    uint16_t data_len;  // the length in bytes of this configuration byte array.
    uint8_t data[MILAN_CONFIG_CONTENT_MAX_LEN];  // the pointer to the detail configuration content.
} milan_chip_config_unit_t;

typedef struct  // Define the structure for different mode' configuration array.
{
    milan_chip_config_unit_t ff;
    milan_chip_config_unit_t fdt_manual_down;
    milan_chip_config_unit_t fdt_down;
    milan_chip_config_unit_t fdt_up;
    milan_chip_config_unit_t image;
    milan_chip_config_unit_t nav_fdt_down;
    milan_chip_config_unit_t nav_fdt_up;
    milan_chip_config_unit_t nav;
    milan_chip_config_unit_t nav_base;
} milan_chip_config_t;

typedef struct gf_dev_info
{
    gf_cmd_header_t cmd_header;
    char android_version[GF_BIGDATA_ANDROID_VERSION_LEN];
    char service_version[GF_BIGDATA_SERVICE_VERSION_LEN];
    char platform[GF_BIGDATA_PLATFORM_LEN];
    char algo_version[GF_BIGDATA_ALGO_VERSION_LEN];
    char preprocess_version[GF_BIGDATA_ALGO_VERSION_LEN];
    char tee_version[GF_BIGDATA_TEE_VERSION_LEN];
    char ta_version[GF_BIGDATA_TA_VERSION_LEN];
    char chip_id[GF_BIGDATA_CHIP_ID_LEN];
    char vendor_id[GF_BIGDATA_VENDOR_ID_LEN];
    char sensor_id[GF_BIGDATA_SENSOR_ID_LEN * 2];
    char otp_info[GF_BIGDATA_SENSOR_OTP_INFO_LEN];
    char production_date[GF_BIGDATA_PRODUCTION_DATE_LEN];
    char module_version[GF_BIGDATA_MODULE_VERSION_LEN];
    uint32_t row;
    uint32_t col;
    uint32_t nav_row;
    uint32_t nav_col;
    uint16_t orientation;
    uint16_t facing;
    milan_chip_config_t g_milan_config;
} gf_dev_info_t;

/**
 * milan_xxx_series bigdata dump device info
 * ...
 * add there
 */


typedef enum
{
    ENROLL = 0,
    AUTHENTICATE,
    NAVIGATION,
    FINGER_BASE,
    NAV_BASE,
    DEVICE,
    HBD,
    GSC,
    TEMPLATE,
    INVALID,
    MAX
} gf_bigdata_type_t;

typedef struct gf_bigdata_scene
{
    uint32_t index;
    gf_bigdata_type_t type;
    int32_t scene;  // application scene, reserved
    int32_t charge;  // charge status, 0:no charging, 1:charging
    int32_t screen_status;  // screen status, 0:off, 1:on
    int32_t lock_status;  // lock status, 0:unlocked, 1:locked
} gf_bigdata_scene_t;

typedef struct gf_bigdata_basic_json
{
    gf_bigdata_scene_t scene_info;
    int32_t reserved;
} gf_bigdata_basic_json_t;

typedef struct gf_bigdata_device_info
{
    gf_bigdata_scene_t scene_info;
    char android_version[GF_BIGDATA_ANDROID_VERSION_LEN];
    char service_version[GF_BIGDATA_SERVICE_VERSION_LEN];
    char platform[GF_BIGDATA_PLATFORM_LEN];
    char algo_version[GF_BIGDATA_ALGO_VERSION_LEN];
    char preprocess_version[GF_BIGDATA_ALGO_VERSION_LEN];
    char fw_version[GF_BIGDATA_FW_VERSION_LEN];
    char tee_version[GF_BIGDATA_TEE_VERSION_LEN];
    char ta_version[GF_BIGDATA_TA_VERSION_LEN];
    char config_version[GF_BIGDATA_CONFIG_VERSION_LEN];
    char chip_id[GF_BIGDATA_CHIP_ID_LEN];
    char vendor_id[GF_BIGDATA_VENDOR_ID_LEN];
    char sensor_id[GF_BIGDATA_SENSOR_ID_LEN * 2];
    char otp_version[GF_BIGDATA_SENSOR_OTP_VERSION_LEN];
    char otp_chipid[GF_BIGDATA_SENSOR_OTP_CHIPID_LEN];
    char otp_info[GF_BIGDATA_SENSOR_OTP_INFO_LEN];
    char production_date[GF_BIGDATA_PRODUCTION_DATE_LEN];
    char module_version[GF_BIGDATA_MODULE_VERSION_LEN];
    uint32_t row;
    uint32_t col;
    uint32_t nav_row;
    uint32_t nav_col;
    uint16_t orientation;
    uint16_t facing;
    char cfg_data[GF_BIGDATA_CFG_DATA_LEN];
    milan_chip_config_t g_milan_config;
} gf_bigdata_device_info_t;

typedef struct gf_bigdata_image
{
    gf_bigdata_scene_t scene_info;

    uint64_t timestamp_keydown;
    uint32_t get_gsc_data_time;
    uint32_t get_raw_data_time;
    uint32_t broken_check_time;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t enroll_time;
    uint32_t authenticate_time;
    uint32_t bio_assay_time;
    uint32_t error_code;
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t template_count;
    uint32_t key_point_num;
    uint32_t enrolling_finger_id;
    uint32_t overlay;
    uint32_t increase_rate_between_stitch_info;
    uint32_t overlap_rate_between_last_template;
    uint32_t duplicated_finger_id;
    uint32_t increase_rate;
    uint32_t match_score;
    uint32_t match_finger_id;
    uint32_t study_flag;
    uint32_t authenticate_update_flag;
    uint32_t try_count;
    uint32_t is_final;
    uint32_t frame_num;
    uint32_t select_index;
    uint32_t image_origin_data_len;
    uint32_t image_raw_data_len;
    uint32_t gsc_untouch_data_len;
    uint32_t gsc_touch_data_len;
    uint32_t bad_point_num;
    uint32_t fp_rawdata_max;
    uint32_t fp_rawdata_min;
    uint32_t fp_rawdata_average;
    uint32_t gsc_rawdata_max;
    uint32_t gsc_rawdata_min;
    uint32_t gsc_rawdata_average;
} gf_bigdata_image_t;

typedef struct gf_bigdata_nav
{
    gf_bigdata_scene_t scene_info;
    uint32_t nav_code;
    gf_nav_config_t nav_config;
    uint64_t timestamp_keydown;
    uint8_t frame_num;
    uint32_t nav_origin_data_len;
    uint32_t nav_raw_data_len;
    uint32_t nav_times;
    uint32_t nav_frame_index;
    uint32_t nav_frame_count;
} gf_bigdata_nav_t;

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus
gf_error_t gf_bigdata_dump_template_info(dump_data_encoder_t* data_encoder, uint8_t* fileName);

gf_error_t gf_bigdata_dump_device_info(dump_data_encoder_t* data_encoder,
                                       uint8_t* dir,
                                       gf_dev_info_t* dev_info,
                                       uint8_t* cur_time_str);

gf_error_t gf_bigdata_dump_image_data(dump_data_encoder_t* data_encoder, const uint8_t* filepath,
                                      gf_dump_data_t* dump_data,
                                      gf_operation_type_t operation, gf_error_t error_code);
gf_error_t gf_bigdata_dump_nav_data(dump_data_encoder_t* data_encoder, const uint8_t* filepath,
                                    gf_dump_data_t* dump_data,
                                    gf_operation_type_t operation);
gf_error_t gf_bigdata_dump_basic_json_data(dump_data_encoder_t* data_encoder,
                                           const uint8_t* filepath,
                                           gf_dump_data_t* dump_data,
                                           gf_operation_type_t operation);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_DUMP_BIGDATA_H_
