/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _SENSORCONFIGPROVIDER_H_
#define _SENSORCONFIGPROVIDER_H_

#include <stdint.h>
#include "gf_delmar_types.h"

#define MAX_PATH_LEN (256)

namespace goodix {

    typedef struct {
        int32_t sensorX[MAX_SENSOR_NUM];
        int32_t sensorY[MAX_SENSOR_NUM];
        int32_t radius;
        float radiusInMM;
        int32_t sensorRotation[MAX_SENSOR_NUM];
    } GoodixSensorConfig;  // NOLINT(402)

    class SensorConfigProvider {
    public:
        explicit SensorConfigProvider(char *configDir = nullptr);  // NOLINT(575)
        virtual ~SensorConfigProvider() {}
        virtual gf_error_t getConfig(GoodixSensorConfig *config, int32_t sensorNum);
        gf_error_t saveConfig(GoodixSensorConfig *config, int32_t sensorNum);
    private:
        char mpConfigDir[MAX_PATH_LEN];
    };
}  // namespace goodix

#endif  // _SENSORCONFIGPROVIDER_H_
