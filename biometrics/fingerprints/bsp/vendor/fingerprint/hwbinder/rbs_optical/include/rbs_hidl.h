//
//    Copyright 2017 Egis Technology Inc.
//
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Egis in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//

#ifndef ETS_HIDL_HEADER
#define ETS_HIDL_HEADER

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "rbs_hal.h"
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallbackEx.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
#include <android/hardware/biometrics/fingerprint/2.1/types.h>
#include "plat_log.h"
#include "Perf.h"
using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallbackEx;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;


class ets_hidl : public IBiometricsFingerprint
{
    public:
	ets_hidl();
	~ets_hidl();

	int open();
	int close();

        Return<uint64_t> setNotify(const sp<IBiometricsFingerprintClientCallback>&clientCallback) override;
        Return<uint64_t> preEnroll() override;
        Return<RequestStatus> enroll(const hidl_array<uint8_t, 69>& hat, uint32_t gid, uint32_t timeoutSec) override;

	Return<RequestStatus> postEnroll() override;
	Return<uint64_t> getAuthenticatorId() override;
	Return<RequestStatus> cancel() override;
	Return<RequestStatus> enumerate() override;

	Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
	Return<RequestStatus> setActiveGroup(uint32_t gid, const hidl_string& storePath) override;
	Return<RequestStatus> authenticate(uint64_t operation_id, uint32_t gid) override;
        Return<bool> isUdfps(uint32_t sensorId) override;
        Return<void> onFingerDown(uint32_t x, uint32_t y, float minor, float major) override;
        Return<void> onFingerUp() override;
        Return<int32_t> sendFingerprintCmd(int32_t cmdId,  const hidl_vec<int8_t>& in_buf) override;
        Return<uint64_t> setHalCallback(const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) override;
//For OPLUS
	Return<RequestStatus> authenticateAsType(uint64_t operation_id, uint32_t gid, int32_t authtype) ;
	Return<int32_t> init();

	Return<int32_t> getEnrollmentTotalTimes() ;
	Return<RequestStatus> pauseEnroll() ;
	Return<RequestStatus> continueEnroll() ;
	Return<RequestStatus> pauseIdentify() ;
	Return<RequestStatus> continueIdentify() ;
	Return<RequestStatus> setTouchEventListener() ;
	Return<RequestStatus> setScreenState(int32_t screen_state) ;
	Return<RequestStatus> dynamicallyConfigLog(uint32_t on) ;
	Return<int32_t> getEngineeringInfo(uint32_t type) ;


    private:
	static void egis_bind_big_core(void);
	static void on_touch_down(void* context);
	static void on_touch_up(void* context);
	static void set_action(void* context, uint32_t type, uint32_t time_out);
	static void on_monitor_event_triggered(void* context, uint32_t type, char* data);
	static void on_image_info_acquired(void* context, uint32_t type, uint32_t quality, uint32_t match_score);
	static void on_sync_templates(void* context, uint32_t* fingerIds, int fingerIds_size, uint32_t groupId);
	static hidl_string getHidlstring(uint32_t param);
	static void on_engineering_info_updated(void* context, uint32_t lenth, uint32_t* keys, int keys_size, uint32_t* values, int values_size);
	static void on_fingerprint_cmd(void* context, int32_t cmdId, int8_t* result, uint32_t resultLen);
	static void on_enroll_result(void* context, uint32_t fid, uint32_t gid,
				     uint32_t remaining);
	static void on_acquired(void* context, int info);
	static void on_authenticated(void* context, uint32_t fid, uint32_t gid,
				     const uint8_t* token, uint32_t size_token);
	static void on_error(void* context, int code);
	static void on_removed(void* context, uint32_t fid, uint32_t gid,
			       uint32_t remaining);
	static void on_enumerate(void* context, uint32_t fid, uint32_t gid,
				 uint32_t remaining);
	static void on_send_dcsmsg(void* context, int32_t cmdId, int8_t* dcsmsg, uint32_t dcsmsgLen);

        std::mutex callback_mutex;
	sp<IBiometricsFingerprintClientCallback> mClientCallback;
        sp<IBiometricsFingerprintClientCallbackEx> mClientCallbackEx;
	fingerprint_callback_t ets_callback;
	egis_fingerprint_hal_device_t* mDevice;
};

#endif  // ETS_HIDL_HEADER
