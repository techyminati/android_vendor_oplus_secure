/*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
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

#include "fpc_tee_qc_auth.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_qc_auth_interface.h"
#include "fpc_types.h"
#include "fpc_ta_targets.h"

int fpc_tee_set_qc_auth_nonce(fpc_tee_t* tee,
        const uint8_t* nonce,
        const uint32_t nonce_size)
{
    int status = 0;

    LOGD("%s", __func__);

    LOGD("%s setting nonce 0x%2x with size %i",
            __func__, nonce[0], nonce_size);

    if (nonce == NULL) {
        LOGE("%s: input parameter nonce is NULL", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    if (nonce_size > QC_AUTH_NONCE) {
        LOGE("%s: input parameter nonce_size out of range", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    size_t size = sizeof(fpc_ta_qc_auth_command_t) + QC_AUTH_NONCE;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if(!shared_buffer) {
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    fpc_ta_qc_auth_command_t* command =
        (fpc_ta_qc_auth_command_t*) shared_buffer->addr;

    command->header.command     = FPC_TA_QC_AUTH_SET_NONCE;
    command->header.target      = TARGET_FPC_TA_QC_AUTH;
    command->set_nonce.size     = nonce_size;

    memcpy(command->set_nonce.array, nonce, nonce_size);

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto free;
    }

    status = command->set_nonce.response;
    if(status) {
        goto free;
    }

free:
    fpc_tac_free_shared(shared_buffer);
out:
    return status;
}

int fpc_tee_get_qc_auth_result(fpc_tee_t* tee,
        const uint8_t* nonce,
        const uint32_t nonce_size,
        char* sec_app_name,
        const uint32_t sec_app_name_size,
        fpc_tee_qc_auth_data_t* qc_auth_data)
{
    int status = 0;

    LOGD("%s", __func__);

    if (nonce == NULL) {
        LOGE("%s: input parameter nonce is NULL", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    if (nonce_size > QC_AUTH_NONCE) {
        LOGE("%s: input parameter nonce_size out of range", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    if (sec_app_name == NULL) {
        LOGE("%s: input parameter sec_app_name is NULL", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    if (sec_app_name_size > QC_SEC_APP_NAME_LEN) {
        LOGE("%s: input parameter sec_app_name_size out of range", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    if (qc_auth_data == NULL) {
        LOGE("%s: output parameter qc_auth_data is NULL", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    size_t size = sizeof(fpc_ta_qc_auth_command_t);

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if(!shared_buffer) {
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    fpc_ta_qc_auth_command_t* command =
        (fpc_ta_qc_auth_command_t*) shared_buffer->addr;

    command->header.command     = FPC_TA_QC_AUTH_GET_RESULT;
    command->header.target      = TARGET_FPC_TA_QC_AUTH;

    memcpy(command->get_auth_data.nonce, nonce, nonce_size);
    memcpy(command->get_auth_data.sec_app_name, sec_app_name, sec_app_name_size);

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto free;
    }

    status = command->get_auth_data.response;
    if(status) {
        goto free;
    }

    qc_auth_data->user_id = command->get_auth_data.user_id;
    qc_auth_data->user_entity_id =
        command->get_auth_data.user_entity_id;
    qc_auth_data->encapsulated_result_length =
        command->get_auth_data.encapsulated_result_length;

    memcpy(qc_auth_data->encapsulated_result,
           command->get_auth_data.encapsulated_result,
           command->get_auth_data.encapsulated_result_length);

free:
    fpc_tac_free_shared(shared_buffer);

out:
    return status;
}

int fpc_tee_is_user_valid(fpc_tee_t* tee, uint64_t user_id, uint32_t *result)
{
    LOGD("%s", __func__);

    int status;

    if (result == NULL) {
        LOGE("%s: input parameter result is NULL", __func__);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    size_t size = sizeof(fpc_ta_qc_auth_command_t);

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if(!shared_buffer) {
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    fpc_ta_qc_auth_command_t* command =
      (fpc_ta_qc_auth_command_t*) shared_buffer->addr;

    command->header.command        = FPC_TA_QC_AUTH_IS_USER_ID_VALID;
    command->header.target         = TARGET_FPC_TA_QC_AUTH;
    command->user_id_valid.user_id = user_id;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto free;
    }

    status = command->get_auth_data.response;
    if(status) {
        goto free;
    }

    *result = command->user_id_valid.result;

free:
    fpc_tac_free_shared(shared_buffer);

out:
    return status;
}

