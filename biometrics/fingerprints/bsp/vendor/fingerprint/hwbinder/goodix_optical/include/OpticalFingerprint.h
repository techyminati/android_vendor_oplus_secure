/************************************************************************************
 ** File: - vendor\fingerprint\hwbinder\goodix_optical\OpticalFingerprint.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation header file for goodix optical
 **
 ** Version: 1.0
 ** Date created: 15:09:11,20/08/2019
 ** Author: Ziqing.Guo@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 ** Ziqing.Guo         2019/08/29       create file
 ** Ziqing.Guo         2019/08/29       add to include perf.h && dcs.h
 ** Qijia.Zhou         2021/04/27       modify hidl interface for Android S
 ************************************************************************************/

#ifndef VENDOR_OPLUS_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_OPTICALFINGERPRINT_H
#define VENDOR_OPLUS_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_OPTICALFINGERPRINT_H

#include <log/log.h>
#include <android/log.h>
#include <hardware/hardware.h>
//#include <hardware/fingerprint.h>
#include <fingerprint.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallbackEx.h>
#ifdef OPLUS_GOODIX_SUPPORT
//#include "public/gf_fingerprint.h"
#endif
#include <utils/String16.h>

#include "Perf.h"
#include "dcs.h"
#include "HealthMonitor.h"

namespace vendor {
namespace oplus {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {
namespace implementation {


using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallbackEx;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_array;
using ::android::sp;
using android::String16;

struct OpticalFingerprint : public IBiometricsFingerprint {
public:
    OpticalFingerprint();
    ~OpticalFingerprint();

    // Method to wrap legacy HAL with BiometricsFingerprint class
    static IBiometricsFingerprint* getInstance();

    // Methods from ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint follow.
    Return<uint64_t> setNotify(const sp<IBiometricsFingerprintClientCallback>& clientCallback) override;
    Return<uint64_t> preEnroll() override;
    Return<RequestStatus> enroll(const hidl_array<uint8_t, 69>& hat, uint32_t gid, uint32_t timeoutSec) override;
    Return<RequestStatus> postEnroll() override;
    Return<uint64_t> getAuthenticatorId() override;
    Return<RequestStatus> cancel() override;
    Return<RequestStatus> enumerate() override;
    Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
    Return<RequestStatus> setActiveGroup(uint32_t gid, const hidl_string& storePath) override;
    Return<RequestStatus> authenticate(uint64_t operationId, uint32_t gid) override;
    Return<bool> isUdfps(uint32_t sensorId) override;
    Return<void> onFingerDown(uint32_t x, uint32_t y, float minor, float major) override;
    Return<void> onFingerUp() override;
    Return<int32_t> sendFingerprintCmd(int32_t cmdId,  const hidl_vec<int8_t>& in_buf) override;
    Return<uint64_t> setHalCallback(const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) override;

    Return<int32_t> init();
//add by chenran
    Return<RequestStatus> authenticateAsType(uint64_t operationId, uint32_t gid, int32_t authtype);
    Return<int32_t> getEnrollmentTotalTimes();
    Return<RequestStatus> pauseEnroll();
    Return<RequestStatus> continueEnroll();
    Return<RequestStatus> setTouchEventListener();
    Return<RequestStatus> setScreenState(int32_t screen_state);
    Return<RequestStatus>  dynamicallyConfigLog(uint32_t on);
    Return<int32_t> getEngineeringInfo(uint32_t type);
    int getBrightnessValue();
    int setBrightnessValue();
    char mBrightValue[50];

private:
    static fingerprint_device_t* openHal();
    static void notify(const fingerprint_msg_t *msg); /* Static callback for legacy HAL implementation */
    static Return<RequestStatus> ErrorFilter(int32_t error);
    static FingerprintError vendorErrorFilter(int32_t error, int32_t* vendorCode);
    static FingerprintAcquiredInfo vendorAcquiredFilter(int32_t error, int32_t* vendorCode);
    static OpticalFingerprint* sInstance;

    std::mutex mClientCallbackMutex;
    sp<IBiometricsFingerprintClientCallback> mClientCallback;
    sp<IBiometricsFingerprintClientCallbackEx> mClientCallbackEx;
    fingerprint_device_t *mDevice;

    static hidl_string getHidlstring(uint32_t param);

};


}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace oplus
}  // namespace vendor


#endif  // ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H
