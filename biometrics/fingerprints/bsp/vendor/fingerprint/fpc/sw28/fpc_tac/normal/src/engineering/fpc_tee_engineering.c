/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fpc_tac.h"
#include "fpc_log.h"

#include "fpc_tee_internal.h"
#include "fpc_tee_engineering.h"

#include "fpc_ta_interface.h"
#include "fpc_ta_engineering_interface.h"
#include "fpc_ta_targets.h"
#include "fpc_types.h"

int fpc_tee_debug_get_raw_size(
    fpc_tee_t *tee,
    uint32_t  *raw_size)
{
    LOGD("%s", __func__);

    fpc_ta_engineering_command_t *command =
        (fpc_ta_engineering_command_t*) tee->shared_buffer->addr;
    memset(command, 0, sizeof(*command));

    command->header.target  = TARGET_FPC_TA_ENGINEERING;
    command->header.command = FPC_TA_ENGINEERING_GET_RAW_SIZE;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *raw_size = command->raw_size.size;
    return status;
}

int fpc_tee_debug_retrieve(
    fpc_tee_t* tee,
    uint32_t type,
    void *buffer,
    size_t buffer_size)
{
    LOGD("%s", __func__);
    size_t size = sizeof(fpc_ta_engineering_retrieve_t) + buffer_size;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, (uint32_t)size);
    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_engineering_command_t* command =
        (fpc_ta_engineering_command_t*) shared_buffer->addr;

    command->header.target  = TARGET_FPC_TA_ENGINEERING;
    command->header.command = FPC_TA_ENGINEERING_RETRIEVE;
    command->retrieve.type  = type;
    command->retrieve.size  = (uint32_t)buffer_size;

    int status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    memcpy(buffer, command->retrieve.array, buffer_size);

out:
    fpc_tac_free_shared(shared_buffer);

    return status;
}

int fpc_tee_debug_inject(fpc_tee_t* tee,
                               const void *buffer,
                               size_t buffer_size)
{
    LOGD("%s", __func__);
    size_t size = sizeof(fpc_ta_byte_array_msg_t) + buffer_size;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, (uint32_t)size);
    if(!shared_buffer)
    {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_engineering_command_t* command =
        (fpc_ta_engineering_command_t*) shared_buffer->addr;

    command->header.command     = FPC_TA_ENGINEERING_INJECT_RAW;
    command->header.target      = TARGET_FPC_TA_ENGINEERING;
    command->inject.size = (uint32_t)buffer_size;
    memcpy(command->inject.array, buffer, buffer_size);

    int status = fpc_tac_transfer(tee->tac, shared_buffer);
    fpc_tac_free_shared(shared_buffer);

    return status;
}

int fpc_tee_get_sensor_info(
    fpc_tee_t* tee,
    uint8_t *width,
    uint8_t *height)
{
    LOGD("%s", __func__);

    fpc_ta_engineering_command_t* command =
        (fpc_ta_engineering_command_t*) tee->shared_buffer->addr;
    memset(command, 0, sizeof(*command));

    command->header.command = FPC_TA_ENGINEERING_GET_SENSOR_INFO;
    command->header.target  = TARGET_FPC_TA_ENGINEERING;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *width  = command->sensor_info.width;
    *height = command->sensor_info.height;

    return status;
}
