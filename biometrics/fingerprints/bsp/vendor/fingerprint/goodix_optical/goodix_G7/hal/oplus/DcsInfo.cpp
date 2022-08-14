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
#include "fingerprint.h"

#include <ctime>
#include "FingerprintCore.h"
#include <cutils/properties.h>
#include <stdio.h>
#include <string.h>

namespace goodix
{

DcsInfo::DcsInfo(HalContext* context) : HalBase(context)
{
    LOG_D(LOG_TAG, "--------------- ctor ---------------");
    mContext = context;
}

DcsInfo::~DcsInfo()
{
}

int DcsInfo::init()
{
    LOG_D(LOG_TAG, "init");
    mDcsStaticInfo.need_notify_init_info = true;
    return 0;
}

void DcsInfo::setAlgoVer(char* dst) {
    LOG_D(LOG_TAG, "src algo ver:%s", mAlgoVer);
    strncpy(dst, mAlgoVer, 32);
}


int DcsInfo::printInitEventInfo(oplus_fingerprint_init_event_info_t* int_event_info) {
    oplus_fingerprint_init_ta_info_t *p = &int_event_info->int_ta_info;
    LOG_D(LOG_TAG, "--------------- printInitEventInfo start ---------------");
    LOG_D(LOG_TAG, "--------------- hal info ---------------");

    LOG_D(LOG_TAG, "init_event_time =%d", int_event_info->init_event_time);
    LOG_D(LOG_TAG, "init_result =%d, ", int_event_info->init_result);
    LOG_D(LOG_TAG, "init_fail_reason =%d, ", int_event_info->init_fail_reason);

    //kpi_info
    LOG_D(LOG_TAG, "init_time_cost_all =%d ms", int_event_info->init_time_cost_all);
    LOG_D(LOG_TAG, "init_time_cost_cdsp =%d ms", int_event_info->init_time_cost_cdsp);
    LOG_D(LOG_TAG, "init_time_cost_driver =%d ms", int_event_info->init_time_cost_driver);
    LOG_D(LOG_TAG, "init_time_cost_ta =%d ms", int_event_info->init_time_cost_ta);

    LOG_D(LOG_TAG, "lcd_type =%s, ", int_event_info->lcd_type);
    LOG_D(LOG_TAG, "dsp_available =%d, ", int_event_info->dsp_available);
    LOG_D(LOG_TAG, "hal_version =%d, ", int_event_info->hal_version);
    LOG_D(LOG_TAG, "driver_version =%d, ", int_event_info->driver_version);
    LOG_D(LOG_TAG, "cdsp_version =%d, ", int_event_info->cdsp_version);

    //------ta info------
    LOG_D(LOG_TAG, "--------------- ta info ---------------");
    LOG_D(LOG_TAG, "init_result =%d, ", p->init_result);
    LOG_D(LOG_TAG, "init_fail_reason =%d, ", p->init_fail_reason);

    //ic_info
    LOG_I(LOG_TAG, "sensor_id =%d 0X%X ", p->sensor_id, p->sensor_id);
    LOG_I(LOG_TAG, "lens_type =%d, ", p->lens_type);
    LOG_I(LOG_TAG, "chip_type =%s, ", p->chip_type);
    LOG_I(LOG_TAG, "factory_type =%s, ", p->factory_type);

    //algo_info
    LOG_I(LOG_TAG, "algo_version =%s, ", p->algo_version);
    LOG_I(LOG_TAG, "core algo_version1 =0X%08X, ", p->algo_version1);
    LOG_I(LOG_TAG, "fake algo_version2 =0X%08X, ", p->algo_version2);
    LOG_D(LOG_TAG, "algo_version3 =0X%08X, ", p->algo_version3);
    LOG_D(LOG_TAG, "algo_version4 =%d, ", p->algo_version4);
    LOG_D(LOG_TAG, "algo_version5 =%d, ", p->algo_version5);

    //ic_status
    LOG_D(LOG_TAG, "badpixel_num =%d, ", p->badpixel_num);
    LOG_D(LOG_TAG, "badpixel_num_local =%d, ", p->badpixel_num_local);
    LOG_D(LOG_TAG, "init_finger_number =%d, ", p->init_finger_number);
    LOG_D(LOG_TAG, "template_verison =%d, ", p->template_verison);
    LOG_D(LOG_TAG, "template_num= %d,  %d,   %d,  %d,   %d", p->template_num[0],
        p->template_num[1], p->template_num[2], p->template_num[3], p->template_num[4]);
    LOG_D(LOG_TAG, "all_template_num =%d, ", p->all_template_num);

    //calabration_info
    LOG_I(LOG_TAG, "exposure_value =%d, ", p->exposure_value);
    LOG_I(LOG_TAG, "exposure_time =%d, ", p->exposure_time);
    LOG_D(LOG_TAG, "calabration_signal_value =%d, ", p->calabration_signal_value);
    LOG_D(LOG_TAG, "factory_type =%d, ", p->calabration_tsnr);
    LOG_D(LOG_TAG, "flesh_touch_diff =%d, ", p->flesh_touch_diff);
    LOG_D(LOG_TAG, "scale =%d, ", p->scale);
    LOG_I(LOG_TAG, "gain =0X%X, ", p->gain);

    //reserve_info
    LOG_D(LOG_TAG, "init_event_state1 =%d, ", p->init_event_state1);
    LOG_D(LOG_TAG, "init_event_state2 =%d, ", p->init_event_state2);
    LOG_D(LOG_TAG, "init_event_sting1 =%s, ", p->init_event_sting1);
    LOG_D(LOG_TAG, "init_event_sting2 =%s, ", p->init_event_sting2);

    //print_end
    LOG_D(LOG_TAG, "------ta recode info------ ");
    char data[512] = {0};
    snprintf(data, 512, "\
        init_event_time:%04d, \
        lcd_type:%s, \
        sensor_id:0x%X, \
        badpixel_num:%d, \
        algo_version:%s, \
        core_algo:0x%08X, \
        fake_algo:0x%08X, \
        exposure_value:%d, \
        exposure_time:%d, \
        gain:0x%X",
        int_event_info->init_event_time,
        int_event_info->lcd_type,
        p->sensor_id,
        p->badpixel_num,
        p->algo_version,
        p->algo_version1,
        p->algo_version2,
        p->exposure_value,
        p->exposure_time,
        p->gain);
    mRecord.recordCsvData(data, RECORD_INIT);
    return 0;
}

int DcsInfo::printAuthEventInfo(oplus_fingerprint_auth_event_info_t* auth_event) {
    oplus_fingerprint_auth_ta_info_t* p = &auth_event->auth_ta_info;
    FUNC_ENTER();

    LOG_D(LOG_TAG, "--------------- printAuthEventInfo start ---------------");
    LOG_D(LOG_TAG, "--------------- hal info ---------------");

    LOG_D(LOG_TAG, "auth_event_time =%d, ", auth_event->auth_event_time);
    LOG_I(LOG_TAG, "auth_type =%d, ", auth_event->auth_type);
    LOG_I(LOG_TAG, "auth_result =%d, ", auth_event->auth_result);
    LOG_D(LOG_TAG, "dsp_available =%d, ", auth_event->dsp_available);
    LOG_I(LOG_TAG, "retry_times =%d, ", auth_event->retry_times);
    LOG_I(LOG_TAG, "continuous_authsuccess_count =%d, ", auth_event->continuous_authsuccess_count);
    LOG_I(LOG_TAG, "continuous_authfail_count =%d, ", auth_event->continuous_authfail_count);
    LOG_D(LOG_TAG, "continuous_badauth_count =%d, ", auth_event->continuous_badauth_count);

    LOG_I(LOG_TAG, "user_gid =%d, ", auth_event->user_gid);
    LOG_I(LOG_TAG, "screen_state =%d, ", auth_event->screen_state);
    LOG_I(LOG_TAG, "fingerprintid =%d, ", auth_event->fingerprintid);
    LOG_I(LOG_TAG, "pid_info =%d, ", auth_event->pid_info);
    LOG_D(LOG_TAG, "captureimg_retry_count =%d, ", auth_event->captureimg_retry_count);
    LOG_D(LOG_TAG, "captureimg_retry_reason =%d, ", auth_event->captureimg_retry_reason);

    LOG_D(LOG_TAG, "auth_total_time =%d, ", auth_event->auth_total_time);
    LOG_D(LOG_TAG, "ui_ready_time =%d, ", auth_event->ui_ready_time);
    LOG_D(LOG_TAG, "pressxy[0] =%d pressxy[1] =%d  , ", auth_event->pressxy[0], auth_event->pressxy[1]);
    LOG_D(LOG_TAG, "area_rate =%d, ", auth_event->area_rate);
    LOG_D(LOG_TAG, "brightness_value =%d, ", auth_event->brightness_value);
    LOG_D(LOG_TAG, "lcd_type =%s, ", auth_event->lcd_type);
    LOG_D(LOG_TAG, "hal_version =%d, ", auth_event->hal_version);
    LOG_D(LOG_TAG, "driver_version =%d, ", auth_event->driver_version);
    LOG_D(LOG_TAG, "cdsp_version =%d, ", auth_event->cdsp_version);

    //------ta info------
    //------base_info------
    LOG_D(LOG_TAG, "--------------- ta info ---------------");
    LOG_I(LOG_TAG, "fail_reason =%d, ", p->fail_reason);
    LOG_I(LOG_TAG, "fail_reason_retry= %d,   %d,  %d", p->fail_reason_retry[0],
        p->fail_reason_retry[1], p->fail_reason_retry[2]);
    LOG_I(LOG_TAG, "algo_version =%s, ", p->algo_version);

    //img_info
    LOG_I(LOG_TAG, "quality_score =%d, ", p->quality_score);
    LOG_I(LOG_TAG, "match_score =%d, ", p->match_score);
    LOG_I(LOG_TAG, "signal_value =%d, ", p->signal_value);
    LOG_I(LOG_TAG, "img_area =%d, ", p->img_area);
    LOG_D(LOG_TAG, "img_direction =%d, ", p->img_direction);

    LOG_D(LOG_TAG, "finger_type =%d, ", p->finger_type);
    LOG_D(LOG_TAG, "ta_retry_times =%d, ", p->ta_retry_times);// G3s: set to 0
    LOG_D(LOG_TAG, "recog_round =%d, ", p->recog_round);
    LOG_D(LOG_TAG, "exposure_flag =%d, ", p->exposure_flag);
    LOG_D(LOG_TAG, "study_flag =%d, ", p->study_flag);

    LOG_D(LOG_TAG, "fdt_base_flag =%d, ", p->fdt_base_flag);
    LOG_D(LOG_TAG, "image_base_flag =%d, ", p->image_base_flag);
    LOG_D(LOG_TAG, "finger_number =%d, ", p->finger_number);
    LOG_D(LOG_TAG, "errtouch_flag =%d, ", p->errtouch_flag);
    LOG_D(LOG_TAG, "memory_info =%d, ", p->memory_info);

    LOG_D(LOG_TAG, "screen_protector_type =%d, ", p->screen_protector_type);
    LOG_D(LOG_TAG, "touch_diff =%d, ", p->touch_diff);
    LOG_D(LOG_TAG, "mp_touch_diff =%d, ", p->mp_touch_diff);
    LOG_I(LOG_TAG, "fake_result =%d, ", p->fake_result);

    //rawdata_info
    LOG_I(LOG_TAG, "auth_rawdata= %d,   %d,  %d", p->auth_rawdata[0],
        p->auth_rawdata[1], p->auth_rawdata[2]);

    //template_info
    LOG_I(LOG_TAG, "one_finger_template_num= %d,  %d,  %d,  %d,  %d", p->one_finger_template_num[0],
        p->one_finger_template_num[1], p->one_finger_template_num[2],
        p->one_finger_template_num[3], p->one_finger_template_num[4]);
    LOG_I(LOG_TAG, "all_template_num =%d, ", p->all_template_num);

    //kpi_info
    LOG_I(LOG_TAG, "capture_time= %dms,  %dms,   %dms,  %dms", p->capture_time[0],
        p->capture_time[1], p->capture_time[2], p->capture_time[3]);
    LOG_I(LOG_TAG, "preprocess_time= %dms,  %dms,   %dms,  %dms", p->preprocess_time[0],
        p->preprocess_time[1], p->preprocess_time[2], p->preprocess_time[3]);
    LOG_I(LOG_TAG, "get_feature_time= %dms,  %dms,   %dms,  %dms", p->get_feature_time[0],
        p->get_feature_time[1], p->get_feature_time[2], p->get_feature_time[3]);
    LOG_I(LOG_TAG, "auth_time= %dms,  %dms,   %dms,  %dms", p->auth_time[0],
        p->auth_time[1], p->auth_time[2], p->auth_time[3]);
    LOG_I(LOG_TAG, "detect_fake_time= %dms,  %dms,   %dms,  %dms", p->detect_fake_time[0],
        p->detect_fake_time[1], p->detect_fake_time[2], p->detect_fake_time[3]);
    LOG_I(LOG_TAG, "kpi_time_all= %dms,  %dms,   %dms,  %dms", p->kpi_time_all[0],
        p->kpi_time_all[1], p->kpi_time_all[2], p->kpi_time_all[3]);
    LOG_I(LOG_TAG, "study_time =%d, ", p->study_time);

    //bak_info
    LOG_D(LOG_TAG, "auth_event_state1 =%d ", p->auth_event_state1);
    LOG_D(LOG_TAG, "auth_event_state2 =%d ", p->auth_event_state2);
    LOG_D(LOG_TAG, "auth_event_string1 =%s ", p->auth_event_string1);
    LOG_D(LOG_TAG, "auth_event_string2 =%s ", p->auth_event_string2);

    LOG_D(LOG_TAG, "--------------- record data ---------------");
    char data[1024] = {0};
    snprintf(data, 1024, "\
        auth_event_time:%04d, \
        algo_version:%s, \
        auth_result:%d, \
        fail_reason:%d, \
        quality_score:%d, \
        match_score:%d, \
        fake_result:%d, \
        all_template_num:%d, \
        signal_value:%d, \
        img_area:%d, \
        recog_round:%d, \
        study_flag:%d, \
        ui_ready_time:%dms, \
        retry_times:%d, \
        auth_rawdata:(r0)%d-(r1)%d-(r2)%d, \
        kpi_time_all:%dms-%dms-%dms, \
        capture_time:%dms-%dms-%dms, \
        preprocess_time:%dms-%dms-%dms, \
        get_feature_time:%dms-%dms-%dms, \
        auth_time:%dms-%dms-%dms, \
        detect_fake_time:%dms-%dms-%dms",
        auth_event->auth_event_time,
        p->algo_version,
        auth_event->auth_result,
        p->fail_reason,
        p->quality_score,
        p->match_score,
        p->fake_result,
        p->all_template_num,
        p->signal_value,
        p->img_area,
        p->recog_round,
        p->study_flag,
        auth_event->ui_ready_time,
        auth_event->retry_times,
        p->auth_rawdata[0], p->auth_rawdata[1], p->auth_rawdata[2],
        p->kpi_time_all[0], p->kpi_time_all[1], p->kpi_time_all[2],
        p->capture_time[0], p->capture_time[1], p->capture_time[2],
        p->preprocess_time[0], p->preprocess_time[1], p->preprocess_time[2],
        p->get_feature_time[0], p->get_feature_time[1], p->get_feature_time[2],
        p->auth_time[0], p->auth_time[1], p->auth_time[2],
        p->detect_fake_time[0], p->detect_fake_time[1], p->detect_fake_time[2]);
    mRecord.recordCsvData(data, RECORD_AUTH);
    FUNC_EXIT(GF_SUCCESS);
    return 0;
}

int DcsInfo::printSingleEnrollEventInfo(oplus_fingerprint_singleenroll_event_info_t* single_enroll_event) {
    oplus_fingerprint_singleenroll_ta_info_t* p = &single_enroll_event->singleenroll_ta_info;
    LOG_D(LOG_TAG, "--------------- printSingleEnrollEventInfo start ---------------");
    LOG_D(LOG_TAG, "--------------- hal info ---------------");
    //base_info
    LOG_D(LOG_TAG, "singleenroll_event_time =%d, ", single_enroll_event->singleenroll_event_time);
    LOG_D(LOG_TAG, "singleenroll_result =%d, ", single_enroll_event->singleenroll_result);
    LOG_D(LOG_TAG, "user_gid =%d, ", single_enroll_event->user_gid);

    //kpi_info
    LOG_D(LOG_TAG, "singleenroll_total_time =%d, ", single_enroll_event->singleenroll_total_time);
    LOG_I(LOG_TAG, "ui_ready_time =%d, ", single_enroll_event->ui_ready_time);

    //tp_info
    LOG_D(LOG_TAG, "pressxy0 =%d, ", single_enroll_event->pressxy[0]);
    LOG_D(LOG_TAG, "pressxy1 =%d, ", single_enroll_event->pressxy[1]);
    LOG_D(LOG_TAG, "area_rate =%d, ", single_enroll_event->area_rate);

    //lcd_info
    LOG_D(LOG_TAG, "brightness_value =%d, ", single_enroll_event->brightness_value);
    LOG_D(LOG_TAG, "lcd_type =%s, ", single_enroll_event->lcd_type);

    //------ta info------
    LOG_D(LOG_TAG, "--------------- ta info ---------------");
    //base_info
    LOG_D(LOG_TAG, "singleenroll_result =%d, ", p->singleenroll_result);
    LOG_I(LOG_TAG, "fail_reason =%d, ", p->fail_reason);
    LOG_D(LOG_TAG, "fail_reason_param1 =%d, ", p->fail_reason_param1);
    LOG_D(LOG_TAG, "fail_reason_param2 =%d, ", p->fail_reason_param2);
    LOG_I(LOG_TAG, "algo_version =%s, ", p->algo_version);
    LOG_D(LOG_TAG, "current_enroll_times =%d, ", p->current_enroll_times);

    //img_info
    LOG_I(LOG_TAG, "quality_score =%d, ", p->quality_score);
    LOG_I(LOG_TAG, "signal_value =%d, ", p->signal_value);
    LOG_I(LOG_TAG, "img_area =%d, ", p->img_area);
    LOG_I(LOG_TAG, "img_direction =%d, ", p->img_direction);
    LOG_D(LOG_TAG, "ta_retry_times =%d, ", p->ta_retry_times);

    LOG_D(LOG_TAG, "exposure_flag =%d, ", p->exposure_flag);
    LOG_D(LOG_TAG, "fdt_base_flag =%d, ", p->fdt_base_flag);
    LOG_D(LOG_TAG, "image_base_flag =%d, ", p->image_base_flag);
    LOG_D(LOG_TAG, "repetition_rate =%d, ", p->repetition_rate);
    LOG_I(LOG_TAG, "enroll_rawdata =%d, ", p->enroll_rawdata);

    LOG_I(LOG_TAG, "anomaly_flag =%d, ", p->anomaly_flag);
    LOG_D(LOG_TAG, "screen_protector_type =%d, ", p->screen_protector_type);
    LOG_D(LOG_TAG, "key_point_num =%d, ", p->key_point_num);
    LOG_D(LOG_TAG, "increase_rate =%d, ", p->increase_rate);

    //kpi_info
    LOG_I(LOG_TAG, "capture_time =%dms, ", p->capture_time);
    LOG_I(LOG_TAG, "preprocess_time =%dms, ", p->preprocess_time);
    LOG_I(LOG_TAG, "get_feature_time =%dms, ", p->get_feature_time);
    LOG_I(LOG_TAG, "enroll_time =%dms, ", p->enroll_time);
    LOG_I(LOG_TAG, "detect_fake_time =%dms, ", p->detect_fake_time);
    LOG_I(LOG_TAG, "kpi_time_all =%dms, ", p->kpi_time_all);

    //lcd_info
    LOG_D(LOG_TAG, "singleenroll_event_state1 =%d, ", p->singleenroll_event_state1);
    LOG_D(LOG_TAG, "singleenroll_event_state2 =%d, ", p->singleenroll_event_state2);
    LOG_D(LOG_TAG, "singleenroll_event_string1 =%s, ", p->singleenroll_event_string1);
    LOG_D(LOG_TAG, "singleenroll_event_string2 =%s, ", p->singleenroll_event_string2);

    //------ta info------
    LOG_D(LOG_TAG, "-----print end------ ");
    LOG_D(LOG_TAG, "--------------- record data ---------------");
    char data[1024] = {0};
    snprintf(data, 1024, "\
        enroll_event_time:%04d, \
        algo_version:%s, \
        singleenroll_result:%d, \
        fail_reason:%d, \
        current_enroll_times:%d,  \
        quality_score:%d, \
        img_direction:%d, \
        anomaly_flag:%d, \
        increase_rate:%d, \
        signal_value:%d, \
        img_area:%d, \
        key_point_num:%d, \
        enroll_rawdata:%d, \
        ui_ready_time:%dms, \
        kpi_time_all:%dms, \
        capture_time:%dms, \
        preprocess_time:%dms, \
        get_feature_time:%dms, \
        enroll_time:%dms, \
        detect_fake_time:%dms",
        single_enroll_event->singleenroll_event_time,
        p->algo_version,
        p->singleenroll_result,
        p->fail_reason,
        p->current_enroll_times,
        p->quality_score,
        p->img_direction,
        p->anomaly_flag,
        p->increase_rate,
        p->signal_value,
        p->img_area,
        p->key_point_num,
        p->enroll_rawdata,
        single_enroll_event->ui_ready_time,
        p->kpi_time_all,
        p->capture_time,
        p->preprocess_time,
        p->get_feature_time,
        p->enroll_time,
        p->detect_fake_time);
        mRecord.recordCsvData(data, RECORD_ENROLL);
    return 0;
}

int DcsInfo::printEnrollEventInfo(oplus_fingerprint_enroll_event_info_t* enroll_event) {
    oplus_fingerprint_enroll_ta_info_t* p = &enroll_event->enroll_ta_info;
    LOG_D(LOG_TAG, "--------------- printEnrollEventInfo start ---------------");
    LOG_D(LOG_TAG, "--------------- hal info ---------------");

    //base_info
    LOG_D(LOG_TAG, "enroll_event_time =%dms, ", enroll_event->enroll_event_time);
    LOG_D(LOG_TAG, "enroll_result =%d, ", enroll_event->enroll_result);
    LOG_D(LOG_TAG, "user_gid =%d, ", enroll_event->user_gid);
    LOG_D(LOG_TAG, "total_press_times =%dms, ", enroll_event->total_press_times);
    LOG_D(LOG_TAG, "fingerprintid =%d, ", enroll_event->fingerprintid);
    LOG_D(LOG_TAG, "pid_info =%d, ", enroll_event->pid_info);
    LOG_D(LOG_TAG, "lcd_type =%s, ", enroll_event->lcd_type);

    //------ta info------
    LOG_D(LOG_TAG, "--------------- ta info ---------------");
    //base_info
    LOG_I(LOG_TAG, "enroll_result =%d, ", p->enroll_result);
    LOG_I(LOG_TAG, "enroll_reason =%d, ", p->enroll_reason);
    LOG_D(LOG_TAG, "cdsp_flag =%d, ", p->cdsp_flag);
    LOG_D(LOG_TAG, "repetition_enroll_number =%d, ", p->repetition_enroll_number);
    LOG_D(LOG_TAG, "total_enroll_times =%d, ", p->total_enroll_times);
    LOG_D(LOG_TAG, "finger_number =%d, ", p->finger_number);
    LOG_D(LOG_TAG, "lcd_type =%d, ", p->lcd_type);

    //version_info
    LOG_I(LOG_TAG, "algo_version =%s, ", p->algo_version);
    LOG_I(LOG_TAG, "template_version =%d, ", p->template_version);

    //bak_info
    LOG_D(LOG_TAG, "enroll_event_state1 =%d, ", p->enroll_event_state1);
    LOG_D(LOG_TAG, "enroll_event_state2 =%d, ", p->enroll_event_state2);
    LOG_D(LOG_TAG, "enroll_event_string1 =%s, ", p->enroll_event_string1);
    LOG_D(LOG_TAG, "enroll_event_string2 =%s, ", p->enroll_event_string2);

    //------ta info------
    LOG_D(LOG_TAG, "------print end------ ");
    LOG_D(LOG_TAG, "--------------- record data ---------------");
    char data[512] = {0};
    snprintf(data, 512, "\
        enroll_event_time:%04d, \
        algo_version:%s, \
        enroll_result:%d, \
        enroll_reason:%d, \
        user_gid:%d, \
        cdsp_flag:%d, \
        finger_number:%d, \
        total_enroll_times:%d",
        enroll_event->enroll_event_time,
        p->algo_version,
        p->enroll_result,
        p->enroll_reason,
        enroll_event->user_gid,
        p->cdsp_flag,
        p->finger_number,
        p->total_enroll_times);
        mRecord.recordCsvData(data, RECORD_ENROLL_FINISH);
    return 0;
}

int DcsInfo::printSpecialEventInfo(oplus_fingerprint_special_event_info_t* p) {
    LOG_D(LOG_TAG, "--------------- printSpecialEventInfo start ---------------");
    LOG_D(LOG_TAG, "--------------- hal info ---------------");

    //base_info
    LOG_D(LOG_TAG, "event_time =%dms, ", p->event_time);
    LOG_I(LOG_TAG, "event_type =%d, ", p->event_type);
    LOG_D(LOG_TAG, "event_trigger_flag =%d, ", p->event_trigger_flag);
    LOG_D(LOG_TAG, "event_reason =%d, ", p->event_reason);
    LOG_D(LOG_TAG, "event_count =%d, ", p->event_count);
    LOG_D(LOG_TAG, "lcd_type =%s, ", p->lcd_type);

    LOG_D(LOG_TAG, "hal_version =%d, ", p->hal_version);
    LOG_I(LOG_TAG, "algo_version =%s, ", p->algo_version);
    LOG_I(LOG_TAG, "user_gid =%d, ", p->user_gid);
    LOG_I(LOG_TAG, "pid_info =%d, ", p->pid_info);

    //fail_reason
    LOG_D(LOG_TAG, "special_event_state1 =%d, ", p->special_event_state1);
    LOG_D(LOG_TAG, "special_event_state2 =%d, ", p->special_event_state2);
    LOG_D(LOG_TAG, "special_event_state3 =%d, ", p->special_event_state3);
    LOG_D(LOG_TAG, "special_event_state4 =%d, ", p->special_event_state4);
    LOG_D(LOG_TAG, "special_event_string1 =%s, ", p->special_event_string1);
    LOG_D(LOG_TAG, "special_event_string2 =%s, ", p->special_event_string2);

    //------ta info------
    LOG_D(LOG_TAG, "------print end------ ");
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
    /*[Notice] 15:30: 15*100 + 30 */
    *event_times = current_tm.tm_hour*100 + current_tm.tm_min;
    LOG_D(LOG_TAG, "event_times  =%d " , *event_times);

    time_t t;
    char time_buf[64] = {0};
    time(&t);
    ctime_r(&t, time_buf);
    LOG_I(LOG_TAG, "current time:%s" , time_buf);
    FUNC_EXIT(GF_SUCCESS);
    return err;
}

int DcsInfo::getDcsBrightnessValue(uint32_t* brightness_value)
{
    char buf[50] = {'\0'};
    int32_t length = 0;
    uint32_t bright_value = 0;
    /* error brightness:55555 */
    *brightness_value = 55555;
    LOG_D(LOG_TAG, "getDcsBrightnessValue start ");

    FUNC_ENTER();

    char *brightness_paths[] = {
        "/sys/class/leds/lcd-backlight/brightness",
        "/sys/class/backlight/panel0-backlight/brightness",
        "/sys/kernel/oplus_display/oplus_brightness",
    };
    int index = 0;
    int N = sizeof(brightness_paths)/sizeof(brightness_paths[0]);

    for (index = 0; index < N; index ++) {
        if (access(brightness_paths[index], 0) == 0) {
            LOG_E(LOG_TAG, "Brightness path index %d, path:%s", index, brightness_paths[index]);
            break;
        }
    }
    if (index == N) {
        LOG_E(LOG_TAG, "no brightness path available");
        return GF_ERROR_BASE;
    }
    int fd = open(brightness_paths[index], O_RDONLY);
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

int DcsInfo::getDcsLcdTypeWithAlgoVer(HalContext* context)
{
    int err = 0;
    FUNC_ENTER();

    char value[PROPERTY_VALUE_MAX] = { 0 };
    err = property_get("persist.vendor.fingerprint.optical.lcdtype", value, "0");
    if (err <= 0) {
        LOG_D(LOG_TAG, "[%s] property not set.", __func__);
        return err;
    }
    oplus_fingerprint_init_ta_info_t* p = &mInitInfo.int_ta_info;
    /* char algo_version[32]
     * char lcd_type[4];
     * value:AA262_SDC or SDC
     */
    /* 1st: find the _ */
    int index = -1;
    for (int i = 0;  i < PROPERTY_VALUE_MAX; i++) {
        if (value[i] == '\0') {
            break;
        }

        if (value[i] == '_') {
            index = i;
            break;
        }
    }

    LOG_I(LOG_TAG, "new lcd type:%s, index:%d", value, index);

    #define LEN 32
    char hw_id[LEN] = {0};
    char lcd_type[LEN] = {0};

    if (index != -1) {
        LOG_D(LOG_TAG, "new lcd type, need deal with the algo_version.");
        strncpy(hw_id, value, index);
        strncpy(lcd_type, &value[index+1], LEN);
    } else {
        LOG_D(LOG_TAG, "old lcd type");
        strncpy(lcd_type, value, LEN);
    }

    /* algo_version + (if)hw_id + lcd_type */
    snprintf(mAlgoVer, LEN, "%s_%s", p->algo_version, value);
    snprintf(p->algo_version, LEN, "%s", mAlgoVer);

    snprintf(mInitInfo.lcd_type, 4, "%s", lcd_type);
    snprintf(mDcsStaticInfo.lcd_type, 4, "%s", lcd_type);
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
    LOG_D(LOG_TAG, "[%s] start", __func__);
    oplus_fingerprint_init_event_info_t* p = &mInitInfo;
    FUNC_ENTER();
    if (context->mFingerprintCore->mNotify == nullptr) {
        err = GF_ERROR_BAD_PARAMS;
        goto out;
    }

    LOG_D(LOG_TAG, "oplus_dcs_event_ta_cmd_t size= %uB", (unsigned int)sizeof(oplus_dcs_event_ta_cmd_t));
    LOG_D(LOG_TAG, "gf_cmd_header_t size= %uB", (unsigned int)sizeof(gf_cmd_header_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_dcs_event_type_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_dcs_event_type_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_dcs_ta_cmd_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_dcs_ta_cmd_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_init_ta_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_init_ta_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_auth_ta_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_auth_ta_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_singleenroll_ta_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_singleenroll_ta_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_enroll_ta_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_enroll_ta_info_t));
    LOG_D(LOG_TAG, "fingerprint_auth_dcsmsg_t size= %uB", (unsigned int)sizeof(gf_fingerprint_auth_dcsmsg_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_init_event_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_init_event_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_auth_event_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_auth_event_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_singleenroll_event_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_singleenroll_event_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_enroll_event_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_enroll_event_info_t));
    LOG_D(LOG_TAG, "oplus_fingerprint_special_event_info_t size= %uB", (unsigned int)sizeof(oplus_fingerprint_special_event_info_t));

    do
    {
        oplus_dcs_event_ta_cmd_t cmd;
        memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
        cmd.header.target = GF_TARGET_ALGO;
        cmd.header.cmd_id = GF_CUSTOMIZED_CMD_ALGO_BIG_DATA;
        cmd.dcs_ta_cmd_info.dcs_type = DCS_INTI_EVENT_INFO;
        err = invokeCommand(&cmd, sizeof(cmd));
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
            goto out;
        }
        memcpy(&p->int_ta_info, &cmd.dcs_ta_cmd_info.data.init_ta_info, sizeof(oplus_fingerprint_init_ta_info_t));//count set to 0?
    } while (0);

    /* 1st save the algo ver */
    memcpy(mAlgoVer, p->int_ta_info.algo_version, sizeof(mAlgoVer));

    getDcsEventTime(&p->init_event_time);
    getDcsLcdTypeWithAlgoVer(context);
    getDcsHalVersion(&p->hal_version);
    getDcsDriverVersion(&p->driver_version);
    getDcsCdspVersion(&p->cdsp_version);
    p->dsp_available = context->mFingerprintCore->mDSPAvailable;
    getDcsPidInfo(&(p->pid_info));

    p->init_result = 0;
    p->init_fail_reason = 0;

    printInitEventInfo(p);
    if (context->mFingerprintCore->mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGRPRINT_DCS_INFO;
        message.data.dcs_info.dcs_type = DCS_INTI_EVENT_INFO;
        message.data.dcs_info.data.init_event_info = *p;
        context->mFingerprintCore->mNotify(&message);
        mDcsStaticInfo.need_notify_init_info = false;
    } else {
        LOG_D(LOG_TAG, "[sendDcsInitEventInfo]mNotify  nullptr!!!!");
    }

out:
    memset(p, 0, sizeof(*p));//count set to 0?
    LOG_D(LOG_TAG, "[sendDcsInitEventInfo] end");
    FUNC_EXIT(err);
    return err;
}

int DcsInfo::sendDcsAuthEventInfo(HalContext* context)
{
    gf_error_t err = GF_SUCCESS;
    LOG_D(LOG_TAG, "[sendDcsAuthEventInfo] start");
    oplus_fingerprint_auth_event_info_t* p = &mAuthInfo;
    FingerprintCore::AuthenticateContext *authContext = &context->mFingerprintCore->authContext;
    FUNC_ENTER();
    p->auth_result = context->mFingerprintCore->dcs_auth_result_type;//??
    getDcsEventTime(&p->auth_event_time);
    memcpy(p->lcd_type, &mDcsStaticInfo.lcd_type, sizeof(p->lcd_type));
    getDcsHalVersion(&p->hal_version);
    getDcsDriverVersion(&p->driver_version);
    getDcsCdspVersion(&p->cdsp_version);
    getDcsBrightnessValue(&(p->brightness_value));

    LOG_D(LOG_TAG, "[getDcsBrightnessValue]p->brightness_value=%d", p->brightness_value);
    //FingerprintCore::getBrightness(&(p->brightness_value));
    p->auth_type = context->mFingerprintCore->mAuthType;//??

    p->auth_ta_info.fail_reason = authContext->result;//
    p->dsp_available = context->mFingerprintCore->mDSPAvailable;//??
    p->retry_times = authContext->retry;//

    if (p->auth_result == DCS_AUTH_FAIL) {
        mDcsStaticInfo.continuous_authsuccess_count = 0;
        mDcsStaticInfo.continuous_authfail_count++;
    } else if (p->auth_result == DCS_AUTH_SUCCESS) {
        mDcsStaticInfo.continuous_authsuccess_count++;
        mDcsStaticInfo.continuous_authfail_count = 0;
    }

    if (((p->auth_result == DCS_AUTH_SUCCESS) && (authContext->retry < 2)) ||
    (p->auth_result == DCS_AUTH_TOO_FAST_NO_IMGINFO)) {
        mDcsStaticInfo.continuous_badauth_count = 0;
    } else {
        mDcsStaticInfo.continuous_badauth_count++;
    }

    if ((p->auth_result == DCS_AUTH_TOO_FAST_NO_IMGINFO) || (p->auth_result == DCS_AUTH_TOO_FAST_GET_IMGINFO)) {
        //break;
    }

    p->continuous_authsuccess_count = mDcsStaticInfo.continuous_authsuccess_count;
    p->continuous_authfail_count = mDcsStaticInfo.continuous_authfail_count;
    p->continuous_badauth_count = mDcsStaticInfo.continuous_badauth_count;
    p->user_gid = context->mFingerprintCore->mGid;
    p->screen_state = context->mFingerprintCore->mScreenInAuthMode;
    getDcsPidInfo(&(p->pid_info));
    p->fingerprintid = authContext->auth_cmd->o_finger_id;
    p->captureimg_retry_count = 0;
    p->captureimg_retry_reason = 0;

    //kpi_info
    p->auth_total_time = context->mFingerprintCore->authTotalTime / 1000;
    p->ui_ready_time = context->mFingerprintCore->uiReadyTime / 1000;

    //tpinfo
    p->pressxy[0] = 0;
    p->pressxy[1] = 0;
    p->area_rate = 0;

    //lcd_info
    p->pressxy[1] = 0;
    p->area_rate = 0;

    if (p->auth_result != DCS_AUTH_TOO_FAST_NO_IMGINFO) {
        oplus_dcs_event_ta_cmd_t cmd;
        memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
        cmd.header.target = GF_TARGET_ALGO;
        cmd.header.cmd_id = GF_CUSTOMIZED_CMD_ALGO_BIG_DATA;
        cmd.dcs_ta_cmd_info.dcs_type = DCS_AUTH_EVENT_INFO;
        err = invokeCommand(&cmd, sizeof(cmd));
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
            goto out;
        }
        memcpy(&p->auth_ta_info, &cmd.dcs_ta_cmd_info.data.auth_ta_info, sizeof(oplus_fingerprint_auth_ta_info_t));//count set to 0?
        LOG_I(LOG_TAG, "set ta_retry_times:%d", p->auth_ta_info.ta_retry_times);
        p->retry_times = p->auth_ta_info.ta_retry_times;

        /* deal rawdata */
        int rawdata1 = 0;
        int rawdata2 = 0;
        int rawdata3 = 0;
        int rawdata4 = 0;
        for (int i = 0; i <= p->retry_times; i++) {
            /* protect the arr len */
            if (i > 2) {
                break;
            }
            rawdata1 = p->auth_ta_info.auth_rawdata[i] & 0x0000FFFF;
            rawdata2 = (p->auth_ta_info.auth_rawdata[i] & 0xFFFF0000) >> 16;

            if (i == 0 || i == 1) {
                rawdata3 = p->auth_ta_info.touch_diff & 0x0000FFFF;
                rawdata4 = (p->auth_ta_info.touch_diff & 0xFFFF0000) >> 16;
            } else if (i == 2) {
                rawdata3 = p->auth_ta_info.mp_touch_diff & 0x0000FFFF;
                rawdata4 = (p->auth_ta_info.mp_touch_diff & 0xFFFF0000) >> 16;
            }
            LOG_I(LOG_TAG, "set r%d auth_rawdata:%d %d %d %d", i, rawdata1, rawdata2, rawdata3, rawdata4);
            p->auth_ta_info.auth_rawdata[i] = rawdata1;
        }
    }

    setAlgoVer(p->auth_ta_info.algo_version);

    printAuthEventInfo(p);

    if (context->mFingerprintCore->mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGRPRINT_DCS_INFO;
        message.data.dcs_info.dcs_type = DCS_AUTH_EVENT_INFO;
        message.data.dcs_info.data.auth_event_info = *p;
        context->mFingerprintCore->mNotify(&message);
    }

    LOG_I(LOG_TAG, "[getDcsDriverVersion] fail_reason =%d, context->result ", p->auth_ta_info.fail_reason);

out:
    memset(p, 0, sizeof(*p));//count set to 0?
    LOG_D(LOG_TAG, "[sendDcsAuthEventInfo] %d:%d end", (int)sizeof(*p), (int)sizeof(oplus_fingerprint_auth_event_info_t));
    FUNC_EXIT(err);
    return err;
}

int DcsInfo::sendDcsSingleEnrollEventInfo(HalContext* context)
{
    gf_error_t err = GF_SUCCESS;
    LOG_D(LOG_TAG, "sendDcsSingleEnrollEventInfo start ");
    oplus_fingerprint_singleenroll_event_info_t* p = &mSingleEnrollInfo;
    FUNC_ENTER();

    //info
    getDcsEventTime(&p->singleenroll_event_time);
    p->user_gid = context->mFingerprintCore->mGid;

    //kpi_info
    p->singleenroll_total_time = 0;
    p->ui_ready_time = context->mFingerprintCore->uiReadyTime / 1000;

    //tpinfo
    p->pressxy[0] = 0;
    p->pressxy[1] = 0;
    p->area_rate = 0;

    //lcd_info
    getDcsBrightnessValue(&(p->brightness_value));
    //FingerprintCore::getBrightness(&(p->brightness_value));
    memcpy(p->lcd_type, &mDcsStaticInfo.lcd_type, sizeof(p->lcd_type));

    do
    {
        oplus_dcs_event_ta_cmd_t cmd;
        memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
        cmd.header.target = GF_TARGET_ALGO;
        cmd.header.cmd_id = GF_CUSTOMIZED_CMD_ALGO_BIG_DATA;
        cmd.dcs_ta_cmd_info.dcs_type = DCS_SINGLEENROLL_EVENT_INFO;
        err = invokeCommand(&cmd, sizeof(cmd));
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
            goto out;
        }

        memcpy(&p->singleenroll_ta_info,
        &cmd.dcs_ta_cmd_info.data.singleenroll_ta_info, sizeof(oplus_fingerprint_singleenroll_ta_info_t));//count set to 0?
    } while (0);

    setAlgoVer(p->singleenroll_ta_info.algo_version);

    printSingleEnrollEventInfo(p);
    if (context->mFingerprintCore->mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGRPRINT_DCS_INFO;//need type
        message.data.dcs_info.dcs_type = DCS_SINGLEENROLL_EVENT_INFO;
        message.data.dcs_info.data.singleenroll_event_info = *p;
        context->mFingerprintCore->mNotify(&message);
    }
out:
    memset(p, 0, sizeof(*p));//count set to 0
    LOG_D(LOG_TAG, "sendDcsSingleEnrollEventInfo end");
    FUNC_EXIT(err);
    return 0;
}

int DcsInfo::sendDcsEnrollEventInfo(HalContext* context)
{
    gf_error_t err = GF_SUCCESS;
    LOG_D(LOG_TAG, "sendDcsEnrollEventInfo start");
    oplus_fingerprint_enroll_event_info_t* p = &mEnrollInfo;
    FUNC_ENTER();

    //info
    getDcsEventTime(&p->enroll_event_time);
    p->user_gid = context->mFingerprintCore->mGid;

    //enroll_info
    p->total_press_times = 0;
    p->fingerprintid = 0;

    //lcd_info
    memcpy(p->lcd_type, &mDcsStaticInfo.lcd_type, sizeof(p->lcd_type));
    getDcsPidInfo(&(p->pid_info));

    do
    {
        oplus_dcs_event_ta_cmd_t cmd;
        memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
        cmd.header.target = GF_TARGET_ALGO;
        cmd.header.cmd_id = GF_CUSTOMIZED_CMD_ALGO_BIG_DATA;
        cmd.dcs_ta_cmd_info.dcs_type = DCS_ENROLL_EVENT_INFO;
        err = invokeCommand(&cmd, sizeof(cmd));
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] invokeCommand err =%d ,return", __func__, err);
            goto out;
        }
        memcpy(&p->enroll_ta_info, &cmd.dcs_ta_cmd_info.data.enroll_ta_info, sizeof(oplus_fingerprint_enroll_ta_info_t));//count set to 0?
    } while (0);

    setAlgoVer(p->enroll_ta_info.algo_version);

    printEnrollEventInfo(p);
    if (context->mFingerprintCore->mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGRPRINT_DCS_INFO;//need type
        message.data.dcs_info.dcs_type = DCS_ENROLL_EVENT_INFO;
        message.data.dcs_info.data.enroll_event_info = *p;
        context->mFingerprintCore->mNotify(&message);
    }

out:
    memset(p, 0, sizeof(*p));//count set to 0
    LOG_D(LOG_TAG, "sendDcsEnrollEventInfo end");
    FUNC_EXIT(err);
    return err;
}

int DcsInfo::sendDcsSpecialEventInfo(HalContext* context)
{
    gf_error_t err = GF_SUCCESS;
    LOG_D(LOG_TAG, "sendDcsSpecialEventInfo start");
    oplus_fingerprint_special_event_info_t* p = &mSpecialInfo;
    FUNC_ENTER();

    //base_info
    getDcsEventTime(&p->event_time);
    p->event_type = 0;
    p->event_trigger_flag = 0;
    p->event_reason = 0;
    p->event_count = 0;
    memcpy(p->lcd_type, &mDcsStaticInfo.lcd_type, sizeof(p->lcd_type));

    //version_info
    //memcpy(&p->algo_version, &context->mFingerprintCore->report_data.algo_version, sizeof(p->algo_version));
    getDcsHalVersion(&p->hal_version);
    //p->algo_version = "\0";
    p->user_gid = 0;
    p->pid_info = 0;

    //fail_reason
    p->special_event_state1 = 0;
    p->special_event_state2 = 0;
    p->special_event_state3 = 0;
    p->special_event_state4 = 0;

    memset(p->special_event_string1, 0 , sizeof(*(p->special_event_string1)));//count set to 0
    memset(p->special_event_string2, 0 , sizeof(*(p->special_event_string2)));//count set to 0

    setAlgoVer(p->algo_version);
    printSpecialEventInfo(p);
    if (context->mFingerprintCore->mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGRPRINT_DCS_INFO;//need type
        message.data.dcs_info.dcs_type = DCS_SPECIAL_EVENT_INFO;
        message.data.dcs_info.data.special_event_info = *p;
        context->mFingerprintCore->mNotify(&message);
    }
    memset(p, 0, sizeof(*p));//count set to 0
    LOG_D(LOG_TAG, "sendDcsSpecialEventInfo end");
    FUNC_EXIT(err);
    return err;
}
}   // namespace goodix
