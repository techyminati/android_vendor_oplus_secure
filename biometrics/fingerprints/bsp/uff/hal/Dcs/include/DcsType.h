/************************************************************************************
 ** File: - bsp\hal\hwbinder\dcs\include\DcsType.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **     DcsType for Dcs
 **
 ** Version: 1.0
 ** Date created: 10:58:11,01/09/2020
 ** Author: Chen.ran@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         200/09/09       add for dcs_type struct
 ************************************************************************************/
#ifndef DCSTYPE_H
#define DCSTYPE_H

#include <stdint.h>
#include <stddef.h>
#include "FpCommon.h"
#include "FpType.h"



typedef struct dcs_init_ta_info {
    int init_result;
    int init_fail_reason;
    //ic info
    int sensor_id;  //only sensor_id
    int lens_type;
    char chip_type[8];  //example:G7 G3s G3
    char factory_type[8];  //oflim..
    //algo vesion
    char algo_version[32];  //example: v03.02.02.206
    int algo_version1;  //production alog
    int algo_version2;  //fake algo
    int algo_version3;  //errtouch algo
    int algo_version4;
    int algo_version5;
    //ic_status
    int badpixel_num;  //
    int badpixel_num_local;  //for capacitance fingerprint
    int init_finger_number;  //
    int template_verison;  //example::0x22
    int template_num[5];  //
    int all_template_num;  //
    //calabration_info
    int exposure_value;  //
    int exposure_time;  //
    int calabration_signal_value;  //
    int calabration_tsnr;  //
    int flesh_touch_diff;
    int scale;
    int gain;
    //reserve_info
    int init_event_state1;  //
    int init_event_state2;  //
    char init_event_sting1[8];  //
    char init_event_sting2[8];  //
} dcs_init_ta_info_t;

//auth
typedef struct dcs_auth_ta_info {
    //base_info
    int auth_result;
    int fail_reason;
    int fail_reason_retry[3];
    char algo_version[32];  //v03.02.01.06
    //img_info
    int quality_score;
    int match_score;
    int signal_value;
    int img_area;
    int img_direction;
    int finger_type;
    int ta_retry_times;
    int recog_round;  //in auth mode
    int exposure_flag;  //example:o_high_light
    int study_flag;
    int fdt_base_flag;  //for capacitance fingerprint
    int image_base_flag;  //image base
    int finger_number;
    int errtouch_flag;
    int memory_info;  //for ta info
    int screen_protector_type;
    int touch_diff;
    int mp_touch_diff;
    int fake_result;

    //rawdata_info
    int auth_rawdata[3];  //retry 0\2\3

    //template_info
    int one_finger_template_num[5];
    int all_template_num;

    //kpi_info
    int capture_time[4];  //retry0-3
    int preprocess_time[4];  //retry0-3
    int get_feature_time[4];  //retry0-3
    int auth_time[4];  //retry0-3
    int detect_fake_time[4];  //retry0-3
    int kpi_time_all
        [4];  //capture_time + preprocess_time + get_feature_time + auth_time + detect_fake_time
    int study_time;

    //other_info
    int auth_ta_data[32];
    //for example:
    //int rawdata_max;//for capacitance fingerprint
    //int rawdata_min;//for capacitance fingerprint
    //int calbration_rawdata_avg;//for capacitance fingerprint
    //int calbration_rawdata_std;//for capacitance fingerprint

    //bak_info
    int auth_event_state1;
    int auth_event_state2;
    char auth_event_string1[8];
    char auth_event_string2[8];
} dcs_auth_ta_info_t;

//single_enroll
typedef struct dcs_singleenroll_ta_info {
    //base_info
    int singleenroll_result;
    int fail_reason;
    int fail_reason_param1;
    int fail_reason_param2;
    char algo_version[32];
    int current_enroll_times;

    //img_info
    int quality_score;
    int signal_value;
    int img_area;
    int img_direction;
    int finger_type;
    int ta_retry_times;
    int exposure_flag;
    int fdt_base_flag;
    int image_base_flag;
    int repetition_rate;  //0-100
    int enroll_rawdata;
    int anomaly_flag;
    int screen_protector_type;
    int key_point_num;  //for capacitance fingerprint
    int increase_rate;  //
    //int icon_status;

    //kpi_info
    int capture_time;
    int preprocess_time;
    int get_feature_time;
    int enroll_time;
    int detect_fake_time;
    /* capture_time + preprocess_time + get_feature_time + enroll_time + detect_fake_time */
    int kpi_time_all;

    //bak_info
    int singleenroll_event_state1;
    int singleenroll_event_state2;
    char singleenroll_event_string1[8];
    char singleenroll_event_string2[8];
} dcs_singleenroll_ta_info_t;

//enroll_end
typedef struct dcs_enroll_ta_info {
    //base_info
    int enroll_result;
    int enroll_reason;
    int cdsp_flag;
    int repetition_enroll_number;
    int total_enroll_times;
    int finger_number;
    int lcd_type;
    //version_info
    char algo_version[32];
    int template_version;
    //calabration_info
    //bak_info
    int enroll_event_state1;
    int enroll_event_state2;
    char enroll_event_string1[8];
    char enroll_event_string2[8];
} dcs_enroll_ta_info_t;

//special_event
typedef struct dcs_special_ta_info {
    //base_info
    int event_type;
    int event_trigger_flag;
    int event_reason;
    int event_count;

    //bak_info
    int special_event_state1;
    char special_event_string2[8];
} dcs_special_ta_info_t;

//init
typedef struct {
    int32_t init_event_time;  //example:2035, hour:20, minutes:35
    int32_t init_result;
    int32_t init_fail_reason;
    int32_t pid_info;
    //kpi_time
    int32_t init_time_cost_all;
    int32_t init_time_cost_cdsp;
    int32_t init_time_cost_driver;
    int32_t init_time_cost_ta;
    char    lcd_type[4];  //example:SDC, BOE
    int32_t dsp_available;

    int32_t hal_version;
    int32_t driver_version;
    int32_t cdsp_version;

    dcs_init_ta_info_t init_ta_info;
} dcs_init_event_info_t;

//auth
typedef struct {
    //base_info
    int32_t auth_event_time;
    int32_t auth_type;
    int32_t auth_result;
    int32_t dsp_available;
    int32_t retry_times;
    int32_t continuous_authsuccess_count;
    int32_t continuous_authfail_count;
    int32_t continuous_badauth_count;  //auth fail or retry_index >=2
    int32_t user_gid;
    int32_t screen_state;
    int32_t fingerprintid;  //enroll success fingerprintid
    int32_t pid_info;
    int32_t captureimg_retry_count;  //capture img in retry0 (G5\G6)
    int32_t captureimg_retry_reason;

    //kpi_info
    int32_t auth_total_time;
    int32_t ui_ready_time;  //for lcd
        //tp info
    int32_t pressxy[2];  //for TP
    int32_t area_rate;  //for TP
        //lcd info
    uint32_t brightness_value;
    char     lcd_type[4];
    //version_info
    int32_t hal_version;
    int32_t driver_version;
    int32_t cdsp_version;

    int32_t auth_ta_data[32];
    dcs_auth_ta_info_t auth_ta_info;
} dcs_auth_event_info_t;

//single_enroll
typedef struct {
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
    char     lcd_type[4];

    dcs_singleenroll_ta_info_t singleenroll_ta_info;
} dcs_singleenroll_event_info_t;

//enroll_end
typedef struct {
    //base_info
    int32_t enroll_event_time;
    int32_t enroll_result;
    int32_t user_gid;
    int32_t total_press_times;
    int32_t fingerprintid;
    int32_t pid_info;
    char    lcd_type[4];

    dcs_enroll_ta_info_t enroll_ta_info;
} dcs_enroll_event_info_t;

typedef struct {
    //base info
    int32_t event_time;
    int32_t event_type;
    int32_t event_trigger_flag;
    int32_t event_reason;
    int32_t event_count;
    char    lcd_type[4];

    int32_t hal_version;
    char    algo_version[32];
    int32_t user_gid;
    int32_t pid_info;

    //fail_reason
    int32_t special_event_state1;
    int32_t special_event_state2;
    int32_t special_event_state3;
    int32_t special_event_state4;
    char    special_event_string1[8];
    char    special_event_string2[8];
    dcs_special_ta_info_t special_ta_info;
} dcs_special_event_info_t;

typedef struct {
    // base info
    int32_t continuous_authsuccess_count;
    int32_t continuous_authfail_count;
    int32_t continuous_badauth_count;
    char    lcd_type[4];
    int32_t special_event_count;  //
    char    algo_version[32];
    bool    need_notify_init_info = false;  //
} dcs_static_info_t;

#endif /* DCSTYPE_H */
