/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _SENSOR_H_
#define _SENSOR_H_
#include "gf_error.h"
#include "HalBase.h"
#include "gf_event.h"
#include "gf_sensor_types.h"

namespace goodix
{
    class Sensor: public HalBase
    {
    public:
        explicit Sensor(HalContext* context);
        virtual ~Sensor();
        virtual gf_error_t init(gf_sensor_ids_t* ids);
        virtual gf_error_t captureImage(int32_t op, uint32_t retry);
        virtual gf_event_type_t getIrqEventType();
        virtual gf_error_t sleepSensor();
        virtual gf_error_t wakeupSensor();
   public:
        int mModuleType;
        int mLenseType;
    };
}  // namespace goodix

#endif /* _SENSOR_H_ */
