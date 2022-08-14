/*
* Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#include <string.h>

#include "fpc_tac.h"
#include "fpc_log.h"

#include "fpc_tee_internal.h"

#include "fpc_tee_fido_auth.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_fido_auth_interface.h"
#include "fpc_types.h"
#include "fpc_ta_targets.h"

int fpc_tee_fido_auth_set_nonce(fpc_tee_t *tee,
                                const uint8_t *nonce,
                                const uint32_t nonce_size)
{
    int status = 0;

    LOGD("%s", __func__);

    if (nonce == NULL) {
        LOGE("%s: failed nonce is NULL", __func__);
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    if (nonce_size == 0 || nonce_size > FPC_FIDO_AUTH_NONCE_MAX_SIZE) {
        LOGE("%s: failed nonce_size is %d", __func__, nonce_size);
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    LOGD("%s nonce 0x%2x with size %i", __func__, nonce[0], nonce_size);

    size_t size = sizeof(fpc_ta_fido_auth_command_t);

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        status = -FPC_ERROR_MEMORY;
        goto out;
    }

    fpc_ta_fido_auth_command_t *command = (fpc_ta_fido_auth_command_t *) shared_buffer->addr;

    command->header.command        = FPC_TA_FIDO_AUTH_SET_NONCE;
    command->header.target         = TARGET_FPC_TA_FIDO_AUTH;
    command->set_nonce.nonce_size  = nonce_size;

    memcpy(command->set_nonce.nonce, nonce, nonce_size);

    status = fpc_tac_transfer(tee->tac, shared_buffer);

    fpc_tac_free_shared(shared_buffer);
out:
    return status;
}

/*
 * fido_auth_input_1 is 1st input parameter for fido_auth:
 * . used as 'nonce', for FPC_CONFIG_FIDO_AUTH_VER_GENERIC
 * . used as 'fido chanllenge', for FPC_CONFIG_FIDO_AUTH_VER_GMRZ
 * fido_auth_input_2 is 2nd input parameter for fido_auth:
 * . not used, for FPC_CONFIG_FIDO_AUTH_VER_GENERIC
 * . used as 'fido aaid', for FPC_CONFIG_FIDO_AUTH_VER_GMRZ
 */
int fpc_tee_fido_auth_get_result(fpc_tee_t *tee,
                                 const uint8_t *fido_auth_input_1,
                                 const uint32_t fido_auth_input_1_len,
                                 const uint8_t *fido_auth_input_2,
                                 const uint32_t fido_auth_input_2_len,
                                 char *sec_app_name,
                                 const uint32_t sec_app_name_size,
                                 fpc_tee_fido_auth_result_t *fido_auth_result)
{
    int status = 0;

    LOGD("%s", __func__);

    if (fido_auth_input_1_len > FPC_FIDO_AUTH_INPUT_1_MAX_LEN) {
        LOGE("%s: failed fido_auth_input_1_len is %d", __func__, fido_auth_input_1_len);
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    if (fido_auth_input_2_len > FPC_FIDO_AUTH_INPUT_2_MAX_LEN) {
        LOGE("%s: failed fido_auth_input_2_len is %d", __func__, fido_auth_input_2_len);
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    if (sec_app_name_size > FPC_FIDO_SEC_APP_NAME_MAX_LEN) {
        LOGE("%s: failed sec_app_name_size %d", __func__, sec_app_name_size);
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    if (fido_auth_result == NULL) {
        LOGE("%s: failed fido_auth_data is NULL", __func__);
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    size_t size = sizeof(fpc_ta_fido_auth_command_t);

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        status = -FPC_ERROR_MEMORY;
        goto out;
    }

    fpc_ta_fido_auth_command_t *command = (fpc_ta_fido_auth_command_t *) shared_buffer->addr;

    command->header.command     = FPC_TA_FIDO_AUTH_GET_RESULT;
    command->header.target      = TARGET_FPC_TA_FIDO_AUTH;

    memcpy(command->get_result.fido_auth_input_1, fido_auth_input_1, fido_auth_input_1_len);
    command->get_result.fido_auth_input_1_len = fido_auth_input_1_len;

    memcpy(command->get_result.fido_auth_input_2, fido_auth_input_2, fido_auth_input_2_len);
    command->get_result.fido_auth_input_2_len = fido_auth_input_2_len;

    memcpy(command->get_result.sec_app_name, sec_app_name, sec_app_name_size);
    command->get_result.sec_app_name_size = sec_app_name_size;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto free;
    }

    fido_auth_result->user_id = command->get_result.user_id;
    fido_auth_result->user_entity_id = command->get_result.user_entity_id;
    fido_auth_result->encapsulated_result_length = command->get_result.encapsulated_result_length;

    memcpy(fido_auth_result->encapsulated_result,
           command->get_result.encapsulated_result,
           command->get_result.encapsulated_result_length);

free:
    fpc_tac_free_shared(shared_buffer);

out:
    return status;
}

int fpc_tee_fido_auth_is_user_valid(fpc_tee_t *tee, uint64_t user_id, uint32_t *result)
{
    LOGD("%s", __func__);

    int status;

    if (result == NULL) {
        LOGE("%s: failed result is NULL", __func__);
        status = -FPC_ERROR_PARAMETER;
        goto out;
    }

    size_t size = sizeof(fpc_ta_fido_auth_command_t);

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        status = -FPC_ERROR_MEMORY;
        goto out;
    }

    fpc_ta_fido_auth_command_t *command = (fpc_ta_fido_auth_command_t *) shared_buffer->addr;

    command->header.command        = FPC_TA_FIDO_AUTH_IS_USER_ID_VALID;
    command->header.target         = TARGET_FPC_TA_FIDO_AUTH;
    command->user_id_valid.user_id = user_id;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto free;
    }

    *result = command->user_id_valid.result;

free:
    fpc_tac_free_shared(shared_buffer);

out:
    return status;
}

