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

#include "HalContext.h"
#include "SZSensor.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "FingerprintCore.h"
#include "Device.h"

namespace goodix
{

    SZSensor::SZSensor(HalContext *context) : Sensor(context), mIsSensorSleep(false)
    {
    }

    SZSensor::~SZSensor()
    {
    }

    gf_error_t SZSensor::init(gf_sensor_ids_t* ids)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            gf_sz_sensor_init_t cmd;
            memset(&cmd, 0, sizeof(gf_sz_sensor_init_t));
            gf_cmd_header_t *header = (gf_cmd_header_t *)&cmd;
            header->target = GF_TARGET_SENSOR;
            header->cmd_id = GF_CMD_SENSOR_INIT;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            gf_sz_config_t *config = new gf_sz_config_t();

            if (config == nullptr)
            {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memcpy(config, &cmd.o_shenzhen_config, sizeof(gf_sz_config_t));
            mContext->mConfig = (gf_config_t *)config;

            memcpy(ids, &(cmd.o_sensor_ids), sizeof(gf_sensor_ids_t));

            LOG_D(LOG_TAG, "[%s] sensor_id = 0x%x", __func__, ids->sensor_id);
            LOG_D(LOG_TAG, "[%s] sensor_version = 0x%x", __func__, ids->sensor_version);
            LOG_I(LOG_TAG, "[%s] vendor_id = 0x%x", __func__, cmd.o_flash_vendor_id);
            LOG_E(LOG_TAG, "[%s] module_type = 0x%x", __func__, cmd.o_module_type);
            LOG_E(LOG_TAG, "[%s] lens_type = 0x%x", __func__, cmd.o_lens_type);
            mModuleType = cmd.o_module_type;
            mLenseType = cmd.o_lens_type;
            // disable irq when booting
            mContext->mDevice->disableIrq();
            mContext->mDevice->enablePower();
            usleep(10 * 1000); // according to GM11 timing sequence
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZSensor::wakeupSensor()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (mIsSensorSleep)
        {
            gf_cmd_header_t cmd;
            memset(&cmd, 0, sizeof(gf_cmd_header_t));

            cmd.target = GF_TARGET_SENSOR;
            cmd.cmd_id = GF_SZ_CMD_RESET;

            err = invokeCommand(&cmd, sizeof(cmd));
            // mContext->mDevice->enablePower();
            // usleep(10 * 1000); // according to GM11 timing sequence
            err = mContext->mDevice->reset();

            LOG_D(LOG_TAG, "[%s] ## wakeup ## ", __func__);

            mIsSensorSleep = false;
        }
        else
        {
            LOG_D(LOG_TAG, "[%s] sensor has alreadly been waked up ", __func__);
        }

        FUNC_EXIT(err);
        return err;
    }
    gf_error_t SZSensor::sleepSensor()
    {
        gf_cmd_header_t cmd;
        memset(&cmd, 0, sizeof(gf_cmd_header_t));
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (!mIsSensorSleep)
        {
            cmd.target = GF_TARGET_SENSOR;
            cmd.cmd_id = GF_SZ_CMD_SLEEP;
            err = invokeCommand(&cmd, sizeof(cmd));

            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] sleep invoke command failed ", __func__);
            }

            LOG_D(LOG_TAG, "[%s] ## sleep ## ", __func__);
            // err = mContext->mDevice->disablePower();
            mIsSensorSleep = true;
        }
        else
        {
            LOG_D(LOG_TAG, "[%s] sensor has alreadly been sleep", __func__);
        }

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
