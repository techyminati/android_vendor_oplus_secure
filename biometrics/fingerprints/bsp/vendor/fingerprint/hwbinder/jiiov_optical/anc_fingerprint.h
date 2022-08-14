/************************************************************************************
 ** File: - vendor\fingerprint\hwbinder\jiiov_optical\anc_fingerprint.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for Jiiov(android O)
 **
 ** Version: 1.0
 ** Date created: 15:09:11,23/8/2021
 ** Author: Fengxiong.Wu@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Fengxiong.Wu         2021/08/23       modify hidl interface for Android S
 ************************************************************************************/

#ifndef ANC_FINGERPRINT_H
#define ANC_FINGERPRINT_H

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
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using android::String16;

struct AncFingerprint : public IBiometricsFingerprint {
public:
    AncFingerprint();
//    explicit AncFingerprint();
    ~AncFingerprint();

//    static AncFingerprint* GetInstance();
    // Method to wrap legacy HAL with BiometricsFingerprint class
    static IBiometricsFingerprint* getInstance();
    bool init();

    // Methods from ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint follow.
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

    Return<RequestStatus> authenticateAsType(uint64_t operationId, uint32_t gid, int32_t authtype);
//    Return<RequestStatus> cleanUp();
    Return<RequestStatus> pauseEnroll();
    Return<RequestStatus> continueEnroll();
    Return<RequestStatus> setTouchEventListener();
    Return<RequestStatus> dynamicallyConfigLog(uint32_t on);
//    Return<RequestStatus> pauseIdentify();
//    Return<RequestStatus> continueIdentify();
//    Return<RequestStatus> getAlikeyStatus();
    Return<int32_t> getEnrollmentTotalTimes();
    Return<RequestStatus> setScreenState(int32_t screen_state);
    Return<int32_t> getEngineeringInfo(uint32_t type);
    int getBrightnessValue();
    int setBrightnessValue();
    char mBrightValue[50];
    Return<RequestStatus> touchDown();
    Return<RequestStatus> touchUp();

private:
    static void onEnrollResult(void *p_device, uint32_t finger_id, uint32_t group_id, uint32_t remaining);
    static void onAcquired(void *p_device, int32_t vendor_code);
    static void onAuthenticated(void *p_device, uint32_t finger_id, uint32_t group_id,
                             const uint8_t* token, uint32_t token_length);
    static void onError(void *p_device, int vendor_code);
    static void onRemoved(void *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);
    static void onEnumerate(void *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);
    static void onTouchDown(void *p_device);
    static void onTouchUp(void *p_device);
    static void onMonitorEventTriggered(const uint8_t *p_out, uint32_t out_length);
    static void onImageInfoAcquired(const uint8_t *p_out, uint32_t out_length);
    static void onSyncTemplates(void *p_device, const uint8_t *p_out, uint32_t out_length);
    static void onEngineeringInfoUpdated(const uint8_t *p_out, uint32_t out_length);
    static void onFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& result, uint32_t resultLen);

    std::mutex mClientCallbackMutex;

    int excuteCommand(int32_t command_id, const std::vector<uint8_t>& inputData, std::vector<uint8_t>& outputData);
    static void onExcuteCommand(void *p_device, int32_t command_id, int32_t argument, const uint8_t *out, uint32_t out_length);
    static hidl_string getHidlstring(uint32_t param);
    static void doExternalWork(void *p_device, int32_t type, const uint8_t *p_buffer, uint32_t buffer_length);

    static AncFingerprint* s_instance_;

    android::sp<IBiometricsFingerprintClientCallback> sp_fp_client_callback_;
    android::sp<IBiometricsFingerprintClientCallbackEx> sp_fp_client_callback_Ex;
    void* p_device_;
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace oplus
}  // namespace vendor

#endif  // VENDOR_OPLUS_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H
