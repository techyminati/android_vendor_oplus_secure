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
//#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
//#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>

//#include <vendor/egistec/hardware/fingerprint/4.0/IBiometricsFingerprintRbs.h>

using ::android::sp;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
//using ::android::hardware::biometrics::fingerprint::V2_1::
//FingerprintAcquiredInfo;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
//using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::FingerprintError;
//using ::android::hardware::biometrics::fingerprint::V2_1::
//  IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;

//using ::android::hardware::biometrics::fingerprint::V2_1::
//IBiometricsFingerprintClientCallback;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
//using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::RequestStatus;

//using ::vendor::egistec::hardware::fingerprint::V4_0::IBiometricsFingerprintRbs;
//using ::vendor::egistec::hardware::fingerprint::V4_0::IBiometricsFingerprintRbsCallback;

/*
class ets_extra_hidl : public IBiometricsFingerprint
{
	Return<void> extra_api(int32_t pid, const hidl_vec<uint8_t>& in_buffer, extra_api_cb _hidl_cb) override;
	Return<int32_t> set_on_callback_proc(const ::android::sp<IBiometricsFingerprintClientCallback>& clientCallback) override;
};
*/
#endif  // ETS_HIDL_HEADER
