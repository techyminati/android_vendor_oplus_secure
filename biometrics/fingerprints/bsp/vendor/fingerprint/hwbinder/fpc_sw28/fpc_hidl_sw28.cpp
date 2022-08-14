/************************************************************************************
 ** File: - fpc\fpc_hal\fingerprint_hwbinder\fpc_hidl.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for FPC (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,21/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Ziqing.guo   2017/10/21        create the file
 **  Ziqing.guo   2017/10/21        use hidl to replace bindle interface
 **  Ziqing.guo   2017/10/26        add the new apis
 **  Ziqing.guo   2017/11/01        remove the vendor message conversion
 **  Ziqing.guo   2017/11/06        add the callback on_imageinfo_acquired
 **  Ziqing.guo   2017/11/06        add the callback on_sync_templates
 **  Ziqing.guo   2017/11/09        add the callback on_engineeringinfo_updated
 **  Ziqing.guo   2017/11/10        fix the problem of string length
 **  Ran.Chen     2018/03/15        add for SNR test
 **  Ran.Chen     2018/03/21        add fpc_monitor
 **  Ran.Chen     2018/03/27        add for pid notifyInit
 **  Ziqing.Guo   2018/09/13        add for distingue fingerprint unlock and pay
 **  Yang.Tan     2018/11/20        modify to sw28
 **  Yang.Tan     2018/11/26        add fpc sw23 and sw28 compatible
 **  Yang.Tan     2018/12/11        add fingerprint image quality pass  in engineeringinfo
 **  Ziqing.Guo   2019/01/07        add for sloving the problem of memory leak
 **  Long.Liu     2019/03/06        add for commondcs msg (fingerprint_MonitorEvent)
 ************************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <cutils/log.h>
#include <fingerprint.h>

#include "fpc_hidl_sw28.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include <string.h>
#include "fingerprint_type.h"

#ifdef FP_DCS_ENABLE
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"
#endif /* FP_DCS_ENABLE */

#ifdef FP_HYPNUSD_ENABLE
using android::Hypnus;
#endif /* FP_HYPNUSD_ENABLE */

#ifdef FP_DCS_ENABLE
using android::Dcs;
using android::HealthMonitor;
#endif /* FP_DCS_ENABLE */

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

static int gCode = 0;

fpc_hidl::fpc_hidl() : hal(nullptr), mClientCallbackEx(nullptr) {
    compat_callback.on_acquired = on_acquired;
    compat_callback.on_authenticated = on_authenticated;
    compat_callback.on_enroll_result = on_enroll_result;
    compat_callback.on_enumerate = on_enumerate;
    compat_callback.on_error = on_error;
    compat_callback.on_removed = on_removed;
    compat_callback.on_touch_down = on_touch_down;
    compat_callback.on_touch_up = on_touch_up;
    //compat_callback.on_imageinfo_acquired = on_imageinfo_acquired;
    compat_callback.on_engineeringinfo_updated = on_engineeringinfo_updated;
    //compat_callback.on_monitor = on_monitor;
    compat_callback.on_dcsmsg = on_dcsmsg;
    compat_callback.hypnusSetAction = hypnusSetAction;
    compat_callback.fpc_bind_bigcore_bytid = fpc_bind_bigcore_bytid;
}

fpc_hidl::~fpc_hidl() {
    fpc_hal_close(hal);
}

int fpc_hidl::init() {
    return fpc_hal_open(&hal, &compat_callback, this);
}

void fpc_hidl::hypnusSetAction() {
#ifdef FP_HYPNUSD_ENABLE
    Hypnus::getInstance()->setAction(ACTION_IO, 1000);
#endif
}

void fpc_hidl::fpc_bind_bigcore_bytid(uint32_t tid) {
#ifdef FP_HYPNUSD_ENABLE
    Hypnus::getInstance()->bind_big_core_bytid(tid);
#endif
}

String16 fpc_hidl::getString16(uint32_t param) {
    char data[64];
    snprintf(data, 63, "%u", param);
    LOGD("getString16(param=%u)", param);
    return android::String16(data);
}

String16 fpc_hidl::getString16(double param) {
    char data[64];
    snprintf(data, 63, "%-10.3f", param);
    LOGD("getString16(param=%-10.3f)", param);
    return android::String16(data);
}

String16 fpc_hidl::getString16(float param) {
    char data[64];
    snprintf(data, 63, "%-10.3f", param);
    LOGD("getString16(param=%-10.3f)", param);
    return android::String16(data);
}

hidl_string fpc_hidl::getHidlstring(uint32_t param) {
    char data[64];
    snprintf(data, 63, "%u", param);
    LOGD("getString16(param=%u)", param);
    return hidl_string(data, strlen(data));
}

Return<uint64_t> fpc_hidl::setNotify(
        const sp<IBiometricsFingerprintClientCallback>& clientCallback) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    callback = clientCallback;
    // This is here because HAL 2.1 doesn't have a way to propagate a
    // unique token for its driver. Subsequent versions should send a unique
    // token for each call to setNotify(). This is fine as long as there's only
    // one fingerprint device on the platform.
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> fpc_hidl::setHalCallback(
        const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) {
    std::lock_guard<std::mutex> lock(callback_mutex);
    mClientCallbackEx = clientCallbackEx;
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> fpc_hidl::preEnroll() {
    return fpc_pre_enroll(hal);
}

Return<RequestStatus> fpc_hidl::enroll(const hidl_array<uint8_t, 69>& hat,
        uint32_t gid, uint32_t timeoutSec) {
    if (fpc_enroll(hal, static_cast<const uint8_t*>(hat.data()), 69, gid, timeoutSec)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::postEnroll() {
    if (fpc_post_enroll(hal)) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<uint64_t> fpc_hidl::getAuthenticatorId() {
    return fpc_get_authenticator_id(hal);
}

Return<RequestStatus> fpc_hidl::cancel() {
    fpc_cancel(hal);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::enumerate() {
    fpc_enumerate(hal);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::remove(uint32_t gid, uint32_t fid) {
    if (fpc_remove(hal, gid, fid)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::setActiveGroup(uint32_t gid, const hidl_string& storePath) {
    uint32_t template_status = 0;
    if (gCode >= HAL_COMPAT_ERROR_ALIVE_CHECK) {
        ALOGE("[%s] notify android to restart or shutdown", __func__);
        on_error(this, gCode);
        gCode = 0;
        return RequestStatus::SYS_UNKNOWN;
    }
    int ret = 0;
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        ALOGE("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        return RequestStatus::SYS_EINVAL;
    }

    ret = fpc_set_active_group(hal, gid, storePath.c_str(), &template_status);

#ifdef FP_DCS_ENABLE
    sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
       vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
    if (service != NULL) {
        ALOGE(" has service setActiveGroup template_status =%d", template_status);
        hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> msg;
        vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[1] = {0};
        dataArray[0].key = "template_status";
        if (HAL_LOAD_TEMPLATE_NORMAL == template_status) {
            dataArray[0].value = "HAL_LOAD_TMMPLATE_NORMAL";
        } else if (HAL_LOAD_TEMPLATE_RESTORED_SUCCEE == template_status) {
            dataArray[0].value = "TMMPLATE_RESTORED_SUCCEE";
        } else if (HAL_LOAD_TEMPLATE_RESTORED_FAIL_EMPTYDB_SUCC == template_status) {
            dataArray[0].value = "TMMPLATE_RESTORED_FAIL_EMPTYDB_SUCC";
        } else if (HAL_LOAD_TEMPLATE_RESTORED_FAIL_EMPTYDB_FAIL == template_status) {
            dataArray[0].value = "TMMPLATE_RESTORED_FAIL_EMPTYDB_FAIL";
        }
        msg.setToExternal(dataArray, 1);
        service->notifyMsgToCommonDcs(msg, "20120", "fingerprint_template_event");
    } else {
        ALOGE("service NULL");
    }
#endif /* FP_DCS_ENABLE */

    if (ret) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::authenticate(uint64_t operationId, uint32_t gid) {
    if (fpc_authenticate(hal, operationId, gid)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::authenticateAsType(uint64_t operationId, uint32_t gid, int32_t authtype) {
    UNUSED(operationId);
    UNUSED(gid);
    UNUSED(authtype);
    return RequestStatus::SYS_OK;
}

Return<int32_t> fpc_hidl::getEnrollmentTotalTimes() {
    return fpc_get_enrollment_total_times(hal);
}

Return<RequestStatus> fpc_hidl::pauseEnroll() {
    if (fpc_pause_enroll(hal)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::continueEnroll() {
    if (fpc_continue_enroll(hal)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::pauseIdentify() {
    if (fpc_pause_identify(hal)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::continueIdentify() {
    if (fpc_continue_identify(hal)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::setTouchEventListener() {
    if (fpc_wait_finger_down(hal)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::setScreenState(int32_t screen_state) {
    if (fpc_set_screen_state(hal, static_cast<int32_t>(screen_state))) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::dynamicallyConfigLog(uint32_t on) {
    if (fpc_open_log(hal, on)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<int32_t> fpc_hidl::getEngineeringInfo(uint32_t type) {
    return fpc_get_engineering_Info(hal, type);
}

void fpc_hidl::on_enroll_result(void* context, uint32_t fid, uint32_t gid, uint32_t remaining)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    std::lock_guard<std::mutex> lock(me->callback_mutex);
    if (me->callback == NULL) {
        ALOGE("Callback not yet registered");
        return;
    }
    ALOGD("onEnrollResult(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);

    if (!me->callback->onEnrollResult(reinterpret_cast<uint64_t>(context),
                fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnrollResult callback");
    }
}

void fpc_hidl::on_acquired(void* context, int code)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    int32_t vendor_code = 0;
    FingerprintAcquiredInfo acquired_info = static_cast<FingerprintAcquiredInfo>(code);
    std::lock_guard<std::mutex> lock(me->callback_mutex);
    if (me->callback == NULL) {
        ALOGE("Callback not yet registered");
        return;
    }
#if 0
    if (code >= HAL_COMPAT_VENDOR_BASE) {
        vendor_code = code - HAL_COMPAT_VENDOR_BASE;
        acquired_info = FingerprintAcquiredInfo::ACQUIRED_VENDOR;
    }
#endif
    ALOGD("onAcquired(code=%d, vendor=%d)", acquired_info, vendor_code);
    if (!me->callback->onAcquired(reinterpret_cast<uint64_t>(context),
                acquired_info, vendor_code).isOk()) {
        ALOGE("failed to invoke fingerprint onAcquired callback");
    }
}

void fpc_hidl::on_authenticated(void* context, uint32_t fid, uint32_t gid,
        const uint8_t* token, uint32_t size_token)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    const std::vector<uint8_t> hidl_token(token, token + size_token);
    std::lock_guard<std::mutex> lock(me->callback_mutex);
    if (me->callback == NULL) {
        ALOGE("Callback not yet registered");
        return;
    }
    ALOGD("onAuthenticated(fid=%d, gid=%d)", fid, gid);
    if (!me->callback->onAuthenticated(reinterpret_cast<uint64_t>(context),
                fid, gid, hidl_vec<uint8_t>(hidl_token)).isOk()) {
        ALOGE("failed to invoke fingerprint onAuthenticated callback");
    }
}

void fpc_hidl::on_error(void* context, int code)
{
    /* for case of code >= 9000, only if F_1541 and Homer project can call onError, otherwise return directly. */
    if (code >= HAL_COMPAT_ERROR_ALIVE_CHECK) {
        if (0 != strcmp("F_1541", fp_config_info_init.fp_id_string)) {
            ALOGE("not F_1541 chip (code=%d), return ", code);
            return;
        }
#ifndef FPC_DDR_SAME_POWER_SUPPLY
        ALOGE("not homer project (code=%d), return ", code);
        return;
#endif
    }
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    std::lock_guard<std::mutex> lock(me->callback_mutex);
    if (me->callback == NULL) {
        gCode = code;
        ALOGE("Callback not yet registered, (code=%d)", code);
        return;
    }
    int32_t vendor_code = 0;
    FingerprintError error = static_cast<FingerprintError>(code);

    if (code >= HAL_COMPAT_VENDOR_BASE) {
        vendor_code = code - HAL_COMPAT_VENDOR_BASE;
        error = FingerprintError::ERROR_VENDOR;
    }
    ALOGD("onError(error=%d, vendor=%d)", code, vendor_code);
    if (!me->callback->onError(reinterpret_cast<uint64_t>(context),
                error, vendor_code).isOk()) {
        ALOGE("failed to invoke fingerprint onError callback");
    }
}

void fpc_hidl::on_removed(void* context, uint32_t fid, uint32_t gid,
        uint32_t remaining)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    std::lock_guard<std::mutex> lock(me->callback_mutex);
    if (me->callback == NULL) {
        ALOGE("Callback not yet registered");
        return;
    }
    ALOGD("onRemoved(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if (!me->callback->onRemoved(reinterpret_cast<uint64_t>(context),
                fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onRemoved callback");
    }
}

void fpc_hidl::on_enumerate(void* context, uint32_t fid,
        uint32_t gid, uint32_t remaining)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    std::lock_guard<std::mutex> lock(me->callback_mutex);
    if (me->callback == NULL) {
        ALOGE("Callback not yet registered");
        return;
    }
    ALOGD("onEnumerate(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if (!me->callback->onEnumerate(reinterpret_cast<uint64_t>(context),
                fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnumerate callback");
    }
}


void fpc_hidl::on_touch_down(void* context) {
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    const uint64_t devId = reinterpret_cast<uint64_t>(context);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    ALOGD("onTouchDown");
    if (me->mClientCallbackEx == NULL) {
        ALOGE("CallbackEx not yet registered");
        return;
    }
    me->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_DOWN,
               hidl_vec<int8_t>(deviceId), sizeof(uint64_t));
}

void fpc_hidl::on_touch_up(void* context) {
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    const uint64_t devId = reinterpret_cast<uint64_t>(context);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    ALOGD("onTouchUp");
    if (me->mClientCallbackEx == NULL) {
        ALOGE("CallbackEx not yet registered");
        return;
    }
    me->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_UP,
                hidl_vec<int8_t>(deviceId), sizeof(uint64_t));
}

void fpc_hidl::on_engineeringinfo_updated(void* context, engineering_info_t engineering) {
    uint32_t len = 0;
    uint32_t *key = NULL;
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    hidl_string *value = NULL;
    ALOGD("on_engineeringinfo_updated(type=%d)", engineering.type);
    switch (engineering.type) {
        case FINGERPRINT_IMAGE_QUALITY:
            len = 3;
            key = (uint32_t*)malloc(len * sizeof(uint32_t));
            value = new hidl_string[len];
            key[0]   = SUCCESSED;
            value[0] = getHidlstring(engineering.quality.successed);
            key[1]   = IMAGE_QUALITY;
            value[1] = getHidlstring(engineering.quality.image_quality);
            key[2]   = QUALITY_PASS;
            value[2] = getHidlstring(engineering.quality.quality_pass);
            break;
        case FINGERPRINT_IMAGE_SNR:
            len = 2;
            key = (uint32_t*)malloc(len * sizeof(uint32_t));
            value = new hidl_string[len];

            key[0]   = SNR_SUCCESSED;
            value[0] = getHidlstring(engineering.snr.snr_successed);
            key[1]   = IMAGE_SNR;
            value[1] = getHidlstring(engineering.snr.image_snr);
            break;
        case FINGERPRINT_INAGE_SELF_TEST:
            ALOGE(" callback ENGINEERING_INFO FINGERPRINT_INAGE_SELF_TEST result=%d", engineering.self_test_result);
            len = 1;
            key = (uint32_t*)malloc(len * sizeof(uint32_t));
            value = new hidl_string[len];
            if (key == NULL || value == NULL) {
                ALOGE("malloc fail");
                break;
            }
            key[0]   = SUCCESSED;
            value[0] = getHidlstring(engineering.self_test_result);
            break;
        default:
            break;
    }
    if (key == NULL || value == NULL) {
        return;
    }
    const std::vector<uint32_t> hidl_key(key, key + len);
    const std::vector<hidl_string> hidl_value(value, value+len);
    int count = hidl_value.size();
    for (int i = 0; i < count; i++) {
        ALOGD("onEngineeringInfoUpdated ,key[%d] = %d, value[%d] = %s", i, hidl_key[i], i, (android::String8(hidl_value[i].c_str())).string());
    }
    if (me->mClientCallbackEx != NULL) {
        me->mClientCallbackEx->onEngineeringInfoUpdated(len, hidl_vec<uint32_t>(hidl_key), hidl_vec<hidl_string>(hidl_value));
    } else {
        ALOGE("CallbackEx not yet registered");
    }
    if (key) {
        free(key);
        key = NULL;
    }
    if (value) {
        delete []value;
        value = NULL;
    }
}

#if 0
void fpc_hidl::on_imageinfo_acquired(void* context, uint32_t type, uint32_t quality, uint32_t match_score) {
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    ALOGD("on_imageinfo_acquired");
}

void fpc_hidl::on_sync_templates(void* context, const uint32_t* fids, uint32_t size_fid, uint32_t gid) {
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    const std::vector<uint32_t> hidl_fids(fids, fids + size_fid);
    ALOGD("on_sync_templates(size_fid=%d, gid=%d)", size_fid, gid);
    me->callback->onSyncTemplates(reinterpret_cast<uint64_t>(context),
            hidl_vec<uint32_t>(hidl_fids), gid);
}

void fpc_hidl::on_monitor(void* context, uint32_t type, double data) {
    char data_buffer[64];
    size_t size = 0;
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    ALOGD("on_monitor(type=%d, data=%f)", type, data);
    switch (type) {
        case fingerprint_monitor_type_t::FINGER_POWER_MONITOR:
            ALOGD("FINGER_POWER_MONITOR(type=%d, battery_mode=%f)", type, data);
            size = snprintf(data_buffer, 63, "%f", data);
            me->callback->onMonitorEventTriggered(type, hidl_string(data_buffer));
            break;
        case fingerprint_monitor_type_t::FINGER_TP_PROTECT_MONITOR:
            ALOGD("FINGER_TP_PROTECT_MONITOR(type=%d, battery_mode=%f)", type, data);
            size = snprintf(data_buffer, 63, "%d", (int)data);
            me->callback->onMonitorEventTriggered(type, hidl_string(data_buffer));
            break;
        case fingerprint_monitor_type_t::FINGER_ERROR_STATE:
            ALOGD("on_monitor FINGER_ERROR_STATE");
            break;
        default:
            {}//ignore
    }
#ifdef FP_DCS_ENABLE
    if (size > 0) {
        ALOGD("enter into onMonitorEventTriggered CommonDcsmsg ");
        sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service =
           vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
        if (service != NULL) {
            ALOGD(" send fpc28 fingerprint onMonitorEventTriggered dcsmsg");
            hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> dcsmsg;
            vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[1] = {0};
            dataArray[0].key = "MonitorEvent";
            dataArray[0].value = hidl_string(data_buffer);

            dcsmsg.setToExternal(dataArray, 1);
            service->notifyMsgToCommonDcs(dcsmsg, "20120", "fingerprint_MonitorEvent");
        } else {
            ALOGE("service NULL");
        }
    }
#endif /* FP_DCS_ENABLE */
}
#endif

void fpc_hidl::on_dcsmsg(fingerprint_auth_dcsmsg_t auth_context) {
    ALOGD("Auth, auth_result = %d", auth_context.auth_result);
    ALOGD("Auth, fail_reason = %d", auth_context.fail_reason);
    ALOGD("Auth, quality_score = %d", auth_context.quality_score);
    ALOGD("Auth, match_score = %d",  auth_context.match_score);
    ALOGD("Auth, signal_value = %d",  auth_context.signal_value);
    ALOGD("Auth, img_area = %d", auth_context.img_area);
    ALOGD("Auth, retry_times = %d", auth_context.retry_times);
    ALOGD("Auth, algo_version = %s", auth_context.algo_version);
    ALOGD("Auth, chip_ic = %d",  auth_context.chip_ic);
    ALOGD("Auth, module_type = %d", auth_context.module_type);
    ALOGD("Auth, lense_type = %d", auth_context.lense_type);
    ALOGD("Auth, dsp_available = %d", auth_context.dsp_availalbe);
#ifdef FP_DCS_ENABLE
    Dcs::getInstance()->reportAuthenticatedInfo(auth_context);
#endif /* FP_DCS_ENABLE */
}

Return<bool> fpc_hidl::isUdfps(uint32_t sensorId) {
    ALOGD("isUdfps sensor_id = %d", sensorId);
    if (sensorId >= E_SENSOR_ID_MAX) {
        ALOGE("unknown sensor");
        return false;
    }
    return (fp_config_info[sensorId].fp_type) == 4? true:false;
}

Return<void> fpc_hidl::onFingerDown(uint32_t x, uint32_t y, float minor, float major) {
    ALOGD("onFingerDown");
    //mDevice->touchDown(mDevice);
    return Void();
}

Return<void> fpc_hidl::onFingerUp() {
    ALOGD("onFingerUp");
    //mDevice->touchUp(mDevice);
    return Void();
}


Return<int32_t> fpc_hidl::sendFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& in_buf) {
    ALOGD("sendFingerprintCmd ");
    ALOGE("sendFingerprintCmd in_buf= %s ", (int8_t*)in_buf.data());
    int ret = FINGERPRINT_RIGHTNESS_ERROR;
    switch (cmdId) {
        case FINGERPRINT_CMD_ID_CAMERA: //CMD_FINGERPRINT_CAMERA
            ALOGD("keymode_enable(enable = %d)", in_buf[0]);
            ret = fpc_keymode_enable(hal, static_cast<int32_t>(in_buf[0]));
            break;
        case FINGERPRINT_PRODUCT_TEST_CMD_GET_OTP_QRCODE:
            ALOGD("sendfingercmd = %d)", cmdId);
            ret = fpc_hal_notify_qrcode(hal, cmdId);
            break;
        case FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE:
        {
            uint64_t operationId = 0;
            uint32_t gid = 0;
            int32_t authtype;

            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&authtype, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE operationId = %d, gid = %d, authtype = %d", operationId, gid, authtype);

            authenticateAsType(operationId, gid, authtype);
            return 0;
        }
        case FINGERPRINT_CMD_ID_PAUSE_ENROLL:
        {
            pauseEnroll();
            return 0;
        }
        case FINGERPRINT_CMD_ID_CONTINUE_ENROLL:
        {
            continueEnroll();
            return 0;
        }
        case FINGERPRINT_CMD_ID_SET_TOUCHEVENT_LISTENER:
        {
            setTouchEventListener();
            return 0;
        }
        case FINGERPRINT_CMD_ID_DYNAMICALLY_LOG:
        {
            uint32_t on = 0;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&on, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_DYNAMICALLY_LOG on = %d", on);
            dynamicallyConfigLog(on);
            return 0;
        }
        case FINGERPRINT_CMD_ID_GET_ENROLL_TIMES:
        {
            int32_t enroll_times = 0;
            enroll_times = getEnrollmentTotalTimes();
            return enroll_times;
        }
        case FINGERPRINT_CMD_ID_SET_SCREEN_STATE:
        {
            int32_t screen_state;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&screen_state, value, sizeof(int32_t));
            ALOGE("FINGERPRINT_CMD_ID_SET_SCREEN_STATE screen_state = %d", screen_state);
            setScreenState(screen_state);
            return 0;
        }
        case FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO:
        {
            uint32_t type = 0;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&type, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO type = %d", type);
            return getEngineeringInfo(type);
        }
        case FINGERPRINT_CMD_ID_GET_SENSOR_ID:
        {
            ALOGE("FINGERPRINT_CMD_ID_GET_SENSOR_ID = %d", fp_config_info_init.sensor_id);
            return fp_config_info_init.sensor_id;
        }
        case FINGERPRINT_CMD_ID_SIDE_PRESS_ENABLE:
        {
            return 0;
        }
        case FINGERPRINT_CMD_ID_SIDE_SCREEN_STATE:
        {
            return 0;
        }
        case FINGERPRINT_CMD_ID_SIDE_POWER_KEY_PRESSED:
        {
            return 0;
        }
        default:
            break;
    }
    return ret;
}

