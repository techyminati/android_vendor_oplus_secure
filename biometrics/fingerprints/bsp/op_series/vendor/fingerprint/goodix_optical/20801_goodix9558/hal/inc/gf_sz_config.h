/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_SZ_CONFIG_H_
#define _GF_SZ_CONFIG_H_

#include "gf_config_type.h"

typedef struct GF_SHZENZHEN_CONFIG {
    gf_config_t common;
    uint32_t screen_on_authenticate_fail_retry_count;
    uint32_t screen_off_authenticate_fail_retry_count;
    uint32_t screen_on_valid_touch_frame_threshold;
    uint32_t screen_off_valid_touch_frame_threshold;
    uint32_t image_quality_threshold_for_mistake_touch;
    uint32_t auth_continuous_sampling_number;
    uint32_t enroll_continuous_sampling_number;
    uint32_t enable_force_study;
    uint32_t enable_retry_study;
    uint32_t enable_fake_detect;
    uint32_t enable_residual_finger;
    uint32_t sum_frames;
    uint32_t enable_28k_per_package_first_auth;
    uint32_t enable_partial;
    uint32_t enable_logo_judge;
    uint16_t logo_x_start;
    uint16_t logo_y_start;
    uint16_t logo_row_len;
    uint16_t logo_col_len;
    uint16_t logo_max_judge_num;
    uint32_t nAlgKey;
    uint32_t correct_valid_image_quality_threshold;
    uint32_t correct_valid_image_area_threshold;
    int32_t fake_signala_th;
    int32_t fake_signalb_th;
    int32_t fake_signalc_th;
    int32_t fake_cover_th;
    int32_t fake_diff_th_a;
    int32_t fake_diff_th_b;
    int32_t rotation_type;
    int32_t partial_touch_th;
    double fakea_th;
    double fakeb_th;
    double fakea_b_th;
    double fakeb_b_th;
    int32_t edge_len;
    int32_t slight_th;
    int32_t ptype;
    int32_t study_disable;
    uint32_t black_base_thd;
    int32_t sig_dirty_thd;
    int32_t sig_dry_thd;
    uint32_t screen_type;
    uint32_t light_color;
    uint32_t illumination_mode;
    uint32_t enable_set_expo_with_capture_cmd;
    uint32_t report_wet_finger;
    uint16_t gain;
    uint32_t saveLevel;
    uint32_t show_enroll_and_authenticate_bmp_in_apk;
    uint32_t enable_fake_detect_for_enroll;
    uint16_t enroll_control_begin;
    uint16_t enroll_control_end;
    uint16_t enroll_control_times_per_tmp;
    uint16_t enable_rpmb;
    uint32_t enable_algo_logo_judge;
    uint32_t enroll_control_duplicate_judge_type;
    uint32_t disable_cf_in_fake_detect;
    uint32_t fake_serial_type;
    uint32_t die;
    uint32_t customer_number;
    uint32_t g2_g24_solve_main_frequency_offset;
    int32_t  nSpeedupMode;  // just for G3
    int32_t oem_unlock_state;
    uint32_t support_factory_test_dump_raw_data;
} gf_sz_config_t;

#endif /* _GF_SZ_CONFIG_H_ */
