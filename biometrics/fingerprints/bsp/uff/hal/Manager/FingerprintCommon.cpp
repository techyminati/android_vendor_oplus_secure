/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Manager/FingerprintCommon.cpp
 **
 ** Description:
 **      FingerprintCommon for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][FingerprintCommon]"

#include <cutils/properties.h>
#include <endian.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "HalContext.h"
#include "HalLog.h"
#include "FpCommon.h"
#include "FpType.h"
#include "Utils.h"
#include "FingerprintCommon.h"

namespace android {
FingerprintCommon::FingerprintCommon(HalContext *context):
    mContext(context) {
    LOG_I(LOG_TAG, "enter to FingerprintCommon");
    sem_init(&mUireadySem, 0, 0);
    UNUSED(context);
}

FingerprintCommon::~FingerprintCommon() {
    LOG_I(LOG_TAG, "enter to ~FingerprintCommon");
}

void FingerprintCommon::print_token(fp_hw_auth_token_t* token) {
    FUNC_ENTER();
    (void)token;
    LOG_V(LOG_TAG, "print_token token->version = %d", token->version);
    LOG_V(LOG_TAG, "print_token token->challenge = %llu", token->challenge);
    LOG_V(LOG_TAG, "print_token token->user_id = %llu", token->user_id);
    LOG_V(LOG_TAG, "print_token token->authenticator_id = %llu", token->authenticator_id);
    LOG_V(LOG_TAG, "print_token token->authenticator_type = %u", token->authenticator_type);
    LOG_V(LOG_TAG, "print_token token->timestamp = %llu", token->timestamp);
    LOG_V(LOG_TAG, "print_token token hmackey_length :%lu", sizeof(token->timestamp));
}

fp_return_type_t FingerprintCommon::waitUiready(uint32_t ms) {
    int32_t err = FP_SUCCESS;
    TIME_START(waitUiready);
    struct timespec t;
    struct timeval time;
    int32_t semvalue = -1;
    gettimeofday(&time, NULL);
    LOG_D(LOG_TAG, "[%s]Wait %u ms", __func__, ms);
    time.tv_usec += ms * 1000;
    if (time.tv_usec >= 1000000) {
        time.tv_sec += time.tv_usec / 1000000;
        time.tv_usec %= 1000000;
    }
    t.tv_sec = time.tv_sec;
    t.tv_nsec = time.tv_usec * 1000;
    sem_getvalue(&mUireadySem, &semvalue);
    while (semvalue > 1) {
        LOG_D(LOG_TAG, "[%s] while sem value(%d).", __func__, semvalue);
        sem_timedwait(&mUireadySem, &t);
        sem_getvalue(&mUireadySem, &semvalue);
    }
    err = sem_timedwait(&mUireadySem, &t);
    LOG_D(LOG_TAG, "[%s] exit, ret = %d.", __func__, err);
    TIME_END(waitUiready);
    return (fp_return_type_t)err;
}

fp_return_type_t FingerprintCommon::postUiready() {
    int32_t err = FP_SUCCESS;
    int32_t semvalue = -1;
    sem_getvalue(&mUireadySem, &semvalue);
    if (semvalue <= 0) {
        err = sem_post(&mUireadySem);
    }
    sem_getvalue(&mUireadySem, &semvalue);
    LOG_I(LOG_TAG, "[%s]sem value = %d", __func__, semvalue);
    return (fp_return_type_t)err;
}

fp_return_type_t FingerprintCommon::perfSetAction(int orms_type, int orms_timeout) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] perfSetAction orms_timeout= %d", __func__, orms_timeout);
    perf_action_t action;
    memset(&action, 0, sizeof(perf_action_t));
    action.event = PERF_SET_ACTION;
    action.orms_type = orms_type;
    action.orms_timeout = orms_timeout;
    mContext->mPerf->improvePerf(action);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintCommon::perfSetUxThread(int ux_enable) {
    fp_return_type_t err = FP_SUCCESS;
    static int last_ux_status = 0;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] perfSetUxThread ux_enable =%d", __func__, ux_enable);
    perf_action_t action;
    memset(&action, 0, sizeof(perf_action_t));
    if (last_ux_status != ux_enable) {
        action.event = PERF_SET_UXTHREAD;
        action.bind_pid = getpid();
        action.bind_tid = gettid();
        action.ux_enable = ux_enable;
        mContext->mPerf->improvePerf(action);
        last_ux_status = ux_enable;
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintCommon::perfBindBigCore() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] perfBindBigCore", __func__);
    perf_action_t action;
    memset(&action, 0, sizeof(perf_action_t));
    action.event = PERF_BIND_BIGCORE_BY_PID;
    mContext->mPerf->improvePerf(action);
    FUNC_EXIT(err);
    return err;
}
}  // namespace android
