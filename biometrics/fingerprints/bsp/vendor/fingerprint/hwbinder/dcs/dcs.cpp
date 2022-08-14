/**************************************************************************************
 ** File: - dcs.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2019-08-29
 ** Author: Ziqing.Guo@BSP.Fingerprint.Basic, Add for fingerprint dcs module
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ziqing.Guo       2019/08/29       create file
 **  Ziqing.Guo       2019/08/29       add the dcs implementation
 ************************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "dcs.h"

#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"

namespace android {

    // --- Hypnus ---
    Dcs* Dcs::sInstance = NULL;

    Dcs::Dcs() {
        ALOGD("Dcs create");
    }

    Dcs::~Dcs() {
    }

    hidl_string Dcs::getHidlstring(uint32_t param) {
        char data[64];
        snprintf(data, 63, "%u", param);
        return hidl_string(data, strlen(data));
    }

    /*description:  */
    void Dcs::reportMonitor(uint32_t param) {

        ALOGD("enter into onMonitorEventTriggered CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD(" send goodixfp fingerprint onMonitorEventTriggered dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[1] = {{0, 0}};
            dataArray[0].key = "MonitorEvent";
            dataArray[0].value = getHidlstring(param);

            dcsmsg.setToExternal(dataArray, 1);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_MonitorEvent");
        } else {
            ALOGE("service NULL");
        }
        return;
    }

    /*description: to collect algo information for every authentication */
    void Dcs::reportAuthenticatedInfo(fingerprint_auth_dcsmsg_t authenticated_msg) {

        ALOGD("DcsInfo enter into FINGERPRINT_AUTHENTICATED CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD("DcsInfo send fingerprint auth dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[FP_AUTH_DCSMSG_LEN] = {{0, 0}};
            dataArray[0].key = "auth_result";
            dataArray[0].value = getHidlstring(authenticated_msg.auth_result);
            dataArray[1].key = "fail_reason";
            dataArray[1].value = getHidlstring(authenticated_msg.fail_reason);
            dataArray[2].key = "quality_score";
            dataArray[2].value = getHidlstring(authenticated_msg.quality_score);
            dataArray[3].key = "match_score";
            dataArray[3].value = getHidlstring(authenticated_msg.match_score);
            dataArray[4].key = "signal_value";
            dataArray[4].value = getHidlstring(authenticated_msg.signal_value);
            dataArray[5].key = "img_area";
            dataArray[5].value = getHidlstring(authenticated_msg.img_area);
            dataArray[6].key = "retry_times";
            dataArray[6].value = getHidlstring(authenticated_msg.retry_times);
            dataArray[7].key = "algo_version";
            dataArray[7].value = hidl_string(authenticated_msg.algo_version, strlen(authenticated_msg.algo_version));
            dataArray[8].key = "chip_ic";
            dataArray[8].value = getHidlstring(authenticated_msg.chip_ic);
            dataArray[9].key = "module_type";
            dataArray[9].value = getHidlstring(authenticated_msg.module_type);
            dataArray[10].key = "lense_type";
            dataArray[10].value = getHidlstring(authenticated_msg.lense_type);
            dataArray[11].key = "dsp_available";
            dataArray[11].value = getHidlstring(authenticated_msg.dsp_availalbe);

            /* add performance data */
            dataArray[12].key = "auth_total_time";
            dataArray[12].value = getHidlstring(authenticated_msg.auth_total_time);
            dataArray[13].key = "ui_ready_time";
            dataArray[13].value = getHidlstring(authenticated_msg.ui_ready_time);
            dataArray[14].key = "capture_time";
            dataArray[14].value = getHidlstring(authenticated_msg.capture_time);
            dataArray[15].key = "preprocess_time";
            dataArray[15].value = getHidlstring(authenticated_msg.preprocess_time);
            dataArray[16].key = "get_feature_time";
            dataArray[16].value = getHidlstring(authenticated_msg.get_feature_time);
            dataArray[17].key = "auth_time";
            dataArray[17].value = getHidlstring(authenticated_msg.auth_time);
            dataArray[18].key = "detect_fake_time";
            dataArray[18].value = getHidlstring(authenticated_msg.detect_fake_time);

            dcsmsg.setToExternal(dataArray, FP_AUTH_DCSMSG_LEN);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_auth_dcsmsg");
        } else {
            ALOGE("service NULL");
        }
        return;
    }

    /*description: to collect algo information for every init */
    void Dcs::reportInitEventInfo(oplus_fingerprint_init_event_info_t init_event_info) {
        ALOGD("DcsInfo enter into reportInitEventInfo CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD("DcsInfo send fingerprint reportInitEventInfo dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[39] = {{0, 0}};
            dataArray[0].key = "init_event_time";
            dataArray[0].value = getHidlstring(init_event_info.init_event_time);
            dataArray[1].key = "init_result";
            dataArray[1].value = getHidlstring(init_event_info.init_result);
            dataArray[2].key = "init_fail_reason";
            dataArray[2].value = getHidlstring(init_event_info.init_fail_reason);

            //kpi_time
            dataArray[3].key = "init_time_cost_all";
            dataArray[3].value = getHidlstring(init_event_info.init_time_cost_all);
            dataArray[4].key = "init_time_cost_cdsp";
            dataArray[4].value = getHidlstring(init_event_info.init_time_cost_cdsp);
            dataArray[5].key = "init_time_cost_driver";
            dataArray[5].value = getHidlstring(init_event_info.init_time_cost_driver);
            dataArray[6].key = "init_time_cost_ta";
            dataArray[6].value = getHidlstring(init_event_info.init_time_cost_ta);
            dataArray[7].key = "lcd_type";
            dataArray[7].value = hidl_string(init_event_info.lcd_type, strlen(init_event_info.lcd_type));
            dataArray[8].key = "dsp_available";
            dataArray[8].value = getHidlstring(init_event_info.dsp_available);

            //version_info
            dataArray[9].key = "hal_version";
            dataArray[9].value = getHidlstring(init_event_info.hal_version);
            dataArray[10].key = "driver_version";
            dataArray[10].value = getHidlstring(init_event_info.driver_version);
            dataArray[11].key = "cdsp_version";
            dataArray[11].value = getHidlstring(init_event_info.cdsp_version);

            //ta_info
            dataArray[12].key = "sensor_id";
            dataArray[12].value = getHidlstring(init_event_info.int_ta_info.sensor_id);
            dataArray[13].key = "lens_type";
            dataArray[13].value = getHidlstring(init_event_info.int_ta_info.lens_type);
            dataArray[14].key = "chip_type";
            dataArray[14].value = hidl_string(init_event_info.int_ta_info.chip_type, strlen(init_event_info.int_ta_info.chip_type));
            dataArray[15].key = "factory_type";
            dataArray[15].value = hidl_string(init_event_info.int_ta_info.factory_type, strlen(init_event_info.int_ta_info.factory_type));
            //algo_version
            dataArray[16].key = "algo_version";
            dataArray[16].value = hidl_string(init_event_info.int_ta_info.algo_version, strlen(init_event_info.int_ta_info.algo_version));
            dataArray[17].key = "algo_version1";
            dataArray[17].value = getHidlstring(init_event_info.int_ta_info.algo_version1);
            dataArray[18].key = "algo_version2";
            dataArray[18].value = getHidlstring(init_event_info.int_ta_info.algo_version2);
            dataArray[19].key = "algo_version3";
            dataArray[19].value = getHidlstring(init_event_info.int_ta_info.algo_version3);
            dataArray[20].key = "algo_version4";
            dataArray[20].value = getHidlstring(init_event_info.int_ta_info.algo_version4);
            dataArray[21].key = "algo_version5";
            dataArray[21].value = getHidlstring(init_event_info.int_ta_info.algo_version5);
            //ic_status
            dataArray[22].key = "badpixel_num";
            dataArray[22].value = getHidlstring(init_event_info.int_ta_info.badpixel_num);
            dataArray[23].key = "badpixel_num_local";
            dataArray[23].value = getHidlstring(init_event_info.int_ta_info.badpixel_num_local);
            dataArray[24].key = "init_finger_number";
            dataArray[24].value = getHidlstring(init_event_info.int_ta_info.init_finger_number);
            dataArray[25].key = "template_verison";
            dataArray[25].value = getHidlstring(init_event_info.int_ta_info.template_verison);
            //template_info
            dataArray[26].key = "all_template_num";
            dataArray[26].value = getHidlstring(init_event_info.int_ta_info.all_template_num);
            //calabration_info
            dataArray[27].key = "exposure_value";
            dataArray[27].value = getHidlstring(init_event_info.int_ta_info.exposure_value);
            dataArray[28].key = "exposure_time";
            dataArray[28].value = getHidlstring(init_event_info.int_ta_info.exposure_time);
            dataArray[29].key = "calabration_signal_value";
            dataArray[29].value = getHidlstring(init_event_info.int_ta_info.calabration_signal_value);
            dataArray[30].key = "calabration_tsnr";
            dataArray[30].value = getHidlstring(init_event_info.int_ta_info.calabration_tsnr);
            dataArray[31].key = "flesh_touch_diff";
            dataArray[31].value = getHidlstring(init_event_info.int_ta_info.flesh_touch_diff);
            dataArray[32].key = "scale";
            dataArray[32].value = getHidlstring(init_event_info.int_ta_info.scale);
            dataArray[33].key = "gain";
            dataArray[33].value = getHidlstring(init_event_info.int_ta_info.gain);
            //calabration_info
            dataArray[34].key = "init_event_state1";
            dataArray[34].value = getHidlstring(init_event_info.int_ta_info.init_event_state1);
            dataArray[35].key = "init_event_state2";
            dataArray[35].value = getHidlstring(init_event_info.int_ta_info.init_event_state2);
            dataArray[36].key = "init_event_sting1";
            dataArray[36].value = hidl_string(init_event_info.int_ta_info.init_event_sting1, strlen(init_event_info.int_ta_info.init_event_sting1));
            dataArray[37].key = "init_event_sting2";
            dataArray[37].value = hidl_string(init_event_info.int_ta_info.init_event_sting2, strlen(init_event_info.int_ta_info.init_event_sting2));

            //add_info
            dataArray[38].key = "pid_info";
            dataArray[38].value = getHidlstring(init_event_info.pid_info);

            dcsmsg.setToExternal(dataArray, 39);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_init_event");
        } else {
            ALOGE("service NULL");
        }
        return;
    }

    void Dcs::reportAuthEventInfo(oplus_fingerprint_auth_event_info_t auth_event_info) {
        ALOGD("DcsInfo enter into FINGERPRINT_AUTHENTICATED CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD("DcsInfo send fingerprint auth dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[64] = {{0, 0}};
            dataArray[0].key = "auth_event_time";
            dataArray[0].value = getHidlstring(auth_event_info.auth_event_time);
            dataArray[1].key = "auth_type";
            dataArray[1].value = getHidlstring(auth_event_info.auth_type);
            dataArray[2].key = "auth_result";
            dataArray[2].value = getHidlstring(auth_event_info.auth_result);
            dataArray[3].key = "dsp_available";
            dataArray[3].value = getHidlstring(auth_event_info.dsp_available);
            dataArray[4].key = "retry_times";
            dataArray[4].value = getHidlstring(auth_event_info.retry_times);

            dataArray[5].key = "continuous_authsuccess_count";
            dataArray[5].value = getHidlstring(auth_event_info.continuous_authsuccess_count);
            dataArray[6].key = "continuous_authfail_count";
            dataArray[6].value = getHidlstring(auth_event_info.continuous_authfail_count);
            dataArray[7].key = "continuous_badauth_count";
            dataArray[7].value = getHidlstring(auth_event_info.continuous_badauth_count);
            dataArray[8].key = "user_gid";
            dataArray[8].value = getHidlstring(auth_event_info.auth_event_time);
            dataArray[10].key = "screen_state";
            dataArray[10].value = getHidlstring(auth_event_info.screen_state);
            dataArray[11].key = "fingerprintid";
            dataArray[11].value = getHidlstring(auth_event_info.fingerprintid);
            dataArray[12].key = "pid_info";
            dataArray[12].value = getHidlstring(auth_event_info.pid_info);
            dataArray[13].key = "captureimg_retry_count";
            dataArray[13].value = getHidlstring(auth_event_info.captureimg_retry_count);
            dataArray[14].key = "captureimg_retry_reason";
            dataArray[14].value = getHidlstring(auth_event_info.captureimg_retry_reason);

            //kpi_info
            dataArray[15].key = "auth_total_time";
            dataArray[15].value = getHidlstring(auth_event_info.auth_total_time);
            dataArray[16].key = "ui_ready_time";
            dataArray[16].value = getHidlstring(auth_event_info.ui_ready_time);
            //tp_info
            dataArray[17].key = "press_x";
            dataArray[17].value = getHidlstring(auth_event_info.pressxy[0]);
            dataArray[18].key = "press_y";
            dataArray[18].value = getHidlstring(auth_event_info.pressxy[1]);
            dataArray[19].key = "area_rate";
            dataArray[19].value = getHidlstring(auth_event_info.area_rate);
            //lcd_info
            dataArray[20].key = "brightness_value";
            dataArray[20].value = getHidlstring(auth_event_info.brightness_value);
            dataArray[21].key = "lcd_type";
            dataArray[21].value = hidl_string(auth_event_info.lcd_type, strlen(auth_event_info.lcd_type));
            //version_info
            dataArray[22].key = "hal_version";
            dataArray[22].value = getHidlstring(auth_event_info.hal_version);
            dataArray[23].key = "driver_version";
            dataArray[23].value = getHidlstring(auth_event_info.driver_version);
            dataArray[24].key = "cdsp_version";
            dataArray[24].value = getHidlstring(auth_event_info.cdsp_version);

            //------------ta_info-----------
            dataArray[25].key = "fail_reason";
            dataArray[25].value = getHidlstring(auth_event_info.auth_ta_info.fail_reason);
            dataArray[26].key = "fail_reason_retry[0]";
            dataArray[26].value = getHidlstring(auth_event_info.auth_ta_info.fail_reason_retry[0]);
            dataArray[27].key = "fail_reason_retry[1]";
            dataArray[27].value = getHidlstring(auth_event_info.auth_ta_info.fail_reason_retry[1]);
            dataArray[28].key = "fail_reason_retry[2]";
            dataArray[28].value = getHidlstring(auth_event_info.auth_ta_info.fail_reason_retry[2]);
            dataArray[29].key = "algo_version";
            dataArray[29].value = hidl_string(auth_event_info.auth_ta_info.algo_version, strlen(auth_event_info.auth_ta_info.algo_version));

            //img_info
            dataArray[30].key = "quality_score";
            dataArray[30].value = getHidlstring(auth_event_info.auth_ta_info.quality_score);
            dataArray[31].key = "match_score";
            dataArray[31].value = getHidlstring(auth_event_info.auth_ta_info.match_score);
            dataArray[32].key = "signal_value";
            dataArray[32].value = getHidlstring(auth_event_info.auth_ta_info.signal_value);
            dataArray[33].key = "img_area";
            dataArray[33].value = getHidlstring(auth_event_info.auth_ta_info.img_area);
            dataArray[34].key = "img_direction";
            dataArray[34].value = getHidlstring(auth_event_info.auth_ta_info.img_direction);

            dataArray[35].key = "finger_type";
            dataArray[35].value = getHidlstring(auth_event_info.auth_ta_info.finger_type);
            dataArray[36].key = "ta_retry_times";
            dataArray[36].value = getHidlstring(auth_event_info.auth_ta_info.ta_retry_times);
            dataArray[37].key = "recog_round";
            dataArray[37].value = getHidlstring(auth_event_info.auth_ta_info.recog_round);
            dataArray[38].key = "exposure_flag";
            dataArray[38].value = getHidlstring(auth_event_info.auth_ta_info.exposure_flag);
            dataArray[39].key = "study_flag";
            dataArray[39].value = getHidlstring(auth_event_info.auth_ta_info.study_flag);

            dataArray[40].key = "fdt_base_flag";
            dataArray[40].value = getHidlstring(auth_event_info.auth_ta_info.fdt_base_flag);
            dataArray[41].key = "image_base_flag";
            dataArray[41].value = getHidlstring(auth_event_info.auth_ta_info.image_base_flag);
            dataArray[42].key = "finger_number";
            dataArray[42].value = getHidlstring(auth_event_info.auth_ta_info.finger_number);
            dataArray[43].key = "errtouch_flag";
            dataArray[43].value = getHidlstring(auth_event_info.auth_ta_info.errtouch_flag);
            dataArray[44].key = "memory_info";
            dataArray[44].value = getHidlstring(auth_event_info.auth_ta_info.memory_info);

            dataArray[45].key = "screen_protector_type";
            dataArray[45].value = getHidlstring(auth_event_info.auth_ta_info.screen_protector_type);
            dataArray[46].key = "touch_diff";
            dataArray[46].value = getHidlstring(auth_event_info.auth_ta_info.touch_diff);
            dataArray[47].key = "mp_touch_diff";
            dataArray[47].value = getHidlstring(auth_event_info.auth_ta_info.mp_touch_diff);
            dataArray[48].key = "fake_result";
            dataArray[48].value = getHidlstring(auth_event_info.auth_ta_info.fake_result);
            //rawdata_info
            dataArray[49].key = "auth_rawdata[0]";
            dataArray[49].value = getHidlstring(auth_event_info.auth_ta_info.auth_rawdata[0]);
            dataArray[50].key = "auth_rawdata[1]";
            dataArray[50].value = getHidlstring(auth_event_info.auth_ta_info.auth_rawdata[1]);
            dataArray[51].key = "auth_rawdata[2]";
            dataArray[51].value = getHidlstring(auth_event_info.auth_ta_info.auth_rawdata[2]);

            //template_info
            dataArray[52].key = "all_template_num";
            dataArray[52].value = getHidlstring(auth_event_info.auth_ta_info.all_template_num);

            //kpi_info
            dataArray[53].key = "capture_time[0]";
            dataArray[53].value = getHidlstring(auth_event_info.auth_ta_info.capture_time[0]);
            dataArray[54].key = "preprocess_time[0]";
            dataArray[54].value = getHidlstring(auth_event_info.auth_ta_info.preprocess_time[0]);
            dataArray[55].key = "get_feature_time[0]";
            dataArray[55].value = getHidlstring(auth_event_info.auth_ta_info.get_feature_time[0]);
            dataArray[56].key = "auth_time[0]";
            dataArray[56].value = getHidlstring(auth_event_info.auth_ta_info.auth_time[0]);
            dataArray[57].key = "detect_fake_time[0]";
            dataArray[57].value = getHidlstring(auth_event_info.auth_ta_info.detect_fake_time[0]);
            dataArray[58].key = "kpi_time_all[0]";
            dataArray[58].value = getHidlstring(auth_event_info.auth_ta_info.kpi_time_all[0]);
            dataArray[59].key = "study_time";
            dataArray[59].value = getHidlstring(auth_event_info.auth_ta_info.study_time);

            //bak_info
            dataArray[60].key = "auth_event_state1";
            dataArray[60].value = getHidlstring(auth_event_info.auth_ta_info.auth_event_state1);
            dataArray[61].key = "auth_event_state2";
            dataArray[61].value = getHidlstring(auth_event_info.auth_ta_info.auth_event_state2);
            dataArray[62].key = "auth_event_string1";
            dataArray[62].value = hidl_string(auth_event_info.auth_ta_info.auth_event_string1, strlen(auth_event_info.auth_ta_info.auth_event_string1));
            dataArray[63].key = "auth_event_string2";
            dataArray[63].value = hidl_string(auth_event_info.auth_ta_info.auth_event_string2, strlen(auth_event_info.auth_ta_info.auth_event_string2));

            //dcs_end
            dcsmsg.setToExternal(dataArray, 64);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_auth_dcsmsg");
        } else {
            ALOGE("service NULL");
        }
        return;
    }

    void Dcs::reportSingleEnrollEventInfo(oplus_fingerprint_singleenroll_event_info_t singleenroll_event_info) {
        ALOGD("DcsInfo enter into reportSingleEnrollEventInfo CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD("DcsInfo send reportSingleEnrollEventInfo dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[40] = {{0, 0}};
            dataArray[0].key = "singleenroll_event_time";
            dataArray[0].value = getHidlstring(singleenroll_event_info.singleenroll_event_time);
            dataArray[1].key = "singleenroll_result";
            dataArray[1].value = getHidlstring(singleenroll_event_info.singleenroll_result);
            dataArray[2].key = "user_gid";
            dataArray[2].value = getHidlstring(singleenroll_event_info.user_gid);
            //kpi_info
            dataArray[3].key = "singleenroll_total_time";
            dataArray[3].value = getHidlstring(singleenroll_event_info.singleenroll_total_time);
            dataArray[4].key = "ui_ready_time";
            dataArray[4].value = getHidlstring(singleenroll_event_info.ui_ready_time);
            //tp_info
            dataArray[5].key = "pressxy[0]";
            dataArray[5].value = getHidlstring(singleenroll_event_info.pressxy[0]);
            dataArray[6].key = "pressxy[1]";
            dataArray[6].value = getHidlstring(singleenroll_event_info.pressxy[1]);
            dataArray[7].key = "area_rate";
            dataArray[7].value = getHidlstring(singleenroll_event_info.area_rate);
            //lcd_info
            dataArray[8].key = "brightness_value";
            dataArray[8].value = getHidlstring(singleenroll_event_info.brightness_value);
            dataArray[9].key = "lcd_type";
            dataArray[9].value = hidl_string(singleenroll_event_info.lcd_type, strlen(singleenroll_event_info.lcd_type));

            //------------ta_info-----------
            dataArray[10].key = "fail_reason";
            dataArray[10].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.fail_reason);
            dataArray[11].key = "fail_reason_param1";
            dataArray[11].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.fail_reason_param1);
            dataArray[12].key = "fail_reason_param2";
            dataArray[12].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.fail_reason_param2);
            dataArray[13].key = "algo_version";
            dataArray[13].value = hidl_string(singleenroll_event_info.singleenroll_ta_info.algo_version,
                strlen(singleenroll_event_info.singleenroll_ta_info.algo_version));
            dataArray[14].key = "current_enroll_times";
            dataArray[14].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.fail_reason_param1);

            //img_info
            dataArray[15].key = "quality_score";
            dataArray[15].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.quality_score);
            dataArray[16].key = "signal_value";
            dataArray[16].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.signal_value);
            dataArray[17].key = "img_area";
            dataArray[17].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.img_area);
            dataArray[18].key = "img_direction";
            dataArray[18].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.img_direction);
            dataArray[19].key = "finger_type";
            dataArray[19].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.finger_type);

            dataArray[20].key = "ta_retry_times";
            dataArray[20].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.ta_retry_times);
            dataArray[21].key = "exposure_flag";
            dataArray[21].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.exposure_flag);
            dataArray[22].key = "fdt_base_flag";
            dataArray[22].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.fdt_base_flag);
            dataArray[23].key = "image_base_flag";
            dataArray[23].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.image_base_flag);
            dataArray[24].key = "repetition_rate";
            dataArray[24].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.repetition_rate);

            dataArray[25].key = "enroll_rawdata";
            dataArray[25].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.enroll_rawdata);
            dataArray[26].key = "anomaly_flag";
            dataArray[26].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.anomaly_flag);
            dataArray[27].key = "screen_protector_type";
            dataArray[27].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.screen_protector_type);
            dataArray[28].key = "key_point_num";
            dataArray[28].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.key_point_num);
            dataArray[29].key = "increase_rate";
            dataArray[29].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.increase_rate);

            //kpi_info
            dataArray[30].key = "capture_time";
            dataArray[30].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.capture_time);
            dataArray[31].key = "preprocess_time";
            dataArray[31].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.preprocess_time);
            dataArray[32].key = "get_feature_time";
            dataArray[32].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.get_feature_time);
            dataArray[33].key = "enroll_time";
            dataArray[33].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.enroll_time);
            dataArray[34].key = "detect_fake_time";
            dataArray[34].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.detect_fake_time);
            dataArray[35].key = "kpi_time_all";
            dataArray[35].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.kpi_time_all);

            //bak_info
            dataArray[36].key = "singleenroll_event_state1";
            dataArray[36].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.singleenroll_event_state1);
            dataArray[37].key = "singleenroll_event_state2";
            dataArray[37].value = getHidlstring(singleenroll_event_info.singleenroll_ta_info.singleenroll_event_state2);
            dataArray[38].key = "singleenroll_event_string1";
            dataArray[38].value = hidl_string(singleenroll_event_info.singleenroll_ta_info.singleenroll_event_string1,
                strlen(singleenroll_event_info.singleenroll_ta_info.singleenroll_event_string1));
            dataArray[39].key = "singleenroll_event_string2";
            dataArray[39].value = hidl_string(singleenroll_event_info.singleenroll_ta_info.singleenroll_event_string2,
                strlen(singleenroll_event_info.singleenroll_ta_info.singleenroll_event_string2));

            dcsmsg.setToExternal(dataArray, 40);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_singleenroll_event");
        } else {
            ALOGE("service NULL");
        }
        return;
    }

    void Dcs::reportEnrollEventInfo(oplus_fingerprint_enroll_event_info_t enroll_event_info) {
        ALOGD("DcsInfo enter into reportEnrollEventInfo CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD("DcsInfo send reportEnrollEventInfo dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[18] = {{0, 0}};
            dataArray[0].key = "enroll_event_time";
            dataArray[0].value = getHidlstring(enroll_event_info.enroll_event_time);
            dataArray[1].key = "enroll_result";
            dataArray[1].value = getHidlstring(enroll_event_info.enroll_result);
            dataArray[2].key = "user_gid";
            dataArray[2].value = getHidlstring(enroll_event_info.user_gid);
            dataArray[3].key = "total_press_times";
            dataArray[3].value = getHidlstring(enroll_event_info.total_press_times);
            dataArray[4].key = "fingerprintid";
            dataArray[4].value = getHidlstring(enroll_event_info.fingerprintid);
            dataArray[5].key = "pid_info";
            dataArray[5].value = getHidlstring(enroll_event_info.pid_info);
            dataArray[6].key = "lcd_type";
            dataArray[6].value = hidl_string(enroll_event_info.lcd_type, strlen(enroll_event_info.lcd_type));

            //------------ta_info-----------
            dataArray[7].key = "enroll_reason";
            dataArray[7].value = getHidlstring(enroll_event_info.enroll_ta_info.enroll_reason);
            dataArray[8].key = "cdsp_flag";
            dataArray[8].value = getHidlstring(enroll_event_info.enroll_ta_info.cdsp_flag);
            dataArray[9].key = "repetition_enroll_number";
            dataArray[9].value = getHidlstring(enroll_event_info.enroll_ta_info.repetition_enroll_number);
            dataArray[10].key = "total_enroll_times";
            dataArray[10].value = getHidlstring(enroll_event_info.enroll_ta_info.total_enroll_times);
            dataArray[11].key = "finger_number";
            dataArray[11].value = getHidlstring(enroll_event_info.enroll_ta_info.finger_number);
            //version_info
            dataArray[12].key = "algo_version";
            dataArray[12].value = hidl_string(enroll_event_info.enroll_ta_info.algo_version,
                strlen(enroll_event_info.enroll_ta_info.algo_version));
            dataArray[13].key = "template_version";
            dataArray[13].value = getHidlstring(enroll_event_info.enroll_ta_info.template_version);
            //bak_info
            dataArray[14].key = "enroll_event_state1";
            dataArray[14].value = getHidlstring(enroll_event_info.enroll_ta_info.enroll_event_state1);
            dataArray[15].key = "enroll_event_state2";
            dataArray[15].value = getHidlstring(enroll_event_info.enroll_ta_info.enroll_event_state2);
            dataArray[16].key = "enroll_event_string1";
            dataArray[16].value = hidl_string(enroll_event_info.enroll_ta_info.enroll_event_string1,
                strlen(enroll_event_info.enroll_ta_info.enroll_event_string1));
            dataArray[17].key = "enroll_event_string2";
            dataArray[17].value = hidl_string(enroll_event_info.enroll_ta_info.enroll_event_string2,
                strlen(enroll_event_info.enroll_ta_info.enroll_event_string2));

            dcsmsg.setToExternal(dataArray, 18);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_enroll_event");
        } else {
            ALOGE("service NULL");
        }
        return;
    }

    void Dcs::reportSpecialEventInfo(oplus_fingerprint_special_event_info_t special_event_info) {
        ALOGD("DcsInfo enter into FINGERPRINT_AUTHENTICATED CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD("DcsInfo send fingerprint auth dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[16] = {{0, 0}};
            dataArray[0].key = "event_time";
            dataArray[0].value = getHidlstring(special_event_info.event_time);
            dataArray[1].key = "event_type";
            dataArray[1].value = getHidlstring(special_event_info.event_type);
            dataArray[2].key = "event_trigger_flag";
            dataArray[2].value = getHidlstring(special_event_info.event_trigger_flag);
            dataArray[3].key = "event_reason";
            dataArray[3].value = getHidlstring(special_event_info.event_reason);
            dataArray[4].key = "event_count";
            dataArray[4].value = getHidlstring(special_event_info.event_count);
            dataArray[5].key = "lcd_type";
            dataArray[5].value = hidl_string(special_event_info.lcd_type, strlen(special_event_info.lcd_type));

            dataArray[6].key = "hal_version";
            dataArray[6].value = getHidlstring(special_event_info.hal_version);
            dataArray[7].key = "algo_version";
            dataArray[7].value = hidl_string(special_event_info.algo_version, strlen(special_event_info.algo_version));
            dataArray[8].key = "user_gid";
            dataArray[8].value = getHidlstring(special_event_info.user_gid);
            dataArray[9].key = "pid_info";
            dataArray[9].value = getHidlstring(special_event_info.pid_info);

            //fail_reason
            dataArray[10].key = "special_event_state1";
            dataArray[10].value = getHidlstring(special_event_info.special_event_state1);
            dataArray[11].key = "special_event_state2";
            dataArray[11].value = getHidlstring(special_event_info.special_event_state2);
            dataArray[12].key = "special_event_state3";
            dataArray[12].value = getHidlstring(special_event_info.special_event_state3);
            dataArray[13].key = "special_event_state4";
            dataArray[13].value = getHidlstring(special_event_info.special_event_state4);
            dataArray[14].key = "special_event_string1";
            dataArray[14].value = hidl_string(special_event_info.special_event_string1, strlen(special_event_info.special_event_string1));
            dataArray[15].key = "special_event_string2";
            dataArray[15].value = hidl_string(special_event_info.special_event_string2, strlen(special_event_info.special_event_string2));

            dcsmsg.setToExternal(dataArray, 16);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_special_event");
        } else {
            ALOGE("service NULL");
        }
        return;
    }


    /*description: to collect algo information for every Dcs event */
    void Dcs::reportDcsEventInfo(oplus_fingerprint_dcs_info_t dcs_info) {
        ALOGD("DcsInfo enter into reportDcsEventInfo");
        switch (dcs_info.dcs_type) {
            case DCS_INTI_EVENT_INFO: {
                reportInitEventInfo(dcs_info.data.init_event_info);
            }
                break;

            case DCS_AUTH_EVENT_INFO: {
                reportAuthEventInfo(dcs_info.data.auth_event_info);
            }
                break;

            case DCS_SINGLEENROLL_EVENT_INFO: {
                reportSingleEnrollEventInfo(dcs_info.data.singleenroll_event_info);
            }
                break;

            case DCS_ENROLL_EVENT_INFO: {
                reportEnrollEventInfo(dcs_info.data.enroll_event_info);
            }
                break;

            case DCS_SPECIAL_EVENT_INFO: {
                reportSpecialEventInfo(dcs_info.data.special_event_info);
            }
                break;

            case DCS_DEFAULT_EVENT_INFO: {
                ALOGE("do nothing for DCS_DEFAULT_EVENT_INFO ");
            }
                break;

            default: {
                ALOGE("Error; enter into default dcs_info->type");
            }
                break;
        }
    }

}; // namespace android
