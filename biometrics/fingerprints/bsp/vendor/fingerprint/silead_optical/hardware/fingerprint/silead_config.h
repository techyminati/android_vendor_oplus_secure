/******************************************************************************
 * @file   silead_config.h
 * @brief  Contains Chip configurations header file.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Martin Wu  2018/4/2    0.1.0      Init version
 * Martin Wu  2018/5/7    0.1.1      Support 6150 related configurations.
 * Martin Wu  2018/5/9    0.1.2      Add 6150 deadpix test param
 * Martin Wu  2018/5/10   0.1.3      Add 6150 nav base param
 * Martin Wu  2018/5/11   0.1.4      Add 6150 auto adjust param
 * Martin Wu  2018/5/14   0.1.5      Add finger detect loop mode
 * Martin Wu  2018/6/8    0.1.6      Add ESD related param
 * Martin Wu  2018/6/16   0.1.7      Add OTP related param
 * Martin Wu  2018/6/18   0.1.8      Add cut_size param
 * Martin Wu  2018/6/25   0.1.9      Add optical base & SNR param
 * Martin Wu  2018/6/25   0.2.0      Add optical factory test param
 * Martin Wu  2018/6/26   0.2.1      Add optical middle tone base param
 * Martin Wu  2018/6/27   0.2.2      Add SPI check 0xBF front porch.
 * Martin Wu  2018/6/30   0.2.3      Add distortion & finger_num param.
 * Martin Wu  2018/7/4    0.2.4      Add AEC param.
 * Martin Wu  2018/7/6    0.2.5      Add dead pixel radius.
 * Martin Wu  2018/7/14   0.2.6      Add Auth/Enroll capture image param.
 * Martin Wu  2018/7/20   0.2.7      Add postprocess normalize param.
 * Martin Wu  2018/7/23   0.2.8      Add postprocess remove deadpx param.
 * Martin Wu  2018/7/25   0.2.9      Add R9O03 spd param.
 * Martin Wu  2018/7/30   0.3.0      Add enroll/auth quality param.
 * Martin Wu  2018/8/1    0.3.1      Add optic finger detect param.
 * Martin Wu  2018/8/4    0.3.2      Add post-enroll control param.
 * Martin Wu  2018/8/11   0.3.3      Add icon detect param.
 * Martin Wu  2018/8/26   0.3.4      Add dual auth base param.
 * Martin Wu  2018/8/28   0.3.5      Add dual templates param.
 * Martin Wu  2018/9/5    0.3.6      Add remove slope param.
 * Martin Wu  2018/9/19   0.3.7      Add optic mask param.
 * Martin Wu  2018/9/19   0.3.8      Add optic clip param.
 * Martin Wu  2018/9/27   0.4.0      Universalize capacitor and optics param.
 * Martin Wu  2018/10/18  0.4.1      Add dry/wet template param.
 * Martin Wu  2018/10/28  0.4.2      Combine optic/capacitor param.
 * Martin Wu  2018/10/29  0.4.3      Add dry2wet update threshold param.
 * Martin Wu  2018/11/5   0.4.4      Add artifact detect param.
 * Martin Wu  2018/11/7   0.4.5      Add config file version.
 * Martin Wu  2018/11/15  0.4.6      Add Shading & P Value param.
 * Martin Wu  2018/11/27  0.4.7      Add fingerprint fake param.
 * Martin Wu  2018/12/5   0.4.8      Add factory test black_signal/fov_test/circle_test param.
 * Martin Wu  2018/12/7   0.4.9      Add check same finger mask.
 *
 *****************************************************************************/

#ifndef __SILEAD_FP_CONFIG_H__
#define __SILEAD_FP_CONFIG_H__

#define SILEAD_CONFIG_VER  (150)

typedef struct __attribute__ ((packed)) _sl_dev_ver {
    uint32_t id;
    uint32_t sid;
    uint32_t vid;
} cf_dev_ver_t;

typedef struct __attribute__ ((packed)) _sl_dev_list {
    cf_dev_ver_t *ver;
    uint32_t len;
    uint32_t updated;
} cf_dev_list_t;

typedef struct __attribute__ ((packed)) _sl_register {
    uint32_t addr;
    uint32_t val;
} cf_register_t;

typedef struct __attribute__ ((packed)) _sl_config {
    cf_register_t *reg;
    uint32_t len;
    uint32_t updated;
} cf_mode_config_t;

typedef enum {
    CFG_NORMAL = 0,
    CFG_FG_DETECT,
    CFG_PARTIAL,
    CFG_FULL,
    CFG_FRM_START,
    CFG_DS_START,
    CFG_STOP,
    CFG_DOWN_INT,
    CFG_VKEY_INT,
    CFG_UP_INT,
    CFG_NAV,
    CFG_NAV_HG,
    CFG_NAV_DS,
    CFG_NAV_HG_DS,
    CFG_SNR,
    CFG_DEADPX1,
    CFG_DEADPX2,
    CFG_DEADPX3,
    CFG_HG_COVERAGE,
    CFG_HWAGC_READ,
    CFG_HWCOV_START,
    CFG_HWAGC_START,
    CFG_BASE_ORI,
    CFG_AUTO_ADJUST,
    CFG_MAX,
} e_mode_config_t;

typedef struct __attribute__ ((packed)) _sl_spi_cfg {
    uint16_t ms_frm; /* wait ms before read frame */
    uint8_t retry;  /* wait 0xBF retry times. */
    uint8_t reinit; /* re-init chip after receive IRQ, some product can't auto wakeup after receive IRQ */
    uint16_t start; /* wait ms before start check 0xBF */
    uint8_t msb;    /* most significant bit offset. for 16bit version. */
    uint32_t updated;
} cf_spi_t;

typedef struct __attribute__ ((packed)) _sl_pb_threshold {
    int8_t alg_select;
    int8_t enrolNum;  /* enroll numbers */
    int16_t max_templates_num;
    int32_t templates_size; /* 模板的最大尺寸 bytes,不能设置为0, 500K  */
    int32_t identify_far_threshold;
    int32_t update_far_threshold;
    uint16_t enroll_quality_threshold;
    uint16_t enroll_coverage_threshold;
    uint8_t quality_threshold;
    uint8_t coverage_threshold;
    uint16_t skin_threshold;
    uint16_t artificial_threshold;
    uint16_t samearea_detect;//default 1
    uint16_t samearea_threshold;//same with identify_far_threshold
    uint16_t samearea_dist;//default 43
    uint16_t samearea_start;//default 3
    uint16_t samearea_check_once_num;//default 1
    uint16_t samearea_check_num_total;//default 6
    uint16_t dy_fast;//default 3 template dynamic update stategy
    uint32_t segment;//default 0
    uint32_t water_finger_detect;
    uint32_t shake_coe;
    uint32_t noise_coe;
    uint32_t gray_prec;
    uint32_t water_detect_threshold;
    uint16_t fail_threshold;
    uint8_t  spd_flag;
    uint16_t samefinger_threshold;
    uint16_t identify_epay_threshold;
    uint16_t force_up_threshold;
    uint32_t samearea_mask;
    uint32_t updated;
} cf_algo_param_t;

typedef enum {
    CFG_PB_PARAM_FINETUNE = 0,
    CFG_PB_PARAM_NAVI,
    CFG_PB_PARAM_COVER,
    CFG_PB_PARAM_BASE,
    CFG_PB_PARAM_REDUCE_NOISE,
    CFB_PB_PARAM_OPT_PARMC,
    CFB_PB_PARAM_OPT_FD,
    CFG_PB_PARAM_MAX,
} e_pb_param_t;

typedef struct __attribute__ ((packed)) _sl_pb_param {
    int32_t *val;
    uint32_t len;
    uint32_t updated;
} cf_pb_param_t;

typedef struct __attribute__ ((packed)) _sl_pb_agc {
    uint8_t skip_fd; // Skip finger detect
    uint8_t fd_threshold; // Finger detect threshold
    uint8_t skip_small; // SKip partial finger AGC
    uint8_t max; // AGC maxinum trys.
    uint8_t max_small; // AGC maxinum trys for SMALL
    uint8_t hwagc_enable;
    uint16_t hwcov_wake;
    uint16_t hwcov_tune;
    uint16_t exp_size;
    uint32_t updated;
} cf_pb_agc_t;

typedef struct __attribute__ ((packed)) _sl_pb_cfg {
    cf_pb_param_t param[CFG_PB_PARAM_MAX];
    cf_pb_agc_t agc;
    cf_algo_param_t threshold;
} cf_pb_config_t;

typedef struct __attribute__ ((packed)) _sl_gain {
    uint32_t v0c;
    uint32_t v20;
    uint32_t v2c;
    uint32_t v24;
    uint32_t updated;
} cf_gain_t;

typedef struct __attribute__ ((packed)) _sl_gain_reg {
    uint32_t reg0c;
    uint32_t reg20;
    uint32_t reg2c;
    uint32_t reg24;
    uint32_t updated;
} cf_gain_reg_t;

typedef enum {
    CFG_NAV_AGC_MODE_TUNE = 0,
    CFG_NAV_AGC_MODE_HG,
    CFG_NAV_AGC_MODE_DS,
    CFG_NAV_AGC_MODE_HG_DS,
    CFG_NAV_AGC_MODE_MAX,
} e_nav_type_t;

typedef struct __attribute__ ((packed)) _sl_nav {
    uint8_t enable; /* Enabled or disable */
    uint8_t mode; /* Navi mode */
    uint8_t type; /* Navi type, e_nav_type_t */
    uint8_t con_frame_get_num; /* continue frame count */
    uint32_t w;  /* Navi read width, pixel */
    uint32_t h;  /* Navi read height pixel */
    uint32_t wh; /* HG Navi read width, pixel */
    uint32_t hh; /* HG Navi read height, pixel */
    uint32_t w_ds; /* DS Navi read width, pixel */
    uint32_t h_ds; /* DS Navi read height, pixel */
    uint32_t w_hg_ds; /* HG DS Navi read width, pixel */
    uint32_t h_hg_ds; /* HG DS Navi read height, pixel */
    cf_gain_t gain[CFG_NAV_AGC_MODE_MAX];
    uint32_t vk_timeout;  /* 必须上报的超时, default 350ms */
    uint32_t dclick_gap;  /* 双击的判断标准，default 250ms */
    uint32_t longpress;   /* 长按的判断标准，default 400ms */
    uint32_t updated;
} cf_nav_t;

typedef struct __attribute__ ((packed)) _sl_test {
    uint8_t fd_threshold;
    uint8_t deadpx_hard_threshold;
    uint8_t deadpx_norm_threshold;
    uint8_t scut;
    uint16_t detev_ww;
    uint16_t detev_hh;
    uint16_t deteline_h;
    uint16_t deteline_w;
    uint8_t deadpx_max;
    uint8_t badline_max;
    uint16_t finger_detect_mode;
    uint32_t deadpx_cut;
    uint32_t updated;
} cf_test_t;

typedef struct __attribute__ ((packed)) _sl_otp {
    uint32_t otp0;
    uint32_t otp1;
    uint32_t otp2;
    uint32_t otp3;
    uint32_t otp4;
    uint32_t otp5;
    uint32_t otp_a0;
    uint32_t updated;
} cf_otp_t;

typedef struct __attribute__ ((packed)) _sl_common {
    uint32_t id;  /* ChipID */
    uint32_t sid; /* Sub ChipID */
    uint32_t vid; /* Viture ID*/
    uint32_t w;   /* Width, pixel */
    uint32_t h;   /* Height pixel */
    uint32_t wp;  /* Partial read width, pixel */
    uint32_t hp;  /* Partial read height pixel */
    uint32_t w_hwagc;
    uint32_t h_hwagc;
    uint32_t wc; /* cut width */
    uint32_t hc; /* cut height */
    uint32_t rw;  /* Read/Raw Width, pixel, if rw != w, need cut the extra column after get frame */
    uint32_t wdpi; /* Row/Width DPI/PPI, default 508 */
    uint32_t hdpi; /* Column/Height DPI/PPI, default 508 */
    uint16_t fg_loop;
    uint16_t ver;
    cf_gain_t gain;
    cf_gain_reg_t gain_reg;
    cf_otp_t otp;
    uint32_t updated;
} cf_common_t;

typedef struct __attribute__ ((packed)) _sl_tp_info {
    uint32_t center_x;
    uint32_t center_y;
    uint32_t b1_distance_threshold;
    uint32_t b2_distance_threshold;
    uint32_t b2_b1_distance_threshold;
    uint32_t c1_coverage_threshold;
    uint32_t c2_coverage_threshold;
    uint32_t updated;
} cf_touch_info_t;

typedef struct __attribute__ ((packed)) _sl_mmi {
    uint16_t dac_min;
    uint16_t dac_max;
    int16_t grey_range_left;
    int16_t grey_range_right;
    uint32_t nav_base_frame_num;
    uint8_t max_tune_time;
    uint8_t auto_adjust_w;
    uint8_t auto_adjust_h;
    uint8_t frm_loop_max;
    uint32_t postprocess_ctl;
    uint8_t white_base_white_thr;
    uint8_t white_base_black_thr;
    uint8_t black_base_white_thr;
    uint8_t black_base_black_thr;
    uint8_t middle_base_white_thr;
    uint8_t middle_base_black_thr;
    uint8_t diff_base_min_thr;
    uint8_t diff_base_max_thr;
    uint32_t snr_cut;
    uint8_t base_size;
    uint8_t snr_img_num;
    uint16_t snr_thr;
    uint8_t distortion;
    uint8_t finger_num;
    uint8_t storage_interval;
    uint8_t sum_type;
    uint8_t deadpx_radius;
    uint8_t cut_radius;
    uint16_t normalize_blk;
    uint32_t normalize_ratio;
    uint32_t fft_ratio;
    cf_touch_info_t touch_info;
    cf_gain_t gain_snr_signal;   //add  0108
    cf_gain_t gain_snr_white;//add  0108
    uint32_t updated;
} cf_mmi_t;

typedef struct __attribute__ ((packed)) _sl_pp {
    uint32_t aec_left;
    uint32_t aec_right;
    uint8_t aec_time;
    uint8_t cal_max_loop;
    uint8_t dead_a;
    uint8_t dead_b;
    uint32_t quality_cut;
    uint16_t quality_thr;
    uint8_t enroll_quality_chk_num;  // the max number to check enroll image quality
    uint8_t enroll_post_num;  // the number which used for post-enroll process
    uint32_t enroll_post_mask;  // the mask indicate the post-enroll number for each step
    uint16_t icon_ratio_z;
    uint16_t icon_ratio_m;
    uint16_t big_blot_thr; // detect black base image big blot threshold
    uint16_t duo_tpl_thr;  // duo templates update threshold
    uint8_t  tpl_upd_leakage_thr; // Leakage threshold for template update
    uint8_t  tp_coverage_default; // TP check fail, coverage value
    uint8_t  slope_len;
    uint8_t  slope_cnt;
    int16_t  slope_h;
    int16_t  slope_w;
    int16_t  cut_angle;
    int16_t  cut_ud;
    int16_t  cut_lr;
    uint8_t  dry_sub_cnt;
    uint8_t  after_verify_cnt;
    uint8_t  wts_threshold;
    uint8_t  wts_up_cnt;
    uint8_t  attack_fail_cnt;
    uint8_t  w2d_verify;
    uint8_t  w2d_update;
    uint32_t updated;
} cf_pp_t;

typedef struct __attribute__ ((packed)) _sl_ft {
    uint8_t line_step_min;
    uint8_t ignore;
    uint16_t min_theta;
    uint16_t max_theta;
    uint16_t quality_thr;
    uint16_t line_distance_min;
    uint16_t line_distance_max;
    uint32_t cut;
    int16_t  mask_min1; // Mask MIN
    int16_t  mask_min2;
    int16_t  mask_min3;
    int16_t  mask_max1; // Mask MAX
    int16_t  mask_max2;
    int16_t  mask_max3;
    int16_t  mask_thr;  // Mask threshold
    int16_t  mask_err_thr;  // Mask error threshold
    int16_t  mask_ex;
    int16_t  bias_thr;
    int16_t  bias_err_thr;
    int16_t  shading_thr;
    int16_t  shading_unit_thr;
    uint8_t  p_gray_thr;
    uint8_t  p_w_b_thr;
    int16_t  p_gray_area_thr;
    int16_t  p_w_b_area_thr;
    int32_t dark_percent_thr;
    int32_t max_diameter_thr;
    int32_t min_diameter_thr;
    int32_t circle_thr;
    int32_t black_signal_thr;
    int32_t mean_diff;     //add  0108
    int32_t r_square_thr;//add  0108
    uint32_t updated;
} cf_ft_t;

typedef struct __attribute__ ((packed)) _sl_esd {
    uint32_t irq_check;
    uint32_t irq_reg;
    uint32_t irq_val;
    uint32_t int_reg;
    uint32_t int_val;
    uint32_t int_beacon;
    uint32_t updated;
} cf_esd_t;

typedef struct __attribute__ ((packed)) _sl_ci {
    uint8_t auth_reverse_skip;
    uint8_t auth_reverse_grey;
    uint8_t enroll_loop;
    uint8_t enroll_skip;
    uint8_t auth_buf_num;
    uint8_t artificial_verify_bias;
    uint8_t artificial_update_bias;
    uint8_t reserved8;
    int32_t fingerprint_spd_score_thr;
    uint32_t alg_ctl;
    int32_t fake_score;
    uint32_t updated;
} cf_ci_t;

typedef struct __attribute__ ((packed)) _sl_config_set {
    cf_dev_list_t dev;
    uint32_t mask[3]; /* ChipID match masks */
    cf_common_t common;
    cf_spi_t spi;
    cf_nav_t nav;
    cf_pb_config_t pb;
    cf_test_t test;
    cf_mmi_t mmi;
    cf_pp_t pp;
    cf_ft_t ft;
    cf_esd_t esd;
    cf_ci_t ci;
    cf_mode_config_t cfg[CFG_MAX];
} cf_set_t;

typedef struct __attribute__ ((packed)) _sl_icon_ref {
    uint8_t *val;
    uint32_t len;
} icon_ref_t;

#ifdef __cplusplus
extern "C" {
#endif

int32_t silfp_cfg_update(cf_set_t *pcfgs, const e_mode_config_t type, const cf_register_t *preg, const uint32_t len);
int32_t silfp_cfg_update_ex(cf_set_t *pcfgs, const e_mode_config_t type, const uint32_t addr, const uint32_t val);
int32_t silfp_cfg_pb_param_update(cf_set_t *pcfgs, const e_pb_param_t type, const int32_t *val, const uint32_t len);
int32_t silfp_cfg_dev_ver_update(cf_set_t *pcfgs, const cf_dev_ver_t *ver, const uint32_t len);

cf_set_t * silfp_cfg_malloc(void);
void silfp_cfg_free(cf_set_t *pcfgs);
cf_set_t * silfp_cfg_init(int32_t fd);
void silfp_cfg_deinit(cf_set_t *pcfgs);

const char * silfp_cfg_get_config_name(const uint32_t idx);
const char *silfp_cfg_get_param_name(const uint32_t idx);

int32_t silfp_cfg_get_update_length(cf_set_t *pcfgs);
int32_t silfp_cfg_get_update_buffer(void *buffer, const uint32_t len, const cf_set_t *pcfgs);
int32_t silfp_cfg_update_config(const void *buffer, const uint32_t len, cf_set_t *psyscfgs);

uint32_t silfp_cfg_xml_config_support(void);

#define ADD_CFG(x) {(cf_register_t *)reg_##x, sizeof(reg_##x)/sizeof(cf_register_t), 0}

#ifdef __cplusplus
}
#endif

#define cfg_upd_index_common_w             2
#define cfg_upd_index_common_h             3
#define cfg_upd_index_common_wp            4
#define cfg_upd_index_common_hp            5
#define cfg_upd_index_common_rw            6
#define cfg_upd_index_common_wdpi          7
#define cfg_upd_index_common_hdpi          8
#define cfg_upd_index_common_w_hwagc       9
#define cfg_upd_index_common_h_hwagc       10
#define cfg_upd_index_common_fg_loop       11
#define cfg_upd_index_common_wc            12
#define cfg_upd_index_common_hc            13
#define cfg_upd_index_common_ver           14

#define cfg_upd_index_common_gain_v0c       0
#define cfg_upd_index_common_gain_v20       1
#define cfg_upd_index_common_gain_v2c       2
#define cfg_upd_index_common_gain_v24       3

#define cfg_upd_index_common_gain_reg_reg0c     0
#define cfg_upd_index_common_gain_reg_reg20     1
#define cfg_upd_index_common_gain_reg_reg2c     2
#define cfg_upd_index_common_gain_reg_reg24     3

#define cfg_upd_index_common_otp_otp0           0
#define cfg_upd_index_common_otp_otp1           1
#define cfg_upd_index_common_otp_otp2           2
#define cfg_upd_index_common_otp_otp3           3
#define cfg_upd_index_common_otp_otp4           4
#define cfg_upd_index_common_otp_otp5           5
#define cfg_upd_index_common_otp_otp_a0         6

#define cfg_upd_index_spi_ms_frm 0
#define cfg_upd_index_spi_retry  1
#define cfg_upd_index_spi_reinit 2
#define cfg_upd_index_spi_start  3
#define cfg_upd_index_spi_msb    4

#define cfg_upd_index_nav_enable            0
#define cfg_upd_index_nav_mode              1
#define cfg_upd_index_nav_type              2
#define cfg_upd_index_nav_con_frame_get_num 3
#define cfg_upd_index_nav_w                 4
#define cfg_upd_index_nav_h                 5
#define cfg_upd_index_nav_wh                6
#define cfg_upd_index_nav_hh                7
#define cfg_upd_index_nav_w_ds              8
#define cfg_upd_index_nav_h_ds              9
#define cfg_upd_index_nav_w_hg_ds           10
#define cfg_upd_index_nav_h_hg_ds           11
#define cfg_upd_index_nav_vk_timeout        12
#define cfg_upd_index_nav_dclick_gap        13
#define cfg_upd_index_nav_longpress         14

#define cfg_upd_index_test_fd_threshold           0
#define cfg_upd_index_test_deadpx_hard_threshold  1
#define cfg_upd_index_test_deadpx_norm_threshold  2
#define cfg_upd_index_test_scut                   3
#define cfg_upd_index_test_detev_ww               4
#define cfg_upd_index_test_detev_hh               5
#define cfg_upd_index_test_deteline_h             6
#define cfg_upd_index_test_deteline_w             7
#define cfg_upd_index_test_deadpx_max             8
#define cfg_upd_index_test_badline_max            9
#define cfg_upd_index_test_finger_detect_mode     10
#define cfg_upd_index_test_deadpx_cut             11

#define cfg_upd_index_mmi_dac_min               0
#define cfg_upd_index_mmi_dac_max               1
#define cfg_upd_index_mmi_grey_range_left       2
#define cfg_upd_index_mmi_grey_range_right      3
#define cfg_upd_index_mmi_nav_base_frame_num    4
#define cfg_upd_index_mmi_max_tune_time         5
#define cfg_upd_index_mmi_auto_adjust_w         6
#define cfg_upd_index_mmi_auto_adjust_h         7
#define cfg_upd_index_mmi_frm_loop_max          8
#define cfg_upd_index_mmi_postprocess_ctl       9
#define cfg_upd_index_mmi_white_base_white_thr  10
#define cfg_upd_index_mmi_white_base_black_thr  11
#define cfg_upd_index_mmi_black_base_white_thr  12
#define cfg_upd_index_mmi_black_base_black_thr  13
#define cfg_upd_index_mmi_middle_base_white_thr 14
#define cfg_upd_index_mmi_middle_base_black_thr 15
#define cfg_upd_index_mmi_diff_base_min_thr     16
#define cfg_upd_index_mmi_diff_base_max_thr     17
#define cfg_upd_index_mmi_snr_cut               18
#define cfg_upd_index_mmi_base_size             19
#define cfg_upd_index_mmi_snr_img_num           20
#define cfg_upd_index_mmi_snr_thr               21
#define cfg_upd_index_mmi_distortion            22
#define cfg_upd_index_mmi_finger_num            23
#define cfg_upd_index_mmi_storage_interval      24
#define cfg_upd_index_mmi_sum_type              25
#define cfg_upd_index_mmi_deadpx_radius         26
#define cfg_upd_index_mmi_cut_radius            27
#define cfg_upd_index_mmi_normalize_blk         28
#define cfg_upd_index_mmi_normalize_ratio       29
#define cfg_upd_index_mmi_fft_ratio             30

#define cfg_upd_index_mmi_touch_info_center_x                   0
#define cfg_upd_index_mmi_touch_info_center_y                   1
#define cfg_upd_index_mmi_touch_info_b1_distance_threshold      2
#define cfg_upd_index_mmi_touch_info_b2_distance_threshold      3
#define cfg_upd_index_mmi_touch_info_b2_b1_distance_threshold   4
#define cfg_upd_index_mmi_touch_info_c1_coverage_threshold      5
#define cfg_upd_index_mmi_touch_info_c2_coverage_threshold      6
//add  0108
#define cfg_upd_index_mmi_gain_snr_signal_v0c      0
#define cfg_upd_index_mmi_gain_snr_signal_v20      1
#define cfg_upd_index_mmi_gain_snr_signal_v2c      2
#define cfg_upd_index_mmi_gain_snr_signal_v24      3

#define cfg_upd_index_mmi_gain_snr_white_v0c       0
#define cfg_upd_index_mmi_gain_snr_white_v20       1
#define cfg_upd_index_mmi_gain_snr_white_v2c       2
#define cfg_upd_index_mmi_gain_snr_white_v24       3
//add  0108
#define cfg_upd_index_pp_aec_left                  1
#define cfg_upd_index_pp_aec_right                 2
#define cfg_upd_index_pp_aec_time                  3
#define cfg_upd_index_pp_cal_max_loop              4
#define cfg_upd_index_pp_dead_a                    5
#define cfg_upd_index_pp_dead_b                    6
#define cfg_upd_index_pp_quality_cut               7
#define cfg_upd_index_pp_quality_thr               8
#define cfg_upd_index_pp_enroll_quality_chk_num    9
#define cfg_upd_index_pp_enroll_post_num           10
#define cfg_upd_index_pp_enroll_post_mask          11
#define cfg_upd_index_pp_icon_ratio_z              12
#define cfg_upd_index_pp_icon_ratio_m              13
#define cfg_upd_index_pp_big_blot_thr              14
#define cfg_upd_index_pp_duo_tpl_thr               15
#define cfg_upd_index_pp_tpl_upd_leakage_thr       16
#define cfg_upd_index_pp_tp_coverage_default       17
#define cfg_upd_index_pp_slope_len                 18
#define cfg_upd_index_pp_slope_cnt                 19
#define cfg_upd_index_pp_slope_h                   20
#define cfg_upd_index_pp_slope_w                   21
#define cfg_upd_index_pp_cut_angle                 22
#define cfg_upd_index_pp_cut_ud                    23
#define cfg_upd_index_pp_cut_lr                    24
#define cfg_upd_index_pp_dry_sub_cnt               25
#define cfg_upd_index_pp_after_verify_cnt          26
#define cfg_upd_index_pp_wts_threshold             27
#define cfg_upd_index_pp_wts_up_cnt                28
#define cfg_upd_index_pp_attack_fail_cnt           29
#define cfg_upd_index_pp_w2d_verify                30
#define cfg_upd_index_pp_w2d_update                31

#define cfg_upd_index_ft_line_step_min      0
#define cfg_upd_index_ft_ignore             1
#define cfg_upd_index_ft_min_theta          2
#define cfg_upd_index_ft_max_theta          3
#define cfg_upd_index_ft_quality_thr        4
#define cfg_upd_index_ft_line_distance_min  5
#define cfg_upd_index_ft_line_distance_max  6
#define cfg_upd_index_ft_cut                7
#define cfg_upd_index_ft_mask_min1          8
#define cfg_upd_index_ft_mask_min2          9
#define cfg_upd_index_ft_mask_min3          10
#define cfg_upd_index_ft_mask_max1          11
#define cfg_upd_index_ft_mask_max2          12
#define cfg_upd_index_ft_mask_max3          13
#define cfg_upd_index_ft_mask_thr           14
#define cfg_upd_index_ft_mask_err_thr       15
#define cfg_upd_index_ft_bias_thr           16
#define cfg_upd_index_ft_bias_err_thr       17
#define cfg_upd_index_ft_mask_ex            18
#define cfg_upd_index_ft_shading_thr        19
#define cfg_upd_index_ft_shading_unit_thr   20
#define cfg_upd_index_ft_p_gray_thr         21
#define cfg_upd_index_ft_p_w_b_thr          22
#define cfg_upd_index_ft_p_gray_area_thr    23
#define cfg_upd_index_ft_p_w_b_area_thr     24
#define cfg_upd_index_ft_dark_percent_thr   25
#define cfg_upd_index_ft_max_diameter_thr   26
#define cfg_upd_index_ft_min_diameter_thr   27
#define cfg_upd_index_ft_circle_thr         28
#define cfg_upd_index_ft_black_signal_thr   29
#define cfg_upd_index_ft_mean_diff          30//add  0108
#define cfg_upd_index_ft_r_square_thr       31//add  0108

#define cfg_upd_index_esd_irq_check             0
#define cfg_upd_index_esd_irq_reg               1
#define cfg_upd_index_esd_irq_val               2
#define cfg_upd_index_esd_int_reg               3
#define cfg_upd_index_esd_int_val               4
#define cfg_upd_index_esd_int_beacon            5

#define cfg_upd_index_ci_auth_reverse_skip      0
#define cfg_upd_index_ci_auth_reverse_grey      1
#define cfg_upd_index_ci_enroll_loop            2
#define cfg_upd_index_ci_enroll_skip            3
#define cfg_upd_index_ci_auth_buf_num           4
#define cfg_upd_index_ci_artificial_verify_bias 5
#define cfg_upd_index_ci_artificial_update_bias 6
#define cfg_upd_index_ci_reserved8              7
#define cfg_upd_index_ci_fingerprint_spd_score_thr       8
#define cfg_upd_index_ci_alg_ctl                9
#define cfg_upd_index_ci_fake_score             10

#define cfg_upd_index_pb_agc_skip_fd                0
#define cfg_upd_index_pb_agc_fd_threshold           1
#define cfg_upd_index_pb_agc_skip_small             2
#define cfg_upd_index_pb_agc_max                    3
#define cfg_upd_index_pb_agc_max_small              4
#define cfg_upd_index_pb_agc_hwagc_enable           5
#define cfg_upd_index_pb_agc_hwcov_wake             6
#define cfg_upd_index_pb_agc_hwcov_tune             7
#define cfg_upd_index_pb_agc_exp_size               8

#define cfg_upd_index_pb_threshold_alg_select                   0
#define cfg_upd_index_pb_threshold_enrolNum                     1
#define cfg_upd_index_pb_threshold_max_templates_num            2
#define cfg_upd_index_pb_threshold_templates_size               3
#define cfg_upd_index_pb_threshold_identify_far_threshold       4
#define cfg_upd_index_pb_threshold_update_far_threshold         5
#define cfg_upd_index_pb_threshold_enroll_quality_threshold     6
#define cfg_upd_index_pb_threshold_enroll_coverage_threshold    7
#define cfg_upd_index_pb_threshold_quality_threshold            8
#define cfg_upd_index_pb_threshold_coverage_threshold           9
#define cfg_upd_index_pb_threshold_skin_threshold               10
#define cfg_upd_index_pb_threshold_artificial_threshold         11
#define cfg_upd_index_pb_threshold_samearea_detect              12
#define cfg_upd_index_pb_threshold_samearea_threshold           13
#define cfg_upd_index_pb_threshold_samearea_dist                14
#define cfg_upd_index_pb_threshold_samearea_start               15
#define cfg_upd_index_pb_threshold_samearea_check_once_num      16
#define cfg_upd_index_pb_threshold_samearea_check_num_total     17
#define cfg_upd_index_pb_threshold_dy_fast                      18
#define cfg_upd_index_pb_threshold_segment                      19
#define cfg_upd_index_pb_threshold_water_finger_detect          20
#define cfg_upd_index_pb_threshold_shake_coe                    21
#define cfg_upd_index_pb_threshold_noise_coe                    22
#define cfg_upd_index_pb_threshold_gray_prec                    23
#define cfg_upd_index_pb_threshold_water_detect_threshold       24
#define cfg_upd_index_pb_threshold_fail_threshold               25
#define cfg_upd_index_pb_threshold_spd_flag                     26
#define cfg_upd_index_pb_threshold_samefinger_threshold         27
#define cfg_upd_index_pb_threshold_identify_epay_threshold      28
#define cfg_upd_index_pb_threshold_force_up_threshold           29
#define cfg_upd_index_pb_threshold_samearea_mask                30

#define GET_UPD_VALUE(cfg, upd_cfg, a) \
    if ((upd_cfg->updated & (1 << cfg_upd_index_##a)) != 0) { \
        cfg->a = upd_cfg->a; \
        LOG_MSG_VERBOSE("update %s = %d", #a, cfg->a); \
    }

#define GET_UPD_VALUE_2(cfg, upd_cfg, a, b) \
    if ((upd_cfg->a.updated & (1 << cfg_upd_index_##a##_##b)) != 0) { \
        cfg->a.b = upd_cfg->a.b; \
        LOG_MSG_VERBOSE("update %s.%s = %d", #a, #b, cfg->a.b); \
    }

#define GET_UPD_VALUE_3(cfg, upd_cfg, a, b, c) \
    if ((upd_cfg->a.b.updated & (1 << cfg_upd_index_##a##_##b##_##c)) != 0) { \
        cfg->a.b.c = upd_cfg->a.b.c; \
        LOG_MSG_VERBOSE("update %s.%s.%s = %d", #a, #b, #c, cfg->a.b.c); \
    }

#define GET_UPD_GAIN_ITEM_VALUE_3(cfg, upd_cfg, a, b, index, c) \
    if ((upd_cfg->a.b[index].updated & (1 << cfg_upd_index_common_gain##_##c)) != 0) { \
        cfg->a.b[index].c = upd_cfg->a.b[index].c; \
        LOG_MSG_VERBOSE("update %s.%s[%d].%s = %d", #a, #b, index, #c, cfg->a.b[index].c); \
    }

#define SET_UPD_VALUE(cfg, a, v) \
    do { \
        cfg->a = v; \
        cfg->updated |= (1 << cfg_upd_index_##a); \
    } while (0)

#define SET_UPD_VALUE_2(cfg, a, b, v) \
    do { \
        cfg->a.b = v; \
        cfg->a.updated |= (1 << cfg_upd_index_##a##_##b); \
    } while (0)

#define SET_UPD_VALUE_3(cfg, a, b, c, v) \
    do { \
        cfg->a.b.c = v; \
        cfg->a.b.updated |= (1 << cfg_upd_index_##a##_##b##_##c); \
    } while (0)

#define SET_UPD_GAIN_ITEM_VALUE_3(cfg, a, b, index, c, v) \
    do { \
        cfg->a.b[index].c = v; \
        cfg->a.b[index].updated |= (1 << cfg_upd_index_common_gain##_##c); \
    } while (0)
#endif /* __SILEAD_FP_CONFIG_H__ */

