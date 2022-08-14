/************************************************************************************
 ** File: - FingerprintCore.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix optical fingerprint (android O)
 **
 ** Version: 1.0
 ** Date : 18:03:11,17/10/2018
 ** Author: oujinrong@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  luhongyu   2018/10/17           create file, and add checking result of enroll
 **  oujinrong   2018/10/19           add sleep for UI Ready, set hypnus when touch down
 **  luhongyu    2018/10/31           add debug info, sleep sensor when cancel
 **  luhongyu    2018/11/06           add screen state
 **  Ran.Chen   2018/12/28           add for IMAGE_PASS_SCORE
 **  Ran.Chen   2018/01/07           modify for IMAGE_PASS_SCORE (setvalue 20 )
 **  Ran.Chen   2018/01/28           add for demsg msg in auth mode
 **  Ran.Chen   2019/02/18           modify for coverity 775753 775750
 **  Dongnan.Wu 2019/02/23           add goodix optical HBM & brightness node for 18073
 **  Ran.Chen   2019/03/03           remove notifyAuthFail, when finger move too fast
 **  Ran.Chen   2019/04/08           add for GF_ERROR_RESIDUAL_FINGER msg info
 **  Dongnan.Wu 2019/04/18           modify the way to get timestamp
 **  Ran.Chen   2019/04/22           add hypnus for goodix calabration
 **  Bangxiong.Wu 2019/05/25         move ALGO_VERSION to HalContext.h for more file using
 **  Bangxiong.Wu 2019/07/24         modify hypnusd and hypnus
 **  oujinrong  2019/08/21           add for multi-user
 **  Ran.Chen  2020/01/09           add  for set enrollErrorCount to 0, after send err2 msg
 **  Ran.Chen  2020/02/19         set the right gid number when template is null
 **  Ran.Chen  2020/03/18         modify for fail_result
 **  Ran.Chen  2020/06/06         modify for coverity 828730  828663
 **  Ran.Chen  2020/09/29         add for dcs_event_info v2.0
 **  Bangxiong,Wu 2020/11/10      usleep 51ms after write Brightness Node
 ************************************************************************************/

#define LOG_TAG "[GF_HAL][FingerprintCore]"

#include <string.h>
#include <endian.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
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
#include "ProductTest.h"
#include "SZProductTestDefine.h"
#include "fingerprint.h"
#include "gf_sz_types.h"
#include "SZHalDump.h"
#include <errno.h>
#include <cutils/properties.h>
#include <unistd.h>
#include "FpMonitorTool.h"
#include "Fingerprint_cali_tool.h"
#ifdef FP_CONFIG_SETTINGS_ENABLE
#include "FingerprintConfig.h"
#endif

#ifdef SUPPORT_DSP
#include "HalDsp.h"
extern bool mDspSupport;
#endif   // SUPPORT_DSP

#include "DcsInfo.h"

#define OPLUS_ENROLL_TIME_OUT (10 * 60 * 1000)
#define ENROLL_ERROR_MAX (10)
#define IMAGE_PASS_SCORE (20)
#define TA_VALID (1)
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"
#define FP_SCREEN_ON (1)
#define FP_SCREEN_OFF (0)

uint32_t gDumpLevelVal;

namespace goodix
{

    FingerprintCore::FingerprintCore(HalContext* context) :
                              HalBase(context),
                              mNotify(nullptr),
                              mFidoNotify(nullptr),
                              mAuthDownDetected(false),
                              mTotalKpiTime(0),
                              mWorkState(STATE_IDLE),
                              mEnrollTimerSec(0),
                              mFailedAttempts(0),
                              mEnrollTimer(nullptr),
                              mAuthType(AUTH_TYPE_LOCK),
                              mFingerStatus(STATE_INIT),
                              mAuthUpNotified(false),
                              mScreenStatus(0),
                              mScreenInAuthMode(0),
                              mDSPAvailable(1),
                              enrollErrorCount(0),
                              mGid(0),
                              authTotalTime(0),
                              uiReadyTime(0),
                              captureTime(0),
                              fingerDownTime(0),
                              handleUIReadyEvent(false),
                              dcs_auth_result_type(DCS_DEFAULT_AUTH_RESULT_INFO)
    {
    }

    FingerprintCore::~FingerprintCore()
    {
        mContext->mCenter->deregisterHandler(this);
    }

    gf_error_t FingerprintCore::init()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        doCancel();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::setActiveGroup(uint32_t gid, const char *path)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] gid : %u.", __func__, gid);
        do {
            Mutex::Autolock _l(mContext->mHalLock);
            mGid = gid;
            gf_set_active_group_t cmd;
            memset(&cmd, 0, sizeof(gf_set_active_group_t));
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_SETACTIVITY_GROUP;
            cmd.i_group_id = gid;
            if (path != NULL && strlen(path) < MAX_FILE_ROOT_PATH_LEN)
            {
                memcpy(cmd.i_path, path, strlen(path));
            } else {
                LOG_E(LOG_TAG, "patch is null or length is too long");
            }
            LOG_D(LOG_TAG, "patch (%s)", path);
            err = invokeCommand(&cmd, sizeof(cmd));
        } while(0);
        if (mContext->mDcsInfo->dcs_static_info.need_notify_init_info) {
            mContext->mDcsInfo->sendDcsInitEventInfo(mContext);
            LOG_D(LOG_TAG, "sendDcsInitEventInfo debug");
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::setNotify(fingerprint_notify_t notify)
    {
        gf_error_t err = GF_SUCCESS;
        mNotify = notify;
        return err;
    }

    gf_error_t FingerprintCore::setFidoNotify(fingerprint_notify_t notify)
    {
        gf_error_t err = GF_SUCCESS;
        mFidoNotify = notify;
        return err;
    }

    uint64_t FingerprintCore::preEnroll()
    {
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mContext->mHalLock);
        gf_pre_enroll_cmd_t cmd;
        memset(&cmd, 0, sizeof(gf_pre_enroll_cmd_t));
        FUNC_ENTER();
        do {
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_PRE_ENROLL;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] Some wrong happend in pre enroll", __func__);
                break;
            }
            mPrenrollTime = HalUtils::getrealtime();

        } while(0);
        FUNC_EXIT(err);
        return cmd.o_challenge;
    }

    gf_error_t FingerprintCore::onEnrollRequested(const void *hat, uint32_t gid, uint32_t timeoutSec)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(hat);
        UNUSED_VAR(gid);
        UNUSED_VAR(timeoutSec);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::enroll(const void *hat, uint32_t gid, uint32_t timeoutSec)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] gid : %u.", __func__, gid);
        Mutex::Autolock _l(mContext->mHalLock);
        do
        {
            if (prepareEnrollRequest() != GF_SUCCESS)
            {
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
            gf_enroll_cmd_t cmd;
            memset(&cmd, 0, sizeof(gf_enroll_cmd_t));
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_ENROLL;
            cmd.i_group_id = gid;
            cmd.i_system_auth_token_version = GF_HW_AUTH_TOKEN_VERSION;
            if (NULL != hat)
            {
                memcpy(&cmd.i_auth_token, hat, sizeof(gf_hw_auth_token_t));
            }
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS)
            {
                //cancelEnrollTimer();
#if defined(__ANDROID_O) || defined(__ANDROID_P)
                // support android o vts
                if (GF_ERROR_INVALID_CHALLENGE == err || GF_ERROR_INVALID_HAT_VERSION == err)
                {
                    LOG_E(LOG_TAG, "[%s] hardware unavailable", __func__);
                    notifyErrorInfo(FINGERPRINT_ERROR_HW_UNAVAILABLE);
                    err = GF_SUCCESS;
                }
#endif  // __ANDROID_O
                break;
            }
            mContext->mCenter->registerHandler(this);
            mWorkState = STATE_ENROLL;

            onEnrollRequested(hat, gid, timeoutSec);

            sendMessage(MsgBus::MSG_ENROLL_REQUESTED);
        }
        while (0);
        int64_t now = HalUtils::getrealtime();
        if (now > mPrenrollTime && now - mPrenrollTime > OPLUS_ENROLL_TIME_OUT) {
            startEnrollTimer(100000);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::prepareEnrollRequest()
    {
        return GF_SUCCESS;
    }

    gf_error_t FingerprintCore::onEnrollStart(EnrollContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeEnrollCapture(EnrollContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onAfterEnrollCapture(EnrollContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeEnrollAlgo(EnrollContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onAfterEnrollAlgo(EnrollContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onEnrollStop(EnrollContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
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

    gf_error_t FingerprintCore::onEnrollDownEvt()
    {
        gf_error_t err = GF_SUCCESS;
        EnrollContext context;
        handleUIReadyEvent = true;
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] Enroll Down.", __func__);


        notifyTouch(FINGERPRINT_TOUCH_DOWN);
        sendMessage(MsgBus::MSG_ENROLL_START);
        fingerDownTime = HalUtils::getCurrentTimeMicrosecond();
        onEnrollStart(&context);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onEnrollUpEvt()
    {
        VOID_FUNC_ENTER();
        notifyTouch(FINGERPRINT_TOUCH_UP);

        if (STATE_DOWN == mFingerStatus) {
            onError(GF_ERROR_TOO_FAST);
            mContext->mSensor->sleepSensor();
        }
        VOID_FUNC_EXIT();
        return GF_SUCCESS;
    }

    void FingerprintCore::startEnrollTimer(uint32_t timeoutSec)
    {
        VOID_FUNC_ENTER();
        do
        {
            gf_error_t err = GF_SUCCESS;

            if (NULL == mEnrollTimer)
            {
                mEnrollTimer = Timer::createTimer((timer_thread_t) enrollTimerThread, this);
                if (NULL == mEnrollTimer)
                {
                    LOG_E(LOG_TAG, "[%s] create enroll timer failed", __func__);
                    break;
                }
            }
            mEnrollTimerSec = timeoutSec;
            err = mEnrollTimer->set(0, 0, 0, timeoutSec);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] start enroll timer failed", __func__);
            }
        }
        while (0);

        VOID_FUNC_EXIT();
    }

    void FingerprintCore::cancelEnrollTimer()
    {
        VOID_FUNC_ENTER();
        if (mEnrollTimer != NULL)
        {
            delete mEnrollTimer;
            mEnrollTimer = NULL;
        }
        mEnrollTimerSec = 0;
        VOID_FUNC_EXIT();
    }

    void FingerprintCore::wakeEnrollTimer()
    {
        VOID_FUNC_ENTER();
        if (mEnrollTimer != NULL)
        {
            gf_error_t err = mEnrollTimer->set(0, 0, 0, mEnrollTimerSec);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] wake enroll timer failed", __func__);
            }
        }
        VOID_FUNC_EXIT();
    }

    void FingerprintCore::enrollTimerThread(union sigval v)
    {
        VOID_FUNC_ENTER();
        do
        {
            if (NULL == v.sival_ptr)
            {
                LOG_E(LOG_TAG, "[%s] invalid timer signal data", __func__);
                break;
            }
            FingerprintCore* auth = static_cast<FingerprintCore*>(v.sival_ptr);
            auth->notifyErrorInfo(FINGERPRINT_ERROR_TIMEOUT);
            auth->cancel();
            LOG_E(LOG_TAG, "[%s] enroll timeout", __func__);
        }
        while (0);
        // TODO implement
        VOID_FUNC_EXIT();
    }

    gf_error_t FingerprintCore::postEnroll()
    {
        gf_error_t err = GF_SUCCESS;
        gf_post_enroll_cmd_t cmd;
        memset(&cmd, 0, sizeof(gf_post_enroll_cmd_t));
        FUNC_ENTER();
        cmd.target = GF_TARGET_BIO;
        cmd.cmd_id = GF_CMD_AUTH_POST_ENROLL;
        Mutex::Autolock _l(mContext->mHalLock);
        err = invokeCommand(&cmd, sizeof(cmd));
        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::onEnrollError(EnrollContext* context)
    {
        VOID_FUNC_ENTER();
        onError(context->result);
        VOID_FUNC_EXIT();
    }

    gf_error_t FingerprintCore::notifyEnrollProgress(EnrollContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        uint32_t gid = context->enroll_cmd->o_gid;
        uint32_t fid =  context->enroll_cmd->o_finger_id;
        uint16_t samplesRemaining = context->enroll_cmd->o_samples_remaining;

        LOG_D(LOG_TAG, "[%s] gid=%d, fid=%d, remaining=%d.", __func__, gid, fid, samplesRemaining);

        if (mNotify != nullptr)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_TEMPLATE_ENROLLING;
            message.data.enroll.finger.gid = gid;
            message.data.enroll.finger.fid = fid;
            message.data.enroll.samples_remaining = samplesRemaining;
            mNotify(&message);
            if (mContext->mConfig->support_performance_dump)
            {
                dumpKPI(__func__);
            }
        }
        enrollTotalTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime;
        memcpy(&enrollContext, context, sizeof(EnrollContext));
        if (0 == samplesRemaining) {
            mContext->mDcsInfo->sendDcsEnrollEventInfo(mContext);
        } else {
            mContext->mDcsInfo->sendDcsSingleEnrollEventInfo(mContext);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::prepareAuthRequest()
    {
        return GF_SUCCESS;
    }

    gf_error_t FingerprintCore::onAuthRequested(uint64_t operationId, uint32_t gid)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(operationId);
        UNUSED_VAR(gid);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::authenticate(uint64_t operationId, uint32_t gid)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] gid : %u, mAuthType : %d", __func__, gid, mAuthType);
        Mutex::Autolock _l(mContext->mHalLock);
        do
        {
            if (prepareAuthRequest() != GF_SUCCESS)
            {
                err = GF_ERROR_CANCELED;
                break;
            }

            if (mAuthType == GF_TP_AUTHQKTYPE) {
                err = mContext->mDevice->enable_tp(GP_TP_OTHER);
            } else {
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

            gf_authenticate_cmd_t cmd;
            memset(&cmd, 0, sizeof(gf_authenticate_cmd_t));
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
            onAuthRequested(operationId, gid);

            sendMessage(MsgBus::MSG_AUTHENTICATE_REQUESTED);
#ifdef SWITCH_RAWDATA_MODE
            if (is_rawdata_mode()) {
                fp_monitor_init(mContext->mDevice);
            }
#endif
        }
        while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::authenticateAsType(uint64_t operationId, uint32_t gid, uint32_t authtype)
    {
        UNUSED_VAR(operationId);
        UNUSED_VAR(gid);
        gf_error_t err = GF_SUCCESS;
        mAuthType = (gf_algo_auth_type_t)authtype;
        LOG_I(LOG_TAG, "[%s] authtype =%d.", __func__, mAuthType);

        return err;
    }


    bool FingerprintCore::isRegionSupported()
    {
        bool ret = false;
        char area[PROPERTY_VALUE_MAX] = {'\0'};
        char *area_prop = "ro.vendor.oplus.regionmark";
        property_get(area_prop, area, "0");
        if (strcmp(area, "CN") == 0) {
            ret = true;
        }
        return ret;
    }

    void FingerprintCore::setImageDumpFlag(int flag)
    {
        if (gDumpLevelVal == 0) {
            if (isRegionSupported() == false) {
                property_set(PROPERTY_DUMP_DATA, "0");
                return;
            }
            LOG_I(LOG_TAG, "[%s] setImageDumpFlag, %d.", __func__, flag);
            if (flag) {
                property_set(PROPERTY_DUMP_DATA, "1");
            } else {
                property_set(PROPERTY_DUMP_DATA, "0");
            }
        }
    }

    gf_error_t FingerprintCore::onAuthDownEvt()
    {
        gf_error_t err = GF_SUCCESS;
        AuthenticateContext context;
        handleUIReadyEvent = true;
        FUNC_ENTER();

        LOG_E(LOG_TAG, "[%s] Auth Down.", __func__);
        mAuthDownDetected = true;
        mSkipFlag = false;
        notifyTouch(FINGERPRINT_TOUCH_DOWN);
        sendMessage(MsgBus::MSG_AUTHENTICATE_START);
        mScreenInAuthMode = mScreenStatus;//record Screenstatus in onAuthDownEvt;  //syncbyllh frommaster
        fingerDownTime = HalUtils::getCurrentTimeMicrosecond();
        onAuthStart(&context);
        mFingerStatus = STATE_DOWN;

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onAuthStart(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeAuthCapture(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onAfterAuthCapture(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onBeforeAuthAlgo(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    gf_error_t FingerprintCore::onAfterAuthAlgo(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(context);
        return err;
    }

    bool FingerprintCore::needRetry(AuthenticateContext* context)
    {
        bool retry = false;
        int32_t maxRetry = mContext->mConfig->max_authenticate_failed_attempts;
        if (context->retry < maxRetry)
        {
            retry =  ((GF_ERROR_NOT_MATCH == context->result || GF_ERROR_DSP_GET_FEATURE_FAIL == context->result) && (!mContext->mCenter->hasUpEvt()));
            // only for cache
            if (context->retry == 1 && mSkipFlag) {
                retry &= context->auth_cmd->o_retry_flag;
            }
        }
        else
        {
            retry =  false;
        }
        return retry;
    }

    gf_error_t FingerprintCore::onAfterAuthRetry(AuthenticateContext* context)
    {
        return context->result;
    }

    gf_error_t FingerprintCore::onAfterAuthSuccess(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        gf_algo_auth_image_t* auth = context->auth_cmd;
        FUNC_ENTER();
        do
        {
            gf_auth_post_auth_t cmd;
            memset(&cmd, 0, sizeof(gf_auth_post_auth_t));
            cmd.i_retry_count = auth->i_retry_count;
            cmd.i_gid = auth->o_gid;
            cmd.i_finger_id = auth->o_finger_id;
            cmd.i_study_flag = auth->o_study_flag;
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_POST_AUTHENTICATE;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] authImageFinish fail", __func__);
                break;
            }
            if (cmd.o_save_flag > 0)
            {
                //  teplate saved in ta
                sendMessage(MsgBus::MSG_AUTHENTICATE_SAVE_TEMPLATE_END);
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onAuthStop(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;

        if (GF_SUCCESS == context->result)
        {
            doCancel();
        }

        return err;
    }
#ifdef FP_HYPNUSD_ENABLE
    gf_error_t FingerprintCore::set_hypnus(int32_t action_type, int32_t action_timeout) {
        gf_error_t err = GF_SUCCESS;

        LOG_E(LOG_TAG, "[%s] entry, action_type is %d, action_timeout is %d", __func__, action_type, action_timeout);
        fingerprint_msg_t message;
        memset(&message, 0, sizeof(fingerprint_msg_t));

        message.type = FINGERPRINT_HYPNUSSETACION;
        message.data.hypnus_setting.action_type = action_type;
        message.data.hypnus_setting.action_timeout = action_timeout;

        mNotify(&message);
        return err;
    }
#endif

    gf_error_t FingerprintCore::send_auth_dcsmsg(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        notifyAcquiredInfo(FINGERPRINT_ACQUIRED_GOOD);
        if (mNotify != nullptr)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_AUTHENTICATED_DCSSTATUS;
            switch (context->result)
            {
                case GF_SUCCESS:
                {
                    if (0 != context->auth_cmd->o_finger_id)
                    {
                            message.data.auth_dcsmsg.auth_result = 1;
                    } else{
                        return GF_SUCCESS;
                    }
                    break;
                }
                case GF_ERROR_NOT_MATCH:
                case GF_ERROR_NOT_LIVE_FINGER:
                case GF_ERROR_NOT_MATCH_NOT_LIVE_FINGER:
                case GF_ERROR_RESIDUAL_FINGER:
                {
                    message.data.auth_dcsmsg.auth_result = 0;
                    break;
                }
                default:
                    return GF_SUCCESS;
                    break;
            }
            message.data.auth_dcsmsg.fail_reason = context->result;
            message.data.auth_dcsmsg.quality_score = context->auth_cmd->o_image_quality;
            message.data.auth_dcsmsg.match_score = context->auth_cmd->o_match_score;
            message.data.auth_dcsmsg.signal_value = context->auth_cmd->o_sig_val;
            message.data.auth_dcsmsg.img_area = context->auth_cmd->o_valid_area;
            message.data.auth_dcsmsg.retry_times = context->retry;
            if (this->report_data.is_ta_valid) {
                memcpy(message.data.auth_dcsmsg.algo_version, this->report_data.algo_version, 16);
            } else {
                memcpy(message.data.auth_dcsmsg.algo_version, ALGO_VERSION, strlen(ALGO_VERSION));
            }

            message.data.auth_dcsmsg.chip_ic = mContext->mSensorId.sensor_id;
            message.data.auth_dcsmsg.module_type = mContext->mSensor->mModuleType;
            message.data.auth_dcsmsg.lense_type = mContext->mSensor->mLenseType;
            message.data.auth_dcsmsg.dsp_availalbe = mDSPAvailable;

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
            mNotify(&message);

        }

        FUNC_EXIT(err);
        return err;
    }


    gf_error_t FingerprintCore::notifyAuthSuccess(AuthenticateContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        notifyAcquiredInfo(FINGERPRINT_ACQUIRED_GOOD);
        dcs_auth_result_type = DCS_AUTH_SUCCESS;
        if (mNotify != nullptr)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_AUTHENTICATED;
            message.data.authenticated.finger.gid = context->auth_cmd->o_gid;
            message.data.authenticated.finger.fid = context->auth_cmd->o_finger_id;
            LOG_D(LOG_TAG, "[%s] Auth success, fid = %u", __func__,  context->auth_cmd->o_finger_id);
            memcpy(&message.data.authenticated.hat, &(context->auth_cmd->io_auth_token), sizeof(gf_hw_auth_token_t));
            mNotify(&message);
            if (mContext->mConfig->support_performance_dump)
            {
                dumpKPI(__func__);
            }
        }
        mFailedAttempts = 0;

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::notifyAuthFail()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        if (mNotify != nullptr)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_AUTHENTICATED;
            message.data.authenticated.finger.gid = 0;
            message.data.authenticated.finger.fid = 0;
            LOG_D(LOG_TAG, "[%s] Auth fail", __func__);
            mNotify(&message);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onAuthUpEvt()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        if (mAuthDownDetected)
        {
            mAuthDownDetected = false;
        }

        if (STATE_DOWN == mFingerStatus) {
            onError(GF_ERROR_TOO_FAST);

            mContext->mSensor->sleepSensor();
        }

        FUNC_EXIT(err);
        return err;
    }

    // overide IEventHandler
    gf_error_t FingerprintCore::onEvent(gf_event_type_t e)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] event = %d workstate = %d", __func__, e, mWorkState);
        if(EVENT_FINGER_DOWN == e){
#ifdef FP_BINDCORE_BYTID
            bind_bigcore_bytid();
#ifdef FP_SET_PRIORITY
            setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_VIDEO);
#endif
#endif
        }

        if (mWorkState == STATE_ENROLL)
        {
            err = onEnrollReceivedEvt(e);
        }
        else if (mWorkState == STATE_AUTHENTICATE)
        {
            err = onAuthReceivedEvt(e);
        }
        else
        {
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
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_BINDCORE;
            message.data.bindcore_setting.tid = gettid();
            LOG_D(LOG_TAG, "[%s]  bindcore_tid = %u", __func__, message.data.bindcore_setting.tid);
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
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_SETUXTHREAD;
            message.data.setuxthread_info.pid = getpid();
            message.data.setuxthread_info.tid = gettid();
            message.data.setuxthread_info.enable = enable;
            LOG_D(LOG_TAG, "[%s]  tid = %u, enable = %d  ", __func__, message.data.setuxthread_info.tid, enable);
            mNotify(&message);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onEnrollReceivedEvt(gf_event_type_t event)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        switch (event)
        {
            case EVENT_FINGER_DOWN:
            {
                err = onEnrollDownEvt();
                mFingerStatus = STATE_DOWN;
                break;
            }
            case EVENT_FINGER_UP:
            {
                err = onEnrollUpEvt();
                mFingerStatus = STATE_UP;
                break;
            }
            case EVENT_IRQ_RESET:
            {
                break;
            }
            case EVENT_UI_READY:
            {
                if (handleUIReadyEvent) {
                    err = onEnrollUIReady();
                    mFingerStatus = STATE_UIREADY;
                }
                break;
            }
            default:
            {
                LOG_D(LOG_TAG, "[%s] event %d is not handled.", __func__, event);
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::sensorIsBroken()
    {
        notifyErrorInfo(FINGERPRINT_ERROR_HW_UNAVAILABLE);
    }

    void FingerprintCore::onError(gf_error_t err)
    {
        VOID_FUNC_ENTER();
#ifdef SWITCH_RAWDATA_MODE
        if (is_rawdata_mode()) {
            err = GF_ERROR_TOO_FAST;
        }
#endif
        notifyAcquiredInfo(FINGERPRINT_ACQUIRED_GOOD);
        dcs_auth_result_type = DCS_AUTH_OTHER_FAIL_REASON;
        switch (err)
        {
            case GF_ERROR_TOO_FAST:
            {
                LOG_E(LOG_TAG, "[%s] Up too fast.", __func__);
                if ((mWorkState == STATE_AUTHENTICATE) || (mWorkState == STATE_ENROLL))
                {
                    notifyAcquiredInfo(FINGERPRINT_ACQUIRED_TOO_FAST);
                }
                break;
            }
            case GF_ERROR_SENSOR_IS_BROKEN:
            {
                sensorIsBroken();
                break;
            }
            case GF_ERROR_ALGO_DIRTY_FINGER:
            case GF_ERROR_ALGO_COVER_BROKEN:
            case GF_ERROR_ACQUIRED_IMAGER_DIRTY:
            case GF_ERROR_ANOMALY_FINGER:
            {
                notifyAcquiredInfo(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                if ((mWorkState == STATE_AUTHENTICATE) && (mScreenStatus == FP_SCREEN_ON)) {
                    notifyAuthNotMatched();
                } else if ((mWorkState == STATE_AUTHENTICATE) && (mScreenStatus == FP_SCREEN_OFF)) {
                    onAuthUpEvt();
                } else {
                    LOG_D(LOG_TAG, "%s no states need to change", __func__);
                }
                break;
            }
            case GF_ERROR_SENSOR_BROKEN_CHECK_ALGO_ERROR:
            case GF_ERROR_ACQUIRED_PARTIAL:
            {
                notifyAcquiredInfo(FINGERPRINT_ACQUIRED_PARTIAL);

                if ((mWorkState == STATE_AUTHENTICATE) && (mScreenStatus == FP_SCREEN_ON)) {
                    notifyAuthNotMatched();
                } else if ((mWorkState == STATE_AUTHENTICATE) && (mScreenStatus == FP_SCREEN_OFF)) {
                    onAuthUpEvt();
                } else {
                    LOG_D(LOG_TAG, "no states need to change", __func__);
                }
                break;
            }
            case GF_ERROR_DUPLICATE_FINGER:
            {
                notifyAcquiredInfo(FINGERPRINT_ACQUIRED_VENDOR_DUPLICATE_FINGER);
                break;
            }
            case GF_ERROR_DUPLICATE_AREA:
            {
                notifyAcquiredInfo(FINGERPRINT_ACQUIRED_VENDOR_SAME_AREA);
                break;
            }
            case GF_ERROR_SPI_RAW_DATA_CRC_FAILED:
            {
                notifyErrorInfo(GF_FINGERPRINT_ERROR_SPI_COMMUNICATION);
                if (mWorkState == STATE_AUTHENTICATE) {
                    notifyAuthNotMatched();
                }
                break;
            }
            case GF_ERROR_NOT_MATCH:
            case GF_ERROR_NOT_LIVE_FINGER:
            case GF_ERROR_NOT_MATCH_NOT_LIVE_FINGER:
            case GF_ERROR_RESIDUAL_FINGER:
            case GF_ERROR_DSP_GET_FEATURE_FAIL:
            {
                notifyAuthNotMatched();
                break;
            }
            case GF_ERROR_PREPROCESS_FAILED:
            {
                LOG_E(LOG_TAG, "[%s] GF_ERROR_PREPROCESS_FAILED. mWorkState = %d", __func__, mWorkState);
                if (mWorkState == STATE_ENROLL) {
                    notifyAcquiredInfo(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                }
                else {
                    notifyAuthNotMatched();
                }
                break;
            }
            default:
            {
                notifyErrorInfo(GF_FINGERPRINT_ERROR_INVALID_DATA);
                break;
            }
        }
        VOID_FUNC_EXIT();
    }

    void FingerprintCore::notifyAuthNotMatched()
    {
        VOID_FUNC_ENTER();
        dcs_auth_result_type = DCS_AUTH_FAIL;
        if (mNotify != nullptr)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_AUTHENTICATED;
            message.data.authenticated.finger.gid = 0;
            message.data.authenticated.finger.fid = 0;  // 0 is authenticate failed
            mNotify(&message);
        }
        mFailedAttempts++;

        if (mContext->mConfig->support_performance_dump)
        {
            dumpKPI(__func__);
        }
        VOID_FUNC_EXIT();
    }

    void FingerprintCore::onAuthError(AuthenticateContext* context)
    {
        VOID_FUNC_ENTER();
        onError(context->result);
        VOID_FUNC_EXIT();
    }

    gf_error_t FingerprintCore::onAuthReceivedEvt(gf_event_type_t event)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        switch (event)
        {
            case EVENT_FINGER_DOWN:
            {
                err = onAuthDownEvt();
                mFingerStatus = STATE_DOWN;
                break;
            }
            case EVENT_FINGER_UP:
            {
                err = onAuthUpEvt();
                mFingerStatus = STATE_UP;
                break;
            }
            case EVENT_IRQ_RESET:
            {
                break;
            }
            case EVENT_UI_READY:
            {
                if (handleUIReadyEvent) {
                    err = onAuthUIReady();
                    mFingerStatus = STATE_UIREADY;
                }
                break;
            }
            default:
            {
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    uint64_t FingerprintCore::getAuthenticatorId()
    {
        gf_error_t err = GF_SUCCESS;
        gf_get_auth_id_t cmd;
        memset(&cmd, 0, sizeof(gf_get_auth_id_t));
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_GET_ID;
        FUNC_ENTER();
        Mutex::Autolock _l(mContext->mHalLock);
        err = invokeCommand(&cmd, sizeof(cmd));
        FUNC_EXIT(err);
        return cmd.o_auth_id;
    }

    gf_error_t FingerprintCore::remove(uint32_t gid, uint32_t fid)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, gid, fid);
        Mutex::Autolock _l(mContext->mHalLock);
        WORK_STATE old = mWorkState;
        mWorkState = STATE_REMOVE;
        do
        {
            uint32_t remainingTemplates = 0;
            gf_remove_t cmd;
            memset(&cmd, 0, sizeof(gf_remove_t));
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_REMOVE;
            cmd.i_group_id = gid;
            cmd.i_finger_id = fid;
            err = invokeCommand(&cmd, sizeof(cmd));
            // android framework require success
            err = GF_SUCCESS;
            if (0 == cmd.o_removing_templates)
            {
                LOG_D(LOG_TAG, "[%s] no fingers are removed.", __func__);
#if defined(__ANDROID_O) || defined(__ANDROID_P)
                remainingTemplates = 0;
                notifyRemove(gid, fid, remainingTemplates);
#endif  // __ANDROID_O
            }
            else
            {
                for (uint32_t i = 0; i < MAX_FINGERS_PER_USER && 0 != cmd.o_deleted_fids[i]; i++)
                {
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
        }
        while (0);

        if (err == GF_SUCCESS) {
            enumerate();
        }
        mWorkState = old;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::removeTemplates(uint32_t gid, uint32_t fid)
    {
        gf_remove_t cmd;
        memset(&cmd, 0, sizeof(gf_remove_t));
        gf_error_t err = GF_SUCCESS;
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_REMOVE;
        cmd.i_group_id = gid;
        cmd.i_finger_id = fid;
        FUNC_ENTER();
        err = invokeCommand(&cmd, sizeof(cmd));
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] Remove template error.", __func__);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::notifyRemove(uint32_t gid, uint32_t fid, uint32_t remainingTemplates)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        fingerprint_msg_t message;
        memset(&message, 0, sizeof(fingerprint_msg_t));
        fingerprint_finger_id_t pfpList[MAX_FINGERS_PER_USER];
        memset(pfpList, 0, sizeof(pfpList));
        int32_t fingerCount = MAX_FINGERS_PER_USER;

        UNUSED_VAR(remainingTemplates);
        message.type = FINGERPRINT_TEMPLATE_REMOVED;
        message.data.removed.finger.gid = gid;
        message.data.removed.finger.fid = fid;
        message.data.removed.finger_count = remainingTemplates;

        do {
            err = enumerate(pfpList, (uint32_t *) &fingerCount);

            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] gf_hal_common_enumerate.", __func__);
                break;
            }
            for (int i = 0; i < fingerCount; i++) {
                message.data.removed.total_fingers[i]= pfpList[i];
            }
            if (mNotify != nullptr)
            {
                mNotify(&message);
            }
        } while(0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::enumerate()
    {
        gf_error_t err = GF_SUCCESS;
        gf_enumerate_t cmd;
        memset(&cmd, 0, sizeof(gf_enumerate_t));
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

    gf_error_t FingerprintCore::notifyEnumerate(gf_enumerate_t* result)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        assert(result != nullptr);
        if (mNotify != nullptr)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_TEMPLATE_ENUMERATING;
            if (result->o_size == 0)
            {
                message.data.enumerated.gid = mGid;
                LOG_D(LOG_TAG, "[%s] gid=%d, template is null", __func__, mGid);
                mNotify(&message);
            }
            else
            {
                for (uint32_t i = 0; i < result->o_size; i++)
                {
                    message.data.enumerated.finger[i].gid = result->o_group_ids[i];
                    message.data.enumerated.finger[i].fid = result->o_finger_ids[i];
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

    gf_error_t FingerprintCore::enumerate(void *result, uint32_t *maxSize)
    {
        gf_error_t err = GF_SUCCESS;
        uint32_t count = 0;
        uint32_t len = 0;
        fingerprint_finger_id_t *results = (fingerprint_finger_id_t *) result;
        FUNC_ENTER();
        Mutex::Autolock _l(mContext->mHalLock);
        do
        {
            if (results == nullptr || maxSize == nullptr)
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            len = (mContext->mConfig->max_fingers_per_user) < *maxSize ?
                    (mContext->mConfig->max_fingers_per_user) : *maxSize;
            LOG_D(LOG_TAG, "[%s] size=%u", __func__, len);
            gf_enumerate_t cmd;
            memset(&cmd, 0, sizeof(gf_enumerate_t));
            cmd.header.target = GF_TARGET_BIO;
            cmd.header.cmd_id = GF_CMD_AUTH_ENUMERATE;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            for (uint32_t i = 0; i < len; i++)
            {
                LOG_D(LOG_TAG, "[%s] group_id[%d]=%u, finger_id[%d]=%u", __func__, i,
                        cmd.o_group_ids[i], i, cmd.o_finger_ids[i]);

                if (cmd.o_finger_ids[i] != 0)
                {
                    results[count].gid = cmd.o_group_ids[i];
                    results[count].fid = cmd.o_finger_ids[i];
                    count++;
                }
            }
        }
        while (0);

        if (maxSize != nullptr)
        {
            *maxSize = count;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::cancel()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        Mutex::Autolock _l(mContext->mHalLock);
        gf_cancel_cmd_t cmd;
        memset(&cmd, 0, sizeof(gf_cancel_cmd_t));
        handleUIReadyEvent = false;

        /* cancel command */
        LOG_E(LOG_TAG, "[%s] Fingerprint cancel.", __func__);
        cmd.target = GF_TARGET_BIO;
        cmd.cmd_id = GF_CMD_AUTH_CANCEL;
        err = invokeCommand(&cmd, sizeof(gf_cancel_cmd_t));
        sendMessage(MsgBus::MSG_CANCELED);
        doCancel();

        err = mContext->mDevice->enable_tp(GF_TP_DISENABLE);
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] disable tp fail, err code : %d.", __func__, err);
        }

        /* sleep sensor */
        mContext->mSensor->sleepSensor();

        notifyErrorInfo(FINGERPRINT_ERROR_CANCELED);

        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::doCancel()
    {
        VOID_FUNC_ENTER();
        do
        {
            if (mWorkState == STATE_IDLE)
            {
                break;
            }
            mWorkState = STATE_IDLE;
            mContext->mCenter->deregisterHandler(this);
            cancelEnrollTimer();
            enrollErrorCount = 0;
        }
        while (0);
        VOID_FUNC_EXIT();
    }

    // notify message
    gf_error_t FingerprintCore::notifyAcquiredInfo(fingerprint_acquired_info_t info)
    {
        gf_error_t err = GF_SUCCESS;
        fingerprint_msg_t message;
        memset(&message, 0, sizeof(fingerprint_msg_t));
        FUNC_ENTER();
        if (nullptr != mNotify)
        {
            LOG_D(LOG_TAG, "[%s] acquired_info=%d", __func__, info);
            message.type = FINGERPRINT_ACQUIRED;
            message.data.acquired.acquired_info = (fingerprint_acquired_info_t) info;
            mNotify(&message);
        }
        if (GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT == info)
        {
            sendMessage(MsgBus::MSG_WAIT_FOR_FINGER_INPUT);
        }
        FUNC_EXIT(err);
        return err;
    }

#ifdef FP_CONFIG_SETTINGS_ENABLE
    gf_error_t FingerprintCore::getFpConfigData(void *para)
    {
        gf_error_t err = GF_SUCCESS;
        fingerprint_msg_t message;
        memset(&message, 0, sizeof(fingerprint_msg_t));
        FUNC_ENTER();
        if (nullptr != mNotify)
        {
            message.type = FINGERPRINT_GET_CONFIG_DATA;
            message.data.data_config.para = para;
            mNotify(&message);
        }
        FUNC_EXIT(err);
        return err;
    }
#endif

    gf_error_t FingerprintCore::notifyErrorInfo(fingerprint_error_t error)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        if (nullptr != mNotify)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_ERROR;
            message.data.error = error;
            LOG_E(LOG_TAG, "[%s] err code : %d.", __func__, message.data.error);
            mNotify(&message);
            // FIXME: doCancel in enroll?
            // doCancel();
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::notifyTouch(fingerprint_msg_type_t type)
    {
        gf_error_t err = GF_SUCCESS;
        fingerprint_msg_t message;
        memset(&message, 0, sizeof(fingerprint_msg_t));
        FUNC_ENTER();
        if (nullptr != mNotify)
        {
            if (type == FINGERPRINT_TOUCH_UP) {
                if (true == getAuthUpNotified()) {
                    FUNC_EXIT(err);
                    return err;
                }
                LOG_D(LOG_TAG, "[%s] notifyTouch Up", __func__);
                setAuthUpNotified(true);
            } else {
                LOG_D(LOG_TAG, "[%s] notifyTouch Down", __func__);
                setAuthUpNotified(false);
            }
            message.type = type;
            mNotify(&message);
        }

        FUNC_EXIT(err);
        return err;
    }

    FingerprintCore::WORK_STATE FingerprintCore::getWorkState()
    {
        return mWorkState;
    }

    gf_error_t FingerprintCore::saveTemplates(uint32_t gid, uint32_t fid)
    {
        gf_error_t err = GF_SUCCESS;
        gf_auth_save_templates_t templates;
        memset(&templates, 0, sizeof(gf_auth_save_templates_t));
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] gid : %u.", __func__, gid);
        templates.header.target = GF_TARGET_BIO;
        templates.header.cmd_id = GF_CMD_AUTH_SAVE_TEMPLATES;
        templates.i_gid = gid;
        templates.i_finger_id = fid;
        err = invokeCommand(&templates, sizeof(gf_auth_save_templates_t));
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::authenticateFido(uint32_t groupId, uint8_t *aaid, uint32_t aaidLen,
            uint8_t *finalChallenge, uint32_t challengeLen)
    {
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

    gf_error_t FingerprintCore::cancelFido()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        // TODO
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::getIdList(uint32_t gid, uint32_t *list, int32_t *count)
    {
        fingerprint_finger_id_t pfpList[MAX_FINGERS_PER_USER];
        memset(pfpList, 0, sizeof(pfpList));
        int32_t fingerCount = MAX_FINGERS_PER_USER;
        gf_error_t err = GF_SUCCESS;
        int32_t i = 0;
        int32_t num = 0;
        FUNC_ENTER();

        do
        {
            if (list == NULL || count == NULL)
            {
                LOG_E(LOG_TAG, "[%s] bad parameter: list or count is NULL", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            err = enumerate(pfpList, (uint32_t *) &fingerCount);

            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] gf_hal_common_enumerate.", __func__);
                break;
            }

            if (*count < fingerCount)
            {
                LOG_E(LOG_TAG, "[%s] count is smaller than fp_count, *count=%d, fp_count=%d",
                        __func__, *count, fingerCount);
                break;
            }

            for (i = 0; i < fingerCount; i++)
            {
                if (pfpList[i].gid == gid)
                {
                    list[i] = pfpList[i].fid;
                    num++;
                }
            }
            *count = num;
        }
        while (0);

        if ((GF_SUCCESS != err) && (count != NULL))
        {
            *count = 0;
        }

        FUNC_EXIT(err);
        return err;
    }

    uint8_t FingerprintCore::isIdValid(uint32_t gid, uint32_t fid)
    {
        gf_error_t err = GF_SUCCESS;
        fingerprint_finger_id_t pfpList[MAX_FINGERS_PER_USER];
        memset(pfpList, 0, sizeof(pfpList));
        uint32_t fingerCount = MAX_FINGERS_PER_USER;
        uint32_t i = 0;
        uint8_t isIdValid = 0;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "finger_id=%u", fid);

        do
        {
            err = enumerate(pfpList, &fingerCount);

            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] enumerate.", __func__);
                break;
            }

            for (i = 0; i < fingerCount; i++)
            {
                LOG_D(LOG_TAG, "[%s] finger_id=%u, fid[%d]=%u", __func__, fid, i, pfpList[i].fid);

                if (pfpList[i].fid == fid && pfpList[i].gid == gid)
                {
                    LOG_D(LOG_TAG, "[%s] finger_id is valid", __func__);
                    isIdValid = 1;
                    break;
                }
            }

            err = isIdValid == 1 ? GF_SUCCESS : GF_ERROR_INVALID_FP_ID;
        }
        while (0);
        FUNC_EXIT(err);
        return isIdValid;
    }

    gf_error_t FingerprintCore::dumpKPI(const char *func_name)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            if (nullptr == func_name)
            {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] func_name is nullptr", __func__)
                break;
            }

            // get kpi
            gf_algo_kpi_t kpi;
            memset(&kpi, 0, sizeof(gf_algo_kpi_t));
            kpi.header.target = GF_TARGET_ALGO;
            kpi.header.cmd_id = GF_CMD_ALGO_KPI;
            gf_error_t err = invokeCommand(&kpi, sizeof(gf_algo_kpi_t));
            if (GF_SUCCESS != err)
            {
                break;
            }

            gf_algo_performance_t* dump_performance = &kpi.o_performance;
            // do dump
#ifdef SUPPORT_DSP
            if (mDspSupport) {
                dump_performance->get_feature_time += (uint32_t)(mContext->mDsp->mDspTime);
            }
#endif
            LOG_I(LOG_TAG, "[%s]     image_quality=%d", func_name, dump_performance->image_quality);
            LOG_I(LOG_TAG, "[%s]     valid_area=%d", func_name, dump_performance->valid_area);
            LOG_I(LOG_TAG, "[%s]     key_point_num=%d", func_name, dump_performance->key_point_num);
            LOG_I(LOG_TAG, "[%s]     get_raw_data_time=%dms", func_name, dump_performance->get_raw_data_time / 1000);
            LOG_I(LOG_TAG, "[%s]     preprocess_time=%dms", func_name, dump_performance->preprocess_time / 1000);
            LOG_I(LOG_TAG, "[%s]     get_feature_time=%dms", func_name, dump_performance->get_feature_time / 1000);

            switch (mWorkState)
            {
                case STATE_ENROLL:
                {
                    LOG_I(LOG_TAG, "[%s]     increase_rate=%d", func_name, dump_performance->increase_rate);
                    LOG_I(LOG_TAG, "[%s]     overlay=%d", func_name, dump_performance->overlay);
                    LOG_I(LOG_TAG, "[%s]     enroll_time=%dms", func_name, dump_performance->enroll_time / 1000);
                    break;
                }
                case STATE_AUTHENTICATE:
                {
                    LOG_I(LOG_TAG, "[%s]     study flag =%u", func_name, dump_performance->authenticate_study_flag);
                    LOG_I(LOG_TAG, "[%s]     match_score=%d", func_name, dump_performance->match_score);
                    LOG_I(LOG_TAG, "[%s]     authenticate_time=%dms", func_name, dump_performance->authenticate_time / 1000);
                    LOG_I(LOG_TAG, "[%s]     fake detect time=%dms", func_name, dump_performance->fake_detect_time / 1000);
                    LOG_I(LOG_TAG, "[%s]     KPI time(get_raw_data_time + preprocess+get_feature_time+fake_time+authenticate)=%dms",
                            func_name, (dump_performance->get_raw_data_time
                            + dump_performance->preprocess_time
                            + dump_performance->get_feature_time
                            + dump_performance->authenticate_time
                            + dump_performance->fake_detect_time) / 1000);
                    break;
                }
                default:
                {
                    err = GF_ERROR_GENERIC;
                    break;
                }
            }
            if (GF_SUCCESS == err)
            {
                LOG_I(LOG_TAG, "[%s]    total time=%ums", func_name, (uint32_t)(mTotalKpiTime/ 1000));
            }
        }
        while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::screenState(uint32_t state)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            if (state != 0 && state != 1)
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            mScreenStatus = state;
            sendMessage(MsgBus::MSG_SCREEN_STATE, state);

            gf_screen_state_t screen_state;
            memset(&screen_state, 0, sizeof(gf_screen_state_t));
            screen_state.header.target = GF_TARGET_BIO;
            screen_state.header.cmd_id = GF_CMD_AUTH_SCREEN_STATE;
            Mutex::Autolock _l(mContext->mHalLock);
            screen_state.i_screen_state = state;
            if (mWorkState == STATE_AUTHENTICATE)
            {
                screen_state.i_authenticating = 1;
            }
            else
            {
                screen_state.i_authenticating = 0;
            }
            err = invokeCommand(&screen_state, sizeof(gf_screen_state_t));
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::touchdown()
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        LOG_E(LOG_TAG, "[touchdown]in");

#ifdef FP_HYPNUSD_ENABLE
        set_hypnus(ACTION_TYPE, ACTION_TIMEOUT_500);
#endif

        err = mContext->mCenter->postEvent(EVENT_FINGER_DOWN);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::touchup()
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        LOG_E(LOG_TAG, "[touchup] in");

        err = mContext->mCenter->postEvent(EVENT_FINGER_UP);
        FUNC_EXIT(err);
        return err;
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

    gf_error_t FingerprintCore::sendFingerprintCmd(int32_t cmd_id, int8_t* in_buf, uint32_t size)
    {
        gf_error_t err = GF_SUCCESS;
        int8_t* out = NULL;
        uint32_t outLen = 0;
        int cali_brightness = 0;

        FUNC_ENTER();

        LOG_E(LOG_TAG, "[sendFingerprintCmd] cmd_id =%d, %s", cmd_id, ft_id_to_string((int)cmd_id));
#ifdef OPLUS_SUPPORT_STABILITYTEST
        if (cmd_id > 15000) {
            LOG_E(LOG_TAG, "[sendFingerprintCmd] jump in StabilityTest! cmd_id = %d", cmd_id);
            err = mContext->mStabilityTest->onStabilityTestcmd(cmd_id, in_buf, size);
            FUNC_EXIT(err);
            return err;
        }
#endif //OPLUS_SUPPORT_STABILITYTEST

#ifdef FP_HYPNUSD_ENABLE
        set_hypnus(ACTION_TYPE, ACTION_TIMEOUT_1000);
#endif

#ifdef FP_CONFIG_SETTINGS_ENABLE
        fpTransforInfo info;
        info.cmd = FP_CMD_GET_BRIGHTNESS;
        info.response = -1;
        getFpConfigData((void *)&info);
        cali_brightness = info.response;
        LOG_E(LOG_TAG, "[sendFingerprintCmd] brightness =%d", cali_brightness);
#endif

        switch (cmd_id){
            case CMD_TEST_SZ_FT_INIT:
            case CMD_TEST_SZ_FT_CAPTURE_L_FLESH:
            case CMD_TEST_SZ_FT_CAPTURE_L_DARK:
            case CMD_TEST_SZ_FT_SAMPLE_CALI_EXIT:
            {
                setHbmMode(0);
#ifndef FP_CONFIG_SETTINGS_ENABLE
                #ifndef MTK_HBM_NODE
                setBrightness(580);
                #else
                setBrightness(660);
                #endif
#else
                LOG_E(LOG_TAG, "[sendFingerprintCmd] low brightness =%d, cmd1:%d", cali_brightness, cmd_id);
                setBrightness(cali_brightness);
#endif
                break;
            }
            case CMD_TEST_SZ_FT_EXPO_AUTO_CALIBRATION:
            case CMD_TEST_SZ_FT_CAPTURE_H_FLESH:
            case CMD_TEST_SZ_FT_CAPTURE_H_DARK:
            case CMD_TEST_SZ_FT_CAPTURE_CHECKBOX_PERFORMANCE:
            case CMD_TEST_SZ_FT_SAMPLE_CALI_INIT:
            {
                setHbmMode(1);
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_CHART:
            case CMD_TEST_SZ_FT_CAPTURE_CHECKBOX:
            {
                setHbmMode(1);
#ifdef FP_HYPNUSD_ENABLE
                set_hypnus(ACTION_TYPE, ACTION_TIMEOUT_6000);
#endif
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_DARK_BASE:
            {
                setHbmMode(0);
                setBrightness(0);
                break;
            }
            case CMD_TEST_SZ_FT_EXIT:
            {
                setHbmMode(0);
#ifndef FP_CONFIG_SETTINGS_ENABLE
                #ifndef MTK_HBM_NODE
                setBrightness(580);
                #else
                setBrightness(660);
                #endif
#else
                LOG_E(LOG_TAG, "[sendFingerprintCmd] low brightness =%d, cmd2:%d", cali_brightness, cmd_id);
                setBrightness(cali_brightness);
#endif
                break;
            }
            default:
            {
                break;
            }
        }

        err = mContext->mProductTest->onCommand(cmd_id, in_buf, size, &out, &outLen);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::notifyFingerprintCmd(int64_t devId, int32_t cmdId, const int8_t *result, int32_t resultLen)
    {
        UNUSED_VAR(devId);
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

#ifdef FP_CONFIG_SETTINGS_ENABLE
        int cali_brightness = 0;
        fpTransforInfo info;
        info.cmd = FP_CMD_GET_BRIGHTNESS;
        info.response = -1;
        getFpConfigData((void *)&info);
        cali_brightness = info.response;
        LOG_E(LOG_TAG, "[notifyFingerprintCmd] brightness =%d", cali_brightness);
#endif

        if (CMD_TEST_SZ_FT_CAPTURE_DARK_BASE == cmdId)
        {
            setHbmMode(0);
#ifndef FP_CONFIG_SETTINGS_ENABLE
            #ifndef MTK_HBM_NODE
            setBrightness(580);
            #else
            setBrightness(660);
            #endif
#else
            LOG_E(LOG_TAG, "[notifyFingerprintCmd] low brightness =%d, cmd2:%d", cali_brightness, cmdId);
            setBrightness(cali_brightness);
#endif
        }

        if (mNotify != nullptr)
        {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            switch (cmdId)
            {
                case CMD_TEST_OPLUS_GET_IMAGE_QUALITY://CMD_TEST_SZ_FT_INIT:
                {
                    message.type = FINGERPRINT_ENGINEERING_INFO;
                    message.data.engineering.type = FINGERPRINT_IMAGE_QUALITY;
                    message.data.engineering.quality.successed = 1;
                    message.data.engineering.quality.image_quality = ((uint32_t *)result)[0];
                    message.data.engineering.quality.quality_pass = (message.data.engineering.quality.image_quality > IMAGE_PASS_SCORE ? 1:0);

                    LOG_E(LOG_TAG, "[%s] GF_ENGINEERING_INFO, cmdId = %d, result = %s, resultLen = %d,resultLen[0] = %d", __func__, cmdId, result, resultLen, ((uint32_t *)result)[0]);
                    break;
                }
                default:
                {
                    message.type = FINGERPRINT_OPTICAL_SENDCMD;
                    message.data.test.cmd_id = cmdId;
                    message.data.test.result = (int8_t *) result;
                    message.data.test.result_len = resultLen;
                    break;
                }
            }
            LOG_D(LOG_TAG, "[%s] Auth success, cmdId = %d, result = %s, resultLen = %d", __func__, cmdId, result, resultLen);
            mNotify(&message);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::setHbmMode(uint32_t Mode)
    {
        char buf[50] = {'\0'};
        FUNC_ENTER();
        LOG_E(LOG_TAG, "SetActionMode start %d", Mode);
        char *hbm_paths[] = {
            "/sys/kernel/oplus_display/hbm",
            "/sys/devices/virtual/mtk_disp_mgr/mtk_disp_mgr/LCM_HBM",
        };
        int index = 0;

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

        int fd = open(hbm_paths[index], O_WRONLY);
        if (fd < 0) {
            LOG_E(LOG_TAG, "SetAction open err :%d, errno =%d", fd, errno);
            return  GF_ERROR_BASE;
        }
        snprintf(buf, sizeof(buf), "%d", Mode);
        write(fd, buf, 50);
        close(fd);
        #ifdef FP_HBM_BRIGHTNESS_DELAY
        LOG_E(LOG_TAG, "setHbmMode delay 200ms %d", Mode);
        usleep(200*1000);
        #endif
        LOG_E(LOG_TAG, "setHbmMode %d", Mode);
        return GF_SUCCESS;
    }

    gf_error_t FingerprintCore::setBrightness(uint32_t Mode)
    {

        char buf[50] = {'\0'};
        int32_t length = 0;
        FUNC_ENTER();

        char *brightness_paths[] = {
            "/sys/class/leds/lcd-backlight/brightness",
            "/sys/class/backlight/panel0-backlight/brightness",
        };
        int index = 0;

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
            return  GF_ERROR_BASE;
        }
        snprintf(buf, sizeof(buf), "%d", Mode);
        length = write(fd, buf, 50);
        LOG_E(LOG_TAG, "write length %d", length);
        //length = read(fd, buf, 50);
        //LOG_E(LOG_TAG, "read length %d", length);

        close(fd);
        usleep(51*1000);
        #ifdef FP_HBM_BRIGHTNESS_DELAY
        LOG_E(LOG_TAG, "setBrightness delay 200ms %d", Mode);
        usleep(200*1000);
        #endif
        LOG_E(LOG_TAG, "setBrightness %d", Mode);

        return GF_SUCCESS;
    }

    void FingerprintCore::syncFingerList(uint32_t groupId)
    {
        UNUSED_VAR(groupId);
#if defined(__ANDROID_N) || defined(__ANDROID_M)
        syncFingerListBasedOnSettings(mContext, groupId);
#endif  // defined(__ANDROID_N) || defined(__ANDROID_M)
        return;
    }

    void FingerprintCore::setAuthType(uint32_t authType)
    {
        VOID_FUNC_ENTER();
        if (AUTH_TYPE_UNKNOWN < authType && authType <= AUTH_TYPE_OTHER_UNKOWN)
        {
            mAuthType = (gf_algo_auth_type_t)authType;
        }
        VOID_FUNC_EXIT();
    }

    gf_error_t FingerprintCore::pauseEnroll()
    {
        gf_error_t err = GF_SUCCESS;
        LOG_I(LOG_TAG, "enter %s", __func__);

        if(STATE_ENROLL != mWorkState ){
            LOG_E(LOG_TAG, "[%s] state err g_work_state =%d", __func__, mWorkState);
            return GF_ERROR_SET_MODE_FAILED;
        }
        mWorkState = STATE_PAUSE_ENUMERATE;
        err = mContext->mDevice->enable_tp(GF_TP_DISENABLE);
        LOG_I(LOG_TAG, "[%s] err = %d", __func__, err);
        return err;
    }

    gf_error_t FingerprintCore::continueEnroll()
    {
        gf_error_t err = GF_SUCCESS;
        if(STATE_PAUSE_ENUMERATE != mWorkState ){
            LOG_E(LOG_TAG, "[%s] state err g_work_state =%d", __func__, mWorkState);
            return GF_ERROR_SET_MODE_FAILED;
        }
        mWorkState = STATE_ENROLL;
        err = mContext->mDevice->enable_tp(GF_TP_ENABLE);
        LOG_I(LOG_TAG, "[%s] err = %d", __func__, err);
        err = mContext->mDevice->clear_kernel_touch_flag();
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] clear_kernel_touch_flag fail, err code : %d.", __func__, err);
        }

        return err;
    }

    gf_error_t FingerprintCore::checkEnrollResult(gf_error_t result) {
        if (GF_SUCCESS != result) {
            enrollErrorCount++;
            if (enrollErrorCount >= ENROLL_ERROR_MAX) {
                fingerprint_msg_t message;
                memset(&message, 0, sizeof(fingerprint_msg_t));
                message.type = FINGERPRINT_ERROR;
                message.data.error = FINGERPRINT_ERROR_UNABLE_TO_PROCESS;
                mNotify(&message);
                enrollErrorCount = 0;
            }
        } else {
            enrollErrorCount = 0;
        }
        return GF_SUCCESS;
    }

    gf_error_t FingerprintCore::get_total_enroll_times(uint32_t *enroll_times)
    {
        gf_error_t err = GF_SUCCESS;
        *enroll_times = mContext->mConfig->enrolling_min_templates;
        LOG_I(LOG_TAG, "[%s] *enroll_times = %d", __func__, *enroll_times);
        return err;
    }

    gf_error_t FingerprintCore::onAuthUIReady()
    {
        gf_error_t err = GF_SUCCESS;
        gf_algo_auth_image_t cmd;
        memset(&cmd, 0, sizeof(gf_algo_auth_image_t));
        AuthenticateContext context;
        memset(&context, 0, sizeof(context));
        context.result = err;
        context.auth_cmd = &cmd;

#ifdef SUPPORT_DSP
        gf_sz_config_t *szConfig = (gf_sz_config_t*) mContext->mConfig;
        bool dspHighSpeedFlag = false;
#endif   // SUPPORT_DSP
        FUNC_ENTER();

        LOG_E(LOG_TAG, "[%s] onAuthUIReady.", __func__);
        if (mFingerStatus != STATE_DOWN) {
            notifyTouch(FINGERPRINT_TOUCH_UP);
            err = GF_ERROR_OPLUS_NEED_UNVALID_UI_READY;
            // send too fast dcs msg, uiready err
            LOG_E(LOG_TAG, "[%s] unvild ui ready mFingerStatus = %d,err = %d.", __func__, mFingerStatus, err);
            dcs_auth_result_type = DCS_AUTH_TOO_FAST_NO_IMGINFO;
            memcpy(&authContext, &context, sizeof(AuthenticateContext));
            mContext->mDcsInfo->sendDcsAuthEventInfo(mContext);
            FUNC_EXIT(err);
            return err;
        }

        context.up  = 0;
        if (mContext->mConfig->support_performance_dump)
        {
            mTotalKpiTime = 0;
        }
        context.retry = 0;
        context.up = 0;
        do
        {
#ifdef FP_HYPNUSD_ENABLE
            set_hypnus(ACTION_TYPE, ACTION_TIMEOUT_2000);
#endif
#ifdef FP_SET_UXTREAD
            setUxThread(1);
#endif
            /* caculate ui ready time cost */
            uiReadyTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime;   ////syncbyllh frommaster
            LOG_D(LOG_TAG, "[%s] Begin capture after finger down %d ms.", __func__, uiReadyTime / 1000);

            do
            {
#ifdef SUPPORT_DSP
                if (mDspSupport) {
                    // 0. set Dsp in high freq
                    if ((szConfig->disable_retry0_dsp && 1 == context.retry)
                            || (!szConfig->disable_retry0_dsp && 0 == context.retry)) {
                        if (DSP_AVAILABLE == mContext->mDsp->checkDspValid()) {
                            mContext->mDsp->sendCmdToDsp(DSP_CMD_SET_HIGH_SPEED);
                            dspHighSpeedFlag = true;
                            mDSPAvailable = 1;
                        } else {
                            mDSPAvailable = 0;
                        }
                    }
                }
#endif   // SUPPORT_DSP
                // 1. capture image
                if (context.retry != 1) {
                    onBeforeAuthCapture(&context);
#ifdef SUPPORT_DUMP
                    SZHalDump* szdump = static_cast<SZHalDump*>(mContext->mHalDump);
                    err = szdump->szDumpSettingScreenState(mScreenStatus);
                    LOG_D(LOG_TAG, "[%s] mScreenStatus: %d", __func__, mScreenStatus);
#endif  // SUPPORT_DUMP
                    err = mContext->mSensor->captureImage(GF_OPERATION_AUTHENTICATE, context.retry);
                    context.result = err;
                    if (context.result == GF_ERROR_TOO_FAST) {
                        dcs_auth_result_type = DCS_AUTH_TOO_FAST_GET_IMGINFO;
                        memcpy(&authContext, &context, sizeof(AuthenticateContext));
                        #ifdef FP_SET_UXTREAD
                        setUxThread(0);
                        #endif
                        mContext->mDcsInfo->sendDcsAuthEventInfo(mContext);
                    }
                    onAfterAuthCapture(&context);
                    //needRetryIfCaptureFailed  ?  CRC failreason
                    /* caculate capture time cost */
                    captureTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime - uiReadyTime;
                    GF_ERROR_BREAK(err);
                }

                // 2. algo authenticate
                onBeforeAuthAlgo(&context);
                cmd.io_auth_token.version = GF_HW_AUTH_TOKEN_VERSION;
                cmd.io_auth_token.authenticator_type = htobe32(GF_HW_AUTH_FINGERPRINT);
                cmd.i_retry_count = context.retry;
                cmd.i_auth_type = mAuthType;
                cmd.i_recog_round = 1;
                err = mContext->mAlgo->authImage(&cmd);
                context.result = err;
                cmd.o_dismatch_reason = err;
                LOG_E(LOG_TAG, "[%s] antipeep & screen struct flag : %d", __func__, cmd.o_antipeep_screen_struct_flag);
                //LOG_D(LOG_TAG, "[%s] template version mode = [%x]", __func__, cmd.o_tmpl_version);
                if (context.retry == 0 && (cmd.o_round_two || cmd.o_retry_flag)) {
                    mSkipFlag = true;
                }
                LOG_D(LOG_TAG, "[%s] round_two : %d", __func__, cmd.o_round_two);
                LOG_D(LOG_TAG, "[%s] mSkipFlag : %d", __func__, mSkipFlag);
                onAfterAuthAlgo(&context);
                LOG_D(LOG_TAG, "[%s] retry flag : %d", __func__, cmd.o_retry_flag);
                if (GF_SUCCESS == context.result && 0 != cmd.o_finger_id) {
                    if (mContext->mConfig->support_performance_dump)
                    {
                        mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime;
                    }

                    LOG_E(LOG_TAG, "[%s] debug KPI notifyAuthSuccess start", __func__);
                    notifyAuthSuccess(&context);
                    authTotalTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime;
                    LOG_E(LOG_TAG, "[%s] authTotalTime %d", __func__, authTotalTime / 1000);
                    mContext->mDevice->enable_tp(GF_TP_DISENABLE);
                    memcpy(&authContext, &context, sizeof(AuthenticateContext));
                    mContext->mDcsInfo->sendDcsAuthEventInfo(mContext);
                }

                if (needRetry(&context))
                {
                    sendMessage(MsgBus::MSG_AUTHENTICATE_NEED_RETRY, context.result, context.retry, context.auth_cmd->i_recog_round);
                    context.retry++;
                    // retry authenticate
                    LOG_E(LOG_TAG, "[%s] Authenticate is retrying,retry time is %d", __func__, context.retry);
                    continue;
                }
                else
                {
                    break;
                }
            }
            while (true);
#ifdef SUPPORT_DSP
            if (mDspSupport) {
                // 3. set dsp to normal status
                if (dspHighSpeedFlag && DSP_AVAILABLE == mContext->mDsp->checkDspValid()) {
                     mContext->mDsp->sendCmdToDsp(DSP_CMD_SET_NORMAL_SPEED);
                }
            }
#endif   // SUPPORT_DSP

            err = onAfterAuthRetry(&context);
            context.result = err;
            sendMessage(MsgBus::MSG_AUTHENTICATE_RETRY_END, context.result, context.retry);
            GF_ERROR_BREAK(err);
            // 4. do post something, such as study
            onAfterAuthSuccess(&context);
            // dump data after notify when auth success
            sendMessage(MsgBus::MSG_AUTHENTICATE_SUCCESS_NOTIFY_FIRST,context.result,
                context.retry, context.auth_cmd->i_recog_round);
            }
        while (0);

        onAuthStop(&context);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] debug KPI onAuthError start", __func__);
            onAuthError(&context);
            // dump data after notify when auth fail
            sendMessage(MsgBus::MSG_AUTHENTICATE_NOT_MATCH_NOTIFY_FIRST,
                    context.result, context.retry, context.auth_cmd->i_recog_round);

            authTotalTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime;
            LOG_E(LOG_TAG, "[%s] authTotalTime %d", __func__, (int)(authTotalTime / 1000.0));
            //set dcs_auth_result_type in onAuthError;
            memcpy(&authContext, &context, sizeof(AuthenticateContext));
            mContext->mDcsInfo->sendDcsAuthEventInfo(mContext);
        }
#ifdef FP_SET_UXTREAD
        setUxThread(0);
#endif
        LOG_E(LOG_TAG, "[%s] info:0x%x,0x%x,0x%x,0x%x,0x%x", __func__,
            context.auth_cmd->o_fake_para_info[0], context.auth_cmd->o_fake_para_info[1],
            context.auth_cmd->o_fake_para_info[2], context.auth_cmd->o_fake_para_info[3],
            context.auth_cmd->o_fake_para_info[4]);
        sendMessage(MsgBus::MSG_AUTHENTICATE_END);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t FingerprintCore::onEnrollUIReady()
    {
        gf_error_t err = GF_SUCCESS;
        EnrollContext context;
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] onEnrollUIReady.", __func__);
        if (mFingerStatus != STATE_DOWN) {
            LOG_E(LOG_TAG, "[%s] unvild ui ready, err = %d.", __func__, err);
            notifyTouch(FINGERPRINT_TOUCH_UP);
            err = GF_ERROR_OPLUS_NEED_UNVALID_UI_READY;
            FUNC_EXIT(err);
            return err;
        }

        if (mContext->mConfig->support_performance_dump)
        {
            mTotalKpiTime = 0;
        }

        do
        {
            gf_algo_enroll_image_t cmd;
            memset(&cmd, 0, sizeof(gf_algo_enroll_image_t));
            context.enroll_cmd = &cmd;
            uiReadyTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime;
            // 1. capture image
            onBeforeEnrollCapture(&context);
            err = mContext->mSensor->captureImage(GF_OPERATION_ENROLL, 0);
            context.result = err;
            onAfterEnrollCapture(&context);
            captureTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime - uiReadyTime;
            if (err != GF_SUCCESS)
            {
                onEnrollError(&context);
                checkEnrollResult(err);
                break;
            }

            // 2. algo enroll
            onBeforeEnrollAlgo(&context);
            err = mContext->mAlgo->enrollImage(context.enroll_cmd);
            context.result = err;
            onAfterEnrollAlgo(&context);
            checkEnrollResult(err);
            if (err != GF_SUCCESS)
            {
                if (GF_ERROR_NEED_CANCLE_ENROLL== err) {
                    notifyErrorInfo(FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
                } else {
                    onEnrollError(&context);
                }
                break;
            }

            // 3. notify
            notifyAcquiredInfo(FINGERPRINT_ACQUIRED_GOOD);
            if (mContext->mConfig->support_performance_dump)
            {
                mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - fingerDownTime;
            }
            if (cmd.o_samples_remaining == 0)
            {
                // enroll success save template
                err = saveTemplates(cmd.o_gid, cmd.o_finger_id);
                if (err != GF_SUCCESS)
                {
                    notifyErrorInfo(FINGERPRINT_ERROR_NO_SPACE);
                    break;
                }
                else
                {
                    if (mWorkState != STATE_ENROLL)
                    {
                        err = removeTemplates(cmd.o_gid, cmd.o_finger_id);
                        if (err != GF_SUCCESS)
                        {
                            LOG_E(LOG_TAG, "[%s] Remove template error.", __func__);
                        }
                    }
                    else
                    {
                        sendMessage(MsgBus::MSG_ENROLL_SAVE_TEMPLATE_END);
                    }
                }
                context.result = err;
                notifyEnrollProgress(&context);
                if (err == GF_SUCCESS) {
                    enumerate();
                }
                doCancel();
            }
            else
            {
                notifyEnrollProgress(&context);
            }
        }
        while (0);

        sendMessage(MsgBus::MSG_ENROLL_END, &err, sizeof(gf_error_t));
        context.result = err;
        onEnrollStop(&context);

        FUNC_EXIT(err);
        return err;
    }

    void FingerprintCore::init_report_data(void)
    {
        oplus_report_data_t data;
        char lcdtype_prop[PROPERTY_VALUE_MAX] = {0};
        int32_t ret = -1;

        memset(&data, 0, sizeof(data));
        data.header.target = GF_TARGET_ALGO;
        data.header.cmd_id = GF_CMD_ALGO_DATA_INIT;
        gf_error_t err = invokeCommand(&data, sizeof(data));

        if (data.is_ta_valid != TA_VALID) {
            LOG_E(LOG_TAG, "[%s], ta can not dynamic get version", __func__);
            property_set(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, "unknown version");
            return;
        }
        LOG_E(LOG_TAG, "[%s] algo version is %s", __func__, (char *)data.algo_version);
        this->report_data.is_ta_valid = TA_VALID;
        memcpy(this->report_data.algo_version, data.algo_version, STRING_LENGTH);

        ret = property_get("persist.vendor.fingerprint.optical.lcdtype", lcdtype_prop, "");
        if (ret) {
            strcat(this->report_data.algo_version, "_R_");
            strncpy(this->report_data.algo_version + strlen(this->report_data.algo_version), lcdtype_prop, strlen(lcdtype_prop));
            LOG_E(LOG_TAG, "[%s] lcdtype_prop = %s", __func__, lcdtype_prop);
        } else {
            LOG_E(LOG_TAG, "[%s] no lcdtype_prop", __func__);
        }

        property_set(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, this->report_data.algo_version);
        LOG_E(LOG_TAG, "[%s] type = %s", __func__, this->report_data.algo_version);

    }

    void FingerprintCore::getDumpPictureLevel(void)
    {
        gf_dump_level_t dumpLvl;

        memset(&dumpLvl, 0, sizeof(dumpLvl));

        dumpLvl.header.target = GF_TARGET_ALGO;
        dumpLvl.header.cmd_id = GF_CMD_GET_DUMP_LEVEL;
        dumpLvl.i_dump_level  = 100;

        gf_error_t err = invokeCommand(&dumpLvl, sizeof(dumpLvl));
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] getDumpPictureLevel to TA failed", __func__);
            return;
        }

        /***************************************************************************
         *
         * gDumpLevelVal value description:
         *
         * IMAGE_ENCRYPT_SAVE - save encrypted data for trial user of release image
         * IMAGE_PLAIN_SAVE   - save plain data for debug image
         * IMAGE_NOT_SAVE     - no need save for release image expect trial user
         *
         ***************************************************************************/
        gDumpLevelVal = dumpLvl.i_dump_level;

        LOG_E(LOG_TAG, "[%s] gDumpLevelVal = %u", __func__, gDumpLevelVal);

        return;
    }

    gf_error_t FingerprintCore::sendDumpdataToTa(uint8_t* image_data, uint32_t src_len, uint8_t* enc_buf, uint32_t *encrypt_len) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        (void)encrypt_len;
        do {
            if (nullptr == image_data || nullptr == enc_buf || nullptr == encrypt_len) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            uint32_t total_size = sizeof(gf_enc_dump_t) + src_len + 256;
            gf_enc_dump_t *dump_info = (gf_enc_dump_t *)new uint8_t[total_size];
            if (dump_info == nullptr) {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            memset(dump_info, 0, total_size);
            dump_info->header.target = GF_TARGET_ALGO;
            dump_info->header.cmd_id = GF_CMD_AUTH_CUSTOMENC;
            dump_info->size = src_len;
            memcpy(dump_info->dump_data, image_data, src_len);
            err = invokeCommand(dump_info, total_size);
            memcpy(enc_buf, dump_info->dump_data, src_len + 256);
            if (dump_info) {
                delete []dump_info;
            }
        }while(0);

        FUNC_EXIT(err);

        return err;
    }

}   // namespace goodix
