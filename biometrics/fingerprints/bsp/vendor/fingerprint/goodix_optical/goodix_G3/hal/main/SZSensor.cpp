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

#define PROPERTY_SCREEN_TYPE "persist.vendor.fingerprint.optical.lcdtype"
#define PROPERTY_FINGERPRINT_QRCODE "oplus.fingerprint.qrcode.support"
#define PROPERTY_FINGERPRINT_QRCODE_VALUE "oplus.fingerprint.qrcode.value"

namespace goodix
{

    SZSensor::SZSensor(HalContext *context) : Sensor(context)
    {
        mContext->mMsgBus.addMsgListener(this);
    }

    SZSensor::~SZSensor()
    {
        mContext->mMsgBus.removeMsgListener(this);
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
            if (checkScreenType((char *)"SDC")) {
                cmd.screen_id = SCREEN_TYPE_ID_SDC;
            } else if (checkScreenType((char *)"BOE")) {
                cmd.screen_id = SCREEN_TYPE_ID_BOE;
            } else if (checkScreenType((char *)"TM")) {
                cmd.screen_id = SCREEN_TYPE_ID_TM;
            } else if (checkScreenType((char *)"LTPO")) {
                cmd.screen_id = SCREEN_TYPE_ID_LTPO;
            } else {
                cmd.screen_id = 0;
                LOG_D(LOG_TAG, "[%s] screen type property not set", __func__);
            }
#ifdef FP_ENABLE_UNLOCK_CALBRATION
            if(!property_get_bool("ro.boot.flash.locked", 1)){
                cmd.o_shenzhen_config.oem_unlock_state = 1;
                LOG_E(LOG_TAG, "[%s] enter to unlock", __func__);
            } else {
                cmd.o_shenzhen_config.oem_unlock_state = 1;
                LOG_E(LOG_TAG, "[%s] enter to locked", __func__);
            }
            LOG_D(LOG_TAG, "ro.boot.flash.locked: %u", cmd.o_shenzhen_config.oem_unlock_state);
#endif
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            gf_sz_config_t *config = new gf_sz_config_t();

            if (config == nullptr)
            {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            LOG_E(LOG_TAG, "[%s] gainvalue: %d/100", __func__, cmd.gainvalue);
            LOG_E(LOG_TAG, "[%s] expotime %d", __func__, cmd.expotime);

            memcpy(config, &cmd.o_shenzhen_config, sizeof(gf_sz_config_t));
            mContext->mConfig = (gf_config_t *)config;

            memcpy(ids, &(cmd.o_sensor_ids), sizeof(gf_sensor_ids_t));

            memset(mQrCode, 0, MAX_QR_CODE_INFO_LEN);
            memcpy(mQrCode, cmd.o_sensor_ids.qr_code, sizeof(cmd.o_sensor_ids.qr_code));
            LOG_E(LOG_TAG, "[%s] @@@@@ mQRCode=%s,len=%d", __func__, ids->qr_code,strlen((const char *)ids->qr_code));
            property_set(PROPERTY_FINGERPRINT_QRCODE_VALUE, (const char *)ids->qr_code);
            if(strlen((const char *)ids->qr_code)!=0)
                property_set(PROPERTY_FINGERPRINT_QRCODE, "1");
            LOG_D(LOG_TAG, "[%s] sensor_id = 0x%x", __func__, ids->sensor_id);
            LOG_D(LOG_TAG, "[%s] sensor_version = 0x%x", __func__, ids->sensor_version);
            LOG_I(LOG_TAG, "[%s] vendor_id = 0x%x", __func__, cmd.o_flash_vendor_id);
            LOG_E(LOG_TAG, "[%s] module_type = 0x%x", __func__, cmd.o_module_type);
            LOG_E(LOG_TAG, "[%s] lens_type = 0x%x", __func__, cmd.o_lens_type);
			LOG_D(LOG_TAG, "[%s] DR code = %s", __func__, ids->qr_code);
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
        err = Sensor::wakeupSensor();
        FUNC_EXIT(err);
        return err;
    }
    gf_error_t SZSensor::sleepSensor()
    {
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

    gf_error_t SZSensor::onMessage(const MsgBus::Message &msg)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        switch (msg.msg)
        {
            case MsgBus::MSG_HARDWARE_RESET:
            {
                gf_cmd_header_t cmd = { 0 };
                cmd.target = GF_TARGET_SENSOR;
                cmd.cmd_id = GF_SZ_CMD_RESET;
                err = invokeCommand(&cmd, sizeof(cmd));
                break;
            }

            default:
            {
                LOG_D(LOG_TAG, "msg : %d", msg.msg);
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    bool SZSensor::checkScreenType(char *type) {
        bool ret = false;
        char value[PROPERTY_VALUE_MAX] = { 0 };
        int len = 0;
        do {
            if (NULL == type) {
                break;
            }
            len = property_get(PROPERTY_SCREEN_TYPE, value, NULL);
            if (len <= 0) {
                LOG_D(LOG_TAG, "[%s] property not set.", __func__);
                break;
            }
            LOG_D(LOG_TAG, "[%s] in type=%s system type=%s.", __func__, type, value);
            if (!strncmp(type, value, strlen(type))) {
                ret = true;
            }
        } while (0);

        LOG_D(LOG_TAG, "[%s] ret=%d.", __func__, ret);
        return ret;
    }
}  // namespace goodix
