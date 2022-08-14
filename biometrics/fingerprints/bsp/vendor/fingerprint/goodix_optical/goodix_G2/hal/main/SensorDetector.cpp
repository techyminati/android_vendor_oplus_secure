 /************************************************************************************
 ** File: - Device.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      sensor detect class
 **
 ** Version: 1.0
 ** Date : 18:03:11,04/10/2018
 ** Author: wudongnan@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  Dongnan.Wu   2019/04/08     get different spi speed according to different pcb version
 ************************************************************************************/
#define LOG_TAG "[GF_HAL][SensorDetector]"

#include <string.h>
#include "SensorDetector.h"
#include "CaEntry.h"
#include "gf_sensor_types.h"
#include "HalLog.h"
#include "HalContext.h"
#include "Device.h"

namespace goodix
{

    SensorDetector::SensorDetector(HalContext* context) :
            HalBase(context)
    {
    }

    gf_error_t SensorDetector::init()
    {
        gf_error_t err = GF_SUCCESS;
        gf_sensor_preinit_t cmd;
        memset(&cmd, 0, sizeof(gf_sensor_preinit_t));
        int32_t i = 0;
#ifdef __TRUSTONIC
        uint16_t oplus_pcb_ver = 0;
#endif
        FUNC_ENTER();

#ifdef __TRUSTONIC
        err = mContext->mDevice->get_pcb_version(&oplus_pcb_ver);
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] get pcb version fail \n", __func__);
        } else {
            LOG_I(LOG_TAG, "[%s] oplus pcb version is %d \n", __func__, oplus_pcb_ver);
        }
#endif
        cmd.target = GF_TARGET_SENSOR;
        cmd.cmd_id = GF_CMD_SENSOR_PRE_DETECT;
#ifdef __TRUSTONIC
        cmd.reserved[0] = oplus_pcb_ver;
#endif
        err = invokeCommand(&cmd, sizeof(gf_sensor_preinit_t));

        if (GF_ERROR_PRE_DETECT_FAILED_AND_RETRY == err)
        {
            do
            {
                i++;
                LOG_E(LOG_TAG, "[%s] sensor detctor init retry times = %d", __func__, i);
                mContext->mDevice->reset();
                err = invokeCommand(&cmd, sizeof(gf_sensor_preinit_t));
            }while ((i < SENSOR_INIT_FAILED_RETRY_TIMES)&&(GF_ERROR_PRE_DETECT_FAILED_AND_RETRY == err));
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SensorDetector::detectSensor(gf_sensor_info_t* info)
    {
        gf_error_t err = GF_SUCCESS;
        gf_detect_sensor_cmd_t cmd;
        memset(&cmd, 0, sizeof(gf_detect_sensor_cmd_t));
        FUNC_ENTER();
        do
        {
            GF_NULL_BREAK(info, err);
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_DETECT_SENSOR;
            err = invokeCommand(&cmd, sizeof(gf_detect_sensor_cmd_t));
            GF_ERROR_BREAK(err);
            memcpy(info, &(cmd.o_sensor_info), sizeof(gf_sensor_info_t));
        }
        while (0);
        FUNC_EXIT(err);
        return err;
    }
}   // namespace goodix
