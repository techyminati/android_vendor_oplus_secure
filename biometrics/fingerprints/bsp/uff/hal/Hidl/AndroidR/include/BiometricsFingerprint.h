/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef VENDOR_OPLUS_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H
#define VENDOR_OPLUS_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H

#include <android/log.h>
#include <hardware/hardware.h>
#include <log/log.h>
#include <fingerprint.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
#include <utils/String16.h>

#include "HealthMonitor.h"
#include "HalContext.h"

namespace vendor {
namespace oplus {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {
namespace implementation {

using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::
    FingerprintAcquiredInfo;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::
    FingerprintError;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::
    IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::
    IBiometricsFingerprintClientCallback;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::RequestStatus;

using ::android::sp;
using android::String16;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::
    FingerprintAuthType;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::
    FingerprintScreenState;
using ::android::HalContext;

struct BiometricsFingerprint : public IBiometricsFingerprint {
   public:
    BiometricsFingerprint();
    ~BiometricsFingerprint();

    static IBiometricsFingerprint *getInstance();
    Return<uint64_t> setNotify(const sp<IBiometricsFingerprintClientCallback>
                                   &clientCallback) override;
    Return<uint64_t> preEnroll() override;
    Return<RequestStatus> enroll(const hidl_array<uint8_t, 69> &hat,
                                 uint32_t gid, uint32_t timeoutSec) override;
    Return<RequestStatus> postEnroll() override;
    Return<uint64_t> getAuthenticatorId() override;
    Return<RequestStatus> cancel() override;
    Return<RequestStatus> enumerate() override;
    Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
    Return<RequestStatus> setActiveGroup(uint32_t gid,
                                         const hidl_string &storePath) override;
    Return<RequestStatus> authenticate(uint64_t operationId,
                                       uint32_t gid) override;

    // add by vendor
    Return<int32_t> init();
    Return<RequestStatus> authenticateAsType(
        uint64_t operationId, uint32_t gid,
        FingerprintAuthType authtype) override;
    Return<int32_t> getEnrollmentTotalTimes() override;
    Return<RequestStatus> setScreenState(
        FingerprintScreenState screen_state) override;
    Return<int32_t> getEngineeringInfo(uint32_t type) override;
    Return<RequestStatus> sendFingerprintCmd(
        int32_t cmdId, const hidl_vec<int8_t> &in_buf) override;
    Return<RequestStatus> touchDown() override;
    Return<RequestStatus> touchUp() override;

    // del for androidR
    Return<RequestStatus> pauseEnroll() override;
    Return<RequestStatus> continueEnroll() override;
    Return<RequestStatus> pauseIdentify() override;
    Return<RequestStatus> continueIdentify() override;
    Return<RequestStatus> getAlikeyStatus() override;
    Return<RequestStatus> cleanUp() override;
    Return<RequestStatus> setTouchEventListener() override;
    Return<RequestStatus> dynamicallyConfigLog(uint32_t on) override;

   private:
    static void notify(
        const fingerprint_msg_t
            *msg); /* Static callback for legacy HAL implementation */
    static Return<RequestStatus> ErrorFilter(int32_t error);
    static FingerprintError vendorErrorFilter(int32_t error,
                                              int32_t *vendorCode);
    static FingerprintAcquiredInfo vendorAcquiredFilter(int32_t error,
                                                        int32_t *vendorCode);
    static BiometricsFingerprint *sInstance;

    std::mutex mClientCallbackMutex;
    sp<IBiometricsFingerprintClientCallback> mClientCallback;
    static hidl_string getHidlstring(uint32_t param);
    HalContext *mHalContext;
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace oplus
}  // namespace vendor

#endif  // ANDROID_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BIOMETRICSFINGERPRINT_H
