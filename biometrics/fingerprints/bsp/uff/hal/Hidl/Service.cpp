/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Service.cpp
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][Service]"

#include <cutils/properties.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <stdio.h>
#include <stdlib.h>
#include "HalLog.h"
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#ifdef VERSION_ANDROID_R
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/types.h>
#include "BiometricsFingerprint.h"
#else
#include "BiometricsFingerprintAdaptS.h"
#endif

using android::OK;
using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using vendor::oplus::hardware::biometrics::fingerprint::V2_1::implementation::BiometricsFingerprint;

#define OPTICAL_FEATURE "persist.vendor.fingerprint.optical.support"

int main() {
    LOG_E(LOG_TAG, "enter to fp service");
    int err = 0;
    property_set(OPTICAL_FEATURE, "1");
    configureRpcThreadpool(1, false /*callerWillJoin*/);
    android::sp<BiometricsFingerprint> bio =
        static_cast<BiometricsFingerprint *>(BiometricsFingerprint::getInstance());
    do {
        if (bio->init()) {
            LOG_E(LOG_TAG, "init failed!");
            goto out;
        } else {
            err = bio->registerAsService();
            LOG_E(LOG_TAG, "add to service!");
            break;
        }
    } while (1);

out:
    joinRpcThreadpool();
    return 0;
}
