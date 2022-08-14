/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/


#ifndef FPC_HIDL_SW28_H
#define FPC_HIDL_SW28_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallbackEx.h>
#include "fpc_tee_hal.h"
#include <utils/String16.h>

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


#ifdef FP_HYPNUSD_ENABLE
#include "Perf.h"
#endif /* FP_HYPNUSD_ENABLE */

#ifdef FP_DCS_ENABLE
#include "dcs.h"
#include "HealthMonitor.h"
#endif /* FP_DCS_ENABLE */

class fpc_hidl : public IBiometricsFingerprint {
public:
    fpc_hidl();
    ~fpc_hidl();

    int init();

    Return<uint64_t> setNotify(const sp<IBiometricsFingerprintClientCallback>&clientCallback) override;
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
    Return<int32_t> getEnrollmentTotalTimes();
    Return<RequestStatus> pauseEnroll();
    Return<RequestStatus> continueEnroll();
    Return<RequestStatus> pauseIdentify();
    Return<RequestStatus> continueIdentify();
    Return<RequestStatus> setTouchEventListener();
    Return<RequestStatus> setScreenState(int32_t screen_state);
    Return<RequestStatus>  dynamicallyConfigLog(uint32_t on);
    Return<int32_t> getEngineeringInfo(uint32_t type);

private:
    static void on_enroll_result(void* context, uint32_t fid, uint32_t gid,
                             uint32_t remaining);
    static void on_acquired(void* context, int code);
    static void on_authenticated(void* context, uint32_t fid, uint32_t gid,
                                 const uint8_t* token, uint32_t size_token);
    static void on_error(void* context, int code);
    static void on_removed(void* context, uint32_t fid, uint32_t gid,
                       uint32_t remaining);
    static void on_enumerate(void* context, uint32_t fid, uint32_t gid,
                       uint32_t remaining);
    static void on_touch_down(void* context);
    static void on_touch_up(void* context);
    static void on_imageinfo_acquired(void* context, uint32_t type, uint32_t quality, uint32_t match_score);
    static void on_sync_templates(void* context, const uint32_t* fids, uint32_t size_fid, uint32_t gid);
    static void on_engineeringinfo_updated(void* context, engineering_info_t engineering);
    static void on_monitor(void* context, uint32_t type, double data);
    static void on_dcsmsg(fingerprint_auth_dcsmsg_t auth_context);

    static void hypnusSetAction();
    static void fpc_bind_bigcore_bytid(uint32_t tid);

    static String16 getString16(uint32_t param);
    static String16 getString16(double param);
    static String16 getString16(float param);
    static hidl_string getHidlstring(uint32_t param);

    std::mutex callback_mutex;
    sp<IBiometricsFingerprintClientCallback> callback;
    sp<IBiometricsFingerprintClientCallbackEx> mClientCallbackEx;
    fpc_hal_compat_callback_t compat_callback;
    fpc_hal_common_t* hal;
};

#endif  // FPC_HIDL_SW28_H
