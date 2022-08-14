/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define _FINGERPRINTCORE_CPP_
#define LOG_TAG "[GF_HAL][FingerprintCore]"

#include <string.h>
#include <endian.h>
#include "gf_fpcore_types.h"
#include "gf_algo_types.h"
#include "FingerprintCore.h"
#include "HalLog.h"
#include "Timer.h"
#include "Sensor.h"
#include "Device.h"
#include "SensorDetector.h"
#include "Algo.h"
#include "HalContext.h"
#include "HalUtils.h"
#include "gf_global_define.h"
#include "DelmarProductTestDefine.h"
#include "gf_delmar_product_test_types.h"
#include "ProductTest.h"
#ifdef SUPPORT_DSP_HAL
#include "HalDsp.h"
#endif   // SUPPORT_DSP_HAL


#define OPLUS_ENROLL_TIME_OUT (10 * 60 * 1000)
#define ENROLL_ERROR_MAX (10)
#define IMAGE_PASS_SCORE (20)
static gf_fingerprint_notify_t factory_notify;


extern goodix::FingerprintCore::hypnus_state_t hypnus_state;


namespace goodix {

    FingerprintCore::FingerprintCore(HalContext *context) :
        HalBase(context),
        mNotify(nullptr),
        mFidoNotify(nullptr),
        mTotalKpiTime(0),
        mFailedAttempts(0),
        mAuthDownDetected(false),
        mWorkState(STATE_IDLE),
        mEnrollTimerSec(0),
        mEnrollTimer(nullptr),
        mAuthType(AUTH_TYPE_LOCK),
        mScreenStatus(0),
        mGid(0),
        enrollErrorCount(0) {
        memset(mFpDataPath, 0, sizeof(mFpDataPath));
    }

    FingerprintCore::~FingerprintCore() {
        mContext->mCenter->deregisterHandler(this);
    }

    static gf_error_t notifyFingerprintCmd(int64_t devId, int32_t msgId, int32_t cmdId, const int8_t *result, int32_t resultLen) {
        UNUSED_VAR(devId);
        UNUSED_VAR(msgId);
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] enter notifyFingerprintCmd, return GF_SUCCESS cmdId = %d", __func__, cmdId);

        if( (cmdId != PRODUCT_TEST_CMD_SPI) &&
            (cmdId != PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN) &&
            (cmdId != PRODUCT_TEST_CMD_OTP_FLASH) &&
            (cmdId != PRODUCT_TEST_CMD_PERFORMANCE_TESTING) &&
            (cmdId != PRODUCT_TEST_CMD_AGE_START) &&
            (cmdId != PRODUCT_TEST_CMD_AGE_START) &&
            (cmdId != PRODUCT_TEST_CMD_IMAGE_QUALITY) &&
            (cmdId != PRODUCT_TEST_CMD_GET_VERSION) &&
            (cmdId != PRODUCT_TEST_CMD_CAPTURE_IMAGE)&&
            (cmdId != PRODUCT_TEST_CMD_GET_OTP_QRCODE)) {
            LOG_E(LOG_TAG, "[%s] cmdId = %d, cmdId is not PRODUCT_TEST_CMD, do nothing!!!", __func__, cmdId);
            return GF_SUCCESS;
        }

        if (factory_notify != nullptr) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));

            switch (cmdId) {
                case PRODUCT_TEST_CMD_IMAGE_QUALITY: {
                    //decode image quality test error code and image quality
                    uint32_t error_code = GF_SUCCESS;
                    uint32_t image_quality = 0;
                    memcpy(&error_code, result + 4, sizeof(uint32_t));
                    if (GF_SUCCESS == error_code) {
                        memcpy(&image_quality, result + 12, sizeof(uint32_t));
                    }
                    message.type = GF_FINGERPRINT_ENGINEERING_INFO;
                    message.data.engineering.type = GF_FINGERPRINT_IMAGE_QUALITY;
                    message.data.engineering.quality.successed = (error_code == GF_SUCCESS ? 1 : 0);
                    message.data.engineering.quality.image_quality = image_quality;
                    message.data.engineering.quality.quality_pass = (message.data.engineering.quality.image_quality > IMAGE_PASS_SCORE ? 1 : 0);

                    LOG_E(LOG_TAG, "[%s] GF_FINGERPRINT_ENGINEERING_INFO, cmdId = %d, error_code = %d, resultLen = %d, image_quality = %d",
                            __func__, cmdId, error_code, resultLen, image_quality);
                    break;
                }

                default: {
                    message.type = GF_FINGERPRINT_OPTICAL_SENDCMD;
                    message.data.test.cmd_id = cmdId;
                    message.data.test.result = (int8_t *) result;
                    message.data.test.result_len = resultLen;
                    break;
                }
            }

            LOG_D(LOG_TAG, "[%s] factory_notify success, cmdId = %d", __func__, cmdId);
            factory_notify(&message);
        }
        FUNC_EXIT(err);
        return err;
    }


    gf_error_t FingerprintCore::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        doCancel();

        if (mContext != NULL) {
            mProductTest = createProductTest(mContext);
            mProductTest->setNotify((ext_module_callback)notifyFingerprintCmd);
            LOG_I(LOG_TAG, "[%s] set  notifyFingerprintCmd callback", __func__);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::reInit() {
        gf_error_t err = GF_SUCCESS;

        do {
            err = init();

            if (err != GF_SUCCESS) {
                break;
            }

            err = setActiveGroup(mGid, mFpDataPath);
        } while (0);

        return err;
    }

    gf_error_t FingerprintCore::setActiveGroup(uint32_t gid, const char *path) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] setActiveGroup gid =%d", __func__, gid);
        Mutex::Autolock _l(mContext->mHalLock);
        gf_set_active_group_t cmd = {{ 0 }};
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_SETACTIVITY_GROUP;
        cmd.i_group_id = gid;

        if (path != NULL
            && strlen(path) > 0
            && (strlen(path) <= (sizeof(cmd.i_path) - 1))
            && (strlen(path) <= (sizeof(mFpDataPath) - 1))) {
            memcpy(cmd.i_path, path, strlen(path) + 1);
            memcpy(mFpDataPath, path, strlen(path) + 1);
        }

        mGid = gid;
        err = invokeCommand(&cmd, sizeof(cmd));
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::setNotify(gf_fingerprint_notify_t notify) {
        gf_error_t err = GF_SUCCESS;
        mNotify = notify;
        factory_notify = notify;
        return err;
    }

    gf_error_t FingerprintCore::setFidoNotify(gf_fingerprint_notify_t notify) {
        gf_error_t err = GF_SUCCESS;
        mFidoNotify = notify;
        return err;
    }

    uint64_t FingerprintCore::preEnroll() {
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mContext->mHalLock);
        gf_pre_enroll_cmd_t cmd = {{ 0 }};
        FUNC_ENTER();
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_PRE_ENROLL;
        err = invokeCommand(&cmd, sizeof(cmd));

        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] Some wrong happend in pre enroll", __func__);
        }
        mPrenrollTime = HalUtils::getrealtime();

        FUNC_EXIT(err);
        return cmd.o_challenge;
    }

    gf_error_t FingerprintCore::onEnrollRequested(const void *hat, uint32_t gid,
                                                  uint32_t timeoutSec) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(hat);
        UNUSED_VAR(gid);
        UNUSED_VAR(timeoutSec);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::enroll(const void *hat, uint32_t gid,
                                       uint32_t timeoutSec) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] Enroll.", __func__);
        Mutex::Autolock _l(mContext->mHalLock);

        do {
            if ((err = prepareEnrollRequest()) != GF_SUCCESS) {
                // if GF_ERROR_CALIBRATION_NOT_READY, return it directly.
                if (err != GF_ERROR_CALIBRATION_NOT_READY && err != GF_ERROR_SENSOR_NOT_AVAILABLE) {
                    err = GF_ERROR_CANCELED;
                }
                break;
            }
            
            err = mContext->mDevice->enable_tp(GF_TP_ENABLE);
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] enable_tp fail, err code : %d.", __func__, err);
                break;
            }
            //reset lasttouchmode to touchup
            err = mContext->mDevice->clear_kernel_touch_flag();
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] clear_kernel_touch_flag fail, err code : %d.", __func__, err);
                break;
            }
            doCancel();
            gf_enroll_cmd_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_ENROLL;
            cmd.i_group_id = gid;
            cmd.i_system_auth_token_version = GF_HW_AUTH_TOKEN_VERSION;

            if (NULL != hat) {
                memcpy(&cmd.i_auth_token, hat, sizeof(gf_hw_auth_token_t));
            }

            err = invokeCommand(&cmd, sizeof(cmd));

            if (err != GF_SUCCESS) {
                cancelEnrollTimer();
#if defined(__ANDROID_O) || defined(__ANDROID_P) || defined(__ANDROID_Q) || defined(__ANDROID_R)

                // support android o vts
                if (GF_ERROR_INVALID_CHALLENGE == err || GF_ERROR_INVALID_HAT_VERSION == err || GF_ERROR_UNTRUSTED_ENROLL == err) {
                    LOG_E(LOG_TAG, "[%s] hardware unavailable", __func__);
                    notifyErrorInfo(GF_FINGERPRINT_ERROR_HW_UNAVAILABLE);
                    err = GF_SUCCESS;
                }

#endif  // __ANDROID_O
                break;
            }

            mContext->mCenter->registerHandler(this);
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT);
            mWorkState = STATE_ENROLL;
            onEnrollRequested(hat, gid, timeoutSec);
            sendMessage(MsgBus::MSG_ENROLL_REQUESTED);
        } while (0);
        int64_t now = HalUtils::getrealtime();
        if ((now > mPrenrollTime) && (now - mPrenrollTime > OPLUS_ENROLL_TIME_OUT)) {
            startEnrollTimer(1);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::prepareEnrollRequest() {
        return GF_SUCCESS;
    }

    gf_error_t FingerprintCore::onEnrollStart(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeEnrollCapture(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        if (mContext->mConfig->support_ui_ready) {
            if (!mContext->mSensor->waitSensorUIReady()) {
                err = mContext->mCenter->hasUpEvt() ? GF_ERROR_TOO_FAST : GF_ERROR_UI_READY_TIMEOUT;
            }
        }
        context->result = err;
        return err;
    }

    gf_error_t FingerprintCore::onAfterEnrollCapture(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeEnrollAlgo(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onAfterEnrollAlgo(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onAfterEnrollNotify(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onEnrollStop(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    bool FingerprintCore::checkFingerLeave() {
        return mContext->mCenter->hasUpEvt();
    }

    bool FingerprintCore::waitDspSetSpeed(uint8_t operationCmd) {
        bool set_speed_complete = false;
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(operationCmd);
        FUNC_ENTER();
#ifdef SUPPORT_DSP_HAL
        HalDsp* dsp = mContext->getDsp();
        if (nullptr != dsp && DSP_AVAILABLE == dsp->checkDspValid()) {
            if ((operationCmd == GF_OPERATION_ENROLL && mContext->isDSPEnroll()) ||
                operationCmd == GF_OPERATION_AUTHENTICATE) {
                err = dsp->waitDspNotify();
                set_speed_complete = true;
            } else {
                LOG_E(LOG_TAG, "[%s] no need wait dsp!", __func__);
            }
        }
#endif   // SUPPORT_DSP_HAL
        FUNC_EXIT(err);
        return set_speed_complete;
    }

    gf_error_t FingerprintCore::setDspSpeed(uint8_t operationCmd, uint8_t SpeedMode) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(operationCmd);
        UNUSED_VAR(SpeedMode);
#ifdef SUPPORT_DSP_HAL
        gf_dsp_cmd_t speedModelCmd;
        //  0: dspSpeedStatus is normal speed status, only exec set high speed
        //  1: dspSpeedStatus is high speed status, only exec set normal speed
        static uint8_t dspSpeedStatus = 0;
        HalDsp* dsp = mContext->getDsp();
        FUNC_ENTER();
        do {
            if (dspSpeedStatus == 0 && SpeedMode == 1) {
                dspSpeedStatus = 1;
                speedModelCmd = DSP_CMD_SET_HIGH_SPEED;
            } else if (dspSpeedStatus == 1 && SpeedMode == 0) {
                dspSpeedStatus = 0;
                speedModelCmd = DSP_CMD_SET_NORMAL_SPEED;
            } else {
                break;
            }
            if (nullptr != dsp &&
                (DSP_AVAILABLE == dsp->checkDspValid() || (SpeedMode == 0 && mContext->isDSPEnabled()))) {
                if ((operationCmd == GF_OPERATION_ENROLL && mContext->isDSPEnroll()) ||
                        operationCmd == GF_OPERATION_AUTHENTICATE) {
                    dsp->sendCmdToDsp(speedModelCmd);
                    LOG_D(LOG_TAG, "[%s] set DSP SpeedMode:%d", __func__, SpeedMode);
                }
            } else {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] DSP unavailable, set DSP speed fail!", __func__);
            }
        } while (0);
        FUNC_EXIT(err);
#endif   // SUPPORT_DSP_HAL
        return err;
    }

    gf_error_t FingerprintCore::onEnrollDownEvt() {
        gf_error_t err = GF_SUCCESS;
        EnrollContext context;
        gf_algo_enroll_image_t* cmd = mContext->mAlgo->createEnrollCmd();
        FUNC_ENTER();

#ifdef SUPPORT_DSP_HAL
        bool is_wait_set_speed_complete = false;
        setDspSpeed(GF_OPERATION_ENROLL, 1);
#endif   // SUPPORT_DSP_HAL
        do {
            GF_NULL_BREAK(cmd, err);
            context.enroll_cmd = cmd;

            if (mContext->mConfig->support_performance_dump) {
                context.fingerDownTime = HalUtils::getCurrentTimeMicrosecond();
                mTotalKpiTime = 0;
            }

            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_DOWN);
            sendMessage(MsgBus::MSG_ENROLL_START);
            onEnrollStart(&context);
            // 1. capture image
            onBeforeEnrollCapture(&context);
            GF_ERROR_BREAK(context.result);
            err = mContext->mSensor->captureImage(GF_OPERATION_ENROLL, 0);
            context.result = err;
            if (context.result != GF_SUCCESS) {
                onEnrollError(&context);
                checkEnrollResult(err);
                break;
            }

            err = onAfterEnrollCapture(&context);
            if (context.result != GF_SUCCESS) {
                onEnrollError(&context);
                checkEnrollResult(err);
                break;
            }
            // 2. algo enroll
            onBeforeEnrollAlgo(&context);
            LOG_D(LOG_TAG, "[%s] set enroll stage:%d", __func__, context.enroll_cmd->header.reserved[11]);
#ifdef SUPPORT_DSP_HAL
            if (is_wait_set_speed_complete == false) {
                is_wait_set_speed_complete = waitDspSetSpeed(GF_OPERATION_ENROLL);
            }
#endif   // SUPPORT_DSP_HAL
            err = mContext->mAlgo->enrollImage(context.enroll_cmd);
            if (context.enroll_cmd->o_antipeep_screen_struct_flag) {
                LOG_D(LOG_TAG, "[%s] enroll: antipeep & screen struct flag : %d (1:ANTIPEEPING, 2:SCREEN_STRUCT)", __func__, context.enroll_cmd->o_antipeep_screen_struct_flag);
            }
            context.result = err;
            onAfterEnrollAlgo(&context);
            checkEnrollResult(err);

            if (context.result != GF_SUCCESS) {
                onEnrollError(&context);
                break;
            }

            // 3. notify
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_GOOD);

            if (mContext->mConfig->support_performance_dump) {
                mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - context.fingerDownTime;
            }

            if (cmd->o_samples_remaining == 0) {
                // enroll success save template
                err = saveTemplates(cmd->o_gid, cmd->o_finger_id);

                if (err != GF_SUCCESS) {
                    notifyErrorInfo(GF_FINGERPRINT_ERROR_NO_SPACE);
                    break;
                } else {
                    if (mWorkState != STATE_ENROLL) {
                        err = removeTemplates(cmd->o_gid, cmd->o_finger_id);

                        if (err != GF_SUCCESS) {
                            LOG_E(LOG_TAG, "[%s] Remove template error.", __func__);
                        }
                    } else {
                        sendMessage(MsgBus::MSG_ENROLL_SAVE_TEMPLATE_END);
                        GF_SWITCH_NEXT_FINGER
                    }
                }

                context.result = err;
                notifyEnrollProgress(&context);
                if (err == GF_SUCCESS) {
                    enumerate();
                }
                doCancel();
            } else {
                notifyEnrollProgress(&context);
                //wakeEnrollTimer();
            }

            onAfterEnrollNotify(&context);
        } while (0);

#ifdef SUPPORT_DSP_HAL
        if (is_wait_set_speed_complete == false) {
            waitDspSetSpeed(GF_OPERATION_ENROLL);
        }
        // 3. set dsp to normal status
        setDspSpeed(GF_OPERATION_ENROLL, 0);
#endif   // SUPPORT_DSP_HAL

        sendMessage(MsgBus::MSG_ENROLL_END, &(context.result), sizeof(gf_error_t));
        context.result = err;
        onEnrollStop(&context);
        mContext->mAlgo->destroyEnrollCmd(cmd);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onEnrollUpEvt() {
        VOID_FUNC_ENTER();
        notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_UP);
        VOID_FUNC_EXIT();
        return GF_SUCCESS;
    }

    void FingerprintCore::startEnrollTimer(uint32_t timeoutSec) {
        VOID_FUNC_ENTER();

        do {
            gf_error_t err = GF_SUCCESS;

            if (NULL == mEnrollTimer) {
                mEnrollTimer = Timer::createTimer((timer_thread_t) enrollTimerThread, this);

                if (NULL == mEnrollTimer) {
                    LOG_E(LOG_TAG, "[%s] create enroll timer failed", __func__);
                    break;
                }
            }

            mEnrollTimerSec = timeoutSec;
            err = mEnrollTimer->set(timeoutSec, 0, timeoutSec, 0);

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] start enroll timer failed", __func__);
            }
        } while (0);

        VOID_FUNC_EXIT();
    }

    void FingerprintCore::cancelEnrollTimer() {
        VOID_FUNC_ENTER();

        if (mEnrollTimer != NULL) {
            delete mEnrollTimer;
            mEnrollTimer = NULL;
        }

        mEnrollTimerSec = 0;
        VOID_FUNC_EXIT();
    }

    void FingerprintCore::wakeEnrollTimer() {
        VOID_FUNC_ENTER();

        if (mEnrollTimer != NULL) {
            gf_error_t err = mEnrollTimer->set(mEnrollTimerSec, 0, mEnrollTimerSec, 0);

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] wake enroll timer failed", __func__);
            }
        }

        VOID_FUNC_EXIT();
    }

    void FingerprintCore::enrollTimerThread(union sigval v) {
        VOID_FUNC_ENTER();

        do {
            if (NULL == v.sival_ptr) {
                LOG_E(LOG_TAG, "[%s] invalid timer signal data", __func__);
                break;
            }

            FingerprintCore *auth = static_cast<FingerprintCore *>(v.sival_ptr);
            auth->notifyErrorInfo(GF_FINGERPRINT_ERROR_TIMEOUT);
            auth->cancel(false);
        } while (0);

        // TODO implement
        VOID_FUNC_EXIT();
    }

    gf_error_t FingerprintCore::postEnroll() {
        gf_error_t err = GF_SUCCESS;
        gf_post_enroll_cmd_t cmd = { 0 };
        FUNC_ENTER();
        cmd.target = GF_TARGET_BIO;
        cmd.cmd_id = GF_CMD_AUTH_POST_ENROLL;
        Mutex::Autolock _l(mContext->mHalLock);
        err = invokeCommand(&cmd, sizeof(cmd));
        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::onEnrollError(EnrollContext *context) {
        VOID_FUNC_ENTER();
        onError(context->result);
        VOID_FUNC_EXIT();
    }

    gf_error_t FingerprintCore::notifyEnrollProgress(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        uint32_t gid = context->enroll_cmd->o_gid;
        uint32_t fid = context->enroll_cmd->o_finger_id;
        uint16_t samplesRemaining = context->enroll_cmd->o_samples_remaining;
        LOG_D(LOG_TAG, "[%s] gid=%d, fid=%d, remaining=%d.", __func__, gid, fid,
              samplesRemaining);

        if (mNotify != nullptr) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            message.type = GF_FINGERPRINT_TEMPLATE_ENROLLING;
            message.data.enroll.finger.gid = gid;
            message.data.enroll.finger.fid = fid;
            message.data.enroll.samples_remaining = samplesRemaining;
            mNotify(&message);

            if (mContext->mConfig->support_performance_dump) {
                dumpKPI(__func__);
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::prepareAuthRequest() {
        return GF_SUCCESS;
    }

    gf_error_t FingerprintCore::onAuthRequested(uint64_t operationId,
                                                uint32_t gid) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(operationId);
        UNUSED_VAR(gid);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::authenticate(uint64_t operationId, uint32_t gid) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] Authenticate.", __func__);
        Mutex::Autolock _l(mContext->mHalLock);

        do {
            if (prepareAuthRequest() != GF_SUCCESS) {
                err = GF_ERROR_CANCELED;
                break;
            }

            err = mContext->mDevice->enable_tp(GF_TP_ENABLE);
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] enable_tp fail, err code : %d.", __func__, err);
                break;
            }
            err = mContext->mDevice->clear_kernel_touch_flag();
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] clear_kernel_touch_flag fail, err code : %d.", __func__, err);
                break;
            }

            doCancel();
            gf_authenticate_cmd_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_AUTHENTICATE;
            cmd.i_group_id = gid;
            cmd.i_operation_id = operationId;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            mContext->mCenter->registerHandler(this);
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT);
            mWorkState = STATE_AUTHENTICATE;
            mFailedAttempts = 0;
            mAuthDownDetected = false;
            mAuthResultNotified = false;
            onAuthRequested(operationId, gid);
            sendMessage(MsgBus::MSG_AUTHENTICATE_REQUESTED);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    bool FingerprintCore::needRetryIfCaptureFailed(AuthenticateContext* context) {
        UNUSED_VAR(context);
        return false;
    }

    gf_error_t FingerprintCore::onAuthDownEvt() {
        gf_error_t err = GF_SUCCESS;
        gf_algo_auth_image_t* cmd = mContext->mAlgo->createAuthCmd();
        AuthenticateContext context;
        context.result = err;
        context.auth_cmd = cmd;
        mAuthResultNotified = false;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] Auth Down.", __func__);
#ifdef SUPPORT_DSP_HAL
        bool is_wait_set_speed_complete = false;
        // 0. set Dsp in high freq
        setDspSpeed(GF_OPERATION_AUTHENTICATE, 1);
#endif   // SUPPORT_DSP_HAL
        do {
            GF_NULL_BREAK(cmd, err);
            if (mContext->mConfig->support_performance_dump) {
                context.fingerDownTime = HalUtils::getCurrentTimeMicrosecond();
                mTotalKpiTime = 0;
            }

            mAuthDownDetected = true;
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_DOWN);
            sendMessage(MsgBus::MSG_AUTHENTICATE_START);
            onAuthStart(&context);
            do {
                // 1. capture image
                onBeforeAuthCapture(&context);
                GF_ERROR_BREAK(context.result);
                LOG_D(LOG_TAG, "[%s] Begin capture after finger down %d ms.", __func__, (int32_t)((HalUtils::getCurrentTimeMicrosecond()- context.fingerDownTime) / 1000.0));
                err = mContext->mSensor->captureImage(GF_OPERATION_AUTHENTICATE, context.retry);
                context.result = err;
                onAfterAuthCapture(&context);
                if (needRetryIfCaptureFailed(&context)) {
                    LOG_D(LOG_TAG, "[%s] Retry with capture failed", __func__);
                    continue;
                }
                GF_ERROR_BREAK(context.result);
                // 2. algo authenticate
                onBeforeAuthAlgo(&context);
#ifdef SUPPORT_DSP_HAL
                if (is_wait_set_speed_complete == false) {
                    is_wait_set_speed_complete = waitDspSetSpeed(GF_OPERATION_AUTHENTICATE);
                }
#endif   // SUPPORT_DSP_HAL
                cmd->io_auth_token.version = GF_HW_AUTH_TOKEN_VERSION;
                cmd->io_auth_token.authenticator_type = htobe32(GF_HW_AUTH_FINGERPRINT);
                cmd->i_retry_count = context.retry;
                cmd->i_auth_type = mAuthType;
                err = mContext->mAlgo->authImage(cmd);
                if (cmd->o_antipeep_screen_struct_flag) {
                    LOG_D(LOG_TAG, "[%s] auth: antipeep & screen struct flag : %d (1:ANTIPEEPING, 2:SCREEN_STRUCT)", __func__, cmd->o_antipeep_screen_struct_flag);
                }
                context.result = err;
                LOG_D(LOG_TAG, "[%s] high light flag = %d", __func__, cmd->o_high_light);
                sendMessage(MsgBus::MSG_AUTHENTICATE_ALGO_END,  context.result, context.retry);
                onAfterAuthAlgo(&context);

                if (needRetry(&context)) {
                    sendMessage(MsgBus::MSG_AUTHENTICATE_RETRYING, context.result, context.retry);
                    context.retry++;
                    // retry authenticate
                    LOG_D(LOG_TAG, "[%s] Authenticate is retrying,", __func__);
                    continue;
                } else {
                    break;
                }
            } while (true);

            err = onAfterAuthRetry(&context);
            context.result = err;
            sendMessage(MsgBus::MSG_AUTHENTICATE_RETRY_END, context.result, context.retry);

            if (mContext->mConfig->support_performance_dump) {
                mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - context.fingerDownTime;
            }
            GF_ERROR_BREAK(context.result);

            // 3. notify
            notifyAuthSuccess(&context);
            mContext->mDevice->enable_tp(GF_TP_DISENABLE);
            // 4. do post something, such as study
            onAfterAuthSuccess(&context);
        } while (0);

#ifdef SUPPORT_DSP_HAL
        if (is_wait_set_speed_complete == false) {
            waitDspSetSpeed(GF_OPERATION_AUTHENTICATE);
        }
        // 3. set dsp to normal status
        setDspSpeed(GF_OPERATION_AUTHENTICATE, 0);
#endif   // SUPPORT_DSP_HAL

        onAuthStop(&context);

        if (err != GF_SUCCESS) {
            onAuthError(&context);
        }
        sendMessage(MsgBus::MSG_AUTHENTICATE_END,  context.result, context.retry);
        send_auth_dcsmsg(&context, mAuthResultNotified);
        mContext->mAlgo->destroyAuthCmd(cmd);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onAuthStart(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeAuthCapture(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        if (mContext->mConfig->support_ui_ready) {
            if (!mContext->mSensor->waitSensorUIReady()) {
                err = mContext->mCenter->hasUpEvt() ? GF_ERROR_TOO_FAST : GF_ERROR_UI_READY_TIMEOUT;
            }
        }
        context->result = err;
        return err;
    }

    gf_error_t FingerprintCore::onAfterAuthCapture(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeAuthAlgo(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onAfterAuthAlgo(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    bool FingerprintCore::needRetry(AuthenticateContext *context) {
        bool retry = false;
        int32_t maxRetry = mContext->mConfig->max_authenticate_failed_attempts;

        if (context->retry < maxRetry) {
            retry = (GF_ERROR_NOT_MATCH == context->result
                     && !checkFingerLeave());
        } else {
            retry = false;
        }

        return retry;
    }

    gf_error_t FingerprintCore::onAfterAuthRetry(AuthenticateContext *context) {
        return context->result;
    }

    gf_error_t FingerprintCore::onAfterAuthSuccess(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        gf_algo_auth_image_t *auth = context->auth_cmd;
        FUNC_ENTER();

        do {
            gf_auth_post_auth_t cmd = { { 0 } };
            cmd.i_retry_count = auth->i_retry_count;
            cmd.i_gid = auth->o_gid;
            cmd.i_finger_id = auth->o_finger_id;
            cmd.i_study_flag = auth->o_study_flag;
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_POST_AUTHENTICATE;
            err = invokeCommand(&cmd, sizeof(cmd));

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] authImageFinish fail", __func__);
                break;
            }

            if (cmd.o_save_flag > 0) {
                // teplate saved in ta
                sendMessage(MsgBus::MSG_AUTHENTICATE_SAVE_TEMPLATE_END);
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onAuthStop(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;

        if (GF_SUCCESS == context->result) {
            doCancel();
        }

        return err;
    }

    gf_error_t FingerprintCore::notifyAuthSuccess(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_GOOD);

        if (mNotify != nullptr) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            message.type = GF_FINGERPRINT_AUTHENTICATED;
            message.data.authenticated.finger.gid = context->auth_cmd->o_gid;
            message.data.authenticated.finger.fid = context->auth_cmd->o_finger_id;
            LOG_D(LOG_TAG, "[%s] Auth success, fid = %u", __func__,
                  context->auth_cmd->o_finger_id);
            memcpy(&message.data.authenticated.hat, &(context->auth_cmd->io_auth_token),
                   sizeof(gf_hw_auth_token_t));
            mNotify(&message);

            if (mContext->mConfig->support_performance_dump) {
                dumpKPI(__func__);
            }
        }

        mFailedAttempts = 0;
        mAuthResultNotified = true;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onAuthUpEvt() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (mAuthDownDetected) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_UP);
            mAuthDownDetected = false;
        }

        FUNC_EXIT(err);
        return err;
    }

    // overide IEventHandler
    gf_error_t FingerprintCore::onEvent(gf_event_type_t e) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] event = %d workstate = %d", __func__, e, mWorkState);

        if (mWorkState == STATE_ENROLL) {
            err = onEnrollReceivedEvt(e);
        } else if (mWorkState == STATE_AUTHENTICATE) {
            err = onAuthReceivedEvt(e);
        } else {
            LOG_D(LOG_TAG, "[%s] Event %d is ignored", __func__, e);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onEnrollReceivedEvt(gf_event_type_t event) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        switch (event) {
            case EVENT_FINGER_DOWN: {
                err = onEnrollDownEvt();
                break;
            }

            case EVENT_FINGER_UP: {
                err = onEnrollUpEvt();
                break;
            }

            case EVENT_IRQ_RESET: {
                break;
            }

            default: {
                LOG_D(LOG_TAG, "[%s] event %d is not handled.", __func__, event);
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::sensorIsBroken() {
        notifyErrorInfo(GF_FINGERPRINT_ERROR_HW_UNAVAILABLE);
    }

    void FingerprintCore::onError(gf_error_t err) {
        VOID_FUNC_ENTER();
        //notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_GOOD);
        switch (err) {
            case GF_ERROR_UI_DISAPPEAR:
            case GF_ERROR_UI_READY_TIMEOUT:
            case GF_ERROR_TOO_FAST: {
                LOG_E(LOG_TAG, "[%s] Up too fast.", __func__);

                if ((mWorkState == STATE_AUTHENTICATE) || (mWorkState == STATE_ENROLL)) {
                    notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
                }

                break;
            }

            case GF_ERROR_SENSOR_IS_BROKEN: {
                sensorIsBroken();
                break;
            }

            case GF_ERROR_ALGO_DIRTY_FINGER:
            case GF_ERROR_ALGO_COVER_BROKEN:
            case GF_ERROR_ACQUIRED_IMAGER_DIRTY: {
                notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                if ((mWorkState == STATE_AUTHENTICATE) && ( mScreenStatus != 0)) {
                    notifyAuthNotMatched();
                } else {
                    onAuthUpEvt();
                }
                break;
            }

            case GF_ERROR_SENSOR_BROKEN_CHECK_ALGO_ERROR:
            case GF_ERROR_ACQUIRED_PARTIAL: {
                notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_PARTIAL);
                if ((mWorkState == STATE_AUTHENTICATE) && ( mScreenStatus != 0)) {
                    notifyAuthNotMatched();
                } else {
                    onAuthUpEvt();
                }
                break;
            }

            case GF_ERROR_DUPLICATE_FINGER: {
                notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER);
                break;
            }

            case GF_ERROR_DUPLICATE_AREA: {
                notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA);
                break;
            }

            case GF_ERROR_SPI_RAW_DATA_CRC_FAILED: {
                notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                if (mWorkState == STATE_AUTHENTICATE) {
                    notifyAuthNotMatched();
                }
                break;
            }

            case GF_ERROR_NOT_MATCH: {
                notifyAuthNotMatched();
                break;
            }

            case GF_ERROR_NOT_MATCH_NOT_LIVE_FINGER:
            case GF_ERROR_RESIDUAL_FINGER:
            case GF_ERROR_NOT_LIVE_FINGER:
            case GF_ERROR_ACQUIRED_IMAGER_DRY: {
                notifyAuthNotMatched();
                break;
            }

            default: {
                notifyErrorInfo(GF_FINGERPRINT_ERROR_INVALID_DATA);
                break;
            }
        }

        VOID_FUNC_EXIT();
    }

    void FingerprintCore::notifyAuthNotMatched() {
        VOID_FUNC_ENTER();

        if (mNotify != nullptr) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            message.type = GF_FINGERPRINT_AUTHENTICATED;
            message.data.authenticated.finger.gid = 0;
            message.data.authenticated.finger.fid = 0;  // 0 is authenticate failed
            mNotify(&message);
        }

        mFailedAttempts++;
        mAuthResultNotified = true;

        if (mContext->mConfig->max_authenticate_failed_lock_out > 0
            && mFailedAttempts > mContext->mConfig->max_authenticate_failed_lock_out) {
#if defined(__ANDROID_O) || defined(__ANDROID_P) || defined(__ANDROID_Q) || defined(__ANDROID_R)
            notifyErrorInfo(GF_FINGERPRINT_ERROR_LOCKOUT);
#endif  // __ANDROID_O
        }

        if (mContext->mConfig->support_performance_dump) {
            dumpKPI(__func__);
        }

        VOID_FUNC_EXIT();
    }

    void FingerprintCore::onAuthError(AuthenticateContext *context) {
        gf_error_t result =  context->result;
        VOID_FUNC_ENTER();
        if (context->retry > 0 && (GF_ERROR_TOO_FAST == result
                    || GF_ERROR_UI_DISAPPEAR == result)) {
            result = GF_ERROR_NOT_MATCH;
        }
        onError(result);
        VOID_FUNC_EXIT();
    }

    gf_error_t FingerprintCore::onAuthReceivedEvt(gf_event_type_t event) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        switch (event) {
            case EVENT_FINGER_DOWN: {
                err = onAuthDownEvt();
                break;
            }

            case EVENT_FINGER_UP: {
                err = onAuthUpEvt();
                break;
            }

            case EVENT_IRQ_RESET: {
                err = onResetEvent();
                break;
            }

            default: {
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onResetEvent() {
        return GF_SUCCESS;
    }

    uint64_t FingerprintCore::getAuthenticatorId() {
        gf_error_t err = GF_SUCCESS;
        gf_get_auth_id_t cmd = {{ 0 }};
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_GET_ID;
        FUNC_ENTER();
        Mutex::Autolock _l(mContext->mHalLock);
        err = invokeCommand(&cmd, sizeof(cmd));
        FUNC_EXIT(err);
        return cmd.o_auth_id;
    }

    gf_error_t FingerprintCore::remove(uint32_t gid, uint32_t fid) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, gid, fid);
        Mutex::Autolock _l(mContext->mHalLock);
        WORK_STATE old = mWorkState;
        mWorkState = STATE_REMOVE;

        do {
            uint32_t remainingTemplates = 0;
            gf_remove_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_REMOVE;
            cmd.i_group_id = gid;
            cmd.i_finger_id = fid;
            err = invokeCommand(&cmd, sizeof(cmd));
            // android framework require success
            err = GF_SUCCESS;

            if (0 == cmd.o_removing_templates) {
                LOG_D(LOG_TAG, "[%s] no fingers are removed.", __func__);
#if defined(__ANDROID_O) || defined(__ANDROID_P) || defined(__ANDROID_Q) || defined(__ANDROID_R)
                remainingTemplates = 0;
                notifyRemove(gid, fid, remainingTemplates);
#endif  // __ANDROID_O
            } else {
                for (uint32_t i = 0; i < MAX_FINGERS_PER_USER
                     && 0 != cmd.o_deleted_fids[i]; i++) {
                    remainingTemplates = cmd.o_removing_templates - i - 1;
                    notifyRemove(gid, cmd.o_deleted_fids[i], remainingTemplates);
                    LOG_D(LOG_TAG, "[%s] remove finger. gid=%u, fid=%u, remaining_templates=%u",
                          __func__, gid, cmd.o_deleted_fids[i], cmd.o_removing_templates);
                }

                sendMessage(MsgBus::MSG_TEMPLATE_REMOVED, gid, fid);
            }

#ifdef __ANDROID_N
            notifyRemove(0, 0, remainingTemplates);
#endif  // __ANDROID_N
        } while (0);

        if (err == GF_SUCCESS) {
            enumerate();
        }

        mWorkState = old;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::removeTemplates(uint32_t gid, uint32_t fid) {
        gf_remove_t cmd = {{ 0 }};
        gf_error_t err = GF_SUCCESS;
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_REMOVE;
        cmd.i_group_id = gid;
        cmd.i_finger_id = fid;
        FUNC_ENTER();
        err = invokeCommand(&cmd, sizeof(cmd));

        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] Remove template error.", __func__);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::notifyRemove(uint32_t gid, uint32_t fid,
                                             uint32_t remainingTemplates) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        gf_fingerprint_msg_t message;
        UNUSED_VAR(remainingTemplates);
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_TEMPLATE_REMOVED;
        message.data.removed.finger.gid = gid;
        message.data.removed.finger.fid = fid;
#if defined(__ANDROID_O) || defined(__ANDROID_P) || defined(__ANDROID_Q) || defined(__ANDROID_R)
        message.data.removed.fingers_count = remainingTemplates;
#endif  // __ANDROID_O

        if (mNotify != nullptr) {
            mNotify(&message);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::enumerate() {
        gf_error_t err = GF_SUCCESS;
        gf_enumerate_t cmd = {{ 0 }};
        Mutex::Autolock _l(mContext->mHalLock);
        FUNC_ENTER();
        WORK_STATE old = mWorkState;
        mWorkState = STATE_ENUMERATE;
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_ENUMERATE;
        err = invokeCommand(&cmd, sizeof(cmd));
        notifyEnumerate(&cmd);
        mWorkState = old;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::notifyEnumerate(gf_enumerate_t *result) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        assert(result != nullptr);

        if (mNotify != nullptr) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            message.type = GF_FINGERPRINT_TEMPLATE_ENUMERATING;

            if (result->o_size == 0) {
                message.data.enumerated.gid = mGid;
                LOG_D(LOG_TAG, "[%s] gid=%d, template is null", __func__, mGid);
                mNotify(&message);
            } else {
                for (uint32_t i = 0; i < result->o_size; i++) {
                    message.data.enumerated.fingers[i].gid = result->o_group_ids[i];
                    message.data.enumerated.fingers[i].fid = result->o_finger_ids[i];
                    message.data.enumerated.remaining_templates = result->o_size;
                    message.data.enumerated.gid = result->o_group_ids[i];
                    LOG_D(LOG_TAG, "[%s] group_id[%d]=%u, finger_id[%d]=%u, remains=%u", __func__,
                          i, result->o_group_ids[i], i, result->o_finger_ids[i],
                            message.data.enumerated.remaining_templates);
                }
                mNotify(&message);
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::enumerate(void *result, uint32_t *maxSize) {
        gf_error_t err = GF_SUCCESS;
        uint32_t count = 0;
        uint32_t len = 0;
        gf_fingerprint_finger_id_t *results = (gf_fingerprint_finger_id_t *) result;
        FUNC_ENTER();
        Mutex::Autolock _l(mContext->mHalLock);

        do {
            if (results == nullptr || maxSize == nullptr) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            len = (mContext->mConfig->max_fingers_per_user) < *maxSize ?
                  (mContext->mConfig->max_fingers_per_user) : *maxSize;
            LOG_D(LOG_TAG, "[%s] size=%u", __func__, len);
            gf_enumerate_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_ENUMERATE;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);

            for (uint32_t i = 0; i < len; i++) {
                LOG_D(LOG_TAG, "[%s] group_id[%d]=%u, finger_id[%d]=%u", __func__, i,
                      cmd.o_group_ids[i], i, cmd.o_finger_ids[i]);

                if (cmd.o_finger_ids[i] != 0) {
                    results[count].gid = cmd.o_group_ids[i];
                    results[count].fid = cmd.o_finger_ids[i];
                    count++;
                }
            }
        } while (0);

        if (maxSize != nullptr) {
            *maxSize = count;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::cancel(bool notifyCancelMsg ) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(notifyCancelMsg);
        LOG_D(LOG_TAG, "[%s]Cancel received.", __func__);
        Mutex::Autolock _l(mContext->mHalLock);
        gf_cancel_cmd_t cmd = { 0 };
        cmd.target = GF_TARGET_BIO;
        cmd.cmd_id = GF_CMD_AUTH_CANCEL;
        err = invokeCommand(&cmd, sizeof(gf_cancel_cmd_t));
        sendMessage(MsgBus::MSG_CANCELED);
        doCancel();
        err = mContext->mDevice->enable_tp(GF_TP_DISENABLE);
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] disable tp fail, err code : %d.", __func__, err);
        }
        notifyErrorInfo(GF_FINGERPRINT_ERROR_CANCELED);
        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::doCancel() {
        VOID_FUNC_ENTER();

        do {
            if (mWorkState == STATE_IDLE) {
                break;
            }

            mWorkState = STATE_IDLE;
            mContext->mCenter->deregisterHandler(this);
            cancelEnrollTimer();
            enrollErrorCount = 0;//reset enrollErrorCount
        } while (0);

        VOID_FUNC_EXIT();
    }

    // notify message
    gf_error_t FingerprintCore::notifyAcquiredInfo(gf_fingerprint_acquired_info_t
                                                   info) {
        gf_error_t err = GF_SUCCESS;
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        FUNC_ENTER();

        if (nullptr != mNotify) {
            LOG_D(LOG_TAG, "[%s] acquired_info=%d", __func__, info);
            message.type = GF_FINGERPRINT_ACQUIRED;
            message.data.acquired.acquired_info = (gf_fingerprint_acquired_info_t) info;
            mNotify(&message);
        }

        if (GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT == info) {
            sendMessage(MsgBus::MSG_WAIT_FOR_FINGER_INPUT);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::notifyErrorInfo(gf_fingerprint_error_t error) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (nullptr != mNotify) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            message.type = GF_FINGERPRINT_ERROR;
            message.data.error = error;
            LOG_E(LOG_TAG, "[%s] err code : %d.", __func__, message.data.error);
            mNotify(&message);
            // FIXME: doCancel in enroll?
            // doCancel();
        }

        FUNC_EXIT(err);
        return err;
    }

    FingerprintCore::WORK_STATE FingerprintCore::getWorkState() {
        return mWorkState;
    }

    gf_error_t FingerprintCore::saveTemplates(uint32_t gid, uint32_t fid) {
        gf_error_t err = GF_SUCCESS;
        gf_auth_save_templates_t templates = {{ 0 }};
        FUNC_ENTER();
        templates.header.target = GF_TARGET_BIO;
        templates.header.cmd_id = GF_CMD_AUTH_SAVE_TEMPLATES;
        templates.i_gid = gid;
        templates.i_finger_id = fid;
        err = invokeCommand(&templates, sizeof(gf_auth_save_templates_t));
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::authenticateFido(uint32_t groupId, uint8_t *aaid,
                                                 uint32_t aaidLen,
                                                 uint8_t *finalChallenge, uint32_t challengeLen) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(groupId);
        UNUSED_VAR(aaid);
        UNUSED_VAR(aaidLen);
        UNUSED_VAR(finalChallenge);
        UNUSED_VAR(challengeLen);
        // TODO
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::cancelFido() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        // TODO
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::getIdList(uint32_t gid, uint32_t *list,
                                          int32_t *count) {
        gf_fingerprint_finger_id_t pfpList[MAX_FINGERS_PER_USER] = { { 0 } };
        int32_t fingerCount = MAX_FINGERS_PER_USER;
        gf_error_t err = GF_SUCCESS;
        int32_t i = 0;
        int32_t num = 0;
        FUNC_ENTER();

        do {
            if (list == NULL || count == NULL) {
                LOG_E(LOG_TAG, "[%s] bad parameter: list or count is NULL", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            err = enumerate(pfpList, (uint32_t *) &fingerCount);

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] gf_hal_common_enumerate.", __func__);
                break;
            }

            if (*count < fingerCount) {
                LOG_E(LOG_TAG, "[%s] count is smaller than fp_count, *count=%d, fp_count=%d",
                      __func__, *count, fingerCount);
                break;
            }

            for (i = 0; i < fingerCount; i++) {
                if (pfpList[i].gid == gid) {
                    list[i] = pfpList[i].fid;
                    num++;
                }
            }

            *count = num;
        } while (0);

        if ((GF_SUCCESS != err) && (count != NULL)) {
            *count = 0;
        }

        FUNC_EXIT(err);
        return err;
    }

    uint8_t FingerprintCore::isIdValid(uint32_t gid, uint32_t fid) {
        gf_error_t err = GF_SUCCESS;
        gf_fingerprint_finger_id_t pfpList[MAX_FINGERS_PER_USER] = { { 0 } };
        uint32_t fingerCount = MAX_FINGERS_PER_USER;
        uint32_t i = 0;
        uint8_t isIdValid = 0;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "finger_id=%u", fid);

        do {
            err = enumerate(pfpList, &fingerCount);

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] enumerate.", __func__);
                break;
            }

            for (i = 0; i < fingerCount; i++) {
                LOG_D(LOG_TAG, "[%s] finger_id=%u, fid[%d]=%u", __func__, fid, i,
                      pfpList[i].fid);

                if (pfpList[i].fid == fid && pfpList[i].gid == gid) {
                    LOG_D(LOG_TAG, "[%s] finger_id is valid", __func__);
                    isIdValid = 1;
                    break;
                }
            }

            err = isIdValid == 1 ? GF_SUCCESS : GF_ERROR_INVALID_FP_ID;
        } while (0);

        FUNC_EXIT(err);
        return isIdValid;
    }

    gf_error_t FingerprintCore::dumpKPI(const char *func_name) {
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
            // do dump
            LOG_I(LOG_TAG, "[%s]     image_quality=%d", func_name,
                  dump_performance->image_quality);
            LOG_I(LOG_TAG, "[%s]     valid_area=%d", func_name,
                  dump_performance->valid_area);
            LOG_I(LOG_TAG, "[%s]     key_point_num=%d", func_name,
                  dump_performance->key_point_num);
            LOG_I(LOG_TAG, "[%s]     get_raw_data_time=%dms", func_name,
                  dump_performance->get_raw_data_time / 1000);
            LOG_I(LOG_TAG, "[%s]     preprocess_time=%dms", func_name,
                  dump_performance->preprocess_time / 1000);
            LOG_I(LOG_TAG, "[%s]     get_feature_time=%dms", func_name,
                  dump_performance->get_feature_time / 1000);

            switch (mWorkState) {
                case STATE_ENROLL: {
                    LOG_I(LOG_TAG, "[%s]     increase_rate=%d", func_name,
                          dump_performance->increase_rate);
                    LOG_I(LOG_TAG, "[%s]     overlay=%d", func_name, dump_performance->overlay);
                    LOG_I(LOG_TAG, "[%s]     enroll_time=%dms", func_name,
                          dump_performance->enroll_time / 1000);
                    break;
                }

                case STATE_AUTHENTICATE: {
                    LOG_I(LOG_TAG, "[%s]     study flag =%u", func_name,
                          dump_performance->authenticate_study_flag);
                    LOG_I(LOG_TAG, "[%s]     match_score=%d", func_name,
                          dump_performance->match_score);
                    LOG_I(LOG_TAG, "[%s]     authenticate_time=%dms", func_name,
                          dump_performance->authenticate_time / 1000);
                    LOG_I(LOG_TAG,
                          "[%s]     KPI time(get_raw_data_time + preprocess+get_feature_time+authenticate)=%dms",
                          func_name, (dump_performance->get_raw_data_time
                                      + dump_performance->preprocess_time
                                      + dump_performance->get_feature_time
                                      + dump_performance->authenticate_time) / 1000);
                    break;
                }

                default: {
                    err = GF_ERROR_GENERIC;
                    break;
                }
            }

            if (GF_SUCCESS == err) {
                LOG_I(LOG_TAG, "[%s]    total time=%ums", func_name,
                      (uint32_t)(mTotalKpiTime / 1000));
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::screenState(uint32_t state) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (state != 0 && state != 1) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            mScreenStatus = state;
            LOG_D(LOG_TAG, "[%s]mScreenStatus = %d.", __func__, mScreenStatus);
            sendMessage(MsgBus::MSG_SCREEN_STATE, state);
            gf_screen_state_t screen_state = {{ 0 }};
            screen_state.header.target = GF_TARGET_BIO;
            screen_state.header.cmd_id = GF_CMD_AUTH_SCREEN_STATE;
            Mutex::Autolock _l(mContext->mHalLock);
            screen_state.i_screen_state = state;

            if (mWorkState == STATE_AUTHENTICATE) {
                screen_state.i_authenticating = 1;
            } else {
                screen_state.i_authenticating = 0;
            }

            err = invokeCommand(&screen_state, sizeof(gf_screen_state_t));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::syncFingerList(uint32_t groupId) {
        UNUSED_VAR(groupId);
#if defined(__ANDROID_N) || defined(__ANDROID_M)
        syncFingerListBasedOnSettings(mContext, groupId);
#endif  // defined(__ANDROID_N) || defined(__ANDROID_M)
        return;
    }

    void FingerprintCore::setAuthType(uint32_t authType) {
        VOID_FUNC_ENTER();

        if (AUTH_TYPE_UNKNOWN < authType && authType <= AUTH_TYPE_OTHER) {
            mAuthType = (gf_algo_auth_type_t)authType;
        }

        VOID_FUNC_EXIT();
    }

    bool FingerprintCore::isAuthDownDetected() {
        return mAuthDownDetected;
    }

    gf_error_t FingerprintCore::enrollPause() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        mWorkState = STATE_IDLE;
        FUNC_EXIT(err);
        return err;
    }
    gf_error_t FingerprintCore::enrollResume() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        mWorkState = STATE_ENROLL;
        FUNC_EXIT(err);
        return err;
    }
    gf_error_t FingerprintCore::checkEnrollResult(gf_error_t result) {
        if (GF_SUCCESS != result) {
            enrollErrorCount++;
            if (enrollErrorCount >= ENROLL_ERROR_MAX) {
                gf_fingerprint_msg_t message;
                memset(&message, 0, sizeof(gf_fingerprint_msg_t));
                message.type = GF_FINGERPRINT_ERROR;
                message.data.error = GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS;
                mNotify(&message);
                enrollErrorCount = 0;
            }
        } else {
            enrollErrorCount = 0;
        }
        return GF_SUCCESS;
    }

    const int8_t *testDecodeUint32(uint32_t *value, const int8_t *buf)
    {
        // little endian
        const uint8_t* tmp = (const uint8_t*) buf;
        *value = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;
        buf = buf + sizeof(uint32_t);
        return buf;
    }
    gf_error_t FingerprintCore::sendFingerprintCmd(int32_t cmd_id, int8_t* in_buf, uint32_t size)
    {
        gf_error_t err = GF_SUCCESS;
        int8_t* out = NULL;
        uint32_t outLen = 0;
        uint32_t token = 0;
        uint32_t operationStep = 0;
        uint32_t temp = 0;
        const int8_t *in_buf_temp = in_buf;

        FUNC_ENTER();

#ifdef FP_HYPNUSD_ENABLE
        hypnus_request_change(ACTION_TYPE, ACTION_TIMEOUT_2000);
#endif

        if (NULL != in_buf_temp) {
            do {
                in_buf_temp = testDecodeUint32(&token, in_buf_temp);
                switch (token) {
                    case PRODUCT_TEST_TOKEN_COLLECT_PHASE: {
                        in_buf_temp = testDecodeUint32(&operationStep, in_buf_temp);
                        break;
                    }
                    default: {
                        in_buf_temp = testDecodeUint32(&temp, in_buf_temp);
                        break;
                    }
                }
            } while (in_buf_temp < in_buf + size);
        }

        LOG_E(LOG_TAG, "[sendFingerprintCmd]operationStep =%d, cmd_id =%d", operationStep, cmd_id);
        if(PRODUCT_TEST_CMD_PERFORMANCE_TESTING == cmd_id){
            switch (operationStep){
                //l3
                case OPERATION_STEP_CALCULATE_GAIN_COLLECT:
                case OPERATION_STEP_BASEDATA_MAX_COLLECT:
                case OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT:
                case OPERATION_STEP_CIRCLEDATA_COLLECT:
                case OPERATION_STEP_CHARTDATA_COLLECT:
                    setHbmMode(1);
                    LOG_I(LOG_TAG, "[sendFingerprintCmd] l3, operationStep = %d, cmd_id =%d", operationStep, cmd_id);
                    break;

                //l2
                case OPERATION_STEP_BASEDATA_MID_DARK_COLLECT:
                case OPERATION_STEP_BASEDATA_MID_COLLECT:
                    setHbmMode(0);
                    setBrightness(2615);
                    LOG_I(LOG_TAG, "[sendFingerprintCmd] l2, operationStep = %d, cmd_id =%d", operationStep, cmd_id);
                    break;

                //l1
                case OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT:
                case OPERATION_STEP_BASEDATA_MIN_COLLECT:
                    setHbmMode(0);
                    setBrightness(1830);
                    LOG_I(LOG_TAG, "[sendFingerprintCmd] l1, operationStep = %d, cmd_id =%d", operationStep, cmd_id);
                    break;

                //l0
                case OPERATION_STEP_BASEDATA_DARK_COLLECT:
                    setHbmMode(0);
                    setBrightness(0);
                    LOG_I(LOG_TAG, "[sendFingerprintCmd] l0, operationStep = %d, cmd_id =%d", operationStep, cmd_id);
                    break;

                //finished
                case OPERATION_STEP_FINISHED:
                    setHbmMode(0);
                    LOG_I(LOG_TAG, "[sendFingerprintCmd] operationFinished, operationStep = %d, cmd_id =%d", operationStep, cmd_id);
                    break;

                default:
                {
                    break;
                }
            }
        }else if((PRODUCT_TEST_CMD_IMAGE_QUALITY == cmd_id)||(PRODUCT_TEST_CMD_AGE_START == cmd_id)||(PRODUCT_TEST_CMD_CAPTURE_IMAGE == cmd_id)){
            setHbmMode(1);
            LOG_I(LOG_TAG, "[sendFingerprintCmd] l1,");
        }
        err = mProductTest->onCommand(cmd_id, in_buf, size, &out, &outLen);
        if((PRODUCT_TEST_CMD_IMAGE_QUALITY == cmd_id)||(PRODUCT_TEST_CMD_AGE_STOP == cmd_id)||(PRODUCT_TEST_CMD_CAPTURE_IMAGE == cmd_id)){
            setHbmMode(0);
            LOG_I(LOG_TAG, "[sendFingerprintCmd] exit AGE-IMGQUALITY CAPTURE");
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::setHbmMode(uint32_t Mode)
    {

        char buf[50] = {'\0'};
        FUNC_ENTER();
        LOG_E(LOG_TAG, "SetActionMode start %d", Mode);
        #ifndef MTK_HBM_NODE
        static const char* Hbm_info_sdm660 = "/sys/devices/virtual/graphics/fb0/hbm";
        static const char* Hbm_info_sdm670 = "/sys/kernel/oplus_display/hbm";
        #else
        static const char* Hbm_info_sdm670 = "/sys/devices/virtual/mtk_disp_mgr/mtk_disp_mgr/LCM_HBM";
        #endif
        int fd = open(Hbm_info_sdm670, O_WRONLY);
        if (fd < 0) {
            LOG_E(LOG_TAG, "SetAction open err1 :%d, errno =%d",fd, errno);
            return  GF_ERROR_BASE;
        }
        snprintf(buf, sizeof(buf), "%d", Mode);
        write(fd, buf, 50);
        close(fd);
        #ifdef FP_HBM_BRIGHTNESS_DELAY
        LOG_E(LOG_TAG, "setHbmMode delay 200ms %d", Mode);
        usleep(200*1000);
        #endif
        LOG_E(LOG_TAG, "SetActionMode %d", Mode);
        return GF_SUCCESS;
    }

    gf_error_t FingerprintCore::setBrightness(uint32_t Mode)
    {

        char buf[50] = {'\0'};
        FUNC_ENTER();
        LOG_E(LOG_TAG, "setBrightness start %d", Mode);
        #ifndef MTK_HBM_NODE
        static const char* Brightness_sdm660 = "/sys/class/leds/lcd-backlight/brightness";
        static const char* Brightness_sdm670 = "/sys/class/backlight/panel0-backlight/brightness";
        #else
        static const char* Brightness_sdm660 = "/sys/kernel/oplus_display/oplus_brightness";
        static const char* Brightness_sdm670 = "/sys/kernel/oplus_display/oplus_brightness";
        #endif
        int fd = open(Brightness_sdm670, O_WRONLY);
        if (fd < 0) {
             LOG_E(LOG_TAG, "setBrightness 670 open err1 :%d, errno =%d, open 660 continue",fd, errno);
             fd = open(Brightness_sdm660, O_WRONLY);
             if (fd < 0) {
                 LOG_E(LOG_TAG, "setBrightness 660 open err1 :%d, errno =%d",fd, errno);
                 return  GF_ERROR_BASE;
             }
         }

        snprintf(buf, sizeof(buf), "%d", Mode);
        write(fd, buf, 50);
        close(fd);
        usleep(51*1000);
        #ifdef FP_HBM_BRIGHTNESS_DELAY
        LOG_E(LOG_TAG, "setHbmMode delay 200ms %d", Mode);
        usleep(200*1000);
        #endif
        LOG_E(LOG_TAG, "setBrightness %d", Mode);
        return GF_SUCCESS;
    }
    gf_error_t FingerprintCore::send_auth_dcsmsg(AuthenticateContext* context, bool notified) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(context);
        UNUSED_VAR(notified);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::set_hypnus(int32_t action_type, int32_t action_timeout) {
        gf_error_t err = GF_SUCCESS;

        LOG_D(LOG_TAG, "[%s] entry, action_type is %d, action_timeout is %d", __func__, action_type, action_timeout);
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));

        message.type = GF_FINGERPRINT_HYPNUSSETACION;
        message.data.hypnusd_setting.action_type = action_type;
        message.data.hypnusd_setting.action_timeout = action_timeout;

        mNotify(&message);
        return err;
    }

    void FingerprintCore::hypnus_request_change(int32_t action_type, int32_t action_timeout) {
        pthread_mutex_lock(&hypnus_state.hypnusd_lock);
        hypnus_state.hypnusd_request = true;
        hypnus_state.hypnusd_action_type = action_type;
        hypnus_state.hypnusd_action_timeout = action_timeout;
        pthread_cond_signal(&hypnus_state.hypnusd_cond);
        pthread_mutex_unlock(&hypnus_state.hypnusd_lock);
    }

    gf_error_t FingerprintCore::setScreenState(uint32_t state)
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();

        switch (state)
        {
            case 1://SCREEN_ON:
            {
                mContext->mCenter->postEvent(EVENT_SCREEN_ON);
                break;
            }
            case 0://SCREEN_OFF:
            {
                mContext->mCenter->postEvent(EVENT_SCREEN_OFF);
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
