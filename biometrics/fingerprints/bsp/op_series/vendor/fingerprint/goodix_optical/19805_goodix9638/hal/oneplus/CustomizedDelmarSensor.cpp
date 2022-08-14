/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][CustomizedDelmarSensor]"
#include <string.h>
#include <cutils/properties.h>
#include "HalContext.h"
#include "CustomizedDelmarSensor.h"
#include "CustomizedDevice.h"
#include "gf_customized_sensor_types.h"
#include "CustomizedFingerprintCore.h"
#include <iostream>
#include <fstream>
extern "C" {
#include "Fpsys.h"
}

#ifndef TEMPRATURE_DEVICE_PATH
// battery
#define TEMPRATURE_DEVICE_PATH "/sys/class/thermal/thermal_zone74/temp"
#endif  // TEMPRATURE_DEVICE_PATH

#define FP_QR_CODE_PATH "/mnt/vendor/persist/engineermode/fpqrcode"
using namespace std;

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
        LOG_D(LOG_TAG, "ro.boot.vbmeta.device_state: %s ret %d", buf, ret);
        if (0 == strcmp(buf, "unlocked")) {
            LOG_D(LOG_TAG, "Device is unlocked");
            res = true;
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

    gf_error_t CustomizedDelmarSensor::sensorGetQRCode(uint8_t *otp_qr_info, int8_t* qr_code) {
        gf_error_t err = GF_SUCCESS;
        char str[19];
        uint32_t m_code = 0;
        uint16_t date = 0;
        uint64_t status = 0;
        // Q9180695A3WX001BY4
        uint8_t *data = NULL;
        uint8_t full_table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
        uint8_t day[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X'};
        uint8_t status_table[] = {'O', 'X', 'G', 'P'};
        FUNC_ENTER();

        do {
            if (NULL == otp_qr_info || NULL == qr_code) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            memset(str, 0, sizeof(str));
            data = otp_qr_info;
            m_code = data[1] << 16 | data[2] << 8 | data[3];
            date = data[4] << 8 | data[5];
            status = data[6];
            status = status << 32;
            status = status | (uint32_t)(data[7] << 24) | data[8] << 16 | data[9] << 8 | data[10];

            snprintf(str, sizeof(str), "%c%07d%c%c%c%c%c%c%c%c%c%c", data[0], m_code, full_table[date >> 10 & 0x3f],
                        full_table[date >> 6 & 0x0f], day[date & 0x3f], status_table[status >> 36 & 0x0f],
                        full_table[status >> 30 & 0x3f], full_table[status >> 24 & 0x3f], full_table[status >> 18 & 0x3f],
                        full_table[status >> 12 & 0x3f], full_table[status >> 6 & 0x3f], full_table[status & 0x3f]);
            memcpy(qr_code, str, sizeof(str));
            // LOG_D(LOG_TAG, "[%s] module QR code: %s", __func__, qr_code);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarSensor::customizedGetQrCode() {
        gf_error_t err = GF_SUCCESS;
        int8_t qr_code[19];
        FUNC_ENTER();

        do {
            gf_delmar_get_qr_code_t cmd = {{0}};
            cmd.header.cmd_id = GF_CMD_SENSOR_GET_QR_CODE;
            cmd.header.target = GF_TARGET_SENSOR;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            sensorGetQRCode(cmd.otp_qr_info, qr_code);
            ofstream fout(FP_QR_CODE_PATH);      //creat fpqrcode file
            if ( ! fout){
               LOG_D(LOG_TAG, "[%s] can not open file", __func__);
            } else {
               // stream output qr_code to disk file (fout)
               fout << qr_code << endl;
            }
            fout.close();
            LOG_D(LOG_TAG, "[%s] module QR code: %s", __func__, qr_code);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarSensor::checkSensor() {
        int32_t sensor_type_from_path = 0;
        gf_error_t ret = GF_SUCCESS;
        if (getOpticalType() == DELMAR_OPTICAL_TYPE_7_0) {
            mSensorType = 7;
        } else if(getOpticalType() == DELMAR_OPTICAL_TYPE_3_0) {
            mSensorType = 3;
        } else {
            LOG_E(LOG_TAG, "[%s] Ooops!!! optic_type(%d) is invaild!", __func__, getOpticalType());
            isSensorVaild = -1;
            return ret;
        }
        sensor_type_from_path =  fp_get_sensor_type_from_path();
        LOG_D(LOG_TAG, "[%s] current sensor_info = G6_%d sensor_type_from_path = 0x%x", __func__, mSensorType, sensor_type_from_path);

        if (sensor_type_from_path < 0  || ((sensor_type_from_path >> 4) & 0xF) != mSensorType) {
            LOG_E(LOG_TAG, "[%s] Ooops!!! Fingerprint pin id(0x%x) can't match sensorType(%d)!", __func__, sensor_type_from_path, mSensorType);
            isSensorVaild = -2;
        }
        isSensorVaild = 0;
        return ret;
    }

    gf_error_t CustomizedDelmarSensor::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            setSensorIds(1);
            err = DelmarSensor::init();
            GF_ERROR_BREAK(err);
            err = checkSensor();
            GF_ERROR_BREAK(err);
            customizedGetQrCode();
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
        coordinate_info.sensor_rotation[0] = 90;
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
