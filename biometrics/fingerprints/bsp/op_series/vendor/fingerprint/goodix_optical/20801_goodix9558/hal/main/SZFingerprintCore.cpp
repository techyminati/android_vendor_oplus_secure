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
#include "HalContext.h"
#include "SZFingerprintCore.h"
#include "gf_algo_types.h"
#include "gf_sz_types.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "HalUtils.h"
#include "SZSensor.h"
#include "Device.h"
#include "Algo.h"
#include "SZAlgo.h"
#include "szMsgBus.h"

extern "C" {
#include "Fpsys.h"
}

#define SET_ORIGIN_LEVEL_FAR_THRESHOLD  101  // default far
#define SET_HIGH_LEVEL_FAR_THRESHOLD  102   // high far

namespace goodix {

    SZFingerprintCore::SZFingerprintCore(HalContext *context) :
        FingerprintCore(context) {
    }

    SZFingerprintCore::~SZFingerprintCore() {
    }

    gf_error_t SZFingerprintCore::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::init();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onAuthStop(AuthenticateContext *context) {
        gf_cmd_header_t cmd = { 0 };
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] retry count:%d", __func__, context->retry);
        sendMessage(MSG_BIG_DATA_ENROLL_AUTH_END);
        err = FingerprintCore::onAuthStop(context);
        // stop capture image
        cmd.target = GF_TARGET_SENSOR;
        cmd.cmd_id = GF_SZ_CMD_STOP_CAPTURE_IMAGE;
        err = invokeCommand(&cmd, sizeof(cmd));

        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] Stopping cature image failed", __func__);
        }

        // sleep sensor
        mContext->mSensor->sleepSensor();
        mContext->isStopCaptureDone = true;
        FUNC_EXIT(err);
        return err;
    }

    void SZFingerprintCore::forceStudy(void) {
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mContext->mHalLock);
        gf_cmd_header_t cmd = { 0 };
        FUNC_ENTER();
        cmd.target = GF_TARGET_ALGO;
        cmd.cmd_id = GF_SZ_CMD_FORCE_STUDY;
        err = invokeCommand(&cmd, sizeof(cmd));

        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] Some wrong happend in force study", __func__);
        }

        FUNC_EXIT(err);
    }

    gf_error_t SZFingerprintCore::onEnrollStart(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        mContext->mSensor->wakeupSensor();
        err = FingerprintCore::onEnrollStart(context);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onEnrollStop(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        sendMessage(MSG_BIG_DATA_ENROLL_AUTH_END);
        err = FingerprintCore::onEnrollStop(context);
        mContext->mSensor->sleepSensor();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onAuthStart(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        mContext->mSensor->wakeupSensor();
        mContext->isStopCaptureDone = false;
        err = FingerprintCore::onAuthStart(context);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onAfterAuthAlgo(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);

        if ((mContext->mConfig->support_performance_dump)
            && (context->result != GF_SUCCESS)) {
            dumpKPI(__func__);
        }

        return err;
    }

    gf_error_t SZFingerprintCore::onAfterAuthRetry(AuthenticateContext *context) {
        return context->result;
    }

    gf_error_t SZFingerprintCore::setEnvironmentLevel(uint32_t save_level) {
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mContext->mHalLock);
        gf_cmd_header_t cmd = { 0 };
        FUNC_ENTER();

        if (SET_ORIGIN_LEVEL_FAR_THRESHOLD == save_level) {
            save_level = 0;
        } else if (SET_HIGH_LEVEL_FAR_THRESHOLD == save_level) {
            save_level = 1;
        } else {
            save_level = 0;
        }

        LOG_D(LOG_TAG, "[%s] save level  %d", __func__, save_level);
        cmd.target = GF_TARGET_ALGO;
        cmd.cmd_id = GF_SZ_CMD_SET_ENVIRONMENT_LEVEL;
        *(uint32_t *) cmd.reserved = save_level;
        err = invokeCommand(&cmd, sizeof(cmd));

        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] Some wrong happend when set envirment", __func__);
        }

        FUNC_EXIT(err);
        return err;
    }
    gf_error_t SZFingerprintCore::prepareAuthRequest() {
        uint32_t spmt_pass = 0;
        gf_error_t err = GF_SUCCESS;
        SZAlgo *algo = static_cast<SZAlgo *>(mContext->mAlgo);
        algo->getSpmtPassOrNot(&spmt_pass);

        if (0 == spmt_pass) {
            LOG_E(LOG_TAG, "[%s] spmt is not pass, please to do the process of spmt",
                  __func__);
            err = GF_ERROR_INVALID_DATA;
        }

        LOG_D(LOG_TAG, "[%s] spmt is pass", __func__);
        return err;
    }

    gf_error_t SZFingerprintCore::prepareEnrollRequest() {
        uint32_t spmt_pass = 0;
        gf_error_t err = GF_SUCCESS;
        SZAlgo *algo = static_cast<SZAlgo *>(mContext->mAlgo);
        algo->getSpmtPassOrNot(&spmt_pass);

        if (0 == spmt_pass) {
            LOG_E(LOG_TAG, "[%s] spmt is not pass, please to do the process of spmt",
                  __func__);
            err = GF_ERROR_INVALID_DATA;
        }

        return err;
    }
    gf_error_t SZFingerprintCore::dumpKPI(const char *func_name) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (nullptr == func_name) {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] func_name is nullptr", __func__)
                break;
            }

            // get kpi
            gf_algo_kpi_t kpi = { { 0 } };
            kpi.header.target = GF_TARGET_ALGO;
            kpi.header.cmd_id = GF_CMD_ALGO_KPI;
            gf_error_t err = invokeCommand(&kpi, sizeof(gf_algo_kpi_t));

            if (GF_SUCCESS != err) {
                break;
            }

            gf_algo_performance_t *dump_performance = &kpi.o_performance;
            uint32_t first_get_feature_time = 0;
            uint32_t second_get_feature_time = 0;
            uint32_t second_authenticate_time = 0;
#ifdef SUPPORT_DSP_HAL
            SZAlgo *algo = static_cast<SZAlgo *>(mContext->mAlgo);
            algo->getFeatureTime(&first_get_feature_time, &second_get_feature_time);
#else   // SUPPORT_DSP_HAL
            first_get_feature_time = dump_performance->get_feature_time;
            second_get_feature_time = *(uint32_t*)&dump_performance->reserve[0];
#endif   // SUPPORT_DSP_HAL
            second_authenticate_time = *(uint32_t*)&dump_performance->reserve[4];
            // do dump
            LOG_D(LOG_TAG, "[%s]     goodix_quality=%d", func_name,
                  dump_performance->image_quality);
            LOG_V(LOG_TAG, "[%s]     valid_area=%d", func_name,
                  dump_performance->valid_area);
            LOG_V(LOG_TAG, "[%s]     key_point_num=%d", func_name,
                  dump_performance->key_point_num);
            LOG_V(LOG_TAG, "[%s]     get_raw_data_time=%dms", func_name,
                  dump_performance->get_raw_data_time / 1000);
            LOG_V(LOG_TAG, "[%s]     preprocess_time=%dms", func_name,
                  dump_performance->preprocess_time / 1000);
            LOG_V(LOG_TAG, "[%s]     get_feature_time=%dms", func_name,
                  first_get_feature_time / 1000);
            LOG_V(LOG_TAG, "[%s]     second_get_feature_time=%dms", func_name,
                  second_get_feature_time / 1000);

            switch (getWorkState()) {
                case STATE_ENROLL: {
                    LOG_V(LOG_TAG, "[%s]     increase_rate=%d", func_name,
                          dump_performance->increase_rate);
                    LOG_V(LOG_TAG, "[%s]     overlay=%d", func_name, dump_performance->overlay);
                    LOG_V(LOG_TAG, "[%s]     enroll_time=%dms", func_name,
                          dump_performance->enroll_time / 1000);
                    break;
                }

                case STATE_AUTHENTICATE: {
                    LOG_V(LOG_TAG, "[%s]     study flag =%u", func_name,
                          dump_performance->authenticate_study_flag);
                    LOG_V(LOG_TAG, "[%s]     match_score=%d", func_name,
                          dump_performance->match_score);
                    LOG_V(LOG_TAG, "[%s]     authenticate_time=%dms", func_name,
                          dump_performance->authenticate_time / 1000);
                    LOG_V(LOG_TAG, "[%s]     second_authenticate_time=%dms", func_name,
                          second_authenticate_time / 1000);
                    LOG_V(LOG_TAG,
                            "[%s]     KPI time(get_raw_data_time + preprocess+get_feature_time+authenticate)=%dms",
                            func_name, (dump_performance->get_raw_data_time
                                + dump_performance->preprocess_time
                                + first_get_feature_time
                                + second_get_feature_time
                                + second_authenticate_time
                                + dump_performance->authenticate_time) / 1000);
                    break;
                }

                default: {
                    err = GF_ERROR_GENERIC;
                    break;
                }
            }

            if (GF_SUCCESS == err) {
                LOG_V(LOG_TAG, "[%s]    total time=%ums", func_name,
                      (uint32_t)(mTotalKpiTime / 1000));
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZFingerprintCore::onEnrollRequested(const void *hat, uint32_t gid,
                                                    uint32_t timeoutSec) {
        return FingerprintCore::onEnrollRequested(hat, gid, timeoutSec);
    }

    gf_error_t SZFingerprintCore::onEnrollUpEvt() {
        return FingerprintCore::onEnrollUpEvt();
    }

    gf_error_t SZFingerprintCore::onBeforeEnrollCapture(EnrollContext *context) {
        return FingerprintCore::onBeforeEnrollCapture(context);
    }

    gf_error_t SZFingerprintCore::onAfterEnrollCapture(EnrollContext *context) {
        return FingerprintCore::onAfterEnrollCapture(context);
    }

    gf_error_t SZFingerprintCore::onBeforeEnrollAlgo(EnrollContext *context) {
        return FingerprintCore::onBeforeEnrollAlgo(context);
    }

    gf_error_t SZFingerprintCore::onAfterEnrollAlgo(EnrollContext *context) {
        return FingerprintCore::onAfterEnrollAlgo(context);
    }

    void SZFingerprintCore::onEnrollError(EnrollContext *context) {
        FingerprintCore::onEnrollError(context);
    }

    gf_error_t SZFingerprintCore::notifyEnrollProgress(EnrollContext *context) {
        return FingerprintCore::notifyEnrollProgress(context);
    }

    gf_error_t SZFingerprintCore::onAuthRequested(uint64_t operationId,
                                                  uint32_t gid) {
        return FingerprintCore::onAuthRequested(operationId, gid);
    }

    gf_error_t SZFingerprintCore::onAuthUpEvt() {
        return FingerprintCore::onAuthUpEvt();
    }

    gf_error_t SZFingerprintCore::onBeforeAuthCapture(AuthenticateContext
                                                      *context) {
        return FingerprintCore::onBeforeAuthCapture(context);
    }

    gf_error_t SZFingerprintCore::onAfterAuthCapture(AuthenticateContext *context) {
        return FingerprintCore::onAfterAuthCapture(context);
    }

    gf_error_t SZFingerprintCore::onBeforeAuthAlgo(AuthenticateContext *context) {
        return FingerprintCore::onBeforeAuthAlgo(context);
    }

    gf_error_t SZFingerprintCore::onAfterAuthSuccess(AuthenticateContext *context) {
        return FingerprintCore::onAfterAuthSuccess(context);
    }

    void SZFingerprintCore::onAuthError(AuthenticateContext *context) {
        return FingerprintCore::onAuthError(context);
    }

    gf_error_t SZFingerprintCore::notifyAuthSuccess(AuthenticateContext *context) {
        return FingerprintCore::notifyAuthSuccess(context);
    }

    void SZFingerprintCore::notifyAuthNotMatched() {
        return FingerprintCore::notifyAuthNotMatched();
    }

    gf_error_t SZFingerprintCore::onResetEvent() {
        return FingerprintCore::onResetEvent();
    }

    // notify message
    gf_error_t SZFingerprintCore::notifyAcquiredInfo(gf_fingerprint_acquired_info_t
                                                     info) {
        gf_error_t err = GF_SUCCESS;
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        FUNC_ENTER();

        if (info > GF_FINGERPRINT_ACQUIRED_VENDOR_BASE) {
            switch (info) {
                case GF_FINGERPRINT_ACQUIRED_FINGER_DOWN: {
                    notifyTouch(GF_FINGERPRINT_TOUCH_DOWN);
                    break;
                }

                case GF_FINGERPRINT_ACQUIRED_FINGER_UP: {
                    notifyTouch(GF_FINGERPRINT_TOUCH_UP);
                    clear_finger_ready_flag();
                    break;
                }

                default : {
                    err = FingerprintCore::notifyAcquiredInfo(info);
                    break;
                }
            }
        }else{
            err = FingerprintCore::notifyAcquiredInfo(info);
        }
        return err;

    }

    gf_error_t SZFingerprintCore::notifyErrorInfo(gf_fingerprint_error_t err) {
        return FingerprintCore::notifyErrorInfo(err);
    }

    gf_error_t SZFingerprintCore::notifyRemove(uint32_t gid, uint32_t fid,
                                               uint32_t remainingTemplates) {
        return FingerprintCore::notifyRemove(gid, fid, remainingTemplates);
    }

    gf_error_t SZFingerprintCore::notifyEnumerate(gf_enumerate_t *result) {
        return FingerprintCore::notifyEnumerate(result);
    }

    void SZFingerprintCore::doCancel() {
        return FingerprintCore::doCancel();
    }
}  // namespace goodix
