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


#include <hardware/hw_auth_token.h>

#include "fpc_tee_hw_auth.h"
#include "fpc_ta_hw_auth_interface.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_targets.h"
#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_error_str.h"

int fpc_tee_init_hw_auth(fpc_tee_t* tee)
{
    int status = 0;

    fpc_ta_hw_auth_command_t* command = tee->shared_buffer->addr;
    command->header.command = FPC_TA_HW_AUTH_SET_SHARED_KEY;
    command->header.target = TARGET_FPC_TA_HW_AUTH;
    command->set_shared_key.size = 0;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status)
    {
        LOGE("%s, xfer failed with error: %s", __func__, fpc_error_str(status));
        goto out;
    }

out:
    return status;
}
