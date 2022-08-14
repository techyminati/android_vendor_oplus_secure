/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][Sensor]"

#include "HalContext.h"
#include "Sensor.h"
#include "HalLog.h"
#include "gf_sensor_types.h"
#include "FingerprintCore.h"
#include "Device.h"

namespace goodix
{
    Sensor::SENSOR_STATUS Sensor::mSensorSleep = INIT;
    Sensor::Sensor(HalContext* context) : HalBase(context),mModuleType(-1),mLenseType(-1)
    {
    }

    Sensor::~Sensor()
    {
    }

    gf_error_t Sensor::init(gf_sensor_ids_t* ids)
    {
        gf_error_t err = GF_SUCCESS;
        do
        {
            gf_sensor_init_t cmd;
            memset(&cmd, 0, sizeof(gf_sensor_init_t));
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_INIT;
            err = invokeCommand(&cmd, sizeof(gf_sensor_init_t));
            GF_ERROR_BREAK(err);
            gf_config_t* config = new gf_config_t();
            if (config == nullptr)
            {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            memcpy(config, &cmd.o_config, sizeof(gf_config_t));
            mContext->mConfig = (gf_config_t*) config;
            memcpy(ids, &(cmd.o_sensor_ids), sizeof(gf_sensor_ids_t));
        }
        while (0);
        return err;
    }

    gf_error_t Sensor::captureImage(int32_t op, uint32_t retry)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            gf_capture_image_t cmd;
            memset(&cmd, 0, sizeof(gf_capture_image_t));
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_CAPTURE_IMG;
            cmd.i_operation = op;
            cmd.i_retry_count = retry;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            if ((mContext->mCenter->hasUpEvt()) && (retry == 0))
            {
                err = GF_ERROR_TOO_FAST;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_event_type_t Sensor::getIrqEventType()
    {
        gf_event_type_t event = EVENT_UNKNOWN;
        gf_error_t err = GF_SUCCESS;
        bool openFlag = false;
        FUNC_ENTER();
        do {
            if (mSensorSleep == SLEEP) {
                err = wakeupSensor();
                if (err != GF_SUCCESS) {
                    LOG_E(LOG_TAG, "[%s] wake sensor fail", __func__);
                    break;
                }
                openFlag = true;
            }
            gf_get_irq_type_cmd_t cmd;
            memset(&cmd, 0, sizeof(gf_get_irq_type_cmd_t));
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_GET_IRQ_TYPE;
            err = invokeCommand(&cmd, sizeof(gf_get_irq_type_cmd_t));
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] retrieve irq type fail", __func__);
                break;
            }
            event = (gf_event_type_t)cmd.o_irq_type;
        } while (0);

        if (openFlag == true) {
            err = sleepSensor();
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] sleep sensor fail", __func__);
            }
        }
        while (0);
        FUNC_EXIT(err);
        return event;
    }
    gf_error_t Sensor::sleepSensor()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            Mutex::Autolock _l(mContext->mWakeUpLock);
            err = doSleep(mContext);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
    gf_error_t Sensor::wakeupSensor()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            Mutex::Autolock _l(mContext->mWakeUpLock);
            bool needReset = 0;
            err = doWakeup(mContext, &needReset);
            if (needReset) {
                err = mContext->mDevice->reset();
            }
            if (err != GF_SUCCESS)
            {
                break;
            }
        }
        while (0);
        FUNC_EXIT(err);
        return err;
    }
    void Sensor::resetWakeupFlag()
    {
        mSensorSleep = INIT;
    }

    gf_error_t Sensor::doWakeup(HalContext* halContext, bool* needReset) {
        gf_error_t err = GF_SUCCESS;
        gf_sensor_wakeup_t cmd = {{ 0 }};
        FUNC_ENTER();
        if (mSensorSleep != WAKEUP) {
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SNESOR_WAKEUP;
            err = halContext->invokeCommand(&cmd, sizeof(gf_sensor_wakeup_t));
            if (err == GF_SUCCESS) {
                mSensorSleep = WAKEUP;
                if (needReset) {
                    *needReset = cmd.o_need_reset;
                    LOG_D(LOG_TAG, "[%s] ### wakeup ###", __func__);
                }
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Sensor::doSleep(HalContext* halContext) {
        gf_error_t err = GF_SUCCESS;
        gf_sensor_sleep_t cmd = { 0 };
        FUNC_ENTER();
        if (mSensorSleep != SLEEP) {
            cmd.target = GF_TARGET_SENSOR;
            cmd.cmd_id = GF_CMD_SNESOR_SLEEP;
            err = halContext->invokeCommand(&cmd, sizeof(gf_sensor_sleep_t));
            if (err == GF_SUCCESS) {
                mSensorSleep = SLEEP;
                LOG_D(LOG_TAG, "[%s] ### sleep ###", __func__);
            }
        }
        FUNC_EXIT(err);
        return err;
    }
}   // namespace goodix
