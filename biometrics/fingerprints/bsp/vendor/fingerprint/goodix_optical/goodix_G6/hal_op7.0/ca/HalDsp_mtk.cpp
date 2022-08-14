/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *Version:
 *Description:
 *History:
 */

#include <fcntl.h>
#include <unistd.h>
#include <uree/system.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "CaEntry.h"
#include "Vpu_drv.h"
#include "HalDsp.h"
#include "HalLog.h"
#include "gf_base_types.h"
#define LOG_TAG "[GF_HAL][HalDsp]"

#ifndef UNUSED_VAR
#define UNUSED_VAR(X)   ((void)(X))
#endif  // UNUSED_VAR

#define VPU0_BUF_OFFSET        0x3ff000
#define VPU0_PROP_OFFSET       0x3ffc00
#define VPU1_BUF_OFFSET        0x3fe000
#define VPU1_PROP_OFFSET       0x3fec00
#define CMD_DSP_GET_FEATURE                      110
#define CMD_GET_MTEE_VERSION                     111
#define CMD_GET_VPU_ALGO_VERSION                 112
#define CMD_DSP_ACTIVE                           113
#define CMD_DSP_DEACTIVE                         114
#define MUTEX_TRY_LOCK_TIMEOUT  (3*1000*1000)   // ns
// mtee name
static char goodix_ha_name[] = "com.mediatek.geniezone.gf_ha";
// mtee session
static UREE_SESSION_HANDLE g_mtee_session;
// 0:unlock 1:lock
static uint g_apusys_lock_status;
#ifndef USE_NORMAL_M4U

#define APUSYS_DEV_NODE_NAME "/dev/apusys"
#define PROC_CHIP_INFO       "/proc/chip/info"
// using AcquireVPU_Main active VPU
static void (*AcquireVPU_Main)(char *server_name);
// using ReleaseVPU_Main release VPU
static void (*ReleaseVPU_Main)(char *server_name);

enum {
    APUSYS_DEVICE_NONE   = 0,
    APUSYS_DEVICE_SAMPLE = 1,
    APUSYS_DEVICE_MDLA   = 2,
    APUSYS_DEVICE_VPU    = 3,
    APUSYS_DEVICE_EDMA   = 4,
    APUSYS_DEVICE_WAIT   = 63,  // subgraph mean wait event
    APUSYS_DEVICE_MAX    = 64,  // total support 64 different devices
};

enum {
    DEVICE_UNLOCK = 0,
    DEVICE_LOCK = 1,
};

/* secure lock dont include header */
struct APUSYS_IOCTL_SEC_T {
    int dev_type;
    unsigned int core_num;
    unsigned int reserved0;
    unsigned int reserved1;
    unsigned int reserved2;
    unsigned int reserved3;
    unsigned int reserved4;
    unsigned int reserved5;
};

#define APUSYS_MAGICNO 'A'
#define APUSYS_IOCTL_SEC_DEVICE_LOCK    _IOW(APUSYS_MAGICNO, 60, int)
#define APUSYS_IOCTL_SEC_DEVICE_UNLOCK  _IOW(APUSYS_MAGICNO, 61, int)
// AcquireVPU_V1_fd result
int AcquireVPU_V1_fd = 0;
#endif  // USE_NORMAL_M4U


#ifndef USE_NORMAL_M4U

static void setDspActiveState() {
    TZ_RESULT ret;
    MTEEC_PARAM param[4];
    uint32_t types;
    uint32_t vpu_struct[2] = { 0 };

    param[0].value.a = VPU0_BUF_OFFSET;
    param[0].value.b = 1;
    param[1].value.a = VPU0_PROP_OFFSET;
    param[1].value.b = sizeof(vpu_struct);
    param[2].value.a = VPU1_BUF_OFFSET;
    param[2].value.b = 1;
    param[3].value.a = VPU1_PROP_OFFSET;
    param[3].value.b = sizeof(vpu_struct);

    types = TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT,
                    TZPT_VALUE_INOUT, TZPT_VALUE_INOUT);

    ret = UREE_TeeServiceCall(g_mtee_session, CMD_DSP_ACTIVE, types, param);
    if (ret) {
        LOG_E(LOG_TAG, "UREE_TeeServiceCall Error: ret %d\n", ret);
    } else {
        if (param[1].value.a) {
            LOG_E(LOG_TAG, "fail to do CMD_DSP_ACTIVE ");
        } else {
#ifndef USE_NORMAL_M4U
            LOG_D(LOG_TAG, "success to do CMD_DSP_ACTIVE");
#endif  // USE_NORMAL_M4U
        }
    }
}

static void setDspDeactiveState() {
    TZ_RESULT ret;
    MTEEC_PARAM param[4];
    uint32_t types;
    uint32_t vpu_struct[2] = { 0 };

    param[0].value.a = VPU0_BUF_OFFSET;
    param[0].value.b = 1;
    param[1].value.a = VPU0_PROP_OFFSET;
    param[1].value.b = sizeof(vpu_struct);
    param[2].value.a = VPU1_BUF_OFFSET;
    param[2].value.b = 1;
    param[3].value.a = VPU1_PROP_OFFSET;
    param[3].value.b = sizeof(vpu_struct);

    types = TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT,
                    TZPT_VALUE_INOUT, TZPT_VALUE_INOUT);

    ret = UREE_TeeServiceCall(g_mtee_session, CMD_DSP_DEACTIVE, types, param);
    if (ret) {
        LOG_E(LOG_TAG, "UREE_TeeServiceCall Error: ret %d\n", ret);
    } else {
        if (param[1].value.a) {
            LOG_E(LOG_TAG, "fail to do CMD_DSP_ACTIVE ");
        } else {
#ifndef USE_NORMAL_M4U
            LOG_D(LOG_TAG, "success to do CMD_DSP_ACTIVE");
#endif  // USE_NORMAL_M4U
        }
    }
}

static void acquireVpuV0(char *server_name) {
    int fd;
    int ret;
    (void)(server_name);

    setDspActiveState();

    fd = open("/dev/vpu", O_RDWR);
    if (fd < 0) {
        LOG_E(LOG_TAG, "%s open fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }
    ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_ON, NULL);
    if (ret != 0) {
        LOG_E(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_ON fail, err = %s\n", __func__, strerror(errno));
        return;
    } else {
        LOG_D(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_ON done\n", __func__);
        g_apusys_lock_status = DEVICE_LOCK;
    }

    ret = close(fd);
    if (ret != 0) {
        LOG_E(LOG_TAG, "%s close fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }

    return;
}

static void releaseVpuV0(char *server_name) {
    int fd;
    int ret;
    (void)(server_name);

    setDspDeactiveState();

    fd = open("/dev/vpu", O_RDWR);
    if (fd < 0) {
        LOG_E(LOG_TAG, "%s open fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }

    ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_OFF, NULL);
    if (ret != 0) {
        LOG_E(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_OFF fail, err = %s\n", __func__, strerror(errno));
        return;
    } else {
        LOG_D(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_OFF done\n", __func__);
        g_apusys_lock_status = DEVICE_UNLOCK;
    }

    ret = close(fd);
    if (ret != 0) {
        LOG_E(LOG_TAG, "%s close fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }

    return;
}

static void acquireVpuV1(char *server_name) {
    int fd;
    int ret;
    struct APUSYS_IOCTL_SEC_T ioctl_param = { .dev_type = APUSYS_DEVICE_VPU };
    (void)(server_name);

    setDspActiveState();

    fd = open(APUSYS_DEV_NODE_NAME, O_RDWR);
    if (fd < 0) {
        LOG_E(LOG_TAG, "%s open fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }
    AcquireVPU_V1_fd = fd;

    ret = ioctl(fd, APUSYS_IOCTL_SEC_DEVICE_LOCK, &ioctl_param);
    if (ret != 0) {
        LOG_E(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK fail, err = %s\n", __func__, strerror(errno));
        return;
    } else {
        LOG_D(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK done\n", __func__);
        g_apusys_lock_status = DEVICE_LOCK;

    }

    return;
}

static void releaseVpuV1(char *server_name) {
    int fd;
    int ret = -1;
    struct APUSYS_IOCTL_SEC_T ioctl_param = { .dev_type = APUSYS_DEVICE_VPU };
    (void)(server_name);

    setDspActiveState();

    if (0 > AcquireVPU_V1_fd) {
        LOG_E(LOG_TAG, "%s , 0 > AcquireVPU_V1_fd \n", __func__);
        return;
    }
    fd = AcquireVPU_V1_fd;
    ret = ioctl(fd, APUSYS_IOCTL_SEC_DEVICE_UNLOCK, &ioctl_param);
    if (0 != ret) {
        LOG_E(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK fail, err = %s\n", __func__, strerror(errno));
        return;
    } else {
        LOG_D(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK done\n", __func__);
        g_apusys_lock_status = DEVICE_UNLOCK;
    }

    ret = close(fd);
    if (0  != ret) {
        LOG_E(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK fail, err = %s\n", __func__, strerror(errno));
        return;
    } else {
        LOG_D(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK done\n", __func__);
        AcquireVPU_V1_fd = 0;
    }

    return;
}

static int secureM4uOpen(void) {
    int m4u_fd;
    int ret;
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    m4u_fd = open("/proc/m4u", O_RDONLY);
    if (m4u_fd < 0) {
        LOG_E(LOG_TAG, "%s open fd fail, err = %s\n", __func__, strerror(errno));
        return -1;
    }

    ret = ioctl(m4u_fd, MTK_M4U_T_SEC_INIT, NULL);
    if (ret != 0) {
        LOG_E(LOG_TAG, "%s ioctl MTK_M4U_T_SEC_INIT fail, err = %s\n", __func__, strerror(errno));
        return -1;
    }
    LOG_D(LOG_TAG, "%s ioctl MTK_M4U_T_SEC_INIT done\n", __func__);

    ret = close(m4u_fd);
    if (ret != 0) {
        LOG_E(LOG_TAG, "%s close fd fail, err = %s\n", __func__, strerror(errno));
        return -1;
    }
    LOG_D(LOG_TAG, "%s close fd done\n", __func__);

    FUNC_EXIT(err);
    return 0;
}
#endif  // USE_NORMAL_M4U

namespace goodix {
    sem_t HalDsp::sSem;
    HalDsp::HalDsp(CaEntry *caEntry) : mDspStatus(DSP_NOT_AVAILABLE), mDspSpeedStatus(DSP_NORMAL_SPEED), mDspResult(TZ_RESULT_SUCCESS) {
        mCaEntry = caEntry;
        memset(&mProxyCmd, 0, sizeof(gf_dsp_proxy_cmd_t));
    }

    HalDsp::~HalDsp() {
    }

    gf_error_t HalDsp::init(gf_sensor_info_t info) {
        gf_error_t err = GF_SUCCESS;
        TZ_RESULT ret;
        UNUSED_VAR(info);
        Mutex::Autolock _l(mDspMutex);
        int32_t vpu_version = 1;

        FUNC_ENTER();
        do {
            // run thread
            if (!isRunning()) {
                if (!run("dsp_handle_thread")) {
                    err = GF_ERROR_DSP_NOT_AVAILABLE;
                }
            } else {
                err = GF_ERROR_DSP_NOT_AVAILABLE;
            }

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] pthread create failed", __func__);
                break;
            }
            // wait thread signal
            mCond.wait(mDspMutex);
            LOG_D(LOG_TAG, "[%s] receive ready", __func__);

            err = dspSemInit();
            if (err != GF_SUCCESS) {
                break;
            }
            mDspStatus = DSP_INITIALIZING;
            ret = UREE_CreateSession(goodix_ha_name, &g_mtee_session);

            LOG_D(LOG_TAG, "[%s] UREE_CreateSession ret = %d, g_mtee_session = %d", __func__, ret, g_mtee_session);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] UREE_CreateSession fail(%s), %d", __func__, goodix_ha_name, ret);
                mDspStatus = DSP_NOT_AVAILABLE;
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

#ifndef USE_NORMAL_M4U

            if (vpu_version == 0) {
                AcquireVPU_Main = acquireVpuV0;
                ReleaseVPU_Main = releaseVpuV0;
                LOG_D(LOG_TAG, "[%s] use Acquire/Release V0 VPU API", __func__);
            } else {
                AcquireVPU_Main = acquireVpuV1;
                ReleaseVPU_Main = releaseVpuV1;
                LOG_D(LOG_TAG, "[%s] use Acquire/Release V1 VPU API", __func__);
            }

            if (vpu_version) {
                AcquireVPU_Main(NULL);
            }

            ret = secureM4uOpen();
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] secureM4uOpen fail", __func__);
                mDspStatus = DSP_NOT_SECURE;
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            if (vpu_version) {
                ReleaseVPU_Main(NULL);
            }
#endif  // USE_NORMAL_M4U
            // start init dsp shared memory
            gf_cmd_header_t header = { 0 };
            header.target = GF_TARGET_MAX;
            header.cmd_id = 0;
            err = mCaEntry->sendCommand(&header, sizeof(header));
            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] init dsp shared memory failed", __func__);
                break;
            }
            mDspStatus = DSP_AVAILABLE;
            g_apusys_lock_status = DEVICE_UNLOCK;
            setDspHighFreq();
            sendCmdToDsp(DSP_CMD_GET_VERSION);
            setDspNormalFreq();
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::reinit() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::deinit() {
        gf_error_t err = GF_SUCCESS;
        TZ_RESULT ret;

        FUNC_ENTER();

        do {
            Mutex::Autolock _l(mDspMutex);
            dspSemDeinit();
            ret = UREE_CloseSession(g_mtee_session);
            LOG_D(LOG_TAG, "[%s] UREE_CloseSession ret = %d, g_mtee_session = %d", __func__, ret, g_mtee_session);
            if (ret) {
                LOG_E(LOG_TAG, "[%s] UREE_CloseSession fail(%s)", __func__,  goodix_ha_name);
                err = GF_ERROR_BASE;
            }
            mDspStatus = DSP_NOT_AVAILABLE;
        } while (0);

        mProxyCmd.type = DSP_PROXY_EXIT;
        requestExit();
        mCond.signal();
        join();

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::sendCmdToDsp(gf_dsp_cmd_t cmd) {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();

        do {
            TZ_RESULT ret;
            MTEEC_PARAM param[4];
            uint32_t types;
            uint32_t vpu_struct[2] = {0};
            char mtee_version[256] = {0};

            LOG_D(LOG_TAG, "[%s] gf_hal_send_cmd_to_dsp  g_mtee_session = %d", __func__, g_mtee_session);

            if (mDspStatus != DSP_AVAILABLE) {
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            switch (cmd) {
                case DSP_CMD_GET_FEATURE_TWO : {
                    if (mDspStatus != DSP_AVAILABLE) {
                        LOG_D(LOG_TAG, "[%s] dsp not available", __func__);
                        err = GF_ERROR_DSP_NOT_AVAILABLE;
                        break;
                    }

                    if (mDspMutex.timedLock(MUTEX_TRY_LOCK_TIMEOUT)) {
                        LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available", __func__);
                        err = GF_ERROR_DSP_NOT_AVAILABLE;
                        break;
                    }
                    memset(&mProxyCmd, 0, sizeof(gf_dsp_proxy_cmd_t));

                    clearDspSem();

                    mProxyCmd.cmd_type = SEC_FP_HVX_GET_FEATURE_TWO;
                    mDspMutex.unlock();
                    mCond.signal();
                    break;
                }

                case DSP_CMD_SET_HIGH_SPEED : {
                    if (mDspMutex.timedLock(MUTEX_TRY_LOCK_TIMEOUT)) {
                        LOG_E(LOG_TAG, "[%s] trylock fail, set dsp high speed fail!", __func__);
                        err = GF_ERROR_DSP_NOT_AVAILABLE;
                        break;
                    }
                    memset(&mProxyCmd, 0, sizeof(gf_dsp_proxy_cmd_t));

                    clearDspSem();

                    mProxyCmd.cmd_type = SEC_FP_SET_HIGH_SPEED;
                    mDspMutex.unlock();
                    mCond.signal();
                    break;
                }

                case DSP_CMD_SET_NORMAL_SPEED : {
                    if (mDspMutex.timedLock(MUTEX_TRY_LOCK_TIMEOUT)) {
                        LOG_E(LOG_TAG, "[%s] trylock fail, set dsp normal speed fail!", __func__);
                        err = GF_ERROR_DSP_NOT_AVAILABLE;
                        break;
                    }
                    memset(&mProxyCmd, 0, sizeof(gf_dsp_proxy_cmd_t));

                    clearDspSem();

                    mProxyCmd.cmd_type = SEC_FP_SET_NORMAL_SPEED;
                    mDspMutex.unlock();
                    mCond.signal();
                    break;
                }

                case DSP_CMD_GET_VERSION: {
                    types = TZ_ParamTypes2(TZPT_MEM_OUTPUT, TZPT_VALUE_OUTPUT);
                    param[0].mem.buffer = (void *)mtee_version;
                    param[0].mem.size = 256;
                    param[1].value.a = 0;
                    param[1].value.b = 0;

                    ret = UREE_TeeServiceCall(g_mtee_session, CMD_GET_MTEE_VERSION, types, param);
                    if (ret) {
                        LOG_E(LOG_TAG, "[%s] UREE_TeeServiceCall Error: ret %d", __func__,  ret);
                    } else {
                        if (param[1].value.a) {
                            LOG_E(LOG_TAG, "[%s] Get MTEE Version Error: 0X%X", __func__, param[1].value.a);
                        } else {
                            LOG_D(LOG_TAG, "[%s] Get MTEE Version Success : %s", __func__, (char *)param[0].mem.buffer);
                        }
                    }

                    types = TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT,
                                    TZPT_VALUE_INOUT, TZPT_VALUE_INOUT);
                    param[0].value.a = VPU0_BUF_OFFSET;
                    param[0].value.b = 1;
                    param[1].value.a = VPU0_PROP_OFFSET;
                    param[1].value.b = sizeof(vpu_struct);
                    param[2].value.a = VPU1_BUF_OFFSET;
                    param[2].value.b = 1;
                    param[3].value.a = VPU1_PROP_OFFSET;
                    param[3].value.b = sizeof(vpu_struct);
                    ret = UREE_TeeServiceCall(g_mtee_session, CMD_GET_VPU_ALGO_VERSION, types, param);
                    if (ret) {
                        LOG_E(LOG_TAG, "[%s] UREE_TeeServiceCall Error: ret %d", __func__, ret);
                    } else {
                        if (param[1].value.a) {
                            LOG_E(LOG_TAG, "[%s] Get DSP Algo Version Error, errno = 0X%X", __func__, param[1].value.a);
                        } else {
                            LOG_D(LOG_TAG, "[%s] Dsp algo version: %d", __func__, param[0].value.b);
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        } while (0);
        UNUSED_VAR(cmd);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::setDspHighFreq() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
#ifndef USE_NORMAL_M4U
            AcquireVPU_Main(NULL);
#else    // USE_NORMAL_M4U
            int fd;
            int ret;
            fd = open("/dev/vpu", O_RDWR);
            if (fd < 0) {
                LOG_E(LOG_TAG, "[%s] open fd fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }
            ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_ON, NULL);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] ioctl VPU_IOCTL_SDSP_POWER_ON fail", __func__);
                err = GF_ERROR_BASE;
                break;
            } else {
                LOG_D(LOG_TAG, "[%s] ioctl VPU_IOCTL_SDSP_POWER_ON done", __func__);
                g_apusys_lock_status = DEVICE_LOCK;
            }
            ret = close(fd);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] close fd fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }
#endif  // USE_NORMAL_M4U
        } while (0);
        if (g_apusys_lock_status == DEVICE_LOCK) {
            mDspSpeedStatus = DSP_HIGH_SPEED;
        } else {
            mDspSpeedStatus = DSP_NORMAL_SPEED;
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::setDspNormalFreq() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
#ifndef USE_NORMAL_M4U
            ReleaseVPU_Main(NULL);
#else  // USE_NORMAL_M4U
            int fd;
            int ret;
            fd = open("/dev/vpu", O_RDWR);
            if (fd < 0) {
                LOG_E(LOG_TAG, "[%s] open fd fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }

            ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_OFF, NULL);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] ioctl VPU_IOCTL_SDSP_POWER_OFF fail", __func__);
                err = GF_ERROR_BASE;
                break;
            } else {
                LOG_D(LOG_TAG, "[%s] ioctl VPU_IOCTL_SDSP_POWER_OFF done", __func__);
                g_apusys_lock_status = DEVICE_UNLOCK;
            }
            ret = close(fd);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] close fd fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }
#endif  // USE_NORMAL_M4U
        } while (0);
        if (g_apusys_lock_status == DEVICE_UNLOCK) {
            mDspSpeedStatus = DSP_NORMAL_SPEED;
        } else {
            mDspSpeedStatus = DSP_HIGH_SPEED;
        }
        FUNC_EXIT(err);
        return err;
    }

    void HalDsp::processSendCmd() {
        while (1) {
            // wait for condition
            mCond.wait(mDspMutex);
            LOG_D(LOG_TAG, "[%s] receive cmd", __func__);
            LOG_D(LOG_TAG, "[%s] gf_hal_send_cmd_to_dsp g_mtee_session = %d", __func__, g_mtee_session);
            TZ_RESULT ret;
            MTEEC_PARAM param[4];
            uint32_t types;
            uint32_t vpu_struct[2] = {0};
            gf_error_t err = GF_SUCCESS;

            switch (mProxyCmd.cmd_type) {
                case SEC_FP_HVX_GET_FEATURE_TWO : {
                    types = TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT,
                                                        TZPT_VALUE_INOUT, TZPT_VALUE_INOUT);
                    param[0].value.a = VPU0_BUF_OFFSET;
                    param[0].value.b = 1;
                    param[1].value.a = VPU0_PROP_OFFSET;
                    param[1].value.b = sizeof(vpu_struct);
                    param[2].value.a = VPU1_BUF_OFFSET;
                    param[2].value.b = 1;
                    param[3].value.a = VPU1_PROP_OFFSET;
                    param[3].value.b = sizeof(vpu_struct);
                    ret = UREE_TeeServiceCall(g_mtee_session, CMD_DSP_GET_FEATURE, types, param);
                    if (ret) {
                        LOG_E(LOG_TAG, "[%s] UREE_TeeServiceCall Error: ret=%d", __func__, ret);
                        err = GF_ERROR_DSP_GET_FEATURE_FAIL;
                        mDspResult = ret;
                        break;
                    } else {
                        if (param[1].value.a) {
                            LOG_E(LOG_TAG, "[%s] Get VPU Feature Error: 0X%X", __func__, param[1].value.a);
                            err  = GF_ERROR_DSP_GET_FEATURE_FAIL;
                        }
                    }
                    mDspResult = param[1].value.a;
                    break;
                }

                case SEC_FP_SET_HIGH_SPEED : {
                    setDspHighFreq();
                    break;
                }

                case SEC_FP_SET_NORMAL_SPEED : {
                    setDspNormalFreq();
                    break;
                }

                default: {
                    LOG_E(LOG_TAG, "[%s] wrong dsp cmd", __func__);
                    err = GF_ERROR_BASE;
                    break;
                }
            }

            if (err == GF_SUCCESS) {
                LOG_I(LOG_TAG, "[%s] post sem", __func__);
                sem_post(&sSem);
            }

            if (DSP_PROXY_EXIT == mProxyCmd.type) {
                LOG_D(LOG_TAG, "[%s] exit thread", __func__);
                break;
            }
        }
        return;
    }

    gf_error_t HalDsp::dspSemInit() {
        gf_error_t err = GF_SUCCESS;
        int32_t ret = 0;
        FUNC_ENTER();
        do {
            ret = sem_init(&sSem, 0, 0);
            if (ret != 0) {
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                LOG_E(LOG_TAG, "[%s] init semaphore failed, ret=%d", __func__, ret);
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void HalDsp::dspSemDeinit() {
        LOG_D(LOG_TAG, "[%s] sem destroied!", __func__);
        sem_destroy(&sSem);
        return;
    }

    gf_error_t HalDsp::waitDspNotify(uint32_t wait_time_sec, uint32_t wait_time_us) {
        gf_error_t err = GF_SUCCESS;
        int ret_sem = -1;
        struct timespec abs_time;
        uint32_t wait_time_nsec = wait_time_us * 1000;

        FUNC_ENTER();

        do {
            if (clock_gettime(CLOCK_REALTIME, &abs_time) < 0) {
                LOG_E(LOG_TAG, "[%s] clock gettime fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }

            // set wait time, TODO: when system time change,timeout??
            abs_time.tv_sec += wait_time_sec;
            abs_time.tv_nsec += wait_time_nsec;
            abs_time.tv_sec += abs_time.tv_nsec / 1000000000;   // Nanoseconds [0 ..999999999], to avoid overflow
            abs_time.tv_nsec = abs_time.tv_nsec % 1000000000;

            LOG_D(LOG_TAG, "[%s] hal begin wait sem", __func__);
            if ((wait_time_sec != 0) || (wait_time_us != 0)) {
                ret_sem = sem_timedwait(&sSem, &abs_time);
            } else {
                ret_sem = sem_wait(&sSem);
            }
            LOG_D(LOG_TAG, "[%s] hal end wait sem, ret_sem:%d", __func__, ret_sem);

            if (0 != ret_sem) {
                err = GF_ERROR_DSP_WAIT_TIMEOUT;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::waitDspNotify() {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();

        do {
            if (mDspStatus != DSP_AVAILABLE) {
                LOG_D(LOG_TAG, "[%s] dsp not available", __func__);
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            err = waitDspNotify(0, 500 * 1000);
            if (GF_SUCCESS == err) {
                if (mDspResult != 0) {
                    LOG_E(LOG_TAG, "[%s] dsp get feature fail, result:0x%x", __func__, mDspResult);
                    err = GF_ERROR_DSP_GET_FEATURE_FAIL;
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void HalDsp::clearDspSem() {
        gf_error_t err = GF_SUCCESS;
        VOID_FUNC_ENTER();

        do {
            if (mDspStatus != DSP_AVAILABLE) {
                LOG_D(LOG_TAG, "[%s] dsp not available", __func__);
                break;
            }

            if (mDspMutex.timedLock(MUTEX_TRY_LOCK_TIMEOUT)) {
                LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available", __func__);
                break;
            }

            while (1) {
                int count = 0;
                sem_getvalue(&sSem, &count);
                if (count > 0) {
                    err = waitDspNotify(0, 1);  // wait 1US
                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] wait notify fail", __func__);
                        break;
                    } else {
                        LOG_D(LOG_TAG, "[%s] there is dsp sem and decrease it", __func__);
                    }
                } else {
                    LOG_D(LOG_TAG, "[%s] there is no dsp sem", __func__);
                    break;
                }
            }
            mDspMutex.unlock();
        } while (0);

        VOID_FUNC_EXIT();
        return;
    }

    void HalDsp::markDspNotAvailable(void) {
        mDspStatus = DSP_NOT_AVAILABLE;
        LOG_D(LOG_TAG, "[%s] after set dsp status:%d", __func__, mDspStatus);
    }

    gf_dsp_status_t HalDsp::checkDspValid() {
        LOG_D(LOG_TAG, "[%s] dsp status:%d", __func__, mDspStatus);

        if (mDspMutex.timedLock(MUTEX_TRY_LOCK_TIMEOUT)) {
            LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available", __func__);
            return DSP_NOT_AVAILABLE;
        }

        mDspMutex.unlock();

        return mDspStatus;
    }

    gf_dsp_status_t HalDsp::checkDspSpeed() {
        LOG_D(LOG_TAG, "[%s] Dsp Speed Status:%d", __func__, mDspSpeedStatus);
        if (mDspMutex.timedLock(MUTEX_TRY_LOCK_TIMEOUT)) {
            LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available", __func__);
            return DSP_NOT_AVAILABLE;
        }
        mDspMutex.unlock();
        return mDspSpeedStatus;
    }

    bool HalDsp::threadLoop() {
        while (!Thread::exitPending()) {
            LOG_D(LOG_TAG, "[%s] thread runing", __func__);
            Mutex::Autolock _l(mDspMutex);

            mCond.signal();
            LOG_D(LOG_TAG, "[%s] notify ready", __func__);

            processSendCmd();
        }

        return false;
    }
    gf_error_t HalDsp::dspProxySendCmd(gf_dsp_proxy_cmd_t *proxy_cmd) {
        UNUSED_VAR(proxy_cmd);
        return GF_SUCCESS;
    }

    gf_error_t HalDsp::proxyHandleInit(void) {
        return GF_SUCCESS;
    }

    void HalDsp::proxyHandleDeinit(void) {
        return;
    }

    gf_error_t HalDsp::initInternal() {
        return GF_SUCCESS;
    }

    int32_t HalDsp::onDspCmdResult(int cmd, int result) {
        UNUSED_VAR(cmd);
        UNUSED_VAR(result);
        return 0;
    }
    int32_t HalDsp::convertChipType(gf_sensor_info_t info) {
        UNUSED_VAR(info);
        return 0;
    }
}  // namespace goodix
