/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][ShenzhenSensor]"

#include <stdlib.h>
#include <string.h>
#include <cutils/fs.h>
#include <cutils/properties.h>

#include "HalContext.h"
#include "SZSensor.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "FingerprintCore.h"
#include "Device.h"

namespace goodix {

    SZSensor::SZSensor(HalContext *context) : Sensor(context), mExpoTimetLevel(1) {
    }

    SZSensor::~SZSensor() {
    }

    gf_error_t SZSensor::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            gf_sz_sensor_init_t cmd = {{ 0 }};
            gf_cmd_header_t *header = (gf_cmd_header_t *)&cmd;
            header->target = GF_TARGET_SENSOR;
            header->cmd_id = GF_CMD_SENSOR_INIT;
            char buf[PROPERTY_VALUE_MAX] = {0};
            int ret = GF_SUCCESS;
            ret = property_get("vendor.boot.verifiedbootstate", buf, NULL);
            LOG_V(LOG_TAG, "verifiedbootstate:%s,ret=%d", buf, ret);
            if(GF_SUCCESS == strcmp(buf, "orange")) {
                cmd.o_shenzhen_config.oem_unlock_state = 1;
            } else {
                cmd.o_shenzhen_config.oem_unlock_state = 0;
            }
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            gf_sz_config_t *config = new gf_sz_config_t();

            if (config == nullptr) {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memcpy(config, &cmd.o_shenzhen_config, sizeof(gf_sz_config_t));
            mContext->mConfig = (gf_config_t *)config;
            // disable irq when booting
            mContext->mDevice->disableIrq();
            mContext->mDevice->enablePower();
            usleep(10 * 1000);  // according to GM11 timing sequence
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZSensor::wakeupSensor() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = Sensor::wakeupSensor();
        FUNC_EXIT(err);
        return err;
    }
    gf_error_t SZSensor::sleepSensor() {
        gf_error_t err = GF_SUCCESS;
        uint32_t retryCount = 2;
        FUNC_ENTER();
        err = Sensor::sleepSensor();
        while ((err != GF_SUCCESS)&&(retryCount > 0)) {
            mSensorSleep = SLEEP;
            retryCount--;
            LOG_D(LOG_TAG, "[%s] sleep failed, retry = %d", __func__, retryCount);
            Sensor::wakeupSensor();
            err = Sensor::sleepSensor();  // after wake up, retry sleep again
        }
        mSensorSleep = SLEEP;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZSensor::setStudyDisable(int32_t disable) {
        gf_error_t err = GF_SUCCESS;
        gf_sz_set_config_cmd_t cmd = {{0}};
        gf_sensor_set_config_t *header = NULL;
        gf_sz_config_t * config = NULL;
        FUNC_ENTER();
        do {
            config = (gf_sz_config_t *) mContext->mConfig;
            config->study_disable = disable;
            header = (gf_sensor_set_config_t *) &cmd;
            header->target = GF_TARGET_SENSOR;
            header->cmd_id = GF_CMD_SENSOR_SET_CONFIG;
            cmd.i_config = *config;
            err = invokeCommand(&cmd, sizeof(cmd));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void SZSensor::setMulExpoLevel(uint32_t level, uint32_t force) {
        if ((1 == force) || (SETTING_CUSTOMER_EXPO != mExpoTimetLevel)) {
            mExpoTimetLevel = level;
        }
    }

    gf_error_t SZSensor::setExpoTimetLevel() {
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mContext->mHalLock);
        gf_cmd_header_t cmd = { 0 };
        FUNC_ENTER();

        do {
            LOG_D(LOG_TAG, "[%s] expo level  %d", __func__, mExpoTimetLevel);
            cmd.target = GF_TARGET_SENSOR;
            cmd.cmd_id = GF_SZ_CMD_SET_EXPO;
            cmd.reserved[0] = mExpoTimetLevel;
            err = invokeCommand(&cmd, sizeof(cmd));

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] Set expo time level failed", __func__);
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZSensor::onMessage(const MsgBus::Message &msg) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        switch (msg.msg) {
            case MsgBus::MSG_HARDWARE_RESET: {
                gf_cmd_header_t cmd = { 0 };
                cmd.target = GF_TARGET_SENSOR;
                cmd.cmd_id = GF_SZ_CMD_RESET;
                err = invokeCommand(&cmd, sizeof(cmd));
                break;
            }

            default: {
                LOG_D(LOG_TAG, "msg : %d", msg.msg);
                err = Sensor::onMessage(msg);
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
