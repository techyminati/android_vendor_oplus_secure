/************************************************************************************
 ** File: - vendor/oplus/secure/biometrics/fingerprints/bsp/vendor/fingerprint/hwbinder/egis_hidl.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for EGISTEC (sw23.2 android P)
 **
 ** Version: 1.0
 ** Date created: 15:03:11,18/10/2018
 ** Author: liuqingwen@RM.BSP.Fingerprint.Basic
 ** TAG: RM.BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  liuqingwen   2018/10/18    create the file
 ************************************************************************************/

#include <stdint.h>
#include <vector>
#include <cutils/log.h>
#include <fingerprint.h>
#include "egis_hidl.h"
#include "egis_fingerprint.h"
#include <string.h>
#include "fingerprint_type.h"
//sp<IBiometricsFingerprintClientCallback> mCallback;

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#if 0
void trans_callback(int event_id, int value1, int value2, uint8_t* buffer, int buffer_size)
{
    ALOGD("buffer_size = %d",  buffer_size);
    const android::hardware::hidl_vec<uint8_t> hv1 = std::vector<uint8_t>(buffer, buffer + buffer_size);
    mCallback->egis_ipc_callback(event_id, value1, value2, hv1, buffer_size);
}
#endif

egis_hidl::egis_hidl() : mDevice(nullptr)
{
    ALOGD("egis_hidl enter!");
    ets_callback.on_acquired = on_acquired;
    ets_callback.on_authenticated = on_authenticated;
    ets_callback.on_enroll_result = on_enroll_result;
    ets_callback.on_enumerate = on_enumerate;
    ets_callback.on_error = on_error;
    ets_callback.on_removed = on_removed;
    //ets_callback.trans_callback = trans_callback;
    ets_callback.on_sync_templates = on_sync_templates;
    ets_callback.on_touch_up = on_touch_up;
    ets_callback.on_touch_down = on_touch_down;
    ets_callback.on_monitor_event_triggered = on_monitor_event_triggered;
    ets_callback.on_image_info_acquired = on_image_info_acquired;
    ets_callback.on_engineering_info_update = on_engineering_info_update;
}

egis_hidl::~egis_hidl()
{
    ALOGD("~egis_hidl enter!");
    close();
}

int egis_hidl::close()
{
    ALOGD("egis_hidl close enter!");
    if (mDevice != nullptr) {
        fingerprint_close(mDevice);
        mDevice = nullptr;
        ALOGE("egis_hidl close");
    }
    ALOGD("egis_hidl close leave");
    return 0;
}

int egis_hidl::open()
{
    ALOGD("egis_hidl open enter!");

    int ret = fingerprint_open(&mDevice);
    if (!ret) {
        fingerprint_set_callback(&mDevice, &ets_callback, this);
    }

    ALOGD("egis_hidl open leave ret:%d!", ret);
    return ret;
}
/*
int egis_hidl::start_service()
{
    ALOGD("egis_hidl start_inline_service enter!");

    int ret = start_inline_service();

    ALOGD("egis_hidl start_inline_service ret:%d!", ret);
    return ret;
}
*/

Return<uint64_t> egis_hidl::setNotify(const sp<IBiometricsFingerprintClientCallback>& clientCallback)
{
    ALOGD("egis_hidl setNotify enter!");
    mClientCallback = clientCallback;
        if (!mDevice) {
                ALOGD("egis_hidl setNotify enter mDevice is null!");
                return NULL;
        }
        ALOGD("egis_hidl setNotify enter mDevice is no null!");
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> egis_hidl::preEnroll()
{
    ALOGD("egis_hidl preEnroll enter!");
    return fingerprint_pre_enroll(mDevice);
}
Return<uint64_t> egis_hidl::setHalCallback(const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) {
    ALOGD("egis_hidl setHalCallback enter!");
    mClientCallbackEx = clientCallbackEx;
    if (!mDevice) {
        ALOGD("egis_hidl setHalCallback enter mDevice is null!");
        return NULL;
        }
    ALOGD("egis_hidl setHalCallback enter mDevice is no null!");
    return reinterpret_cast<uint64_t>(this);
}

Return<RequestStatus> egis_hidl::enroll(const hidl_array<uint8_t, 69>& hat,
                                       uint32_t gid, uint32_t timeout_sec)
{
    ALOGD("egis_hidl enroll enter!");
    int ret = fingerprint_enroll(mDevice, static_cast<const uint8_t*>(hat.data()),
                gid, timeout_sec);
    ALOGD("egis_hidl enroll fingerprint_enroll ret:%d", ret);

    if (ret != 0) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::postEnroll()
{
    ALOGD("egis_hidl postEnroll enter!");
    if (fingerprint_post_enroll(mDevice)) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::cancel()
{
    ALOGD("egis_hidl cancel enter!");
    fingerprint_cancel();
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::enumerate()
{
    ALOGD("egis_hidl enumrate enter!");
    fingerprint_enumerate();
    return RequestStatus::SYS_OK;
}

Return<uint64_t> egis_hidl::getAuthenticatorId()
{
    ALOGD("egis_hidl getAuthenticatorId enter!");
    return fingerprint_get_auth_id(mDevice);
}

Return<RequestStatus> egis_hidl::remove(uint32_t gid, uint32_t fid)
{
    ALOGD("egis_hidl remove enter!");
    if (fingerprint_remove(gid, fid)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::setActiveGroup(uint32_t gid, const hidl_string& storePath)
{
    ALOGD("egis_hidl setActiveGroup enter!");
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        ALOGE("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        return RequestStatus::SYS_EINVAL;
    }
    if (fingerprint_set_active_group(gid, storePath.c_str())) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::authenticate(uint64_t operation_id, uint32_t gid)
{
    if (fingerprint_authenticate(mDevice, operation_id, gid)) {
    return RequestStatus::SYS_EINVAL;
}
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::pauseEnroll()
{
    ALOGD("egis_hidl pauseEnroll enter!");
    fingerprint_pause_enroll();
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::continueEnroll()
{
    ALOGD("egis_hidl continueEnroll enter!");
    fingerprint_continue_enroll();
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::setTouchEventListener()
{
    ALOGD("egis_hidl setTouchEventListener enter!");
    fingerprint_set_touch_event_listener();
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::dynamicallyConfigLog(uint32_t on)
{
    ALOGD("egis_hidl dynamicallyConfigLog enter!");
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::pauseIdentify()
{
    ALOGD("egis_hidl pauseIdentify enter!");
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::continueIdentify()
{
    ALOGD("egis_hidl continueIdentify enter!");
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> egis_hidl::getAlikeyStatus()
{
    ALOGD("egis_hidl getAlikeyStatus enter!");
    return RequestStatus::SYS_OK;
}

Return<int32_t > egis_hidl::getEnrollmentTotalTimes()
{
    ALOGD("egis_hidl getEnrollmentTotalTimes enter!");
    return fingerprint_get_enrollment_total_times();
}

Return<RequestStatus> egis_hidl::setScreenState(int32_t screen_state)
{
    ALOGD("egis_hidl setScreenState enter!");
    int32_t state = static_cast<int32_t>(screen_state);
    fingerprint_set_screen_state(state);
    return RequestStatus::SYS_OK;
}

Return<int32_t> egis_hidl::getEngineeringInfo(uint32_t type)
{
    ALOGD("egis_hidl getEngineeringInfo enter!");
    return fingerprint_get_engineering_info(type);
}
Return<bool> egis_hidl::isUdfps(uint32_t sensorId) {
    ALOGD("isUdfps sensor_id = %d", sensorId);
    if (sensorId >= E_SENSOR_ID_MAX) {
        ALOGE("unknown sensor");
        return false;
    }
    return (fp_config_info[sensorId].fp_type) == 3 ? true:false;
}
Return<void> egis_hidl::onFingerDown(uint32_t x, uint32_t y, float minor, float major)
{
    ALOGD("egis_hidl touchDown enter!");
    return Void();
}

Return<void> egis_hidl::onFingerUp()
{
    ALOGD("egis_hidl touchUp enter!");
    return Void();
}

Return<int32_t> egis_hidl::sendFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& in_buf)
{
    int32_t ret = -1;
    ALOGD("egis_hidl sendFingerprintCmd enter, cmdId = %d, in_buf[0] = %d", cmdId, static_cast<int32_t>(in_buf[0]));
    switch (cmdId) {
        case  FINGERPRINT_CMD_ID_GET_ENROLL_TIMES:
        {
            int32_t enroll_times = 0;
            enroll_times = getEnrollmentTotalTimes();
            return enroll_times;
        }
        case  FINGERPRINT_CMD_ID_SET_SCREEN_STATE:
        {
            uint32_t screen_state;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&screen_state, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_SET_SCREEN_STATE screen_state = %d", screen_state);
            setScreenState(screen_state);
            ALOGE("qweqweqwe");
            return 0;
        }
        case  FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO:
        {
            uint32_t type = 0;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&type, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO type = %d", type);
            return getEngineeringInfo(type);
        }
        case  FINGERPRINT_CMD_ID_GET_SENSOR_ID:
        {
            ALOGE("FINGERPRINT_CMD_ID_GET_SENSOR_ID = %d", fp_config_info_init.sensor_id);
            return fp_config_info_init.sensor_id;
        }
        case FINGERPRINT_CMD_ID_CAMERA: //CMD_FINGERPRINT_CAMERA
        {
            ALOGD("keymode_enable(enable = %d)", in_buf[0]);
            ret = fingerprint_keymode_enable(mDevice, static_cast<int32_t>(in_buf[0]));
        }
        break;
        default:
            ALOGE("unkonwn cmd.");
            break;
    }
    return ret;

}

Return<RequestStatus> egis_hidl::authenticateAsType(uint64_t operationId, uint32_t gid, int32_t authtype)
{
    UNUSED(authtype);
    if (fingerprint_authenticate(mDevice, operationId, gid)) {
    return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

void egis_hidl::on_enroll_result(void* context, uint32_t fid, uint32_t gid, uint32_t remaining)
{
    ALOGD("egis_hidl on_enroll_result enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);

    ALOGD("onEnrollResult(fid=%d, gid=%d, remaing=%d)", fid, gid, remaining);
    if (!me->mClientCallback->onEnrollResult(reinterpret_cast<uint64_t>(context), fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnrollResult callback");
    }
}

void egis_hidl::on_acquired(void* context, int info)
{
    ALOGD("egis_hidl on_acquired enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    int32_t vendor_code = 0;
    FingerprintAcquiredInfo acquired_info = static_cast<FingerprintAcquiredInfo>(info);
    int accquired_vendor = static_cast<int>(FingerprintAcquiredInfo::ACQUIRED_VENDOR);
    ALOGD("onAcquired(info=%d, vendor=%d)", acquired_info, vendor_code);
    if (!me->mClientCallback->onAcquired(reinterpret_cast<uint64_t>(context), acquired_info, vendor_code).isOk()) {
        ALOGE("failed to invoke fingerprint onAcquired callback");
    }
}

void egis_hidl::on_authenticated(void* context, uint32_t fid, uint32_t gid,
                                const uint8_t* token, uint32_t size_token)
{
    ALOGD("egis_hidl on_authenticated enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    const std::vector<uint8_t> hidl_token(token, token + size_token);
    ALOGI("onAuthenticated(fid=%d, gid=%d)", fid, gid);
    if (!me->mClientCallback->onAuthenticated(reinterpret_cast<uint64_t>(context), fid, gid,
                                  hidl_vec<uint8_t>(hidl_token)).isOk()) {
        ALOGE("failed to invoke fingerprint onAuthenticated callback(fid=%d, gid=%d)", fid, gid);
    }
}

void egis_hidl::on_error(void* context, int code)
{
    ALOGD("egis_hidl on_error enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    int32_t vendor_code = 0;
    FingerprintError error = static_cast<FingerprintError>(code);
    int error_vendor = static_cast<int>(FingerprintError::ERROR_VENDOR);

    if (code >= error_vendor) {
        vendor_code = code - error_vendor;
        error = FingerprintError::ERROR_VENDOR;
    }
    ALOGE("onError(error=%d, vendor=%d)", code, vendor_code);
    if (!me->mClientCallback->onError(reinterpret_cast<uint64_t>(context), error, vendor_code).isOk()) {
        ALOGE("failed to invoke fingerprint onError callback");
    }
}

void egis_hidl::on_removed(void* context, uint32_t fid, uint32_t gid,
                   uint32_t remaining)
{
    ALOGD("egis_hidl on_removed enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    ALOGD("onRemoved(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if (!me->mClientCallback->onRemoved(reinterpret_cast<uint64_t>(context), fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onRemoved callback");
    }
}

void egis_hidl::on_enumerate(void* context, uint32_t fid,
                                       uint32_t gid, uint32_t remaining)
{
    ALOGD("egis_hidl on_enumerate enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    ALOGD("onEnumerate(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if (!me->mClientCallback->onEnumerate(reinterpret_cast<uint64_t>(context), fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnumerate callback");
    }
}

void egis_hidl::on_sync_templates(void *context, uint32_t *fids, uint32_t fids_count, uint32_t user_id)
{
    ALOGD("egis_hidl on_sync_templates enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    const std::vector<uint32_t> hidl_fids(fids, fids + fids_count);
    if (fids_count == 0) {
        ALOGD("fingerprint template is null");
        if (!me->mClientCallback->onEnumerate(reinterpret_cast<uint64_t>(context), 0, user_id, 0).isOk()) {
            ALOGE("failed to invoke fingerprint onEnumerate callback");
        }
    }
    for (int32_t index = 0; index < fids_count; index++) {
        ALOGE("fids[%d] = %d, gids[%d] = %d", index, fids[index], index, user_id);
        if (!me->mClientCallback->onEnumerate(reinterpret_cast<uint64_t>(context), fids[index], user_id, fids_count - index - 1).isOk()) {
            ALOGE("failed to invoke fingerprint onEnumerate callback");
        }
    }
}

void egis_hidl::on_touch_up(void *context)
{
    ALOGD("egis_hidl on_touch_up enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    const uint64_t devId = reinterpret_cast<uint64_t>(context);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    if (!me->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_UP, hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
        ALOGE("failed to invoke fingerprint on_touch_up callback");
    }
}

void egis_hidl::on_touch_down(void *context)
{
    ALOGD("egis_hidl on_touch_down enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    const uint64_t devId = reinterpret_cast<uint64_t>(context);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    if (!me->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_DOWN,
    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
        ALOGE("failed to invoke fingerprint on_touch_down callback");
    }
}

void egis_hidl::on_monitor_event_triggered(void *context, uint32_t type, char* data)
{
    ALOGD("egis_hidl on_monitor_event_triggered enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    hidl_string str_data;

}

void egis_hidl::on_image_info_acquired(void *context, uint32_t type, uint32_t quality, uint32_t match_score)
{
    ALOGD("egis_hidl on_image_info_acquired enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
}

hidl_string egis_hidl::getHidlstring(uint32_t param)
{
    char data[64];
    snprintf(data, 63, "%u", param);
    ALOGD("getString16 param=%u", param);
    return hidl_string(data, strlen(data));
}

void egis_hidl::on_engineering_info_update(void *context, uint32_t type, int result, int ext_info)
{
    ALOGD("egis_hidl on_engineering_info_update enter!");
    egis_hidl* me = static_cast<egis_hidl*>(context);
    int len = 0;
    //hidl_vec<uint32_t> key;
    //hidl_vec<hidl_string> value;

    std::vector<uint32_t> key(3);
    std::vector<hidl_string> value(3);
    switch (type) {
        case EGIS_FINGERPRINT_IMAGE_SNR:
            len = 2;
            key[0] = EGIS_SNR_SUCCESSED;
            value[0] = getHidlstring((result == 0 ? 1 : 0));
            key[1] = EGIS_IMAGE_SNR;
            value[1] = getHidlstring(ext_info);
        break;

        case EGIS_FINGERPRINT_IMAGE_QUALITY: {
            int quality_pass = ext_info > 50 ? 1:0;
            len = 3;
            key[0] = EGIS_SUCCESSED;
            value[0] = getHidlstring((result == 0 ? 1 : 0));
            key[1] = EGIS_IMAGE_QUALITY;
            value[1] = getHidlstring(ext_info);
            key[2] = EGIS_QUALITY_PASS;
            value[2] = getHidlstring(quality_pass);
        }
        break;

        case EGIS_FINGERPRINT_BAD_PIXELS:
            len = 2;
            key[0] = EGIS_SNR_SUCCESSED;
            value[0] = getHidlstring((result == 0 ? 1 : 0));
            key[1] = EGIS_BAD_PIXEL_NUM;
            value[1] = getHidlstring(ext_info);
        break;

        default:
            ALOGE("error test case");
            return;
        break;
    }

    if (!me->mClientCallbackEx->onEngineeringInfoUpdated(len, key, value).isOk()) {
        ALOGE("failed to invoke fingerprint onEngineeringInfoUpdated callback");
    }
}

//RM.BSP.Fingerprint.Basic Qingwen.Liu 20190109 add for fingerprint-key mode begin
Return<int32_t> egis_hidl::setFingerKeymode(uint32_t enable)
{
    ALOGI("setFingerKeymode enable:%d", enable);
    return fingerprint_keymode_enable(mDevice, enable);
}
//RM.BSP.Fingerprint.Basic Qingwen.Liu 20190109  add for fingerprint-key mode end
