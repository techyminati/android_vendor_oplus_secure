/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _CUSTOMIZEDDELMARSENSOR_H_
#define _CUSTOMIZEDDELMARSENSOR_H_

#include "DelmarSensor.h"
#include "HalLog.h"

namespace goodix {

    class CustomizedDelmarSensor : public DelmarSensor {
    public:
        explicit CustomizedDelmarSensor(HalContext *context, SensorConfigProvider *provider = nullptr);  // NOLINT(575)
        virtual ~CustomizedDelmarSensor();
        gf_error_t customizedExposureTime();
        gf_error_t customizedGetSpmtPassFlag();
        virtual gf_error_t init();
        gf_error_t readImage(uint32_t retry, uint64_t sensorIds);
        gf_error_t wakeupSensor();
        virtual int32_t detectTemperature();

    protected:
        virtual bool isDeviceUnlocked(void);
    };
}  // namespace goodix

#endif /* _CUSTOMIZEDDELMARSENSOR_H_ */
