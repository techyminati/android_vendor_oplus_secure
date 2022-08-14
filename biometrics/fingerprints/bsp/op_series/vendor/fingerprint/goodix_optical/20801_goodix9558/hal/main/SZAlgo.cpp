/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][ShenzhenAlgo]"

#include <stdlib.h>
#include <string.h>
#include <cutils/fs.h>

#include "HalContext.h"
#include "SZAlgo.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "FingerprintCore.h"
#include "szMsgBus.h"

#include "HalUtils.h"
#ifdef SUPPORT_DSP_HAL
#include "HalDsp.h"
#endif   // SUPPORT_DSP_HAL

namespace goodix {

    SZAlgo::SZAlgo(HalContext *context) : Algo(context), mExpoTime(0), mValidTime(0), spmt_pass_or_not(1),
                   first_get_feature_time(0), second_get_feature_time(0) {
    }

    SZAlgo::~SZAlgo() {
    }

    gf_error_t SZAlgo::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            gf_sz_algo_init_t cmd = {{ 0 }};
            gf_cmd_header_t *header = (gf_cmd_header_t *)&cmd;
            header->target = GF_TARGET_ALGO;
            header->cmd_id = GF_CMD_ALGO_INIT;
            /*spmt pass is 1*/
            cmd.spmt_pass = 1;
            err = invokeCommand(&cmd, sizeof(cmd));
            spmt_pass_or_not = cmd.spmt_pass;
            mExpoTime = cmd.expoTime;
            mValidTime = cmd.valid_time;
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZAlgo::authImage(gf_algo_auth_image_t *cmd_auth) {
        gf_error_t err = GF_SUCCESS;
        uint32_t enhance_get_feature_two_time = 0;
        uint32_t first_image_auth_count = 0;
        gf_sz_algo_auth_image_t *auth = (gf_sz_algo_auth_image_t*)cmd_auth;
        FUNC_ENTER();
        UNUSED_VAR(first_image_auth_count);
        first_get_feature_time = 0;
        second_get_feature_time = 0;

        do {
            GF_NULL_BREAK(cmd_auth, err);
            auth->cmd.header.target = GF_TARGET_ALGO;

#ifdef SUPPORT_DSP_HAL
            HalDsp *dsp = mContext->getDsp();
            if (nullptr != dsp && DSP_AVAILABLE == dsp->checkDspValid()) {
                uint64_t time_start_two = 0;
                uint64_t time_end_two = 0;
                uint64_t time_start_four = 0;
                uint64_t time_end_four = 0;
                /*call func1 for get feature*/
                /*first step preprocess and get feature 1*/
                auth->cmd.header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_ONE;
                err = invokeCommand(auth, sizeof(gf_sz_algo_auth_image_t));
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "[%s] cpu getfeature 1 fail", __func__);
                    break;
                }
                /*in the meantime call dsp and do auth second part*/
                /*call dsp function*/
                first_image_auth_count = auth->cmd.i_retry_count ? 1 : 2;
                while (first_image_auth_count--) {
                    time_start_two = HalUtils::getCurrentTimeMicrosecond();
                    err = dsp->sendCmdToDsp(DSP_CMD_GET_FEATURE_TWO);
                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] fail send get two cmd to dsp !", __func__);
                        break;
                    }

                    LOG_D(LOG_TAG, "[%s] wait cdsp return", __func__);
                    err = dsp->waitDspNotify();

                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] no return from cdsp", __func__);
                        err = GF_ERROR_ACQUIRED_IMAGER_DIRTY;
                        break;
                    }

                    time_end_two = HalUtils::getCurrentTimeMicrosecond();
                    LOG_D(LOG_TAG, "[%s] dsp retry frame %d dsp_get_feature2 time cost = %ldms!",
                            __func__, auth->cmd.i_retry_count, (long)(time_end_two - time_start_two) / 1000);

                    auth->cmd.header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_THREE;

                    err = invokeCommand(auth, sizeof(gf_sz_algo_auth_image_t));
                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_THREE fail", __func__);
                        break;
                    }

                    if (auth->cmd.o_feature_mode != GF_SZ_ALGO_FEATURE_MODE_G2) {
                        LOG_D(LOG_TAG, "[%s] o_feature_mode is 0 or 1, call dsp to getfeature four",
                                __func__);
                        time_start_four = HalUtils::getCurrentTimeMicrosecond();
                        /*err = dsp->sendCmdToDsp(DSP_CMD_GET_FEATURE_FOUR);
                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "[%s] fail to send get four cmd to dsp !", __func__);
                            break;
                        }*/
                    }

                    if (auth->cmd.o_feature_mode != GF_SZ_ALGO_FEATURE_MODE_NEW) {
                        LOG_D(LOG_TAG, "[%s] o_feature_mode is 0 or 2, getfeature old", __func__);
                        time_end_four = HalUtils::getCurrentTimeMicrosecond();
                        if (auth->cmd.i_retry_count == 0 && first_image_auth_count == 0) {
                            second_get_feature_time = time_end_four - time_start_two;
                        } else {
                            first_get_feature_time = time_end_four - time_start_two;
                        }
                        auth->cmd.header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_OLD;
                        err = invokeCommand(auth, sizeof(gf_sz_algo_auth_image_t));
                        if (auth->cmd.o_feature_mode == GF_SZ_ALGO_FEATURE_MODE_G2) {
                            LOG_E(LOG_TAG, "[%s] o_feature_mode:%d GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_OLD fail",
                                    __func__, auth->cmd.o_feature_mode);
                            if ((GF_ERROR_NOT_MATCH == err || GF_ERROR_RESIDUAL_FINGER == err)
                                    && GF_SUCCESS == auth->err_feature_one && 0 == auth->cmd.i_retry_count) {
                                LOG_E(LOG_TAG, "[%s] get cache authenticate", __func__);
                                continue;
                            } else {
                                LOG_E(LOG_TAG, "[%s] get feature old error, err %d, retry %d", __func__, auth->err_feature_one, auth->cmd.i_retry_count);
                                break;
                            }
                        }
                    }

                    if (auth->cmd.o_feature_mode != GF_SZ_ALGO_FEATURE_MODE_G2) {
                        LOG_D(LOG_TAG, "[%s] o_feature_mode is 0 or 1, wait dsp and getfeature five" , __func__);
                        /*err = dsp->waitDspNotify();

                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "[%s] no return from cdsp", __func__);
                            err = GF_ERROR_ACQUIRED_IMAGER_DIRTY;
                            break;
                        }*/
                        time_end_four = HalUtils::getCurrentTimeMicrosecond();
                        if (auth->cmd.i_retry_count == 0 && first_image_auth_count == 0) {
                            second_get_feature_time = time_end_four - time_start_two;
                        } else {
                            first_get_feature_time = time_end_four - time_start_two;
                        }
                        LOG_D(LOG_TAG, "[%s] dsp_get_feature4 time or get feature old cost = %ldms!",
                                __func__, (long)(time_end_four - time_start_four) / 1000);

                        auth->cmd.header.cmd_id = GF_CMD_ALGO_AUTHENTICATE_HVX_GET_FEATURE_FIVE;
                        err = invokeCommand(auth, sizeof(gf_sz_algo_auth_image_t));
                        if ((GF_ERROR_NOT_MATCH == err || GF_ERROR_RESIDUAL_FINGER == err)
                                && GF_SUCCESS == auth->err_feature_one && 0 == auth->cmd.i_retry_count) {
                            LOG_E(LOG_TAG, "[%s] get cache authenticate", __func__);
                            continue;
                        } else {
                            LOG_E(LOG_TAG, "[%s] getfeature5 done" , __func__);
                            break;
                        }
                    }
                }
            } else {
                auth->cmd.header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
                err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
                LOG_D(LOG_TAG, "[%s] dsp is not available, normal authentication", __func__);
            }
#else   // SUPPORT_DSP_HAL
            auth->cmd.header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
            err = invokeCommand(auth, sizeof(gf_algo_auth_image_t));
#endif   // SUPPORT_DSP_HAL
        } while (0);

        sendMessage(MSG_BIG_DATA_GET_FEAURE_TIME, first_get_feature_time, enhance_get_feature_two_time);
        FUNC_EXIT(err);
        return err;
    }
    void SZAlgo::getSpmtPassOrNot(uint32_t *spmt_pass) {
        VOID_FUNC_ENTER();
        *spmt_pass = spmt_pass_or_not;
        VOID_FUNC_EXIT();
    }

    void SZAlgo::setSpmtPassOrNot(uint32_t spmt_pass) {
        VOID_FUNC_ENTER();
        spmt_pass_or_not = spmt_pass;
        VOID_FUNC_EXIT();
    }

    void SZAlgo::getFeatureTime(uint32_t* first_time, uint32_t* second_time) {
        VOID_FUNC_ENTER();
        *first_time = first_get_feature_time;
        *second_time = second_get_feature_time;
        VOID_FUNC_EXIT();
    }

    gf_algo_enroll_image_t* SZAlgo::createEnrollCmd() {
        gf_sz_algo_enroll_image_t* cmd = new gf_sz_algo_enroll_image_t();
        memset(cmd, 0, sizeof(gf_sz_algo_enroll_image_t));
        return (gf_algo_enroll_image_t*)cmd;
    }

    gf_algo_auth_image_t* SZAlgo::createAuthCmd() {
        gf_sz_algo_auth_image_t* cmd = new gf_sz_algo_auth_image_t();
        memset(cmd, 0, sizeof(gf_sz_algo_auth_image_t));
        return (gf_algo_auth_image_t*)cmd;
    }

}  // namespace goodix
