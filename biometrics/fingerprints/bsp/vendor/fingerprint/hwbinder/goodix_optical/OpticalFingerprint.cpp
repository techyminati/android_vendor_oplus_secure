/************************************************************************************
 ** File: - goodix\fingerprint\fingerprint_hwbinder\OpticalFingerprint.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
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
 **  Ran.Chen         2018/06/06       add IMAGET_QUALITY for goodix
 **  Ran.Chen         2018/06/07       remove ERROR_VENDOR
 **  Ziqing.Guo       2018/09/13       add for distingue fingerprint unlock and pay
 **  Ran.Chen         2018/12/28       add for quality_pass info
 **  Ran.Chen         2019/01/12       add for commondcs msg
 **  Ran.Chen         2019/01/12       add for commondcs msg (signal_value / algo_version)
 **  Ran.Chen         2019/03/03       add for commondcs msg (fingerprint_MonitorEvent)
 **  Dongnan.Wu       2019/03/28       add for hypnus interface
 **  oujinrong        2019/06/18       add macro for DCS
 **  Ziqing.Guo       2019/08/16       modify for goodix optical android Q (Euclid)
 **  Ziqing.Guo       2019/08/21       fix some warnings
 **  Ziqing.Guo       2019/08/21       move hypnus to fingerprint common module
 **  Ziqing.Guo       2019/08/29       move dcs to fingerprint common module
 **  Ziqing.Guo       2019/08/29       import healthmonitor
 **  Tiexin.Kou       2019/11/06       modified for G5 EngineeringMode SelfTest
 **  Ran.Chen         2019/11/07       add for bingcore by tid
 **  Bangxiong.Wu     2020/02/24       no need to require lock_guard when handle hypnusd msg
 **  Fangjie.Lei      2020/08/26       setActiveGroup add path check
 **  Qijia.Zhou         2021/04/27       modify hidl interface for Android S
 **  mingzhi.Guo      2021/09/20        add and modify to support heartrate detect
 ************************************************************************************/

#define LOG_VERBOSE "vendor.oplus.hardware.biometrics.fingerprint@2.1-service_optical"

#include <hardware/hw_auth_token.h>

#include <hardware/hardware.h>
#include "fingerprint.h"
#include "OpticalFingerprint.h"
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <utils/Timers.h>
#include <string.h>

#include "fingerprint_type.h"

#ifdef FP_DCS_ENABLE
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"
#endif /* FP_DCS_ENABLE */
#ifdef FP_CONFIG_SETTINGS_ENABLE
#include "FingerprintSettings.h"
#endif

#define CMD_TEST_SZ_FT_SPI_RST_INT (0X620)//1568 (0x600+32) GOODIXG3 CMD_ID
#define CMD_TEST_SZ_FT_SPI (0X621)//1569 (0x600+33)
#define CMD_TEST_SZ_FT_KPI (0X626)//1574 (0X600+38)//G3 IMAGE_QUALITY

#define PRODUCT_TEST_CMD_SPI    8  // GOODIXG5 CMD_ID
#define PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN 9
#define PRODUCT_TEST_CMD_OTP_FLASH 10
#define PRODUCT_TEST_CMD_IMAGE_QUALITY 22

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

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


// Supported fingerprint HAL version
static const uint16_t kVersion = HARDWARE_MODULE_API_VERSION(2, 1);

using RequestStatus =
        android::hardware::biometrics::fingerprint::V2_1::RequestStatus;

OpticalFingerprint *OpticalFingerprint::sInstance = nullptr;
OpticalFingerprint::OpticalFingerprint() : mClientCallback(nullptr), mClientCallbackEx(nullptr), mDevice(nullptr) {
    sInstance = this; // keep track of the most recent instance
    mDevice = openHal();
    if (!mDevice) {
        ALOGE("Can't open HAL module");
    }
}

OpticalFingerprint::~OpticalFingerprint() {
    ALOGV("~OpticalFingerprint()");
    if (mDevice == nullptr) {
        ALOGE("No valid device");
        return;
    }
    int err;
    if (0 != (err = mDevice->common.close(
            reinterpret_cast<hw_device_t*>(mDevice)))) {
        ALOGE("Can't close fingerprint module, error: %d", err);
        return;
    }
    mDevice = nullptr;
}

Return<RequestStatus> OpticalFingerprint::ErrorFilter(int32_t error) {
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
FingerprintError OpticalFingerprint::vendorErrorFilter(int32_t error,
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
FingerprintAcquiredInfo OpticalFingerprint::vendorAcquiredFilter(
        int32_t info, int32_t* vendorCode) {
    *vendorCode = 0;
    ALOGE("vendorAcquiredFilter: %d", info);
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

hidl_string OpticalFingerprint::getHidlstring(uint32_t param) {
        char data[64];
        snprintf(data, 63, "%u", param);
        ALOGD("getString16(param=%u)", param);
        return hidl_string(data, strlen(data));
}

Return<uint64_t> OpticalFingerprint::setNotify(
        const sp<IBiometricsFingerprintClientCallback>& clientCallback) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    mClientCallback = clientCallback;
    // This is here because HAL 2.1 doesn't have a way to propagate a
    // unique token for its driver. Subsequent versions should send a unique
    // token for each call to setNotify(). This is fine as long as there's only
    // one fingerprint device on the platform.
    return reinterpret_cast<uint64_t>(mDevice);
}

Return<uint64_t> OpticalFingerprint::setHalCallback(
        const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    mClientCallbackEx = clientCallbackEx;
    return reinterpret_cast<uint64_t>(mDevice);
}

Return<uint64_t> OpticalFingerprint::preEnroll()  {
    return mDevice->pre_enroll(mDevice);
}

Return<RequestStatus> OpticalFingerprint::enroll(const hidl_array<uint8_t, 69>& hat,
        uint32_t gid, uint32_t timeoutSec) {
    ALOGD("OpticalFingerprint enroll");
    const hw_auth_token_t* authToken =
        reinterpret_cast<const hw_auth_token_t*>(hat.data());
    return ErrorFilter(mDevice->enroll(mDevice, authToken, gid, timeoutSec));
}

Return<RequestStatus> OpticalFingerprint::postEnroll() {
    return ErrorFilter(mDevice->post_enroll(mDevice));
}

Return<uint64_t> OpticalFingerprint::getAuthenticatorId() {
    return mDevice->get_authenticator_id(mDevice);
}

Return<RequestStatus> OpticalFingerprint::cancel() {
    ALOGD("OpticalFingerprint cancel");
    return ErrorFilter(mDevice->cancel(mDevice));
}

Return<RequestStatus> OpticalFingerprint::enumerate()  {
    ALOGD("OpticalFingerprint enumerate");
    return ErrorFilter(mDevice->enumerate(mDevice));//we need it
    //return RequestStatus::SYS_OK;
}

Return<RequestStatus> OpticalFingerprint::remove(uint32_t gid, uint32_t fid) {
    return ErrorFilter(mDevice->remove(mDevice, gid, fid));
}

Return<RequestStatus> OpticalFingerprint::setActiveGroup(uint32_t gid,
        const hidl_string& storePath) {
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        ALOGE("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        return RequestStatus::SYS_EINVAL;
    }

    return ErrorFilter(mDevice->set_active_group(mDevice, gid, storePath.c_str()));
}

Return<RequestStatus> OpticalFingerprint::authenticate(uint64_t operationId,
        uint32_t gid) {
    ALOGD("OpticalFingerprint authenticate");
    return ErrorFilter(mDevice->authenticate(mDevice, operationId, gid));
}

Return<bool> OpticalFingerprint::isUdfps(uint32_t sensorId) {
    ALOGD("isUdfps sensor_id = %d", sensorId);
    if (sensorId >= E_SENSOR_ID_MAX) {
        ALOGE("unknown sensor");
        return false;
    }
    return (fp_config_info[sensorId].fp_type) == 4? true:false;
}

Return<void> OpticalFingerprint::onFingerDown(uint32_t x, uint32_t y, float minor, float major) {
    ALOGD("onFingerDown");
    mDevice->touchDown(mDevice);
    return Void();
}

Return<void> OpticalFingerprint::onFingerUp() {
    ALOGD("onFingerUp");
    mDevice->touchUp(mDevice);
    return Void();
}

Return<int32_t> OpticalFingerprint::getEnrollmentTotalTimes() {
    return mDevice->get_enrollment_total_times(mDevice);
}

Return<RequestStatus> OpticalFingerprint::pauseEnroll() {
    ALOGI("enter pauseEnroll");
    return ErrorFilter(mDevice->pause_enroll(mDevice));
}

Return<RequestStatus> OpticalFingerprint::continueEnroll() {
    ALOGI("enter continueEnroll");
    return ErrorFilter(mDevice->continue_enroll(mDevice));
}

Return<RequestStatus> OpticalFingerprint::setTouchEventListener() {
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> OpticalFingerprint::dynamicallyConfigLog(uint32_t on) {
    UNUSED(on);
    mDevice->set_image_dump_flag(mDevice, 0, on);
    return RequestStatus::SYS_OK;
}

Return<int32_t> OpticalFingerprint::getEngineeringInfo(uint32_t type) {
    int32_t ret = -1;
    fingerprint_msg_t msg;
    switch (type) {
        case FINGERPRINT_GET_IMAGE_SNR:
            ALOGD("getImageSnr   BEGIN_CALL_API");
            //mDevice->getImageSnr(mDevice);
            ALOGD("getImageSnr   END_CALL_API");
            return 0;
        case FINGERPRINT_GET_IMAGET_QUALITY:
            ALOGD("getImageQuality   BEGIN_CALL_API");
            if ((FP_OPTICAL_GOODIX_G5 == fp_config_info_init.fp_factory_type) ||
                (FP_OPTICAL_GOODIX_G6 == fp_config_info_init.fp_factory_type) ||
                (FP_OPTICAL_GOODIX_G7 == fp_config_info_init.fp_factory_type)) {
                //fixed value for G5 IMAGE_QUALITY, not universal
                int8_t in_buf[8];
                memset(in_buf, 0, 8);
                in_buf[0] = -128;
                in_buf[1] = 23;
                mDevice->sendFingerprintCmd(mDevice, PRODUCT_TEST_CMD_IMAGE_QUALITY, in_buf, 8 * sizeof(int8_t));
                ALOGE("goodix_G5 IMAGET_QUALITY");
            } else {
                mDevice->sendFingerprintCmd(mDevice, CMD_TEST_SZ_FT_KPI, NULL, 0);
                ALOGE("goodix_G2/G3 IMAGET_QUALITY");
            }
            return 0;
        case FINGERPRINT_GET_BAD_PIXELS:
            ALOGD("getBadPixels   BEGIN_CALL_API");
            //ret = mDevice->getBadPixels(mDevice);
            ALOGD("getBadPixels   END_CALL_API");
            break;
        case FINGERPRINT_SELF_TEST:
            ALOGD("selftest   BEGIN_CALL_API");
            if ((FP_OPTICAL_GOODIX_G5 == fp_config_info_init.fp_factory_type) ||
                (FP_OPTICAL_GOODIX_G6 == fp_config_info_init.fp_factory_type) ||
                (FP_OPTICAL_GOODIX_G7 == fp_config_info_init.fp_factory_type)) {
                ret = mDevice->sendFingerprintCmd(mDevice, PRODUCT_TEST_CMD_SPI, NULL, 0);
                ALOGE("goodix_G7 selftest spi ret=%d", ret);
                ret = (ret || mDevice->sendFingerprintCmd(mDevice, PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN, NULL, 0));
                ALOGE("goodix_G7 selftest reset ret =%d", ret);
                ret = (ret || mDevice->sendFingerprintCmd(mDevice, PRODUCT_TEST_CMD_OTP_FLASH, NULL, 0));
                ALOGE("goodix_G7 selftest ret = %d", ret);
            } else {
                ret = mDevice->sendFingerprintCmd(mDevice, CMD_TEST_SZ_FT_SPI, NULL, 0);
                ALOGE("selftest spi ret=%d", ret);
                ret = (ret || mDevice->sendFingerprintCmd(mDevice, CMD_TEST_SZ_FT_SPI_RST_INT, NULL, 0));
                ALOGE("selftest  ret=%d", ret);
            }
            msg.type = ENGINEERING_INFO;
            msg.data.engineering.type = FINGERPRINT_INAGE_SELF_TEST;
            msg.data.engineering.self_test_result = ret;
            notify(&msg);
            ALOGD("selftest   END_CALL_API");
            break;
        default:
            ALOGD("invalid msg type: %d", type);
            return -1;
    }
    return ret;
}

Return<int32_t> OpticalFingerprint::init() {
    return 0;
}

IBiometricsFingerprint* OpticalFingerprint::getInstance() {
    if (!sInstance) {
        sInstance = new OpticalFingerprint();
    }
    return sInstance;
}

fingerprint_device_t* OpticalFingerprint::openHal() {
    int err;
    const hw_module_t *hw_mdl = nullptr;
    ALOGD("OpticalFingerprint Opening fingerprint hal library....");

    if (FP_OPTICAL_GOODIX_G2 == fp_config_info_init.fp_factory_type) {
        if (0 != (err = hw_get_module("fingerprint.goodix_G2", &hw_mdl))) {
            ALOGE("Can't open fingerprint HW Module, error: %d", err);
            return nullptr;
        }
    } else if ((FP_OPTICAL_GOODIX_G3 == fp_config_info_init.fp_factory_type) || (FP_OPTICAL_GOODIX_G3S == fp_config_info_init.fp_factory_type)) {
        if (0 != (err = hw_get_module("fingerprint.goodix_G3", &hw_mdl))) {
            ALOGE("Can't open fingerprint HW Module, error: %d", err);
            return nullptr;
        }
    } else if (FP_OPTICAL_GOODIX_G5 == fp_config_info_init.fp_factory_type) {
        if (0 != (err = hw_get_module("fingerprint.goodix_G5", &hw_mdl))) {
            ALOGE("Can't open fingerprint HW Module, error: %d", err);
            return nullptr;
        }
    } else if (FP_OPTICAL_GOODIX_G6 == fp_config_info_init.fp_factory_type) {
        if (0 != (err = hw_get_module("fingerprint.goodix_G6", &hw_mdl))) {
            ALOGE("Can't open fingerprint HW Module, error: %d", err);
            return nullptr;
        }
    } else if (FP_OPTICAL_GOODIX_G7 == fp_config_info_init.fp_factory_type) {
        if (0 != (err = hw_get_module("fingerprint.goodix_G7", &hw_mdl))) {
            ALOGE("Can't open fingerprint G7 HW Module, error: %d", err);
            return nullptr;
        }
    }

    if (hw_mdl == nullptr) {
        ALOGE("No valid fingerprint module");
        return nullptr;
    }

    fingerprint_module_t const *module =
        reinterpret_cast<const fingerprint_module_t*>(hw_mdl);
    if (module->common.methods->open == nullptr) {
        ALOGE("No valid open method");
        return nullptr;
    }

    hw_device_t *device = nullptr;
    if (0 != (err = module->common.methods->open(hw_mdl, nullptr, &device))) {
        ALOGE("Can't open fingerprint methods, error: %d", err);
        return nullptr;
    }

    if (kVersion != device->version) {
        // enforce version on new devices because of HIDL@2.1 translation layer
        ALOGE("Wrong fp version. Expected %d, got %d", kVersion, device->version);
        return nullptr;
    }

    fingerprint_device_t* fp_device =
        reinterpret_cast<fingerprint_device_t*>(device);

    if (0 != (err =
            fp_device->set_notify(fp_device, OpticalFingerprint::notify))) {
        ALOGE("Can't register fingerprint module callback, error: %d", err);
        return nullptr;
    }

    return fp_device;
}

void OpticalFingerprint::notify(const fingerprint_msg_t *msg) {
    OpticalFingerprint* thisPtr = static_cast<OpticalFingerprint*>(
            OpticalFingerprint::getInstance());
#ifdef FP_HYPNUSD_ENABLE
    if (FINGERPRINT_HYPNUSDSETACION != msg->type) {
        std::lock_guard<std::mutex> lock(thisPtr->mClientCallbackMutex);
    }
#else
    std::lock_guard<std::mutex> lock(thisPtr->mClientCallbackMutex);
#endif
    if (thisPtr == nullptr || thisPtr->mClientCallback == nullptr) {
        ALOGE("Receiving callbacks before the client callback is registered.");
        return;
    }
    const uint64_t devId = reinterpret_cast<uint64_t>(thisPtr->mDevice);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    switch (msg->type) {
        case FINGERPRINT_ERROR: {
                int32_t vendorCode = 0;
                FingerprintError result = vendorErrorFilter(msg->data.error, &vendorCode);
                ALOGD("onError(result:%d, error:%d)", result, msg->data.error);
                if (FingerprintError::ERROR_VENDOR == result) {
                    ALOGE("do nothing with ERROR_VENDOR");
                    break;
                }
                if (!thisPtr->mClientCallback->onError(devId, result, vendorCode).isOk()) {
                    ALOGE("failed to invoke fingerprint onError callback");
                }
            }
            break;
        case FINGERPRINT_ACQUIRED: {
                int32_t vendorCode = 0;
                FingerprintAcquiredInfo result =
                    vendorAcquiredFilter(msg->data.acquired.acquired_info, &vendorCode);
                ALOGD("onAcquired(%d)", result);
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
            if (!thisPtr->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_DOWN,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
                ALOGE("failed to invoke FINGERPRINT_TOUCH_DOWN callback");
            }
            break;
        case FINGERPRINT_TOUCH_UP:
            ALOGD("onTouchUp()");
            if (!thisPtr->mClientCallbackEx->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_UP,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
                ALOGE("failed to invoke FINGERPRINT_TOUCH_UP callback");
            }
#ifdef FP_HYPNUSD_ENABLE
            Hypnus::getInstance()->stopOrms();
#endif
            break;
        case ENGINEERING_INFO:
            {
            uint32_t len = 0;
            uint32_t *key = NULL;
            hidl_string *value = NULL;
            ALOGE("ENGINEERING_INFO ");
            switch (msg->data.engineering.type) {
                case FINGERPRINT_IMAGE_QUALITY:
                    ALOGE(" callback ENGINEERING_INFO FINGERPRINT_IMAGE_QUALITY QUALITY_PASS=%d", msg->data.engineering.quality.quality_pass);
                    len = 3;
                    key = (uint32_t*)malloc(len * sizeof(uint32_t));
                    value = new hidl_string[len];
                    if (key == NULL || value == NULL) {
                        ALOGE("malloc fail");
                        break;
                    }

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
            if (key != NULL) {
                const std::vector<uint32_t> hidl_key(key, key + len);
                const std::vector<hidl_string> hidl_value(value, value+len);
                int count = hidl_value.size();
                for (int i=0; i < count; i++) {
                    ALOGD("onEngineeringInfoUpdated, key[%d] = %d, value[%d] = %s", i, hidl_key[i], i, (android::String8(hidl_value[i].c_str())).string());
                }
                if (!thisPtr->mClientCallbackEx->onEngineeringInfoUpdated(len, hidl_vec<uint32_t>(hidl_key), hidl_vec<hidl_string>(hidl_value)).isOk()) {
                    ALOGE("failed to invoke onEngineeringInfoUpdated callback");
                }
            }
            if (key) {
                free(key);
                key = NULL;
            }
            if (value) {
                delete [] value;
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
                        ALOGD("fingerprint monitor default");
                    }
                if (size > 0) {
#ifdef FP_DCS_ENABLE
                    Dcs::getInstance()->reportMonitor((uint32_t)(msg->data.monitor.data.power.battery));
#endif /* FP_DCS_ENABLE */
                }
            }
            break;

        case FINGERPRINT_OPTICAL_SENDCMD:
                {
                    ALOGD("onFingerprintCmd type==");

                    const std::vector<int8_t> test_result(msg->data.test.result, msg->data.test.result + msg->data.test.result_len);

                    if (!thisPtr->mClientCallbackEx->onFingerprintCmd(msg->data.test.cmd_id,
                            hidl_vec<int8_t>(test_result),
                            msg->data.test.result_len).isOk()) {
                        ALOGE(" FINGERPRINT_OPTICAL_SENDCMD ");
                    }
                }
                break;
        case FINGERPRINT_HEART_RATE_INFO:
                {
                    ALOGD("onFingerprintCmd heartrate type=%d", msg->type);

                    const std::vector<int8_t> test_result(msg->data.test.result, msg->data.test.result + msg->data.test.result_len);

                    if (!thisPtr->mClientCallbackEx->onFingerprintCmd(msg->data.test.cmd_id,
                            hidl_vec<int8_t>(test_result),
                            msg->data.test.result_len).isOk()) {
                        ALOGE(" GF_FINGERPRINT_HEART_RATE_INFO ");
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
        case OPLUS_FINGRPRINT_DCS_INFO:
                {
#ifdef FP_DCS_ENABLE
                    Dcs::getInstance()->reportDcsEventInfo(msg->data.dcs_info);
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
#ifdef FP_CONFIG_SETTINGS_ENABLE
        case FINGERPRINT_GET_CONFIG_DATA:
        {
            fpGetDataById(msg->data.data_config.para);
        }
            break;
#endif

        case FINGERPRINT_BINDCORE:
        {
            int32_t tid = msg->data.bindcore_setting.tid;
            Hypnus::getInstance()->bind_big_core_bytid(tid);
        }
        break;

        case FINGERPRINT_SETUXTHREAD:
        {
            int32_t pid = msg->data.setuxthread_info.pid;
            int32_t tid = msg->data.setuxthread_info.tid;
            int32_t enable = msg->data.setuxthread_info.enable;
            Hypnus::getInstance()->setUxthread(pid, tid, enable);
        }
        break;

        default:
            ALOGE("unkonown msg->type= %d", msg->type);
            break;
        }
    }//notify end

    Return<RequestStatus> OpticalFingerprint::setScreenState(int32_t screen_state) {
        ALOGD("setScreenState ");
        return ErrorFilter(mDevice->setScreenState(mDevice, (uint32_t)screen_state));
    }

    int OpticalFingerprint::getBrightnessValue() {
        int32_t length = 0;
        int index = 0;
        ALOGD("[OpticalFingerprint] getBrightnessValue start");

        memset(mBrightValue, 0, sizeof(mBrightValue));
        char *brightness_paths[] = {
            "/sys/class/leds/lcd-backlight/brightness",
            "/sys/class/backlight/panel0-backlight/brightness",
        };
        for (index = 0; index < sizeof(brightness_paths)/sizeof(brightness_paths[0]); index ++) {
            if (access(brightness_paths[index], 0) == 0) {
                ALOGI("[OpticalFingerprint] Brightness path index %d, path:%s", index, brightness_paths[index]);
                break;
            }
        }
        if (index == sizeof(brightness_paths)/sizeof(brightness_paths[0])) {
            ALOGI("[OpticalFingerprint] no brightness path available");
            return FINGERPRINT_RIGHTNESS_ERROR;
        }
        int fd = open(brightness_paths[index], O_RDONLY);
        if (fd < 0) {
            ALOGI("[OpticalFingerprint] setBrightness err:%d, errno =%d", fd, errno);
            return  FINGERPRINT_RIGHTNESS_ERROR;
        }
        length = read(fd, mBrightValue, sizeof(mBrightValue));
        if (length > 0) {
            ALOGI("[OpticalFingerprint] get mBrightValue = %s  length = %d ", mBrightValue, length);
        }
        else {
            ALOGI("[OpticalFingerprint] read brightness value fail");
            close(fd);
            return  FINGERPRINT_RIGHTNESS_ERROR;
        }
        close(fd);
        return FINGERPRINT_RIGHTNESS_SUCCESS;
    }

    int OpticalFingerprint::setBrightnessValue() {
        int32_t length = 0;
        int index = 0;
        ALOGD("[OpticalFingerprint]setBrightnessValue start");
        char *brightness_paths[] = {
            "/sys/class/leds/lcd-backlight/brightness",
            "/sys/class/backlight/panel0-backlight/brightness",
        };
        for (index = 0; index < sizeof(brightness_paths)/sizeof(brightness_paths[0]); index ++) {
            if (access(brightness_paths[index], 0) == 0) {
                ALOGI("[OpticalFingerprint] Brightness path index %d, path: %s", index, brightness_paths[index]);
                break;
            }
        }
        if (index == sizeof(brightness_paths)/sizeof(brightness_paths[0])) {
            ALOGE("[OpticalFingerprint] no brightness path available");
            return FINGERPRINT_RIGHTNESS_ERROR;
        }

        int fd = open(brightness_paths[index], O_WRONLY);
        if (fd < 0) {
            ALOGI("[OpticalFingerprint] setBrightness err:%d, errno = %d", fd, errno);
            return  FINGERPRINT_RIGHTNESS_ERROR;
        }
        length = write(fd, mBrightValue, sizeof(mBrightValue));
        ALOGI("[OpticalFingerprint] set mBrightValue = %s  length = %d ", mBrightValue, length);
        if (length < 0) {
            ALOGE("[OpticalFingerprint] write brightness value fail");
            close(fd);
            return  FINGERPRINT_RIGHTNESS_ERROR;
        }
        close(fd);
        return FINGERPRINT_RIGHTNESS_SUCCESS;
    }


    Return<int32_t> OpticalFingerprint::sendFingerprintCmd(int32_t cmdId,  const hidl_vec<int8_t>& in_buf ) {
        ALOGD("optical sendFingerprintCmd ");
        ALOGE("sendFingerprintCmd in_buf= %s ", (int8_t*)in_buf.data());
        int ret = FINGERPRINT_RIGHTNESS_ERROR;

        switch (cmdId) {
#ifndef CLOSE_SIMULATOR_TP
        case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIMULATOR_TP: //CMD_FINGERPRINT_TP
        {
                if (0 == in_buf[0]) {
                ALOGD("simulator_tp down");
                mDevice->touchDown(mDevice);
            }
            else {
                ALOGD("simulator_tp up");
                mDevice->touchUp(mDevice);
            }
            break;
        }

        case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_START_CALI:
        {
            ret = getBrightnessValue();
            if (FINGERPRINT_RIGHTNESS_ERROR == ret) {
                ALOGE("[OpticalFingerprint] getBrightnessValue fail");
                return -14;    //RequestStatus::SYS_EFAULT;
            }
            return 0;    //RequestStatus::SYS_OK;
        }
        case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_END_CALI:
        {
            ret = setBrightnessValue();
            if (FINGERPRINT_RIGHTNESS_ERROR == ret) {
                ALOGE("[OpticalFingerprint] setBrightnessValue fail");
                return -14;    //RequestStatus::SYS_EFAULT;
            }
            return 0;    //RequestStatus::SYS_OK;
        }
#endif
        case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE:
        {
            uint64_t operationId = 0;
            uint32_t gid = 0;
            int32_t authtype;

            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&authtype, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE  authtype = %d", authtype);

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
        return mDevice->sendFingerprintCmd(mDevice, cmdId, (int8_t*)in_buf.data(), (uint32_t)in_buf.size());
    }

    Return<RequestStatus> OpticalFingerprint::authenticateAsType(uint64_t operationId,
        uint32_t gid, int32_t authtype) {
        ALOGI("authenticate type: %d", authtype);
        return ErrorFilter(mDevice->authenticateAsType(mDevice, operationId, gid, static_cast<uint32_t>(authtype)));
    }

} // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace oplus
}  // namespace vendor

