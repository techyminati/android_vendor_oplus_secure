/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _SZSENSOR_H_
#define _SZSENSOR_H_

#include "Sensor.h"
#include "gf_sz_types.h"
#include "gf_sensor_types.h"

namespace goodix
{
    class SZSensor : public Sensor
    {
    public:
        explicit SZSensor(HalContext *context);
        virtual ~SZSensor();
        virtual gf_error_t init(gf_sensor_ids_t* ids);
        virtual gf_error_t sleepSensor();
        virtual gf_error_t wakeupSensor();
    private:
        bool mIsSensorSleep;
    };
}

#endif /* _SZSENSOR_H_ */

