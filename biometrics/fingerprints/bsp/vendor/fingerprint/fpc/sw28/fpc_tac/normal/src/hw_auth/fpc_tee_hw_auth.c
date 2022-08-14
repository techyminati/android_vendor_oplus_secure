/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <string.h>

#include "fpc_tee_hw_auth.h"
#include "fpc_ta_hw_auth_interface.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_targets.h"
#include "fpc_tee.h"
#include "fpc_log.h"
#include "fpc_types.h"

int fpc_tee_set_auth_challenge(fpc_tee_t* tee, uint64_t challenge)
{
    LOGD("%s", __func__);
    fpc_ta_hw_auth_command_t* command =
            (fpc_ta_hw_auth_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_HW_AUTH_SET_AUTH_CHALLENGE;
    command->header.target = TARGET_FPC_TA_HW_AUTH;
    command->set_auth_challenge.challenge = challenge;
    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

int fpc_tee_get_enrol_challenge(fpc_tee_t* tee, uint64_t* challenge)
{
    LOGD("%s", __func__);
    fpc_ta_hw_auth_command_t* command =
            (fpc_ta_hw_auth_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_HW_AUTH_GET_ENROL_CHALLENGE;
    command->header.target = TARGET_FPC_TA_HW_AUTH;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    *challenge = command->get_enrol_challenge.challenge;
    return status;
}

int fpc_tee_authorize_enrol(fpc_tee_t* tee, const uint8_t* token,
                            uint32_t size_token)
{
    LOGD("%s", __func__);
    const uint32_t size = sizeof(fpc_ta_byte_array_msg_t) + size_token;

    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, size);

    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    int status;
    fpc_ta_hw_auth_command_t* command = shared_buffer->addr;
    command->header.command = FPC_TA_HW_AUTH_AUTHORIZE_ENROL;
    command->header.target = TARGET_FPC_TA_HW_AUTH;
    command->authorize_enrol.size = size_token;
    memcpy(command->authorize_enrol.array, token, size_token);

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    fpc_tac_free_shared(shared_buffer);
    return status;
}

int fpc_tee_get_auth_result(fpc_tee_t* tee, uint8_t* token, uint32_t size_token)
{
    LOGD("%s", __func__);
    const uint32_t size = sizeof(fpc_ta_byte_array_msg_t) + size_token;

    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, size);

    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    int status;
    fpc_ta_hw_auth_command_t* command = shared_buffer->addr;
    command->header.command = FPC_TA_HW_AUTH_GET_AUTH_RESULT;
    command->header.target = TARGET_FPC_TA_HW_AUTH;
    command->get_auth_result.size = size_token;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    memcpy(token, command->get_auth_result.array, size_token);
out:
    fpc_tac_free_shared(shared_buffer);
    return status;
}

int fpc_tee_set_token_validation_enable(fpc_tee_t *tee, bool enabled)
{
    LOGD("%s", __func__);
    fpc_ta_hw_auth_command_t *command = (fpc_ta_hw_auth_command_t *) tee->shared_buffer->addr;

    command->header.command = FPC_TA_HW_AUTH_SET_TOKEN_VALIDATION;
    command->header.target  = TARGET_FPC_TA_HW_AUTH;
    command->token_val.enabled = enabled;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    return status;
}

