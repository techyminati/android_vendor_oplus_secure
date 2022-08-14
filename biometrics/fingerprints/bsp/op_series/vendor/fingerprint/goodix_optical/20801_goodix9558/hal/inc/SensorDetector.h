/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _SENSORDETECTOR_H_
#define _SENSORDETECTOR_H_
#include "gf_config_type.h"
#include "gf_error.h"
#include "HalBase.h"
#include "gf_sensor_types.h"

namespace goodix {
    class SensorDetector : public HalBase {
    public:
        explicit SensorDetector(HalContext *context);
        gf_error_t init();
        gf_error_t detectSensor(gf_sensor_info_t *info);
    };
}  // namespace goodix

#endif /* _SENSORDETECTOR_H_ */
