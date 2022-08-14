/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HALCONTEXT_H_
#define _HALCONTEXT_H_

#include <utils/Vector.h>
#include "Mutex.h"
#include "gf_config_type.h"
#include "gf_error.h"
#include "MsgBus.h"
#include "gf_sensor_types.h"
#include "gf_global_define.h"
#include "Thread.h"

using ::android::Vector;

namespace goodix {
    class FingerprintCore;
    class Sensor;
    class Algo;
    class Device;
    class SensorDetector;
    class CaEntry;
    class EventCenter;
#ifdef SUPPORT_DSP_HAL
    class HalDsp;
#endif  // SUPPORT_DSP_HAL
    class ExtModuleBase;
    class InitFinishThread;
#ifdef SUPPORT_PERSIST_DATA_BACKUP
    class DataSynchronizer;
#endif  // SUPPORT_PERSIST_DATA_BACKUP

#ifdef SUPPORT_DUMP
    class HalDump;
#endif  // SUPPORT_DUMP
    HAL_CONTEXT_DECLARE_INJ;

    class HalContext {
    public:
        HalContext();
        virtual ~HalContext();
        virtual gf_error_t init();
        virtual gf_error_t deinit();
        virtual gf_error_t reInit();
        virtual bool isDSPEnabled();
        virtual bool isDSPValid();
        virtual uint8_t isDSPEnroll();
        gf_error_t invokeCommand(void *cmd, int32_t size);
        void createExtModules(void* callback);
        virtual gf_error_t onExtModuleCommand(int32_t cmdId, const int8_t *in, uint32_t inLen,
                             int8_t **out, uint32_t *outLen);

        FingerprintCore *mFingerprintCore;
        Sensor *mSensor;
        Algo *mAlgo;
        Device *mDevice;
        EventCenter *mCenter;
        CaEntry *mCaEntry;
        gf_sensor_info_t mSensorInfo;
        gf_config_t *mConfig;
#ifdef SUPPORT_PERSIST_DATA_BACKUP
        DataSynchronizer *mDataSynchronizer;
#endif  // SUPPORT_PERSIST_DATA_BACKUP
        Mutex mHalLock;
        Mutex mSensorLock;
        Mutex mWakeUpLock;
        MsgBus mMsgBus;
        Vector<ExtModuleBase*> mExtModuleList;
#ifdef SUPPORT_DUMP
        HalDump *mHalDump;
#endif  // SUPPORT_DUMP
        HAL_CONTEXT_DEFINE_INJ;
        InitFinishThread* mInitFinishThread;
#ifdef SUPPORT_DSP_HAL
        HalDsp* getDsp();
        virtual void createDsp();
        HalDsp *mDsp;
#endif  // SUPPORT_DSP_HAL

    private:
        void destroy();
        bool mInited;
    };

    class InitFinishThread : public Thread {
    public:
        explicit InitFinishThread(HalContext& context);
        virtual ~InitFinishThread();
    protected:
        virtual bool threadLoop();
        HalContext& mContext;
    };

}  // namespace goodix

#endif /* _HALCONTEXT_H_ */
