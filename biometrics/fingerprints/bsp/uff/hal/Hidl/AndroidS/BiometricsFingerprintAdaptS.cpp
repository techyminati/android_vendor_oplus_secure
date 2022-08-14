/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/BiometricsFingerprint.cpp
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 **
 ** --------------------------- Revision History -------------------------------
 **  <author>      <data>            <desc>
 **  zoulian   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][BiometricsFingerprintAdaptS]"

#include "BiometricsFingerprintAdaptS.h"
#include <fingerprint.h>
#include <hardware/hardware.h>
#include <hardware/hw_auth_token.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include "FingerprintManager.h"
#include "HalLog.h"
#include "Perf.h"
#include "ActionType.h"
#include "FpCommon.h"
#include "FpType.h"

#ifdef FP_DCS_ENABLE
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"
#endif

using android::HealthMonitor;
using android::Perf;

namespace vendor {
namespace oplus {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {
namespace implementation {

using RequestStatus = android::hardware::biometrics::fingerprint::V2_1::RequestStatus;

BiometricsFingerprint* BiometricsFingerprint::sInstance = nullptr;

BiometricsFingerprint::BiometricsFingerprint() : mClientCallback(nullptr) {
    sInstance = this;  // keep track of the most recent instance
}

BiometricsFingerprint::~BiometricsFingerprint() {
    //TBD
}

Return<RequestStatus> BiometricsFingerprint::ErrorFilter(int32_t error) {
    switch (error) {
        case 0:
            return RequestStatus::SYS_OK;
        case -2:
            return RequestStatus::SYS_ENOENT;
        case -4:
            return RequestStatus::SYS_EINTR;
        case -5:
            return RequestStatus::SYS_EIO;
        case -11:
            return RequestStatus::SYS_EAGAIN;
        case -12:
            return RequestStatus::SYS_ENOMEM;
        case -13:
            return RequestStatus::SYS_EACCES;
        case -14:
            return RequestStatus::SYS_EFAULT;
        case -16:
            return RequestStatus::SYS_EBUSY;
        case -22:
            return RequestStatus::SYS_EINVAL;
        case -28:
            return RequestStatus::SYS_ENOSPC;
        case -110:
            return RequestStatus::SYS_ETIMEDOUT;
        default:
            LOG_E(LOG_TAG, "An unknown error returned from fingerprint vendor library: %d", error);
            return RequestStatus::SYS_UNKNOWN;
    }
}

// Translate from errors returned by traditional HAL (see fingerprint.h) to
// HIDL-compliant FingerprintError.
FingerprintError BiometricsFingerprint::vendorErrorFilter(int32_t error, int32_t* vendorCode) {
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
    LOG_E(LOG_TAG, "Unknown error from fingerprint vendor library: %d", error);
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
            return (FingerprintAcquiredInfo)1002;
        case FINGERPRINT_ACQUIRED_TOO_SIMILAR:
            return (FingerprintAcquiredInfo)1001;
        default:
            if (info >= FINGERPRINT_ACQUIRED_VENDOR_BASE) {
                // vendor specific code.
                *vendorCode = info - FINGERPRINT_ACQUIRED_VENDOR_BASE;
                return FingerprintAcquiredInfo::ACQUIRED_VENDOR;
            }
    }
    LOG_E(LOG_TAG, "Unknown acquiredmsg from fingerprint vendor library: %d", info);

    return FingerprintAcquiredInfo::ACQUIRED_INSUFFICIENT;
}

hidl_string BiometricsFingerprint::getHidlstring(uint32_t param) {
    char data[64];
    snprintf(data, 63, "%u", param);
    LOG_D(LOG_TAG, "getString16(param=%u)", param);
    return hidl_string(data, strlen(data));
}

IBiometricsFingerprint* BiometricsFingerprint::getInstance() {
    if (!sInstance) {
        sInstance = new BiometricsFingerprint();
    }
    return sInstance;
}

Return<int32_t> BiometricsFingerprint::init() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    fp_return_type_t err = FP_SUCCESS;

    mHalContext = new HalContext();
    CHECK_RESULT_NULLPTR(mHalContext);

    err = mHalContext->init();
    if (err != FP_SUCCESS) {
        delete mHalContext;
        mHalContext = nullptr;
    }

    err = mHalContext->mFingerprintManager->setNotify(BiometricsFingerprint::notify);
    CHECK_RESULT_SUCCESS(err);

fp_out:
    return err;
}

Return<uint64_t> BiometricsFingerprint::preEnroll() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    uint64_t challenge = 0;
    challenge = mHalContext->mFingerprintManager->preEnroll();
    return challenge;
}

Return<RequestStatus> BiometricsFingerprint::enroll(
    const hidl_array<uint8_t, 69>& hat, uint32_t gid, uint32_t timeoutSec) {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    const hw_auth_token_t* authToken = reinterpret_cast<const hw_auth_token_t*>(hat.data());
    action_message_t       action;
    memset(&action, 0, sizeof(action_message_t));
    action.type                   = FP_ENROLL_ACTION;
    action.data.enroll.authToken  = (hw_auth_token_t*)authToken;
    action.data.enroll.gid        = gid;
    action.data.enroll.timeoutSec = timeoutSec;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::postEnroll() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return ErrorFilter(mHalContext->mFingerprintManager->postEnroll());
}


Return<uint64_t> BiometricsFingerprint::setNotify(
    const sp<IBiometricsFingerprintClientCallback>& clientCallback) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    mClientCallback = clientCallback;
    // This is here because HAL 2.1 doesn't have a way to propagate a
    // unique token for its driver. Subsequent versions should send a unique
    // token for each call to setNotify(). This is fine as long as there's only
    // one fingerprint device on the platform.
    BiometricsFingerprint* thisPtr =
        static_cast<BiometricsFingerprint*>(BiometricsFingerprint::getInstance());
    return reinterpret_cast<uint64_t>(thisPtr);
}

Return<uint64_t> BiometricsFingerprint::setHalCallback(
        const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    mClientCallbackEx = clientCallbackEx;
    BiometricsFingerprint* thisPtr =
        static_cast<BiometricsFingerprint*>(BiometricsFingerprint::getInstance());
    return reinterpret_cast<uint64_t>(thisPtr);
}

Return<uint64_t> BiometricsFingerprint::getAuthenticatorId() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    uint64_t authenticatorid = 0;
    authenticatorid = mHalContext->mFingerprintManager->getAuthenticatorId();
    return authenticatorid;
}

Return<RequestStatus> BiometricsFingerprint::cancel() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    action_message_t action;
    memset(&action, 0, sizeof(action_message_t));
    action.type = FP_CANCEL;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::enumerate() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    action_message_t action;
    memset(&action, 0, sizeof(action_message_t));
    action.type = FP_ENUMERATE;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::remove(uint32_t gid, uint32_t fid) {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    action_message_t action;
    memset(&action, 0, sizeof(action_message_t));
    action.type            = FP_REMOVE;
    action.data.remove.gid = gid;
    action.data.remove.fid = fid;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return RequestStatus::SYS_OK;
}

// sendMessageToWorkThread???
Return<RequestStatus> BiometricsFingerprint::setActiveGroup(
    uint32_t gid, const hidl_string& storePath) {
    LOG_I(LOG_TAG, "enter to [%s] gid=%d, storePath =%s", __func__, gid, storePath.c_str());
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        LOG_E(LOG_TAG, "Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        LOG_E(LOG_TAG, "storePath can not access");
        return RequestStatus::SYS_EINVAL;
    }
    return ErrorFilter(mHalContext->mFingerprintManager->setActiveGroup(gid, storePath.c_str()));
}

Return<RequestStatus> BiometricsFingerprint::authenticate(uint64_t operationId, uint32_t gid) {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return authenticateAsType(operationId, gid, 1);
}

//add by vendor
Return<RequestStatus> BiometricsFingerprint::authenticateAsType(
    uint64_t operationId, uint32_t gid, int32_t authtype) {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    action_message_t action;
    memset(&action, 0, sizeof(action_message_t));
    action.type                          = FP_AUTH_ACTION;
    action.data.authenticate.operationId = operationId;
    action.data.authenticate.gid         = gid;
    action.data.authenticate.authtype    = authtype;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return RequestStatus::SYS_OK;
}

Return<int32_t> BiometricsFingerprint::getEnrollmentTotalTimes() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    uint32_t total_enroll_times = 20;//TBD: modify for debug
    mHalContext->mFingerprintManager->getTotalEnrollTimes(&total_enroll_times);
    return total_enroll_times;
}

Return<RequestStatus> BiometricsFingerprint::setScreenState(int32_t screen_state) {
    LOG_I(LOG_TAG, "enter to [%s], screen_state =%d", __func__, screen_state);
    action_message_t action;
    memset(&action, 0, sizeof(action_message_t));
    action.type                   = FP_SET_SCREEN_STATE;
    action.data.screenstate.state = (uint32_t)screen_state;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return RequestStatus::SYS_OK;
}

Return<int32_t> BiometricsFingerprint::getEngineeringInfo(uint32_t type) {
    LOG_I(LOG_TAG, "enter to [%s] type =%d", __func__, type);
    UNUSED(type);
    //TBD:sendFingerprintCmd
    return 0;
}

Return<int32_t> BiometricsFingerprint::sendFingerprintCmd(
    int32_t cmdId, const hidl_vec<int8_t>& in_buf) {
    LOG_I(LOG_TAG, "enter to [%s], cmdId =%d", __func__, cmdId);
    int32_t ret = 0;
    switch (cmdId) {
        case FINGERPRINT_CMD_ID_GET_ENROLL_TIMES : {
            ret = getEnrollmentTotalTimes();
        } break;
        default : {
            action_message_t action;
            memset(&action, 0, sizeof(action_message_t));
            action.type                        = FP_SENDFPCMD;
            action.data.sendfpcmd.cmdid        = cmdId;
            action.data.sendfpcmd.in_buff_data = (int8_t*)in_buf.data();
            action.data.sendfpcmd.in_buff_size = in_buf.size();
            ret = mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
        } break;
    }
    return ret;
}

Return<bool> BiometricsFingerprint::isUdfps(uint32_t sensorId) {
    ALOGD("isUdfps sensor_id = %d", sensorId);
    return true;
}

Return<void> BiometricsFingerprint::onFingerDown(uint32_t x, uint32_t y, float minor, float major) {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    UNUSE(x);
    UNUSE(y);
    UNUSE(minor);
    UNUSE(major);
    action_message_t action;
    memset(&action, 0, sizeof(action_message_t));
    action.type = FP_TOUCHDOWN;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return Void();
}

Return<void> BiometricsFingerprint::onFingerUp() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    action_message_t action;
    memset(&action, 0, sizeof(action_message_t));
    action.type = FP_TOUCHUP;
    mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    return Void();
}

// del for androidR
Return<RequestStatus> BiometricsFingerprint::pauseEnroll() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::continueEnroll() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::pauseIdentify() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::continueIdentify() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::getAlikeyStatus() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::cleanUp() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::setTouchEventListener() {
    LOG_I(LOG_TAG, "enter to [%s]", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> BiometricsFingerprint::dynamicallyConfigLog(uint32_t on) {
    LOG_I(LOG_TAG, "enter to [%s], on =%d", __func__, on);
    UNUSED(on);
    return RequestStatus::SYS_OK;
}

void BiometricsFingerprint::notify(const fingerprint_msg_t* msg) {
    BiometricsFingerprint* thisPtr =
        static_cast<BiometricsFingerprint*>(BiometricsFingerprint::getInstance());
    std::lock_guard<std::mutex> lock(thisPtr->mClientCallbackMutex);
    if (thisPtr == nullptr || thisPtr->mClientCallback == nullptr) {
        //LOG_E(LOG_TAG, "Receiving callbacks before the client callback is registered.");
        return;
    }
    const uint64_t devId = reinterpret_cast<uint64_t>(thisPtr);  //need modify ???
    const std::vector<int8_t> devIdExt(&devId, &devId + sizeof(uint64_t));
    switch (msg->type) {
        case FINGERPRINT_ERROR: {
            int32_t          vendorCode = 0;
            FingerprintError result     = vendorErrorFilter(msg->data.error, &vendorCode);
            LOG_D(LOG_TAG, "onError(%d)", result);
            if (!thisPtr->mClientCallback->onError(devId, result, vendorCode).isOk()) {
                LOG_E(LOG_TAG, "failed to invoke fingerprint onError callback");
            }
        } break;
        case FINGERPRINT_ACQUIRED: {
            int32_t                 vendorCode = 0;
            FingerprintAcquiredInfo result =
                vendorAcquiredFilter(msg->data.acquired.acquired_info, &vendorCode);
            LOG_D(LOG_TAG, "onAcquired(%d), vendorCode(%d)", result, vendorCode);
            if (!thisPtr->mClientCallback->onAcquired(devId, result, vendorCode).isOk()) {
                LOG_E(LOG_TAG, "failed to invoke fingerprint onAcquired callback");
            }
        } break;
        case FINGERPRINT_TEMPLATE_ENROLLING:
            LOG_D(LOG_TAG, "onEnrollResult(fid=%d, gid=%d, rem=%d)", msg->data.enroll.finger.fid,
                msg->data.enroll.finger.gid, msg->data.enroll.samples_remaining);
            if (!thisPtr->mClientCallback
                    ->onEnrollResult(devId, msg->data.enroll.finger.fid,
                    msg->data.enroll.finger.gid, msg->data.enroll.samples_remaining).isOk()) {
                LOG_E(LOG_TAG, "failed to invoke fingerprint onEnrollResult callback");
            }
            break;
        case FINGERPRINT_TEMPLATE_REMOVED:
            LOG_D(LOG_TAG, "onRemove(fid=%d, gid=%d, rem=%d)", msg->data.removed.finger.fid,
                msg->data.removed.finger.gid, msg->data.removed.remaining_templates);
            if (!thisPtr->mClientCallback
                    ->onRemoved(devId, msg->data.removed.finger.fid, msg->data.removed.finger.gid,
                    msg->data.removed.remaining_templates).isOk()) {
                LOG_E(LOG_TAG, "failed to invoke fingerprint onRemoved callback");
            }
            break;
        case FINGERPRINT_AUTHENTICATED:
            LOG_D(LOG_TAG, "onAuthenticated(fid=%d, gid=%d)", msg->data.authenticated.finger.fid,
                msg->data.authenticated.finger.gid);
            if (msg->data.authenticated.finger.fid != 0) {
                const uint8_t* hat = reinterpret_cast<const uint8_t*>(&msg->data.authenticated.hat);
                const hidl_vec<uint8_t> token(
                    std::vector<uint8_t>(hat, hat + sizeof(msg->data.authenticated.hat)));
                if (!thisPtr->mClientCallback
                        ->onAuthenticated(devId, msg->data.authenticated.finger.fid,
                        msg->data.authenticated.finger.gid, token).isOk()) {
                    LOG_E(LOG_TAG,
                        "failed to invoke fingerprint onAuthenticated "
                        "callback");
                }
            } else {
                // Not a recognized fingerprint
                if (!thisPtr->mClientCallback
                        ->onAuthenticated(devId, msg->data.authenticated.finger.fid,
                        msg->data.authenticated.finger.gid, hidl_vec<uint8_t>()).isOk()) {
                    LOG_E(LOG_TAG,
                        "failed to invoke fingerprint onAuthenticated "
                        "callback");
                }
            }
            break;
        case FINGERPRINT_TOUCH_DOWN:
            LOG_D(LOG_TAG, "onTouchDown()");
            if (!thisPtr->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_DOWN,
                    hidl_vec<int8_t>(devIdExt), sizeof(uint64_t)).isOk()) {
                LOG_E(LOG_TAG, "failed to invoke FINGERPRINT_TOUCH_DOWN callback");
            }
            break;
        case FINGERPRINT_TOUCH_UP:
            LOG_D(LOG_TAG, "onTouchUp()");
            if (!thisPtr->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_UP,
                    hidl_vec<int8_t>(devIdExt), sizeof(uint64_t)).isOk()) {
                LOG_E(LOG_TAG, "failed to invoke FINGERPRINT_TOUCH_UP callback");
            }
            break;
        case ENGINEERING_INFO: {
            uint32_t     len   = 0;
            uint32_t*    key   = NULL;
            hidl_string* value = NULL;
            switch (msg->data.engineering.type) {
                case FINGERPRINT_IMAGE_QUALITY:
                    len   = 3;
                    key   = (uint32_t*)malloc(len * sizeof(uint32_t));
                    value = new hidl_string[len];

                    key[0]   = SUCCESSED;
                    value[0] = getHidlstring(msg->data.engineering.quality.successed);
                    key[1]   = IMAGE_QUALITY;
                    value[1] = getHidlstring(msg->data.engineering.quality.image_quality);
                    key[2]   = QUALITY_PASS;
                    value[2] = getHidlstring(msg->data.engineering.quality.quality_pass);
                    break;
                default:
                    break;
            }

            if (key == NULL || value == NULL) {
                break;
            }

            const std::vector<uint32_t>    hidl_key(key, key + len);
            const std::vector<hidl_string> hidl_value(value, value + len);
            int                            count = hidl_value.size();
            for (int i = 0; i < count; i++) {
                LOG_D(LOG_TAG, "onEngineeringInfoUpdated, key[%d] = %d, value[%d] = %s", i,
                    hidl_key[i], i, (android::String8(hidl_value[i].c_str())).string());
            }
            if (!thisPtr->mClientCallbackEx->onEngineeringInfoUpdated(len,
                    hidl_vec<uint32_t>(hidl_key), hidl_vec<hidl_string>(hidl_value)).isOk()) {
                LOG_E(LOG_TAG, "failed to invoke onEngineeringInfoUpdated callback");
            }
            if (key) {
                free(key);
                key = NULL;
            }
            if (value) {
                delete[] value;
                value = NULL;
            }
        } break;

        case FINGERPRINT_TEMPLATE_ENUMERATING: {
            int32_t  rem;
            int32_t* fids;
            rem  = msg->data.enumerated.remaining_templates;
            fids = (int32_t*)malloc(rem * sizeof(int32_t));
            if (fids == NULL) {
                LOG_D(LOG_TAG, "malloc fail");
                break;
            }
            LOG_D(LOG_TAG, "onEnumerate()");
            if (rem == 0) {
                ALOGD("fingerprint template is null");
                if (!thisPtr->mClientCallback->onEnumerate(devId, 0, msg->data.enumerated.gid, 0).isOk()) {
                    LOG_D(LOG_TAG, "failed to invoke fingerprint onEnumerate callback");
                }
            }
            for (int32_t index = 0; index < rem; index++) {
                fids[index] = msg->data.enumerated.fingers[index].fid;
                LOG_E(LOG_TAG, "fids[%d] = %d, gids[%d] = %d", index, fids[index], index,
                    msg->data.enumerated.fingers[index].gid);
                if (!thisPtr->mClientCallback->onEnumerate(devId, fids[index],
                        msg->data.enumerated.fingers[index].gid, rem - index - 1).isOk()) {
                    ALOGE("failed to invoke fingerprint onEnumerate callback");
                }
            }
            if (fids) {
                free(fids);
                fids = NULL;
            }
        } break;

        case FINGERPRINT_OPTICAL_SENDCMD: {
            LOG_D(LOG_TAG, "onFINGERPRINT_OPTICAL_SENDCMD");
            const std::vector<int8_t> test_result(
                msg->data.test.result, msg->data.test.result + msg->data.test.result_len);

            if (!thisPtr->mClientCallbackEx->onFingerprintCmd(msg->data.test.cmd_id, hidl_vec<int8_t>(test_result),
                    msg->data.test.result_len).isOk()) {
                LOG_E(LOG_TAG, " FINGERPRINT_OPTICAL_SENDCMD ");
            }
        } break;

        default:
            LOG_E(LOG_TAG, "unkonown msg->type= %d", msg->type);
            break;
    }
}  // notify end

}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace oplus
}  // namespace vendor
