/**************************************************************************************
 ** File: -DcsReport.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2020-12-14
 ** Author: Ran.Chen@BSP.Biometrics.Fingerprint,  Add for fingerprint dcs report
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>          <data>           <desc>
 **  Ran. Chen      2020/12/14       create file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][DcsReport]"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/properties.h>
#include <ctime>
#include "DcsInfo.h"
#include "HalLog.h"
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"

namespace android {
DcsReport* DcsReport::sInstance = NULL;
DcsReport::DcsReport() {
    LOG_D(LOG_TAG, "---- [DcsReport] DcsReport ----");
}

DcsReport::~DcsReport() {
    LOG_D(LOG_TAG, "---- [DcsReport] init ----");
}

hidl_string DcsReport::getHidlstring(uint32_t param) {
    char data[64];
    snprintf(data, 63, "%u", param);
    return hidl_string(data, strlen(data));
}
void DcsReport::dcsprintf(vendor::oplus::hardware::commondcs::V1_0::StringPair *dataArray, int dcs_key_number) {
    (void)dataArray;
    int index = 0;
    LOG_D(LOG_TAG, "dcs_key_number:%d", dcs_key_number);
    for (index = 0; index < (dcs_key_number/6) *6; ++index) {
        LOG_D(LOG_TAG, "[%d]:%s:%s, [%d]:%s:%s, [%d]:%s:%s, [%d]:%s:%s, [%d]:%s:%s, [%d]:%s:%s", \
            index, dataArray[index].key.c_str(), dataArray[index].value.c_str(), ++index, dataArray[index].key.c_str(), dataArray[index].value.c_str(), \
            ++index, dataArray[index].key.c_str(), dataArray[index].value.c_str(), ++index, dataArray[index].key.c_str(), dataArray[index].value.c_str(), \
            ++index, dataArray[index].key.c_str(), dataArray[index].value.c_str(), ++index, dataArray[index].key.c_str(), dataArray[index].value.c_str());
    }
    for (; index < dcs_key_number; index++) {
        LOG_D(LOG_TAG, "[%d]:%s:%s", index, dataArray[index].key.c_str(), dataArray[index].value.c_str());
    }
}

void DcsReport::reportInitEventInfo(dcs_init_event_info_t* init_event_info) {
    FUNC_ENTER();
    int dcs_key_number = 0;
    dcs_init_ta_info_t* init_ta_info = &init_event_info->init_ta_info;
    sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
       vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
    if (service != NULL) {
        LOG_D(LOG_TAG, "DcsInfo send fingerprint reportInitEventInfo dcsmsg");
        hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
        vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[39] = {{0, 0}};
        TODCSINT(init_event_info->init_event_time, "init_event_time");
        TODCSINT(init_event_info->init_result, "init_result");
        TODCSINT(init_event_info->init_fail_reason, "init_fail_reason");
        TODCSINT(init_event_info->init_time_cost_all, "init_time_cost_all");
        TODCSINT(init_event_info->init_time_cost_cdsp, "init_time_cost_cdsp");
        TODCSINT(init_event_info->init_time_cost_driver, "init_time_cost_driver");

        TODCSINT(init_event_info->init_time_cost_ta, "init_time_cost_ta");
        TODCSSTRING(init_event_info->lcd_type, "lcd_type");
        TODCSINT(init_event_info->dsp_available, "dsp_available");
        TODCSINT(init_event_info->hal_version, "hal_version");
        TODCSINT(init_event_info->driver_version, "driver_version");
        TODCSINT(init_event_info->cdsp_version, "cdsp_version");

        TODCSINT(init_ta_info->sensor_id, "sensor_id");
        TODCSINT(init_ta_info->lens_type, "lens_type");
        TODCSSTRING(init_ta_info->chip_type, "chip_type");
        TODCSSTRING(init_ta_info->factory_type, "factory_type");
        TODCSSTRING(init_ta_info->algo_version, "algo_version");
        TODCSINT(init_ta_info->algo_version1, "algo_version1");

        TODCSINT(init_ta_info->algo_version2, "algo_version2");
        TODCSINT(init_ta_info->algo_version3, "algo_version3");
        TODCSINT(init_ta_info->algo_version4, "algo_version4");
        TODCSINT(init_ta_info->algo_version5, "algo_version5");
        TODCSINT(init_ta_info->badpixel_num, "badpixel_num");
        TODCSINT(init_ta_info->badpixel_num_local, "badpixel_num_local");

        TODCSINT(init_ta_info->init_finger_number, "init_finger_number");
        TODCSINT(init_ta_info->template_verison, "template_verison");
        TODCSINT(init_ta_info->all_template_num, "all_template_num");
        TODCSINT(init_ta_info->exposure_value, "exposure_value");
        TODCSINT(init_ta_info->exposure_time, "exposure_time");
        sensorExposureTime = init_ta_info->exposure_time;
        TODCSINT(init_ta_info->calabration_signal_value, "calabration_signal_value");

        TODCSINT(init_ta_info->calabration_tsnr, "calabration_tsnr");
        TODCSINT(init_ta_info->flesh_touch_diff, "flesh_touch_diff");
        TODCSINT(init_ta_info->scale, "scale");
        TODCSINT(init_ta_info->gain, "gain");
        TODCSINT(init_ta_info->init_event_state1, "init_event_state1");

        dcsmsg.setToExternal(dataArray, dcs_key_number);
        service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_init_event");
        dcsprintf(dataArray, dcs_key_number);
    } else {
        LOG_E(LOG_TAG, "service NULL");
    }
    FUNC_EXIT(FP_SUCCESS);
    return;
}

void DcsReport::reportAuthEventInfo(dcs_auth_event_info_t* auth_event_info) {
    FUNC_ENTER();
    int dcs_key_number = 0;
    dcs_auth_ta_info_t* auth_ta_info = &auth_event_info->auth_ta_info;
    sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
       vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
    if (service != NULL) {
        LOG_D(LOG_TAG, "DcsInfo send fingerprint auth dcsmsg");
        hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
        vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[64] = {{0, 0}};
        TODCSINT(auth_event_info->auth_event_time, "auth_event_time");
        TODCSINT(auth_event_info->auth_type, "auth_type");
        TODCSINT(auth_event_info->auth_result, "auth_result");
        TODCSINT(auth_event_info->dsp_available, "dsp_available");
        TODCSINT(auth_event_info->retry_times, "retry_times");
        TODCSINT(auth_event_info->continuous_authsuccess_count, "continuous_authsuccess_count");

        TODCSINT(auth_event_info->continuous_authfail_count, "continuous_authfail_count");
        TODCSINT(auth_event_info->continuous_badauth_count, "continuous_badauth_count");
        TODCSINT(auth_event_info->user_gid, "user_gid");
        dcs_key_number++;
        TODCSINT(auth_event_info->screen_state, "screen_state");
        TODCSINT(auth_event_info->fingerprintid, "fingerprintid");
        TODCSINT(auth_event_info->pid_info, "pid_info");

        TODCSINT(auth_event_info->captureimg_retry_count, "captureimg_retry_count");
        TODCSINT(auth_event_info->captureimg_retry_reason, "captureimg_retry_reason");
        TODCSINT(auth_event_info->auth_total_time, "auth_total_time");
        TODCSINT(auth_event_info->ui_ready_time, "ui_ready_time");
        TODCSINT(auth_event_info->pressxy[0], "press_x");
        TODCSINT(auth_event_info->pressxy[1], "press_y");

        TODCSINT(auth_event_info->area_rate, "area_rate");
        TODCSINT(auth_event_info->brightness_value, "brightness_value");
        TODCSSTRING(auth_event_info->lcd_type, "lcd_type");
        TODCSINT(auth_event_info->area_rate, "hal_version");
        TODCSINT(auth_event_info->brightness_value, "driver_version");
        TODCSINT(auth_event_info->area_rate, "cdsp_version");

        TODCSINT(auth_ta_info->fail_reason, "fail_reason");
        TODCSINT(auth_ta_info->fail_reason_retry[0], "fail_reason_retry[0]");
        TODCSINT(auth_ta_info->fail_reason_retry[1], "fail_reason_retry[1]");
        TODCSINT(auth_ta_info->fail_reason_retry[2], "fail_reason_retry[2]");
        TODCSSTRING(auth_ta_info->algo_version, "algo_version");
        TODCSINT(auth_ta_info->quality_score, "quality_score");

        TODCSINT(auth_ta_info->match_score, "match_score");
        TODCSINT(auth_ta_info->signal_value, "signal_value");
        TODCSINT(auth_ta_info->img_area, "img_area");
        TODCSINT(auth_ta_info->img_direction, "img_direction");
        TODCSINT(auth_ta_info->finger_type, "finger_type");
        TODCSINT(auth_ta_info->ta_retry_times, "ta_retry_times");

        TODCSINT(auth_ta_info->recog_round, "recog_round");
        TODCSINT(auth_ta_info->exposure_flag, "exposure_flag");
        TODCSINT(auth_ta_info->study_flag, "study_flag");
        TODCSINT(auth_ta_info->fdt_base_flag, "fdt_base_flag");
        TODCSINT(auth_ta_info->image_base_flag, "image_base_flag");
        TODCSINT(auth_ta_info->finger_number, "finger_number");

        TODCSINT(auth_ta_info->errtouch_flag, "errtouch_flag");
        TODCSINT(auth_ta_info->memory_info, "memory_info");
        TODCSINT(auth_ta_info->screen_protector_type, "screen_protector_type");
        TODCSINT(auth_ta_info->touch_diff, "touch_diff");
        TODCSINT(auth_ta_info->fake_result, "fake_result");
        TODCSINT(auth_ta_info->auth_rawdata[0], "auth_rawdata[0]");

        TODCSINT(auth_ta_info->auth_rawdata[0], "auth_rawdata[0]");
        TODCSINT(auth_ta_info->auth_rawdata[1], "auth_rawdata[1]");
        TODCSINT(auth_ta_info->auth_rawdata[2], "auth_rawdata[2]");
        TODCSINT(auth_ta_info->all_template_num, "all_template_num");
        TODCSINT(auth_ta_info->capture_time[0], "capture_time[0]");
        TODCSINT(auth_ta_info->preprocess_time[0], "preprocess_time[0]");

        TODCSINT(auth_ta_info->get_feature_time[0], "get_feature_time[0]");
        TODCSINT(auth_ta_info->auth_time[0], "auth_time[0]");
        TODCSINT(auth_ta_info->detect_fake_time[0], "detect_fake_time[0]");
        TODCSINT(auth_ta_info->kpi_time_all[0], "kpi_time_all[0]");
        TODCSINT(auth_ta_info->study_time, "study_time");
        TODCSINT(auth_ta_info->auth_event_state1, "auth_event_state1");

        TODCSINT(auth_ta_info->auth_event_state2, "auth_event_state2");
        TODCSSTRING(auth_ta_info->auth_event_string1, "auth_event_string1");
        TODCSSTRING(auth_ta_info->auth_event_string2, "auth_event_string2");

        LOG_D("QUALITY", "13002201: rs:%s, rc:%s, rr:%s, ut:%s, kt:%s, et:%d, lcd:%s, r1:%s, r2:%s, r3:%s",
            dataArray[2].value.c_str(), dataArray[4].value.c_str(), dataArray[37].value.c_str(),
            dataArray[16].value.c_str(), dataArray[15].value.c_str(), sensorExposureTime,
            dataArray[21].value.c_str(), dataArray[49].value.c_str(), dataArray[50].value.c_str(),
            dataArray[51].value.c_str());

        dcsmsg.setToExternal(dataArray, dcs_key_number);
        service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_auth_dcsmsg");
        dcsprintf(dataArray, dcs_key_number);
    } else {
        LOG_E(LOG_TAG, "service NULL");
    }
    FUNC_EXIT(FP_SUCCESS);
    return;
}

void DcsReport::reportSingleEnrollEventInfo(dcs_singleenroll_event_info_t* singleenroll_event_info) {
    FUNC_ENTER();
    int dcs_key_number = 0;
    dcs_singleenroll_ta_info_t* singleenroll_ta_info = &singleenroll_event_info->singleenroll_ta_info;
    sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
       vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
    if (service != NULL) {
        LOG_D(LOG_TAG, "DcsInfo send reportSingleEnrollEventInfo dcsmsg");
        hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
        vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[40] = {{0, 0}};

        TODCSINT(singleenroll_event_info->singleenroll_event_time, "singleenroll_event_time");
        TODCSINT(singleenroll_event_info->singleenroll_result, "singleenroll_result");
        TODCSINT(singleenroll_event_info->user_gid, "user_gid");
        TODCSINT(singleenroll_event_info->singleenroll_total_time, "singleenroll_total_time");
        TODCSINT(singleenroll_event_info->ui_ready_time, "ui_ready_time");
        TODCSINT(singleenroll_event_info->pressxy[0], "pressxy[0]");

        TODCSINT(singleenroll_event_info->pressxy[1], "pressxy[1]");
        TODCSINT(singleenroll_event_info->area_rate, "area_rate");
        TODCSINT(singleenroll_event_info->brightness_value, "brightness_value");
        TODCSSTRING(singleenroll_event_info->lcd_type, "lcd_type");
        TODCSINT(singleenroll_ta_info->fail_reason, "fail_reason");
        TODCSINT(singleenroll_ta_info->fail_reason_param1, "fail_reason_param1");

        TODCSINT(singleenroll_ta_info->fail_reason_param2, "fail_reason_param2");
        TODCSSTRING(singleenroll_ta_info->algo_version, "algo_version");
        TODCSINT(singleenroll_ta_info->current_enroll_times, "current_enroll_times");
        TODCSINT(singleenroll_ta_info->quality_score, "quality_score");
        TODCSINT(singleenroll_ta_info->signal_value, "signal_value");
        TODCSINT(singleenroll_ta_info->img_area, "img_area");

        TODCSINT(singleenroll_ta_info->img_direction, "img_direction");
        TODCSINT(singleenroll_ta_info->finger_type, "finger_type");
        TODCSINT(singleenroll_ta_info->ta_retry_times, "ta_retry_times");
        TODCSINT(singleenroll_ta_info->exposure_flag, "exposure_flag");
        TODCSINT(singleenroll_ta_info->fdt_base_flag, "fdt_base_flag");
        TODCSINT(singleenroll_ta_info->image_base_flag, "image_base_flag");

        TODCSINT(singleenroll_ta_info->repetition_rate, "repetition_rate");
        TODCSINT(singleenroll_ta_info->enroll_rawdata, "enroll_rawdata");
        TODCSINT(singleenroll_ta_info->anomaly_flag, "anomaly_flag");
        TODCSINT(singleenroll_ta_info->screen_protector_type, "screen_protector_type");
        TODCSINT(singleenroll_ta_info->key_point_num, "key_point_num");
        TODCSINT(singleenroll_ta_info->increase_rate, "increase_rate");


        TODCSINT(singleenroll_ta_info->capture_time, "capture_time");
        TODCSINT(singleenroll_ta_info->preprocess_time, "preprocess_time");
        TODCSINT(singleenroll_ta_info->get_feature_time, "get_feature_time");
        TODCSINT(singleenroll_ta_info->enroll_time, "enroll_time");
        TODCSINT(singleenroll_ta_info->detect_fake_time, "detect_fake_time");
        TODCSINT(singleenroll_ta_info->kpi_time_all, "kpi_time_all");

        TODCSINT(singleenroll_ta_info->singleenroll_event_state1, "singleenroll_event_state1");
        TODCSINT(singleenroll_ta_info->singleenroll_event_state2, "singleenroll_event_state2");
        TODCSSTRING(singleenroll_ta_info->singleenroll_event_string1, "singleenroll_event_string1");
        TODCSSTRING(singleenroll_ta_info->singleenroll_event_string2, "singleenroll_event_string2");

        dcsmsg.setToExternal(dataArray, dcs_key_number);
        service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_singleenroll_event");
        dcsprintf(dataArray, dcs_key_number);

    } else {
        LOG_E(LOG_TAG, "service NULL");
    }
    FUNC_EXIT(FP_SUCCESS);
    return;
}

void DcsReport::reportEnrollEventInfo(dcs_enroll_event_info_t* enroll_event_info) {
    FUNC_ENTER();
    int dcs_key_number = 0;
    dcs_enroll_ta_info_t* enroll_ta_info = &enroll_event_info->enroll_ta_info;
    sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
       vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
    if (service != NULL) {
        LOG_D(LOG_TAG, "DcsInfo send reportEnrollEventInfo dcsmsg");
        hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
        vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[18] = {{0, 0}};
        TODCSINT(enroll_event_info->enroll_event_time, "enroll_event_time");
        TODCSINT(enroll_event_info->enroll_result, "enroll_result");
        TODCSINT(enroll_event_info->user_gid, "user_gid");
        TODCSINT(enroll_event_info->total_press_times, "total_press_times");
        TODCSINT(enroll_event_info->fingerprintid, "fingerprintid");
        TODCSINT(enroll_event_info->pid_info, "pid_info");

        TODCSSTRING(enroll_event_info->lcd_type, "lcd_type");
        TODCSINT(enroll_ta_info->enroll_reason, "enroll_reason");
        TODCSINT(enroll_ta_info->cdsp_flag, "cdsp_flag");
        TODCSINT(enroll_ta_info->repetition_enroll_number, "repetition_enroll_number");
        TODCSINT(enroll_ta_info->total_enroll_times, "total_enroll_times");
        TODCSINT(enroll_ta_info->finger_number, "finger_number");

        TODCSSTRING(enroll_ta_info->algo_version, "algo_version");
        TODCSINT(enroll_ta_info->template_version, "template_version");
        TODCSINT(enroll_ta_info->enroll_event_state1, "enroll_event_state1");
        TODCSINT(enroll_ta_info->enroll_event_state2, "enroll_event_state2");
        TODCSSTRING(enroll_ta_info->enroll_event_string1, "enroll_event_string1");
        TODCSSTRING(enroll_ta_info->enroll_event_string2, "enroll_event_string2");

        dcsmsg.setToExternal(dataArray, dcs_key_number);
        service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_enroll_event");
        dcsprintf(dataArray, dcs_key_number);
    } else {
        LOG_E(LOG_TAG, "service NULL");
    }
    FUNC_EXIT(FP_SUCCESS);
    return;
}

void DcsReport::reportSpecialEventInfo(dcs_special_event_info_t* special_event_info) {
    FUNC_ENTER();
    int dcs_key_number = 0;
    sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
       vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
    if (service != NULL) {
        LOG_D(LOG_TAG, "DcsInfo send fingerprint auth dcsmsg");
        hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
        vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[16] = {{0, 0}};
        TODCSINT(special_event_info->event_time, "event_time");
        TODCSINT(special_event_info->event_type, "event_type");
        TODCSINT(special_event_info->event_trigger_flag, "event_trigger_flag");
        TODCSINT(special_event_info->event_reason, "event_reason");
        TODCSINT(special_event_info->event_count, "event_count");
        TODCSSTRING(special_event_info->lcd_type, "lcd_type");

        TODCSINT(special_event_info->hal_version, "hal_version");
        TODCSSTRING(special_event_info->algo_version, "algo_version");
        TODCSINT(special_event_info->user_gid, "user_gid");
        TODCSINT(special_event_info->pid_info, "pid_info");
        TODCSINT(special_event_info->special_event_state1, "special_event_state1");
        TODCSINT(special_event_info->special_event_state2, "special_event_state2");

        TODCSINT(special_event_info->special_event_state3, "special_event_state3");
        TODCSINT(special_event_info->special_event_state3, "special_event_state4");
        TODCSSTRING(special_event_info->special_event_string1, "special_event_string1");
        TODCSSTRING(special_event_info->special_event_string1, "special_event_string2");

        dcsmsg.setToExternal(dataArray, dcs_key_number);
        service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_special_event");
        dcsprintf(dataArray, dcs_key_number);
    } else {
        LOG_E(LOG_TAG, "service NULL");
    }
    FUNC_EXIT(FP_SUCCESS);
    return;
}

}  // namespace android
