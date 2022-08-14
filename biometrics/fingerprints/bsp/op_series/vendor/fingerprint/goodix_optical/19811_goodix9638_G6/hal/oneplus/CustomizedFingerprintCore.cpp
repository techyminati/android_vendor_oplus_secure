/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][CustomizedFingerprintCore]"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <endian.h>
#include <dlfcn.h>
#include "HalContext.h"
#include "CustomizedFingerprintCore.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "HalUtils.h"
#include "CustomizedDelmarSensor.h"
#include "Algo.h"
#include "CustomizedDelmarCommon.h"
#include "CustomizedDevice.h"
#include "gf_customized_algo_types.h"
#include "GoodixFingerprint.h"
#include <cutils/properties.h>


extern "C" {
#include "Fpsys.h"
}

#ifdef FP_TRACE_DEBUG
#include <cutils/trace.h>
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#endif  // FP_TRACE_DEBUG

#define PERF_HAL_PATH "libqti-perfd-client.so"
static void *qcopt_handle;  // qcopt_handle
static int (*perf_lock_acq)(unsigned long handle, int duration, int list[], int numArgs);  // perf_lock_acq
static int (*perf_lock_rel)(int handle);  // perf_lock_rel
static int g_lokcAcq = 0;  // mLokcAcq
static int lock_handle = 0;  // lock_handle
static int init_status = 0;  // lock_handle
// 0 cycle, 1 ellipse for icon
#define OPTICAL_ENROLLICON  "persist.vendor.fingerprint.optical.enrollicon"
#define OPTICAL_AUTHICON       "persist.vendor.fingerprint.optical.authicon"
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"
#define PROPERTY_SCREEN_TYPE   "persist.vendor.fingerprint.optical.lcdtype"
char algo_package_version[] = "v03.01.03.06";

void getQcoptHandle() {
#if 0    
    if (init_status) {
        return;
    }
    init_status = 1;
    qcopt_handle = dlopen(PERF_HAL_PATH, RTLD_NOW);
    if (!qcopt_handle) {
        return;
    }
    perf_lock_acq = (int (*)(unsigned long, int, int *, int))dlsym(qcopt_handle, "perf_lock_acq");

    if (!perf_lock_acq) {
        LOG_E(LOG_TAG, "Unable to get perf_lock_acq function handle.\n");
    }

    perf_lock_rel = (int (*)(int))dlsym(qcopt_handle, "perf_lock_rel");

    if (!perf_lock_rel) {
        LOG_E(LOG_TAG, "Unable to get perf_lock_rel function handle.\n");
    }
#endif    
}

void setCpubw() {
#if 0    
    int perf_lock_opts[10] = {0x40400000, 0x1, 0x40800000, 0xFFF, 0x40800100, 0xFFF, 0x43400000, 0xFFFF, 0x43000000, 0xFF};
    if (perf_lock_acq && !g_lokcAcq) {
        lock_handle = perf_lock_acq(lock_handle, 0, perf_lock_opts, 10);
        g_lokcAcq = 1;
    }
#endif    
}

void releaseCpubw() {
#if 0    
    LOG_D(LOG_TAG, "releaseCpubw E");
    if (perf_lock_rel && g_lokcAcq) {
        perf_lock_rel(lock_handle);
        lock_handle = 0;
        g_lokcAcq = 0;
    }
#endif    
}

#ifdef SL_FP_FEATURE_OP_CUSTOMIZE
void sileadHypnusSetAction() {
    char buf[MAX_LEN];
    int fd = open(action_info, O_WRONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "SetAction open err :%d", fd);
        return;
    }
    snprintf(buf, sizeof(buf), "%d %d", ACTION_IO, ACTION_TIMEOUT);
    write(fd, buf, MAX_LEN);
    close(fd);
    LOG_E(LOG_TAG, "Fingerprint SetAction for silead optical");
    return;
}
#endif  // SL_FP_FEATURE_OP_CUSTOMIZE
namespace goodix {

    CustomizedFingerprintCore::CustomizedFingerprintCore(HalContext *context)
        : DelmarFingerprintCore(context) {
        gf_error_t err = GF_SUCCESS;
        err = getAlgoVersion();
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] get ALGO_VERSION fail.", __func__);
        }
    }

    CustomizedFingerprintCore::~CustomizedFingerprintCore() {
    }

    gf_error_t CustomizedFingerprintCore::init() {
        gf_error_t err = GF_SUCCESS;
        err = DelmarFingerprintCore::init();
        getQcoptHandle();
        return err;
    }

    // notify message
    gf_error_t CustomizedFingerprintCore::notifyAcquiredInfo(gf_fingerprint_acquired_info_t
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

    
    gf_error_t CustomizedFingerprintCore::notifyTouch(gf_fingerprint_msg_type_t type)
    {
        gf_error_t err = GF_SUCCESS;
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        FUNC_ENTER();
        if (nullptr != mNotify)
        {
            /*
            if (type == GF_FINGERPRINT_TOUCH_UP) {
                //setAuthUpNotified(true);
                LOG_D(LOG_TAG, "[%s] notifyTouch Up", __func__);
            } else {
                message.data.tp_info.x = mPressInfo.pressX;
                message.data.tp_info.y = mPressInfo.pressY;
                if(((getWorkState() == STATE_ENROLL)&&(enrollicon_config == 0))||((getWorkState() ==STATE_AUTHENTICATE)&&(authicon_config == 0))){
                    message.data.tp_info.touch_major = 0;
                    message.data.tp_info.touch_minor = 0;
                    message.data.tp_info.touch_angle = 0;   
                    LOG_I(LOG_TAG, "[%s] notifyTouch  DELMAR_SENSOR_UI_TYPE_CIRCLE ", __func__);
                } else if(((getWorkState() == STATE_ENROLL)&&(enrollicon_config == 1))||((getWorkState() == STATE_AUTHENTICATE)&&(authicon_config == 1))) {
                    message.data.tp_info.touch_major = mPressInfo.touchMajor;
                    message.data.tp_info.touch_minor = mPressInfo.touchMinor;
                    message.data.tp_info.touch_angle = mPressInfo.touchOrientation;
                    LOG_I(LOG_TAG, "[%s] notifyTouch DELMAR_SENSOR_UI_TYPE_ELLIPSE ", __func__);
                }else {
                    LOG_E(LOG_TAG, "[%s] notifyTouch err  ", __func__);
                }
                LOG_D(LOG_TAG, "[%s] notifyTouch Down type=%d, pressX=%d ,pressY=%d ", __func__, type, message.data.tp_info.x,message.data.tp_info.y );
                LOG_D(LOG_TAG, "[%s] notifyTouch Down touch_major=%d, touch_minor=%d ,touch_angle=%d ", __func__, message.data.tp_info.touch_major, message.data.tp_info.touch_minor, message.data.tp_info.touch_angle);
            }
*/
            message.type = type;
            mNotify(&message);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::notifyEnrollProgress(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        uint16_t samplesRemaining = context->enroll_cmd->o_samples_remaining;
        uint64_t start_enroll_end_time = 0;
        FUNC_ENTER();

        do {
            err = FingerprintCore::notifyEnrollProgress(context);
            if (0 == samplesRemaining) {
                start_enroll_end_time = HalUtils::getCurrentTimeMicrosecond();
                LOG_D(LOG_TAG, "[%s] start_enroll_end_time: %ds", __func__, (uint32_t)(start_enroll_end_time / 1000 / 1000));
                gf_delmar_enroll_time_flag_t cmd = {{0}};
                cmd.header.cmd_id = GF_CMD_ALGO_SAVE_PARAMS;
                cmd.header.target = GF_TARGET_ALGO;
                cmd.enroll_time = start_enroll_end_time;
                err = invokeCommand(&cmd, sizeof(cmd));
                GF_ERROR_BREAK(err);
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::prepareAuthRequest() {
        gf_error_t err = GF_SUCCESS;

        err = DelmarFingerprintCore::prepareAuthRequest();
        return err;
    }
    gf_error_t CustomizedFingerprintCore::prepareEnrollRequest() {
        gf_error_t err = GF_SUCCESS;

        err = DelmarFingerprintCore::prepareEnrollRequest();
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAuthStop(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            err = DelmarFingerprintCore::onAuthStop(context);
            sendMessage(MSG_BIG_DATA_ENROLL_AUTH_END, mTotalTime, context->retry);
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::dumpKPI(const char *func_name) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        err = DelmarFingerprintCore::dumpKPI(func_name);

        do {
            // get kpi
            mTotalTime = mTotalKpiTime;
            memset((void*)&kpi, 0, sizeof(kpi));
            kpi.header.target = GF_TARGET_ALGO;
            kpi.header.cmd_id = GF_CMD_ALGO_KPI;
            (void)invokeCommand(&kpi, sizeof(gf_delmar_algo_kpi_t));
            sendMessage(MSG_BIG_DATA_DUMP_KPI, &kpi, sizeof(kpi));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onEnrollRequested(const void *hat, uint32_t gid,
                                                  uint32_t timeoutSec) {
        return DelmarFingerprintCore::onEnrollRequested(hat, gid, timeoutSec);
    }

    gf_error_t CustomizedFingerprintCore::onEnrollStart(EnrollContext *context) {
        return DelmarFingerprintCore::onEnrollStart(context);
    }

    gf_error_t CustomizedFingerprintCore::onBeforeEnrollCapture(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        if (mContext->mCenter->hasUpEvt()) {
            err = GF_ERROR_TOO_FAST;
        } else {
            err = wait_for_finger_ready();
        }
        if (err == GF_SUCCESS) {
            setCpubw();
            #ifdef FP_HYPNUSD_ENABLE
            hypnus_request_change(ACTION_TYPE, ACTION_TIMEOUT_500);
            #endif
        }
        context->result = err;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAfterEnrollCapture(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        err = DelmarFingerprintCore::onAfterEnrollCapture(context);
        if (GF_SUCCESS != err) {
            releaseCpubw();
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAfterEnrollAlgo(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        err = DelmarFingerprintCore::onAfterEnrollAlgo(context);
        releaseCpubw();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAuthRequested(uint64_t operationId,
                                                uint32_t gid) {
        Mutex::Autolock _l(mFpUpLock);
        return FingerprintCore::onAuthRequested(operationId, gid);
    }

    gf_error_t CustomizedFingerprintCore::onAuthStart(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;

        mTotalTime = HalUtils::getCurrentTimeMicrosecond();
        mMDMTime = 0;
        setCpubw();
#ifdef FP_HYPNUSD_ENABLE
        hypnus_request_change(ACTION_TYPE, ACTION_TIMEOUT_500);
#endif
#ifdef FP_TRACE_DEBUG
        atrace_begin(ATRACE_TAG, "fp_set_dim_layer");
#endif  // FP_TRACE_DEBUG
        //fp_set_dim_layer(2);
#ifdef FP_TRACE_DEBUG
        atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG
        time_start = HalUtils::getCurrentTimeMicrosecond();
#ifdef FP_TRACE_DEBUG
        atrace_begin(ATRACE_TAG, "onAuthStart");
#endif  // FP_TRACE_DEBUG
        err = DelmarFingerprintCore::onAuthStart(context);
#ifdef FP_TRACE_DEBUG
        atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onBeforeAuthCapture(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        uint64_t time_cost = 0;

        FUNC_ENTER();
        time_cost = HalUtils::getCurrentTimeMicrosecond()- time_start;
#ifdef FP_TRACE_DEBUG
        atrace_begin(ATRACE_TAG, "wait_for_ui_ready");
#endif  // FP_TRACE_DEBUG
        if (wait_for_finger_ready() != GF_SUCCESS) {
            err = GF_ERROR_UI_READY_TIMEOUT;
            context->result = err;
            return err;
        }
#ifdef FP_TRACE_DEBUG
        atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG

        if (0 != fp_read_aod_mode()) {
            if (time_cost < 35000) {
                usleep(35000 - time_cost);
            }
        }

#ifdef FP_TRACE_DEBUG
        atrace_begin(ATRACE_TAG, "capture_image");
#endif  // FP_TRACE_DEBUG
        err = DelmarFingerprintCore::onBeforeAuthCapture(context);
        if (mMDMTime == 0) {
            time_cost = ((uint32_t)(HalUtils::getCurrentTimeMicrosecond() - mTotalTime)) / 1000;
            mTotalTime = HalUtils::getCurrentTimeMicrosecond();
            if (time_cost > 0x3FF) {
               time_cost = 0x3FF;
            }
            mMDMTime = time_cost & 0x3FF;
        }

        if (mContext->mCenter->hasUpEvt()) {
            err = GF_ERROR_TOO_FAST;
            context->result = err;
            return err;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::notifyAuthSuccess(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
#ifdef FP_TRACE_DEBUG
        atrace_begin(ATRACE_TAG, "notifyAuthSuccess");
#endif  // FP_TRACE_DEBUG
        if (mAuthSuccessNotified == 1) {
            return err;
        }
/*
        if (context->retry == 1) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_BEGIN_RETRY);
        } else if (context->retry == 3) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_END_RETRY);
        }
*/
        err = DelmarFingerprintCore::notifyAuthSuccess(context);
        //fp_set_tpirq_enable(2);
        //fp_set_dim_layer(5);
#ifdef FP_TRACE_DEBUG
        atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAfterAuthRetry(AuthenticateContext *context) {
#ifdef FP_TRACE_DEBUG
        atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG
        notifyMDM(context);
        releaseCpubw();
        return context->result;
    }

    gf_error_t CustomizedFingerprintCore::notifyMDM(AuthenticateContext *context){
        uint32_t time_cost = ((uint32_t)(HalUtils::getCurrentTimeMicrosecond()- mTotalTime)) / 1000;
        gf_error_t err = GF_SUCCESS;
        if (time_cost > 0xFFF) {
           time_cost = 0xFFF;
        }
        mMDMTime = (time_cost << 10) | mMDMTime;
        mMDMTime = (context->retry << 22) | mMDMTime;
        uint32_t error_code = 0;
        if (context->result != 0) {
            error_code = context->result - 1000 > 0xFF ? 0XFF : (uint32_t)(context->result - 1000) & 0xFF;
        }
        mMDMTime = (error_code << 24) | mMDMTime;
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        if (nullptr != mNotify) {
            LOG_D(LOG_TAG, "[%s] notifyMDM = 0x%x", __func__, mMDMTime);
            message.type = GF_FINGERPRINT_MDM;
            message.data.acquired.acquired_info = (gf_fingerprint_acquired_info_t) mMDMTime;
            mNotify(&message);
        }
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAfterAuthSuccess(AuthenticateContext *context) {
        if ((mStatus & 0x04) && isAuthDownDetected()) {
            ((CustomizedDevice*)mContext->mDevice)->startKeyTimer(1);
        }
        usleep(60*1000);
        return DelmarFingerprintCore::onAfterAuthSuccess(context);
    }

    gf_error_t CustomizedFingerprintCore::onAuthUpEvt() {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        Mutex::Autolock _l(mFpUpLock);
        if (isAuthDownDetected()) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_UP);
            mAuthDownDetected = false;
        }
        FUNC_EXIT(err);
        return err;
    }

    void CustomizedFingerprintCore::onError(gf_error_t err) {
        VOID_FUNC_ENTER();
        FingerprintCore:: onError(err);
        if (GF_ERROR_NOT_MATCH_NOT_LIVE_FINGER == err ||
            GF_ERROR_RESIDUAL_FINGER == err ||
            GF_ERROR_NOT_LIVE_FINGER == err) {
            //notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FAKE_FINGER);
        }
        VOID_FUNC_EXIT();
    }

    gf_error_t CustomizedFingerprintCore::cancel(bool notifyCancelMsg ) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        err = FingerprintCore::cancel(notifyCancelMsg);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::enrollPause() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::enrollPause();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::enrollResume() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::enrollResume();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::authenticatePause() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::authenticatePause();
        fp_set_tpirq_enable(0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::authenticateResume() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::authenticateResume();
        fp_set_tpirq_enable(1);
        FUNC_EXIT(err);
        return err;
    }

    int32_t CustomizedFingerprintCore::updateStatus(uint32_t status) {
        LOG_D(LOG_TAG, "status %d", status);
        switch (status) {
            case 0:
            case 2: {
                mStatus = mStatus & 0xfc;
                break;
            }
            case 1: {
                mStatus = mStatus | 0x01;
                break;
            }
            case 3: {
                mStatus = mStatus | 0x04;
                break;
            }
            case 4: {
                mStatus = mStatus & 0xfb;
                break;
            }
            case 0x08: {
                enrollResume();
                break;
            }
            case 0x09: {
                enrollPause();
                break;
            }
            case 0x0B: {
                authenticateResume();
                break;
            }
            case 0x0C: {
                authenticatePause();
                break;
            }
            default: {
                LOG_D(LOG_TAG, "%s unkown status", __func__);
                break;
            }
        }
        return 0;
    }

    int32_t CustomizedFingerprintCore::setFingerReady() {
        int32_t err = 0;
        LOG_D(LOG_TAG, "setFingerReady");
        err = notify_finger_ready();
        return err;
    }

    gf_error_t CustomizedFingerprintCore::notifyAuthUpEvt() {
        gf_error_t err = GF_SUCCESS;
        LOG_V(LOG_TAG, "notifyAuthUpEvt");
        onAuthUpEvt();
        return err;
    }

    gf_error_t CustomizedFingerprintCore::setFingerState(uint32_t state) {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();

        switch (state) {
            case 0: {
                // mContext->mCenter->postEvent(EVENT_FINGER_DOWN);
                fp_set_tpirq_enable(1);  // touch hold enable again
                break;
            }
            case 1: {
                //mContext->mCenter->postEvent(EVENT_FINGER_UP);
                break;
            }
            case 2: {
                mContext->mCenter->postEvent(EVENT_SCREEN_ON);
                break;
            }
            case 3: {
                mContext->mCenter->postEvent(EVENT_SCREEN_OFF);
                break;
            }
            default: {
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    bool CustomizedFingerprintCore::customizedIsAuthDownDetected() {
        return isAuthDownDetected();
    }

    void CustomizedFingerprintCore::onAuthError(AuthenticateContext *context) {
        FingerprintCore::onAuthError(context);
    }

    void CustomizedFingerprintCore::onEnrollError(EnrollContext *context) {
        VOID_FUNC_ENTER();
        if (context->result == GF_ERROR_NEED_CANCLE_ENROLL) {
            cancel(false);
            notifyErrorInfo(GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
        } else {
            FingerprintCore::onEnrollError(context);
        }
        VOID_FUNC_EXIT();
    }

    gf_error_t CustomizedFingerprintCore::send_auth_dcsmsg(AuthenticateContext* context, bool notified) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (notified != true) {
            LOG_D(LOG_TAG, "[%s] no need to add this result to dcs. result = %d", __func__, context->result);
            return GF_SUCCESS;
        }

        gf_delmar_algo_kpi_t kpi = { { 0 } };
        kpi.header.target = GF_TARGET_ALGO;
        kpi.header.cmd_id = GF_CMD_ALGO_KPI;
        err = invokeCommand(&kpi, sizeof(gf_delmar_algo_kpi_t));

        if (GF_SUCCESS != err) {
            LOG_E(LOG_TAG, "[%s] err = %d", __func__, err);
            return err;
        }
        gf_delmar_algo_performance_t *dump_performance = &kpi.o_performance;
        // REMARK:OPLUS particular send dcsmsg
        notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_GOOD);
        if (mNotify != nullptr) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            message.type = GF_FINGERPRINT_AUTHENTICATED_DCSSTATUS;
            message.data.auth_dcsmsg.auth_result = (context->result == GF_SUCCESS)? 1: 0;
            message.data.auth_dcsmsg.fail_reason = context->result;
            message.data.auth_dcsmsg.quality_score = dump_performance->pieces[0][0].image_quality[0];
            message.data.auth_dcsmsg.match_score = dump_performance->pieces[0][0].match_score[0];//invalid
            message.data.auth_dcsmsg.signal_value = 0;//invalid
            message.data.auth_dcsmsg.img_area = dump_performance->pieces[0][0].valid_area[0];//invalid
            message.data.auth_dcsmsg.retry_times = context->retry;
            memcpy(message.data.auth_dcsmsg.algo_version, "v03.01.03.25_DD306_SAMSUNG", sizeof(mALGO_VERSION)/sizeof(char));
            message.data.auth_dcsmsg.chip_ic = 9678;//invalid
            message.data.auth_dcsmsg.module_type = 0;//invalid
            message.data.auth_dcsmsg.lense_type = 0;//invalid
            message.data.auth_dcsmsg.dsp_availalbe = 0;//invalid
            message.data.auth_dcsmsg.auth_total_time = (uint32_t)(mTotalKpiTime / 1000);
            message.data.auth_dcsmsg.ui_ready_time = FingerprintCore::uiReadyTime / 1000;
            message.data.auth_dcsmsg.capture_time = dump_performance->get_raw_data_time / 1000;
            message.data.auth_dcsmsg.preprocess_time = dump_performance->pieces[0][0].preprocess_time / 1000;
            message.data.auth_dcsmsg.get_feature_time = dump_performance->pieces[0][0].get_feature_time[0] / 1000;
            message.data.auth_dcsmsg.auth_time= dump_performance->pieces[0][0].authenticate_time[0]/1000;
            message.data.auth_dcsmsg.detect_fake_time = dump_performance->detect_fake_time / 1000;

            LOG_D(LOG_TAG, "[%s] Auth, fail_reason = %d", __func__, message.data.auth_dcsmsg.fail_reason);
            LOG_D(LOG_TAG, "[%s] Auth, quality_score = %d", __func__, message.data.auth_dcsmsg.quality_score);
            LOG_D(LOG_TAG, "[%s] Auth, match_score = %d", __func__,  message.data.auth_dcsmsg.match_score);
            LOG_D(LOG_TAG, "[%s] Auth, signal_value = %d", __func__,  message.data.auth_dcsmsg.signal_value);
            LOG_D(LOG_TAG, "[%s] Auth, img_area = %d", __func__, message.data.auth_dcsmsg.img_area);
            LOG_D(LOG_TAG, "[%s] Auth, retry_times = %d", __func__, message.data.auth_dcsmsg.retry_times);
            LOG_D(LOG_TAG, "[%s] Auth, algo_version = %s", __func__, message.data.auth_dcsmsg.algo_version);
            LOG_D(LOG_TAG, "[%s] Auth, chip_ic = %d", __func__,  message.data.auth_dcsmsg.chip_ic);
            LOG_D(LOG_TAG, "[%s] Auth, module_type = %d", __func__, message.data.auth_dcsmsg.module_type);
            LOG_D(LOG_TAG, "[%s] Auth, lense_type = %d", __func__, message.data.auth_dcsmsg.lense_type);
            LOG_D(LOG_TAG, "[%s] Auth, dsp_available = %d", __func__, message.data.auth_dcsmsg.dsp_availalbe);

            LOG_D(LOG_TAG, "[%s] Auth, auth_total_time = %d", __func__, message.data.auth_dcsmsg.auth_total_time);
            LOG_D(LOG_TAG, "[%s] Auth, ui_ready_time = %d", __func__, message.data.auth_dcsmsg.ui_ready_time);
            LOG_D(LOG_TAG, "[%s] Auth, capture_time = %d", __func__, message.data.auth_dcsmsg.capture_time);
            LOG_D(LOG_TAG, "[%s] Auth, preprocess_time = %d", __func__, message.data.auth_dcsmsg.preprocess_time);
            LOG_D(LOG_TAG, "[%s] Auth, get_feature_time = %d", __func__, message.data.auth_dcsmsg.get_feature_time);
            LOG_D(LOG_TAG, "[%s] Auth, auth_time = %d", __func__, message.data.auth_dcsmsg.auth_time);
            LOG_D(LOG_TAG, "[%s] Auth, detect_fake_time = %d", __func__, message.data.auth_dcsmsg.detect_fake_time);
            mNotify(&message);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedFingerprintCore::getAlgoVersion() {
        gf_error_t err = GF_SUCCESS;
        int ret = 0;
        char lcdtype_prop[PROPERTY_VALUE_MAX] = {0};
        FUNC_ENTER();
        memcpy(mALGO_VERSION, algo_package_version, sizeof(algo_package_version)/sizeof(char));
        ret = property_get(PROPERTY_SCREEN_TYPE, lcdtype_prop, "ONEPLUS");
        if(ret){
            strcat(mALGO_VERSION, "_R_");
            strncpy(mALGO_VERSION + strlen(mALGO_VERSION), lcdtype_prop, strlen(lcdtype_prop));
            LOG_E(LOG_TAG, "[%s] lcdtype_prop = %s", __func__, lcdtype_prop);
        }else {
            LOG_E(LOG_TAG, "[%s] no lcdtype_prop", __func__);
        }
        property_set(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, mALGO_VERSION);
        LOG_E(LOG_TAG, "[%s] ALGO_VERSION: %s.", __func__, mALGO_VERSION);
        FUNC_EXIT(err);
        return err;
     }
}  // namespace goodix
