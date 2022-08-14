/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][Algo]"

#include "Algo.h"
#include "HalLog.h"
#include "gf_error.h"
#ifdef SUPPORT_DSP
#include "HalContext.h"
#include "HalDsp.h"
#include "HalUtils.h"
extern bool mDspSupport;
#endif   // SUPPORT_DSP

namespace goodix
{

    Algo::Algo(HalContext* context) : HalBase(context)
    {
    }

    Algo::~Algo()
    {
    }

    gf_error_t Algo::init()
    {
        gf_error_t err = GF_SUCCESS;
        gf_algo_init_t cmd;
        memset(&cmd, 0, sizeof(gf_algo_init_t));
        gf_cmd_header_t *header = (gf_cmd_header_t*) &cmd;
        header->target = GF_TARGET_ALGO;
        header->cmd_id = GF_CMD_ALGO_INIT;
        err = invokeCommand(&cmd, sizeof(cmd));
        return err;
    }

    gf_error_t Algo::enrollImage(gf_algo_enroll_image_t* enroll)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            GF_NULL_BREAK(enroll, err);
            enroll->header.target = GF_TARGET_ALGO;
            enroll->header.cmd_id = GF_CMD_ALGO_ENROLL;
            err = invokeCommand(enroll, sizeof(gf_algo_enroll_image_t));
        }
        while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Algo::authImage(gf_algo_auth_image_t* auth)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            GF_NULL_BREAK(auth, err);
            auth->header.target = GF_TARGET_ALGO;
#ifdef SUPPORT_DSP
			if (mDspSupport) {
				int64_t start_time = 0;
                int64_t end_time = 0;
                if (auth->i_recog_round < 2 && DSP_AVAILABLE == mContext->mDsp->checkDspValid())
                {
				    mContext->mDsp->mDspTime = 0;
                    if (1 != auth->i_retry_count)
                    {
                        /*call func1 for get feature*/
                        /*first step preprocess and get feature 1*/
                        auth->i_dsp_getfeature_step = 1;
                        auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
                        err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "[%s] getfeature step one fail", __func__);
                        }
                        /*in the meantime call dsp and do auth second part*/
                        /*call dsp function*/
                        start_time = HalUtils::getCurrentTimeMicrosecond();
                        err = mContext->mDsp->sendCmdToDsp(DSP_CMD_GET_FEATURE_TWO);
                        if (GF_SUCCESS != err)
                        {
                            LOG_E(LOG_TAG, "[%s] fail to gf_hal_send_cmd_to_dsp !", __func__);
                            break;
                        }
                    }

                    if (0 == auth->i_retry_count)
                    {
                        // do fast authenticate
                        auth->i_dsp_getfeature_step = 0;
                        auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
                        err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                        if (GF_SUCCESS == err)
                        {
                            LOG_E(LOG_TAG, "[%s] fast authentication success", __func__);
                        }
                        break;
                    }

                    LOG_D(LOG_TAG, "[%s] wait cdsp return", __func__);
                    err = mContext->mDsp->waitDspNotify();
                    if (GF_SUCCESS != err)
                    {
                        LOG_E(LOG_TAG, "[%s] no return from cdsp", __func__);
                        break;
                    }
                    end_time = HalUtils::getCurrentTimeMicrosecond();
                    LOG_D(LOG_TAG, "[%s] hvx retry frame %d hvx_get_feature2 time cost = %d ms!",
                             __func__, auth->i_retry_count, (int32_t)((end_time - start_time) /1000));
                    mContext->mDsp->mDspTime += (end_time - start_time);

                    /*if fail , use dsp get feature2 data get feature3*/
                    /*call third part get feature*/
                    auth->i_dsp_getfeature_step = 3;
                    auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_THREE;
                    err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                    auth->i_dsp_getfeature_step = 0;
                }
                else
                {
                    auth->i_dsp_getfeature_step = 0;
                    auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
                    err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                    LOG_D(LOG_TAG, "[%s] dsp is not available, normal authentication", __func__);
                }
             } else {
                auth->i_dsp_getfeature_step = 0;
                auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
                err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
             }
#else   // SUPPORT_DSP
            auth->i_dsp_getfeature_step = 0;
            auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
            err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
#endif   // SUPPORT_DSP
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
}   // namespace goodix
