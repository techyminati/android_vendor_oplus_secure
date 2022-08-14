/************************************************************************************
 ** File: - goodix\fingerprint\fingerprint_hwbinder\BiometricsFingerprint.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix(android O)
 **
 ** Version: 1.0
 ** Date created: 15:09:11,18/11/2017
 ** Author: Ran.chen@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         2017/11/18        add IMAGET_QUALITY for goodix
 **  Ran.Chen         2017/11/21        modify enroll_time to 10min
 **  Ziqing.Guo       2017/12/03        fix the warning message
 **  Ran.Chen         2018/02/07        add for FINGERPRINT_IMAGE_INFO
 **  Ran.Chen         2018/03/27        add for pid notifyInit
 **  Ziqing.Guo       2018/09/13        add for distingue fingerprint unlock and pay
 **  Ziqing.Guo       2019/01/07        add for sloving the preblem forward null
 ************************************************************************************/

#define LOG_VERBOSE "vendor.oplus.hardware.biometrics.fingerprint@2.1-service"

#include <hardware/hw_auth_token.h>
#ifdef __FACT_TEST
#include <vendor/goodix/hardware/fingerprintextension/1.0/IGoodixBiometricsFingerprint.h>
#include "GoodixBiometricsFingerprint.h"
#endif

#include <hardware/hardware.h>
#include <fingerprint.h>
#include "BiometricsFingerprint.h"

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <string.h>
#include "public/gf_hal.h"
//#include "registerService.h"

#include "fingerprint_type.h"

#ifdef FP_DCS_ENABLE
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"
#endif /* FP_DCS_ENABLE */

using android::Hypnus;
using android::Dcs;
using android::HealthMonitor;

namespace vendor {
namespace oplus {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {
namespace implementation {

#ifdef __FACT_TEST
using ::vendor::goodix::hardware::fingerprintextension::V1_0::IGoodixBiometricsFingerprint;
using ::vendor::goodix::hardware::fingerprintextension::V1_0::implementation::GoodixBiometricsFingerprint;
#endif

using RequestStatus =
        android::hardware::biometrics::fingerprint::V2_1::RequestStatus;

BiometricsFingerprint *BiometricsFingerprint::sInstance = nullptr;

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

BiometricsFingerprint::BiometricsFingerprint():mClientCallback(nullptr), mClientCallbackEx(nullptr) {
    sInstance = this; // keep track of the most recent instance
}

BiometricsFingerprint::~BiometricsFingerprint() {
}

Return<RequestStatus> BiometricsFingerprint::ErrorFilter(int32_t error) {
    switch (error) {
        case 0: return RequestStatus::SYS_OK;
        case -2: return RequestStatus::SYS_ENOENT;
        case -4: return RequestStatus::SYS_EINTR;
        case -5: return RequestStatus::SYS_EIO;
        case -11: return RequestStatus::SYS_EAGAIN;
        case -12: return RequestStatus::SYS_ENOMEM;
        case -13: return RequestStatus::SYS_EACCES;
        case -14: return RequestStatus::SYS_EFAULT;
        case -16: return RequestStatus::SYS_EBUSY;
        case -22: return RequestStatus::SYS_EINVAL;
        case -28: return RequestStatus::SYS_ENOSPC;
        case -110: return RequestStatus::SYS_ETIMEDOUT;
        default:
            ALOGE("An unknown error returned from fingerprint vendor library: %d", error);
            return RequestStatus::SYS_UNKNOWN;
    }
}

// Translate from errors returned by traditional HAL (see fingerprint.h) to
// HIDL-compliant FingerprintError.
FingerprintError BiometricsFingerprint::vendorErrorFilter(int32_t error,
            int32_t* vendorCode) {
    *vendorCode = 0;
    switch (error) {
        case FINGERPRINT_ERROR_HW_UNAVAILABLE:
            return FingerprintError::ERROR_HW_UNAVAILABLE;
        case FINGERPRINT_ERROR_UNABLE_TO_PROCESS:
            return FingerprintError::ERROR_UNABLE_TO_PROCESS;
        case FINGERPRINT_ERROR_TIMEOUT:
            return FingerprintError::ERROR_TIMEOUT;
        case FINGERPRINT_ERROR_NO_SPACE:
            return FingerprintError::ERROR_NO_SPACE;
        case FINGERPRINT_ERROR_CANCELED:
            return FingerprintError::ERROR_CANCELED;
        case FINGERPRINT_ERROR_UNABLE_TO_REMOVE:
            return FingerprintError::ERROR_UNABLE_TO_REMOVE;
        case FINGERPRINT_ERROR_LOCKOUT:
            return FingerprintError::ERROR_LOCKOUT;
        default:
            if (error >= FINGERPRINT_ERROR_VENDOR_BASE) {
                // vendor specific code.
                *vendorCode = error - FINGERPRINT_ERROR_VENDOR_BASE;
                return FingerprintError::ERROR_VENDOR;
            }
    }
    ALOGE("Unknown error from fingerprint vendor library: %d", error);
    return FingerprintError::ERROR_UNABLE_TO_PROCESS;
}

// Translate acquired messages returned by traditional HAL (see fingerprint.h)
// to HIDL-compliant FingerprintAcquiredInfo.
FingerprintAcquiredInfo BiometricsFingerprint::vendorAcquiredFilter(
        int32_t info, int32_t* vendorCode) {
    *vendorCode = 0;
    switch (info) {
        case FINGERPRINT_ACQUIRED_GOOD:
            return FingerprintAcquiredInfo::ACQUIRED_GOOD;
        case FINGERPRINT_ACQUIRED_PARTIAL:
            return FingerprintAcquiredInfo::ACQUIRED_PARTIAL;
        case FINGERPRINT_ACQUIRED_INSUFFICIENT:
            return FingerprintAcquiredInfo::ACQUIRED_INSUFFICIENT;
        case FINGERPRINT_ACQUIRED_IMAGER_DIRTY:
            return FingerprintAcquiredInfo::ACQUIRED_IMAGER_DIRTY;
        case FINGERPRINT_ACQUIRED_TOO_SLOW:
            return FingerprintAcquiredInfo::ACQUIRED_TOO_SLOW;
        case FINGERPRINT_ACQUIRED_TOO_FAST:
            return FingerprintAcquiredInfo::ACQUIRED_TOO_FAST;
        case FINGERPRINT_ACQUIRED_ALREADY_ENROLLED:
            return (FingerprintAcquiredInfo)1002;    //FingerprintAcquiredInfo::ACQUIRED_ALREADY_ENROLLED;
        case FINGERPRINT_ACQUIRED_TOO_SIMILAR:
            return (FingerprintAcquiredInfo)1001;    //FingerprintAcquiredInfo::ACQUIRED_TOO_SIMILAR;
        default:
            if (info >= FINGERPRINT_ACQUIRED_VENDOR_BASE) {
                // vendor specific code.
                *vendorCode = info - FINGERPRINT_ACQUIRED_VENDOR_BASE;
                return FingerprintAcquiredInfo::ACQUIRED_VENDOR;
            }
    }
    ALOGE("Unknown acquiredmsg from fingerprint vendor library: %d", info);
    return FingerprintAcquiredInfo::ACQUIRED_INSUFFICIENT;
}

hidl_string BiometricsFingerprint::getHidlstring(uint32_t param) {
        char data[64];
        snprintf(data, 63, "%u", param);
        ALOGD("getString16(param=%u)", param);
        return hidl_string(data, strlen(data));
}

Return<uint64_t> BiometricsFingerprint::setNotify(
        const sp<IBiometricsFingerprintClientCallback>& clientCallback) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    mClientCallback = clientCallback;
    // This is here because HAL 2.1 doesn't have a way to propagate a
    // unique token for its driver. Subsequent versions should send a unique
    // token for each call to setNotify(). This is fine as long as there's only
    // one fingerprint device on the platform.
    return reinterpret_cast<uint64_t>(&mDevice);
}

Return<uint64_t> BiometricsFingerprint::setHalCallback(
        const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    mClientCallbackEx = clientCallbackEx;
    return reinterpret_cast<uint64_t>(&mDevice);
}
Return<uint64_t> BiometricsFingerprint::preEnroll()  {
    return gf_hal_pre_enroll(&mDevice);
}

Return<RequestStatus> BiometricsFingerprint::enroll(const hidl_array<uint8_t, 69>& hat,
        uint32_t gid, uint32_t timeoutSec) {
    (void) timeoutSec;
    uint32_t fingerprint_enroll_time = 0;
    const hw_auth_token_t* authToken =
        reinterpret_cast<const hw_auth_token_t*>(hat.data());
    return ErrorFilter(gf_hal_enroll(&mDevice, authToken, gid, fingerprint_enroll_time));
}

Return<RequestStatus> BiometricsFingerprint::postEnroll() {
    return ErrorFilter(gf_hal_post_enroll(&mDevice));
}

Return<uint64_t> BiometricsFingerprint::getAuthenticatorId() {
    return gf_hal_get_auth_id(&mDevice);
}

Return<RequestStatus> BiometricsFingerprint::cancel() {
    return ErrorFilter(gf_hal_cancel(&mDevice));
}

Return<RequestStatus> BiometricsFingerprint::enumerate()  {
    return ErrorFilter(gf_hal_enumerate_with_callback(&mDevice));
}

Return<RequestStatus> BiometricsFingerprint::remove(uint32_t gid, uint32_t fid) {
    return ErrorFilter(gf_hal_remove(&mDevice, gid, fid));
}

Return<RequestStatus> BiometricsFingerprint::setActiveGroup(uint32_t gid,
        const hidl_string& storePath) {
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        ALOGE("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        ALOGE("storePath can not access");
        return RequestStatus::SYS_EINVAL;
    }
    return ErrorFilter(gf_hal_set_active_group(&mDevice, gid, storePath.c_str()));
}

Return<RequestStatus> BiometricsFingerprint::authenticate(uint64_t operationId,
        uint32_t gid) {
    return ErrorFilter(gf_hal_authenticate(&mDevice, operationId, gid));
}

Return<RequestStatus> BiometricsFingerprint::authenticateAsType(uint64_t operationId,
        uint32_t gid, int32_t authtype) {
    UNUSED(operationId);
    UNUSED(gid);
    UNUSED(authtype);
    return RequestStatus::SYS_OK;
}

Return<int32_t> BiometricsFingerprint::getEnrollmentTotalTimes() {
    return gf_get_enrollment_total_times();
}

Return<RequestStatus> BiometricsFingerprint::pauseEnroll() {
    ALOGI("enter pauseEnroll");
    return ErrorFilter(gf_hal_pause_enroll());
}

Return<RequestStatus> BiometricsFingerprint::continueEnroll() {
    ALOGI("enter continueEnroll");
    return ErrorFilter(gf_hal_continue_enroll());
}


Return<RequestStatus> BiometricsFingerprint::setTouchEventListener() {
    return ErrorFilter(gf_hal_wait_touch_down());
}

Return<RequestStatus> BiometricsFingerprint::setScreenState(int32_t screen_state) {
    return ErrorFilter(gf_hal_set_finger_screen(static_cast<int32_t>(screen_state)));
}

Return<RequestStatus> BiometricsFingerprint::dynamicallyConfigLog(uint32_t on) {
    return ErrorFilter(gf_open_debug(on));
}

Return<int32_t> BiometricsFingerprint::getEngineeringInfo(uint32_t type) {
    int32_t ret = -1;
    fingerprint_msg_t msg;
    switch (type) {
        case FINGERPRINT_GET_IMAGE_SNR:
            ALOGE("getImageSnr   BEGIN_CALL_API");
            //mDevice->getImageSnr(mDevice);
            ALOGE("getImageSnr   END_CALL_API");
            return 0;
        case FINGERPRINT_GET_IMAGET_QUALITY:
            ALOGE("getImageQuality   BEGIN_CALL_API");
            gf_hal_get_image_quality();
            ALOGE("getImageQuality   END_CALL_API");
            return 0;
        case FINGERPRINT_GET_BAD_PIXELS:
            ALOGE("getBadPixels   BEGIN_CALL_API");
            //ret = mDevice->getBadPixels(mDevice);
            ALOGE("getBadPixels   END_CALL_API");
            break;
        case FINGERPRINT_SELF_TEST:
            ALOGE("selftest   BEGIN_CALL_API");
            ret = gf_hal_module_test();
            msg.type = ENGINEERING_INFO;
            msg.data.engineering.type = FINGERPRINT_INAGE_SELF_TEST;
            msg.data.engineering.self_test_result = ret;
            notify(&msg);
            ALOGE("selftest   END_CALL_API");
            break;
        default:
            ALOGD("invalid msg type: %d", type);
            return -1;
    }
    return ret;
}

Return<RequestStatus> BiometricsFingerprint::pauseIdentify() {
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::continueIdentify() {
    return RequestStatus::SYS_OK;
}



Return<int32_t> BiometricsFingerprint::init() {
    int32_t err;
    err = gf_hal_open(&mDevice);
    if (err) {
        return err;
    }

    mDevice.notify = notify;

#ifdef __FACT_TEST
    android::sp<GoodixBiometricsFingerprint> extBio =
            static_cast<GoodixBiometricsFingerprint *>(GoodixBiometricsFingerprint::getInstance());

    if (extBio != nullptr) {
        extBio->registerAsService();
    } else {
        ALOGE("Can't create instance of BiometricsFingerprint, nullptr");
        return -1;
    }

    extBio->setDevice(&mDevice);
#endif

#ifdef SUPPORT_CMD_TEST
    registerExtVndService(&mDevice);
#endif
    return 0;
}

IBiometricsFingerprint* BiometricsFingerprint::getInstance() {
    if (!sInstance) {
        sInstance = new BiometricsFingerprint();
    }
    return sInstance;
}

void BiometricsFingerprint::notify(const fingerprint_msg_t *msg) {
    BiometricsFingerprint* thisPtr = static_cast<BiometricsFingerprint*>(
            BiometricsFingerprint::getInstance());
    std::lock_guard<std::mutex> lock(thisPtr->mClientCallbackMutex);
    if (thisPtr == nullptr || thisPtr->mClientCallback == nullptr) {
        ALOGE("Receiving callbacks before the client callback is registered.");
        return;
    }
    const uint64_t devId = reinterpret_cast<uint64_t>(&thisPtr->mDevice);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    switch (msg->type) {
        case FINGERPRINT_ERROR: {
                int32_t vendorCode = 0;
                FingerprintError result = vendorErrorFilter(msg->data.error, &vendorCode);
                ALOGD("onError(%d)", result);
                if (!thisPtr->mClientCallback->onError(devId, result, vendorCode).isOk()) {
                    ALOGE("failed to invoke fingerprint onError callback");
                }
            }
            break;
        case FINGERPRINT_ACQUIRED: {
                int32_t vendorCode = 0;
                FingerprintAcquiredInfo result =
                    vendorAcquiredFilter(msg->data.acquired.acquired_info, &vendorCode);
                ALOGD("onAcquired(%d), vendorCode(%d)", result, vendorCode);
                if (!thisPtr->mClientCallback->onAcquired(devId, result, vendorCode).isOk()) {
                    ALOGE("failed to invoke fingerprint onAcquired callback");
                }
            }
            break;
        case FINGERPRINT_TEMPLATE_ENROLLING:
            ALOGD("onEnrollResult(fid=%d, gid=%d, rem=%d)",
                msg->data.enroll.finger.fid,
                msg->data.enroll.finger.gid,
                msg->data.enroll.samples_remaining);
            if (!thisPtr->mClientCallback->onEnrollResult(devId,
                    msg->data.enroll.finger.fid,
                    msg->data.enroll.finger.gid,
                    msg->data.enroll.samples_remaining).isOk()) {
                ALOGE("failed to invoke fingerprint onEnrollResult callback");
            }
            break;
        case FINGERPRINT_TEMPLATE_REMOVED:
            ALOGE("onRemove(fid=%d, gid=%d, rem=%d)",
                msg->data.removed.finger.fid,
                msg->data.removed.finger.gid,
                msg->data.removed.fingers_count);
            if (!thisPtr->mClientCallback->onRemoved(devId,
                    msg->data.removed.finger.fid,
                    msg->data.removed.finger.gid,
                    msg->data.removed.fingers_count).isOk()) {
                ALOGE("failed to invoke fingerprint onRemoved callback");
            }
            break;
        case FINGERPRINT_AUTHENTICATED:
            ALOGD("onAuthenticated(fid=%d, gid=%d)",
                    msg->data.authenticated.finger.fid,
                    msg->data.authenticated.finger.gid);
            if (msg->data.authenticated.finger.fid != 0) {
                const uint8_t* hat =
                    reinterpret_cast<const uint8_t *>(&msg->data.authenticated.hat);
                const hidl_vec<uint8_t> token(
                    std::vector<uint8_t>(hat, hat + sizeof(msg->data.authenticated.hat)));
                if (!thisPtr->mClientCallback->onAuthenticated(devId,
                        msg->data.authenticated.finger.fid,
                        msg->data.authenticated.finger.gid,
                        token).isOk()) {
                    ALOGE("failed to invoke fingerprint onAuthenticated callback");
                }
            } else {
                // Not a recognized fingerprint
                if (!thisPtr->mClientCallback->onAuthenticated(devId,
                        msg->data.authenticated.finger.fid,
                        msg->data.authenticated.finger.gid,
                        hidl_vec<uint8_t>()).isOk()) {
                    ALOGE("failed to invoke fingerprint onAuthenticated callback");
                }
            }
            break;
        case FINGERPRINT_TOUCH_DOWN:
            ALOGD("onTouchDown()");
            if (thisPtr->mClientCallbackEx == NULL) {
                ALOGE("Callback not yet registered");
            } else if ( !thisPtr->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_DOWN,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
                ALOGE("failed to invoke FINGERPRINT_TOUCH_DOWN callback");
            }
            break;
        case FINGERPRINT_TOUCH_UP:
            ALOGD("onTouchUp()");
            if (thisPtr->mClientCallbackEx == NULL) {
                ALOGE("Callback not yet registered");
            } else if (!thisPtr->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_UP,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
                ALOGE("failed to invoke FINGERPRINT_TOUCH_UP callback");
            }
            break;
        case ENGINEERING_INFO:
            {
            uint32_t len = 0;
            uint32_t *key = NULL;
            hidl_string *value = NULL;
            switch (msg->data.engineering.type) {
                case FINGERPRINT_IMAGE_QUALITY:
                    len = 3;
                    key = (uint32_t*)malloc(len * sizeof(uint32_t));
                    value = new hidl_string[len];

                    key[0]   = SUCCESSED;
                    value[0] = getHidlstring(msg->data.engineering.quality.successed);
                    key[1]   = IMAGE_QUALITY;
                    value[1] = getHidlstring(msg->data.engineering.quality.image_quality);
                    key[2]   = QUALITY_PASS;
                    value[2] = getHidlstring(msg->data.engineering.quality.quality_pass);
                    break;
                case FINGERPRINT_INAGE_SELF_TEST:
                    ALOGE(" callback ENGINEERING_INFO FINGERPRINT_INAGE_SELF_TEST result=%d", msg->data.engineering.self_test_result);
                    len = 1;
                    key = (uint32_t*)malloc(len * sizeof(uint32_t));
                    value = new hidl_string[len];
                    if (key == NULL || value == NULL) {
                        ALOGE("malloc fail");
                        break;
                    }
                    key[0]   = SUCCESSED;
                    value[0] = getHidlstring(msg->data.engineering.self_test_result);
                    break;
                default:
                    break;
            }

            if (key == NULL || value == NULL) {
                break;
            }

            const std::vector<uint32_t> hidl_key(key, key + len);
            const std::vector<hidl_string> hidl_value(value, value+len);
            int count = hidl_value.size();
            for (int i=0; i < count; i++) {
                ALOGD("onEngineeringInfoUpdated, key[%d] = %d, value[%d] = %s", i, hidl_key[i], i, (android::String8(hidl_value[i].c_str())).string());
            }
            if ( thisPtr->mClientCallbackEx == NULL ) {
                ALOGE("Callback not yet registered");
            } else if ( !thisPtr->mClientCallbackEx->onEngineeringInfoUpdated(len, hidl_vec<uint32_t>(hidl_key),
                    hidl_vec<hidl_string>(hidl_value)).isOk()) {
                ALOGE("failed to invoke onEngineeringInfoUpdated callback");
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
            break;
        case FINGERPRINT_TEMPLATE_ENUMERATING:
            {
            int32_t rem;
            int32_t *fids;
            rem = msg->data.enumerated.remaining_templates;
            fids = (int32_t*)malloc(rem * sizeof(int32_t));
            if (fids == NULL) {
                ALOGE("malloc fail");
                break;
            }
            ALOGD("onEnumerate()");
            if (rem == 0) {
                ALOGD("fingerprint template is null");
                if (!thisPtr->mClientCallback->onEnumerate(devId,
                        0,
                        msg->data.enumerated.gid,
                        0).isOk()) {
                    ALOGE("failed to invoke fingerprint onEnumerate callback");
                }
            }

            for (int32_t index = 0; index < rem; index++) {
                fids[index] = msg->data.enumerated.fingers[index].fid;
                ALOGE("fids[%d] = %d, gids[%d] = %d",
                index, fids[index], index, msg->data.enumerated.fingers[index].gid);
                if (!thisPtr->mClientCallback->onEnumerate(devId,
                        fids[index],
                        msg->data.enumerated.fingers[index].gid,
                        rem - index - 1).isOk()) {
                    ALOGE("failed to invoke fingerprint onEnumerate callback");
                }
            }
            if (fids) {
                free(fids);
                fids = NULL;
            }
            }
            break;
        case FINGERPRINT_IMAGE_INFO:
            {
            ALOGD("onImageInfoAcquired(type=%d, quality=%d, match_score=%d)",
                    msg->data.image_info.type,
                    msg->data.image_info.quality,
                    msg->data.image_info.match_score);
            }
            break;
        case FINGERPRINT_OPTICAL_SENDCMD:
                {
                    ALOGD("onFingerprintCmd type==");

                    const std::vector<int8_t> test_result(msg->data.test.result, msg->data.test.result + msg->data.test.result_len);

                    if ( thisPtr->mClientCallbackEx == NULL ) {
                        ALOGE("CallbackEx not yet registered");
                    } else if (!thisPtr->mClientCallbackEx->onFingerprintCmd(msg->data.test.cmd_id,
                            hidl_vec<int8_t>(test_result),
                            msg->data.test.result_len).isOk()) {
                        ALOGE(" FINGERPRINT_OPTICAL_SENDCMD ");
                    }
            #if 0
            thisPtr->mClientCallbackEx->onImageInfoAcquired(
                    msg->data.image_info.type,
                    msg->data.image_info.quality,
                    msg->data.image_info.match_score);
            #endif
            }
            break;
        case FINGERPRINT_MONITOR: {
                char data[64];
                size_t size = 0;
                switch (msg->data.monitor.type) {
                    case fingerprint_monitor_type_t::FINGER_POWER_MONITOR:
                        ALOGD("onMonitorEventTriggered(type=%d, battery=%f)",
                                msg->data.monitor.type,
                                msg->data.monitor.data.power.battery);

                        size = snprintf(data, 63, "%f", msg->data.monitor.data.power.battery);
                        break;
                    case fingerprint_monitor_type_t::FINGER_ERROR_STATE:
                        //ignore, no available yet.
                        break;
                    case fingerprint_monitor_type_t::FINGER_TP_PROTECT_MONITOR:
                        ALOGD("onMonitorEventTriggered(type=%d, mode=%d)",
                                msg->data.monitor.type,
                                msg->data.monitor.data.tp_protect.mode);

                        size = snprintf(data, 63, "%d", msg->data.monitor.data.tp_protect.mode);
                        break;
                    default:
                        ;//ignore
                    }
                if (size > 0) {
                    #if 0
                    thisPtr->mClientCallbackEx->onMonitorEventTriggered(
                            msg->data.monitor.type,
                            hidl_string(data));
                    #endif
#ifdef FP_DCS_ENABLE
                    Dcs::getInstance()->reportMonitor((uint32_t)(msg->data.monitor.data.power.battery));
#endif /* FP_DCS_ENABLE */
                }
            }
            break;

        case FINGERPRINT_AUTHENTICATED_DCSSTATUS:
                {
#ifdef FP_DCS_ENABLE
                    Dcs::getInstance()->reportAuthenticatedInfo(msg->data.auth_dcsmsg);
#endif /* FP_DCS_ENABLE */
                }
            break;

#ifdef FP_HYPNUSD_ENABLE
        case FINGERPRINT_HYPNUSDSETACION:
        {
            int32_t action_type = msg->data.hypnusd_setting.action_type;
            int32_t action_timeout = msg->data.hypnusd_setting.action_timeout;

            ALOGD("hypnusdSetAction(type=%d, timeout=%d)", action_type, action_timeout);
            Hypnus::getInstance()->setAction(action_type, action_timeout);
        }
            break;
#endif

        case FINGERPRINT_BINDCORE:
        {
            int32_t tid = msg->data.bindcore_setting.tid;
            Hypnus::getInstance()->bind_big_core_bytid(tid);
        }
        break;

        default:
            ALOGE("unkonown msg->type= %d", msg->type);
            break;
        }
    }//notify end

    Return<bool> BiometricsFingerprint::isUdfps(uint32_t sensorId) {
        ALOGD("isUdfps sensor_id = %d", sensorId);
        if (sensorId >= E_SENSOR_ID_MAX) {
            ALOGE("unknown sensor");
            return false;
        }
        return (fp_config_info[sensorId].fp_type) == 4? true:false;
    }

    Return<void> BiometricsFingerprint::onFingerDown(uint32_t x, uint32_t y, float minor, float major) {
        ALOGD("onFingerDown");
        //mDevice->touchDown(mDevice);
        return Void();
    }

    Return<void> BiometricsFingerprint::onFingerUp() {
        ALOGD("onFingerUp");
        //mDevice->touchUp(mDevice);
        return Void();
    }
    Return<int32_t> BiometricsFingerprint::sendFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& in_buf) {
        int32_t ret = FINGERPRINT_RIGHTNESS_ERROR;
        ALOGD("sendFingerprintCmd = %d ", cmdId);
        switch (cmdId) {
            case FINGERPRINT_CMD_ID_CAMERA: //CMD_FINGERPRINT_CAMERA
                ALOGD("keymode_enable(enable = %d)", in_buf[0]);
                ret = gf_keymode_enable(&mDevice, static_cast<int32_t>(in_buf[0]));
                break;
            case FINGERPRINT_PRODUCT_TEST_CMD_GET_OTP_QRCODE:
                ret = gf_hal_notify_qrcode(cmdId);
                break;
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE:
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
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_PAUSE_ENROLL:
            {
                pauseEnroll();
                return 0;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_CONTINUE_ENROLL:
            {
                continueEnroll();
                return 0;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SET_TOUCHEVENT_LISTENER:
            {
                setTouchEventListener();
                return 0;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_DYNAMICALLY_LOG:
            {
                uint32_t on = 0;
                int8_t *value = (int8_t*)in_buf.data();
                memcpy(&on, value, sizeof(uint32_t));
                ALOGE("FINGERPRINT_CMD_ID_DYNAMICALLY_LOG on = %d", on);
                dynamicallyConfigLog(on);
                return 0;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_GET_ENROLL_TIMES:
            {
                int32_t enroll_times = 0;
                enroll_times = getEnrollmentTotalTimes();
                return enroll_times;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SET_SCREEN_STATE:
            {
                int32_t screen_state;
                int8_t *value = (int8_t*)in_buf.data();
                memcpy(&screen_state, value, sizeof(int32_t));
                ALOGE("FINGERPRINT_CMD_ID_SET_SCREEN_STATE screen_state = %d", screen_state);
                setScreenState(screen_state);
                return 0;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO:
            {
                uint32_t type = 0;
                int8_t *value = (int8_t*)in_buf.data();
                memcpy(&type, value, sizeof(uint32_t));
                ALOGE("FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO type = %d", type);
                return getEngineeringInfo(type);
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_GET_SENSOR_ID:
            {
                ALOGE("FINGERPRINT_CMD_ID_GET_SENSOR_ID = %d", fp_config_info_init.sensor_id);
                return fp_config_info_init.sensor_id;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIDE_PRESS_ENABLE:
            {
                return 0;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIDE_SCREEN_STATE:
            {
                return 0;
            }
            case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIDE_POWER_KEY_PRESSED:
            {
                return 0;
            }
            default:
                break;
        }
        //TODO
        return 0;
    }
} // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace oplus
}  // namespace vendor
