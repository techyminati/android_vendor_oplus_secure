/**************************************************************************************
 ** File: - Perf.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2019-08-21
 ** Author: Ziqing.Guo@BSP.Fingerprint.Basic, Add for fingerprint performance module
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ziqing.Guo       2019/08/21       create file
 **  Ziqing.Guo       2019/08/22       add for compatible different platforms
 ************************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sched.h>
#include "Perf.h"
#include <errno.h>

#ifdef FP_HYPNUSD_ENABLE
//#include "HypnusUtilClient.h"
#include "OrmsHalClient.h"
#endif

#ifdef FP_OPLUS_PLATFORMCPU_ALL8_BIG67
int big_core[MAX_CORE_NUMBER] = {6, 7, -1};  // -1 represent the end
#elif defined FP_OPLUS_PLATFORMCPU_ALL8_BIG4567
int big_core[MAX_CORE_NUMBER] = {4, 5, 6, 7, -1};
#else
int big_core[MAX_CORE_NUMBER] = {-1};
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define FINGERPRINT_UX_NUMBER (128+2)

namespace android {

    // --- Hypnus ---
    Hypnus* Hypnus::sInstance = NULL;

    Hypnus::Hypnus() {
        ALOGD("Hypnus create");
    }

    Hypnus::~Hypnus() {
    }

    void Hypnus::setAction(int action, int timeout) {
#ifdef FP_HYPNUSD_ENABLE
        int ret = -1;
        UNUSED(action);
        ALOGI("fingerprint try to acquire orms hal client !!!");
        if (ormsHalClient != NULL || (ormsHalClient = OrmsHalClient::getInstance(identity)) != NULL) {
            mSa.scene = "";
            mSa.action = "ORMS_ACTION_UNLOCK_BOOST";
            mSa.timeout = timeout;
            ret = ormsHalClient->ormsSetSceneAction(mSa);
        }
        if (ret == -1) {
            ALOGE("fingerprint orms setAction fail");
        }
        return;
/*     Keep this way to prevent switching back and forth
        int ret = -1;//not set hypnus action, ret = 0 means set successed
        HypnusUtilClient* fp_hypnusUtil = new HypnusUtilClient();

        if (NULL == fp_hypnusUtil) {
            ALOGE("Can't open HypnusUtilClient");
            return;
        }

        ALOGE(" new hypnus set action");
        ret = fp_hypnusUtil->setAction(action, timeout);
        if (ret != 0) {
            ALOGE("setAction fail");
        }
        if (NULL != fp_hypnusUtil) {
            delete fp_hypnusUtil;
            fp_hypnusUtil = NULL;
        }
        return;
*/
#else
        char buf[MAX_LEN];
        int fd = open(ACTION_INFO, O_WRONLY);
        if (fd < 0) {
            ALOGE("SetAction open err :%d", fd);
            return;
        }
        sprintf(buf, "%d %d", action, timeout);
        write(fd, buf, MAX_LEN);
        close(fd);
        return;
#endif
    }

    void Hypnus::bind_big_core(void) {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        for (int i = 0; i < MAX_CORE_NUMBER; i++) {
            if (big_core[i] >= 0) {
                CPU_SET(big_core[i], &mask);
            } else {
                break;
            }
        }
        sched_setaffinity(getpid(), sizeof(cpu_set_t), (cpu_set_t *)&mask);
        ALOGI("bind big core for fingerprint_hidl");
    }

    void Hypnus::bind_big_core_bytid(int tid) {
#ifdef FP_BINDCORE_BYTID
        cpu_set_t mask;
        CPU_ZERO(&mask);
        for (int i = 0; i < MAX_CORE_NUMBER; i++) {
            if (big_core[i] >= 0) {
                CPU_SET(big_core[i], &mask);
            } else {
                break;
            }
        }
        sched_setaffinity(tid, sizeof(cpu_set_t), (cpu_set_t *)&mask);
        ALOGI("bind_big_core_bytid for fingerprint_hidl");
#else
        ALOGE("bind_big_core_bytid does not enable");
#endif
    }

    void Hypnus::fp_tee_bind_core(uint8_t enable) {
        char enable_buffer[MAX_LEN] = {0};
        int set_value = 0;
        char *path = "/proc/tee_bind_core";
        int fd = open(path, O_WRONLY);
        if (fd < 0) {
            ALOGI("fp_tee_bind_core open err fd :%d", fd);
            return;
        }

        set_value = enable ? 1 : 0;
        snprintf(enable_buffer, sizeof(enable_buffer), "%d", set_value);

        if (write(fd, enable_buffer, sizeof(enable_buffer)) != sizeof(enable_buffer)) {
            ALOGI("[%s] write file fail errno %d", __func__, errno);
        } else {
            ALOGI("[%s] success", __func__);
        }
        close(fd);
    }

    /**************************************************************************************
     **feature 11.1: echo 1 > /proc/pid/task/tid/static_ux
     **feature 11.2+: echo 136 > /proc/pid/task/tid/ux_state
     ************************************************************************************/
    void Hypnus::setUxthread(int pid, int tid, uint8_t enable) {
        char path[PATH_LEN] = {0};
        char enable_buffer[MAX_LEN] = {0};
        int set_value = 0;
        int write_len = 0;
        fp_tee_bind_core(enable);
        snprintf(path, sizeof(path), "%s%d%s%d%s", "/proc/", pid, "/task/" , tid, "/ux_state");
        //ALOGI("[%s] path: %s , enable :%d", __func__, path, enable);
        ALOGI("[%s] pid %d, tid %d, enable :%d", __func__, pid, tid, enable);
        int fd = open(path, O_WRONLY);
        if (fd < 0) {
            ALOGI("setUxthread open err fd :%d", fd);
            return;
        }

        set_value = enable ? FINGERPRINT_UX_NUMBER : 0;
        write_len = snprintf(enable_buffer, sizeof(enable_buffer), "%d", set_value);

        if (write(fd, enable_buffer, write_len) != write_len) {
            ALOGI("[%s] write file fail errno %d", __func__, errno);
        } else {
            ALOGI("[%s] success %d", __func__, tid);
        }
        close(fd);
    }

}; // namespace android
