/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_DELMAR_CONFIG_H_
#define _GF_DELMAR_CONFIG_H_

#include "gf_config_type.h"

#define DELMAR_OTP_CP1     (1)
#define DELMAR_OTP_CP2     (1<<1)
#define DELMAR_OTP_FT      (1<<2)
#define DELMAR_OTP_SMT1    (1<<3)
#define DELMAR_OTP_SMT2    (1<<4)
#define DELMAR_OTP_MT      (1<<5)
#define DELMAR_OTP_FULL    (0xFF)

typedef struct gf_delmar_config {
    gf_config_t common;
    /* retry study, weak template save threshold */
    uint32_t wake_template_update_save_threshold;  // deprecated
    uint32_t preprocess_valid_area_threshold;
    uint64_t duplicate_area_judge_area_mask;
    uint32_t duplicate_area_judge_max_times;
    uint32_t duplicate_area_judge_interval;
    uint32_t max_continuous_duplicate_area_times;
    uint16_t support_image_segmentation;
    uint32_t pressure_threshold;
    int16_t seg_threshold;
    int16_t open_auto_calibration;
    int16_t flag_raw_subtract_dark;
    int16_t exposure_short_time;
    int16_t exposure_normal_time;
    int16_t openshort_exposure_short_time;
    int16_t openshort_exposure_normal_time;
    uint16_t openshort_diff;
    uint16_t openshort_os;
    int16_t sampling_interval;
    int16_t calibrate_with_black_rubber;
    uint8_t calibrate_mode;
    uint8_t auth_continuous_sampling_number;
    uint8_t enroll_continuous_sampling_number;
    uint8_t others_continuous_sampling_number;
    uint8_t support_moire_filter;
    uint8_t support_anti_foreign_body;
    uint8_t residual_fingerprint_enable;
    uint8_t continuous_sampling_number;
    uint8_t support_image_overlay_check;
    uint8_t support_retry_study;  // deprecated
    uint8_t enroll_recapture_count;
    uint8_t shift_press_enroll_base_count;
    uint8_t support_continue_enroll_captured_image;
    uint8_t support_rpmb;   // only qsee support rpmb
    uint8_t force_write_cali_data_to_file;  // some vendor need to write the cali to file instead of flash
    /* use default cali data for find sensor only, please disable it in release codes */
    uint8_t disable_default_cali_data;
    uint8_t support_fake_detect;
    uint8_t installation_sensor_mode;
    uint8_t first_frame_speed_up_mode;
    uint8_t enable_two_round_recognition;
    uint8_t enable_cache_inverse_match;
    uint8_t enable_get_moire_coordinate;
    uint16_t sensor_width_pixel;  // mm(with darkpixel) * PixelsPerInch at x direction / 25.4
    uint16_t sensor_height_pixel;  // mm(with darkpixel) * PixelsPerInch at y direction / 25.4
    uint8_t support_long_exposure;
    uint16_t calc_gain_target_value;
    uint8_t use_full_mask_for_single_chip;
    uint32_t fake_device_type;
    uint32_t fake_screen_type;
    int32_t safe_class;
    int16_t estimate_moire_angle;
    uint8_t preprocess_sigma;
    uint8_t decrease_gain_retry_if_highlight;
    uint8_t decrease_gain_fraction;
    uint32_t anomaly_enroll_number;
    uint8_t allow_enrolling_anomaly_template;
    uint8_t early_warning_counts_by_anomaly;
    uint8_t support_enroll_by_press_threshold;
    uint16_t grain_line_fill_type;
    int32_t residual_safe_level;
    int32_t anomaly_auth_level;
    int32_t anomaly_enroll_level;
    int32_t anomaly_study_level;
    int32_t anomaly_detect_mode;
    uint8_t support_independent_fast_auth;
    uint8_t anti_peep_detect;
    uint8_t support_enhanced_fast_auth;
    uint8_t support_unlocked_device;  // 1:support file-system takes data from backup area when device unlocked
    uint8_t support_retry_highlight_base;
    uint8_t support_otp_update_on_calibration;  // 1:support update, 0: not support update
    uint16_t moire_filter_config;
    float exp_time;
    float lexp_time;
    uint8_t highlight_auth_retry_count;
    uint8_t support_check_before_kb_cali;  // 1:support check foolproof when do kb cali, 0:not support
    uint8_t support_adjust_anomaly_level;
    uint8_t antipeeping_detect_max_times;
    uint32_t antipeeping_image_quality_threshold;
    uint8_t max_moire_num;
    uint16_t burlap_adjust_gain;
    uint8_t support_enroll_fake_check;
} gf_delmar_config_t;

/*
* byte 3 reserved
* byte 2 mcu tyoe 0x00:no mcu 0x01:mcu type gm182
* byte 1 chip version, range(0x00-0xFF)
* byte 0 chip sub version, range(0x01-0xFF)
*/
typedef enum {
    DELMAR_SENSOR_TYPE_UNKOWN = 0,

    DELMAR_SENSOR_TYPE_GM182 = 0x00010001,
    DELMAR_SENSOR_TYPE_SINGLE_T_SE1 = 0x00000001,
    DELMAR_SENSOR_TYPE_SINGLE_T_SE2 = 0x00000002,
    DELMAR_SENSOR_TYPE_SINGLE_T_SE5 = 0x00000003,
    DELMAR_SENSOR_TYPE_SINGLE_T = DELMAR_SENSOR_TYPE_SINGLE_T_SE1,
    DELMAR_SENSOR_TYPE_SINGLE_CS_SE1 = 0x00000101,
    DELMAR_SENSOR_TYPE_SINGLE_CS_SE2 = 0x00000102,
    DELMAR_SENSOR_TYPE_SINGLE_CS = DELMAR_SENSOR_TYPE_SINGLE_CS_SE1,
    DELMAR_SENSOR_TYPE_GM182_T_SE1 = DELMAR_SENSOR_TYPE_GM182,
    DELMAR_SENSOR_TYPE_MAX
} gf_delmar_sensor_type_t;

void gf_config_get_sensor_type(gf_delmar_sensor_type_t *sensor_type);
#endif /* _GF_DELMAR_CONFIG_H_ */
