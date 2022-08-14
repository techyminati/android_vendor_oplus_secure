/************************************************************************************
 ** File: - vendor/fingerprint/hwbinder/service.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,21/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Ziqing.guo   2017/10/21        create the file
 **  Ziqing.guo   2017/10/21        use hidl to replace bindle interface
 **  Ziqing.guo   2017/10/27        add some logs for debugging
 **  Ran.Chen     2017/11/09        add for goodix_fp
 **  Ziqing.Guo   2017/12/03        fix the warning message
 **  Bin.Li       2017/12/03        add for goodix_fp config
 **  Ran.Chen     2017/12/26        modify for no fingerprint
 **  Ran.Chen     2018/01/29        modify for fp_id, Code refactoring
 **  Ran.Chen     2018/03/27        add for pid notifyInit
 **  Ran.Chen     2018/06/28        add persist.vendor.fingerprint.optical.support  for optical fingerprint
 **  Ran.Chen     2018/11/26        remove fpc for SDM855
 **  Yang.Tan     2018/11/26        add fpc sw23 and sw28 compatible
 **  Ran.Chen     2018/12/05        add for optical fingerprint iconlocation
 **  Long.Liu     2019/01/18        add fingerprint hal retry init
 **  Qing.Guan    2019/04/01        add egis finger and pid notify
 **  Ziqing.Guo   2019/08/16        modify for goodix optical android Q (Euclid)
 **  Ziqing.Guo   2019/08/21        add hypnus module
 **  Ran.Chen     2019/10/09        modify for setprop befor init_hw
 ************************************************************************************/

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <vendor/oplus/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/types.h>
#include <cutils/properties.h>
#ifdef GOODIX_FP_ENABLE
#include "OpticalFingerprint.h"
#endif
//#include "SileadFingerprint.h"
//#include "BiometricsFingerprint.h"
#include <stdlib.h>
#include <stdio.h>
#include "fingerprint_type.h"

#ifdef FPC_FP_ENABLE
#include "fpc_hidl_sw28.h"
#endif

#ifdef EGIS_FP_ENABLE
#include "rbs_hidl.h"
#endif

#include "Perf.h"

#ifdef FP_CONFIG_SETTINGS_ENABLE
#include "FingerprintSettings.h"
#endif

#ifdef EGIS_CAPACITY_FP_ENABLE
#include "egis_hidl.h"
#endif


using ::vendor::oplus::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
#ifdef GOODIX_FP_ENABLE
using vendor::oplus::hardware::biometrics::fingerprint::V2_1::implementation::OpticalFingerprint;
#endif
//using vendor::oplus::hardware::biometrics::fingerprint::V2_1::implementation::SileadFingerprint;
//using vendor::oplus::hardware::biometrics::fingerprint::V2_1::implementation::BiometricsFingerprint;
using ::android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::OK;
using android::Hypnus;

#define RETRY_TOTAL_TIMES 10
#define SLEEP_TIME 500000  //sleep 500ms

#define OPTICAL_FEATURE "persist.vendor.fingerprint.optical.support"
#define SEPARATE_SOFT_FEATURE "ro.separate.soft"
#define REGIONMARK_FEATURE "ro.vendor.oplus.regionmark"

int main() {
    ALOGE("fingerprint hwbinder service starting");
    char separate_soft[PROPERTY_VALUE_MAX] = {0};
    char regionmark[PROPERTY_VALUE_MAX] = {0};
    int retry_times = 0;
    oplus_fp_type_init();

    if (FP_GOODIX == fp_config_info_init.fp_factory_type) {
        ALOGE("goodix fingerprint");
        property_set(OPTICAL_FEATURE, "0");

#ifdef OPLUS_GOODIX_SUPPORT
        ALOGE("init fingerprint");
        property_set("oplus.goodix.fp", "1");
        configureRpcThreadpool(1, false); /*callerWillJoin*/
        android::sp<BiometricsFingerprint> bio =
            static_cast<BiometricsFingerprint *>(BiometricsFingerprint::getInstance());

        if (bio->init()) {
            ALOGE("init failed!");
            goto out;
        }
        if (bio != nullptr) {
            bio->registerAsService();
        } else {
            ALOGE("Can't create instance of BiometricsFingerprint, nullptr");
            return -1;
        }
        ALOGD("Add fp to ServiceManager ok");
#endif

    } else if (FP_FPC == fp_config_info_init.fp_factory_type) {
        ALOGE("fpc fingerprint");
        property_set(OPTICAL_FEATURE, "0");

#ifdef FPC_FP_ENABLE
        sp<fpc_hidl> hal = new fpc_hidl();
        int err = 0;

        if (hal == nullptr) {
            return -1;
        }
        configureRpcThreadpool(1, false);
        do {
            if (hal->init()) {
                ALOGE("init failed! has retry %d times", retry_times);
                retry_times++;
                if (retry_times <= RETRY_TOTAL_TIMES) {
                    usleep(SLEEP_TIME); //sleep 500ms
                    continue;
                } else {
                    goto out;
                }
            } else {
                break;
            }
        } while (retry_times <= RETRY_TOTAL_TIMES);

        err = hal->registerAsService("default");
        if (err != OK) {
            ALOGE("Cannot register fingerprint HAL service error=%d", err);
            return -1;
        }
        ALOGD("Add fp to ServiceManager ok");
#endif
    }  else  if (
            FP_OPTICAL_GOODIX_G2 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_G3 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_G3S == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_G5 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_G6 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_G7 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_19821_G5 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_19821_G6 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_19811_G5 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_19811_G6 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_19805_G6_3 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_19805_G6_7 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_G6_3 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_G6_7 == fp_config_info_init.fp_factory_type ||
            FP_OPTICAL_GOODIX_20801_G3 == fp_config_info_init.fp_factory_type) {
        ALOGE("optical goodix fingerprint");

#ifdef GOODIX_FP_ENABLE
        property_set(OPTICAL_FEATURE, "1");
        property_set("oplus.optical.goodix.fp", "1");
        set_goodix_icon_property();
#ifdef FP_CONFIG_SETTINGS_ENABLE
        fpSettingsInit();
#endif
        configureRpcThreadpool(1, false /*callerWillJoin*/);
        android::sp<OpticalFingerprint> bio =
            static_cast<OpticalFingerprint *>(OpticalFingerprint::getInstance());

        if (bio != nullptr) {
            bio->registerAsService();
        } else {
            ALOGE("Can't create instance of OpticalBiometricsFingerprint, nullptr");
            return -1;
        }
        ALOGD("Add fp to ServiceManager ok");
#endif
    }  else  if (FP_OPTICAL_SILEAD == fp_config_info_init.fp_factory_type) {
        ALOGE("silead fingerprint");
        property_get(SEPARATE_SOFT_FEATURE, separate_soft, "0");
        property_get(REGIONMARK_FEATURE, regionmark, "0");
        ALOGI("separate_soft =%s, regionmark = %s", separate_soft, regionmark);
        if (!strncmp(separate_soft, "18381", sizeof("18381")) && (!strncmp(regionmark, "SG", sizeof("SG")))) {
            ALOGI("18381-SG not set OPTICAL_FEATURE");
        } else {
            property_set(OPTICAL_FEATURE, "1");
            ALOGI("set OPTICAL_FEATURE to 1");
        }

#ifdef SILEAD_FP_ENABLE
        configureRpcThreadpool(1, false /*callerWillJoin*/);
        android::sp<SileadFingerprint> bio =
            static_cast<SileadFingerprint *>(SileadFingerprint::getInstance());

        if (bio != nullptr) {
            bio->registerAsService();
        } else {
            ALOGE("Can't create instance of OpticalBiometricsFingerprint, nullptr");
        }
        ALOGD("Add fp to ServiceManager ok");
        set_silead_icon_property();
#endif

#ifdef EGIS_FP_ENABLE
    } else if (FP_OPTICAL_EGIS == fp_config_info_init.fp_factory_type) {
        //TODO egis main
        ALOGD("egis fingerprint_hal");
        int retry = 3;
        sp<ets_hidl> fingerprint_hal = new ets_hidl();

        if (fingerprint_hal == nullptr) {
            ALOGE("Failed to create fingerprint_hal");
            return -1;
        }

        while (retry) {
            if (fingerprint_hal->open()) {
                usleep(500 * 1000);
                fingerprint_hal->close();
                retry--;
            } else {
                break;
            }
        }
        ALOGD("retry egis fingerprint_hal time = %d", retry);
        configureRpcThreadpool(1, true);

        if (retry > 0 && fingerprint_hal != nullptr) {
            fingerprint_hal->registerAsService("default");
        } else {
            ALOGE("Can't create fingerprint service");
            return -1;
        }
        set_egis_icon_property();
#endif

#ifdef EGIS_CAPACITY_FP_ENABLE
    } else if (FP_EGIS == fp_config_info_init.fp_factory_type) {
        ALOGD("EGISTEC hidl enter!");
        property_set(OPTICAL_FEATURE, "0");
        int retry = 3;
        int egis_err = 0;

        sp<egis_hidl> fingerprint_hal = new egis_hidl();
        int err = 0;

        if (fingerprint_hal == nullptr) {
            ALOGE("Failed to create fingerprint_hal");
            return -1;
        }

        while (retry) {
            if (fingerprint_hal->open()) {
                ALOGE("EGISTEC hidl fingerprint_hal->open() failed");
                usleep(500 * 1000);
                fingerprint_hal->close();
                retry--;
            } else {
                ALOGD("EGISTEC hidl EGISTEC hidl fingerprint_hal->open() success");
                break;
            }
        }
        configureRpcThreadpool(1, false);

        if (fingerprint_hal != nullptr) {
            err = fingerprint_hal->registerAsService("default");
            if (err != OK) {
                ALOGE("EGISTEC hidl Cannot register fingerprint HAL service error=%d", err);
                return -1;
            }
            ALOGD("Add fp to ServiceManager ok");
        } else {
            ALOGE("Can't create fingerprint service");
        }

#endif
    } else {
        ALOGE("fp_vendor : FP_READ_ERR");
        goto out;
    }

out:
#ifndef FP_BINDCORE_BYTID
    Hypnus::getInstance()->bind_big_core();
#endif
    joinRpcThreadpool();
    return 0;
}
