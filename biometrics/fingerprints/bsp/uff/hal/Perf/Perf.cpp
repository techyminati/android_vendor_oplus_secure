/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Perf.cpp
 **
 ** Description:
 **      Perf for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][Perf]"

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdio.h>
#include "Perf.h"
#include "FpCommon.h"
#include "HalLog.h"
#include "FpType.h"
#include <sys/types.h>
#include <unistd.h>

#ifdef FP_HYPNUSD_ENABLE
#include "OrmsHalClient.h"
#endif

#define MAX_CORE_NUMBER 8
// TBD: {4, 5, 6, 7, -1}
int big_core[MAX_CORE_NUMBER] = {6, 7, -1};  // -1 represent the end


namespace android {
// --- Perf ---
Perf *Perf::sInstance = NULL;

Perf::Perf() {
    LOG_D(LOG_TAG, "Perf create");
    mPerfHandler = new Handler("Perf", this);
}

Perf::~Perf() {}

void Perf::setAction(int action, int timeout) {
#ifdef FP_HYPNUSD_ENABLE
    int ret = -1;
    UNUSED(action);
    LOG_I(LOG_TAG, "fingerprint try to acquire orms hal client !!!");
    if (ormsHalClient != NULL || (ormsHalClient = OrmsHalClient::getInstance(identity)) != NULL) {
        mSa.scene   = "";
        mSa.action  = "ORMS_ACTION_UNLOCK_BOOST";
        mSa.timeout = timeout;
        ret         = ormsHalClient->ormsSetSceneAction(mSa);
    }
    if (ret == -1) {
        LOG_E(LOG_TAG, "fingerprint orms setAction fail");
    }
#else
    UNUSED(action);
    UNUSED(timeout);
#endif
}

void Perf::bind_big_core(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < MAX_CORE_NUMBER; i++) {
        if (big_core[i] >= 0) {
            CPU_SET(big_core[i], &mask);
        } else {
            break;
        }
    }
    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), (cpu_set_t *)&mask) < 0) {
        LOG_E(LOG_TAG, "bind big core for fingerprint pid = %d fail", getpid());
    } else {
        LOG_I(LOG_TAG, "bind big core for fingerprint pid = %d success", getpid());
    }
}

void Perf::bind_big_core_bytid(int tid) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < MAX_CORE_NUMBER; i++) {
        if (big_core[i] >= 0) {
            CPU_SET(big_core[i], &mask);
        } else {
            break;
        }
    }
    if (sched_setaffinity(tid, sizeof(cpu_set_t), (cpu_set_t *)&mask) < 0) {
        LOG_E(LOG_TAG, "bind_big_core_bytid for fingerprint tid:%d fail", tid);
    } else {
        LOG_I(LOG_TAG, "bind_big_core_bytid for fingerprint tid:%d success", tid);
    }
}

/**************************************************************************************
 **feature 11.1: echo 1 > /proc/pid/task/tid/static_ux
 **feature 11.2+: echo 136 > /proc/pid/task/tid/ux_state
 ************************************************************************************/
void Perf::setUxthread(int pid, int tid, int enable) {
    char path[PATH_LEN]         = {0};
    char enable_buffer[MAX_IO_BUFFER_LENGTH] = {0};
    VOID_FUNC_ENTER();
    snprintf(path, sizeof(path), "%s%d%s%d%s", "/proc/", pid, "/task/", tid, "/static_ux");
    //LOG_D(LOG_TAG, "[%s] path: %s , enable :%d", __func__, path, enable);
    LOG_D(LOG_TAG, "[%s] pid %d, tid %d, enable :%d", __func__, pid, tid, enable);
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "setUxthread open err fd  :%d", fd);
        goto fp_out;
    }
    snprintf(enable_buffer, sizeof(enable_buffer), "%d", enable);
    if (write(fd, enable_buffer, sizeof(enable)) != sizeof(enable)) {
        LOG_E(LOG_TAG, "[%s] write file fail errno %d", __func__, errno);
    } else {
        LOG_I(LOG_TAG, "[%s] success %d", __func__, tid);
    }
    close(fd);
fp_out:
    VOID_FUNC_EXIT();
}

void Perf::handleMessage(FpMessage msg) {
    LOG_I(LOG_TAG, "[%s] what %d", __func__, msg.what);
    switch (msg.what) {
        case PERF_SET_ACTION:
            setAction(msg.arg0, msg.arg1);
            break;
        case PERF_BIND_BIGCORE_BY_PID:
            bind_big_core();
            break;
        case PERF_BIND_BIGCORE_BY_TID:
            bind_big_core_bytid(msg.arg0);
            break;
        case PERF_SET_UXTHREAD:
            setUxthread(msg.arg0, msg.arg1, msg.arg2);
            break;
        default:
            break;
    }
}

void Perf::improvePerf(perf_action_t action) {
    LOG_I(LOG_TAG, "[%s] event: %d", __func__, action.event);
    FpMessage   msg;
    switch (action.event) {
        case PERF_SET_ACTION:
            msg.what = action.event;
            msg.arg0 = action.orms_type;
            msg.arg1 = action.orms_timeout;
            mPerfHandler->sendMessage(msg);
            break;
        case PERF_BIND_BIGCORE_BY_PID:
            msg.what = action.event;
            mPerfHandler->sendMessage(msg);
            break;
        case PERF_BIND_BIGCORE_BY_TID:
            msg.what = action.event;
            msg.arg0 = action.bind_tid;
            mPerfHandler->sendMessage(msg);
            break;
        case PERF_SET_UXTHREAD:
            msg.what = action.event;
            msg.arg0 = action.bind_pid;
            msg.arg1 = action.bind_tid;
            msg.arg2 = action.ux_enable;
            mPerfHandler->sendMessage(msg);
            break;
        default:
            LOG_E(LOG_TAG, "[%s]unknown event: %d", __func__, action.event);
            break;
    }
}

};  // namespace android
