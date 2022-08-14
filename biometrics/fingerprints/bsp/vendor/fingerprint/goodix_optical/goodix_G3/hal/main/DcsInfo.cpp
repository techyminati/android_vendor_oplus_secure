/**************************************************************************************
 ** File: -DcsInfo.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2020-08-29
 ** Author: Ran.Chen@BSP.Biometrics.Fingerprint,  Add for fingerprint dcs module
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ran. Chen      2020/09/29       create file
 **  Ran. Chen      2020/10/10       modify for coverity
 ************************************************************************************/
#define LOG_TAG "[GF_HAL][oplus][DcsInfo]"

#include "HalContext.h"
#include "Sensor.h"
#include "HalLog.h"
#include "gf_sensor_types.h"
#include "DcsInfo.h"
#include <ctime>
#include "FingerprintCore.h"
#include <cutils/properties.h>

namespace goodix
{

    DcsInfo::DcsInfo(HalContext* context) : HalBase(context)
    {
        LOG_D(LOG_TAG, "--------------- [DcsInfo] DcsInfo ---------------");
    }

    DcsInfo::~DcsInfo()
    {
    }

    int DcsInfo::init()
    {
        LOG_D(LOG_TAG, "--------------- [DcsInfo] init ---------------");
        dcs_static_info.need_notify_init_info = true;
        return 0;
    }

    int DcsInfo::printInitEventInfo(oplus_fingerprint_init_event_info_t* int_event_info) {
        oplus_fingerprint_init_ta_info_t *int_ta_info = &int_event_info->int_ta_info;
        LOG_D(LOG_TAG, "--------------- [printInitEventInfo] print start ---------------");
        LOG_D(LOG_TAG, "--------------- [printInitEventInfo] hal info ---------------");
        LOG_D(LOG_TAG, "[printInitEventInfo] init_event_time =%d, ", int_event_info->init_event_time);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_result =%d, ", int_event_info->init_result);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_fail_reason =%d, ", int_event_info->init_fail_reason);

        //kpi_info
        LOG_D(LOG_TAG, "[printInitEventInfo] init_time_cost_all =%d, ", int_event_info->init_time_cost_all);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_time_cost_cdsp =%d, ", int_event_info->init_time_cost_cdsp);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_time_cost_driver =%d, ", int_event_info->init_time_cost_driver);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_time_cost_ta =%d, ", int_event_info->init_time_cost_ta);

        LOG_D(LOG_TAG, "[printInitEventInfo] lcd_type =%s, ", int_event_info->lcd_type);
        LOG_D(LOG_TAG, "[printInitEventInfo] dsp_available =%d, ", int_event_info->dsp_available);
        LOG_D(LOG_TAG, "[printInitEventInfo] hal_version =%d, ", int_event_info->hal_version);
        LOG_D(LOG_TAG, "[printInitEventInfo] driver_version =%d, ", int_event_info->driver_version);
        LOG_D(LOG_TAG, "[printInitEventInfo] cdsp_version =%d, ", int_event_info->cdsp_version);

        //------ta info------
        LOG_D(LOG_TAG, "--------------- [printInitEventInfo] ta info ---------------");
        LOG_D(LOG_TAG, "[printInitEventInfo] init_result =%d, ", int_ta_info->init_result);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_fail_reason =%d, ", int_ta_info->init_fail_reason);

        //ic_info
        LOG_D(LOG_TAG, "[printInitEventInfo] sensor_id =%d, ", int_ta_info->sensor_id);
        LOG_D(LOG_TAG, "[printInitEventInfo] lens_type =%d, ", int_ta_info->lens_type);
        LOG_D(LOG_TAG, "[printInitEventInfo] chip_type =%s, ", int_ta_info->chip_type);
        LOG_D(LOG_TAG, "[printInitEventInfo] factory_type =%s, ", int_ta_info->factory_type);

        //algo_info
        LOG_D(LOG_TAG, "[printInitEventInfo] algo_version =%s, ", int_ta_info->algo_version);
        LOG_D(LOG_TAG, "[printInitEventInfo] algo_version1 =%d, ", int_ta_info->algo_version1);
        LOG_D(LOG_TAG, "[printInitEventInfo] algo_version2 =%d, ", int_ta_info->algo_version2);
        LOG_D(LOG_TAG, "[printInitEventInfo] algo_version3 =%d, ", int_ta_info->algo_version3);
        LOG_D(LOG_TAG, "[printInitEventInfo] algo_version4 =%d, ", int_ta_info->algo_version4);
        LOG_D(LOG_TAG, "[printInitEventInfo] algo_version5 =%d, ", int_ta_info->algo_version5);

        //ic_status
        LOG_D(LOG_TAG, "[printInitEventInfo] badpixel_num =%d, ", int_ta_info->badpixel_num);
        LOG_D(LOG_TAG, "[printInitEventInfo] badpixel_num_local =%d, ", int_ta_info->badpixel_num_local);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_finger_number =%d, ", int_ta_info->init_finger_number);
        LOG_D(LOG_TAG, "[printInitEventInfo] template_verison =%d, ", int_ta_info->template_verison);
        LOG_D(LOG_TAG, "[printInitEventInfo] template_num= %d,  %d,   %d,  %d,   %d", int_ta_info->template_num[0],
            int_ta_info->template_num[1], int_ta_info->template_num[2], int_ta_info->template_num[3], int_ta_info->template_num[4]);
        LOG_D(LOG_TAG, "[printInitEventInfo] all_template_num =%d, ", int_ta_info->all_template_num);

        //calabration_info
        LOG_D(LOG_TAG, "[printInitEventInfo] exposure_value =%d, ", int_ta_info->exposure_value);
        LOG_D(LOG_TAG, "[printInitEventInfo] exposure_time =%d, ", int_ta_info->exposure_time);
        LOG_D(LOG_TAG, "[printInitEventInfo] calabration_signal_value =%d, ", int_ta_info->calabration_signal_value);
        LOG_D(LOG_TAG, "[printInitEventInfo] factory_type =%d, ", int_ta_info->calabration_tsnr);
        LOG_D(LOG_TAG, "[printInitEventInfo] flesh_touch_diff =%d, ", int_ta_info->flesh_touch_diff);
        LOG_D(LOG_TAG, "[printInitEventInfo] scale =%d, ", int_ta_info->scale);
        LOG_D(LOG_TAG, "[printInitEventInfo] gain =%d, ", int_ta_info->gain);

        //reserve_info
        LOG_D(LOG_TAG, "[printInitEventInfo] init_event_state1 =%d, ", int_ta_info->init_event_state1);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_event_state2 =%d, ", int_ta_info->init_event_state2);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_event_sting1 =%s, ", int_ta_info->init_event_sting1);
        LOG_D(LOG_TAG, "[printInitEventInfo] init_event_sting2 =%s, ", int_ta_info->init_event_sting2);

        //print_end
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] ------ta info------ ");
        return 0;
    }

    int DcsInfo::printAuthEventInfo(oplus_fingerprint_auth_event_info_t* auth_event_info) {
        oplus_fingerprint_auth_ta_info_t* auth_ta_info = &auth_event_info->auth_ta_info;
        FUNC_ENTER();

        LOG_D(LOG_TAG, "--------------- [printAllAuthEventInfo] print start ---------------");
        LOG_D(LOG_TAG, "--------------- [printAllAuthEventInfo] hal info ---------------");

        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_event_time =%d, ", auth_event_info->auth_event_time);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_type =%d, ", auth_event_info->auth_type);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_result =%d, ", auth_event_info->auth_result);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] dsp_available =%d, ", auth_event_info->dsp_available);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] retry_times =%d, ", auth_event_info->retry_times);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] continuous_authsuccess_count =%d, ", auth_event_info->continuous_authsuccess_count);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] continuous_authfail_count =%d, ", auth_event_info->continuous_authfail_count);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] continuous_badauth_count =%d, ", auth_event_info->continuous_badauth_count);

        LOG_D(LOG_TAG, "[printAllAuthEventInfo] user_gid =%d, ", auth_event_info->user_gid);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] screen_state =%d, ", auth_event_info->screen_state);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] fingerprintid =%d, ", auth_event_info->fingerprintid);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] pid_info =%d, ", auth_event_info->pid_info);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] captureimg_retry_count =%d, ", auth_event_info->captureimg_retry_count);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] captureimg_retry_reason =%d, ", auth_event_info->captureimg_retry_reason);

        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_total_time =%d, ", auth_event_info->auth_total_time);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] ui_ready_time =%d, ", auth_event_info->ui_ready_time);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] pressxy[0] =%d pressxy[1] =%d  , ", auth_event_info->pressxy[0], auth_event_info->pressxy[1]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] area_rate =%d, ", auth_event_info->area_rate);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] brightness_value =%d, ", auth_event_info->brightness_value);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] lcd_type =%s, ", auth_event_info->lcd_type);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] hal_version =%d, ", auth_event_info->hal_version);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] driver_version =%d, ", auth_event_info->driver_version);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] cdsp_version =%d, ", auth_event_info->cdsp_version);

        //------ta info------
        //------base_info------
        LOG_D(LOG_TAG, "--------------- [printAllAuthEventInfo] ta info ---------------");
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] fail_reason =%d, ", auth_ta_info->fail_reason);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] fail_reason_retry= %d,   %d,  %d", auth_ta_info->fail_reason_retry[0],
            auth_ta_info->fail_reason_retry[1], auth_ta_info->fail_reason_retry[2]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] algo_version =%s, ", auth_ta_info->algo_version);

        //img_info
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] quality_score =%d, ", auth_ta_info->quality_score);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] match_score =%d, ", auth_ta_info->match_score);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] signal_value =%d, ", auth_ta_info->signal_value);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] img_area =%d, ", auth_ta_info->img_area);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] img_direction =%d, ", auth_ta_info->img_direction);

        LOG_D(LOG_TAG, "[printAllAuthEventInfo] finger_type =%d, ", auth_ta_info->finger_type);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] ta_retry_times =%d, ", auth_ta_info->ta_retry_times);// G3s: set to 0
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] recog_round =%d, ", auth_ta_info->recog_round);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] exposure_flag =%d, ", auth_ta_info->exposure_flag);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] study_flag =%d, ", auth_ta_info->study_flag);

        LOG_D(LOG_TAG, "[printAllAuthEventInfo] fdt_base_flag =%d, ", auth_ta_info->fdt_base_flag);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] image_base_flag =%d, ", auth_ta_info->image_base_flag);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] finger_number =%d, ", auth_ta_info->finger_number);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] errtouch_flag =%d, ", auth_ta_info->errtouch_flag);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] memory_info =%d, ", auth_ta_info->memory_info);

        LOG_D(LOG_TAG, "[printAllAuthEventInfo] screen_protector_type =%d, ", auth_ta_info->screen_protector_type);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] touch_diff =%d, ", auth_ta_info->touch_diff);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] mp_touch_diff =%d, ", auth_ta_info->mp_touch_diff);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] fake_result =%d, ", auth_ta_info->fake_result);

        //rawdata_info
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_rawdata= %d,   %d,  %d", auth_ta_info->auth_rawdata[0],
            auth_ta_info->auth_rawdata[1], auth_ta_info->auth_rawdata[2]);

        //template_info
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] one_finger_template_num= %d,  %d,  %d,  %d,  %d", auth_ta_info->one_finger_template_num[0],
            auth_ta_info->one_finger_template_num[1], auth_ta_info->one_finger_template_num[2],
            auth_ta_info->one_finger_template_num[3], auth_ta_info->one_finger_template_num[4]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] all_template_num =%d, ", auth_ta_info->all_template_num);

        //kpi_info
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] capture_time= %d,  %d,   %d,  %d", auth_ta_info->capture_time[0],
            auth_ta_info->capture_time[1], auth_ta_info->capture_time[2], auth_ta_info->capture_time[3]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] preprocess_time= %d,  %d,   %d,  %d", auth_ta_info->preprocess_time[0],
            auth_ta_info->preprocess_time[1], auth_ta_info->preprocess_time[2], auth_ta_info->preprocess_time[3]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] get_feature_time= %d,  %d,   %d,  %d", auth_ta_info->get_feature_time[0],
            auth_ta_info->get_feature_time[1], auth_ta_info->get_feature_time[2], auth_ta_info->get_feature_time[3]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_time= %d,  %d,   %d,  %d", auth_ta_info->auth_time[0],
            auth_ta_info->auth_time[1], auth_ta_info->auth_time[2], auth_ta_info->auth_time[3]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] detect_fake_time= %d,  %d,   %d,  %d", auth_ta_info->detect_fake_time[0],
            auth_ta_info->detect_fake_time[1], auth_ta_info->detect_fake_time[2], auth_ta_info->detect_fake_time[3]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] kpi_time_all= %d,  %d,   %d,  %d", auth_ta_info->kpi_time_all[0],
            auth_ta_info->kpi_time_all[1], auth_ta_info->kpi_time_all[2], auth_ta_info->kpi_time_all[3]);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] study_time =%d, ", auth_ta_info->study_time);

        //bak_info
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_event_state1 =%d ", auth_ta_info->auth_event_state1);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_event_state2 =%d ", auth_ta_info->auth_event_state2);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_event_string1 =%s ", auth_ta_info->auth_event_string1);
        LOG_D(LOG_TAG, "[printAllAuthEventInfo] auth_event_string2 =%s ", auth_ta_info->auth_event_string2);

        LOG_D(LOG_TAG, "--------------- [printAllAuthEventInfo] print end ---------------");
        FUNC_EXIT(GF_SUCCESS);
        return 0;
    }

    int DcsInfo::printSingleEnrollEventInfo(oplus_fingerprint_singleenroll_event_info_t* singleenroll_event_info) {
        oplus_fingerprint_singleenroll_ta_info_t* singleenroll_ta_info = &singleenroll_event_info->singleenroll_ta_info;
        LOG_D(LOG_TAG, "--------------- [printSingleEnrollEventInfo] print start ---------------");
        LOG_D(LOG_TAG, "--------------- [printSingleEnrollEventInfo] hal info ---------------");

        //base_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_event_time =%d, ", singleenroll_event_info->singleenroll_event_time);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_result =%d, ", singleenroll_event_info->singleenroll_result);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] user_gid =%d, ", singleenroll_event_info->user_gid);

        //kpi_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_total_time =%d, ", singleenroll_event_info->singleenroll_total_time);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] ui_ready_time =%d, ", singleenroll_event_info->ui_ready_time);

        //tp_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] pressxy0 =%d, ", singleenroll_event_info->pressxy[0]);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] pressxy1 =%d, ", singleenroll_event_info->pressxy[1]);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] area_rate =%d, ", singleenroll_event_info->area_rate);

        //lcd_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] brightness_value =%d, ", singleenroll_event_info->brightness_value);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] lcd_type =%s, ", singleenroll_event_info->lcd_type);

        //------ta info------
        LOG_D(LOG_TAG, "--------------- [printSingleEnrollEventInfo] ta info ---------------");
        //base_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_result =%d, ", singleenroll_ta_info->singleenroll_result);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] fail_reason =%d, ", singleenroll_ta_info->fail_reason);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] fail_reason_param1 =%d, ", singleenroll_ta_info->fail_reason_param1);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] fail_reason_param2 =%d, ", singleenroll_ta_info->fail_reason_param2);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] algo_version =%s, ", singleenroll_ta_info->algo_version);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] current_enroll_times =%d, ", singleenroll_ta_info->current_enroll_times);

        //img_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] quality_score =%d, ", singleenroll_ta_info->quality_score);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] signal_value =%d, ", singleenroll_ta_info->signal_value);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] img_area =%d, ", singleenroll_ta_info->img_area);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] img_direction =%d, ", singleenroll_ta_info->img_direction);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] ta_retry_times =%d, ", singleenroll_ta_info->ta_retry_times);

        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] exposure_flag =%d, ", singleenroll_ta_info->exposure_flag);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] fdt_base_flag =%d, ", singleenroll_ta_info->fdt_base_flag);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] image_base_flag =%d, ", singleenroll_ta_info->image_base_flag);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] repetition_rate =%d, ", singleenroll_ta_info->repetition_rate);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] enroll_rawdata =%d, ", singleenroll_ta_info->enroll_rawdata);

        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] anomaly_flag =%d, ", singleenroll_ta_info->anomaly_flag);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] screen_protector_type =%d, ", singleenroll_ta_info->screen_protector_type);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] key_point_num =%d, ", singleenroll_ta_info->key_point_num);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] increase_rate =%d, ", singleenroll_ta_info->increase_rate);

        //kpi_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] capture_time =%d, ", singleenroll_ta_info->capture_time);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] preprocess_time =%d, ", singleenroll_ta_info->preprocess_time);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] get_feature_time =%d, ", singleenroll_ta_info->get_feature_time);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] enroll_time =%d, ", singleenroll_ta_info->enroll_time);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] detect_fake_time =%d, ", singleenroll_ta_info->detect_fake_time);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] kpi_time_all =%d, ", singleenroll_ta_info->kpi_time_all);

        //lcd_info
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_event_state1 =%d, ", singleenroll_ta_info->singleenroll_event_state1);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_event_state2 =%d, ", singleenroll_ta_info->singleenroll_event_state2);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_event_string1 =%s, ", singleenroll_ta_info->singleenroll_event_string1);
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] singleenroll_event_string2 =%s, ", singleenroll_ta_info->singleenroll_event_string2);

        //------ta info------
        LOG_D(LOG_TAG, "[printSingleEnrollEventInfo] -----print end------ ");
        return 0;
    }

    int DcsInfo::printEnrollEventInfo(oplus_fingerprint_enroll_event_info_t* enroll_event_info) {
        oplus_fingerprint_enroll_ta_info_t* enroll_ta_info = &enroll_event_info->enroll_ta_info;
        LOG_D(LOG_TAG, "--------------- [printEnrollEventInfo] print start ---------------");
        LOG_D(LOG_TAG, "--------------- [printEnrollEventInfo] hal info ---------------");

        //base_info
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_event_time =%d, ", enroll_event_info->enroll_event_time);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_result =%d, ", enroll_event_info->enroll_result);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] user_gid =%d, ", enroll_event_info->user_gid);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] total_press_times =%d, ", enroll_event_info->total_press_times);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] fingerprintid =%d, ", enroll_event_info->fingerprintid);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] pid_info =%d, ", enroll_event_info->pid_info);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] lcd_type =%s, ", enroll_event_info->lcd_type);

        //------ta info------
        LOG_D(LOG_TAG, "--------------- [printEnrollEventInfo] ta info ---------------");
        //base_info
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_result =%d, ", enroll_ta_info->enroll_result);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_reason =%d, ", enroll_ta_info->enroll_reason);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] cdsp_flag =%d, ", enroll_ta_info->cdsp_flag);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] repetition_enroll_number =%d, ", enroll_ta_info->repetition_enroll_number);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] total_enroll_times =%d, ", enroll_ta_info->total_enroll_times);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] finger_number =%d, ", enroll_ta_info->finger_number);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] lcd_type =%d, ", enroll_ta_info->lcd_type);

        //version_info
        LOG_D(LOG_TAG, "[printEnrollEventInfo] algo_version =%s, ", enroll_ta_info->algo_version);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] template_version =%d, ", enroll_ta_info->template_version);

        //bak_info
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_event_state1 =%d, ", enroll_ta_info->enroll_event_state1);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_event_state2 =%d, ", enroll_ta_info->enroll_event_state2);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_event_string1 =%s, ", enroll_ta_info->enroll_event_string1);
        LOG_D(LOG_TAG, "[printEnrollEventInfo] enroll_event_string2 =%s, ", enroll_ta_info->enroll_event_string2);

        //------ta info------
        LOG_D(LOG_TAG, "[printEnrollEventInfo] ------print end------ ");
        return 0;
    }

    int DcsInfo::printSpecialEventInfo(oplus_fingerprint_special_event_info_t* special_event_info) {
        LOG_D(LOG_TAG, "--------------- [printSpecialEventInfo] print start ---------------");
        LOG_D(LOG_TAG, "--------------- [printSpecialEventInfo] hal info ---------------");

        //base_info
        LOG_D(LOG_TAG, "[printSpecialEventInfo] event_time =%d, ", special_event_info->event_time);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] event_type =%d, ", special_event_info->event_type);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] event_trigger_flag =%d, ", special_event_info->event_trigger_flag);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] event_reason =%d, ", special_event_info->event_reason);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] event_count =%d, ", special_event_info->event_count);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] lcd_type =%s, ", special_event_info->lcd_type);

        LOG_D(LOG_TAG, "[printSpecialEventInfo] hal_version =%d, ", special_event_info->hal_version);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] algo_version =%s, ", special_event_info->algo_version);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] user_gid =%d, ", special_event_info->user_gid);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] pid_info =%d, ", special_event_info->pid_info);

        //fail_reason
        LOG_D(LOG_TAG, "[printSpecialEventInfo] special_event_state1 =%d, ", special_event_info->special_event_state1);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] special_event_state2 =%d, ", special_event_info->special_event_state2);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] special_event_state3 =%d, ", special_event_info->special_event_state3);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] special_event_state4 =%d, ", special_event_info->special_event_state4);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] special_event_string1 =%s, ", special_event_info->special_event_string1);
        LOG_D(LOG_TAG, "[printSpecialEventInfo] special_event_string2 =%s, ", special_event_info->special_event_string2);

        //------ta info------
        LOG_D(LOG_TAG, "[printEnrollEventInfo] ------print end------ ");
        return 0;
    }

    int DcsInfo::getDcsEventTime(int32_t* event_times)
    {
        int err = 0;
        FUNC_ENTER();

        struct timeval tv;
        memset(&tv, 0, sizeof(timeval));
        struct tm current_tm;
        memset(&current_tm, 0, sizeof(tm));
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &current_tm);
        *event_times = current_tm.tm_hour*100 + current_tm.tm_min;
        LOG_D(LOG_TAG, "[getDcsEventTime], current_tm.tm_year  =%d ,current_tm.tm_mon =%d,%d,%d,%d,%d" ,
        current_tm.tm_year, current_tm.tm_mon, current_tm.tm_mday, current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec);
        LOG_D(LOG_TAG, "[getDcsEventTime], event_times  =%d " , *event_times);
        FUNC_EXIT(GF_SUCCESS);
        return err;
    }

    int DcsInfo::getDcsBrightnessValue(uint32_t* brightness_value)
    {
        char buf[50] = {'\0'};
        int32_t length = 0;
        uint32_t bright_value = 0;
        LOG_D(LOG_TAG, "getDcsBrightnessValue start ");

        FUNC_ENTER();

        char* brightness_path[] = {
                "/sys/class/backlight/panel0-backlight/brightness",
                "/sys/kernel/oplus_display/oplus_brightness",
                };
        int index = 0;
        int tBrightness_path_num  = sizeof(brightness_path)/sizeof(brightness_path[0]);
        for (index = 0; index < tBrightness_path_num; index ++) {
            if (access(brightness_path[index], 0) == 0) {
                LOG_E(LOG_TAG, "Brightness path index %d, path:%s", index, brightness_path[index]);
                break;
            }
        }
        if (index == tBrightness_path_num) {
            LOG_E(LOG_TAG, "no brightness path available");
            FUNC_EXIT(GF_ERROR_BASE);
            return GF_ERROR_BASE;
        }

        int fd = open(brightness_path[index], O_RDONLY);
        if (fd < 0) {
            LOG_E(LOG_TAG, "getBrightness openerr :%d, errno =%d", fd, errno);
            FUNC_EXIT(GF_ERROR_BASE);
            return GF_ERROR_BASE;
        }
        length = read(fd, buf, sizeof(buf));
        if (length > 0) {
            bright_value = atoi(buf);
        } else {
            LOG_E(LOG_TAG, "read failed.length =%d, errno =%d", length, errno);
        }
        *brightness_value = bright_value;
        close(fd);
        LOG_D(LOG_TAG, "getDcsBrightnessValue end, bright_value = %d, brightness_value =%d", bright_value, *brightness_value);
        FUNC_EXIT(GF_SUCCESS);
        return GF_SUCCESS;
    }

    int DcsInfo::getDcsLcdType(HalContext* context)
    {
        int err = 0;
        FUNC_ENTER();

        char value[PROPERTY_VALUE_MAX] = { 0 };
        err = property_get("persist.vendor.fingerprint.optical.lcdtype", value, "0");
        if (err <= 0) {
            LOG_D(LOG_TAG, "[%s] property not set.", __func__);
            return err;
        }
        memcpy(&dcs_static_info.lcd_type, value, sizeof(dcs_static_info.lcd_type));
        FUNC_EXIT(GF_SUCCESS);
        return err;
    }

    int DcsInfo::getDcsHalVersion(int32_t* hal_version)
    {
        FUNC_ENTER();
        *hal_version = 0x11;
        FUNC_EXIT(GF_SUCCESS);
        return 0;
    }

    int DcsInfo::getDcsDriverVersion(int32_t* driver_version)
    {
        FUNC_ENTER();
        *driver_version = 0x11;
        FUNC_EXIT(GF_SUCCESS);
        return 0;
    }

    int DcsInfo::getDcsCdspVersion(int32_t* cdsp_version)
    {
        FUNC_ENTER();
        *cdsp_version = 0x11;
        FUNC_EXIT(GF_SUCCESS);
        return 0;
    }

    int DcsInfo::getDcsPidInfo(int32_t* pid_info)
    {
        int err = 0;
        FUNC_ENTER();
        *pid_info = (int32_t)getpid();
        LOG_D(LOG_TAG, "[getDcsPidInfo] pid_info =%d", *pid_info);
        FUNC_EXIT(GF_SUCCESS);
        return err;
    }

    int DcsInfo::sendDcsInitEventInfo(HalContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        LOG_D(LOG_TAG, "[sendDcsInitEventInfo] start");
        oplus_fingerprint_init_event_info_t* init_event_info = &context->mDcsInfo->init_event_info;
        FUNC_ENTER();
        if (context->mFingerprintCore->mNotify == nullptr) {
            err = GF_ERROR_BAD_PARAMS;
            goto out;
        }

        LOG_D(LOG_TAG, "[] oplus_dcs_event_ta_cmd_t =%d, ", sizeof(oplus_dcs_event_ta_cmd_t));
        LOG_D(LOG_TAG, "[] gf_cmd_header_t =%d, ", sizeof(gf_cmd_header_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_dcs_event_type_t =%d, ", sizeof(oplus_fingerprint_dcs_event_type_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_dcs_ta_cmd_info_t =%d, ", sizeof(oplus_fingerprint_dcs_ta_cmd_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_init_ta_info_t =%d, ", sizeof(oplus_fingerprint_init_ta_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_auth_ta_info_t =%d, ", sizeof(oplus_fingerprint_auth_ta_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_singleenroll_ta_info_t =%d, ", sizeof(oplus_fingerprint_singleenroll_ta_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_enroll_ta_info_t =%d, ", sizeof(oplus_fingerprint_enroll_ta_info_t));
        LOG_D(LOG_TAG, "[] fingerprint_auth_dcsmsg_t =%d, ", sizeof(fingerprint_auth_dcsmsg_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_init_event_info_t =%d, ", sizeof(oplus_fingerprint_init_event_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_auth_event_info_t =%d, ", sizeof(oplus_fingerprint_auth_event_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_singleenroll_event_info_t =%d, ", sizeof(oplus_fingerprint_singleenroll_event_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_enroll_event_info_t =%d, ", sizeof(oplus_fingerprint_enroll_event_info_t));
        LOG_D(LOG_TAG, "[] oplus_fingerprint_special_event_info_t =%d, ", sizeof(oplus_fingerprint_special_event_info_t));

        do
        {
            oplus_dcs_event_ta_cmd_t cmd;
            memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
            cmd.header.target = GF_TARGET_ALGO;
            cmd.header.cmd_id = GF_CMD_ALGO_BIG_DATA;
            cmd.dcs_ta_cmd_info.dcs_type = DCS_INTI_EVENT_INFO;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
                goto out;
            }
            memcpy(&init_event_info->int_ta_info, &cmd.dcs_ta_cmd_info.data.init_ta_info, sizeof(oplus_fingerprint_init_ta_info_t));//count set to 0?
        } while (0);
        memcpy(&init_event_info->int_ta_info.algo_version, &context->mFingerprintCore->report_data.algo_version, sizeof(init_event_info->int_ta_info.algo_version));

        getDcsEventTime(&init_event_info->init_event_time);
        getDcsLcdType(context);
        memcpy(init_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(init_event_info->lcd_type));
        getDcsHalVersion(&init_event_info->hal_version);
        getDcsDriverVersion(&init_event_info->driver_version);
        getDcsCdspVersion(&init_event_info->cdsp_version);
        init_event_info->dsp_available = context->mFingerprintCore->mDSPAvailable;//??
        getDcsPidInfo(&(init_event_info->pid_info));

        init_event_info->init_result = 0;//??
        init_event_info->init_fail_reason = 0;//??

        printInitEventInfo(init_event_info);
        if (context->mFingerprintCore->mNotify != nullptr) {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = OPLUS_FINGRPRINT_DCS_INFO;
            message.data.dcs_info.dcs_type = DCS_INTI_EVENT_INFO;
            message.data.dcs_info.data.init_event_info = context->mDcsInfo->init_event_info;
            context->mFingerprintCore->mNotify(&message);
            dcs_static_info.need_notify_init_info = false;
        } else {
            LOG_D(LOG_TAG, "[sendDcsInitEventInfo]mNotify  nullptr!!!!");
        }

out:
        memset(init_event_info, 0, sizeof(*init_event_info));//count set to 0?
        LOG_D(LOG_TAG, "[sendDcsInitEventInfo] end");
        FUNC_EXIT(err);
        return err;
    }

    int DcsInfo::sendDcsAuthEventInfo(HalContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        LOG_D(LOG_TAG, "[sendDcsAuthEventInfo] start");
        DcsInfo* mDcsInfo = context->mDcsInfo;
        oplus_fingerprint_auth_event_info_t* auth_event_info = &context->mDcsInfo->auth_event_info;
        FingerprintCore::AuthenticateContext *authContext = &context->mFingerprintCore->authContext;
        FUNC_ENTER();
        auth_event_info->auth_result = context->mFingerprintCore->dcs_auth_result_type;//??
        getDcsEventTime(&auth_event_info->auth_event_time);
        memcpy(auth_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(auth_event_info->lcd_type));
        getDcsHalVersion(&auth_event_info->hal_version);
        getDcsDriverVersion(&auth_event_info->driver_version);
        getDcsCdspVersion(&auth_event_info->cdsp_version);
        getDcsBrightnessValue(&(auth_event_info->brightness_value));

        LOG_D(LOG_TAG, "[getDcsBrightnessValue]auth_event_info->brightness_value=%d", auth_event_info->brightness_value);
        //FingerprintCore::getBrightness(&(auth_event_info->brightness_value));
        auth_event_info->auth_type = context->mFingerprintCore->mAuthType;//??

        auth_event_info->auth_ta_info.fail_reason = authContext->result;//
        auth_event_info->dsp_available = context->mFingerprintCore->mDSPAvailable;//??
        auth_event_info->retry_times = authContext->retry;//

        if (auth_event_info->auth_result == DCS_AUTH_FAIL) {
            mDcsInfo->dcs_static_info.continuous_authsuccess_count = 0;
            mDcsInfo->dcs_static_info.continuous_authfail_count++;
        } else if (auth_event_info->auth_result == DCS_AUTH_SUCCESS) {
            mDcsInfo->dcs_static_info.continuous_authsuccess_count++;
            mDcsInfo->dcs_static_info.continuous_authfail_count = 0;
        }

        if (((auth_event_info->auth_result == DCS_AUTH_SUCCESS) && (authContext->retry < 2)) ||
        (auth_event_info->auth_result == DCS_AUTH_TOO_FAST_NO_IMGINFO)) {
            mDcsInfo->dcs_static_info.continuous_badauth_count = 0;
        } else {
            mDcsInfo->dcs_static_info.continuous_badauth_count++;
        }

        if ((auth_event_info->auth_result == DCS_AUTH_TOO_FAST_NO_IMGINFO) || (auth_event_info->auth_result == DCS_AUTH_TOO_FAST_GET_IMGINFO)) {
            //break;
        }

        auth_event_info->continuous_authsuccess_count = mDcsInfo->dcs_static_info.continuous_authsuccess_count;//??
        auth_event_info->continuous_authfail_count = mDcsInfo->dcs_static_info.continuous_authfail_count;//??
        auth_event_info->continuous_badauth_count = mDcsInfo->dcs_static_info.continuous_badauth_count;//??
        auth_event_info->user_gid = context->mFingerprintCore->mGid;//??
        auth_event_info->screen_state = context->mFingerprintCore->mScreenInAuthMode;//??
        getDcsPidInfo(&(auth_event_info->pid_info));
        auth_event_info->fingerprintid = authContext->auth_cmd->o_finger_id;//??
        auth_event_info->captureimg_retry_count = 0;//??
        auth_event_info->captureimg_retry_reason = 0;//??

        //kpi_info
        auth_event_info->auth_total_time = context->mFingerprintCore->authTotalTime / 1000;
        auth_event_info->ui_ready_time = context->mFingerprintCore->uiReadyTime / 1000;

        //tpinfo
        auth_event_info->pressxy[0] = 0;//??;//??
        auth_event_info->pressxy[1] = 0;//??;//??
        auth_event_info->area_rate = 0;//??;//??

        //lcd_info
        auth_event_info->pressxy[1] = 0;//??
        auth_event_info->area_rate = 0;//??

        if (auth_event_info->auth_result != DCS_AUTH_TOO_FAST_NO_IMGINFO) {
            oplus_dcs_event_ta_cmd_t cmd;
            memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
            cmd.header.target = GF_TARGET_ALGO;
            cmd.header.cmd_id = GF_CMD_ALGO_BIG_DATA;
            cmd.dcs_ta_cmd_info.dcs_type = DCS_AUTH_EVENT_INFO;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
                goto out;
            }
            memcpy(&auth_event_info->auth_ta_info, &cmd.dcs_ta_cmd_info.data.auth_ta_info, sizeof(oplus_fingerprint_auth_ta_info_t));//count set to 0?
        }
        memcpy(&auth_event_info->auth_ta_info.algo_version, &context->mFingerprintCore->report_data.algo_version, sizeof(auth_event_info->auth_ta_info.algo_version));

        printAuthEventInfo(auth_event_info);

        if (context->mFingerprintCore->mNotify != nullptr) {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = OPLUS_FINGRPRINT_DCS_INFO;
            message.data.dcs_info.dcs_type = DCS_AUTH_EVENT_INFO;
            message.data.dcs_info.data.auth_event_info = context->mDcsInfo->auth_event_info;
            context->mFingerprintCore->mNotify(&message);
        }

        LOG_I(LOG_TAG, "[getDcsDriverVersion] fail_reason =%d, context->result ", auth_event_info->auth_ta_info.fail_reason);

out:
        memset(auth_event_info, 0, sizeof(*auth_event_info));//count set to 0?
        LOG_D(LOG_TAG, "[sendDcsAuthEventInfo] end");
        FUNC_EXIT(err);
        return err;
    }

    int DcsInfo::sendDcsSingleEnrollEventInfo(HalContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        LOG_D(LOG_TAG, "sendDcsSingleEnrollEventInfo start ");
        oplus_fingerprint_singleenroll_event_info_t* singleenroll_event_info = &context->mDcsInfo->singleenroll_event_info;
        FingerprintCore::EnrollContext *EnrollContext;
        FUNC_ENTER();

        //info
        getDcsEventTime(&singleenroll_event_info->singleenroll_event_time);
        singleenroll_event_info->user_gid = context->mFingerprintCore->mGid;//??

        //kpi_info
        singleenroll_event_info->singleenroll_total_time = 0;//??;//??
        singleenroll_event_info->ui_ready_time = context->mFingerprintCore->uiReadyTime / 1000;//??;

        //tpinfo
        singleenroll_event_info->pressxy[0] = 0;//??;//??
        singleenroll_event_info->pressxy[1] = 0;//??;//??
        singleenroll_event_info->area_rate = 0;//??;//??

        //lcd_info
        getDcsBrightnessValue(&(singleenroll_event_info->brightness_value));
        //FingerprintCore::getBrightness(&(singleenroll_event_info->brightness_value));
        memcpy(singleenroll_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(singleenroll_event_info->lcd_type));

        do
        {
            oplus_dcs_event_ta_cmd_t cmd;
            memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
            cmd.header.target = GF_TARGET_ALGO;
            cmd.header.cmd_id = GF_CMD_ALGO_BIG_DATA;
            cmd.dcs_ta_cmd_info.dcs_type = DCS_SINGLEENROLL_EVENT_INFO;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
                goto out;
            }
            memcpy(&singleenroll_event_info->singleenroll_ta_info,
            &cmd.dcs_ta_cmd_info.data.singleenroll_ta_info, sizeof(oplus_fingerprint_singleenroll_ta_info_t));//count set to 0?
        } while (0);
        memcpy(&singleenroll_event_info->singleenroll_ta_info.algo_version, &context->mFingerprintCore->report_data.algo_version, sizeof(singleenroll_event_info->singleenroll_ta_info.algo_version));

        printSingleEnrollEventInfo(singleenroll_event_info);
        if (context->mFingerprintCore->mNotify != nullptr) {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = OPLUS_FINGRPRINT_DCS_INFO;//need type
            message.data.dcs_info.dcs_type = DCS_SINGLEENROLL_EVENT_INFO;
            message.data.dcs_info.data.singleenroll_event_info = context->mDcsInfo->singleenroll_event_info;
            context->mFingerprintCore->mNotify(&message);
        }
out:
        memset(singleenroll_event_info, 0, sizeof(*singleenroll_event_info));//count set to 0
        LOG_D(LOG_TAG, "sendDcsSingleEnrollEventInfo end");
        FUNC_EXIT(err);
        return 0;
    }

    int DcsInfo::sendDcsEnrollEventInfo(HalContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        LOG_D(LOG_TAG, "sendDcsEnrollEventInfo start");
        oplus_fingerprint_enroll_event_info_t* enroll_event_info = &context->mDcsInfo->enroll_event_info;
        FUNC_ENTER();

        //info
        getDcsEventTime(&enroll_event_info->enroll_event_time);
        enroll_event_info->user_gid = context->mFingerprintCore->mGid;//??

        //enroll_info
        enroll_event_info->total_press_times = 0;//??;//??
        enroll_event_info->fingerprintid = 0;//??;//??

        //lcd_info
        memcpy(enroll_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(enroll_event_info->lcd_type));
        getDcsPidInfo(&(enroll_event_info->pid_info));

        do
        {
            oplus_dcs_event_ta_cmd_t cmd;
            memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
            cmd.header.target = GF_TARGET_ALGO;
            cmd.header.cmd_id = GF_CMD_ALGO_BIG_DATA;
            cmd.dcs_ta_cmd_info.dcs_type = DCS_ENROLL_EVENT_INFO;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
                goto out;
            }
            memcpy(&enroll_event_info->enroll_ta_info, &cmd.dcs_ta_cmd_info.data.enroll_ta_info, sizeof(oplus_fingerprint_enroll_ta_info_t));//count set to 0?
        } while (0);
        memcpy(&enroll_event_info->enroll_ta_info.algo_version, &context->mFingerprintCore->report_data.algo_version, sizeof(enroll_event_info->enroll_ta_info.algo_version));

        printEnrollEventInfo(enroll_event_info);
        if (context->mFingerprintCore->mNotify != nullptr) {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = OPLUS_FINGRPRINT_DCS_INFO;//need type
            message.data.dcs_info.dcs_type = DCS_ENROLL_EVENT_INFO;
            message.data.dcs_info.data.enroll_event_info = context->mDcsInfo->enroll_event_info;
            context->mFingerprintCore->mNotify(&message);
        }

out:
        memset(enroll_event_info, 0, sizeof(*enroll_event_info));//count set to 0
        LOG_D(LOG_TAG, "sendDcsEnrollEventInfo end");
        FUNC_EXIT(err);
        return err;
    }

    int DcsInfo::sendDcsSpecialEventInfo(HalContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        LOG_D(LOG_TAG, "sendDcsSpecialEventInfo start");
        oplus_fingerprint_special_event_info_t* special_event_info = &context->mDcsInfo->special_event_info;
        FUNC_ENTER();

        //base_info
        getDcsEventTime(&special_event_info->event_time);
        special_event_info->event_type = 0;
        special_event_info->event_trigger_flag = 0;
        special_event_info->event_reason = 0;
        special_event_info->event_count = 0;
        memcpy(special_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(special_event_info->lcd_type));

        //version_info
        memcpy(&special_event_info->algo_version, &context->mFingerprintCore->report_data.algo_version, sizeof(special_event_info->algo_version));
        getDcsHalVersion(&special_event_info->hal_version);
        //special_event_info->algo_version = "\0";
        special_event_info->user_gid = 0;
        special_event_info->pid_info = 0;

        //fail_reason
        special_event_info->special_event_state1 = 0;
        special_event_info->special_event_state2 = 0;
        special_event_info->special_event_state3 = 0;
        special_event_info->special_event_state4 = 0;

        memset(special_event_info->special_event_string1, 0 , sizeof(*(special_event_info->special_event_string1)));//count set to 0
        memset(special_event_info->special_event_string2, 0 , sizeof(*(special_event_info->special_event_string2)));//count set to 0

        printSpecialEventInfo(special_event_info);
        if (context->mFingerprintCore->mNotify != nullptr) {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = OPLUS_FINGRPRINT_DCS_INFO;//need type
            message.data.dcs_info.dcs_type = DCS_SPECIAL_EVENT_INFO;
            message.data.dcs_info.data.special_event_info = context->mDcsInfo->special_event_info;
            context->mFingerprintCore->mNotify(&message);
        }
        memset(special_event_info, 0, sizeof(*special_event_info));//count set to 0
        LOG_D(LOG_TAG, "sendDcsSpecialEventInfo end");
        FUNC_EXIT(err);
        return err;
    }
}   // namespace goodix
