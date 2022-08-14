/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][CustomizedDelmarSensor]"
#include <cutils/properties.h>
#include "HalContext.h"
#include "CustomizedDelmarSensor.h"
#include "CustomizedDevice.h"
#include "gf_customized_sensor_types.h"
#ifndef TEMPRATURE_DEVICE_PATH
// battery
#define TEMPRATURE_DEVICE_PATH "/sys/class/thermal/thermal_zone74/temp"
#endif  // TEMPRATURE_DEVICE_PATH

namespace goodix {
    int32_t readTemperature();

    CustomizedDelmarSensor::CustomizedDelmarSensor(HalContext *context, SensorConfigProvider *provider): DelmarSensor(context, provider) {
    }

    CustomizedDelmarSensor::~CustomizedDelmarSensor() {
    }

    gf_error_t CustomizedDelmarSensor::customizedExposureTime() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            gf_delmar_set_exposure_time_t cmd = {{0}};
            cmd.header.cmd_id = GF_CMD_SENSOR_SET_EXPOSURE_TIME;
            cmd.header.target = GF_TARGET_SENSOR;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    /*terminal state. Indicates whether the device is root*/
    bool CustomizedDelmarSensor::isDeviceUnlocked(void) {
        bool res = false;
        int32_t ret = 0;
        char buf[PROPERTY_VALUE_MAX] = {0};
        ret = property_get("ro.boot.vbmeta.device_state", buf, NULL);
        LOG_E(LOG_TAG, "ro.boot.vbmeta.device_state: %s ret %d", buf, ret);
        if (0 == strcmp(buf, "unlocked")) {
            LOG_D(LOG_TAG, "Device is unlocked");
            res = true;
        } else {
            LOG_D(LOG_TAG, "Device is locked");
        }
        return res;
    }

    int32_t CustomizedDelmarSensor::detectTemperature() {
        int32_t fd = 0;
        char buffer[5] = {0};
        int32_t amt;
        int32_t temp = 0;
        fd = ::open(TEMPRATURE_DEVICE_PATH, O_RDONLY);
        if (fd >= 0) {
            amt = read(fd, buffer, 5);
            ::close(fd);
            if (-1 == amt) {
                temp = -1000;
                LOG_D(LOG_TAG, "[%s] read temperature device failed", __func__);
            } else {
                temp = atoi(buffer)/10;
                LOG_D(LOG_TAG, "[%s] temperature is %d", __func__, temp);
            }
        }
        return temp;
    }

    gf_error_t CustomizedDelmarSensor::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            setSensorIds(1);
            err = DelmarSensor::init();
            GF_ERROR_BREAK(err);
            //  customizedExposureTime();
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarSensor::readImage(uint32_t retry, uint64_t sensorIds) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_coordinate_info_t coordinate_info = {0};
        CustomizedDevice *local_devce = (CustomizedDevice*)mContext->mDevice;
        char value[128] = {0};

        FUNC_ENTER();

        if ((property_get("ro.boot.project_name", value, NULL) != 0) && (strcmp(value, "19811") == 0)) {
            coordinate_info.sensor_x[0] = 720;
            coordinate_info.sensor_y[0] = 2550;
            coordinate_info.radius = 162;
        } else {
            coordinate_info.sensor_x[0] = 540;
            coordinate_info.sensor_y[0] = 1910;
            coordinate_info.radius = 127;
        }
        coordinate_info.sensor_rotation[0] = 84;
        if (local_devce->tp_info.x != 0 && local_devce->tp_info.y != 0) {
            LOG_D(LOG_TAG, "[%s] calculateMask, tp_info_x=%d, tp_info_y=%d", __func__, local_devce->tp_info.x, local_devce->tp_info.y);
            coordinate_info.press_x = local_devce->tp_info.x;
            coordinate_info.press_y = local_devce->tp_info.y;
        } else {
            if ((property_get("ro.boot.project_name", value, NULL) != 0) && (strcmp(value, "19811") == 0)) {
                coordinate_info.press_x = 720;
                coordinate_info.press_y = 2550;
            } else {
                coordinate_info.press_x = 540;
                coordinate_info.press_y = 1910;
            }
        }
        LOG_D(LOG_TAG, "[%s] calculateMask, press_x = %d, press_y = %d", __func__, coordinate_info.press_x, coordinate_info.press_y);

        setCoordinateInfo(&coordinate_info);
        err = DelmarSensor::readImage(retry, sensorIds);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarSensor::wakeupSensor() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            err = DelmarSensor::wakeupSensor();
            if (GF_SUCCESS != err) {
                break;
            }
            //  readTemperature();
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
