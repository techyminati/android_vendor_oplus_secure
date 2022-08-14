/************************************************************************************
 ** File: - fpc\fpc_tac\normal\platform\qsee\src\fpc_tee_hw_auth_qsee.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      fpc tee hw auth api (sw23.2.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,12/08/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 ** 	<author>	     <data>			<desc>
 **    Ziqing.guo      2017/12/08       create file, add to load different keymaster according to porject
 ************************************************************************************/

#include <string.h>
#include <stdlib.h>

#include <errno.h>

#include <hardware/hw_auth_token.h>

#include <QSEEComAPI.h>

#include "fpc_ta_hw_auth_interface.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_targets.h"
#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_types.h"

#define KEYMASTER_UTILS_CMD_ID  0x200UL

#ifndef OPLUS_CONFIG_ANDROID_O_NEW_DEVICE
    #define FPC_TA_KEYMASTER_NAME "keymaster"
#else
    #define FPC_TA_KEYMASTER_NAME "keymaster64"
#endif

typedef enum {
    KEYMASTER_GET_AUTH_TOKEN_KEY = (KEYMASTER_UTILS_CMD_ID + 5UL),
    KEYMASTER_LAST_CMD_ENTRY = (int)0xFFFFFFFFULL
} keymaster_cmd_t;

typedef struct _km_get_auth_token_req_t {
    keymaster_cmd_t cmd_id;
    hw_authenticator_type_t auth_type;
}__attribute__ ((packed)) km_get_auth_token_req_t;

typedef struct _km_get_auth_token_rsp_t {
    int status;
    uint32_t auth_token_key_offset;
    uint32_t auth_token_key_len;
}__attribute__ ((packed)) km_get_auth_token_rsp_t;

static void qsee_km_release_encapsulated_key(uint8_t* encapsulated_key)
{
    free(encapsulated_key);
}

static int qsee_km_get_encapsulated_key(uint8_t** encapsulated_key,
                              uint32_t* size_encapsulated_key)
{
    LOGD("%s begin", __func__);

    int retval = 0;
    *encapsulated_key = NULL;
    *size_encapsulated_key = 0;
    const uint32_t shared_buffer_size = 1024;

    struct QSEECom_handle* keymaster_handle = NULL;
    retval = QSEECom_start_app(&keymaster_handle,
                                  FPC_TA_KEYMASTER_APP_PATH,
                                  FPC_TA_KEYMASTER_NAME,
                                  shared_buffer_size);


    if (retval) {
        LOGE("%s start_app failed %i", __func__, retval);
        goto out;
    }

    km_get_auth_token_req_t* command = (km_get_auth_token_req_t*)
            keymaster_handle->ion_sbuffer;

    uint32_t command_length = QSEECOM_ALIGN(sizeof(km_get_auth_token_req_t));
    km_get_auth_token_rsp_t* response = (km_get_auth_token_rsp_t*)
            (keymaster_handle->ion_sbuffer + command_length);

    command->cmd_id = KEYMASTER_GET_AUTH_TOKEN_KEY;
    command->auth_type = HW_AUTH_FINGERPRINT;

    uint32_t response_length = shared_buffer_size - command_length;

    retval = QSEECom_send_cmd(keymaster_handle,
                              command,
                              command_length,
                              response,
                              response_length);

    if (retval) {
        LOGE("%s failed to send key auth token key command: %d",
             __func__, retval);
        goto out;
    }

    if (response->status) {
        LOGE("%s KEYMASTER_GET_AUTH_TOKEN_KEY returned status=%d",
             __func__, response->status);
        retval = -FPC_ERROR_PARAMETER;
        goto out;
    }

    *encapsulated_key = malloc(response->auth_token_key_len);
    if (*encapsulated_key == NULL) {
        retval = -FPC_ERROR_PARAMETER;
        goto out;
    }

    *size_encapsulated_key = response->auth_token_key_len;

    memcpy(*encapsulated_key,
                   ((uint8_t*) response) + response->auth_token_key_offset,
                   *size_encapsulated_key);

out:
    if (keymaster_handle) {
        QSEECom_shutdown_app(&keymaster_handle);
    }

    return retval;
}


int fpc_tee_init_hw_auth(fpc_tee_t* tee)
{
    uint8_t* key = NULL;
    uint32_t size_key;
    int status = qsee_km_get_encapsulated_key(&key, &size_key);
    if (status) {
        return status;
    }

    const uint32_t size = sizeof(fpc_ta_byte_array_msg_t) + size_key;

    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, size);

    if (!shared_buffer) {
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    fpc_ta_hw_auth_command_t* command = shared_buffer->addr;
    command->header.command = FPC_TA_HW_AUTH_SET_SHARED_KEY;
    command->header.target = TARGET_FPC_TA_HW_AUTH;
    command->set_shared_key.size = size_key;
    memcpy(command->set_shared_key.array, key, size_key);

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto free;
    }


free:
    fpc_tac_free_shared(shared_buffer);
out:
    qsee_km_release_encapsulated_key(key);
    return status;
}
