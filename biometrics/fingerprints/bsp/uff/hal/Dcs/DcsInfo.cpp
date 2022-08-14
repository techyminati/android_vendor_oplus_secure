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
 ** ---------------------------Revision History--------------------------------
 **  <author>          <data>           <desc>
 **  Ran. Chen      2020/09/29       create file
 **  Ran. Chen      2020/10/10       modify for coverity
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][DcsInfo]"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/properties.h>
#include <ctime>
#include "DcsInfo.h"
#include "DcsReport.h"
#include "HalContext.h"
#include "HalLog.h"
#ifdef FP_DCS_ENABLE
#endif /* FP_DCS_ENABLE */

namespace android {
DcsInfo::DcsInfo(HalContext* context) {
    LOG_D(LOG_TAG, "--------------- [DcsInfo] DcsInfo ---------------");
    UNUSED(context);
}

DcsInfo::~DcsInfo() {}

int DcsInfo::init() {
    LOG_D(LOG_TAG, "--------------- [DcsInfo] init ---------------");
    dcs_static_info.need_notify_init_info = true;

    return 0;
}
int DcsInfo::getDcsEventTime(int32_t* event_times) {
    int err = 0;
    FUNC_ENTER();

    struct timeval tv;
    memset(&tv, 0, sizeof(timeval));
    struct tm current_tm;
    memset(&current_tm, 0, sizeof(tm));
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &current_tm);
    *event_times = current_tm.tm_hour * 100 + current_tm.tm_min;
    LOG_D(LOG_TAG,
        "[getDcsEventTime], current_tm.tm_year  =%d ,current_tm.tm_mon "
        "=%d,%d,%d,%d,%d",
        current_tm.tm_year, current_tm.tm_mon, current_tm.tm_mday, current_tm.tm_hour,
        current_tm.tm_min, current_tm.tm_sec);
    LOG_D(LOG_TAG, "[getDcsEventTime], event_times  =%d ", *event_times);
    FUNC_EXIT(FP_SUCCESS);
    return err;
}

int DcsInfo::getDcsBrightnessValue(uint32_t* brightness_value) {
    fp_return_type_t err = FP_SUCCESS;
    char buf[50] = {'\0'};
    int32_t length = 0;
    uint32_t bright_value = 0;
    int fd = 0;
    FUNC_ENTER();
    std::string brightness_paths[] = {
        "/sys/class/leds/lcd-backlight/brightness",
        "/sys/class/backlight/panel0-backlight/brightness",
    };
    int index = 0;
    int N = sizeof(brightness_paths) / sizeof(brightness_paths[0]);
    for (index = 0; index < N; index++) {
        if (access(brightness_paths[index].c_str(), 0) == 0) {
            LOG_E(LOG_TAG, "Brightness path index %d, path:%s", index, brightness_paths[index].c_str());
            break;
        }
    }
    if (index == N) {
        LOG_E(LOG_TAG, "no brightness path available");
        err = FP_ERROR;
        CHECK_RESULT_SUCCESS(err);
    }
    fd = open(brightness_paths[index].c_str(), O_RDONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "getBrightness openerr :%d, errno =%d", fd, errno);
        err = FP_ERROR;
        CHECK_RESULT_SUCCESS(err);
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

fp_out:
    FUNC_EXIT(err);
    return err;
}

int DcsInfo::getDcsPidInfo(int32_t* pid_info) {
    int err = 0;
    FUNC_ENTER();
    *pid_info = (int32_t)getpid();
    LOG_D(LOG_TAG, "[getDcsPidInfo] pid_info =%d", *pid_info);
    FUNC_EXIT(FP_SUCCESS);
    return err;
}

int DcsInfo::getDcsAlogVerison(char* algo_version) {
    int err = 0;
    FUNC_ENTER();
    const char *name = "oplus.fingerprint.gf.package.version";
    property_get(name, algo_version, "0");
    memcpy(dcs_static_info.algo_version, algo_version, sizeof(dcs_static_info.algo_version));
    FUNC_EXIT(FP_SUCCESS);
    return err;
}

int DcsInfo::sendDcsInitEventInfo(HalContext* context) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    dcs_init_event_info_t* init_event_info = &context->mDcsInfo->init_event_info;

    getDcsEventTime(&init_event_info->init_event_time);
    memcpy(init_event_info->lcd_type, &context->mConfig->mTaConfig->mLcdType, sizeof(init_event_info->lcd_type));
    init_event_info->dsp_available = 0;
    getDcsPidInfo(&(init_event_info->pid_info));

    init_event_info->init_result = 0;
    init_event_info->init_fail_reason = 0;

    fp_get_dcsinfo_t cmd;
    memset(&cmd, 0, sizeof(fp_get_dcsinfo_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_GET_DCSINFO;
    cmd.config.dcs_event_type = E_DCS_INTI_EVENT_INFO;
    cmd.config.dcsinfo_length = sizeof(dcs_init_ta_info_t);

    err = context->mCaEntry->sendCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);
    memcpy(&init_event_info->init_ta_info, cmd.result.buffer, sizeof(init_event_info->init_ta_info));
    getDcsAlogVerison(init_event_info->init_ta_info.algo_version);

    DcsReport::getInstance()->reportInitEventInfo(init_event_info);
    dcs_static_info.need_notify_init_info = false;

fp_out:
    memset(init_event_info, 0, sizeof(*init_event_info));

    FUNC_EXIT(err);
    return err;
}

int DcsInfo::sendDcsAuthEventInfo(HalContext* context) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    dcs_auth_event_info_t* auth_event_info = &context->mDcsInfo->auth_event_info;
    fp_auth_compare_result_t* compare_result = &context->mFingerprintManager->compare_result;
    FUNC_ENTER();

    getDcsEventTime(&auth_event_info->auth_event_time);
    //auth_type
    auth_event_info->auth_type = compare_result->result_info.result_type;

    //auth_result
    auth_event_info->auth_result = compare_result->result_info.result_type;
    if ((compare_result->result_info.result_type != DCS_AUTH_SUCCESS)
        || (compare_result->result_info.result_type != DCS_AUTH_TOO_FAST_GET_IMGINFO)) {
        auth_event_info->auth_result = FP_SDK_ERR_COMPARE_FAIL;
    }

    auth_event_info->dsp_available = 0;
    //retry_times
    if (auth_event_info->auth_result == DCS_AUTH_FAIL) {
        dcs_static_info.continuous_authsuccess_count = 0;
        dcs_static_info.continuous_authfail_count++;
    } else if (auth_event_info->auth_result == DCS_AUTH_SUCCESS) {
        dcs_static_info.continuous_authsuccess_count++;
        dcs_static_info.continuous_authfail_count = 0;
    }

    auth_event_info->continuous_authsuccess_count = dcs_static_info.continuous_authsuccess_count;
    auth_event_info->continuous_authfail_count = dcs_static_info.continuous_authfail_count;
    auth_event_info->continuous_badauth_count = dcs_static_info.continuous_badauth_count;

    auth_event_info->user_gid = context->mFingerprintManager->current_gid;
    auth_event_info->screen_state = context->mFingerprintManager->mScreenStatus;
    auth_event_info->fingerprintid = compare_result->finger_id;
    getDcsPidInfo(&(auth_event_info->pid_info));
    auth_event_info->captureimg_retry_count = 0;
    auth_event_info->captureimg_retry_reason = 0;

    //kpi_info
    auth_event_info->auth_total_time = (context->mFingerprintManager->mFunctionFinishTime - context->mFingerprintManager->mFingerDownTime) / 1000;
    auth_event_info->ui_ready_time = (context->mFingerprintManager->mUiReadyTime - context->mFingerprintManager->mFingerDownTime) / 1000;

    //tp_info

    //lcd_info
    getDcsBrightnessValue(&(auth_event_info->brightness_value));
    memcpy(auth_event_info->lcd_type, &context->mConfig->mTaConfig->mLcdType, sizeof(auth_event_info->lcd_type));

    //version_info
    if (auth_event_info->auth_result != DCS_AUTH_TOO_FAST_NO_IMGINFO) {
        fp_get_dcsinfo_t cmd;
        memset(&cmd, 0, sizeof(fp_get_dcsinfo_t));
        cmd.header.module_id = FP_MODULE_FPCORE;
        cmd.header.cmd_id = FP_CMD_FPCORE_GET_DCSINFO;
        cmd.config.dcs_event_type = E_DCS_AUTH_EVENT_INFO;
        cmd.config.dcsinfo_length = sizeof(dcs_auth_ta_info_t);
        err = context->mCaEntry->sendCommand(&cmd, sizeof(cmd));
        CHECK_RESULT_SUCCESS(err);
        memcpy(&auth_event_info->auth_ta_info, cmd.result.buffer, sizeof(auth_event_info->auth_ta_info));
        memcpy(auth_event_info->auth_ta_info.algo_version, dcs_static_info.algo_version, sizeof(dcs_static_info.algo_version));
    }
    DcsReport::getInstance()->reportAuthEventInfo(auth_event_info);

fp_out:
    memset(auth_event_info, 0, sizeof(*auth_event_info));

    FUNC_EXIT(err);
    return err;
}

int DcsInfo::sendDcsSingleEnrollEventInfo(HalContext* context) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    dcs_singleenroll_event_info_t* singleenroll_event_info = &context->mDcsInfo->singleenroll_event_info;
    fp_enroll_image_result_t* enroll_result =  &context->mFingerprintManager->enroll_result;
    //info
    getDcsEventTime(&singleenroll_event_info->singleenroll_event_time);
    singleenroll_event_info->singleenroll_result = enroll_result->result_info.result_type;
    singleenroll_event_info->user_gid = context->mFingerprintManager->current_gid;

    //kpi_info
    singleenroll_event_info->singleenroll_total_time =
        (context->mFingerprintManager->mFunctionFinishTime - context->mFingerprintManager->mFingerDownTime) / 1000;
    singleenroll_event_info->ui_ready_time = (context->mFingerprintManager->mUiReadyTime - context->mFingerprintManager->mFingerDownTime) / 1000;

    //tpinfo
    singleenroll_event_info->pressxy[0] = 0;
    singleenroll_event_info->pressxy[1] = 0;
    singleenroll_event_info->area_rate = 0;

    //lcd_info
    memcpy(singleenroll_event_info->lcd_type, &context->mConfig->mTaConfig->mLcdType, sizeof(singleenroll_event_info->lcd_type));
    getDcsBrightnessValue(&singleenroll_event_info->brightness_value);

    fp_get_dcsinfo_t cmd;
    memset(&cmd, 0, sizeof(fp_get_dcsinfo_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_GET_DCSINFO;
    cmd.config.dcs_event_type = E_DCS_SINGLEENROLL_EVENT_INFO;
    cmd.config.dcsinfo_length = sizeof(dcs_singleenroll_ta_info_t);
    err = context->mCaEntry->sendCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);
    memcpy(&singleenroll_event_info->singleenroll_ta_info, cmd.result.buffer, sizeof(singleenroll_event_info->singleenroll_ta_info));
    memcpy(singleenroll_event_info->singleenroll_ta_info.algo_version, dcs_static_info.algo_version, sizeof(dcs_static_info.algo_version));

    DcsReport::getInstance()->reportSingleEnrollEventInfo(singleenroll_event_info);

fp_out:
    FUNC_EXIT(err);
    return err;
}

int DcsInfo::sendDcsEnrollEventInfo(HalContext* context) {
    fp_return_type_t err = FP_SUCCESS;
    UNUSED(context);
    FUNC_ENTER();
    dcs_enroll_event_info_t* enroll_event_info = &context->mDcsInfo->enroll_event_info;
    fp_enroll_image_result_t* enroll_result =  &context->mFingerprintManager->enroll_result;

    getDcsEventTime(&enroll_event_info->enroll_event_time);
    enroll_event_info->enroll_result = enroll_result->result_info.result_type;
    enroll_event_info->user_gid = context->mFingerprintManager->current_gid;
    //total_press_times
    enroll_event_info->fingerprintid = enroll_result->finger_id;
    getDcsPidInfo(&(enroll_event_info->pid_info));
    memcpy(enroll_event_info->lcd_type, &context->mConfig->mTaConfig->mLcdType, sizeof(enroll_event_info->lcd_type));

    fp_get_dcsinfo_t cmd;
    memset(&cmd, 0, sizeof(fp_get_dcsinfo_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_GET_DCSINFO;
    cmd.config.dcs_event_type = E_DCS_ENROLL_EVENT_INFO;
    cmd.config.dcsinfo_length = sizeof(enroll_event_info);
    err = context->mCaEntry->sendCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);
    memcpy(&enroll_event_info->enroll_ta_info, cmd.result.buffer, sizeof(enroll_event_info->enroll_ta_info));
    memcpy(enroll_event_info->enroll_ta_info.algo_version, dcs_static_info.algo_version, sizeof(dcs_static_info.algo_version));

    DcsReport::getInstance()->reportEnrollEventInfo(enroll_event_info);

fp_out:
    FUNC_EXIT(err);
    return err;
}

int DcsInfo::sendDcsSpecialEventInfo(HalContext* context) {
    fp_return_type_t err = FP_SUCCESS;
    UNUSED(context);
    FUNC_ENTER();
    dcs_special_event_info_t* special_event_info = &context->mDcsInfo->special_event_info;
    FUNC_ENTER();

    //base_info
    getDcsEventTime(&special_event_info->event_time);
    special_event_info->event_type = 0;
    special_event_info->event_trigger_flag = 0;
    special_event_info->event_reason = 0;
    special_event_info->event_count = 0;
    memcpy(special_event_info->lcd_type, &context->mConfig->mTaConfig->mLcdType, sizeof(special_event_info->lcd_type));

    //version_info
    special_event_info->user_gid = 0;
    special_event_info->pid_info = 0;

    //fail_reason
    special_event_info->special_event_state1 = 0;
    special_event_info->special_event_state2 = 0;
    special_event_info->special_event_state3 = 0;
    special_event_info->special_event_state4 = 0;
    memset(special_event_info->special_event_string1, 0 , sizeof(*(special_event_info->special_event_string1)));
    memset(special_event_info->special_event_string2, 0 , sizeof(*(special_event_info->special_event_string2)));

    DcsReport::getInstance()->reportSpecialEventInfo(special_event_info);

    FUNC_EXIT(err);
    return err;
}
}  // namespace android
