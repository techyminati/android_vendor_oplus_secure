#define LOG_TAG "[ANC_TAC][CA]"

#include "anc_ca.h"
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "anc_error.h"
#include "anc_log.h"
#include "anc_memory_wrapper.h"
#include "tee_client_api.h"

#define ANC_CMD_TEEC_PARAM_TYPES TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_MEMREF_TEMP_INOUT, TEEC_NONE)
#define anc_ta_UUID { 0x03020000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}
#define CA_SHARE_BUFFER_LENGTH (1 * 800 *1024) //800k

static TEEC_Context *g_context = NULL;
static TEEC_Session *g_session = NULL;
static const TEEC_UUID UUID = anc_ta_UUID;
uint8_t g_share_memory[CA_SHARE_BUFFER_LENGTH];

static pthread_mutex_t g_share_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_ca_ta_transmit_mutex = PTHREAD_MUTEX_INITIALIZER;

void AncCaCloseSession(void) {
    ANC_LOGD("[%s] enter", __func__);

    if (NULL != g_session) {
        TEEC_CloseSession(g_session);
        AncFree(g_session);
        g_session = NULL;
    }

    if (NULL != g_context) {
        TEEC_FinalizeContext(g_context);
        AncFree(g_context);
        g_context = NULL;
    }
}

ANC_RETURN_TYPE AncCaLockSharedBuffer(uint8_t *p_function_name) {
    pthread_mutex_lock(&g_share_buffer_mutex);
#ifdef ANC_DEBUG
    ANC_LOGW("anc ca lock shared buffer, %s", p_function_name);
#else
    ANC_UNUSED(p_function_name);
#endif

    return ANC_OK;
}

ANC_RETURN_TYPE AncCaUnlockSharedBuffer(uint8_t *p_function_name) {
    pthread_mutex_unlock(&g_share_buffer_mutex);
#ifdef ANC_DEBUG
    ANC_LOGW("anc ca unlock shared buffer, %s", p_function_name);
#else
    ANC_UNUSED(p_function_name);
#endif

    return ANC_OK;
}


static ANC_RETURN_TYPE AncTransmitModifiedMessage(AncSendCommand *send_cmd, AncSendCommandRespond *p_send_rsp_cmd) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint32_t req_len = 0;
    uint32_t rsp_len = 0;
    uint32_t ret = TEEC_SUCCESS;
    uint32_t operation_id = 0;
    TEEC_Operation operation = { 0 };

    req_len = sizeof(AncSendCommand);
    rsp_len = sizeof(AncSendCommandRespond);
    p_send_rsp_cmd->status = 0;
    operation.paramTypes = ANC_CMD_TEEC_PARAM_TYPES;
    operation.params[0].tmpref.buffer  = send_cmd;
    operation.params[0].tmpref.size  = req_len;
    operation.params[1].tmpref.buffer  = p_send_rsp_cmd;
    operation.params[1].tmpref.size  = rsp_len;
    operation.params[2].tmpref.buffer = g_share_memory;
    operation.params[2].tmpref.size = CA_SHARE_BUFFER_LENGTH;

    ret = TEEC_InvokeCommand(g_session, operation_id, &operation, NULL);
    if( TEEC_SUCCESS == ret) {
        ret_val = ANC_OK;
    } else {
        ANC_LOGE("TEEC_InvokeCommand error:0x%x", ret);
        ret_val = ANC_FAIL_TA_TRANSMIT;
    }

    return ret_val;
}

ANC_RETURN_TYPE AncCaOpenSession(void) {
    ANC_RETURN_TYPE err = ANC_OK;
    ANC_LOGD("[%s] enter", __func__);

    do {
        g_context = (TEEC_Context *) AncMalloc(sizeof(TEEC_Context));
        if (NULL == g_context) {
            ANC_LOGE("[%s], malloc g_context failed", __func__);
            err = ANC_FAIL_MEMORY;
            break;
        }
        memset(g_context, 0, sizeof(TEEC_Context));

        TEEC_Result result = TEEC_InitializeContext(NULL, g_context);
        if (TEEC_SUCCESS != result) {
            ANC_LOGE("[%s], TEEC_InitializeContext failed result = 0x%x", __func__, result);
            err = ANC_FAIL_LOAD_TA;
            break;
        }

        TEEC_Operation operation;
        g_session = (TEEC_Session *) AncMalloc(sizeof(TEEC_Session));
        if (NULL == g_session) {
            ANC_LOGE("[%s], malloc g_session failed", __func__);
            err = ANC_FAIL_MEMORY;
            break;
        }

        memset(g_session, 0, sizeof(TEEC_Session));
        memset(&operation, 0, sizeof(TEEC_Operation));
        result = TEEC_OpenSession(g_context, g_session, &UUID, TEEC_LOGIN_PUBLIC, NULL, &operation, NULL);
        if (TEEC_SUCCESS != result) {
            ANC_LOGE("[%s], TEEC_OpenSession failed result = 0x%x", __func__, result);
            err = ANC_FAIL_LOAD_TA;
            break;
        }
    } while (0);

    if (err != ANC_OK) {
        AncCaCloseSession();
    }

    ANC_LOGD("[%s] exit", __func__);

    return err;
}

ANC_RETURN_TYPE InitAncCa() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (ANC_OK != (ret_val = AncCaOpenSession())) {
        ANC_LOGE("AncCaOpenSession error :%d ", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE AncGetIonSharedBuffer(uint8_t **p_share_buffer, uint32_t *p_share_buffer_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if ((NULL == p_share_buffer) || (NULL == p_share_buffer_size)) {
        ANC_LOGE("share buffer : %p", p_share_buffer);
        ANC_LOGE("share buffer size : %p", p_share_buffer_size);
        return ANC_FAIL;
    }
    *p_share_buffer = (uint8_t *)g_share_memory;
    *p_share_buffer_size = CA_SHARE_BUFFER_LENGTH;    
    AncMemset(*p_share_buffer, 0, *p_share_buffer_size);

    return ret_val;
}

ANC_RETURN_TYPE DeinitAncCa() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncCaCloseSession();
    return ret_val;
}

ANC_RETURN_TYPE AncCaTransmit(AncSendCommand *p_send_cmd, AncSendCommandRespond *p_send_rsp_cmd) {
    return AncCaTransmitModified(p_send_cmd, p_send_rsp_cmd);
}

ANC_RETURN_TYPE AncCaTransmitModified(AncSendCommand *p_send_cmd, AncSendCommandRespond *p_send_rsp_cmd) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    pthread_mutex_lock(&g_ca_ta_transmit_mutex);
    ANC_LOGW("anc ca lock transmit , %s", (uint8_t *)__func__);

    ret_val = AncTransmitModifiedMessage(p_send_cmd, p_send_rsp_cmd);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to transmit message id : %d", p_send_cmd->id);
    }

    pthread_mutex_unlock(&g_ca_ta_transmit_mutex);
    ANC_LOGW("anc ca unlock transmit , %s", (uint8_t *)__func__);

    return ret_val;
}

/*************************test share buffer*************************/
static ANC_RETURN_TYPE AncTestShareBufferTransmit() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSendCommand anc_send_command;
    AncSendCommandRespond anc_send_respond;

    AncMemset(&anc_send_respond, 0, sizeof(anc_send_respond));
    AncMemset(&anc_send_command, 0, sizeof(anc_send_command));
    anc_send_command.size = sizeof(AncSendCommand);
    anc_send_command.id = ANC_CMD_TEST_SHARE_BUFFER;
    ret_val = AncCaTransmitModified(&anc_send_command, &anc_send_respond);

    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send test share buffer command, ret value:%d", ret_val);
    } else {
        ret_val = (ANC_RETURN_TYPE)anc_send_respond.status;
    }

    return ret_val;
}

#define SHARED_BUF_PATTERN_LEN 16
ANC_RETURN_TYPE AncTestSharedBuffer() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;
    char *p_verify = NULL;
    uint32_t i = 0;

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        return ANC_FAIL;
    }

    ANC_LOGD("anc malloc verify");
    p_verify = AncMalloc(share_buffer_size);
    if(NULL == p_verify) {
        ANC_LOGE("fail to anc malloc for verifying\n");
        return ANC_FAIL;
    }

    ANC_LOGD("set verify value");
    for (i=0; i<share_buffer_size; i++) {
        *(p_share_buffer + i) = (char)(i % 255);
        *(p_verify + i) = (char)(i % 255);
    }

    ret_val = AncTestShareBufferTransmit();
    /* Verify the first 16 bytes of response data.
        It should be req_data + 10
    */
    if(ANC_OK == ret_val) {
        for (i=0; i<SHARED_BUF_PATTERN_LEN; i++) {
            *(p_share_buffer + i) = *(p_share_buffer + i)-10;
            if(*(p_share_buffer + i) != (char)(i % 255)) {
                ANC_LOGD("Modified buffer check @ %d = %x", i, *(p_share_buffer + i));
                break;
            }
        }
        if (SHARED_BUF_PATTERN_LEN == i) {
            ANC_LOGD("success to Verify");
        }
    }

    ANC_LOGD("anc free verify");
    if (NULL == p_verify) {
        AncFree(p_verify);
    }

    return ret_val;
}
