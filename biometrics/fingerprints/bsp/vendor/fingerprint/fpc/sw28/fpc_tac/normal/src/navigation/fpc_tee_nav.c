/************************************************************************************
 ** File: - SDM660.LA.1.0\android\vendor\qcom\proprietary\securemsm\fpc_tac\normal\src\navigation
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE NAV TAC for FPC (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,14/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>     <data>            <desc>
 **  Ziqing.guo   2017/10/14        create the file, add for reporint homekey
 **  Ziqing.guo   2017/10/21        customization for homekey
 ************************************************************************************/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_navigation_interface.h"
#include "fpc_types.h"
#include "fpc_tee_nav.h"
#include "fpc_ta_targets.h"
#include "fpc_nav_types.h"

int _nav_send_command(fpc_tee_t* tee, int32_t cmd_id) {
    fpc_ta_nav_void_cmd_t* command =
        (fpc_ta_nav_void_cmd_t*) tee->shared_buffer->addr;
    command->header.command = cmd_id;
    command->header.target = TARGET_FPC_TA_NAVIGATION;
    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }
    return status;//command->response;
}

int nav_wait_finger_down(fpc_tee_t* tee, fpc_irq_t* irq) {
    fpc_ta_navigation_finger_det_cmd_t* command =
        (fpc_ta_navigation_finger_det_cmd_t*) tee->shared_buffer->addr;
    command->header.command = FPC_TA_NAVIGATION_FINGER_DETECT;
    command->finger_up = 0;
    int status = _nav_send_command(tee, FPC_TA_NAVIGATION_FINGER_DETECT);
    if (status) {
        return status;
    }
    status = fpc_irq_wait(irq, 1);
    if (status) {
        return status;
    }
    return 0;
}

int nav_wait_finger_up(fpc_tee_t* tee, fpc_tee_sensor_t** sensor, fpc_irq_t* irq) {
    (void)tee;
    (void)irq;
    int status = fpc_tee_wait_finger_lost(sensor);
    if (status) {
        return status;
    }
    return 0;
}

int fpc_tee_nav_poll_data(fpc_tee_t* tee, fpc_nav_data_t* data) {
    fpc_ta_nav_poll_data_cmd_t* command =
        (fpc_ta_nav_poll_data_cmd_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_POLL_DATA;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    data->finger_down = command->finger_down;
    data->nav_event = command->nav_event;
    data->request = command->request;
    data->force = command->force;

    return status;
}

int fpc_tee_nav_set_config(fpc_tee_t* tee, const fpc_nav_config_t* config)
{
    fpc_ta_nav_config_cmd_t* command =
            (fpc_ta_nav_config_cmd_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_SET_CONFIG;
    command->header.target = TARGET_FPC_TA_NAVIGATION;
    command->config = *config;

    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

int fpc_tee_nav_get_config(fpc_tee_t* tee, fpc_nav_config_t* config)
{
    fpc_ta_nav_config_cmd_t* command =
            (fpc_ta_nav_config_cmd_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_GET_CONFIG;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    *config = command->config;

    return status;
}

int fpc_tee_nav_init(fpc_tee_t* tee)
{
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_INIT;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

int fpc_tee_nav_exit(fpc_tee_t* tee)
{
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_EXIT;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

int fpc_tee_nav_get_debug_buffer_size(fpc_tee_t *tee, uint32_t *debug_buffer_size)
{
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_GET_DEBUG_BUFFER_SIZE;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    *debug_buffer_size = command->get_debug_buffer_size.arg_1;

    return status;
}

int fpc_tee_nav_get_debug_buffer(fpc_tee_t *tee, uint8_t *debug_buffer, uint32_t *debug_buffer_size)
{
    int status = 0;
    fpc_ta_byte_array_msg_t *command = NULL;

    fpc_tac_shared_mem_t *shared_ipc_buffer =
        fpc_tac_alloc_shared(tee->tac, *debug_buffer_size + sizeof(fpc_ta_byte_array_msg_t));

    if (!shared_ipc_buffer) {
        status = -FPC_ERROR_MEMORY;
        LOGE("%s: failed to allocate error buffer ret %d", __func__, status);
        goto exit;
    }

    memset(shared_ipc_buffer->addr, 0, *debug_buffer_size + sizeof(fpc_ta_byte_array_msg_t));

    command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_NAVIGATION;
    command->header.command = FPC_TA_NAVIGATION_GET_DEBUG_BUFFER;
    command->size = *debug_buffer_size;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        goto exit;
    }

    if (command->size > 0 && command->size <= *debug_buffer_size) {
        *debug_buffer_size = command->size;
        memcpy(debug_buffer, command->array, command->size);
        LOGD("%s: got %d size of debug information data", __func__, command->size);
    }
exit:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    return status;
}
