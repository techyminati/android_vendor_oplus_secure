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

typedef struct gf_shenzhen_config
{
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
    uint32_t correct_valid_image_quality_threshold;
    uint32_t correct_valid_image_area_threshold;
    uint32_t cache_recon_num;
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
    uint32_t customize_fake_detect_switch;
// add for distinguish customer's display behaviour
// screen_type:  0x01 for soft screen, 0x02 for hard screen
// light color:  0x01 for green light, 
// illumination_mode :  0x01 for uniform, 0x02 for gradual 

    uint32_t screen_type;
    uint32_t light_color;
    uint32_t illumination_mode;
    uint32_t enable_dsp;
    // add for choose different project, default is 1
    // 1 for AD067, 2 for BD187, 0 for others
    uint32_t project_choose;
    uint32_t enable_dynamic_expo_time;
    uint32_t support_factory_test_dump_raw_data;
} gf_sz_config_t;

#endif /* _GF_SZ_CONFIG_H_ */
