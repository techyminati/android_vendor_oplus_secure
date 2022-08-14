/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HALCONTEXT_H_
#define _HALCONTEXT_H_

#include "Mutex.h"
#include "gf_config_type.h"
#include "gf_error.h"
#include "MsgBus.h"
#include "gf_sensor_types.h"

#define MODULE_INFO_STR_MAX_LENGTH (20)

typedef struct {
    const uint32_t module;
    const uint32_t lens;
    const char module_info_string[MODULE_INFO_STR_MAX_LENGTH];
} gf_module_info_t;

namespace goodix
{
    class EventCenter;
    class FingerprintCore;
    class Sensor;
    class Algo;
    class Device;
    class SensorDetector;
    class CaEntry;
    class EventCenter;
    class HalDsp;
	class ProductTest;
#ifdef SUPPORT_DUMP
    class HalDump;
#endif  // SUPPORT_DUMP

    class HalContext
    {
    public:
        HalContext();
        virtual ~HalContext();

        virtual gf_error_t init();
        virtual gf_error_t deinit();
        virtual gf_error_t reInit();
        int setModuleInfo();
        FingerprintCore* mFingerprintCore;
        Sensor* mSensor;
        Algo* mAlgo;
        Device* mDevice;
        EventCenter* mCenter;
        CaEntry* mCaEntry;
        gf_sensor_info_t mSensorInfo;
        gf_config_t* mConfig;
        gf_sensor_ids_t mSensorId;
        Mutex mHalLock;
        Mutex mSensorLock;
        MsgBus mMsgBus;
        HalDsp *mDsp;
        ProductTest* mProductTest;
#ifdef SUPPORT_DUMP
        HalDump* mHalDump;
#endif  // SUPPORT_DUMP
#ifdef DYNAMIC_SUPPORT_DSP
        gf_error_t checkDspSupport(bool *mDspSupport);
#endif   //DYNAMIC_SUPPORT_DSP

    private:
        void destroy();
    };
}  // namespace goodix

#endif /* _HALCONTEXT_H_ */
