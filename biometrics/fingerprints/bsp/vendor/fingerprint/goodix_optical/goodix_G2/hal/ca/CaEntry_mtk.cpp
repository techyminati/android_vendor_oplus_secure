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
 **  Dongnan.Wu    2019/02/23        create ca entry file for mtk platform
 ************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>
#include "gf_error.h"
#include "gf_base_types.h"
#include "HalLog.h"
#include "CaEntry.h"
#include "TaLogDump.h"
#ifdef SUPPORT_DSP
#include "IonMemory_mtk.h"
#endif
#define LOG_TAG "[CaEntry]"
#define GF_CMD_TEEC_PARAM_TYPES (TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE))
#define TA_UUID {0x05060000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
#define GF_OPERATION_ID 1


namespace goodix
{
    static TEEC_Context *g_context = NULL;
    static TEEC_Session *g_session = NULL;
    static const TEEC_UUID UUID = TA_UUID;
#ifdef SUPPORT_DSP
    static IonMemory *g_IonMemory = NULL;
#endif
#ifdef SUPPORT_TA_LOG_DUMP_TO_CA
    static TaLogDump *gpTaLogDump = NULL;
#endif  // #ifdef SUPPORT_TA_LOG_DUMP_TO_CA

    CaEntry::CaEntry() :
            mContext(nullptr)
    {
    }

    CaEntry::~CaEntry()
    {
    }

    gf_error_t CaEntry::startTap()
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();

        do
        {
            g_context = (TEEC_Context *)malloc(sizeof(TEEC_Context));
            if (NULL == g_context)
            {
                LOG_E(LOG_TAG, "[%s] malloc g_context failed", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

#ifdef SUPPORT_DSP
            g_IonMemory = new IonMemory();
            g_IonMemory->init();
#endif  // SUPPORT_DSP

            memset(g_context, 0, sizeof(TEEC_Context));

            TEEC_Result result = TEEC_InitializeContext(NULL, g_context);
            if (TEEC_SUCCESS != result)
            {
                LOG_E(LOG_TAG, "[%s] TEEC_InitializeContext failed result = 0x%x", __func__, result);
                err = GF_ERROR_OPEN_TA_FAILED;
                break;
            }

            TEEC_Operation operation;

            g_session = (TEEC_Session *)malloc(sizeof(TEEC_Session));
            if (NULL == g_session)
            {
                LOG_E(LOG_TAG, "[%s] malloc g_session failed", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(g_session, 0, sizeof(TEEC_Session));
            memset(&operation, 0, sizeof(TEEC_Operation));

            result = TEEC_OpenSession(g_context, g_session, &UUID, TEEC_LOGIN_PUBLIC, NULL, &operation, NULL);
            if (TEEC_SUCCESS != result)
            {
                LOG_E(LOG_TAG, "[%s] TEEC_OpenSession failed result = 0x%x", __func__, result);
                err = GF_ERROR_OPEN_TA_FAILED;
                break;
            }
#ifdef SUPPORT_TA_LOG_DUMP_TO_CA
            gpTaLogDump = new TaLogDump();
#endif  // #ifdef SUPPORT_TA_LOG_DUMP_TO_CA
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CaEntry::shutdownTap()
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();

#ifdef SUPPORT_TA_LOG_DUMP_TO_CA
        if (gpTaLogDump != NULL)
        {
            delete gpTaLogDump;
            gpTaLogDump = NULL;
        }
#endif  // #ifdef SUPPORT_TA_LOG_DUMP_TO_CA
        if (NULL != g_session)
        {
            TEEC_CloseSession(g_session);
            free(g_session);
            g_session = NULL;
        }
#ifdef SUPPORT_DSP
        if (NULL != g_IonMemory) {
            delete g_IonMemory;
            g_IonMemory = NULL;
        }
#endif //SUPPORT_DSP

        if (NULL != g_context) {
            TEEC_FinalizeContext(g_context);
            free(g_context);
            g_context = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CaEntry::sendCommand(void* cmd, uint32_t size)
    {
        gf_error_t err = GF_SUCCESS;
        uint32_t ret = TEEC_SUCCESS;
        TEEC_Operation operation = {0};
        gf_cmd_header_t *header = NULL;

#ifdef SUPPORT_TA_LOG_DUMP_TO_CA
        void *cmdBuf = NULL;
        uint32_t taLogDumpLevel = 0;
        uint32_t taLogBufferLen = 0;
#endif  // #ifdef SUPPORT_TA_LOG_DUMP_TO_CA
        FUNC_ENTER();

        do
        {
#ifdef SUPPORT_TA_LOG_DUMP_TO_CA
            taLogDumpLevel = gpTaLogDump->getDumpLevel();
            operation.params[1].value.a = taLogDumpLevel;

            if (taLogDumpLevel > 0)
            {
                taLogBufferLen = TA_LOG_DUMP_BUFFER_SIZE;
            }
            cmdBuf = malloc(size + taLogBufferLen);
            if (NULL == cmdBuf)
            {
                LOG_E(LOG_TAG, "[%s] allocate cmdBuf memory failed. ", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            memcpy(cmdBuf, cmd, size);
            operation.params[0].tmpref.buffer = cmdBuf;
            operation.params[0].tmpref.size = size + taLogBufferLen;
#else  // #ifdef SUPPORT_TA_LOG_DUMP_TO_CA
            operation.params[0].tmpref.buffer = cmd;
            operation.params[0].tmpref.size = size;
#endif  // #ifdef SUPPORT_TA_LOG_DUMP_TO_CA
            header = (gf_cmd_header_t *)cmd;
            LOG_D(LOG_TAG, "[%s] target: %d, cmd id: %d", __func__, header->target, header->cmd_id);
            operation.paramTypes = GF_CMD_TEEC_PARAM_TYPES;

            ret = TEEC_InvokeCommand(g_session, GF_OPERATION_ID, &operation, NULL);

            if (GF_SUCCESS == ret)
            {
                err = (gf_error_t)operation.params[1].value.b;
            }
            else
            {
                LOG_E(LOG_TAG, "[%s] GF_CA ret = 0x%8X", __func__, ret);
                err = GF_ERROR_TA_DEAD;
            }
        } while (0);
#ifdef SUPPORT_TA_LOG_DUMP_TO_CA
        memcpy(cmd, operation.params[0].tmpref.buffer, size);
        gpTaLogDump->printLog((uint8_t *)operation.params[0].tmpref.buffer + size);
        if (NULL != cmdBuf)
        {
            free(cmdBuf);
        }
#endif  // #ifdef SUPPORT_TA_LOG_DUMP_TO_CA
        FUNC_EXIT(err);

        return err;
    }

    void CaEntry::setDeviceHandle(int32_t fd)
    {
        UNUSED_VAR(fd);
    }

    void CaEntry::getCarveoutFdInfo(int32_t *fd, int32_t *len) {
        *fd = 0;
        *len = 0;
    }
}  // namespace goodix
