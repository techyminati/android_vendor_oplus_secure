/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][SensorDetector]"

#include <string.h>
#include "SensorDetector.h"
#include "CaEntry.h"
#include "gf_sensor_types.h"
#include "HalLog.h"
#include "HalContext.h"
#include "Device.h"

#define SENSOR_DETECT_FAILED_RETRY_TIMES  (3)
#define SENSOR_INIT_FAILED_RETRY_TIMES  (3)

namespace goodix {

    SensorDetector::SensorDetector(HalContext *context) :
            HalBase(context) {
    }

    gf_error_t SensorDetector::init() {
        gf_error_t err = GF_SUCCESS;
        gf_sensor_preinit_t cmd = { 0 };
        FUNC_ENTER();
        cmd.target = GF_TARGET_SENSOR;
        cmd.cmd_id = GF_CMD_SENSOR_PRE_DETECT;
        int32_t i = 0;
        err = invokeCommand(&cmd, sizeof(gf_sensor_preinit_t));

        while (GF_ERROR_FAILED_AND_RETRY == err && i++ < SENSOR_INIT_FAILED_RETRY_TIMES) {
            LOG_E(LOG_TAG, "[%s] sensor init retry times = %d", __func__, i);
            mContext->mDevice->reset();
            err = invokeCommand(&cmd, sizeof(gf_sensor_preinit_t));
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SensorDetector::detectSensor(gf_sensor_info_t *info) {
        gf_error_t err = GF_SUCCESS;
        gf_detect_sensor_cmd_t cmd = { { 0 } };
        FUNC_ENTER();

#ifdef SUPPORT_LOW_SPEED_MODE_DETECT_SENSOR
        mContext->mDevice->setSpiSpeed(GF_SPI_SPEED_LOW);
#endif  // #ifdef SUPPORT_LOW_SPEED_MODE_DETECT_SENSOR
        do {
            int32_t i = 0;
            GF_NULL_BREAK(info, err);
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_DETECT_SENSOR;
            err = invokeCommand(&cmd, sizeof(gf_detect_sensor_cmd_t));
            while (GF_ERROR_FAILED_AND_RETRY == err && i++ < SENSOR_DETECT_FAILED_RETRY_TIMES) {
                LOG_E(LOG_TAG, "[%s] sensor detector retry times = %d", __func__, i);
                mContext->mDevice->reset();
                err = invokeCommand(&cmd, sizeof(gf_detect_sensor_cmd_t));
            }
            GF_ERROR_BREAK(err);
            memcpy(info, &(cmd.o_sensor_info), sizeof(gf_sensor_info_t));
        }
        while (0);
#ifdef SUPPORT_LOW_SPEED_MODE_DETECT_SENSOR
        mContext->mDevice->reset();
        mContext->mDevice->setSpiSpeed(GF_SPI_SPEED_HIGH);
#endif  // #ifdef SUPPORT_LOW_SPEED_MODE_DETECT_SENSOR
        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
