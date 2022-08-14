/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#define LOG_TAG "[GF_HAL][CustomizedSensorConfigProvider]"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cutils/properties.h>
#include "HalUtils.h"
#include "HalLog.h"
#include "gf_base_types.h"
#include "CustomizedSensorConfigProvider.h"

// radius = diameter/2
#define PROPERTY_SENSOR_DIAMETER "persist.vendor.fingerprint.optical.iconsize"
// x0::y0::x1::y1::x2::y2::x3::y3
#define PROPERTY_SENSOR_LOCATION "persist.vendor.fingerprint.optical.sensorlocation"
#define DEFAULT_SENSOR_RADIUS (129)
#define DEFAULT_SENSOR_LOCATION "0::0::0::0::0::0::0::0"
#define PROPERTY_SENSOR_ROTATION "persist.vendor.fingerprint.optical.sensorrotation"
#define DEFAULT_SENSOR_ROTATION "0"
#define SENSOR_CFG_PROVIDER_PATH "/data/vendor_de/0/fpdata/"

namespace goodix {

    CustomizedSensorConfigProvider::CustomizedSensorConfigProvider() : SensorConfigProvider((char*) SENSOR_CFG_PROVIDER_PATH) {
    }

    gf_error_t CustomizedSensorConfigProvider::getConfig(GoodixSensorConfig *config, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        GoodixSensorConfig tmpConfig = {{0}};
        char propertyStr[PROPERTY_VALUE_MAX] = { 0 };
        int32_t ret = 0;
        int32_t i = 0;
        int8_t* value = NULL;
        FUNC_ENTER();

        do {
            if (config == NULL) {
                LOG_D(LOG_TAG, "[%s] config is null.", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            LOG_D(LOG_TAG, "[%s] sensorNum=%d", __func__, sensorNum);

            if (SensorConfigProvider::getConfig(&tmpConfig, sensorNum) == GF_SUCCESS) {
                LOG_D(LOG_TAG, "[%s] load sensor cfg from=%s", __func__, SENSOR_CFG_PROVIDER_PATH);
                for (i = 0; i < sensorNum; i++) {
                    config->sensorX[i] = tmpConfig.sensorX[i];
                    config->sensorY[i] = tmpConfig.sensorY[i];
                    config->sensorRotation[i] = tmpConfig.sensorRotation[i];
                }
                config->radiusInMM = tmpConfig.radiusInMM;
                config->radius = tmpConfig.radius;
            } else {
                ret = property_get_int32(PROPERTY_SENSOR_DIAMETER, -1);
                LOG_D(LOG_TAG, "[%s] get sensor diameter ret=%d.", __func__, ret);
                if (-1 == ret) {
                    config->radius = DEFAULT_SENSOR_RADIUS;
                } else {
                    config->radius = ret / 2;
                }

                ret = property_get(PROPERTY_SENSOR_LOCATION, (char*)propertyStr, DEFAULT_SENSOR_LOCATION);
                LOG_D(LOG_TAG, "[%s] get sensor location ret=%d, propertyStr=%s.",
                               __func__, ret, propertyStr);

                for (i = 0; i < sensorNum * 2; i++) {
                    value = (int8_t*) strtok(i == 0 ? (char*)propertyStr : NULL, "::");
                    if (value != NULL) {
                        if (i % 2 == 0) {
                            config->sensorX[i/2] = atoi((char*)value);
                        } else {
                            config->sensorY[i/2] = atoi((char*)value);
                        }
                    } else {
                        LOG_E(LOG_TAG, "[%s] parse sensor location error.", __func__);
                        err = GF_ERROR_BAD_PARAMS;
                        break;
                    }
                }

                ret = property_get(PROPERTY_SENSOR_ROTATION, (char*)propertyStr, DEFAULT_SENSOR_ROTATION);
                LOG_D(LOG_TAG, "[%s] get sensor rotation ret=%d, propertyStr=%s.",
                               __func__, ret, propertyStr);

                for (i = 0; i < sensorNum; i++) {
                    value = (int8_t*) strtok(i == 0 ? (char*)propertyStr : NULL, "::");
                    if (value != NULL) {
                        config->sensorRotation[i] = atof((char*)value);
                    } else {
                        LOG_E(LOG_TAG, "[%s] parse sensor location error.", __func__);
                        err = GF_ERROR_BAD_PARAMS;
                        break;
                    }
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
