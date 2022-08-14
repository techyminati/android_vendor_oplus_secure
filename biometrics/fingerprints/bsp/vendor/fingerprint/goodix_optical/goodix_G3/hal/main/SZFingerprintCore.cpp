/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][SZFingerprintCore]"

#include <stdlib.h>
#include <endian.h>
#include <cutils/properties.h>
#include "HalContext.h"
#include "SZFingerprintCore.h"
#include "gf_algo_types.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "HalUtils.h"
#include "SZSensor.h"
#include "Device.h"
#include "Algo.h"
#include "SZAlgo.h"

#define SET_ORIGIN_LEVEL_FAR_THRESHOLD  101  //  default far
#define SET_HIGH_LEVEL_FAR_THRESHOLD  102   //  high far
#define FINGERPRINT_CALIPROP      "vendor.fingerprint.cali"

namespace goodix
{

    SZFingerprintCore::SZFingerprintCore(HalContext *context)
        : FingerprintCore(context)
    {
    }

    SZFingerprintCore::~SZFingerprintCore()
    {
    }

    gf_error_t SZFingerprintCore::init()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::init();
        initcheckcalibration();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onAuthStop(AuthenticateContext *context)
    {
        gf_cmd_header_t cmd;
        memset(&cmd, 0, sizeof(gf_cmd_header_t));
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] retry count:%d", __func__, context->retry);
        err = FingerprintCore::onAuthStop(context);
        // stop capture image
        cmd.target = GF_TARGET_SENSOR;
        cmd.cmd_id = GF_SZ_CMD_STOP_CAPTURE_IMAGE;
        err = invokeCommand(&cmd, sizeof(cmd));
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] Stopping cature image failed", __func__);
        }

        // sleep sensor
        mContext->mSensor->sleepSensor();
        FUNC_EXIT(err);
        return err;
    }

    void SZFingerprintCore::forceStudy(void)
    {
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mContext->mHalLock);
        gf_cmd_header_t cmd;
        memset(&cmd, 0, sizeof(gf_cmd_header_t));
        FUNC_ENTER();
        cmd.target = GF_TARGET_ALGO;
        cmd.cmd_id = GF_SZ_CMD_FORCE_STUDY;
        err = invokeCommand(&cmd, sizeof(cmd));

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] Some wrong happend in force study", __func__);
        }

        FUNC_EXIT(err);
    }

    gf_error_t SZFingerprintCore::onEnrollStart(EnrollContext *context)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        mContext->mSensor->wakeupSensor();
        err = FingerprintCore::onEnrollStart(context);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onEnrollStop(EnrollContext *context)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::onEnrollStop(context);
        mContext->mSensor->sleepSensor();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onAuthStart(AuthenticateContext *context)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        mContext->mSensor->wakeupSensor();
        err = FingerprintCore::onAuthStart(context);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onAfterAuthAlgo(AuthenticateContext *context)
    {
        gf_error_t err = GF_SUCCESS;

        if (!mSkipFlag || context->retry <= 1 || context->auth_cmd->o_round_two) {
            if (context->result == GF_ERROR_NOT_MATCH)
            {
                context->auth_cmd->i_recog_round = 2;
                context->result = mContext->mAlgo->authImage(context->auth_cmd);
                LOG_D(LOG_TAG, "[%s] antipeep & screen struct flag : %d", __func__, context->auth_cmd->o_antipeep_screen_struct_flag);
            }
        }
        // if dump propperty is seted, use for dump data
        sendMessage(MsgBus::MSG_AUTHENTICATE_ALGO_END, context->result, context->retry, context->auth_cmd->i_recog_round);


        if ((mContext->mConfig->support_performance_dump)
            && (context->result != GF_SUCCESS))
        {
            dumpKPI(__func__);
        }
        return err;
    }

    gf_error_t SZFingerprintCore::onAfterAuthRetry(AuthenticateContext *context)
    {
        return context->result;
    }

    gf_error_t SZFingerprintCore::setEnvironmentLevel(uint32_t save_level)
    {
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mContext->mHalLock);
        gf_cmd_header_t cmd;
        memset(&cmd, 0, sizeof(gf_cmd_header_t));
        FUNC_ENTER();

        if (SET_ORIGIN_LEVEL_FAR_THRESHOLD == save_level)
        {
            save_level = 0;
        }
        else if (SET_HIGH_LEVEL_FAR_THRESHOLD == save_level)
        {
            save_level = 1;
        }
        else
        {
            save_level = 0;
        }

        LOG_D(LOG_TAG, "[%s] save level  %d", __func__, save_level);
        cmd.target = GF_TARGET_ALGO;
        cmd.cmd_id = GF_SZ_CMD_SET_ENVIRONMENT_LEVEL;
        *(uint32_t*)cmd.reserved = save_level;
        err = invokeCommand(&cmd, sizeof(cmd));

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] Some wrong happend when set envirment", __func__);
        }

        FUNC_EXIT(err);
        return err;
    }

    
    gf_error_t SZFingerprintCore::prepareAuthRequest()
    {
        uint32_t spmt_pass = 0;
        gf_error_t err = GF_SUCCESS;
        SZAlgo * algo = static_cast<SZAlgo*>(mContext->mAlgo);

        algo->getSpmtPassOrNot(&spmt_pass);
        if (0 == spmt_pass)
        {
            LOG_E(LOG_TAG, "[%s] spmt is not pass, please to do the process of spmt", __func__);
            err = GF_ERROR_INVALID_DATA;
        }

        LOG_D(LOG_TAG, "[%s] spmt is pass", __func__);
        return err;
    }

    gf_error_t SZFingerprintCore::prepareEnrollRequest()
    {
        uint32_t spmt_pass = 0;
        gf_error_t err = GF_SUCCESS;
        SZAlgo * algo = static_cast<SZAlgo*>(mContext->mAlgo);

        algo->getSpmtPassOrNot(&spmt_pass);
        if (0 == spmt_pass)
        {
            LOG_E(LOG_TAG, "[%s] spmt is not pass, please to do the process of spmt", __func__);
            err = GF_ERROR_INVALID_DATA;
        }

        return err;
    }

    void SZFingerprintCore::initcheckcalibration()
    {
        VOID_FUNC_ENTER();
        uint32_t spmt_pass = 0;
        SZAlgo * algo = static_cast<SZAlgo*>(mContext->mAlgo);

        algo->getSpmtPassOrNot(&spmt_pass);
        if (0 == spmt_pass)
        {
            property_set(FINGERPRINT_CALIPROP, "0");
            LOG_E(LOG_TAG, "[%s] spmt is not pass, please to do the process of spmt", __func__);
        } else {
            property_set(FINGERPRINT_CALIPROP, "1");
            LOG_D(LOG_TAG, "[%s] spmt is pass", __func__);
        }
        VOID_FUNC_EXIT();
    }
}  // namespace goodix
