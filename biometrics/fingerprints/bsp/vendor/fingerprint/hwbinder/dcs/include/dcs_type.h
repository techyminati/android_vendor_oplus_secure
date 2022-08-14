/************************************************************************************
 ** File: - fingerprint\vendor\fingerprint\hwbinder\dcs\include\dcs_type.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 10:58:11,01/09/2020
 ** Author: Chen.ran@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         200/09/09       add for dcs_type struct
 ************************************************************************************/

#ifndef DCS_TYPE_H
#define DCS_TYPE_H

#include <hardware/hw_auth_token.h>
#include <hardware/hardware.h>

// init
typedef struct
{
    int32_t init_result;
    int32_t init_fail_reason;

//ic info
    int32_t sensor_id;//only sensor_id
    int32_t lens_type;
    char chip_type[8];//example:G7、G3s、G3
    char factory_type[8];//oflim..

//algo vesion
    char algo_version[32];//example: v03.02.02.206
    int32_t algo_version1;//production alog
    int32_t algo_version2;//fake algo
    int32_t algo_version3;//errtouch algo
    int32_t algo_version4;
    int32_t algo_version5;

//ic_status
    int32_t badpixel_num;//
    int32_t badpixel_num_local;//for capacitance fingerprint
    int32_t init_finger_number;//
    int32_t template_verison;//example::0x22
    int32_t template_num[5];//
    int32_t all_template_num;//

//calabration_info
    int32_t exposure_value;//
    int32_t exposure_time;//
    int32_t calabration_signal_value;//
    int32_t calabration_tsnr;//
    int32_t flesh_touch_diff;
    int32_t scale;
    int32_t gain;

//reserve_info
    int32_t init_event_state1;//
    int32_t init_event_state2;//
    char init_event_sting1[8];//
    char init_event_sting2[8];//
} oplus_fingerprint_init_ta_info_t;


typedef struct
{
    int32_t init_event_time;//example:2035, hour:20, minutes:35
    int32_t init_result;
    int32_t init_fail_reason;
    int32_t pid_info;
    //kpi_time
    int32_t init_time_cost_all;
    int32_t init_time_cost_cdsp;
    int32_t init_time_cost_driver;
    int32_t init_time_cost_ta;
    char lcd_type[4];//example:SDC, BOE
    int32_t dsp_available;

    int32_t hal_version;
    int32_t driver_version;
    int32_t cdsp_version;

    oplus_fingerprint_init_ta_info_t int_ta_info;
} oplus_fingerprint_init_event_info_t;


//auth
typedef struct
{
//base_info
    int32_t auth_result;
    int32_t fail_reason;
    int32_t fail_reason_retry[3];
    char algo_version[32];//v03.02.01.06

//img_info

    int32_t quality_score;
    int32_t match_score;
    int32_t signal_value;
    int32_t img_area;
    int32_t img_direction;
    int32_t finger_type;
    int32_t ta_retry_times;
    int32_t recog_round;//in auth mode
    int32_t exposure_flag;//example:o_high_light
    int32_t study_flag;
    int32_t fdt_base_flag;//for capacitance fingerprint
    int32_t image_base_flag;//image base
    int32_t finger_number;
    int32_t errtouch_flag;
    int32_t memory_info;//for ta info
    int32_t screen_protector_type;
    int32_t touch_diff;
    int32_t mp_touch_diff;
    int32_t fake_result;

//rawdata_info
    int32_t auth_rawdata[3];//retry 0\2\3

//template_info
    int32_t one_finger_template_num[5];
    int32_t all_template_num;

//kpi_info
    int32_t capture_time[4];//retry0-3
    int32_t preprocess_time[4];//retry0-3
    int32_t get_feature_time[4];//retry0-3
    int32_t auth_time[4];//retry0-3
    int32_t detect_fake_time[4];//retry0-3
    int32_t kpi_time_all[4];//capture_time + preprocess_time + get_feature_time + auth_time + detect_fake_time
    int32_t study_time;

//other_info
    int32_t auth_ta_data[32];
    //for example:
    //int32_t rawdata_max;//for capacitance fingerprint
    //int32_t rawdata_min;//for capacitance fingerprint
    //int32_t calbration_rawdata_avg;//for capacitance fingerprint
    //int32_t calbration_rawdata_std;//for capacitance fingerprint

//bak_info
    int32_t auth_event_state1;
    int32_t auth_event_state2;
    char auth_event_string1[8];
    char auth_event_string2[8];
} oplus_fingerprint_auth_ta_info_t;


typedef struct
{
//base_info
    int32_t auth_event_time;
    int32_t auth_type;
    int32_t auth_result;
    int32_t dsp_available;
    int32_t retry_times;
    int32_t continuous_authsuccess_count;
    int32_t continuous_authfail_count;
    int32_t continuous_badauth_count;//auth fail or retry_count >=2
    int32_t user_gid;
    int32_t screen_state;
    int32_t fingerprintid;//enroll success fingerprintid
    int32_t pid_info;
    int32_t captureimg_retry_count;//capture img in retry0 (G5\G6)
    int32_t captureimg_retry_reason;

//kpi_info
    int32_t auth_total_time;
    int32_t ui_ready_time;//for lcd
//tp info
    int32_t pressxy[2];//for TP
    int32_t area_rate;//for TP
//lcd info
    uint32_t brightness_value;
    char lcd_type[4];
//version_info
    int32_t hal_version;
    int32_t driver_version;
    int32_t cdsp_version;

    int32_t auth_ta_data[32];
    oplus_fingerprint_auth_ta_info_t  auth_ta_info;
} oplus_fingerprint_auth_event_info_t;




//single_enroll
typedef struct
{
//base_info
    int32_t singleenroll_result;
    int32_t fail_reason;
    int32_t fail_reason_param1;
    int32_t fail_reason_param2;
    char algo_version[32];
    int32_t current_enroll_times;

//img_info
    int32_t quality_score;
    int32_t signal_value;
    int32_t img_area;
    int32_t img_direction;
    int32_t finger_type;
    int32_t ta_retry_times;
    int32_t exposure_flag;
    int32_t fdt_base_flag;
    int32_t image_base_flag;
    int32_t repetition_rate;//0-100
    int32_t enroll_rawdata;
    int32_t anomaly_flag;
    int32_t screen_protector_type;
    int32_t key_point_num;//for capacitance fingerprint
    int32_t increase_rate;//
    //int32_t icon_status;

//kpi_info
    int32_t capture_time;
    int32_t preprocess_time;
    int32_t get_feature_time;
    int32_t enroll_time;
    int32_t detect_fake_time;
    int32_t kpi_time_all;//capture_time + preprocess_time + get_feature_time + enroll_time + detect_fake_time

//bak_info
    int32_t singleenroll_event_state1;
    int32_t singleenroll_event_state2;
    char singleenroll_event_string1[8];
    char singleenroll_event_string2[8];
} oplus_fingerprint_singleenroll_ta_info_t;


typedef struct
{
//base_info
    int32_t singleenroll_event_time;
    int32_t singleenroll_result;
    int32_t user_gid;

//kpi_info
    int32_t singleenroll_total_time;
    int32_t ui_ready_time;
//tp_info：
    int32_t pressxy[2];
    int32_t area_rate;
//lcd info
    uint32_t brightness_value;
    char lcd_type[4];

    oplus_fingerprint_singleenroll_ta_info_t singleenroll_ta_info;
} oplus_fingerprint_singleenroll_event_info_t;

//enroll_end
typedef struct
{
//base_info
    int32_t enroll_result;
    int32_t enroll_reason;
    int32_t cdsp_flag;

    int32_t repetition_enroll_number;
    int32_t total_enroll_times;
    int32_t finger_number;
    int32_t lcd_type;

//version_info
    char algo_version[32];
    int32_t template_version;

//calabration_info

//bak_info
    int32_t enroll_event_state1;
    int32_t enroll_event_state2;
    char enroll_event_string1[8];
    char enroll_event_string2[8];
} oplus_fingerprint_enroll_ta_info_t;


typedef struct
{
//base_info
    int32_t enroll_event_time;
    int32_t enroll_result;
    int32_t user_gid;
    int32_t total_press_times;
    int32_t fingerprintid;
    int32_t pid_info;
    char lcd_type[4];

    oplus_fingerprint_enroll_ta_info_t enroll_ta_info;
} oplus_fingerprint_enroll_event_info_t;

typedef struct
{
//base info
    int32_t event_time;
    int32_t event_type;
    int32_t event_trigger_flag;
    int32_t event_reason;
    int32_t event_count;
    char lcd_type[4];

    int32_t hal_version;
    char algo_version[32];
    int32_t user_gid;
    int32_t pid_info;

//fail_reason
    int32_t special_event_state1;
    int32_t special_event_state2;
    int32_t special_event_state3;
    int32_t special_event_state4;
    char special_event_string1[8];
    char special_event_string2[8];
} oplus_fingerprint_special_event_info_t;

typedef enum oplus_fingerprint_dcs_auth_result_type {
    DCS_AUTH_FAIL = 0,
    DCS_AUTH_SUCCESS = 1,
    DCS_AUTH_TOO_FAST_NO_IMGINFO = 2,
    DCS_AUTH_TOO_FAST_GET_IMGINFO = 3,
    DCS_AUTH_OTHER_FAIL_REASON = 4,
    DCS_DEFAULT_AUTH_RESULT_INFO,
} oplus_fingerprint_dcs_auth_result_type_t;


typedef enum oplus_fingerprint_dcs_event_type {
    DCS_INTI_EVENT_INFO = 0,
    DCS_AUTH_EVENT_INFO = 1,
    DCS_SINGLEENROLL_EVENT_INFO = 2,
    DCS_ENROLL_EVENT_INFO = 3,
    DCS_SPECIAL_EVENT_INFO = 4,
    DCS_DEFAULT_EVENT_INFO,
} oplus_fingerprint_dcs_event_type_t;


typedef struct {
    oplus_fingerprint_dcs_event_type_t dcs_type;
    union {
        oplus_fingerprint_init_event_info_t init_event_info;
        oplus_fingerprint_auth_event_info_t auth_event_info;
        oplus_fingerprint_singleenroll_event_info_t singleenroll_event_info;
        oplus_fingerprint_enroll_event_info_t enroll_event_info;
        oplus_fingerprint_special_event_info_t special_event_info;
    } data;
} oplus_fingerprint_dcs_info_t;

#endif  /* DCS_TYPE_H */
