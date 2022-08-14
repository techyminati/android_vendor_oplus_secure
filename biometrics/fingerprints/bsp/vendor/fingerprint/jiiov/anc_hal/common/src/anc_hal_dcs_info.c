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
#include "anc_hal_manager.h"
#include "anc_log.h"
#include "extension_command.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <log/log.h>
#include <time.h>
#include <sys/time.h>
//#include <ctime.h>
#include <sys/resource.h>
#include <cutils/properties.h>
#include <errno.h>


// #include "anc_common_type.h"
// #include "anc_log.h"
// #include "anc_command.h"
// #include "anc_ca.h"
// #include "anc_algorithm.h"
// #include "anc_tac_sensor.h"
// #include "sensor_command_param.h"
// #include "anc_token.h"
// #include "anc_extension.h"
// #include "anc_hal_extension_command.h"
// #include "anc_hal_sensor_device.h"
// #include "anc_tac_time.h"
#include "anc_hal_dcs_info.h"

extern AncFTAlgoTimeInfo g_time_info;

    int32_t printAuthEventInfo(oplus_fingerprint_auth_event_info_t print_auth_event_info) {
        oplus_fingerprint_auth_ta_info_t* print_auth_ta_info = &print_auth_event_info.auth_ta_info;

        ANC_LOGD("--------------- [printAllAuthEventInfo] print start ---------------");
        ANC_LOGD("--------------- [printAllAuthEventInfo] hal info ---------------");

        ANC_LOGD("[printAllAuthEventInfo] auth_event_time =%d, ", print_auth_event_info.auth_event_time);
        ANC_LOGD("[printAllAuthEventInfo] auth_type =%d, ", print_auth_event_info.auth_type);
        ANC_LOGD("[printAllAuthEventInfo] auth_result =%d, ", print_auth_event_info.auth_result);
        ANC_LOGD("[printAllAuthEventInfo] dsp_available =%d, ", print_auth_event_info.dsp_available);
        ANC_LOGD("[printAllAuthEventInfo] retry_times =%d, ", print_auth_event_info.retry_times);
        ANC_LOGD("[printAllAuthEventInfo] continuous_authsuccess_count =%d, ", print_auth_event_info.continuous_authsuccess_count);
        ANC_LOGD("[printAllAuthEventInfo] continuous_authfail_count =%d, ", print_auth_event_info.continuous_authfail_count);
        ANC_LOGD("[printAllAuthEventInfo] continuous_badauth_count =%d, ", print_auth_event_info.continuous_badauth_count);

        ANC_LOGD("[printAllAuthEventInfo] user_gid =%d, ", print_auth_event_info.user_gid);
        ANC_LOGD("[printAllAuthEventInfo] screen_state =%d, ", print_auth_event_info.screen_state);
        ANC_LOGD("[printAllAuthEventInfo] fingerprintid =%d, ", print_auth_event_info.fingerprintid);
        ANC_LOGD("[printAllAuthEventInfo] pid_info =%d, ", print_auth_event_info.pid_info);
        ANC_LOGD("[printAllAuthEventInfo] captureimg_retry_count =%d, ", print_auth_event_info.captureimg_retry_count);
        ANC_LOGD("[printAllAuthEventInfo] captureimg_retry_reason =%d, ", print_auth_event_info.captureimg_retry_reason);

        ANC_LOGD("[printAllAuthEventInfo] auth_total_time =%d, ", print_auth_event_info.auth_total_time);
        ANC_LOGD("[printAllAuthEventInfo] ui_ready_time =%d, ", print_auth_event_info.ui_ready_time);
        ANC_LOGD("[printAllAuthEventInfo] pressxy[0] =%d pressxy[1] =%d  , ", print_auth_event_info.pressxy[0], print_auth_event_info.pressxy[1]);
        ANC_LOGD("[printAllAuthEventInfo] area_rate =%d, ", print_auth_event_info.area_rate);
        ANC_LOGD("[printAllAuthEventInfo] brightness_value =%d, ", print_auth_event_info.brightness_value);
        ANC_LOGD("[printAllAuthEventInfo] lcd_type =%s, ", print_auth_event_info.lcd_type);
        ANC_LOGD("[printAllAuthEventInfo] hal_version =%d, ", print_auth_event_info.hal_version);
        ANC_LOGD("[printAllAuthEventInfo] driver_version =%d, ", print_auth_event_info.driver_version);
        ANC_LOGD("[printAllAuthEventInfo] cdsp_version =%d, ", print_auth_event_info.cdsp_version);

        //------ta info------
        //------base_info------
        ANC_LOGD("--------------- [printAllAuthEventInfo] ta info ---------------");
        ANC_LOGD("[printAllAuthEventInfo] fail_reason =%d, ", print_auth_ta_info->fail_reason);
        ANC_LOGD("[printAllAuthEventInfo] fail_reason_retry= %d,   %d,  %d", print_auth_ta_info->fail_reason_retry[0],
            print_auth_ta_info->fail_reason_retry[1], print_auth_ta_info->fail_reason_retry[2]);
        ANC_LOGD("[printAllAuthEventInfo] algo_version =%s, ", print_auth_ta_info->algo_version);

        //img_info
        ANC_LOGD("[printAllAuthEventInfo] quality_score =%d, ", print_auth_ta_info->quality_score);
        ANC_LOGD("[printAllAuthEventInfo] match_score =%d, ", print_auth_ta_info->match_score);
        ANC_LOGD("[printAllAuthEventInfo] signal_value =%d, ", print_auth_ta_info->signal_value);
        ANC_LOGD("[printAllAuthEventInfo] img_area =%d, ", print_auth_ta_info->img_area);
        ANC_LOGD("[printAllAuthEventInfo] img_direction =%d, ", print_auth_ta_info->img_direction);

        ANC_LOGD("[printAllAuthEventInfo] finger_type =%d, ", print_auth_ta_info->finger_type);
        ANC_LOGD("[printAllAuthEventInfo] ta_retry_times =%d, ", print_auth_ta_info->ta_retry_times);// G3s: set to 0
        ANC_LOGD("[printAllAuthEventInfo] recog_round =%d, ", print_auth_ta_info->recog_round);
        ANC_LOGD("[printAllAuthEventInfo] exposure_flag =%d, ", print_auth_ta_info->exposure_flag);
        ANC_LOGD("[printAllAuthEventInfo] study_flag =%d, ", print_auth_ta_info->study_flag);

        ANC_LOGD("[printAllAuthEventInfo] fdt_base_flag =%d, ", print_auth_ta_info->fdt_base_flag);
        ANC_LOGD("[printAllAuthEventInfo] image_base_flag =%d, ", print_auth_ta_info->image_base_flag);
        ANC_LOGD("[printAllAuthEventInfo] finger_number =%d, ", print_auth_ta_info->finger_number);
        ANC_LOGD("[printAllAuthEventInfo] errtouch_flag =%d, ", print_auth_ta_info->errtouch_flag);
        ANC_LOGD("[printAllAuthEventInfo] memory_info =%d, ", print_auth_ta_info->memory_info);

        ANC_LOGD("[printAllAuthEventInfo] screen_protector_type =%d, ", print_auth_ta_info->screen_protector_type);
        ANC_LOGD("[printAllAuthEventInfo] touch_diff =%d, ", print_auth_ta_info->touch_diff);
        ANC_LOGD("[printAllAuthEventInfo] mp_touch_diff =%d, ", print_auth_ta_info->mp_touch_diff);
        ANC_LOGD("[printAllAuthEventInfo] fake_result =%d, ", print_auth_ta_info->fake_result);

        //rawdata_info
        ANC_LOGD("[printAllAuthEventInfo] auth_rawdata= %d,   %d,  %d", print_auth_ta_info->auth_rawdata[0],
            print_auth_ta_info->auth_rawdata[1], print_auth_ta_info->auth_rawdata[2]);

        //template_info
        ANC_LOGD("[printAllAuthEventInfo] one_finger_template_num= %d,  %d,  %d,  %d,  %d", print_auth_ta_info->one_finger_template_num[0],
            print_auth_ta_info->one_finger_template_num[1], print_auth_ta_info->one_finger_template_num[2],
            print_auth_ta_info->one_finger_template_num[3], print_auth_ta_info->one_finger_template_num[4]);
        ANC_LOGD("[printAllAuthEventInfo] all_template_num =%d, ", print_auth_ta_info->all_template_num);

        //kpi_info
        ANC_LOGD("[printAllAuthEventInfo] capture_time= %d,  %d,   %d,  %d", print_auth_ta_info->capture_time[0],
            print_auth_ta_info->capture_time[1], print_auth_ta_info->capture_time[2], print_auth_ta_info->capture_time[3]);
        ANC_LOGD("[printAllAuthEventInfo] preprocess_time= %d,  %d,   %d,  %d", print_auth_ta_info->preprocess_time[0],
            print_auth_ta_info->preprocess_time[1], print_auth_ta_info->preprocess_time[2], print_auth_ta_info->preprocess_time[3]);
        ANC_LOGD("[printAllAuthEventInfo] get_feature_time= %d,  %d,   %d,  %d", print_auth_ta_info->get_feature_time[0],
            print_auth_ta_info->get_feature_time[1], print_auth_ta_info->get_feature_time[2], print_auth_ta_info->get_feature_time[3]);
        ANC_LOGD("[printAllAuthEventInfo] auth_time= %d,  %d,   %d,  %d", print_auth_ta_info->auth_time[0],
            print_auth_ta_info->auth_time[1], print_auth_ta_info->auth_time[2], print_auth_ta_info->auth_time[3]);
        ANC_LOGD("[printAllAuthEventInfo] detect_fake_time= %d,  %d,   %d,  %d", print_auth_ta_info->detect_fake_time[0],
            print_auth_ta_info->detect_fake_time[1], print_auth_ta_info->detect_fake_time[2], print_auth_ta_info->detect_fake_time[3]);
        ANC_LOGD("[printAllAuthEventInfo] kpi_time_all= %d,  %d,   %d,  %d", print_auth_ta_info->kpi_time_all[0],
            print_auth_ta_info->kpi_time_all[1], print_auth_ta_info->kpi_time_all[2], print_auth_ta_info->kpi_time_all[3]);
        ANC_LOGD("[printAllAuthEventInfo] study_time =%d, ", print_auth_ta_info->study_time);

        //bak_info
        ANC_LOGD("[printAllAuthEventInfo] auth_event_state1 =%d ", print_auth_ta_info->auth_event_state1);
        ANC_LOGD("[printAllAuthEventInfo] auth_event_state2 =%d ", print_auth_ta_info->auth_event_state2);
        ANC_LOGD("[printAllAuthEventInfo] auth_event_string1 =%s ", print_auth_ta_info->auth_event_string1);
        ANC_LOGD("[printAllAuthEventInfo] auth_event_string2 =%s ", print_auth_ta_info->auth_event_string2);

        ANC_LOGD("--------------- [printAllAuthEventInfo] print end ---------------");
        return 0;
    }


    int32_t getDcsEventTime(int32_t* event_times)
    {
        int err = 0;
        struct timeval tv;
        memset(&tv, 0, sizeof(struct timeval));
        struct tm current_tm;
        memset(&current_tm, 0, sizeof(struct tm));
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &current_tm);
        *event_times = current_tm.tm_hour*100 + current_tm.tm_min;
        ANC_LOGD("[getDcsEventTime], current_tm.tm_year  =%d ,current_tm.tm_mon =%d,%d,%d,%d,%d" ,
        current_tm.tm_year, current_tm.tm_mon, current_tm.tm_mday, current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec);
        ANC_LOGD("[getDcsEventTime], event_times  =%d " , *event_times);
        return err;
    }

    int32_t getDcsBrightnessValue(uint32_t* brightness_value)
    {
        int32_t err = 0;
        char buf[50] = {'\0'};
        int32_t length = 0;
        uint32_t bright_value = 0;
        ANC_LOGD("getDcsBrightnessValue start ");
        char* brightness_path[] = {
                "/sys/class/backlight/panel0-backlight/brightness",
                "/sys/kernel/oplus_display/oplus_brightness",// for mtk to check
                };
        int index = 0;
        int tBrightness_path_num  = sizeof(brightness_path)/sizeof(brightness_path[0]);
        for (index = 0; index < tBrightness_path_num; index ++) {
            if (access(brightness_path[index], 0) == 0) {
                ANC_LOGE(LOG_TAG, "Brightness path index %d, path:%s", index, brightness_path[index]);
                break;
            }
        }
        if (index == tBrightness_path_num) {
            ANC_LOGE(LOG_TAG, "no brightness path available");
            err = -1;
            return err;
        }

        int fd = open(brightness_path[index], O_RDONLY);
        if (fd < 0) {
            ANC_LOGE("getBrightness openerr :%d, errno =%d", fd, errno);
            err = -1;
            return err;
        }
        length = (int32_t)read(fd, buf, sizeof(buf));
        if (length > 0) {
            bright_value = (uint32_t)atoi(buf);
        } else {
            ANC_LOGE("read failed.length =%d, errno =%d", length, errno);
        }
        *brightness_value = bright_value;
        close(fd);
        ANC_LOGD("getDcsBrightnessValue end, bright_value = %d, brightness_value =%d", bright_value, *brightness_value);
        return err;
    }

    int32_t getDcsLcdType(char value[PROPERTY_VALUE_MAX])
    {
        int32_t err = 0;

        //char value[PROPERTY_VALUE_MAX] = { 0 };
        err = property_get("persist.vendor.fingerprint.optical.lcdtype", value, "0");
        if (err <= 0) {
            ANC_LOGD("[%s] property not set.", __func__);
            return err;
        }
        return err;
    }

    int32_t getDcsHalVersion(int32_t* hal_version)
    {
        *hal_version = 104;
        return 0;
    }

    // int DcsInfo::getDcsDriverVersion(int32_t* driver_version)
    // {
    //     FUNC_ENTER();
    //     *driver_version = 0x11;
    //     return 0;
    // }

    // int DcsInfo::getDcsCdspVersion(int32_t* cdsp_version)
    // {
    //     FUNC_ENTER();
    //     *cdsp_version = 0x11;
    //     return 0;
    // }

    int32_t getDcsPidInfo(int32_t* pid_info)
    {
        int32_t err = 0;
        *pid_info = (int32_t)getpid();
        ANC_LOGD("[getDcsPidInfo] pid_info =%d", *pid_info);
        return err;
    }

    // int DcsInfo::sendDcsInitEventInfo(HalContext* context)
    // {
    //     gf_error_t err = GF_SUCCESS;
    //     ANC_LOGD(/ "[sendDcsInitEventInfo] start");
    //     oplus_fingerprint_init_event_info_t* init_event_info = &context->mDcsInfo->init_event_info;
    //     if (context->mFingerprintCore->mNotify == nullptr) {
    //         err = GF_ERROR_BAD_PARAMS;
    //         return err;
    //     }

    //     ANC_LOGD(/ "[] oplus_dcs_event_ta_cmd_t =%d, ", sizeof(oplus_dcs_event_ta_cmd_t));
    //     ANC_LOGD(/ "[] gf_cmd_header_t =%d, ", sizeof(gf_cmd_header_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_dcs_event_type_t =%d, ", sizeof(oplus_fingerprint_dcs_event_type_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_dcs_ta_cmd_info_t =%d, ", sizeof(oplus_fingerprint_dcs_ta_cmd_info_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_init_ta_info_t =%d, ", sizeof(oplus_fingerprint_init_ta_info_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_auth_ta_info->t =%d, ", sizeof(oplus_fingerprint_auth_ta_info->t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_singleenroll_ta_info_t =%d, ", sizeof(oplus_fingerprint_singleenroll_ta_info_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_enroll_ta_info_t =%d, ", sizeof(oplus_fingerprint_enroll_ta_info_t));
    //     ANC_LOGD(/ "[] fingerprint_auth_dcsmsg_t =%d, ", sizeof(fingerprint_auth_dcsmsg_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_init_event_info_t =%d, ", sizeof(oplus_fingerprint_init_event_info_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_auth_event_info.t =%d, ", sizeof(oplus_fingerprint_auth_event_info.t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_singleenroll_event_info_t =%d, ", sizeof(oplus_fingerprint_singleenroll_event_info_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_enroll_event_info_t =%d, ", sizeof(oplus_fingerprint_enroll_event_info_t));
    //     ANC_LOGD(/ "[] oplus_fingerprint_special_event_info_t =%d, ", sizeof(oplus_fingerprint_special_event_info_t));

    //     do
    //     {
    //         oplus_dcs_event_ta_cmd_t cmd;
    //         memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
    //         cmd.header.target = GF_TARGET_ALGO;
    //         cmd.header.cmd_id = GF_CMD_ALGO_BIG_DATA;
    //         cmd.dcs_ta_cmd_info.dcs_type = DCS_INTI_EVENT_INFO;
    //         err = invokeCommand(&cmd, sizeof(cmd));
    //         if (err != GF_SUCCESS) {
    //             ANC_LOGE(/ "[%s] invokeCommand err =%d ,return", __func__, err);
    //             return err;
    //         }
    //         memcpy(&init_event_info->int_ta_info, &cmd.dcs_ta_cmd_info.data.init_ta_info, sizeof(oplus_fingerprint_init_ta_info_t));//count set to 0?
    //     } while (0);
    //     memcpy(&init_event_info->int_ta_info.algo_version, &context->mFingerprintCore->report_data.algo_version,
    //         sizeof(init_event_info->int_ta_info.algo_version));

    //     getDcsEventTime(&init_event_info->init_event_time);
    //     getDcsLcdType(value);
    //     memcpy(init_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(init_event_info->lcd_type));
    //     getDcsHalVersion(&init_event_info->hal_version);
    //     getDcsDriverVersion(&init_event_info->driver_version);
    //     getDcsCdspVersion(&init_event_info->cdsp_version);
    //     init_event_info->dsp_available = context->mFingerprintCore->mDSPAvailable;//??
    //     getDcsPidInfo(&(init_event_info->pid_info));

    //     init_event_info->init_result = 0;//??
    //     init_event_info->init_fail_reason = 0;//??

    //     printInitEventInfo(init_event_info);
    //     if (context->mFingerprintCore->mNotify != nullptr) {
    //         fingerprint_msg_t message;
    //         memset(&message, 0, sizeof(fingerprint_msg_t));
    //         message.type = OPLUS_FINGRPRINT_DCS_INFO;
    //         message.data.dcs_info.dcs_type = DCS_INTI_EVENT_INFO;
    //         message.data.dcs_info.data.init_event_info = context->mDcsInfo->init_event_info;
    //         context->mFingerprintCore->mNotify(&message);
    //         dcs_static_info.need_notify_init_info = false;
    //         ANC_LOGD(/ "[sendDcsInitEventInfo]mNotify  nullptr!!!!");
    //     }

    //     memset(init_event_info, 0, sizeof(*init_event_info));//count set to 0?
    //     ANC_LOGD(/ "[sendDcsInitEventInfo] end");
    //     return err;
    // }

    uint32_t sendDcsAuthEventInfo(AncFingerprintManager *p_manager, oplus_fingerprint_auth_ta_info_t *ta_info_t)
    {
        ANC_LOGD("[sendDcsAuthEventInfo] start");

        uint32_t ret_vel = 0;
        oplus_fingerprint_dcs_info_t auth_dcs_info;
        memset(&auth_dcs_info, 0, sizeof(oplus_fingerprint_dcs_info_t));

        oplus_fingerprint_auth_event_info_t auth_event_info;
        memset(&auth_event_info, 0, sizeof(oplus_fingerprint_auth_event_info_t));
        oplus_fingerprint_dcs_static_info_t dcs_static_info;
        memset(&dcs_static_info, 0, sizeof(oplus_fingerprint_dcs_static_info_t));


        dcs_static_info.need_notify_init_info = false;
        AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
        uint8_t* p_buffer = NULL;
        uint32_t  buffer_length = 0;

        auth_event_info.auth_result = (int32_t)p_manager->p_producer->dcs_auth_result_type;//??
        ret_vel = (uint32_t)getDcsEventTime(&auth_event_info.auth_event_time);
        ret_vel = (uint32_t)getDcsHalVersion(&auth_event_info.hal_version);
        auth_event_info.retry_times = (int32_t)p_manager->p_producer->retry_count;
        ret_vel = (uint32_t)getDcsBrightnessValue(&(auth_event_info.brightness_value));
        ANC_LOGD("[getDcsBrightnessValue] auth_event_info.brightness_value=%d", auth_event_info.brightness_value);

        ///memcpy(print_auth_event_info.lcd_type, dcs_static_info.lcd_type, sizeof(print_auth_event_info.lcd_type));
        //getDcsDriverVersion(&print_auth_event_info.driver_version);
        //getDcsCdspVersion(&print_auth_event_info.cdsp_version);

        //FingerprintCore::getBrightness(&(print_auth_event_info.brightness_value));

        //print_auth_event_info.auth_type = context->mFingerprintCore->mAuthType;//??

        //print_auth_event_info.auth_ta_info.fail_reason = authContext->result;//
        //print_auth_event_info.dsp_available = context->mFingerprintCore->mDSPAvailable;//??


        if (auth_event_info.auth_result == DCS_AUTH_FAIL) {
            dcs_static_info.continuous_authsuccess_count = 0;
            dcs_static_info.continuous_authfail_count++;
        } else if (auth_event_info.auth_result == DCS_AUTH_SUCCESS) {
            dcs_static_info.continuous_authsuccess_count++;
            dcs_static_info.continuous_authfail_count = 0;
        }

        if (((auth_event_info.auth_result == DCS_AUTH_SUCCESS) && (p_manager->p_producer->retry_count < 2)) ||
        (auth_event_info.auth_result == DCS_AUTH_TOO_FAST_NO_IMGINFO)) {
            dcs_static_info.continuous_badauth_count = 0;
        } else {
            dcs_static_info.continuous_badauth_count++;
        }

        if ((auth_event_info.auth_result == DCS_AUTH_TOO_FAST_NO_IMGINFO) || (auth_event_info.auth_result == DCS_AUTH_TOO_FAST_GET_IMGINFO)) {
            //break;
        }

        auth_event_info.continuous_authsuccess_count = dcs_static_info.continuous_authsuccess_count;//??
        auth_event_info.continuous_authfail_count = dcs_static_info.continuous_authfail_count;//??
        auth_event_info.continuous_badauth_count = dcs_static_info.continuous_badauth_count;//??

        auth_event_info.user_gid = (int32_t)p_manager->p_producer->current_group_id;//??
        auth_event_info.screen_state = (int32_t)p_manager->p_producer->AuthScreenState;//??

        ret_vel = (uint32_t)getDcsPidInfo(&(auth_event_info.pid_info));

        auth_event_info.fingerprintid = (int32_t)p_manager->p_producer->finger_id;//??
        auth_event_info.captureimg_retry_count = auth_event_info.retry_times;//??
        auth_event_info.captureimg_retry_reason = 0;//??

        //kpi_info
        //ANC_LOGD("ktxdebugging authtotaltime=%d UIready_time = %d", g_time_info.authtotaltime, g_time_info.UIready_time);
        auth_event_info.auth_total_time = (int32_t)g_time_info.authtotaltime;
        auth_event_info.ui_ready_time = (int32_t)g_time_info.UIready_time;

        //tpinfo
        auth_event_info.pressxy[0] = 0;//??;//??
        auth_event_info.pressxy[1] = 0;//??;//??
        auth_event_info.area_rate = 0;//??;//??

        //lcd_info
        auth_event_info.pressxy[1] = 0;//??
        auth_event_info.area_rate = 0;//??

        memcpy(&auth_event_info.auth_ta_info, ta_info_t, sizeof(oplus_fingerprint_auth_ta_info_t));


        printAuthEventInfo(auth_event_info);

        auth_dcs_info.dcs_type = DCS_AUTH_EVENT_INFO;
        memcpy(&auth_dcs_info.data.auth_event_info, &auth_event_info, sizeof(oplus_fingerprint_auth_event_info_t));



        /*if (context->mFingerprintCore->mNotify != nullptr) {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = OPLUS_FINGRPRINT_DCS_INFO;
            message.data.dcs_info.dcs_type = DCS_AUTH_EVENT_INFO;
            message.data.dcs_info.data.auth_event_info = context->mDcsInfo->auth_event_info;
            context->mFingerprintCore->mNotify(&message);
        }*/

        p_buffer = (uint8_t*)(&auth_dcs_info);
        buffer_length = sizeof(oplus_fingerprint_dcs_info_t);
        p_device->fp_external_worker.DoWork(p_device, ANC_SEND_DCS_EVENT_INFO, p_buffer, (uint32_t)buffer_length);

        ANC_LOGD("[sendDcsAuthEventInfo] end");
        return ret_vel;
    }

    // int DcsInfo::sendDcsSingleEnrollEventInfo(HalContext* context)
    // {
    //     gf_error_t err = GF_SUCCESS;
    //     ANC_LOGD(/ "sendDcsSingleEnrollEventInfo start ");
    //     oplus_fingerprint_singleenroll_event_info_t* singleenroll_event_info = &context->mDcsInfo->singleenroll_event_info;
    //     FingerprintCore::EnrollContext *EnrollContext;

    //     //info
    //     getDcsEventTime(&singleenroll_event_info->singleenroll_event_time);
    //     singleenroll_event_info->user_gid = context->mFingerprintCore->mGid;//??

    //     //kpi_info
    //     singleenroll_event_info->singleenroll_total_time = 0;//??;//??
    //     singleenroll_event_info->ui_ready_time = context->mFingerprintCore->uiReadyTime / 1000;//??;

    //     //tpinfo
    //     singleenroll_event_info->pressxy[0] = 0;//??;//??
    //     singleenroll_event_info->pressxy[1] = 0;//??;//??
    //     singleenroll_event_info->area_rate = 0;//??;//??

    //     //lcd_info
    //     getDcsBrightnessValue(&(singleenroll_event_info->brightness_value));
    //     //FingerprintCore::getBrightness(&(singleenroll_event_info->brightness_value));
    //     memcpy(singleenroll_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(singleenroll_event_info->lcd_type));

    //     do
    //     {
    //         oplus_dcs_event_ta_cmd_t cmd;
    //         memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
    //         cmd.header.target = GF_TARGET_ALGO;
    //         cmd.header.cmd_id = GF_CMD_ALGO_BIG_DATA;
    //         cmd.dcs_ta_cmd_info.dcs_type = DCS_SINGLEENROLL_EVENT_INFO;
    //         err = invokeCommand(&cmd, sizeof(cmd));
    //         if (err != GF_SUCCESS) {
    //             ANC_LOGE(/ "[%s] invokeCommand err =%d ,return", __func__, err);
    //             return err;
    //         }
    //         memcpy(&singleenroll_event_info->singleenroll_ta_info,
    //         &cmd.dcs_ta_cmd_info.data.singleenroll_ta_info, sizeof(oplus_fingerprint_singleenroll_ta_info_t));//count set to 0?
    //     } while (0);
    //     memcpy(&singleenroll_event_info->singleenroll_ta_info.algo_version, &context->mFingerprintCore->report_data.algo_version,
    //         sizeof(singleenroll_event_info->singleenroll_ta_info.algo_version));

    //     printSingleEnrollEventInfo(singleenroll_event_info);
    //     if (context->mFingerprintCore->mNotify != nullptr) {
    //         fingerprint_msg_t message;
    //         memset(&message, 0, sizeof(fingerprint_msg_t));
    //         message.type = OPLUS_FINGRPRINT_DCS_INFO;//need type
    //         message.data.dcs_info.dcs_type = DCS_SINGLEENROLL_EVENT_INFO;
    //         message.data.dcs_info.data.singleenroll_event_info = context->mDcsInfo->singleenroll_event_info;
    //         context->mFingerprintCore->mNotify(&message);
    //     }

    //     memset(singleenroll_event_info, 0, sizeof(*singleenroll_event_info));//count set to 0
    //     ANC_LOGD(/ "sendDcsSingleEnrollEventInfo end");
    //     return 0;
    // }

    // int DcsInfo::sendDcsEnrollEventInfo(HalContext* context)
    // {
    //     gf_error_t err = GF_SUCCESS;
    //     ANC_LOGD(/ "sendDcsEnrollEventInfo start");
    //     oplus_fingerprint_enroll_event_info_t* enroll_event_info = &context->mDcsInfo->enroll_event_info;

    //     //info
    //     getDcsEventTime(&enroll_event_info->enroll_event_time);
    //     enroll_event_info->user_gid = context->mFingerprintCore->mGid;//??

    //     //enroll_info
    //     enroll_event_info->total_press_times = 0;//??;//??
    //     enroll_event_info->fingerprintid = 0;//??;//??

    //     //lcd_info
    //     memcpy(enroll_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(enroll_event_info->lcd_type));
    //     getDcsPidInfo(&(enroll_event_info->pid_info));

    //     do
    //     {
    //         oplus_dcs_event_ta_cmd_t cmd;
    //         memset(&cmd, 0, sizeof(oplus_dcs_event_ta_cmd_t));
    //         cmd.header.target = GF_TARGET_ALGO;
    //         cmd.header.cmd_id = GF_CMD_ALGO_BIG_DATA;
    //         cmd.dcs_ta_cmd_info.dcs_type = DCS_ENROLL_EVENT_INFO;
    //         err = invokeCommand(&cmd, sizeof(cmd));
    //         if (err != GF_SUCCESS) {
    //             ANC_LOGE(/ "[%s] invokeCommand err =%d ,return", __func__, err);
    //             return err;
    //         }
    //         memcpy(&enroll_event_info->enroll_ta_info, &cmd.dcs_ta_cmd_info.data.enroll_ta_info,
    //             sizeof(oplus_fingerprint_enroll_ta_info_t));//count set to 0?
    //     } while (0);
    //     memcpy(&enroll_event_info->enroll_ta_info.algo_version, &context->mFingerprintCore->report_data.algo_version,
    //         sizeof(enroll_event_info->enroll_ta_info.algo_version));

    //     printEnrollEventInfo(enroll_event_info);
    //     if (context->mFingerprintCore->mNotify != nullptr) {
    //         fingerprint_msg_t message;
    //         memset(&message, 0, sizeof(fingerprint_msg_t));
    //         message.type = OPLUS_FINGRPRINT_DCS_INFO;//need type
    //         message.data.dcs_info.dcs_type = DCS_ENROLL_EVENT_INFO;
    //         message.data.dcs_info.data.enroll_event_info = context->mDcsInfo->enroll_event_info;
    //         context->mFingerprintCore->mNotify(&message);
    //     }
    //     memset(enroll_event_info, 0, sizeof(*enroll_event_info));//count set to 0
    //     ANC_LOGD(/ "sendDcsEnrollEventInfo end");
    //     return err;
    // }

    // int DcsInfo::sendDcsSpecialEventInfo(HalContext* context)
    // {
    //     gf_error_t err = GF_SUCCESS;
    //     ANC_LOGD(/ "sendDcsSpecialEventInfo start");
    //     oplus_fingerprint_special_event_info_t* special_event_info = &context->mDcsInfo->special_event_info;

    //     //base_info
    //     getDcsEventTime(&special_event_info->event_time);
    //     special_event_info->event_type = 0;
    //     special_event_info->event_trigger_flag = 0;
    //     special_event_info->event_reason = 0;
    //     special_event_info->event_count = 0;
    //     memcpy(special_event_info->lcd_type, &context->mDcsInfo->dcs_static_info.lcd_type, sizeof(special_event_info->lcd_type));

    //     //version_info
    //     memcpy(&special_event_info->algo_version, &context->mFingerprintCore->report_data.algo_version, sizeof(special_event_info->algo_version));
    //     getDcsHalVersion(&special_event_info->hal_version);
    //     //special_event_info->algo_version = "\0";
    //     special_event_info->user_gid = 0;
    //     special_event_info->pid_info = 0;

    //     //fail_reason
    //     special_event_info->special_event_state1 = 0;
    //     special_event_info->special_event_state2 = 0;
    //     special_event_info->special_event_state3 = 0;
    //     special_event_info->special_event_state4 = 0;

    //     memset(special_event_info->special_event_string1, 0 , sizeof(*(special_event_info->special_event_string1)));//count set to 0
    //     memset(special_event_info->special_event_string2, 0 , sizeof(*(special_event_info->special_event_string2)));//count set to 0

    //     printSpecialEventInfo(special_event_info);
    //     if (context->mFingerprintCore->mNotify != nullptr) {
    //         fingerprint_msg_t message;
    //         memset(&message, 0, sizeof(fingerprint_msg_t));
    //         message.type = OPLUS_FINGRPRINT_DCS_INFO;//need type
    //         message.data.dcs_info.dcs_type = DCS_SPECIAL_EVENT_INFO;
    //         message.data.dcs_info.data.special_event_info = context->mDcsInfo->special_event_info;
    //         context->mFingerprintCore->mNotify(&message);
    //     }
    //     memset(special_event_info, 0, sizeof(*special_event_info));//count set to 0
    //     ANC_LOGD(/ "sendDcsSpecialEventInfo end");
    //     return err;
    // }



    // DcsInfo::DcsInfo(HalContext* context) : HalBase(context)
    // {
    //     ANC_LOGD(/ "--------------- [DcsInfo] DcsInfo ---------------");
    // }

    // DcsInfo::~DcsInfo()
    // {
    // }

    // int DcsInfo::init()
    // {
    //     ANC_LOGD(/ "--------------- [DcsInfo] init ---------------");
    //     dcs_static_info.need_notify_init_info = true;
    //     return 0;
    // }

    // int DcsInfo::printInitEventInfo(oplus_fingerprint_init_event_info_t* int_event_info) {
    //     oplus_fingerprint_init_ta_info_t *int_ta_info = &int_event_info->int_ta_info;
    //     ANC_LOGD(/ "--------------- [printInitEventInfo] print start ---------------");
    //     ANC_LOGD(/ "--------------- [printInitEventInfo] hal info ---------------");
    //     ANC_LOGD(/ "[printInitEventInfo] init_event_time =%d, ", int_event_info->init_event_time);
    //     ANC_LOGD(/ "[printInitEventInfo] init_result =%d, ", int_event_info->init_result);
    //     ANC_LOGD(/ "[printInitEventInfo] init_fail_reason =%d, ", int_event_info->init_fail_reason);

    //     //kpi_info
    //     ANC_LOGD(/ "[printInitEventInfo] init_time_cost_all =%d, ", int_event_info->init_time_cost_all);
    //     ANC_LOGD(/ "[printInitEventInfo] init_time_cost_cdsp =%d, ", int_event_info->init_time_cost_cdsp);
    //     ANC_LOGD(/ "[printInitEventInfo] init_time_cost_driver =%d, ", int_event_info->init_time_cost_driver);
    //     ANC_LOGD(/ "[printInitEventInfo] init_time_cost_ta =%d, ", int_event_info->init_time_cost_ta);

    //     ANC_LOGD(/ "[printInitEventInfo] lcd_type =%s, ", int_event_info->lcd_type);
    //     ANC_LOGD(/ "[printInitEventInfo] dsp_available =%d, ", int_event_info->dsp_available);
    //     ANC_LOGD(/ "[printInitEventInfo] hal_version =%d, ", int_event_info->hal_version);
    //     ANC_LOGD(/ "[printInitEventInfo] driver_version =%d, ", int_event_info->driver_version);
    //     ANC_LOGD(/ "[printInitEventInfo] cdsp_version =%d, ", int_event_info->cdsp_version);

    //     //------ta info------
    //     ANC_LOGD(/ "--------------- [printInitEventInfo] ta info ---------------");
    //     ANC_LOGD(/ "[printInitEventInfo] init_result =%d, ", int_ta_info->init_result);
    //     ANC_LOGD(/ "[printInitEventInfo] init_fail_reason =%d, ", int_ta_info->init_fail_reason);

    //     //ic_info
    //     ANC_LOGD(/ "[printInitEventInfo] sensor_id =%d, ", int_ta_info->sensor_id);
    //     ANC_LOGD(/ "[printInitEventInfo] lens_type =%d, ", int_ta_info->lens_type);
    //     ANC_LOGD(/ "[printInitEventInfo] chip_type =%s, ", int_ta_info->chip_type);
    //     ANC_LOGD(/ "[printInitEventInfo] factory_type =%s, ", int_ta_info->factory_type);

    //     //algo_info
    //     ANC_LOGD(/ "[printInitEventInfo] algo_version =%s, ", int_ta_info->algo_version);
    //     ANC_LOGD(/ "[printInitEventInfo] algo_version1 =%d, ", int_ta_info->algo_version1);
    //     ANC_LOGD(/ "[printInitEventInfo] algo_version2 =%d, ", int_ta_info->algo_version2);
    //     ANC_LOGD(/ "[printInitEventInfo] algo_version3 =%d, ", int_ta_info->algo_version3);
    //     ANC_LOGD(/ "[printInitEventInfo] algo_version4 =%d, ", int_ta_info->algo_version4);
    //     ANC_LOGD(/ "[printInitEventInfo] algo_version5 =%d, ", int_ta_info->algo_version5);

    //     //ic_status
    //     ANC_LOGD(/ "[printInitEventInfo] badpixel_num =%d, ", int_ta_info->badpixel_num);
    //     ANC_LOGD(/ "[printInitEventInfo] badpixel_num_local =%d, ", int_ta_info->badpixel_num_local);
    //     ANC_LOGD(/ "[printInitEventInfo] init_finger_number =%d, ", int_ta_info->init_finger_number);
    //     ANC_LOGD(/ "[printInitEventInfo] template_verison =%d, ", int_ta_info->template_verison);
    //     ANC_LOGD(/ "[printInitEventInfo] template_num= %d,  %d,   %d,  %d,   %d", int_ta_info->template_num[0],
    //         int_ta_info->template_num[1], int_ta_info->template_num[2], int_ta_info->template_num[3], int_ta_info->template_num[4]);
    //     ANC_LOGD(/ "[printInitEventInfo] all_template_num =%d, ", int_ta_info->all_template_num);

    //     //calabration_info
    //     ANC_LOGD(/ "[printInitEventInfo] exposure_value =%d, ", int_ta_info->exposure_value);
    //     ANC_LOGD(/ "[printInitEventInfo] exposure_time =%d, ", int_ta_info->exposure_time);
    //     ANC_LOGD(/ "[printInitEventInfo] calabration_signal_value =%d, ", int_ta_info->calabration_signal_value);
    //     ANC_LOGD(/ "[printInitEventInfo] factory_type =%d, ", int_ta_info->calabration_tsnr);
    //     ANC_LOGD(/ "[printInitEventInfo] flesh_touch_diff =%d, ", int_ta_info->flesh_touch_diff);
    //     ANC_LOGD(/ "[printInitEventInfo] scale =%d, ", int_ta_info->scale);
    //     ANC_LOGD(/ "[printInitEventInfo] gain =%d, ", int_ta_info->gain);

    //     //reserve_info
    //     ANC_LOGD(/ "[printInitEventInfo] init_event_state1 =%d, ", int_ta_info->init_event_state1);
    //     ANC_LOGD(/ "[printInitEventInfo] init_event_state2 =%d, ", int_ta_info->init_event_state2);
    //     ANC_LOGD(/ "[printInitEventInfo] init_event_sting1 =%s, ", int_ta_info->init_event_sting1);
    //     ANC_LOGD(/ "[printInitEventInfo] init_event_sting2 =%s, ", int_ta_info->init_event_sting2);

    //     //print_end
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] ------ta info------ ");
    //     return 0;
    // }



    // int DcsInfo::printSingleEnrollEventInfo(oplus_fingerprint_singleenroll_event_info_t* singleenroll_event_info) {
    //     oplus_fingerprint_singleenroll_ta_info_t* singleenroll_ta_info = &singleenroll_event_info->singleenroll_ta_info;
    //     ANC_LOGD(/ "--------------- [printSingleEnrollEventInfo] print start ---------------");
    //     ANC_LOGD(/ "--------------- [printSingleEnrollEventInfo] hal info ---------------");

    //     //base_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_event_time =%d, ", singleenroll_event_info->singleenroll_event_time);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_result =%d, ", singleenroll_event_info->singleenroll_result);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] user_gid =%d, ", singleenroll_event_info->user_gid);

    //     //kpi_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_total_time =%d, ", singleenroll_event_info->singleenroll_total_time);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] ui_ready_time =%d, ", singleenroll_event_info->ui_ready_time);

    //     //tp_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] pressxy0 =%d, ", singleenroll_event_info->pressxy[0]);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] pressxy1 =%d, ", singleenroll_event_info->pressxy[1]);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] area_rate =%d, ", singleenroll_event_info->area_rate);

    //     //lcd_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] brightness_value =%d, ", singleenroll_event_info->brightness_value);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] lcd_type =%s, ", singleenroll_event_info->lcd_type);

    //     //------ta info------
    //     ANC_LOGD(/ "--------------- [printSingleEnrollEventInfo] ta info ---------------");
    //     //base_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_result =%d, ", singleenroll_ta_info->singleenroll_result);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] fail_reason =%d, ", singleenroll_ta_info->fail_reason);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] fail_reason_param1 =%d, ", singleenroll_ta_info->fail_reason_param1);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] fail_reason_param2 =%d, ", singleenroll_ta_info->fail_reason_param2);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] algo_version =%s, ", singleenroll_ta_info->algo_version);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] current_enroll_times =%d, ", singleenroll_ta_info->current_enroll_times);

    //     //img_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] quality_score =%d, ", singleenroll_ta_info->quality_score);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] signal_value =%d, ", singleenroll_ta_info->signal_value);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] img_area =%d, ", singleenroll_ta_info->img_area);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] img_direction =%d, ", singleenroll_ta_info->img_direction);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] ta_retry_times =%d, ", singleenroll_ta_info->ta_retry_times);

    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] exposure_flag =%d, ", singleenroll_ta_info->exposure_flag);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] fdt_base_flag =%d, ", singleenroll_ta_info->fdt_base_flag);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] image_base_flag =%d, ", singleenroll_ta_info->image_base_flag);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] repetition_rate =%d, ", singleenroll_ta_info->repetition_rate);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] enroll_rawdata =%d, ", singleenroll_ta_info->enroll_rawdata);

    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] anomaly_flag =%d, ", singleenroll_ta_info->anomaly_flag);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] screen_protector_type =%d, ", singleenroll_ta_info->screen_protector_type);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] key_point_num =%d, ", singleenroll_ta_info->key_point_num);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] increase_rate =%d, ", singleenroll_ta_info->increase_rate);

    //     //kpi_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] capture_time =%d, ", singleenroll_ta_info->capture_time);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] preprocess_time =%d, ", singleenroll_ta_info->preprocess_time);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] get_feature_time =%d, ", singleenroll_ta_info->get_feature_time);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] enroll_time =%d, ", singleenroll_ta_info->enroll_time);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] detect_fake_time =%d, ", singleenroll_ta_info->detect_fake_time);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] kpi_time_all =%d, ", singleenroll_ta_info->kpi_time_all);

    //     //lcd_info
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_event_state1 =%d, ", singleenroll_ta_info->singleenroll_event_state1);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_event_state2 =%d, ", singleenroll_ta_info->singleenroll_event_state2);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_event_string1 =%s, ", singleenroll_ta_info->singleenroll_event_string1);
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] singleenroll_event_string2 =%s, ", singleenroll_ta_info->singleenroll_event_string2);

    //     //------ta info------
    //     ANC_LOGD(/ "[printSingleEnrollEventInfo] -----print end------ ");
    //     return 0;
    // }

    // int DcsInfo::printEnrollEventInfo(oplus_fingerprint_enroll_event_info_t* enroll_event_info) {
    //     oplus_fingerprint_enroll_ta_info_t* enroll_ta_info = &enroll_event_info->enroll_ta_info;
    //     ANC_LOGD(/ "--------------- [printEnrollEventInfo] print start ---------------");
    //     ANC_LOGD(/ "--------------- [printEnrollEventInfo] hal info ---------------");

    //     //base_info
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_event_time =%d, ", enroll_event_info->enroll_event_time);
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_result =%d, ", enroll_event_info->enroll_result);
    //     ANC_LOGD(/ "[printEnrollEventInfo] user_gid =%d, ", enroll_event_info->user_gid);
    //     ANC_LOGD(/ "[printEnrollEventInfo] total_press_times =%d, ", enroll_event_info->total_press_times);
    //     ANC_LOGD(/ "[printEnrollEventInfo] fingerprintid =%d, ", enroll_event_info->fingerprintid);
    //     ANC_LOGD(/ "[printEnrollEventInfo] pid_info =%d, ", enroll_event_info->pid_info);
    //     ANC_LOGD(/ "[printEnrollEventInfo] lcd_type =%s, ", enroll_event_info->lcd_type);

    //     //------ta info------
    //     ANC_LOGD(/ "--------------- [printEnrollEventInfo] ta info ---------------");
    //     //base_info
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_result =%d, ", enroll_ta_info->enroll_result);
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_reason =%d, ", enroll_ta_info->enroll_reason);
    //     ANC_LOGD(/ "[printEnrollEventInfo] cdsp_flag =%d, ", enroll_ta_info->cdsp_flag);
    //     ANC_LOGD(/ "[printEnrollEventInfo] repetition_enroll_number =%d, ", enroll_ta_info->repetition_enroll_number);
    //     ANC_LOGD(/ "[printEnrollEventInfo] total_enroll_times =%d, ", enroll_ta_info->total_enroll_times);
    //     ANC_LOGD(/ "[printEnrollEventInfo] finger_number =%d, ", enroll_ta_info->finger_number);
    //     ANC_LOGD(/ "[printEnrollEventInfo] lcd_type =%d, ", enroll_ta_info->lcd_type);

    //     //version_info
    //     ANC_LOGD(/ "[printEnrollEventInfo] algo_version =%s, ", enroll_ta_info->algo_version);
    //     ANC_LOGD(/ "[printEnrollEventInfo] template_version =%d, ", enroll_ta_info->template_version);

    //     //bak_info
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_event_state1 =%d, ", enroll_ta_info->enroll_event_state1);
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_event_state2 =%d, ", enroll_ta_info->enroll_event_state2);
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_event_string1 =%s, ", enroll_ta_info->enroll_event_string1);
    //     ANC_LOGD(/ "[printEnrollEventInfo] enroll_event_string2 =%s, ", enroll_ta_info->enroll_event_string2);

    //     //------ta info------
    //     ANC_LOGD(/ "[printEnrollEventInfo] ------print end------ ");
    //     return 0;
    // }

    // int DcsInfo::printSpecialEventInfo(oplus_fingerprint_special_event_info_t* special_event_info) {
    //     ANC_LOGD(/ "--------------- [printSpecialEventInfo] print start ---------------");
    //     ANC_LOGD(/ "--------------- [printSpecialEventInfo] hal info ---------------");

    //     //base_info
    //     ANC_LOGD(/ "[printSpecialEventInfo] event_time =%d, ", special_event_info->event_time);
    //     ANC_LOGD(/ "[printSpecialEventInfo] event_type =%d, ", special_event_info->event_type);
    //     ANC_LOGD(/ "[printSpecialEventInfo] event_trigger_flag =%d, ", special_event_info->event_trigger_flag);
    //     ANC_LOGD(/ "[printSpecialEventInfo] event_reason =%d, ", special_event_info->event_reason);
    //     ANC_LOGD(/ "[printSpecialEventInfo] event_count =%d, ", special_event_info->event_count);
    //     ANC_LOGD(/ "[printSpecialEventInfo] lcd_type =%s, ", special_event_info->lcd_type);

    //     ANC_LOGD(/ "[printSpecialEventInfo] hal_version =%d, ", special_event_info->hal_version);
    //     ANC_LOGD(/ "[printSpecialEventInfo] algo_version =%s, ", special_event_info->algo_version);
    //     ANC_LOGD(/ "[printSpecialEventInfo] user_gid =%d, ", special_event_info->user_gid);
    //     ANC_LOGD(/ "[printSpecialEventInfo] pid_info =%d, ", special_event_info->pid_info);

    //     //fail_reason
    //     ANC_LOGD(/ "[printSpecialEventInfo] special_event_state1 =%d, ", special_event_info->special_event_state1);
    //     ANC_LOGD(/ "[printSpecialEventInfo] special_event_state2 =%d, ", special_event_info->special_event_state2);
    //     ANC_LOGD(/ "[printSpecialEventInfo] special_event_state3 =%d, ", special_event_info->special_event_state3);
    //     ANC_LOGD(/ "[printSpecialEventInfo] special_event_state4 =%d, ", special_event_info->special_event_state4);
    //     ANC_LOGD(/ "[printSpecialEventInfo] special_event_string1 =%s, ", special_event_info->special_event_string1);
    //     ANC_LOGD(/ "[printSpecialEventInfo] special_event_string2 =%s, ", special_event_info->special_event_string2);

    //     //------ta info------
    //     ANC_LOGD(/ "[printEnrollEventInfo] ------print end------ ");
    //     return 0;
    // }
