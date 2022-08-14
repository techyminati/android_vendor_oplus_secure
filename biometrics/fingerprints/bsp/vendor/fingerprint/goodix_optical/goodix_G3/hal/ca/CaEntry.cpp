/************************************************************************************
 ** File: - CaEntry.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      CA Entry for goodix optical fingerprint (android O)
 **
 ** Version: 1.0
 ** Date : 18:03:11,17/10/2018
 ** Author: oujinrong@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  luhongyu   2018/10/17           create file, and add retry of loading ta
 ************************************************************************************/

#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <hardware/hw_auth_token.h>
#include "gf_error.h"
#include "gf_base_types.h"
#include "QSEEComAPI.h"
#include "HalLog.h"
#include "IonMemory.h"
#include "CaEntry.h"
#include "gf_fpcore_types.h"
#include "QseeParams.h"
#include "TaLogDump.h"
#include <cutils/trace.h>

#define LOG_TAG "[CaEntry]"
#define BASE_SHARED_BUFFER_LEN 4096

#define KEYMASTER_UTILS_CMD_ID 0x200UL
#define KEYMASTER_GET_AUTH_TOKEN_KEY (KEYMASTER_UTILS_CMD_ID + 5UL)

#define OPLUS_TA_LOAD_RETRY_TIMES (5)

#ifdef SUPPORT_DSP
extern bool mDspSupport;
#endif   //SUPPORT_DSP

// keep identical with ta
typedef struct
{
    uint32_t cmd_addr;
    uint32_t cmd_len;
    uint32_t req_token;
    uint64_t timestamp;  // millisecond
    uint32_t ta_log_dump_level;
    gf_error_t status;
    uint32_t carveout_data;
    uint32_t carveout_len;
} QSEE_CMD_T;

// keymaster command definition
typedef struct
{
    int32_t cmd_id;
    hw_authenticator_type_t auth_type;
} __attribute__((packed)) KM_GET_AUTH_TOKEN_REQ_T;

typedef struct
{
    int32_t status;
    uint32_t auth_token_key_offset;
    uint32_t auth_token_key_len;
} __attribute__((packed)) KM_GET_AUTH_TOKEN_RSP_T;

namespace goodix
{
    typedef struct
    {
        QSEECom_handle *taHandle;
        uint32_t requestToken;
        IonMemory *ion;
        TaLogDump *taLogDump;
        QseeParams customizedParams;
    } QSEE_CONTEXT;

    static gf_error_t sendHmacKey(CaEntry *pEntry);

    CaEntry::CaEntry() : mContext(nullptr)
    {
    }

    CaEntry::~CaEntry()
    {
        if (nullptr != mContext)
        {
            delete static_cast<QSEE_CONTEXT*>(mContext);
        }
        mContext = nullptr;
    }

    gf_error_t CaEntry::startTap()
    {
        gf_error_t err = GF_ERROR_OPEN_TA_FAILED;

        int32_t ret = 0;
        FUNC_ENTER();
        if (nullptr != mContext)
        {
            delete static_cast<QSEE_CONTEXT*>(mContext);
            mContext = nullptr;
        }
        mContext = new QSEE_CONTEXT();
        memset(mContext, 0, sizeof(QSEE_CONTEXT));
        QSEE_CONTEXT* qcontext = static_cast<QSEE_CONTEXT*>(mContext);
        initCustomizedParams(&qcontext->customizedParams);
        for (uint32_t i = 0; i < qcontext->customizedParams.goodixTaPathCount; i++)
        {
            for (uint32_t j = 0; j < OPLUS_TA_LOAD_RETRY_TIMES; j++) {
                ret = QSEECom_start_app(&qcontext->taHandle, qcontext->customizedParams.goodixTaNamePaths[i], qcontext->customizedParams.goodixTaName,
                    BASE_SHARED_BUFFER_LEN);
                if (ret == 0) {
                    break;
                }
                usleep(100 * 1000);
                LOG_E(LOG_TAG, "[%s] Load ta: %s failed %d times, ret=%d, errno=%d.", __func__,
                    qcontext->customizedParams.goodixTaName, j, ret, errno);
            }
            if (ret == 0)
            {
                err = GF_SUCCESS;
                break;
            }
        }

        if (err == GF_SUCCESS)
        {
            qcontext->ion = new IonMemory();
            qcontext->taLogDump = new TaLogDump();
            LOG_D(LOG_TAG, "[%s] Load ta: %s succeeded.", __func__, qcontext->customizedParams.goodixTaName);
            sendHmacKey(this);
#ifdef SUPPORT_DSP
            if (mDspSupport) {
                err = qcontext->ion->carveoutIonInit();
            }
#endif  // SUPPORT_DSP
        }
        else
        {
            LOG_E(LOG_TAG, "[%s] Load ta: %s failed, ret=%d, errno=%d.", __func__,
                    qcontext->customizedParams.goodixTaName, ret, errno);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CaEntry::shutdownTap()
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        QSEE_CONTEXT* qcontext = static_cast<QSEE_CONTEXT*>(mContext);
        if (nullptr != qcontext && qcontext->ion != nullptr)
        {
            delete qcontext->ion;
            qcontext->ion = nullptr;
        }
        if (nullptr != qcontext && qcontext->taLogDump != nullptr)
        {
            delete qcontext->taLogDump;
            qcontext->taLogDump = nullptr;
        }

        if (nullptr != qcontext && qcontext->taHandle != nullptr)
        {
            int32_t ret = QSEECom_shutdown_app(&qcontext->taHandle);
            if (ret)
            {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] Unload: %s failed, ret=%d, errno=%d.", __func__,
                        qcontext->customizedParams.goodixTaName, ret, errno);
            }
            else
            {
                qcontext->taHandle = nullptr;
                LOG_D(LOG_TAG, "[%s] Unload: %s succeeded.", __func__, qcontext->customizedParams.goodixTaName);
            }
        }
        else
        {
            LOG_D(LOG_TAG, "[%s] qcontext->gpTaHandle is already nullptr.", __func__);
        }

        if (nullptr != qcontext)
        {
            delete qcontext;
        }
        mContext = nullptr;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CaEntry::sendCommand(void *cmd, uint32_t len)
    {
        gf_error_t err = GF_SUCCESS;
        QSEE_CONTEXT* qcontext = static_cast<QSEE_CONTEXT*>(mContext);
        BUFFER_INFO *bufferInfo = nullptr;

        struct QSEECom_ion_fd_info ionFdInfo;
        memset(&ionFdInfo, 0, sizeof(QSEECom_ion_fd_info));

        QSEE_CMD_T *qseeCmd = nullptr;
        int32_t qseeCmdLen = sizeof(QSEE_CMD_T);
        gf_cmd_header_t *header = nullptr;
        int32_t ret = 0;
        struct timeval time;
        memset(&time, 0, sizeof(timeval));
        struct tm currentTm;
        memset(&currentTm, 0, sizeof(tm));
        uint32_t taLogDumpLevel = 0;
        uint32_t taLogBufferLen = 0;
        int32_t carveout_ion_handle_fd = 0;
        int32_t carveout_ion_handle_len = 0;
        FUNC_ENTER();

        do
        {
            if (nullptr == qcontext || qcontext->taHandle == nullptr)
            {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] QSEE app: %s is not started.", __func__, qcontext->customizedParams.goodixTaName);
                break;
            }
            if (nullptr == qcontext || qcontext->ion == nullptr)
            {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] ION memory environment is not initialized.", __func__);
                break;
            }

            taLogDumpLevel = qcontext->taLogDump->getDumpLevel();
            if (taLogDumpLevel > 0)
            {
                taLogBufferLen = TA_LOG_DUMP_BUFFER_SIZE;
            }

            qcontext->requestToken++;
            qseeCmd = (QSEE_CMD_T*) qcontext->taHandle->ion_sbuffer;
            memset(qseeCmd, 0, sizeof(QSEE_CMD_T));
            header = (gf_cmd_header_t *) cmd;
            LOG_D(LOG_TAG, "[%s] request token: %d, target: %d, cmd id: %d", __func__,
                qcontext->requestToken, header->target, header->cmd_id);
            if ((UINT32_MAX - taLogBufferLen) >= len)
            {
                bufferInfo = qcontext->ion->allocate(len + taLogBufferLen);
            }

            if (bufferInfo == nullptr)
            {
                LOG_E(LOG_TAG, "[%s] allocate ion memory failed. ", __func__);
                err = GF_ERROR_GENERIC;
                break;
            }

            qcontext->ion->getCarveoutIonHandle(&carveout_ion_handle_fd, &carveout_ion_handle_len);
            LOG_D(LOG_TAG, "[%s] carveout_ion_handle_fd = %d,carveout_ion_handle_len = %d ", __func__,
                  carveout_ion_handle_fd, carveout_ion_handle_len);
            // 'cmd_addr' will be initialized by qsee TEE, it's value is
            // virtual address of buffer allocated from 'qcontext->gpIon'
            ionFdInfo.data[0].fd = bufferInfo->bufferFd;
            ionFdInfo.data[0].cmd_buf_offset = offsetof(QSEE_CMD_T, cmd_addr);
            ionFdInfo.data[1].fd = carveout_ion_handle_fd;
            ionFdInfo.data[1].cmd_buf_offset = offsetof(QSEE_CMD_T, carveout_data);
            memcpy(bufferInfo->buffer, cmd, len);

            qseeCmd->req_token = qcontext->requestToken;
            qseeCmd->cmd_len = len;
            qseeCmd->ta_log_dump_level = taLogDumpLevel;
            gettimeofday(&time, nullptr);
            localtime_r(&time.tv_sec, &currentTm);
            qseeCmd->timestamp = (((currentTm.tm_hour * 60 + currentTm.tm_min) * 60)
                + currentTm.tm_sec) * 1000 + time.tv_usec / 1000;
            qseeCmd->carveout_len = carveout_ion_handle_len;
            // response is not used
            atrace_begin(ATRACE_TAG_HAL, "QSEECom_send_modified_cmd");
            ret = QSEECom_send_modified_cmd(qcontext->taHandle, qseeCmd,
                QSEECOM_ALIGN(qseeCmdLen), qseeCmd, QSEECOM_ALIGN(1), &ionFdInfo);
            atrace_end(ATRACE_TAG_HAL);
            if (ret != 0)
            {
                LOG_E(LOG_TAG, "[%s] QSEECom_send_modified_cmd failed, ret=%d",
                    __func__, ret);
                err = GF_ERROR_TA_DEAD;
                break;
            }

            err = qseeCmd->status;
            memcpy(cmd, bufferInfo->buffer, len);
            qcontext->taLogDump->printLog((uint8_t *) bufferInfo->buffer + len);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] QSEE TEE execute command failed.", __func__);
                break;
            }
        }
        while (0);

        if (bufferInfo != nullptr)
        {
            qcontext->ion->free(bufferInfo);
            bufferInfo = nullptr;
        }
        FUNC_EXIT(err);
        return err;
    }

    static gf_error_t fetchHmacKeyFromKeyMaster(char* keymasterTaName, char* keymasterTaPath, uint8_t *buffer, int32_t len)
    {
        gf_error_t err = GF_SUCCESS;
        int32_t ret = 0;
        struct QSEECom_handle *keymasterHandle = nullptr;
        KM_GET_AUTH_TOKEN_REQ_T req;
        memset(&req, 0, sizeof(KM_GET_AUTH_TOKEN_REQ_T));
        KM_GET_AUTH_TOKEN_RSP_T *rsp = (KM_GET_AUTH_TOKEN_RSP_T *) buffer;
        FUNC_ENTER();

        do
        {
            ret = QSEECom_start_app(&keymasterHandle, keymasterTaPath, keymasterTaName,
                BASE_SHARED_BUFFER_LEN);
            if (ret)
            {
                LOG_E(LOG_TAG, "[%s] start keymaster ta failed, ret=%d, errno=%d", __func__,
                    ret, errno);
                err = GF_ERROR_GENERIC;
                break;
            }
            LOG_D(LOG_TAG, "[%s] start keymaster ta succeeded, handle=%p", __func__,
                keymasterHandle);

            req.cmd_id = KEYMASTER_GET_AUTH_TOKEN_KEY;
            req.auth_type = HW_AUTH_FINGERPRINT;

            ret = QSEECom_send_cmd(keymasterHandle, &req, sizeof(req), buffer, len);
            if (ret || rsp->status)
            {
                LOG_E(LOG_TAG, "[%s] fetch hmac key from keymaster failed, ret=%d, response status:0x%x",
                    __func__, ret, rsp->status);
                err = GF_ERROR_GENERIC;
                break;
            }
            LOG_D(LOG_TAG, "[%s] fetch hmac key from keymaster succeeded, key offset:%d, key len:%d",
                __func__, rsp->auth_token_key_offset, rsp->auth_token_key_len);
        }
        while (0);

        if (keymasterHandle != nullptr)
        {
            ret = QSEECom_shutdown_app(&keymasterHandle);
            if (ret)
            {
                LOG_E(LOG_TAG, "[%s] unload %s ta failed, ret=%d, errno=%d", __func__,
                        keymasterTaName, ret, errno);
                err = GF_ERROR_GENERIC;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    static gf_error_t sendHmacKey(CaEntry *pEntry)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        QSEE_CONTEXT* qcontext = static_cast<QSEE_CONTEXT*>(pEntry->mContext);
        gf_set_hmac_key_t cmd;
        memset(&cmd, 0, sizeof(gf_set_hmac_key_t));
        cmd.header.target = GF_TARGET_BIO;
        cmd.header.cmd_id = GF_CMD_AUTH_SET_HMAC_KEY;
        err = fetchHmacKeyFromKeyMaster(qcontext->customizedParams.keymasterTaName, qcontext->customizedParams.keymasterTaPath, cmd.i_hmac_key, QSEE_HMAC_KEY_MAX_LEN);
        if (err == GF_SUCCESS)
        {
            err = pEntry->sendCommand(&cmd, sizeof(cmd));
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] send command to ta failed.", __func__);
            }
            else if (cmd.header.result != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] set hmac key to ta failed.", __func__);
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    void CaEntry::setDeviceHandle(int32_t fd)
    {
        UNUSED_VAR(fd);
    }

    void CaEntry::getCarveoutFdInfo(int32_t *fd, int32_t *len)
    {
        QSEE_CONTEXT *qcontext = static_cast<QSEE_CONTEXT *>(mContext);
        qcontext->ion->getCarveoutIonHandle(fd, len);
    }

}  // namespace goodix
