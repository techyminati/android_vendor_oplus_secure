/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Ca/TrustonicCa.cpp
 **
 ** Description:
 **      TrustonicCa for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][TrustonicCa]"

#ifdef TEE_TBASE

#include "TrustonicCa.h"
#include "FpCommon.h"
#include "HalLog.h"
#include <cutils/trace.h>
#include <errno.h>
#include <hardware/hw_auth_token.h>
#include <string.h>
#include <sys/time.h>
#include <tee_client_api.h>
#include <time.h>
#include <unistd.h>
#include "HalContext.h"

#define FP_CMD_TEEC_PARAM_TYPES (TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE))
#define TA_UUID {0x05060000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
#define FP_OPERATION_ID 1


#define TA_SPI_UUID                                        \
    {                                                      \
        0x05050000, 0x0000, 0x0000, {                      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
        }                                                  \
    }
#define TA_GF_UUID                                         \
    {                                                      \
        0x05060000, 0x0000, 0x0000, {                      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
        }                                                  \
    }
#define TA_JV_UUID                                         \
    {                                                      \
        0x05060000, 0x0000, 0x0000, {                      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
        }                                                  \
    }
#define TA_SILEAD_UUID                                     \
    {                                                      \
        0x05061000, 0x0000, 0x0000, {                      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
        }                                                  \
    }
#define TA_EGIS_UUID                                       \
    {                                                      \
        0x05062000, 0x0000, 0x0000, {                      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
        }                                                  \
    }

typedef struct fp_ta_info {
    fp_ta_name_t index;
    TEEC_UUID name;
} fp_ta_info_t;

fp_ta_info_t taNames[] = {
    {FP_SPI_TA, TA_SPI_UUID},       // 0
    {FP_GOODIX_TA, TA_GF_UUID},     // 1
    {FP_JV0301_TA, TA_JV_UUID},     // 2
    {FP_JV0302_TA, TA_JV_UUID},     // 3
    {FP_SILEAD_TA, TA_SILEAD_UUID}, // 4
    // {FP_EGIS_TA, TA_EGIS_UUID},     // 5
};

namespace android {
static TEEC_Context* fp_context = NULL;
static TEEC_Session* fp_session = NULL;
static const TEEC_UUID UUID     = TA_UUID;
static fp_return_type_t testcommand(TrustonicCa *pEntry);
static pthread_mutex_t cmd_lock;

TrustonicCa::TrustonicCa() : mContext(nullptr) {
    LOG_I(LOG_TAG, "[%s] start ", __func__);
}

TrustonicCa::~TrustonicCa() {
    if (nullptr != mContext) {
        delete mContext;
    }
    mContext = nullptr;
}

fp_return_type_t TrustonicCa::startTa(fp_ta_name_t name) {
    fp_return_type_t err = FP_SUCCESS;
    TEEC_Result result = TEEC_SUCCESS;
    TEEC_Operation operation;
    FUNC_ENTER();
    if (name > sizeof(taNames)/sizeof(taNames[0]) - 1) {
        LOG_E(LOG_TAG, "[%s] %d not match tanames", __func__, (int)name);
        return FP_SUCCESS;
    }
    pthread_mutex_init(&cmd_lock, NULL);

    fp_context = (TEEC_Context*)malloc(sizeof(TEEC_Context));
    if (NULL == fp_context) {
        LOG_E(LOG_TAG, "[%s] malloc fp_context failed", __func__);
        err = FP_ERROR;
        goto exit;
    }
    memset(fp_context, 0, sizeof(TEEC_Context));
    result = TEEC_InitializeContext(NULL, fp_context);
    if (TEEC_SUCCESS != result) {
        LOG_E(LOG_TAG, "[%s] TEEC_InitializeContext failed result = 0x%x",
              __func__, result);
        err = FP_ERROR;
        goto exit;
    }
    fp_session = (TEEC_Session*)malloc(sizeof(TEEC_Session));
    if (NULL == fp_session) {
        LOG_E(LOG_TAG, "[%s] malloc fp_session failed", __func__);
        err = FP_ERROR;
        goto exit;
    }
    memset(fp_session, 0, sizeof(TEEC_Session));
    memset(&operation, 0, sizeof(TEEC_Operation));
    result = TEEC_OpenSession(fp_context, fp_session, &taNames[name].name,
                              TEEC_LOGIN_PUBLIC, NULL, &operation, NULL);
    if (TEEC_SUCCESS != result) {
        LOG_E(LOG_TAG, "[%s] TEEC_OpenSession failed result = 0x%x",
              __func__, result);
        err = FP_ERROR;
        goto exit;
    } else {
        testcommand(this);
    }
    FUNC_EXIT(err);
    return err;

exit:
    if (NULL != fp_session) {
        TEEC_CloseSession(fp_session);
        free(fp_session);
        fp_session = NULL;
    }
    if (NULL != fp_context) {
        TEEC_FinalizeContext(fp_context);
        free(fp_context);
        fp_context = NULL;
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t TrustonicCa::closeTa() {
    fp_return_type_t err = FP_SUCCESS;

    FUNC_ENTER();
    if (NULL != fp_session) {
        TEEC_CloseSession(fp_session);
        free(fp_session);
        fp_session = NULL;
    }
    if (NULL != fp_context) {
        TEEC_FinalizeContext(fp_context);
        free(fp_context);
        fp_context = NULL;
    }
    pthread_mutex_destroy(&cmd_lock);

    FUNC_EXIT(err);
    return err;
}

fp_return_type_t TrustonicCa::sendCommand(void* cmd, uint32_t len) {
    fp_return_type_t err       = FP_SUCCESS;
    uint32_t ret               = TEEC_SUCCESS;
    uint32_t a = 0;
    TEEC_Operation operation   = {0};
    uint32_t taLogBufferLen    = 0;
    void* cmdBuf               = NULL;
    fp_ta_cmd_header_t* header = NULL;
    FUNC_ENTER();
    pthread_mutex_lock(&cmd_lock);
    HalContext::getInstance()->mDevice->controlSpiClock(DEVICE_ENABLE_SPI);
    /*if (taLogDumpLevel > 0)
     {
         taLogBufferLen = TA_LOG_DUMP_BUFFER_SIZE;
    }*/
    cmdBuf = malloc(len + taLogBufferLen);
    if (NULL == cmdBuf) {
        LOG_E(LOG_TAG, "[%s] allocate cmdBuf memory failed. ", __func__);
        err = FP_ERROR;
        goto exit;
    }
    memcpy(cmdBuf, cmd, len);
    operation.params[0].tmpref.buffer = cmdBuf;
    operation.params[0].tmpref.size   = len + taLogBufferLen;
    header                            = (fp_ta_cmd_header_t*)cmd;
    LOG_D(LOG_TAG, "[%s] module_id: %d, cmd id: %d", __func__, header->module_id,
          header->cmd_id);
    operation.paramTypes = FP_CMD_TEEC_PARAM_TYPES;
    ret = TEEC_InvokeCommand(fp_session, FP_OPERATION_ID, &operation, NULL);
    if (TEEC_SUCCESS == ret) {
        LOG_E(LOG_TAG, "[%s] command success. ", __func__);
        a = operation.params[1].value.b;
        LOG_E(LOG_TAG, "[%s] ta send value.b = %d. ", __func__, a);
    } else {
        err = FP_ERR_TA_DEAD;
        goto exit;
    }

exit:
    memcpy(cmd, cmdBuf, len);
    if (NULL != cmdBuf) {
        free(cmdBuf);
    }
    HalContext::getInstance()->mDevice->controlSpiClock(DEVICE_DISABLE_SPI);

    FUNC_EXIT(err);
    pthread_mutex_unlock(&cmd_lock);
    return err;
}

static fp_return_type_t testcommand(TrustonicCa *pEntry) {
    fp_return_type_t err = FP_SUCCESS;
    fp_ta_cmd_header_t header;
    header.module_id = 1001;
    header.cmd_id = 2;
    err = pEntry->sendCommand(&header, sizeof(header));
    if (err != FP_SUCCESS) {
        LOG_E(LOG_TAG, "[%s] send command to ta failed. ", __func__);
    } else {
        LOG_E(LOG_TAG, "[%s] send command to ta success. ", __func__);
    }
    FUNC_EXIT(err);
    return err;
    }

fp_return_type_t TrustonicCa::sendHmacKeyToFpta() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    //send hmackey to fp_ta
    fp_set_hmackey_t cmd;
    memset(&cmd, 0, sizeof(fp_set_hmackey_t));
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_SET_HMACKEY;
    err = sendCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}


}  // namespace android

#else

#include "TrustonicCa.h"

namespace android {

TrustonicCa::TrustonicCa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
}

TrustonicCa::~TrustonicCa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
}

fp_return_type_t TrustonicCa::startTa(fp_ta_name_t name) {
    (void)name;
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t TrustonicCa::closeTa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t TrustonicCa::sendCommand(void* cmd, uint32_t size)  {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    (void)cmd;
    (void)size;
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t TrustonicCa::sendHmacKeyToFpta() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

}
#endif
