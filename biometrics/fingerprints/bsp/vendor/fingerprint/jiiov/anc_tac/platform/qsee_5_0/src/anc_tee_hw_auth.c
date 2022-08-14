#define LOG_TAG "[ANC_TAC][Token]"

#include "anc_tee_hw_auth.h"



#include <hardware/hw_auth_token.h>
#include <QSEEComAPI.h>


#include "anc_error.h"
#include "anc_log.h"
#include "anc_memory_wrapper.h"


// same as QSEECOM_ALIGN, but fixed implicit conversion warning by gcc
#ifdef ANC_EXPLICIT_CONVERSION
#ifdef ANC_QSEECOM_ALIGN
#undef ANC_QSEECOM_ALIGN
#endif
#define ANC_QSEECOM_ALIGN(x)	\
	((x + QSEECOM_ALIGN_MASK) & (~(unsigned int)QSEECOM_ALIGN_MASK))
#else
#define ANC_QSEECOM_ALIGN QSEECOM_ALIGN
#endif

#define KEYMASTER_UTILS_CMD_ID  0x200UL

typedef enum {
    KEYMASTER_GET_AUTH_TOKEN_KEY = (KEYMASTER_UTILS_CMD_ID + 5UL),
    KEYMASTER_LAST_CMD_ENTRY = (int)0xFFFFFFFFULL
} ANC_KEYMASTER_COMMAND_TYPE;

typedef struct {
    ANC_KEYMASTER_COMMAND_TYPE cmd_id;
    hw_authenticator_type_t auth_type;
}__attribute__ ((packed)) AncKeymasterGetAuthTokenRequest;

typedef struct {
    int status;
    uint32_t auth_token_key_offset;
    uint32_t auth_token_key_len;
}__attribute__ ((packed)) AncKeymasterGetAuthTokenRespond;

static void ReleaseHmacKeyFromQseeKeymasterTa(uint8_t *p_hmac_key) {
    if (NULL != p_hmac_key) {
        AncFree(p_hmac_key);
    }
}

static ANC_RETURN_TYPE GetHmacKeyFromQseeKeymasterTa(uint8_t **p_hmac_key,
                              uint32_t *p_hmac_key_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int ret = 0;
    *p_hmac_key = NULL;
    *p_hmac_key_size = 0;
    const uint32_t shared_buffer_size = 1024;

    struct QSEECom_handle *p_keymaster_handle = NULL;
    ret = QSEECom_start_app(&p_keymaster_handle,
                                  ANC_KEYMASTER_TA_PATH,
                                  ANC_KEYMASTER_TA_NAME,
                                  shared_buffer_size);
    if (0 != ret) {
        ANC_LOGE("start %s TA failed, return value:%d", ANC_KEYMASTER_TA_NAME, ret);
        ret_val = ANC_FAIL;
        goto out;
    }

    AncKeymasterGetAuthTokenRequest *p_command = (AncKeymasterGetAuthTokenRequest*)
            p_keymaster_handle->ion_sbuffer;

    uint32_t command_length = ANC_QSEECOM_ALIGN(sizeof(AncKeymasterGetAuthTokenRequest));
    AncKeymasterGetAuthTokenRespond *p_response = (AncKeymasterGetAuthTokenRespond*)
            (p_keymaster_handle->ion_sbuffer + command_length);

    p_command->cmd_id = KEYMASTER_GET_AUTH_TOKEN_KEY;
    p_command->auth_type = HW_AUTH_FINGERPRINT;

    uint32_t response_length = shared_buffer_size - command_length;

    ret = QSEECom_send_cmd(p_keymaster_handle,
                              p_command,
                              command_length,
                              p_response,
                              response_length);
    if (0 != ret) {
        ANC_LOGE("failed to send key auth token key command, return value:%d", ret);
        ret_val = ANC_FAIL;
        goto out;
    }

    if (0 != p_response->status) {
        ANC_LOGE("failed to get auth token key from keymaster TA, return status:%d", p_response->status);
        ret_val = ANC_FAIL;
        goto out;
    }

    *p_hmac_key = AncMalloc(p_response->auth_token_key_len);
    if (NULL == *p_hmac_key) {
        ANC_LOGE("failed to allocate space, need size:%d", p_response->auth_token_key_len);
        ret_val = ANC_FAIL;
        goto out;
    }

    *p_hmac_key_size = p_response->auth_token_key_len;

    AncMemcpy(*p_hmac_key,
                   ((uint8_t*)p_response) + p_response->auth_token_key_offset,
                   *p_hmac_key_size);

out:
    if (NULL != p_keymaster_handle) {
        ret = QSEECom_shutdown_app(&p_keymaster_handle);
        if (0 != ret) {
            ANC_LOGE("shutdown app failed, ret value = %d\n", ret);
            ret_val = ANC_FAIL;
        }
    }

    return ret_val;
}


ANC_RETURN_TYPE GetHmacKeyFromKeymasterTa(uint8_t *p_hmac_key,  uint32_t *p_hmac_key_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if ((NULL == p_hmac_key) || (NULL == p_hmac_key_size)) {
        ANC_LOGE("get hmac key, key:%p, key size:%p\n", p_hmac_key , p_hmac_key_size);
        return ANC_FAIL;
    }

    uint8_t *p_key = NULL;
    uint32_t key_size = 0;
    ret_val = GetHmacKeyFromQseeKeymasterTa(&p_key, &key_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("qsee fail to get hmac key, return value:%d", ret_val);
        return ret_val;
    }
    if (key_size <= 0) {
        ANC_LOGE("qsee fail to get hmac key, key size:%d", key_size);
        return ANC_FAIL;
    }
#ifdef ANC_DEBUG
    ANC_LOGD("key:%p, key size:%d\n", p_key, key_size);
    ANC_LOGD("key[%d] = %x\n",(key_size-1), p_key[(key_size-1)]);
#endif

    *p_hmac_key_size = key_size;
    AncMemcpy(p_hmac_key, p_key, key_size);

    ReleaseHmacKeyFromQseeKeymasterTa(p_key);

    return ret_val;
}

