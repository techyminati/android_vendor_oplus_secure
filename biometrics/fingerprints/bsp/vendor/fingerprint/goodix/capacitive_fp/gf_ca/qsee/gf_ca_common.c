/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_ca common file
 * History:
 * Version: 1.0
 */

#include <stdint.h>
#include <android/log.h>
#include "gf_ca_common.h"
#include "gf_user_type_define.h"
#include "gf_error.h"
#include "gf_ca_entry.h"
#include "gf_whitebox.h"

#define LOG_TAG "[gf_ca_common]"
#define LOG_D(...) (__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOG_E(...) (__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

/**
 * Function: gf_ca_tee_auth
 * Description: Send auth command to ta, receive the seed of whitebox dynamic key.
 * Input: none
 * Output: none
 * Return: error code
 */
gf_error_t gf_ca_tee_auth(void)
{
#ifdef WHITEBOX_TEE_REE
    gf_ca_auth_t auth_seed = { 0 };
    gf_error_t err = gf_ca_invoke_command(GF_USER_OPERATION_ID,
            GF_USER_CMD_INIT_CA_AUTH, &auth_seed, sizeof(auth_seed));
    if (err == GF_SUCCESS)
    {
        gf_ca_auth_t auth_confirm = { 0 };
        whitebox_exec(WB_OP_GEN_AUTH_DATA, auth_seed.data, auth_confirm.data,
                CA_AUTH_DATA_LEN, 0);
        // sizeof(auth_confirm) must equal to AES_BLOCK_ALIGN(sizeof(auth_confirm))
        err = gf_ca_invoke_command(GF_USER_OPERATION_ID, GF_USER_CMD_CONFIRM_CA_AUTH,
                &auth_confirm, sizeof(auth_confirm));
        if (err == GF_SUCCESS)
        {
            whitebox_exec(WB_OP_SET_DYN_KEY, (uint8_t *) auth_confirm.data, NULL,
                    CA_AUTH_DATA_LEN, WB_TRUE);
            LOG_D("[%s] ca auth succeed.", __func__);
        }
        else
        {
            LOG_E("[%s] ca auth confirm failed.", __func__);
        }
    }
    else
    {
        LOG_E("[%s] ca auth init failed.", __func__);
    }
    return err;
#else  // WHITEBOX_TEE_REE
    return GF_SUCCESS;
#endif  // WHITEBOX_TEE_REE
}


