#ifndef AUTOSMOKING_CLIENT_CASE_H
#define AUTOSMOKING_CLIENT_CASE_H

#ifdef VERSION_ANDROID_R
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::FingerprintAuthType;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::FingerprintScreenState;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::FingerprintError;
#else
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallbackEx.h>
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallbackEx;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;
#endif

#include <unistd.h>

#define AUTH_TOKEN_SIZE 69


using android::hardware::hidl_vec;
using android::hardware::hidl_array;
using android::hardware::hidl_string;
using android::hardware::Return;
using android::sp;
using android::hardware::hidl_string;

typedef enum
{
    CMD_TEST_SUPPORT_SMOKING = 5000,
    CMD_TEST_ENROLL_AUTH_SUCCESS = 5001,
    CMD_TEST_ENROLL_AUTH_FAIL = 5002,
    CMD_TEST_ENROLL_DEPLICATE = 5003,
    CMD_TEST_ENROLL_AUTH_SUCCESS_MUL = 5004,
    CMD_TEST_ENROLL_AUTH_FAIL_MUL = 5005,
    CMD_TEST_ENROLL_AUTH_SUCCESS_AND_FAIL = 5006,
    CMD_TEST_ENROLL_EXCEED_UPPER_LIMIT = 5007,
    CMD_TEST_ENROLL_CANCEL = 5008,
    CMD_TEST_ENROLL_REPEAT_PROCESS = 5009,
    CMD_TEST_ENROLL_LOW_QUALITY = 5010,
    CMD_TEST_ENROLL_SAMLL_AREA = 5011,
    CMD_TEST_ENROLL_TIMEOUT_AUTH = 5012,
    CMD_TEST_ENROLL_AUTH_LOW_QUALITY = 5013,
    CMD_TEST_ENROLL_MANAY_ERR_AUTH = 5014,
    CMD_TEST_REMOVE_ALL_FINGERPRINTS = 5015,
    CMD_TEST_SZ_MAX,
} CLIENT_PRODUCT_TEST_CMD;

class BiometricsFingerprintClientCallback : public IBiometricsFingerprintClientCallback {
 public:
    // implement methods of IBiometricsFingerprintClientCallback
    virtual Return<void> onEnrollResult(uint64_t deviceId, uint32_t fingerId, uint32_t groupId,
                                      uint32_t remaining) override {
        printf("FP_DEBUG1 Enroll callback called, deviceId:%llu, fingerId:%d, groupId:%d, remaining:%d\n",
               (unsigned long long)deviceId, fingerId, groupId, remaining);
        return Return<void>();
    }

    virtual Return<void> onAcquired(uint64_t deviceId, FingerprintAcquiredInfo acquiredInfo,
                                 int32_t vendorCode) override {
        printf("Acquired callback called, deviceId:%llu, acquiredInfo:%d, vendorCode:%d\n",
               (unsigned long long)deviceId, (int)acquiredInfo, vendorCode);
        if ((int)acquiredInfo == 1002) {
            printf("finger already enrolled!\n");
        }
        return Return<void>();
    }

    virtual Return<void> onAuthenticated(uint64_t deviceId, uint32_t fingerId, uint32_t groupId,
                                 const hidl_vec<uint8_t>& token) override {
        printf("Authenticated callback called, deviceId:%llu, fingerId:%d, groupId:%d\n",
               (unsigned long long)deviceId, fingerId, groupId);
        return Return<void>();
    }

    virtual Return<void> onError(uint64_t deviceId, FingerprintError error, int32_t vendorCode)
        override {
        printf("Error callback called, deviceId:%llu, error:%d, vendorCode:%d\n",
               (unsigned long long)deviceId, (int)error, vendorCode);
        return Return<void>();
    }

    virtual Return<void> onRemoved(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, uint32_t remaining)
        override {
        printf("Removed callback called, deviceId:%llu, fingerId:%d, groupId:%d, remaining:%d\n",
               (unsigned long long)deviceId, fingerId, groupId, remaining);
        return Return<void>();
    }

    virtual Return<void> onEnumerate(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, uint32_t remaining)
        override {
        printf("FP_DEBUG Enumerate callback called, deviceId:%llu, fingerId:%d, groupId:%d, remaining:%d\n",
               (unsigned long long)deviceId, fingerId, groupId, remaining);
        return Return<void>();
    }

    Return<void> onTouchDown(uint64_t deviceId) {
        return Return<void>();
    }

    Return<void> onTouchUp(uint64_t deviceId) {
        return Return<void>();
    }

    Return<void> onMonitorEventTriggered(uint32_t type, const hidl_string& data) {
        return Return<void>();
    }

    Return<void> onImageInfoAcquired(uint32_t type, uint32_t quality, uint32_t match_score) {
        ALOGD("ImageInfoAcquired callback called.");
        return Return<void>();
    }

    Return<void> onSyncTemplates(uint64_t deviceId, const hidl_vec<uint32_t>& fingerIds, uint32_t groupId)
    {
        ALOGD("SyncTemplates callback called.");
        if (fingerIds.size() != 0) {

            ALOGD("deviceId:%lu, fingerIds:%d, groupId:%d\n", deviceId, (uint32_t)(fingerIds[0]), groupId);
        }
        return Return<void>();
    }

    Return<void> onEngineeringInfoUpdated(uint32_t lenth, const hidl_vec<uint32_t>& keys, const hidl_vec<hidl_string>& values)
    {
        ALOGD("EngineeringInfoUpdated callback called.");
        return Return<void>();
    }

    Return<void> onFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& result, uint32_t resultLen)
    {
        ALOGD("FingerprintCmd callback called.");

        if (resultLen != result.size()) {
            ALOGD("result length : %u, result size : %ld", resultLen, result.size());
            return Return<void>();
        }
        return Return<void>();
    }
};

void enroll_and_authicate(android::sp<IBiometricsFingerprint> service, sp<BiometricsFingerprintClientCallback> client_call_back,int cmd);

#endif
