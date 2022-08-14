/************************************************************************************
 ** File: - CaEntry_mtk.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      CA Entry for goodix optical fingerprint (android O)
 **
 ** Version: 1.0
 ** Date : 18:03:11,23/02/2019
 ** Author: Wudongnan@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  Dongnan.Wu    2019/02/23        create dsp entry file for mtk platform
 **  Dongnan.Wu    2019/03/19        add SDSP support function
 **  Dongnan.Wu    2020/01/21        solve coverity issue
 ************************************************************************************/

#define LOG_TAG "[GF_HAL][HalDsp]"

#include <fcntl.h>
#include <unistd.h>

#ifdef SUPPORT_DSP
#include "uree/system.h"
#include <sys/ioctl.h>
#include "CaEntry.h"
#include "Vpu_drv.h"
#endif  // #ifdef SUPPORT_DSP

#include "HalDsp.h"
#include "HalLog.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef UNUSED_VAR
#define UNUSED_VAR(X)   ((void)(X))
#endif  // UNUSED_VAR

#include "gf_algo_types.h"

#ifdef SUPPORT_DSP
#define VPU0_BUF_OFFSET 0x3ff000
#define VPU0_PROP_OFFSET 0x3ffc00
#define VPU1_BUF_OFFSET 0x3fe000
#define VPU1_PROP_OFFSET 0x3fec00

#define CMD_DSP_GET_FEATURE     110
#define CMD_GET_MTEE_VERSION    111
#define CMD_GET_VPU_ALGO_VERSION     112
#define CMD_DSP_ACTIVE                           113
#define CMD_DSP_DEACTIVE                       114


// mtee name
static char goodix_ha_name[] = "com.mediatek.geniezone.gf_ha";
// mtee session
static UREE_SESSION_HANDLE g_mtee_session;
// vpu status

#ifndef USE_NORMAL_M4U
static void (*AcquireVPU_Main)(char *server_name);
static void (*ReleaseVPU_Main)(char *server_name);

//#define VPU_MAGICNO                 'v'
//#define VPU_IOCTL_SDSP_POWER_ON     _IOW(VPU_MAGICNO,   60, int)
//#define VPU_IOCTL_SDSP_POWER_OFF    _IOW(VPU_MAGICNO,   61, int)

#define APUSYS_DEV_NODE_NAME "/dev/apusys"
#define PROC_CHIP_INFO       "/proc/chip/info"

enum {
	APUSYS_DEVICE_NONE   = 0,
	APUSYS_DEVICE_SAMPLE = 1,
	APUSYS_DEVICE_MDLA   = 2,
	APUSYS_DEVICE_VPU    = 3,
	APUSYS_DEVICE_EDMA   = 4,
	APUSYS_DEVICE_WAIT   = 63, // subgraph mean wait event
	APUSYS_DEVICE_MAX    = 64, //total support 64 different devices
};

/* secure lock dont include header */
struct apusys_ioctl_sec {
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
#define APUSYS_IOCTL_SEC_DEVICE_LOCK 	_IOW(APUSYS_MAGICNO, 60, int)
#define APUSYS_IOCTL_SEC_DEVICE_UNLOCK	_IOW(APUSYS_MAGICNO, 61, int)

int AcquireVPU_V1_fd = 0;
#endif // #ifndef USE_NORMAL_M4U

uint32_t detection_vpu_version(void)
{
    FILE *chip_info;
    char hw_code[16];

    if (!(chip_info = fopen(PROC_CHIP_INFO, "r"))) {
        LOG_E(LOG_TAG, "%s no %s", __func__, PROC_CHIP_INFO);
        exit(0);
    }

    fscanf(chip_info, "%s", hw_code);
    fscanf(chip_info, "%s", hw_code);
    fscanf(chip_info, "%s", hw_code);
    LOG_D(LOG_TAG, "hw_code %s\n", hw_code);

    fclose(chip_info);

    if (strncmp(hw_code, "(0788)", sizeof("0778"))==0) {    //6771
        return 0;
    } else if (strncmp(hw_code, "(0725)", sizeof("0725"))==0) {     //6779
        return 0;
    } else {
        return 1;
    }
}
#endif  // #ifdef SUPPORT_DSP

#ifndef USE_NORMAL_M4U

void gf_hal_dsp_set_active_state()
{
    TZ_RESULT ret;
    MTEEC_PARAM param[4];
    uint32_t types;
    uint32_t vpu_struct[2] ={ 0 };

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
    if (ret)
    {
        LOG_E(LOG_TAG, "UREE_TeeServiceCall Error: ret %d\n", ret);
    }
    else
    {
        if (param[1].value.a)
        {
            LOG_E(LOG_TAG, "fail to do CMD_DSP_ACTIVE ");
        }
        else
        {
#ifndef USE_NORMAL_M4U
            LOG_D(LOG_TAG, "success to do CMD_DSP_ACTIVE");
#endif
        }
    }
}

void gf_hal_dsp_set_deactive_state()
{
    TZ_RESULT ret;
    MTEEC_PARAM param[4];
    uint32_t types;
    uint32_t vpu_struct[2] ={ 0 };

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
    if (ret)
    {
        LOG_E(LOG_TAG, "UREE_TeeServiceCall Error: ret %d\n", ret);
    }
    else
    {
        if (param[1].value.a)
        {
            LOG_E(LOG_TAG, "fail to do CMD_DSP_ACTIVE ");
        }
        else
        {
#ifndef USE_NORMAL_M4U
        LOG_D(LOG_TAG, "success to do CMD_DSP_ACTIVE");
#endif
        }
    }
}

static void AcquireVPU_V0(char *server_name)
{
    int fd;
    int ret;
    (void)(server_name);

    //gf_hal_dsp_set_active_state();

    fd = open("/dev/vpu", O_RDWR);
    if (fd < 0)
    {
        LOG_E(LOG_TAG, "%s open fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }

    ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_ON, NULL);
    if (ret != 0)
    {
        LOG_E(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_ON fail, err = %s\n", __func__, strerror(errno));
        close(fd);
        return;
    }
    else {
        LOG_D(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_ON done\n", __func__);
    }

    ret = close(fd);
    if (ret != 0)
    {
        LOG_E(LOG_TAG, "%s close fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }

    return;
}

static void ReleaseVPU_V0(char *server_name) {
    int fd;
    int ret;
    (void)(server_name);

    //gf_hal_dsp_set_deactive_state();

    fd = open("/dev/vpu", O_RDWR);
    if (fd < 0)
    {
        LOG_E(LOG_TAG, "%s open fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }

    ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_OFF, NULL);
    if (ret != 0)
    {
        LOG_E(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_OFF fail, err = %s\n", __func__, strerror(errno));
        close(fd);
        return;
    }
    else {
        LOG_D(LOG_TAG, "%s ioctl VPU_IOCTL_SDSP_POWER_OFF done\n", __func__);
    }

    ret = close(fd);
    if (ret != 0)
    {
        LOG_E(LOG_TAG, "%s close fd fail, err = %s\n", __func__, strerror(errno));
        return;
    }

    return;
}

static void AcquireVPU_V1(char *server_name) {
    int fd;
    int ret;
    struct apusys_ioctl_sec ioctl_param = { .dev_type = APUSYS_DEVICE_VPU };
    (void)(server_name);

    gf_hal_dsp_set_active_state();

    fd = open(APUSYS_DEV_NODE_NAME, O_RDWR);
    if (fd <0) {
        LOG_E(LOG_TAG, "%s open fd fail, err = %s\n", __func__, strerror(errno));
        return;

    }
    AcquireVPU_V1_fd = fd;

    ret = ioctl(fd, APUSYS_IOCTL_SEC_DEVICE_LOCK, &ioctl_param);
    if (ret != 0)
    {
        LOG_E(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK fail, err = %s\n", __func__, strerror(errno));
        return;
    }
    else {
        LOG_D(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK done\n", __func__);
    }

    return;
}

static void ReleaseVPU_V1(char *server_name) {
    int fd;
    int ret = -1;
    struct apusys_ioctl_sec ioctl_param = { .dev_type = APUSYS_DEVICE_VPU };
    (void)(server_name);

    gf_hal_dsp_set_deactive_state();

    if (0 == AcquireVPU_V1_fd)
    {
        LOG_E(LOG_TAG, "%s , 0 == AcquireVPU_V1_fd \n", __func__);
        return;

    }
    fd = AcquireVPU_V1_fd;

    ret = ioctl(fd, APUSYS_IOCTL_SEC_DEVICE_UNLOCK, &ioctl_param);
    if (0 != ret)
    {
        LOG_E(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK fail, err = %s\n", __func__, strerror(errno));
        return;
    }
    else {
        LOG_D(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK done\n", __func__);
    }

    ret = close(fd);
    if (0  != ret) {
        LOG_E(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK fail, err = %s\n", __func__, strerror(errno));
        return;
    }
       else {
        LOG_D(LOG_TAG, "%s ioctl APUSYS_IOCTL_SEC_DEVICE_LOCK done\n", __func__);
          AcquireVPU_V1_fd = 0;
    }

       return;
}

static int secure_m4u_open(void) {
    int m4u_fd;
    int ret;

    m4u_fd = open("/proc/m4u", O_RDONLY);
    if (m4u_fd <0) {
        LOG_E(LOG_TAG,"%s open fd fail, err = %s\n", __func__, strerror(errno));
        return -1;
    }

    ret = ioctl(m4u_fd, MTK_M4U_T_SEC_INIT, NULL);
    if (ret!=0) {
        LOG_E(LOG_TAG,"%s ioctl MTK_M4U_T_SEC_INIT fail, err = %s\n", __func__, strerror(errno));
        close(m4u_fd);
        return -1;
    }
    LOG_D(LOG_TAG,"%s ioctl MTK_M4U_T_SEC_INIT done\n", __func__);

    ret = close(m4u_fd);
    if (ret!=0) {
        LOG_E(LOG_TAG,"%s close fd fail, err = %s\n", __func__, strerror(errno));
        return -1;
    }
    LOG_D(LOG_TAG,"%s close fd done\n", __func__);

    return 0;
}
#endif

namespace goodix {

    HalDsp::HalDsp(CaEntry *caEntry) : mDspTime(0), mDspStatus(DSP_NOT_AVAILABLE)
    {
        mCaEntry = caEntry;
        memset(&mProxyCmd, 0, sizeof(gf_dsp_proxy_cmd_t));
    }

    HalDsp::~HalDsp()
    {
    }

    gf_error_t HalDsp::init() {
        gf_error_t err = GF_SUCCESS;
#ifdef SUPPORT_DSP
        TZ_RESULT ret;
        int32_t vpu_version = detection_vpu_version();
        gf_algo_init_t cmd = {{ 0 }};
        gf_cmd_header_t *header = (gf_cmd_header_t*) &cmd;

        FUNC_ENTER();
        do {
            mDspStatus = DSP_INITIALIZING;
            ret = UREE_CreateSession(goodix_ha_name, &g_mtee_session);
            LOG_D(LOG_TAG, "[%s] UREE_CreateSession ret = %d, g_mtee_session = %d", __func__, ret, g_mtee_session);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] UREE_CreateSession fail(%s), %d", __func__, goodix_ha_name, ret);
                mDspStatus = DSP_NOT_AVAILABLE;
                err = GF_ERROR_BASE;
                break;
            }

#ifndef USE_NORMAL_M4U
            if (vpu_version==0) {
                AcquireVPU_Main = AcquireVPU_V0;
                ReleaseVPU_Main = ReleaseVPU_V0;
                LOG_D(LOG_TAG, "[%s] use Acquire/Release V0 VPU API", __func__);
            } else {
                AcquireVPU_Main = AcquireVPU_V1;
                ReleaseVPU_Main = ReleaseVPU_V1;
                LOG_D(LOG_TAG, "[%s] use Acquire/Release V1 VPU API", __func__);
            }

            if (vpu_version) {
                AcquireVPU_Main(NULL);
            }

            ret = secure_m4u_open();
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] secure_m4u_open fail", __func__);
                mDspStatus = DSP_NOT_SECURE;
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            if (vpu_version) {
                ReleaseVPU_Main(NULL);
            }

#endif

            header->target = GF_TARGET_ALGO;
            header->cmd_id = GF_CMD_ALGO_DSP_MEM_INIT;
            err = mCaEntry->sendCommand(&cmd, sizeof(cmd));
            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] init dsp shared buffer failed", __func__);
                break;
            }

            mDspStatus = DSP_AVAILABLE;

            setDspHighFreq();
            sendCmdToDsp(DSP_CMD_GET_VERSION);
            setDspNormalFreq();

        } while (0);
#endif  // #ifdef SUPPORT_DSP

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
#ifdef SUPPORT_DSP
        TZ_RESULT ret;

        FUNC_ENTER();

        ret = UREE_CloseSession(g_mtee_session);
        LOG_D(LOG_TAG, "[%s] UREE_CloseSession ret = %d, g_mtee_session = %d", __func__, ret, g_mtee_session);
        if (ret) {
            LOG_E(LOG_TAG, "[%s] UREE_CloseSession fail(%s)", __func__,  goodix_ha_name);
            err = GF_ERROR_BASE;
        }
        mDspStatus = DSP_NOT_AVAILABLE;
#endif  // #ifdef SUPPORT_DSP
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::sendCmdToDsp(gf_dsp_cmd_t cmd) {
        gf_error_t err = GF_SUCCESS;

#ifdef SUPPORT_DSP
        FUNC_ENTER();

        do {
            TZ_RESULT ret;
            MTEEC_PARAM param[4];
            uint32_t types;
            uint32_t vpu_struct[2] = {0};
            char mtee_version[256] = {0};

            LOG_D(LOG_TAG, "[%s] gf_hal_send_cmd_to_dsp g_mtee_session = %d", __func__, g_mtee_session);

            if (mDspStatus != DSP_AVAILABLE) {
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            switch (cmd) {
                case DSP_CMD_GET_FEATURE_TWO : {
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
                        break;
                    } else {
                        if (param[1].value.a) {
                            LOG_E(LOG_TAG, "[%s] Get VPU Feature Error: 0X%X", __func__, param[1].value.a);
                            // add for dsp fail work around start
                            if (0xFFFF0000 == param[1].value.a) {
                                LOG_E(LOG_TAG, "[%s] dsp handle timeout, set to normal process.", __func__);
                                mDspStatus = DSP_NOT_AVAILABLE;
                                setDspNormalFreq(); // power off vpu here
                            }
                            // add for dsp fail work around end
                            err  = GF_ERROR_DSP_GET_FEATURE_FAIL;
                        }
                    }
                    break;
                }

                case DSP_CMD_SET_HIGH_SPEED : {
                    err = setDspHighFreq();
                    break;
                }

                case DSP_CMD_SET_NORMAL_SPEED : {
                    err = setDspNormalFreq();
                    break;
                }

                case DSP_CMD_GET_VERSION: {
                    types = TZ_ParamTypes2(TZPT_MEM_OUTPUT, TZPT_VALUE_INOUT);
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
#else  // #ifdef SUPPORT_DSP
        UNUSED_VAR(cmd);
#endif  // #endif SUPPORT_DSP
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::waitDspNotify() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        FUNC_EXIT(err);
        return err;
    }

    gf_dsp_status_t HalDsp::checkDspValid() {
        LOG_D(LOG_TAG, "[%s] dsp status:%d", __func__, mDspStatus);
        return mDspStatus;
    }

    gf_error_t HalDsp::setDspHighFreq() {
        gf_error_t err = GF_SUCCESS;
#ifdef SUPPORT_DSP
        FUNC_ENTER();

        do {
#ifndef USE_NORMAL_M4U
            AcquireVPU_Main(NULL);
#else
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
                close(fd);
                err = GF_ERROR_BASE;
                break;
            } else {
                LOG_D(LOG_TAG, "[%s] ioctl VPU_IOCTL_SDSP_POWER_ON done", __func__);
            }

            ret = close(fd);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] close fd fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }
#endif
        } while (0);
#endif  // #ifdef SUPPORT_DSP
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::setDspNormalFreq() {
        gf_error_t err = GF_SUCCESS;
#ifdef SUPPORT_DSP
        int fd;
        int ret;
        FUNC_ENTER();
        do {
#ifndef USE_NORMAL_M4U
            ReleaseVPU_Main(NULL);
#else
            fd = open("/dev/vpu", O_RDWR);
            if (fd < 0) {
                LOG_E(LOG_TAG, "[%s] open fd fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }

            ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_OFF, NULL);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] ioctl VPU_IOCTL_SDSP_POWER_OFF fail", __func__);
                close(fd);
                err = GF_ERROR_BASE;
                break;
            } else {
                LOG_D(LOG_TAG, "[%s] ioctl VPU_IOCTL_SDSP_POWER_OFF done", __func__);
            }

            ret = close(fd);
            if (ret != 0) {
                LOG_E(LOG_TAG, "[%s] close fd fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }
#endif
        } while (0);
#endif  // #ifdef SUPPORT_DSP
        FUNC_EXIT(err);
        return err;
    }

    bool HalDsp::threadLoop() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        FUNC_EXIT(err);

        return false;
    }
}  // namespace goodix
