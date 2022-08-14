/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _SENSOR_H_
#define _SENSOR_H_
#include <semaphore.h>
#include "gf_error.h"
#include "HalBase.h"
#include "gf_event.h"
#include "gf_config_type.h"

namespace goodix {
    class Sensor: public HalBase, public MsgBus::IMsgListener{
    public:
        explicit Sensor(HalContext *context);
        virtual ~Sensor();
        virtual gf_error_t init();
        virtual gf_error_t captureImage(int32_t op, uint32_t retry);
        virtual gf_event_type_t getIrqEventType();
        virtual gf_error_t sleepSensor();
        virtual gf_error_t wakeupSensor();
        virtual void setSensorUIReady();
        virtual void clearSensorUIReady();
        virtual bool waitSensorUIReady();
        virtual bool waitSensorUIDisappear();
        virtual bool isSensorUIReady();
        virtual gf_error_t setStudyDisable(int32_t disable);
        virtual uint8_t getLogoTimes();
        virtual int32_t detectTemperature();
        static void resetWakeupFlag();
        static gf_error_t doWakeup(HalContext* halContext, bool* needReset = NULL);  // NOLINT(575)
        static gf_error_t doSleep(HalContext* halContext);
        // override MsgBus::IMsgListener
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        typedef enum {
            INIT,
            SLEEP,
            WAKEUP
        } SENSOR_STATUS;

    protected:
        volatile bool mIsSensorUIReady;
        static SENSOR_STATUS mSensorSleep;
        sem_t mUiReadySem;
    };
}  // namespace goodix

#endif /* _SENSOR_H_ */
