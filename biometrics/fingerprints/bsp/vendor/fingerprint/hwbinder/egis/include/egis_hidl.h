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

#ifndef EGIS_HIDL_HEADER
#define EGIS_HIDL_HEADER

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>

#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallbackEx.h>
#include "egis_fingerprint.h"


using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallbackEx;
using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;


class egis_hidl : public IBiometricsFingerprint
{
       public:
	egis_hidl();
	~egis_hidl();

	int open();
	int close();
//	int start_service();

	Return<uint64_t> setNotify(const sp<IBiometricsFingerprintClientCallback>& clientCallback) override;
	Return<uint64_t> preEnroll() override;
	Return<RequestStatus> enroll(const hidl_array<uint8_t, 69>& hat, uint32_t gid, uint32_t timeoutSec) override;
	Return<RequestStatus> postEnroll() override;
	Return<RequestStatus> cancel() override;
	Return<RequestStatus> enumerate() override;
	Return<uint64_t> getAuthenticatorId() override;
	Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
	Return<RequestStatus> setActiveGroup(uint32_t gid, const hidl_string& storePath) override;
	Return<RequestStatus> authenticate(uint64_t operation_id, uint32_t gid) override;
	Return<RequestStatus> cleanUp() ;
	Return<RequestStatus> pauseEnroll() ;
	Return<RequestStatus> continueEnroll() ;
	Return<RequestStatus> setTouchEventListener() ;
	Return<RequestStatus> dynamicallyConfigLog(uint32_t on) ;
	Return<RequestStatus> pauseIdentify() ;
	Return<RequestStatus> continueIdentify() ;
	Return<RequestStatus> getAlikeyStatus() ;
	Return<int32_t> getEnrollmentTotalTimes() ;
	Return<RequestStatus> setScreenState(int32_t screen_state) ;
	Return<int32_t > getEngineeringInfo(uint32_t type) ;
	Return<bool> isUdfps(uint32_t sensorId) override;
    Return<void> onFingerDown(uint32_t x, uint32_t y, float minor, float major) override;
    Return<void> onFingerUp() override;
	Return<int32_t> sendFingerprintCmd(int32_t cmdId,  const hidl_vec<int8_t>& in_buf) override;
    Return<uint64_t> setHalCallback(const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) override;


	Return<RequestStatus> authenticateAsType(uint64_t operationId, uint32_t gid, int32_t authtype) ;
	//RM.BSP.Fingerprint.Basic Qingwen.Liu 20190109 add for fingerprint-key mode begin
	Return<int32_t> setFingerKeymode(uint32_t enable);
	//RM.BSP.Fingerprint.Basic Qingwen.Liu 20190109  add for fingerprint-key mode end

private:
	static void on_enroll_result(void* context, uint32_t fid, uint32_t gid, uint32_t remaining);
	static void on_acquired(void* context, int info);
	static void on_authenticated(void* context, uint32_t fid, uint32_t gid, const uint8_t* token, uint32_t size_token);
	static void on_error(void* context, int code);
	static void on_removed(void* context, uint32_t fid, uint32_t gid, uint32_t remaining);
	static void on_enumerate(void* context, uint32_t fid, uint32_t gid, uint32_t remaining);
	static void on_sync_templates(void *context, uint32_t *fids, uint32_t fids_count, uint32_t gid);
	static void on_touch_up(void *context);
	static void on_touch_down(void *context);
	static void on_monitor_event_triggered(void *context, uint32_t type, char* data);
	static void on_image_info_acquired(void *context, uint32_t type, uint32_t quality, uint32_t match_score);
	static void on_engineering_info_update(void *context, uint32_t type, int result, int ext_info);
	static hidl_string getHidlstring(uint32_t param);
	
	std::mutex mClientCallbackMutex;	
	sp<IBiometricsFingerprintClientCallback> mClientCallback;
	sp<IBiometricsFingerprintClientCallbackEx> mClientCallbackEx;
	fingerprint_callback_t ets_callback;
	egis_fingerprint_hal_device_t* mDevice;
};

#endif  // ETS_HIDL_HEADER
