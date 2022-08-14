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
#include "HalUtils.h"
#ifdef SUPPORT_DSP_HAL
#include "HalContext.h"
#include "HalDsp.h"
#endif   // SUPPORT_DSP_HAL

namespace goodix {

    Algo::Algo(HalContext *context) : HalBase(context) {
    }

    Algo::~Algo() {
    }

    gf_error_t Algo::init() {
        gf_error_t err = GF_SUCCESS;
        gf_algo_init_t cmd = {{ 0 }};
        gf_cmd_header_t *header = (gf_cmd_header_t *) &cmd;
        header->target = GF_TARGET_ALGO;
        header->cmd_id = GF_CMD_ALGO_INIT;
        err = invokeCommand(&cmd, sizeof(cmd));
        return err;
    }

    gf_error_t Algo::enrollImage(gf_algo_enroll_image_t *enroll) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            GF_NULL_BREAK(enroll, err);
            enroll->header.target = GF_TARGET_ALGO;
            enroll->header.cmd_id = GF_CMD_ALGO_ENROLL;
            err = invokeCommand(enroll, sizeof(gf_algo_enroll_image_t));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Algo::authImage(gf_algo_auth_image_t *auth) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            GF_NULL_BREAK(auth, err);
            auth->header.target = GF_TARGET_ALGO;

#ifdef SUPPORT_DSP_HAL
            HalDsp *dsp = mContext->getDsp();
            if (nullptr != dsp && DSP_AVAILABLE == dsp->checkDspValid()) {
                uint64_t time_start = 0;
                uint64_t time_end = 0;
                /*call func1 for get feature*/
                /*first step preprocess and get feature 1*/
                auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_ONE;
                err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "[%s] cpu getfeature 1 fail", __func__);
                    break;
                }
                /*in the meantime call dsp and do auth second part*/
                /*call dsp function*/
                time_start = HalUtils::getCurrentTimeMicrosecond();
                err = dsp->sendCmdToDsp(DSP_CMD_GET_FEATURE_TWO);
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "[%s] fail to send get two cmd to dsp !", __func__);
                    break;
                }

                if (auth->i_retry_count == 0) {
                    auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_FAST;
                    err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] GF_CMD_ALGO_AUTHENTICATE_HVX_FAST fail", __func__);
                    }
                } else {
                    LOG_D(LOG_TAG, "[%s] wait cdsp return", __func__);
                    err = dsp->waitDspNotify();

                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] no return from cdsp", __func__);
                        break;
                    }

                    time_end = HalUtils::getCurrentTimeMicrosecond();
                    LOG_D(LOG_TAG, "[%s] dsp retry frame %d dsp_get_feature2 time cost = %ldms!",
                            __func__, auth->i_retry_count, (long)(time_end - time_start) / 1000);
                }

                /*end dsp*/

                /*if fail , use dsp get feature2 data get feature3*/
                if (err == GF_ERROR_NOT_MATCH || auth->i_retry_count >= 1) {
                    /*call third part get feature*/
                    if (GF_ERROR_NOT_MATCH == err && 0 == auth->i_retry_count) {
                        LOG_D(LOG_TAG, "[%s] first time authticate failed, wait dsp", __func__);
                        err = dsp->waitDspNotify();

                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "[%s] no return from cdsp", __func__);
                            break;
                        }

                        time_end = HalUtils::getCurrentTimeMicrosecond();
                        LOG_D(LOG_TAG, "[%s] dsp first frame %d dsp_get_feature2 time cost = %ldms!",
                                __func__, auth->i_retry_count, (long)(time_end - time_start) / 1000);
                    }

                    auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_THREE;
                    err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_THREE fail", __func__);
                        break;
                    }
                }
            } else {
                auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
                err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                LOG_D(LOG_TAG, "[%s] dsp is not available, normal authentication", __func__);
            }
#else   // SUPPORT_DSP_HAL
            auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
            err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
#endif   // SUPPORT_DSP_HAL
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_algo_enroll_image_t* Algo::createEnrollCmd() {
        gf_algo_enroll_image_t* cmd = new gf_algo_enroll_image_t();
        memset(cmd, 0, sizeof(gf_algo_enroll_image_t));
        return cmd;
    }

    void Algo::destroyEnrollCmd(gf_algo_enroll_image_t* cmd) {
        if (cmd != nullptr) {
            delete cmd;
        }
    }

    gf_algo_auth_image_t* Algo::createAuthCmd() {
        gf_algo_auth_image_t* cmd = new gf_algo_auth_image_t();
        memset(cmd, 0, sizeof(gf_algo_auth_image_t));
        return cmd;
    }

    void Algo::destroyAuthCmd(gf_algo_auth_image_t* cmd) {
        if (cmd != nullptr) {
            delete cmd;
        }
    }
}  // namespace goodix
