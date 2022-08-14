#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
//#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
//#include <android/hardware/biometrics/fingerprint/2.1/types.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/types.h>

#include <utils/StrongPointer.h>
#include <binder/IPCThreadState.h>
//#include <vendor/egistec/hardware/fingerprint/4.0/IBiometricsFingerprintRbs.h>
#include <stdint.h>
#include <cutils/properties.h>
#include <cutils/log.h>
#include "egis_service.h"
#include "rbs_hidl.h"
//using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;

using ::android::sp;
using android::hardware::configureRpcThreadpool;
using ::android::hardware::hidl_vec;
using android::hardware::joinRpcThreadpool;
//using vendor::egistec::hardware::fingerprint::V4_0::IBiometricsFingerprintRbs;

void egis_server_init()
{
	ALOGE("egis_server_init start");

	android::sp<IBiometricsFingerprint> fingerprint_hal_extra = new ets_extra_hidl();

	if (fingerprint_hal_extra == nullptr) {
		ALOGE("Failed to create fingerprint_hal_extra");
		return;
	}

	configureRpcThreadpool(1, true);  // configureRpcThreadpool(1, false);

	if (fingerprint_hal_extra != nullptr) {
		fingerprint_hal_extra->registerAsService();
	}

	joinRpcThreadpool();  // android::IPCThreadState::self()->joinThreadPool();

	return;
}
