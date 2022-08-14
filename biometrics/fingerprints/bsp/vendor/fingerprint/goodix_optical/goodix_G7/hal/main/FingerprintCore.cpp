/************************************************************************************
 ** File: - hal\main\FingerprintCore.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Goodixr fingerprint
 **
 ** Version: 1.0
 ** Date created: 10:58:11,30/09/2019
 ** Author: Chen.ran@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <who>            <when>            <what>
 **  Ran.Chen         2019/09/16        add for HBM control in IMAGE_QUALITY_TEST
 **  Ran.Chen         2019/10/21        modify for IMAGER_DIRTY\ACQUIRED_PARTIAL msg
 **  Bangxiong.Wu     2019/10/22        modify enroll time out to 600s
 **  Ran.Chen         2019/10/21        add for hypnus
 **  Bangxiong.Wu     2019/10/30        add reset lasttouchmode when enroll and continuenroll
 **  Bangxiong.Wu     2019/11/08        add for dcsmsg
 **  Bangxiong.Wu     2019/11/14        add for enroll error limit times and notify ACQUIRED_GOOD before onError
 **  Ran.Chen         2020/01/09        add for set enrollErrorCount to 0, after send err2 msg
 **  Bangxiong.Wu     2020/01/16        reset hbm node to 0 when calibrate operation finished
 **  Ran.Chen         2020/02/19        set the right gid number when template is null
 **  Bangxiong.Wu     2020/02/24        fix set_hypnus block thread problem
 **  Ran.Chen         2020/04/22        add for 17*3ms delay in setBrightness
 **  Ran.Chen         2020/06/06        modify for coverity 828730  828663
 **  Bangxiong.Wu     2020/06/16        move touchdown notify to netlink thread for fastUIReady
 **  mingzhi.Guo      2021/09/20        add and modify to support heartrate detect
 ***********************************************************************************/
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
#include "fingerprint.h"
#include "DelmarProductTestDefine.h"
#include "gf_delmar_product_test_types.h"
#include "ProductTest.h"
#include "FingerprintConfig.h"
#include <cutils/properties.h>
#include "to_string.h"
#include "FpMonitorTool.h"
#ifdef SUPPORT_DSP_HAL
#include "HalDsp.h"
#endif   // SUPPORT_DSP_HAL


#define OPLUS_ENROLL_TIME_OUT (10 * 60 * 1000)
#define ENROLL_ERROR_MAX (10)
#define IMAGE_PASS_SCORE (20)
static gf_fingerprint_notify_t factory_notify;
static gf_fingerprint_notify_t heartrate_notify;

#ifdef FP_HYPNUSD_ENABLE
extern goodix::FingerprintCore::hypnus_state_t hypnus_state;
#endif

#define DEFAULT_BRIGHTNESS 660

#define TA_VALID (1)
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"

namespace goodix {

FingerprintCore::FingerprintCore(HalContext *context) :
    HalBase(context),
    mNotify(nullptr),
    mFidoNotify(nullptr),
    mTotalKpiTime(0),
    mFailedAttempts(0),
    mAuthDownDetected(false),
    mAuthUpNotified(false),
    mWorkState(STATE_IDLE),
    mEnrollTimerSec(0),
    mEnrollTimer(nullptr),
    mProductTest(nullptr),
    mHeartRate(nullptr),
    mAuthType(AUTH_TYPE_LOCK),
    mFingerStatus(STATE_INIT),
    mScreenStatus(0),
    mGid(0),
    enrollErrorCount(0),
    authTotalTime(0),
    uiReadyTime(0),
    captureTime(0) {
    memset(mFpDataPath, 0, sizeof(mFpDataPath));
}

FingerprintCore::~FingerprintCore() {
    if (mContext->mCenter != nullptr) {
        mContext->mCenter->deregisterHandler(this);
    }
}

/* optimize the hbm set, and reduce the Cali time
 * method 1: read the hbm value
 */
int32_t needSetHbm(int32_t hbm_mode, int32_t hbm_mode_new) {
    int32_t need = 0;

    LOG_I(LOG_TAG, "set hbm old:%d new:%d", hbm_mode, hbm_mode_new);
    if (hbm_mode == hbm_mode_new) {
        need = 0;
    } else {
        need = 1;
    }

    if (need == 0) {
        LOG_I(LOG_TAG, "no need to set hbm");
    }

    return need;
}

static gf_error_t notifyFingerprintCmd(int64_t devId, int32_t msgId, int32_t cmdId,
                                       const int8_t *result, int32_t resultLen) {
    UNUSED_VAR(devId);
    UNUSED_VAR(msgId);
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_E(LOG_TAG, "[%s] enter notifyFingerprintCmd, return GF_SUCCESS cmdId = %d", __func__, cmdId);

    if ((cmdId != PRODUCT_TEST_CMD_SPI) &&
            (cmdId != PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN) &&
            (cmdId != PRODUCT_TEST_CMD_OTP_FLASH) &&
            (cmdId != PRODUCT_TEST_CMD_PERFORMANCE_TESTING) &&
            (cmdId != PRODUCT_TEST_CMD_AGE_START) &&
            (cmdId != PRODUCT_TEST_CMD_AGE_START) &&
            (cmdId != PRODUCT_TEST_CMD_IMAGE_QUALITY) &&
            (cmdId != PRODUCT_TEST_CMD_GET_VERSION) &&
            (cmdId != PRODUCT_TEST_CMD_CAPTURE_IMAGE) &&
            (cmdId != HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_DOWN) &&
            (cmdId != PRODUCT_TEST_CMD_GET_OTP_QRCODE)) {
        LOG_E(LOG_TAG, "[%s] cmdId = %d, cmdId is not PRODUCT_TEST_CMD, do nothing!!!", __func__, cmdId);
        return GF_SUCCESS;
    }

    if (heartrate_notify != nullptr && cmdId == HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_DOWN) {
        gf_fingerprint_msg_t message;

        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_HEART_RATE_INFO;
        message.data.test.cmd_id = cmdId;
        message.data.test.result = (int8_t *) result;
        message.data.test.result_len = resultLen;

        heartrate_notify(&message);
    }

    if (factory_notify != nullptr && cmdId != HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_DOWN) {
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
            message.data.engineering.quality.quality_pass = (message.data.engineering.quality.image_quality >
                    IMAGE_PASS_SCORE ? 1 : 0);

            LOG_E(LOG_TAG,
                  "[%s] GF_FINGERPRINT_ENGINEERING_INFO, cmdId = %d, error_code = %d, resultLen = %d, image_quality = %d",
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


void FingerprintCore::init_report_data(void)
{
    LOG_D(LOG_TAG, "[%s] enter", __func__);
}

gf_error_t FingerprintCore::init() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    screenState(1);
    doCancel();

    if (mContext != NULL) {
        mProductTest = createProductTest(mContext);
        mProductTest->setNotify((ext_module_callback)notifyFingerprintCmd);
        LOG_I(LOG_TAG, "[%s] set  notifyFingerprintCmd callback", __func__);
    }

    if (mContext != NULL) {
        mHeartRate = createHeartbeatRate(mContext);
        mHeartRate->setNotify((ext_module_callback)notifyFingerprintCmd);
        LOG_I(LOG_TAG, "[%s] set  notifyFingerprintCmd callback", __func__);
    }

    init_report_data();
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

    /* for DCS */
    if (mContext->mDcsInfo->mDcsStaticInfo.need_notify_init_info) {
        mContext->mDcsInfo->sendDcsInitEventInfo(mContext);
        LOG_D(LOG_TAG, "sendDcsInitEventInfo debug");
    }
    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::setNotify(gf_fingerprint_notify_t notify) {
    gf_error_t err = GF_SUCCESS;
    Mutex::Autolock _l(mContext->mHalLock);
    mNotify = notify;
    factory_notify = notify;
    heartrate_notify = notify;
    return err;
}

gf_error_t FingerprintCore::setFidoNotify(gf_fingerprint_notify_t notify) {
    gf_error_t err = GF_SUCCESS;
    Mutex::Autolock _l(mContext->mHalLock);
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

uint64_t FingerprintCore::generateChallenge() {
    gf_error_t err = GF_SUCCESS;
    Mutex::Autolock _l(mContext->mHalLock);
    gf_gen_challenge_cmd_t cmd = {{ 0 }};
    FUNC_ENTER();

    cmd.header.target = GF_TARGET_BIO;
    cmd.header.cmd_id = GF_CMD_AUTH_GENERATE_CHALLENGE;
    err = invokeCommand(&cmd, sizeof(cmd));
    if (err != GF_SUCCESS) {
        LOG_E(LOG_TAG, "[%s] generateChallenge failed err=%d", __func__, err);
    }

    FUNC_EXIT(err);
    return cmd.o_challenge;
}

gf_error_t FingerprintCore::revokeChallenge(uint64_t challenge) {
    gf_error_t err = GF_SUCCESS;
    Mutex::Autolock _l(mContext->mHalLock);
    gf_revoke_challenge_cmd_t cmd = {{ 0 }};
    FUNC_ENTER();
    cmd.header.target = GF_TARGET_BIO;
    cmd.header.cmd_id = GF_CMD_AUTH_REVOKE_CHALLENGE;
    cmd.i_challenge = challenge;
    err = invokeCommand(&cmd, sizeof(cmd));

    FUNC_EXIT(err);
    return err;
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
            if (err != GF_ERROR_CALIBRATION_NOT_READY) {
                err = GF_ERROR_CANCELED;
            }
            onError(GF_ERROR_SENSOR_IS_BROKEN);
            LOG_E(LOG_TAG, "[%s] Enroll onError&cancel.", __func__);
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
        if (timeoutSec == 0) {
            timeoutSec = mContext->mConfig->enroll_timeout_sec;
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

            // support android o vts
            if (GF_ERROR_INVALID_CHALLENGE == err || GF_ERROR_INVALID_HAT_VERSION == err
                    || GF_ERROR_UNTRUSTED_ENROLL == err) {
                LOG_E(LOG_TAG, "[%s] hardware unavailable", __func__);
                notifyErrorInfo(GF_FINGERPRINT_ERROR_HW_UNAVAILABLE);
                err = GF_SUCCESS;
            }

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
            err = mContext->mSensor->isUIReadyTimeout() ? GF_ERROR_UI_READY_TIMEOUT : GF_ERROR_TOO_FAST;
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
    HalDsp *dsp = mContext->getDsp();
    if (nullptr != dsp) {
        if ((operationCmd == GF_OPERATION_ENROLL && mContext->isDSPEnroll()) ||
                (operationCmd == GF_OPERATION_AUTHENTICATE && mContext->isDSPAuth())) {
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
    HalDsp *dsp = mContext->getDsp();
    FUNC_ENTER();
    do {
        if (nullptr != dsp && (DSP_AVAILABLE == dsp->checkDspValid())) {
            if ((operationCmd == GF_OPERATION_ENROLL && mContext->isDSPEnroll()) ||
                    (operationCmd == GF_OPERATION_AUTHENTICATE && mContext->isDSPAuth())) {
                if (dsp->checkDspSpeed() == DSP_NORMAL_SPEED && SpeedMode == 1) {
                    speedModelCmd = DSP_CMD_SET_HIGH_SPEED;
                } else if (dsp->checkDspSpeed() == DSP_HIGH_SPEED && SpeedMode == 0) {
                    speedModelCmd = DSP_CMD_SET_NORMAL_SPEED;
                } else {
                    break;
                }
                err = dsp->sendCmdToDsp(speedModelCmd);
                if (err == GF_SUCCESS) {
                    LOG_D(LOG_TAG, "[%s] set DSP SpeedMode:%d success", __func__, SpeedMode);
                } else {
                    LOG_E(LOG_TAG, "[%s] set DSP SpeedMode:%d fail, err:%d", __func__, SpeedMode, err);
                }
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
    gf_algo_enroll_image_t *cmd = mContext->mAlgo->createEnrollCmd();
    FUNC_ENTER();

#ifdef SUPPORT_DSP_HAL
    bool is_wait_set_speed_complete = false;
    setDspSpeed(GF_OPERATION_ENROLL, 1);
#endif   // SUPPORT_DSP_HAL
    do {
        GF_NULL_BREAK(cmd, err);
        context.enroll_cmd = cmd;

        context.fingerDownTime = HalUtils::getCurrentTimeMicrosecond();
        if (mContext->mConfig->support_performance_dump) {
            mTotalKpiTime = 0;
        }

        //notifyTouch(GF_FINGERPRINT_TOUCH_DOWN);
        notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_DOWN);
        sendMessage(MsgBus::MSG_ENROLL_START);
        onEnrollStart(&context);
        // 1. capture image
        onBeforeEnrollCapture(&context);
        uiReadyTime = HalUtils::getCurrentTimeMicrosecond() - context.fingerDownTime;
        LOG_D(LOG_TAG, "[%s] Begin capture after finger down %d ms.", __func__, (int)(uiReadyTime / 1000));
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
            LOG_D(LOG_TAG, "[%s] enroll: antipeep & screen struct flag : %d (1:ANTIPEEPING, 2:SCREEN_STRUCT)",
                  __func__, context.enroll_cmd->o_antipeep_screen_struct_flag);
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
    waitDspSetSpeed(GF_OPERATION_ENROLL);
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
    //notifyTouch(GF_FINGERPRINT_TOUCH_UP);
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

    /* for DCS */
    memcpy(&enrollContext, context, sizeof(EnrollContext));
    if (0 == samplesRemaining) {
        mContext->mDcsInfo->sendDcsEnrollEventInfo(mContext);
    } else {
        mContext->mDcsInfo->sendDcsSingleEnrollEventInfo(mContext);
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
    gf_authenticate_cmd_t cmd = {{ 0 }};
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] Authenticate.", __func__);
    Mutex::Autolock _l(mContext->mHalLock);

    do {
        if (prepareAuthRequest() != GF_SUCCESS) {
            err = GF_ERROR_CANCELED;
            break;
        }

        if (mAuthType == GF_TP_AUTHQKTYPE) {
            LOG_I(LOG_TAG, "[%s] tp quick start", __func__);
            err = mContext->mDevice->enable_tp(GF_TP_OTHER);
        } else {
            LOG_I(LOG_TAG, "[%s] tp enable", __func__);
            err = mContext->mDevice->enable_tp(GF_TP_ENABLE);
        }

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

    checkSensorLockout(err, cmd.o_lockout_duration_millis);

    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::resetLockout(const void* hat) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    Mutex::Autolock _l(mContext->mHalLock);

    do {
        gf_reset_lockout_cmd_t cmd = {{ 0 }};
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_RESET_LOCKOUT;

        if (NULL != hat) {
            memcpy(&cmd.i_auth_token, hat, sizeof(gf_hw_auth_token_t));
        }
        err = invokeCommand(&cmd, sizeof(cmd));
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::authenticateAsType(uint64_t operationId, uint32_t gid, uint32_t authtype) {
    UNUSED_VAR(operationId);
    UNUSED_VAR(gid);
    gf_error_t err = GF_SUCCESS;
    mAuthType = (gf_algo_auth_type_t)authtype;
    LOG_I(LOG_TAG, "[%s] authtype =%d.", __func__, mAuthType);
#ifdef SWITCH_RAWDATA_MODE
    if (is_rawdata_mode()) {
        fp_monitor_init(mContext->mDevice);
    }
#endif
    return err;
}

bool FingerprintCore::needRetryIfCaptureFailed(AuthenticateContext *context) {
    UNUSED_VAR(context);
    return false;
}

gf_error_t FingerprintCore::onAuthDownEvt() {
    gf_error_t err = GF_SUCCESS;
    gf_algo_auth_image_t *cmd = mContext->mAlgo->createAuthCmd();
    AuthenticateContext context;
    context.result = err;
    context.auth_cmd = cmd;
    mAuthResultNotified = false;
    int ui_ready_flag = 0;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] Auth Down.", __func__);
    mScreenInAuthMode = mScreenStatus;
#ifdef SUPPORT_DSP_HAL
    bool is_wait_set_speed_complete = false;
    // 0. set Dsp in high freq
    setDspSpeed(GF_OPERATION_AUTHENTICATE, 1);
#endif   // SUPPORT_DSP_HAL
    do {
        GF_NULL_BREAK(cmd, err);

        if (mContext->mConfig->support_performance_dump) {
            mTotalKpiTime = 0;
        }

        mAuthDownDetected = true;
        context.fingerDownTime = HalUtils::getCurrentTimeMicrosecond();
        //notifyTouch(GF_FINGERPRINT_TOUCH_DOWN);
        notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_DOWN);
        sendMessage(MsgBus::MSG_AUTHENTICATE_START);
        onAuthStart(&context);
        do {
            if (getAuthUpNotified() == true) {
                context.result = GF_ERROR_TOO_FAST;
                dcs_auth_result_type = DCS_AUTH_TOO_FAST_NO_IMGINFO;
                GF_ERROR_BREAK(context.result);
            }
            // 1. capture image
            onBeforeAuthCapture(&context);
            if (context.result == GF_ERROR_TOO_FAST) {
                dcs_auth_result_type = DCS_AUTH_TOO_FAST_NO_IMGINFO;
            }
            /* caculate ui ready time cost */
            if (ui_ready_flag == 0) {
                uiReadyTime = HalUtils::getCurrentTimeMicrosecond() - context.fingerDownTime;
                ui_ready_flag = 1;
            }
            LOG_D(LOG_TAG, "[%s] ui_ready_time begin capture after finger down: %d ms.", __func__, (int)(uiReadyTime / 1000));

            GF_ERROR_BREAK(context.result);
            err = mContext->mSensor->captureImage(GF_OPERATION_AUTHENTICATE, context.retry);
            context.result = err;

            onAfterAuthCapture(&context);

            if (context.result == GF_ERROR_TOO_FAST) {
                dcs_auth_result_type = DCS_AUTH_TOO_FAST_NO_IMGINFO;
            }
            if (needRetryIfCaptureFailed(&context)) {
                LOG_D(LOG_TAG, "[%s] Retry with capture failed", __func__);
                continue;
            }
            /* caculate capture time cost */
            captureTime = HalUtils::getCurrentTimeMicrosecond() - context.fingerDownTime - uiReadyTime;
            LOG_D(LOG_TAG, "[%s] capture_all_time: %d ms.", __func__, (int)(captureTime / 1000.0));

            GF_ERROR_BREAK(context.result);
            // 2. algo authenticate
            onBeforeAuthAlgo(&context);
#ifdef SUPPORT_DSP_HAL
            if (is_wait_set_speed_complete == false) {
                is_wait_set_speed_complete = waitDspSetSpeed(GF_OPERATION_AUTHENTICATE);
            }
#endif   // SUPPORT_DSP_HAL
            usleep(100);
            cmd->io_auth_token.version = GF_HW_AUTH_TOKEN_VERSION;
            cmd->io_auth_token.authenticator_type = htobe32(GF_HW_AUTH_FINGERPRINT);
            cmd->i_retry_count = context.retry;
            cmd->i_auth_type = mAuthType;
            err = mContext->mAlgo->authImage(cmd);
            if (cmd->o_antipeep_screen_struct_flag) {
                LOG_D(LOG_TAG, "[%s] auth: antipeep & screen struct flag : %d (1:ANTIPEEPING, 2:SCREEN_STRUCT)",
                      __func__, cmd->o_antipeep_screen_struct_flag);
            }
            context.result = err;
            LOG_D(LOG_TAG, "[%s] high light flag = %d", __func__, cmd->o_high_light);
            sendMessage(MsgBus::MSG_AUTHENTICATE_ALGO_END,  context.result, context.retry);
            onAfterAuthAlgo(&context);

            if (needRetry(&context)) {
                sendMessage(MsgBus::MSG_AUTHENTICATE_RETRYING, context.result, context.retry);
                context.retry++;
                // retry authenticate
                LOG_I(LOG_TAG, "[%s] Authenticate is retrying,", __func__);
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
        authTotalTime = HalUtils::getCurrentTimeMicrosecond() - context.fingerDownTime;

        mContext->mDevice->enable_tp(GF_TP_DISENABLE);

        // 4. do post something, such as study
        onAfterAuthSuccess(&context);
    } while (0);

#ifdef SUPPORT_DSP_HAL
    if (is_wait_set_speed_complete == false) {
        waitDspSetSpeed(GF_OPERATION_AUTHENTICATE);
    }
    // 5. set dsp to normal status
    setDspSpeed(GF_OPERATION_AUTHENTICATE, 0);
    waitDspSetSpeed(GF_OPERATION_AUTHENTICATE);
#endif   // SUPPORT_DSP_HAL

    onAuthStop(&context);
    if (err != GF_SUCCESS) {
        onAuthError(&context);
    }
    checkSensorLockout(context.lockout_err, context.lockout_duration_millis);
    sendMessage(MsgBus::MSG_AUTHENTICATE_END,  context.result, context.retry);

    /* for DCS */
    memcpy(&authContext, &context, sizeof(AuthenticateContext));
    mContext->mDcsInfo->sendDcsAuthEventInfo(mContext);
    //send_auth_dcsmsg(&context, mAuthResultNotified);
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
        if (getAuthUpNotified() == true) {
            err = GF_ERROR_TOO_FAST;
        } else {
            if (!mContext->mSensor->waitSensorUIReady()) {
                err = mContext->mSensor->isUIReadyTimeout() ? GF_ERROR_UI_READY_TIMEOUT : GF_ERROR_TOO_FAST;
            }
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
        retry = (GF_ERROR_NOT_MATCH == context->result && !checkFingerLeave());
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

void FingerprintCore::checkSensorLockout(gf_error_t err, uint64_t duration) {
    if (mContext->mConfig->support_lockout_in_ta) {
        if (err == GF_ERROR_TEMPORARY_LOCKOUT) {
            notifyLockoutTemporarily(duration);
        } else if (err == GF_ERROR_PERMANENT_LOCKOUT) {
            notifyLockoutPermanently();
        }
    }
}

void FingerprintCore::notifyLockoutTemporarily(uint64_t duration_millis) {
    VOID_FUNC_ENTER();

    if (mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_ERROR;
        message.data.lockout.error = GF_FINGERPRINT_ERROR_LOCKOUT;
        message.data.lockout.duration_mills = duration_millis;
        LOG_D(LOG_TAG, "[%s] lockout temporarily, duration = %d", __func__, (uint32_t)duration_millis);
        mNotify(&message);
    }

    VOID_FUNC_EXIT();
}

void FingerprintCore::notifyLockoutPermanently() {
    VOID_FUNC_ENTER();

    if (mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_ERROR;
        message.data.lockout.error = GF_FINGERPRINT_ERROR_LOCKOUT_PERMANENT;
        message.data.lockout.duration_mills = 0;
        LOG_D(LOG_TAG, "[%s] lockout permanently", __func__);
        mNotify(&message);
    }

    VOID_FUNC_EXIT();
}

gf_error_t FingerprintCore::onAuthStop(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    gf_auth_end_t cmd = { { 0 } };
    FUNC_ENTER();

    do {
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_AUTHENTICATE_END;
        cmd.i_fid = context->auth_cmd->o_finger_id;
        cmd.i_result = context->result;
        cmd.i_retry = context->retry;
        err = invokeCommand(&cmd, sizeof(gf_auth_end_t));
        if (GF_SUCCESS == context->result
            || err == GF_ERROR_TEMPORARY_LOCKOUT
            || err == GF_ERROR_PERMANENT_LOCKOUT) {
            // cancel auth if lockout
            doCancel();
            context->lockout_err = err;
            context->lockout_duration_millis = cmd.o_lockout_duration_millis;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

#ifdef FP_HYPNUSD_ENABLE
gf_error_t FingerprintCore::set_hypnus(int32_t action_type, int32_t action_timeout) {
    gf_error_t err = GF_SUCCESS;

    LOG_D(LOG_TAG, "[%s] entry, action_type is %d, action_timeout is %d", __func__, action_type,
          action_timeout);
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
#endif

gf_error_t FingerprintCore::notifyAuthSuccess(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_GOOD);
    dcs_auth_result_type = DCS_AUTH_SUCCESS;

    if (mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_AUTHENTICATED;
        message.data.authenticated.finger.gid = context->auth_cmd->o_gid;
        message.data.authenticated.finger.fid = context->auth_cmd->o_finger_id;
        LOG_I(LOG_TAG, "[%s] Auth success, fid = %u", __func__,
              context->auth_cmd->o_finger_id);
        LOG_I(LOG_TAG, "[%s]version:%u", __func__,
            (unsigned int)context->auth_cmd->io_auth_token.version);
        LOG_D(LOG_TAG, "[%s] challenge:%llu", __func__,
            context->auth_cmd->io_auth_token.challenge);
        LOG_D(LOG_TAG, "[%s] user_id:%llu", __func__,
            context->auth_cmd->io_auth_token.user_id);
        LOG_D(LOG_TAG, "[%s] authenticator_id:%llu", __func__,
            context->auth_cmd->io_auth_token.authenticator_id);
        LOG_I(LOG_TAG, "[%s] authenticator_type:%u", __func__,
            context->auth_cmd->io_auth_token.authenticator_type);
        LOG_I(LOG_TAG, "[%s] timestamp:%llu", __func__,
            context->auth_cmd->io_auth_token.timestamp);
        memcpy(&message.data.authenticated.hat, &(context->auth_cmd->io_auth_token),
               sizeof(gf_hw_auth_token_t));
        mNotify(&message);

        dumpKPI(__func__);
    }

    mFailedAttempts = 0;
    mAuthResultNotified = true;
    FUNC_EXIT(err);
    return err;
}

void FingerprintCore::setAuthUpNotified(bool mAuthUp) {
    std::lock_guard<std::mutex> lock(mUpNotifiedMutex);
    mAuthUpNotified = mAuthUp;
}

bool FingerprintCore::getAuthUpNotified() {
    std::lock_guard<std::mutex> lock(mUpNotifiedMutex);
    return mAuthUpNotified;
}

gf_error_t FingerprintCore::onAuthUpEvt() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (mAuthDownDetected) {
        //notifyTouch(GF_FINGERPRINT_TOUCH_UP);
        notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_UP);
        mAuthDownDetected = false;
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::notifyTouch(gf_fingerprint_msg_type_t type) {
    (void)type;
    return GF_SUCCESS;
}

// overide IEventHandler
gf_error_t FingerprintCore::onEvent(gf_event_type_t e) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] event = %d workstate = %d", __func__, e, mWorkState);
#ifdef FP_BINDCORE_BYTID
    bind_bigcore_bytid();
#endif
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

gf_error_t FingerprintCore::bind_bigcore_bytid() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "bind_bigcore_bytid", __func__);
    if (mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_BINDCORE;
        message.data.bindcore_setting.tid = gettid();
        LOG_D(LOG_TAG, "[%s]  bindcore_tid = %u", __func__, message.data.bindcore_setting.tid);
        mNotify(&message);
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::onEnrollReceivedEvt(gf_event_type_t event) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    switch (event) {
    case EVENT_FINGER_DOWN: {
        setAuthUpNotified(false);
        err = onEnrollDownEvt();
        mFingerStatus = STATE_DOWN;
        break;
    }

    case EVENT_FINGER_UP: {
        if (getAuthUpNotified() == false) {
            err = onEnrollUpEvt();
        }
        mFingerStatus = STATE_UP;
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
#ifdef SWITCH_RAWDATA_MODE
        if (is_rawdata_mode()) {
            err = GF_ERROR_TOO_FAST;
        }
#endif
    notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_GOOD);
    switch (err) {
    case GF_ERROR_UI_DISAPPEAR:
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
        if ((mWorkState == STATE_AUTHENTICATE) && (mScreenStatus != 0)) {
            notifyAuthNotMatched();
        } else {
            onAuthUpEvt();
        }
        break;
    }

    case GF_ERROR_SENSOR_BROKEN_CHECK_ALGO_ERROR:
    case GF_ERROR_ACQUIRED_PARTIAL: {
        notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_PARTIAL);
        if ((mWorkState == STATE_AUTHENTICATE) && (mScreenStatus != 0)) {
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
    case GF_ERROR_ACQUIRED_IMAGER_DRY:
    case GF_ERROR_DSP_GET_FEATURE_FAIL: {
        notifyAuthNotMatched();
        break;
    }

    default: {
        notifyErrorInfo(GF_FINGERPRINT_ERROR_INVALID_DATA);
        if (mWorkState == STATE_AUTHENTICATE) {
            dcs_auth_result_type = DCS_AUTH_OTHER_FAIL_REASON;
        }
        break;
    }
    }

    VOID_FUNC_EXIT();
}

void FingerprintCore::notifyAuthNotMatched() {
    VOID_FUNC_ENTER();

    dcs_auth_result_type = DCS_AUTH_FAIL;

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
        notifyErrorInfo(GF_FINGERPRINT_ERROR_LOCKOUT);
    }

    if (mContext->mConfig->support_performance_dump) {
        dumpKPI(__func__);
    }

    VOID_FUNC_EXIT();
}

void FingerprintCore::onAuthError(AuthenticateContext *context) {
    gf_error_t result =  context->result;
    VOID_FUNC_ENTER();
    if (context->retry > 0 && (GF_ERROR_TOO_FAST == result || GF_ERROR_UI_DISAPPEAR == result)) {
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
        setAuthUpNotified(false);
        err = onAuthDownEvt();
        break;
    }

    case EVENT_FINGER_UP: {
        if (getAuthUpNotified() == false) {
            err = onAuthUpEvt();
        }
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

uint64_t FingerprintCore::invalidateAuthenticatorId() {
    gf_error_t err = GF_SUCCESS;
    gf_invalidate_auth_id_t cmd = {{ 0 }};
    cmd.header.target = GF_TARGET_BIO;
    cmd.header.cmd_id = GF_CMD_AUTH_INVALIDATE_AUTH_ID;
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
            remainingTemplates = 0;
            notifyRemove(gid, fid, remainingTemplates);
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

gf_error_t FingerprintCore::remove(uint32_t gid, const std::vector<int32_t>& fids) {
    gf_error_t err = GF_SUCCESS;
    std::vector<uint32_t> removedTempls;
    uint32_t fid = 0;
    FUNC_ENTER();

    Mutex::Autolock _l(mContext->mHalLock);
    WORK_STATE old = mWorkState;
    mWorkState = STATE_REMOVE;
    do {
        removedTempls.clear();
        for (uint8_t i = 0; i < fids.size(); i++) {
            fid = (uint32_t)fids.at(i);
            if (fid == 0) {
                LOG_E(LOG_TAG, "[%s] group_id=%u, fids[%d]=%d", __func__, gid, i, fid);
                continue;
            }
            LOG_D(LOG_TAG, "[%s] begin to remove fid:%d", __func__, fid);
            err = removeTemplates(gid, fid);
            if (err == GF_SUCCESS) {
                removedTempls.push_back(fid);
                LOG_D(LOG_TAG, "[%s] success remove fid:%d", __func__, fid);
                sendMessage(MsgBus::MSG_TEMPLATE_REMOVED, gid, fid);
            }
            LOG_D(LOG_TAG, "[%s] end to remove fid:%d err:%d", __func__, fid, err);
        }
        notifyRemoveEnrollments(gid, fids, removedTempls);
        err = GF_SUCCESS;
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

gf_error_t FingerprintCore::notifyRemoveEnrollments(uint32_t gid, const std::vector<int32_t>& fids,
                                                    std::vector<uint32_t>& removedFids) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_fingerprint_msg_t message;
    UNUSED_VAR(fids);
    memset(&message, 0, sizeof(gf_fingerprint_msg_t));
    message.type = GF_FINGERPRINT_TEMPLATE_REMOVED;
    message.data.removed_fingers.gid = gid;
    for (uint8_t i = 0; i < removedFids.size(); i++) {
        message.data.removed_fingers.fid[i] = removedFids.at(i);
    }
    if (mNotify != nullptr) {
        mNotify(&message);
    }
    FUNC_EXIT(err);
    return err;
}
gf_error_t FingerprintCore::notifyRemove(uint32_t gid, uint32_t fid,
        uint32_t remainingTemplates) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    gf_fingerprint_msg_t message;
    memset(&message, 0, sizeof(gf_fingerprint_msg_t));
    gf_fingerprint_finger_id_t pfpList[MAX_FINGERS_PER_USER];
    memset(pfpList, 0, sizeof(pfpList));
    int32_t fingerCount = MAX_FINGERS_PER_USER;

    UNUSED_VAR(remainingTemplates);

    message.type = GF_FINGERPRINT_TEMPLATE_REMOVED;
    message.data.removed.finger.gid = gid;
    message.data.removed.finger.fid = fid;
    message.data.removed.fingers_count = remainingTemplates;
    do {
        err = enumerate(pfpList, (uint32_t *) &fingerCount);

        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] gf_hal_common_enumerate.", __func__);
            break;
        }
        for (int i = 0; i < fingerCount; i++) {
            message.data.removed.total_fingers[i] = pfpList[i];
        }
        if (mNotify != nullptr) {
            mNotify(&message);
        }
    } while (0);

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

gf_error_t FingerprintCore::cancel(bool notifyCancelMsg) {
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
            LOG_E(LOG_TAG, "[%s] func_name is nullptr", __func__);
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
            LOG_I(LOG_TAG, "[%s]     KPI time(get_raw_data_time + preprocess+get_feature_time+authenticate)=%dms",
                func_name, (dump_performance->get_raw_data_time + dump_performance->preprocess_time +
                dump_performance->get_feature_time + dump_performance->authenticate_time) / 1000);
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

gf_error_t FingerprintCore::touchdown() {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    LOG_E(LOG_TAG, "[touchdown]in");

#ifdef FP_HYPNUSD_ENABLE
    hypnus_request_change(ACTION_TYPE, ACTION_TIMEOUT_500);
#endif

    err = mContext->mCenter->postEvent(EVENT_FINGER_DOWN);
    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::touchup() {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    LOG_E(LOG_TAG, "[touchup] in");

    err = mContext->mCenter->postEvent(EVENT_FINGER_UP);
    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::setScreenState(uint32_t state) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    switch (state) {
    case 1: { //SCREEN_ON:
        LOG_I(LOG_TAG, "set screen on");
        mContext->mCenter->postEvent(EVENT_SCREEN_ON);
        break;
    }
    case 0: { //SCREEN_OFF:
        LOG_I(LOG_TAG, "set screen off");
        mContext->mCenter->postEvent(EVENT_SCREEN_OFF);
        break;
    }
    }

    FUNC_EXIT(err);
    return err;
}

const int8_t *testDecodeUint32(uint32_t *value, const int8_t *buf) {
    // little endian
    const uint8_t *tmp = (const uint8_t *) buf;
    *value = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;
    buf = buf + sizeof(uint32_t);
    return buf;
}

gf_error_t FingerprintCore::getFpConfigData(void *para) {
    gf_error_t err = GF_SUCCESS;
    gf_fingerprint_msg_t message;
    memset(&message, 0, sizeof(gf_fingerprint_msg_t));
    FUNC_ENTER();
    if (nullptr != mNotify) {
        message.type = GF_FINGERPRINT_GET_CONFIG_DATA;
        message.data.data_config.para = para;
        mNotify(&message);
    }
    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::setUxThread(uint8_t enable) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "setUxThread");
    if (mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_SETUXTHREAD;
        message.data.setuxthread_info.pid = getpid();
        message.data.setuxthread_info.tid = gettid();
        message.data.setuxthread_info.enable = enable;
        LOG_D(LOG_TAG, "[%s]  tid = %u, enable = %d  ", __func__, message.data.setuxthread_info.tid, enable);
        mNotify(&message);
    }
    FUNC_EXIT(err);
    return err;
}

//50% hbm : offset = 0; 75% hbm : offset = 4;
uint32_t FingerprintCore::getBrightness(uint32_t offset) {
    fpTransforInfo info;
    info.cmd = FP_CMD_GET_BRIGHTNESS;
    info.response = -1;
    getFpConfigData((void *)&info);
    if (info.response > 0 && (offset == 0 || offset == 4)) {
        return  0xFFFF & (info.response >> (offset * 4));
    }

    if (offset == 4) {
        return (uint32_t)(DEFAULT_BRIGHTNESS * 1.5);
    }

    return DEFAULT_BRIGHTNESS;
}

gf_error_t FingerprintCore::sendFingerprintCmd(int32_t cmd_id, int8_t *in_buf, uint32_t size) {
    gf_error_t err = GF_SUCCESS;
    int8_t *out = NULL;
    uint32_t outLen = 0;
    uint32_t token = 0;
    uint32_t operationStep = 0;
    uint32_t temp = 0;
    const int8_t *in_buf_temp = in_buf;
    TIME_START(factory_time);
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
    TIME_START(brightness_time);

    LOG_E(LOG_TAG, "[sendFingerprintCmd]operationStep =%d %s, cmd_id =%d",
        operationStep, cali_op_to_str(operationStep), cmd_id);
    if (PRODUCT_TEST_CMD_PERFORMANCE_TESTING == cmd_id) {
        switch (operationStep) {
        //l3
        case OPERATION_STEP_CALCULATE_GAIN_COLLECT:
        case OPERATION_STEP_BASEDATA_MAX_COLLECT:
        case OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT:
        case OPERATION_STEP_CIRCLEDATA_COLLECT:
        case OPERATION_STEP_CHARTDATA_COLLECT:

            /* <chart collect cmd=109> delay for avoid capture the position circle */
            if (operationStep == OPERATION_STEP_CHARTDATA_COLLECT) {
                LOG_I(LOG_TAG, "delay 250ms for OPERATION_STEP_CHARTDATA_COLLECT");
                usleep(250 * 1000);
            }
            setHbmMode(1);
            LOG_I(LOG_TAG, "[sendFingerprintCmd] l3, operationStep = %d %s, cmd_id =%d",
                operationStep, cali_op_to_str(operationStep), cmd_id);
            break;

        //l2
        case OPERATION_STEP_BASEDATA_MID_DARK_COLLECT:
        case OPERATION_STEP_BASEDATA_MID_COLLECT:
            setHbmMode(0);
            setBrightness(getBrightness(4));
            LOG_I(LOG_TAG, "[sendFingerprintCmd] l2, operationStep = %d %s, cmd_id =%d",
                operationStep, cali_op_to_str(operationStep), cmd_id);
            break;

        //l1
        case OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT:
        case OPERATION_STEP_BASEDATA_MIN_COLLECT:
            setHbmMode(0);
            setBrightness(getBrightness(0));
            LOG_I(LOG_TAG, "[sendFingerprintCmd] l1, operationStep = %d %s, cmd_id =%d",
                operationStep, cali_op_to_str(operationStep), cmd_id);
            break;

        //l0
        case OPERATION_STEP_BASEDATA_DARK_COLLECT:
            setHbmMode(0);
            setBrightness(0);
            LOG_I(LOG_TAG, "[sendFingerprintCmd] l0, operationStep = %d %s, cmd_id =%d",
                operationStep, cali_op_to_str(operationStep), cmd_id);
            break;

        //finished
        case OPERATION_STEP_FINISHED:
            setHbmMode(0);
            LOG_I(LOG_TAG, "[sendFingerprintCmd] operationFinished, operationStep = %d %s, cmd_id =%d",
                operationStep, cali_op_to_str(operationStep),  cmd_id);
            break;

        default: {
            break;
        }
        }
    } else if ((PRODUCT_TEST_CMD_IMAGE_QUALITY == cmd_id) || (PRODUCT_TEST_CMD_AGE_START == cmd_id)
            || (PRODUCT_TEST_CMD_CAPTURE_IMAGE == cmd_id)) {
        setHbmMode(1);
        LOG_I(LOG_TAG, "[sendFingerprintCmd] l1,");
    }

    TIME_END(brightness_time, operationStep);
    TIME_START(goodix_time);
    if (cmd_id >= HEARTBEAT_RATE_CMD_START && cmd_id <= HEARTBEAT_RATE_CMD_END) {
        err = mHeartRate->onCommand(cmd_id, in_buf, size, &out, &outLen);
    } else {
        err = mProductTest->onCommand(cmd_id, in_buf, size, &out, &outLen);
    }
    TIME_END(goodix_time, operationStep);

    if ((PRODUCT_TEST_CMD_IMAGE_QUALITY == cmd_id) || (PRODUCT_TEST_CMD_AGE_STOP == cmd_id)
            || (PRODUCT_TEST_CMD_CAPTURE_IMAGE == cmd_id)) {
        setHbmMode(0);
        LOG_I(LOG_TAG, "[sendFingerprintCmd] exit AGE-IMGQUALITY CAPTURE");
    }
    TIME_END(factory_time, operationStep);
    FUNC_EXIT(err);
    return err;
}

gf_error_t FingerprintCore::setHbmMode(uint32_t Mode) {
    char buf[50] = {'\0'};
    FUNC_ENTER();
    LOG_E(LOG_TAG, "SetActionMode start %d", Mode);
    unsigned int delay_ms = 0;
    char *hbm_paths[] = {
        "/sys/kernel/oplus_display/hbm",
        "/sys/devices/virtual/mtk_disp_mgr/mtk_disp_mgr/LCM_HBM",
    };
    int index = 0;
    fpTransforInfo info;
    memset(&info, 0, sizeof(info));
    info.cmd = FP_CMD_GET_HBM_DELAY;
    info.response = -1;
    getFpConfigData((void *)&info);
    delay_ms = info.response;
    LOG_E(LOG_TAG, "hbm delay time:%d ms", delay_ms);

    for (index = 0; index < sizeof(hbm_paths)/sizeof(hbm_paths[0]); index ++) {
        if (access(hbm_paths[index], 0) == 0) {
            LOG_E(LOG_TAG, "hbm path index %d, path:%s", index, hbm_paths[index]);
            break;
        }
    }

    if (index == sizeof(hbm_paths)/sizeof(hbm_paths[0])) {
        LOG_E(LOG_TAG, "no hbm path available");
        return GF_ERROR_BASE;
    }

    int fd = open(hbm_paths[index], O_RDWR);
    if (fd < 0) {
        LOG_E(LOG_TAG, "SetAction open err1 :%d, errno =%d", fd, errno);
        return  GF_ERROR_BASE;
    }

    /* read the hbm state */
    char hbm_value[8] = {0};
    read(fd, hbm_value, 1);
    int hbm_mode = atoi(hbm_value);
    LOG_E(LOG_TAG, "read hbm_value:%s mode:%d", hbm_value, hbm_mode);
    if (needSetHbm(hbm_mode, (int32_t)Mode) == 0) {
        close(fd);
        return GF_SUCCESS;
    }

    snprintf(buf, sizeof(buf), "%d", Mode);
    write(fd, buf, 50);
    close(fd);
    usleep(delay_ms * 1000);
    return GF_SUCCESS;
}

gf_error_t FingerprintCore::setBrightness(uint32_t Mode) {
    char buf[50] = {'\0'};
    FUNC_ENTER();
    LOG_E(LOG_TAG, "setBrightness start %d", Mode);
    unsigned int delay_ms = 0;
    int index = 0;

    char *brightness_paths[] = {
        "/sys/class/leds/lcd-backlight/brightness",
        "/sys/class/backlight/panel0-backlight/brightness",
    };
    fpTransforInfo info;
    memset(&info, 0, sizeof(info));
    info.cmd = FP_CMD_GET_BRIGHT_DELAY;
    info.response = -1;
    getFpConfigData((void *)&info);
    delay_ms = info.response;
    LOG_E(LOG_TAG, "bright delay time:%d ms", delay_ms);

    for (index = 0; index < sizeof(brightness_paths)/sizeof(brightness_paths[0]); index ++) {
        if (access(brightness_paths[index], 0) == 0) {
            LOG_E(LOG_TAG, "Brightness path index %d, path:%s", index, brightness_paths[index]);
            break;
        }
    }
    if (index == sizeof(brightness_paths)/sizeof(brightness_paths[0])) {
        LOG_E(LOG_TAG, "no brightness path available");
        return GF_ERROR_BASE;
    }

    int fd = open(brightness_paths[index], O_WRONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "setBrightness err:%d, errno =%d", fd, errno);
        return GF_ERROR_BASE;
    }

    snprintf(buf, sizeof(buf), "%d", Mode);
    write(fd, buf, strlen(buf)+1);
    close(fd);
    usleep(delay_ms * 1000);
    return GF_SUCCESS;
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

gf_error_t FingerprintCore::pauseEnroll() {
    gf_error_t err = GF_SUCCESS;
    LOG_I(LOG_TAG, "enter %s", __func__);

    if (STATE_ENROLL != mWorkState) {
        LOG_E(LOG_TAG, "[%s] state err g_work_state =%d", __func__, mWorkState);
        return GF_ERROR_SET_MODE_FAILED;
    }
    mWorkState = STATE_PAUSE_ENUMERATE;
    err = mContext->mDevice->enable_tp(GF_TP_DISENABLE);
    LOG_I(LOG_TAG, "[%s] err = %d", __func__, err);
    return err;
}

gf_error_t FingerprintCore::continueEnroll() {
    gf_error_t err = GF_SUCCESS;
    if (STATE_PAUSE_ENUMERATE != mWorkState) {
        LOG_E(LOG_TAG, "[%s] state err g_work_state =%d", __func__, mWorkState);
        return GF_ERROR_SET_MODE_FAILED;
    }
    mWorkState = STATE_ENROLL;
    err = mContext->mDevice->enable_tp(GF_TP_ENABLE);
    LOG_I(LOG_TAG, "[%s] err = %d", __func__, err);

    //reset lasttouchmode to touchup
    err = mContext->mDevice->clear_kernel_touch_flag();
    if (err != GF_SUCCESS) {
        LOG_E(LOG_TAG, "[%s] clear_kernel_touch_flag fail, err code : %d.", __func__, err);
    }
    return err;
}

gf_error_t FingerprintCore::checkEnrollResult(gf_error_t result) {
    if (GF_ERROR_HIGH_LIGHT == result) {
        LOG_E(LOG_TAG, "[%s] Ignore HighLight Error", __func__);
        return GF_SUCCESS;
    }
    if (GF_SUCCESS != result) {
        enrollErrorCount++;
        if (enrollErrorCount >= ENROLL_ERROR_MAX) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            message.type = GF_FINGERPRINT_ERROR;
            message.data.error = GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS;
            LOG_E(LOG_TAG, "[%s] Enroll error count max ! : %d.", __func__, enrollErrorCount);
            mNotify(&message);
            enrollErrorCount = 0;
            doCancel();
        }
    } else {
        enrollErrorCount = 0;
    }
    return GF_SUCCESS;
}

gf_error_t FingerprintCore::get_total_enroll_times(uint32_t *enroll_times) {
    gf_error_t err = GF_SUCCESS;
    *enroll_times = mContext->mConfig->enrolling_min_templates;
    LOG_I(LOG_TAG, "[%s] *enroll_times = %d", __func__, *enroll_times);
    return err;
}

gf_error_t FingerprintCore::send_auth_dcsmsg(AuthenticateContext *context, bool notified) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    UNUSED_VAR(context);
    UNUSED_VAR(notified);
    FUNC_EXIT(err);
    return err;
}
}  // namespace goodix
