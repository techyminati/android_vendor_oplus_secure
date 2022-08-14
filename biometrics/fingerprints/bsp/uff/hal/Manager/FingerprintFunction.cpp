/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Manager/FingerprintFunction.cpp
 **
 ** Description:
 **      FingerprintFunction for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][FingerprintFunction]"

#include "FingerprintFunction.h"
#include <cutils/properties.h>
#include <endian.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "DcsInfo.h"
#include "Device.h"
#include "HalContext.h"
#include "HalLog.h"
#include "fingerprint.h"
#include "FpCommon.h"
#include "FpType.h"
#include "Device.h"
#include "FingerprintMessage.h"
#include "Dump.h"
#include "Perf.h"
#include "Utils.h"
namespace android {
FingerprintFunction::FingerprintFunction(HalContext *context):
    FingerprintCommon(context),
    mScreenStatus(0),
    current_gid(0),
    mContext(context) {
    LOG_I(LOG_TAG, "enter to FingerprintFunction");
    sem_init(&mUireadySem, 0, 0);
    UNUSED(context);
    mSerialEnrErrCount = 0;
}

FingerprintFunction::~FingerprintFunction() {
    LOG_I(LOG_TAG, "enter to ~FingerprintFunction");
}

//init
fp_return_type_t FingerprintFunction::init() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    fp_init_t cmd;
    memset(&cmd, 0, sizeof(fp_init_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_INIT;
    sendTaCommand(&cmd, sizeof(cmd));
    LOG_I(LOG_TAG, "[%s] spmt state:%d, algo version:%s", __func__, cmd.result.cali_state, cmd.result.algo_version);
    Dump::dumpProcess(FP_MODE_CALI, 0, 0, 0);
    //TBD: init fail
    HalContext::getInstance()->mConfig->setFactoryCaliState(cmd.result.cali_state);
    HalContext::getInstance()->mConfig->setFactoryAlgoVersion(cmd.result.algo_version);
    HalContext::getInstance()->mConfig->setFactoryQrcode(cmd.result.qrcode);
    property_set("persist.vendor.fingerprint.fp_id", "G_OPTICAL_G3S");
    perfBindBigCore();
    err = mContext->mCaEntry->sendHmacKeyToFpta();
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::setNotify(fingerprint_notify_t notify) {
    fp_return_type_t err = FP_SUCCESS;
    mNotify              = notify;
    return err;
}

fp_return_type_t FingerprintFunction::setActiveGroup(uint32_t gid, const char *path) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] gid : %u. path = %s", __func__, gid, path);
    current_gid = gid;
    fp_set_active_group_t cmd;
    memset(&cmd, 0, sizeof(fp_set_active_group_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_SET_ACTIVE_GROUP;
    cmd.config.group_id = gid;
    if (path != NULL && strlen(path) < PATH_LEN) {
        memcpy(cmd.config.path, path, strlen(path));
    } else {
        LOG_E(LOG_TAG, "patch is null or length is too long");
    }
    err = sendTaCommand(&cmd, sizeof(cmd));
    //TBD: load fail;
    //TBD:set Dcs
    if (mContext->mDcsInfo->dcs_static_info.need_notify_init_info) {
        mContext->mDcsInfo->sendDcsInitEventInfo(mContext);
        LOG_D(LOG_TAG, "sendDcsInitEventInfo debug");
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::enumerate() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    fp_enumerate_t cmd;
    memset(&cmd, 0, sizeof(fp_enumerate_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_ENUMERATE;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

//notify
    err = notifyEnumerate(cmd, current_gid);
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::getTotalEnrollTimes(uint32_t *enroll_times) {
    UNUSED(enroll_times);
    fp_return_type_t err = FP_SUCCESS;
    LOG_I(LOG_TAG, "[%s] *enroll_times = %d", __func__, *enroll_times);
    return err;
}

//enroll
uint64_t FingerprintFunction::preEnroll() {
    uint64_t challenge = 0;
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    fp_pre_enroll_t cmd;
    memset(&cmd, 0, sizeof(fp_pre_enroll_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_PRE_ENROLL;
    err = sendTaCommand(&cmd, sizeof(cmd));
    challenge = cmd.result.challenge;
    FUNC_EXIT(err);
    return challenge;
}

fp_return_type_t FingerprintFunction::enroll(const void *hat, uint32_t gid, uint32_t timeoutSec) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] enroll gid = %d, timeout = %d", __func__, gid, timeoutSec);
    current_gid = gid;
    CHECK_RESULT_NULLPTR(hat);

    err = mContext->mDevice->controlTp(DEVICE_ENABLE_TP);
    CHECK_RESULT_SUCCESS(err);
    err = mContext->mDevice->cleanTouchStatus();
    CHECK_RESULT_SUCCESS(err);

    //sendcmd
    fp_enroll_t cmd;
    memset(&cmd, 0, sizeof(fp_enroll_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_ENROLL;
    cmd.config.group_id = gid;
    if (hat != NULL) {
        memcpy(&cmd.config.token, hat, sizeof(fp_hw_auth_token_t));
    } else {
        LOG_E(LOG_TAG, "token is NULL");
    }
    cmd.config.timeoutSec = timeoutSec;
    err = sendTaCommand(&cmd, sizeof(cmd));
    if (cmd.result.result_info.result_type == FP_TA_ERR_ENROLL_TIMEOUT) {
        notifyError(FINGERPRINT_ERROR_TIMEOUT);
        LOG_E(LOG_TAG, "enroll timeout");
    }
    if (err >= FP_TA_ERR_INVALID_TOKEN_CHALLENGE && err <= FP_TA_ERR_UNTRUSTED_ENROLL) {
        LOG_E(LOG_TAG, "invalid token");
        notifyError(FINGERPRINT_ERROR_HW_UNAVAILABLE);
        //vts Enroll with an invalid (all zeroes) HAT should return success, but notify FINGERPRINT_ERROR_HW_UNAVAILABLE.
        err = FP_SUCCESS;
    }
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

void FingerprintFunction::checkEnrollError(fp_return_type_t err) {
    if (FP_SUCCESS == err) {
        mSerialEnrErrCount = 0;
        goto fp_out;
    }

    mSerialEnrErrCount++;
    if (mSerialEnrErrCount >= 10) {
        notifyError(FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
        LOG_E(LOG_TAG, "[%s] serial enroll error reach the max count:%d", __func__, mSerialEnrErrCount);
        mSerialEnrErrCount = 0;
        cancel();
    }
fp_out:
    LOG_I(LOG_TAG, "[%s] deal err:%d errcount:%d", __func__, (int)err, mSerialEnrErrCount);
    return;
}

fp_return_type_t FingerprintFunction::enrollDown(fp_enroll_type_t type) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    mFingerDownTime = getTimeUs();
    perfSetAction(0, 1000);
    perfSetUxThread(1);

    //send enroll down cmd
    fp_enroll_down_t cmd;
    memset(&cmd, 0, sizeof(fp_enroll_down_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_ENROLL_DOWN;

    //cmd.config.tp_info;
    //cmd.config.secreen_status = 0;
    //cmd.config.temperature_info = 0;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

#ifdef TEE_REE
    usleep(50 * 1000);
#else
    if (FP_AUTOSMOKING == type) {
        LOG_I(LOG_TAG, "[%s] auto smoking mode, skip wait ui event", __func__);
    } else {
        //wait uiready
        err = waitUiready(500);
        CHECK_RESULT_SUCCESS(err);
        // sleep 15ms for UI Ready , TBD: 1: msg thread? 2: funtion feature?
        usleep(15 * 1000);
        mUiReadyTime = getTimeUs();
    }
#endif
    //capture
    err = captureImg(FP_MODE_ENROLL, 0);
    CHECK_RESULT_SUCCESS(err);

    //getfeature
    err = getFeature(FP_MODE_ENROLL, 0);
    CHECK_RESULT_SUCCESS(err);

    //enroll
    memset(&enroll_result, 0, sizeof(fp_enroll_image_result_t));
    err = enrollImage(&enroll_result);
    CHECK_RESULT_SUCCESS(err);

    //check result
    if (enroll_result.remaining == 0) {
        setWorkMode(FP_MODE_NONE);
        mContext->mAutoSmoking->setEnrollFinish(1);
        err = enrollSaveTemplate();
        CHECK_RESULT_SUCCESS(err);
        //enumerate
        notifyEnrollResult(&enroll_result);
        err = enumerate();
        CHECK_RESULT_SUCCESS(err);
        //TBD : setting_app bug:
        err = mContext->mDevice->controlTp(DEVICE_DISABLE_TP);
        CHECK_RESULT_SUCCESS(err);
        //dump
        Dump::dumpProcess(FP_MODE_ENROLL, current_gid, enroll_result.finger_id, enroll_result.result_info.result_type);
        Dump::dumpRename(enroll_result.finger_id);
    } else {
        notifyEnrollResult(&enroll_result);
        //dump
        Dump::dumpProcess(FP_MODE_ENROLL, current_gid, enroll_result.finger_id, enroll_result.result_info.result_type);
    }

fp_out:
    checkEnrollError(err);

    if (err != FP_SUCCESS) {
        Dump::dumpProcess(FP_MODE_ENROLL, current_gid, enroll_result.finger_id, enroll_result.result_info.result_type);
    }

    perfSetUxThread(0);
    //set dcs
    mFunctionFinishTime = getTimeUs();
    if (enroll_result.remaining == 0) {
        mContext->mDcsInfo->sendDcsEnrollEventInfo(mContext);
    } else {
        mContext->mDcsInfo->sendDcsSingleEnrollEventInfo(mContext);
    }
    enrollFinish();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::enrollImage(fp_enroll_image_result_t* enroll_result) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    TIME_START(enrollImage);
    //send enroll image cmd
    fp_enroll_image_t cmd;
    memset(&cmd, 0, sizeof(fp_enroll_image_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_ENROLL_IMAGE;
    cmd.config.retry_index = 0;
    err = sendTaCommand(&cmd, sizeof(cmd));
    memcpy(enroll_result, &cmd.result, sizeof(fp_enroll_image_result_t));
    LOG_I(LOG_TAG, "[%s] cmd.result.remaining =%d, fid = %d", __func__, cmd.result.remaining, enroll_result->finger_id);
    CHECK_RESULT_SUCCESS(err);
    err = checkAcquiredInfo(cmd.result.result_info);

fp_out:
    TIME_END(enrollImage);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::enrollSaveTemplate() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    //save temlate cmd
    fp_save_template_t cmd;
    memset(&cmd, 0, sizeof(fp_save_template_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_SAVE_TEMPLATE;
    cmd.config.group_id = 0;
    cmd.config.finger_id = 0;//in result??
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::postEnroll() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    //close tp
    mContext->mDevice->controlTp(DEVICE_DISABLE_TP);
    //cmd
    fp_post_enroll_t cmd;
    memset(&cmd, 0, sizeof(fp_post_enroll_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_ENROLL_POST;
    err = sendTaCommand(&cmd, sizeof(cmd));
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::enrollFinish() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    fp_enroll_finish cmd;
    memset(&cmd, 0, sizeof(fp_enroll_finish));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_ENROLL_FINISH;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

//auth
uint64_t FingerprintFunction::getAuthenticatorId() {
    uint64_t authuthenticatorId = 0;
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    fp_get_authenticator_id_t cmd;
    memset(&cmd, 0, sizeof(fp_get_authenticator_id_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_GET_AUTHENTICATOR_ID;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);
    authuthenticatorId = cmd.result.authenticator_id;

fp_out:
    FUNC_EXIT(err);
    return authuthenticatorId;
}

fp_return_type_t FingerprintFunction::authenticateAsType(
    uint64_t operationId, uint32_t gid, uint32_t authtype) {
    UNUSED(authtype);
    LOG_I(LOG_TAG, "[%s] authtype =%d.", __func__, authtype);
    return authenticate(operationId, gid);
}

fp_return_type_t FingerprintFunction::authenticate(uint64_t operationId, uint32_t gid)
{
    LOG_I(LOG_TAG, "[%s] authenticate gid=%d. operationId = %lu", __func__, gid, operationId);
    FUNC_ENTER();
    fp_return_type_t err = FP_SUCCESS;
    current_gid = gid;
    //sendcmd
    fp_authenticate_t cmd;
    memset(&cmd, 0, sizeof(fp_authenticate_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_AUTHENTICATE;
    cmd.config.group_id = gid;
    cmd.config.operation_id = operationId;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

    err = mContext->mDevice->controlTp(DEVICE_ENABLE_TP);
    CHECK_RESULT_SUCCESS(err);
    err = mContext->mDevice->cleanTouchStatus();
    CHECK_RESULT_SUCCESS(err);
fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::authenticateStart() {
    fp_return_type_t err;
    fp_auth_down_t cmd;
    memset(&cmd, 0, sizeof(fp_auth_down_t));
    TIME_START(authenticateStart);
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_AUTHENTICATE_DOWN;
    //cmd.config.tp_info;
    cmd.config.screen_status = FP_SCREEN_ON;
    cmd.config.temperature_info = {0};
    err =  sendTaCommand(&cmd, sizeof(cmd));
    TIME_END(authenticateStart);
    return err;
}


fp_return_type_t FingerprintFunction::authenticateDown(fp_auth_type_t type) {
    LOG_I(LOG_TAG, "[%s] authenticateDown", __func__);
    fp_return_type_t err = FP_SUCCESS;
    int32_t retry_index = 0;
    FUNC_ENTER();
    mFingerDownTime = getTimeUs();
    perfSetAction(0, 1000);
    perfSetUxThread(1);
    memset(&compare_result, 0, sizeof(fp_auth_compare_result_t));
    compare_result.group_id = current_gid;
    //send auth cmd
    err = authenticateStart();
    CHECK_RESULT_SUCCESS(err);
#ifdef TEE_REE
    usleep(80 * 1000);
#else
    if (FP_AUTOSMOKING == type) {
        LOG_I(LOG_TAG, "[%s] auto smoking mode, skip wait ui event", __func__);
    } else {
        //wait uiready
        err = waitUiready(500);
        CHECK_RESULT_SUCCESS(err);//goto auth fail
        // sleep 15ms for UI Ready , TBD: msg thread??
        usleep(15 * 1000);
        mUiReadyTime = getTimeUs();
    }
#endif
    for (retry_index = 0; retry_index < MAX_RETRY_INDEX; retry_index++) {
        //capture
        err = captureImg(FP_MODE_AUTH, retry_index);
        if (err == FP_ERR_FINGERUP_TOO_FAST) {
            if (retry_index) {
                compare_result.result_info.result_type = DCS_AUTH_FAIL;
                notifyAuthResult(&compare_result);
            } else {
                compare_result.result_info.result_type = DCS_AUTH_TOO_FAST_GET_IMGINFO;
            }
        }
        CHECK_NEED_RETRY(err);
        CHECK_RESULT_SUCCESS(err);

        //getfeature
        err = getFeature(FP_MODE_AUTH, retry_index);
        CHECK_NEED_RETRY(err);
        CHECK_RESULT_SUCCESS(err);

        //compare
        err = authenticateCompare(&compare_result, retry_index);
        LOG_I(LOG_TAG, "ret:%d, qty:%d, area:%d, rawdata:%d, score:%d, retry:%d", compare_result.result_info.result_type,
            compare_result.quality, compare_result.area, compare_result.rawdata, compare_result.score, retry_index);
        // Auth success
        if ((compare_result.result_info.result_type == 0) && (compare_result.finger_id != 0)) {
            setWorkMode(FP_MODE_NONE);
            notifyAuthResult(&compare_result);
            perfSetUxThread(0);
            //study
            if (compare_result.study_flag == E_NEED_STUDEY_TEMPLATE) {
                err = authenticateStudy(compare_result.finger_id);
                CHECK_RESULT_SUCCESS(err);
            }
            mContext->mDevice->controlTp(DEVICE_DISABLE_TP);
            Dump::dumpProcess(FP_MODE_AUTH, current_gid, compare_result.finger_id, compare_result.result_info.result_type);
            goto fp_out;
        }
        // Auth fail
        LOG_I(LOG_TAG, "[%s] result_type =%d, result_value =%d ", __func__,
                compare_result.result_info.result_type, compare_result.result_info.result_value);
        if ((compare_result.result_info.result_type == FP_SDK_ERR_COMPARE_FAIL_AND_NOT_RETRY)
            || ((retry_index == MAX_RETRY_INDEX - 1))) {
            notifyAuthResult(&compare_result);
            Dump::dumpProcess(FP_MODE_AUTH, current_gid, compare_result.finger_id, compare_result.result_info.result_type);
            goto fp_out;
        }
        Dump::dumpProcess(FP_MODE_AUTH, current_gid, compare_result.finger_id, compare_result.result_info.result_type);
    }

fp_out:
    perfSetUxThread(0);
    //dcs
    mFunctionFinishTime = getTimeUs();
    mContext->mDcsInfo->auth_event_info.auth_type = type;
    mContext->mDcsInfo->auth_event_info.retry_times = retry_index;
    mContext->mDcsInfo->sendDcsAuthEventInfo(mContext);
    authenticateFinish();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::captureImg(fp_mode_t mode, uint32_t retry_index) {
    fp_return_type_t err = FP_SUCCESS;
    TIME_START(captureImg);
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] mode=%d, retry_index=%d", __func__, mode, retry_index);

    //check finger
    err = checkFingerUpTooFast(retry_index);
    CHECK_RESULT_SUCCESS(err);

    fp_capture_image_t cmd;
    memset(&cmd, 0, sizeof(fp_capture_image_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_CAPTURE_IMG;
    cmd.config.retry_index = retry_index;
    cmd.config.capture_mode = mode;
    cmd.config.screen_status = FP_SCREEN_ON;
    cmd.config.temperature_info = {0};
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

    err = (fp_return_type_t)cmd.result.result_info.result_type;
    LOG_D(LOG_TAG, "[%s] result_type=%d, retry_value=%d", __func__, err, cmd.result.result_info.result_value);
    switch (err) {
        case FP_SDK_ERR_NO_NEED_TO_CAPTURE:
            err = FP_SUCCESS;
            break;
        case FP_SDK_ERR_NEED_RETRY_CAPTURE:
            err = sendTaCommand(&cmd, sizeof(cmd));
            CHECK_RESULT_SUCCESS(err);
            err = checkFingerUpTooFast(retry_index);
            break;
        case FP_SUCCESS:
            err = checkFingerUpTooFast(retry_index);
            break;
        //other fail type : FP_SDK_ERR_COMMON_FAILED
        default:
            err = checkFingerUpTooFast(retry_index);
            if (err == FP_SUCCESS) {//not too fast
                checkAcquiredInfo(cmd.result.result_info);
                err = FP_HAL_AUTH_NEED_RETRY;
                LOG_I(LOG_TAG, "[%s] FP_HAL_AUTH_NEED_RETRY", __func__);
            }
            break;
    }

    CHECK_RESULT_SUCCESS(err);
    //notify acquired
    notifyAcquiredInfo(FINGERPRINT_ACQUIRED_GOOD);//TBD:check retry action??

fp_out:
    TIME_END(captureImg);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::getFeature(fp_mode_t mode, uint32_t retry_index) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    TIME_START(getFeature);
    fp_get_feature_t cmd;
    memset(&cmd, 0, sizeof(fp_get_feature_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_GET_FEATURE;
    cmd.config.retry_index = retry_index;//TBD
    cmd.config.capture_mode = mode;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);
    err = checkAcquiredInfo(cmd.result.result_info);

    err = (fp_return_type_t)cmd.result.result_info.result_type;
    if (err != FP_SUCCESS) {
        err = FP_HAL_AUTH_NEED_RETRY;
    }

fp_out:
    TIME_END(getFeature);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::authenticateCompare(fp_auth_compare_result_t* compare_result, uint32_t retry_index) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    TIME_START(authenticateCompare);
    fp_auth_compare_t cmd;
    memset(&cmd, 0, sizeof(fp_auth_compare_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_AUTHENTICATE_COMPARE;
    cmd.config.retry_index = retry_index;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);
    memcpy(compare_result, &cmd.result, sizeof(fp_auth_compare_result_t));
    LOG_I(LOG_TAG, "[%s] cmd.result.finger_id =%u, cmd.result.result_info.result =%d", __func__, cmd.result.finger_id, cmd.result.result_info.result_type);
    err = checkAcquiredInfo(cmd.result.result_info);

fp_out:
    TIME_END(authenticateCompare);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::authenticateStudy(uint32_t finger_id) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] finger_id =%d", __func__, finger_id);

    fp_auth_study_t cmd;
    memset(&cmd, 0, sizeof(fp_auth_study_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_AUTHENTICATE_STUDY;
    cmd.config.finger_id = finger_id;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::authenticateFinish() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    fp_auth_finish_t cmd;
    memset(&cmd, 0, sizeof(fp_auth_finish_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_AUTHENTICATE_FINISH;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::removeSingleFinger(uint32_t gid, uint32_t fid, uint32_t remainCount) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u, remainCount=%d", __func__, gid, fid, remainCount);
    fp_remove_t cmd;
    memset(&cmd, 0, sizeof(fp_remove_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_REMOVE;
    cmd.config.group_id = gid;
    cmd.config.finger_id = fid;
    err = sendTaCommand(&cmd, sizeof(cmd));
    if (FP_SUCCESS != err) {
        LOG_E(LOG_TAG, "remove err = %d", err);
        //vts remove test case shold return FP_SUCCESS.
        err = FP_SUCCESS;
    }
    err = notifyRemove(gid, fid, remainCount);
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::remove(uint32_t gid, uint32_t fid) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, gid, fid);
    if (fid != 0) {
        removeSingleFinger(gid, fid, 0);
    } else {
        fp_enumerate_t cmd;
        memset(&cmd, 0, sizeof(fp_enumerate_t));
        cmd.header.module_id = FP_MODULE_FPCORE;
        cmd.header.cmd_id = FP_CMD_FPCORE_ENUMERATE;
        err = sendTaCommand(&cmd, sizeof(cmd));
        if (FP_SUCCESS != err) {
            LOG_E(LOG_TAG, "remove err = %d", err);
            //vts remove test case shold return FP_SUCCESS.
            err = FP_SUCCESS;
        }
        if (cmd.result.finger_count == 0) {
            LOG_D(LOG_TAG, "[%s] finger_count = 0", __func__);
            err = notifyRemove(gid, fid, 0);
            goto fp_out;
        }

        for (int count = 0; count < cmd.result.finger_count; count++) {
            removeSingleFinger(gid, cmd.result.finger_id[count], cmd.result.finger_count - count -1);
        }
    }
fp_out:
    //notify
    enumerate();
    FUNC_EXIT(err);
    return err;
}


//common
fp_return_type_t FingerprintFunction::screenState(uint32_t state) {
    fp_return_type_t err = FP_SUCCESS;
    UNUSED(state);
    mScreenStatus = state;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::cancel() {
    fp_return_type_t err = FP_SUCCESS;
    notifyError(FINGERPRINT_ERROR_CANCELED);
    FUNC_ENTER();
    //TBD: cancel enrollDown or authDown
    fp_cancel_t cmd;
    memset(&cmd, 0, sizeof(fp_cancel_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_CANCEL;
    err = sendTaCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);
    mContext->mDevice->controlTp(DEVICE_DISABLE_TP);
    //TBD: set sensor sleep??
fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::touchdown() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    // FINGERPRINT_FACTORY_QTY_TEST
    sendFingerprintCmd(FINGERPRINT_FACTORY_QTY_TEST, NULL, 0);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::touchup() {
    fp_return_type_t err = FP_SUCCESS;

    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::sendAuthDcsmsg() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    FUNC_EXIT(err);
    return err;
}

void FingerprintFunction::setWorkMode(fp_mode_t mode) {
    Mutex::Autolock _l(mWorkModeMutex);
    LOG_D(LOG_TAG, "%s, current mode: %d set mode:%d", __func__, mWorkMode, mode);
    switch (mWorkMode) {
    case FP_MODE_NONE:
        mWorkMode = mode;
        break;
    case FP_MODE_ENROLL:
        switch (mode) {
        case FP_MODE_NONE:
        case FP_MODE_CANCEL:
            mWorkMode = mode;
            break;
        default:
            break;
        }
        break;
    case FP_MODE_AUTH:
        switch (mode) {
        case FP_MODE_NONE:
        case FP_MODE_CANCEL:
            mWorkMode = mode;
            break;
        default:
            break;
        }
        break;
    case FP_MODE_CANCEL:
        mWorkMode = mode;
        break;
    default:
        break;
    }
    LOG_D(LOG_TAG, "%s, after set mode: %d", __func__, mWorkMode);
}

fp_mode_t FingerprintFunction::getWorkMode() {
    Mutex::Autolock _l(mWorkModeMutex);
    return mWorkMode;
}

fp_return_type_t FingerprintFunction::doTouchDownEvent(fp_auth_type_t type) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "%s doTouchDownEvent,  mode: %d", __func__, getWorkMode());
    if (getWorkMode() == FP_MODE_AUTH) {
        err = authenticateDown(type);
    } else if (getWorkMode()  == FP_MODE_ENROLL) {
        err = enrollDown(type);
    } else {
        LOG_I(LOG_TAG, "[%s] not woking mode, %d", __func__, getWorkMode());
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::setFingerstatus(finger_status_type_t status) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    mFingerStatus = status;
    LOG_D(LOG_TAG, "setFingerstatus, status: %d", status);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::checkFingerUpTooFast(uint32_t retry_index) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    if (mFingerStatus == FP_FINGER_UP) {
        LOG_D(LOG_TAG, "%s, FP_FINGER_UP", __func__);
        err = FP_ERR_FINGERUP_TOO_FAST;
        if (retry_index == 0)  {
            notifyAcquiredInfo(FINGERPRINT_ACQUIRED_TOO_FAST);
        }
    }

    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::sendFingerprintCmd(int32_t cmd_id, int8_t *in_buf, uint32_t size) {
    fingerprint_cmd_id_t cmd = (fingerprint_cmd_id_t)cmd_id;
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "sendFingerprintCmd = %d", cmd_id);
    switch (cmd) {
    case FINGERPRINT_FACTORY_QTY_TEST:
        HalContext::getInstance()->mFacotryTest->setHbmMode(1);
        usleep(100);
        err = HalContext::getInstance()->mFacotryTest->sendFingerprintCmd(cmd_id, in_buf, size);
        HalContext::getInstance()->mFacotryTest->setHbmMode(0);
        break;
    case FINGERPRINT_FACTORY_AUTO_TEST:
    case FINGERPRINT_FACTORY_GET_ALGO_INFO:
    case FINGERPRINT_FACTORY_GET_SENSOR_QRCODE:
    case FINGERPRINT_FACTORY_AGING_TEST:
    case FINGERPRINT_FACTORY_CALI_FLESH_BOX_TEST:
    case FINGERPRINT_FACTORY_CALI_BLACK_BOX_TEST:
    case FINGERPRINT_FACTORY_CALI_CHART_TEST:
    case FINGERPRINT_CAPTURE_TOOL_GET_IMG:
        // spi test
        // err = HalContext::getInstance()->mFacotryTest->sendFingerprintCmd(FINGERPRINT_FACTORY_AUTO_TEST, in_buf, size);
        err = HalContext::getInstance()->mFacotryTest->sendFingerprintCmd(cmd_id, in_buf, size);
        break;
    case FINGERPRINT_CMD_GET_CALIBRATION_FLOW:
        LOG_E(LOG_TAG, "FINGERPRINT_CMD_GET_CALIBRATION_FLOW, string: %s", HalContext::getInstance()->mConfig->mTaConfig->mCaliflow);
        notifyFingerprintCmd(0, cmd_id, (const int8_t *)HalContext::getInstance()->mConfig->mTaConfig->mCaliflow,
            strlen(HalContext::getInstance()->mConfig->mTaConfig->mCaliflow));
        break;
    case FINGERPRINT_SMOKING_SUPPORT:
    case FINGERPRINT_SMOKING_START:
    case FINGERPRINT_SMOKING_AUTH_SUCCESS:
    case FINGERPRINT_SMOKING_AUTH_FAIL:
    case FINGERPRINT_SMOKING_ENROLL_DEPLICATE:
    case FINGERPRINT_SMOKING_AUTH_SUCCESS_MUL:
    case FINGERPRINT_SMOKING_AUTH_FAIL_MUL:
    case FINGERPRINT_SMOKING_AUTH_SUCCESS_AND_FAIL:
    case FINGERPRINT_SMOKING_ENROLL_UPPER_LIMIT:
    case FINGERPRINT_SMOKING_ENROLL_CANCEL:
    case FINGERPRINT_SMOKING_ENROLL_REPEAT:
    case FINGERPRINT_SMOKING_ENROLL_LOW_QUALITY:
    case FINGERPRINT_SMOKING_ENROLL_SAMLL_AREA:
    case FINGERPRINT_SMOKING_ENROLL_TIMEOUT:
    case FINGERPRINT_SMOKING_AUTH_LOW_QUALITY:
    case FINGERPRINT_SMOKING_ENROLL_MANAY_ERR:
    case FINGERPRINT_SMOKING_END:
        err = HalContext::getInstance()->mAutoSmoking->autoSmokingCase(cmd_id, 0);
        break;
    default:
        break;
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintFunction::sendTaCommand(void *cmd, uint32_t size) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    err = mContext->mCaEntry->sendCommand(cmd, size);
    FUNC_EXIT(err);
    return err;
}




}  // namespace android
