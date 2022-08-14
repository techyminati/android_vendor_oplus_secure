/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/FingerprintManager.cpp
 **
 ** Description:
 **      FingerprintManager for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][FingerprintNofity]"

#include "FingerprintManager.h"
#include <cutils/properties.h>
#include <endian.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "FingerprintNotify.h"
#include "fingerprint.h"
#include "FpType.h"


namespace android {
FingerprintNotify::FingerprintNotify():
    mNotify(nullptr) {
    LOG_I(LOG_TAG, "enter to FingerprintNotify");
}

FingerprintNotify::~FingerprintNotify() {
    LOG_I(LOG_TAG, "enter to ~FingerprintNotify");
}

fp_return_type_t FingerprintNotify::setNotify(fingerprint_notify_t Notify) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(Notify);
    mNotify = Notify;
fp_out:
    FUNC_EXIT(err);
    return err;
}

// notify
fp_return_type_t FingerprintNotify::notifyAcquiredInfo(fingerprint_acquired_info_t info) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = FINGERPRINT_ACQUIRED;
    message.data.acquired.acquired_info = (fingerprint_acquired_info_t) info;
    mNotify(&message);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintNotify::notifyEnrollResult(fp_enroll_image_result_t* enroll_result) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = FINGERPRINT_TEMPLATE_ENROLLING;
    message.data.enroll.finger.gid = enroll_result->group_id;
    message.data.enroll.finger.fid = enroll_result->finger_id;
    message.data.enroll.samples_remaining = enroll_result->remaining;
    mNotify(&message);
    LOG_I(LOG_TAG, "[%s] gid=%u, fid=%u, remains=%d", __func__,
                            message.data.enroll.finger.gid, message.data.enroll.finger.fid, message.data.enroll.samples_remaining);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintNotify::notifyAuthResult(fp_auth_compare_result* compare_result) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);

    //notify message
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = FINGERPRINT_AUTHENTICATED;
    message.data.authenticated.finger.gid = compare_result->group_id;
    if (compare_result->result_info.result_type == 0) {
        message.data.authenticated.finger.fid = compare_result->finger_id;
        memcpy(&message.data.authenticated.hat, &(compare_result->token), sizeof(fp_hw_auth_token_t));
    } else {
        message.data.authenticated.finger.fid = 0;
    }
    mNotify(&message);
    LOG_I(LOG_TAG, "[%s] gid=%u, fid=%u ", __func__,
                            message.data.authenticated.finger.gid, message.data.authenticated.finger.fid);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintNotify::notifyEnumerate(fp_enumerate_t cmd, uint32_t current_gid) {
    fp_return_type_t err = FP_SUCCESS;
    uint32_t i = 0;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = FINGERPRINT_TEMPLATE_ENUMERATING;
    message.data.enumerated.gid   = current_gid;
    message.data.enumerated.remaining_templates  = cmd.result.finger_count;
    for (i = 0; i < cmd.result.finger_count; i++) {
        message.data.enumerated.fingers[i].gid = current_gid;
        message.data.enumerated.fingers[i].fid = cmd.result.finger_id[i];
        LOG_D(LOG_TAG, "[%s] group_id[%d]=%u, finger_id[%d]=%u, remains=%u", __func__,
                            i, current_gid, i, cmd.result.finger_id[i], cmd.result.finger_count);
    }
    mNotify(&message);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintNotify::notifyRemove(
    uint32_t gid, uint32_t fid, uint32_t remainCount) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);
    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u, remainingTemplates=%d", __func__, gid, fid, remainCount);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = FINGERPRINT_TEMPLATE_REMOVED;
    message.data.removed.finger.fid = fid;
    message.data.removed.finger.gid = gid;
    message.data.removed.remaining_templates = remainCount;
    mNotify(&message);

fp_out:
    FUNC_EXIT(err);
    return err;
}

//hidl api for debug
fp_return_type_t FingerprintNotify::notifyTouch(fingerprint_touch_state_type_t touch_state) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    if (touch_state == FINGERPRINT_TOUCH_DOWN_STATUE) {
        message.type = FINGERPRINT_TOUCH_DOWN;
    } else if (touch_state == FINGERPRINT_TOUCH_UP_STATUE) {
        message.type = FINGERPRINT_TOUCH_UP;
    }
    mNotify(&message);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintNotify::notifyError(fingerprint_error_t error)
{
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = FINGERPRINT_ERROR;
    message.data.error = error;
    LOG_E(LOG_TAG, "[%s] err code : %d.", __func__, message.data.error);
    mNotify(&message);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintNotify::notifyFingerprintCmd(
    int64_t devId, int32_t cmdId, const int8_t *buffer, int32_t len) {

    UNUSED(devId);
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    CHECK_RESULT_NULLPTR(mNotify);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));
    message.type = FINGERPRINT_OPTICAL_SENDCMD;
    message.data.test.cmd_id = cmdId;
    message.data.test.result = (int8_t *) buffer;
    message.data.test.result_len = len;

    mNotify(&message);

fp_out:
    FUNC_EXIT(err);
    return err;
}

// notify
fp_return_type_t FingerprintNotify::checkAcquiredInfo(fp_common_result_t result_info) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    err = (fp_return_type_t)result_info.result_type;
    LOG_D(LOG_TAG, "[%s] result_type=%d, result_value=%d", __func__, err, result_info.result_value);
    switch (err) {
        case FP_SDK_ERR_ACQUIRED_PARTIAL:
            notifyAcquiredInfo(FINGERPRINT_ACQUIRED_PARTIAL);
            break;
        case FP_SDK_ERR_ACQUIRED_IMAGER_DIRTY:
            notifyAcquiredInfo(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
            break;
        case FP_SDK_ERR_ENROLL_TOO_SIMILAR:
            notifyAcquiredInfo(FINGERPRINT_ACQUIRED_TOO_SIMILAR);
            break;
        case FP_SDK_ERR_ENORLL_ALREADY_ENROLLED:
            notifyAcquiredInfo(FINGERPRINT_ACQUIRED_ALREADY_ENROLLED);
            break;
        case FP_SDK_ERR_ENORLL_NEED_CANCEL:
            notifyError(FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
            break;
        default:
            break;
    }
    FUNC_EXIT(err);
    return err;
}


}  // namespace android
