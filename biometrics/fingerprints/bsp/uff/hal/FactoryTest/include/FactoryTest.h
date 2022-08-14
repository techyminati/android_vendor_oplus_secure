/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/FactoryTest.h
 **
 ** Description:
 **      FactoryTest for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef FACTORY_TEST_H
#define FACTORY_TEST_H

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include "FpCommon.h"
#include "HalContext.h"
#include "VndCode.h"

namespace android {
class FactoryTest {
public:
    static FactoryTest *getInstance() {
        if (sInstance == NULL) {
            sInstance = new FactoryTest();
        }
        return sInstance;
    }
    int init(HalContext *context);
    void setHbmMode(uint32_t val);
    void setBrightness(uint32_t val);
    void getBrightnessValue();
    fp_return_type_t sendFingerprintCmd(int32_t cmd_id, int8_t *in_buf, uint32_t size);

private:
    HalContext *mContext;
    fp_return_type_t notifyFactoryResult(int32_t cmd_id, int result, char *buffer);
    fp_return_type_t sendFingerprintCmdToTA(int32_t cmd_id, vnd_code_t *vnd_code_p);
    FactoryTest();
    virtual ~FactoryTest();
    static FactoryTest *sInstance;
    char mBrightValue[50];
    void storeLcdBrightness();
    void recoveryLcdState();

public:
    std::string gBrightnessPath[2] = {
    "/sys/class/leds/lcd-backlight/brightness",
    "/sys/class/backlight/panel0-backlight/brightness",
    };
    std::string gHbmPath[2] = {
        "/sys/kernel/oplus_display/hbm",
        "/sys/devices/virtual/mtk_disp_mgr/mtk_disp_mgr/LCM_HBM",
    };
};
}  // namespace android

#endif  // _FACTORYTEST_H_
