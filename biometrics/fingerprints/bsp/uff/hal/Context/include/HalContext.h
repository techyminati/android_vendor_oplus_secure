/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/HalContext.h
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
#ifndef _HALCONTEXT_H_
#define _HALCONTEXT_H_

#include <stdint.h>
#include <stddef.h>

#include "FpType.h"
#include "FpCommon.h"
#include "FingerprintManager.h"
#include "FingerprintMessage.h"
#include "Device.h"
#include "FpCa.h"
#include "DcsInfo.h"
#include "Dump.h"
#include "FactoryTest.h"
#include "FingerprintConfig.h"
#include "Perf.h"
#include "AutoSmoking.h"

namespace android {
class FingerprintManager;
class FingerprintMessage;
class Sensor;
class Algo;
class Device;
class FpCa;
class DcsInfo;
class Dump;
class FactoryTest;
class FingerprintConfig;
class Perf;
class AutoSmoking;

class HalContext {
   public:
    static HalContext* getInstance() { return sInstance; }
    HalContext();
    ~HalContext();

    fp_return_type_t init();
    fp_return_type_t deinit();
    fp_return_type_t reInit();
    fp_return_type_t         invokeCommand(void* cmd, int32_t size);
    void                     preChipSelected(unsigned int mdelay);
    fp_return_type_t         doChipSelected(fp_ta_name_t *name);
    FingerprintManager*      mFingerprintManager;
    FingerprintMessage*      mFingerprintMessage;
    Sensor*                  mSensor;
    Device*                  mDevice;
    Algo*                    mAlgo;
    FpCa*                    mCaEntry;
    DcsInfo*                 mDcsInfo;
    Dump*                    mDump;
    Perf*                    mPerf;
    FactoryTest *mFacotryTest;
    FingerprintConfig *mConfig;
    AutoSmoking *mAutoSmoking;

    static HalContext*       sInstance;
};
}  // namespace android

#endif /* _HALCONTEXT_H_ */
