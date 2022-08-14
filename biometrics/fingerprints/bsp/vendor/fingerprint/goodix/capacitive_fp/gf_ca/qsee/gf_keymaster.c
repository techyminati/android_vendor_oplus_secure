/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_keymaster file
 * History:
 * Version: 1.0
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <hardware/hw_auth_token.h>
#include "gf_keymaster.h"
#include "QSEEComAPI.h"
#include "qs_log.h"

#define LOG_TAG "[gf_keymaster]"

/**
 * Commands supported
 * Since Gatekeeper is supported int he same TA, if adding or
 * changing this, keep in sync in Gatekeeper(gk_main.h).
 */
#define KEYMASTER_CMD_ID_OLD  0UL
#define KEYMASTER_CMD_ID  0x100UL
#define KEYMASTER_UTILS_CMD_ID  0x200UL
#define GK_CMD_ID  0x1000UL

typedef enum
{
    /*
     * List the commands supportedin by the hardware.
     */
    KEYMASTER_GET_SUPPORTED_ALGORITHMS = (KEYMASTER_CMD_ID + 1UL),
    KEYMASTER_GET_SUPPORTED_BLOCK_MODES = (KEYMASTER_CMD_ID + 2UL),  // NOLINT
    KEYMASTER_GET_SUPPORTED_PADDING_MODES = (KEYMASTER_CMD_ID + 3UL),  // NOLINT
    KEYMASTER_GET_SUPPORTED_DIGESTS = (KEYMASTER_CMD_ID + 4UL),  // NOLINT
    KEYMASTER_GET_SUPPORTED_IMPORT_FORMATS = (KEYMASTER_CMD_ID + 5UL),  // NOLINT
    KEYMASTER_GET_SUPPORTED_EXPORT_FORMATS = (KEYMASTER_CMD_ID + 6UL),  // NOLINT
    KEYMASTER_ADD_RNG_ENTROPY = (KEYMASTER_CMD_ID + 7UL),  // NOLINT
    KEYMASTER_GENERATE_KEY = (KEYMASTER_CMD_ID + 8UL),  // NOLINT
    KEYMASTER_GET_KEY_CHARACTERISTICS = (KEYMASTER_CMD_ID + 9UL),  // NOLINT
    KEYMASTER_RESCOPE = (KEYMASTER_CMD_ID + 10UL),  // NOLINT
    KEYMASTER_IMPORT_KEY = (KEYMASTER_CMD_ID + 11UL),  // NOLINT
    KEYMASTER_EXPORT_KEY = (KEYMASTER_CMD_ID + 12UL),  // NOLINT
    KEYMASTER_DELETE_KEY = (KEYMASTER_CMD_ID + 13UL),  // NOLINT
    KEYMASTER_DELETE_ALL_KEYS = (KEYMASTER_CMD_ID + 14UL),  // NOLINT
    KEYMASTER_BEGIN = (KEYMASTER_CMD_ID + 15UL),  // NOLINT
    KEYMASTER_UPDATE = (KEYMASTER_CMD_ID + 17UL),  // NOLINT
    KEYMASTER_FINISH = (KEYMASTER_CMD_ID + 18UL),  // NOLINT
    KEYMASTER_ABORT = (KEYMASTER_CMD_ID + 19UL),  // NOLINT

    KEYMASTER_GET_VERSION = (KEYMASTER_UTILS_CMD_ID + 0UL),  // NOLINT
    KEYMASTER_SET_ROT = (KEYMASTER_UTILS_CMD_ID + 1UL),  // NOLINT
    KEYMASTER_READ_KM_DEVICE_STATE = (KEYMASTER_UTILS_CMD_ID + 2UL),  // NOLINT
    KEYMASTER_WRITE_KM_DEVICE_STATE = (KEYMASTER_UTILS_CMD_ID + 3UL),  // NOLINT
    KEYMASTER_MILESTONE_CALL = (KEYMASTER_UTILS_CMD_ID + 4UL),  // NOLINT
    KEYMASTER_GET_AUTH_TOKEN_KEY = (KEYMASTER_UTILS_CMD_ID + 5UL),  // NOLINT

    GK_ENROLL = (GK_CMD_ID + 1UL),  // NOLINT
    GK_VERIFY = (GK_CMD_ID + 2UL),  // NOLINT
    GK_DELETE_USER = (GK_CMD_ID + 3UL),  // NOLINT
    GK_DELETE_ALL_USERS = (GK_CMD_ID + 4UL),  // NOLINT

    KEYMASTER_GENERATE_KEY_OLD = (KEYMASTER_CMD_ID_OLD + 1UL),  // NOLINT
    KEYMASTER_IMPORT_KEY_OLD = (KEYMASTER_CMD_ID_OLD + 2UL),  // NOLINT
    KEYMASTER_SIGN_DATA_OLD = (KEYMASTER_CMD_ID_OLD + 3UL),  // NOLINT
    KEYMASTER_VERIFY_DATA_OLD = (KEYMASTER_CMD_ID_OLD + 4UL),  // NOLINT

    KEYMASTER_LAST_CMD_ENTRY = (int32_t) 0xFFFFFFFFULL
} keymaster_cmd_t;

typedef struct
{
    keymaster_cmd_t cmd_id;
    hw_authenticator_type_t auth_type;
} __attribute__((packed)) km_get_auth_token_req_t;

typedef struct
{
    int32_t status;
    uint32_t auth_token_key_offset;
    uint32_t auth_token_key_len;
} __attribute__((packed)) km_get_auth_token_rsp_t;

/**
 * Function: get_key_from_keymaster
 * Description: get key from keymaster
 * Input: rsp_buf, buf_len
 * Output: none
 * Return:
 * GF_SUCCESS  if succeed to get key from keymaster
 * others      if fail to get key from keymaster
 */
gf_error_t get_key_from_keymaster(uint8_t *rsp_buf, int32_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    struct QSEECom_handle *g_keymaster_handle = NULL;
    char keymaster_name[32] = "keymaster64";
    uint32_t comm_buf_size = 4096;
    km_get_auth_token_req_t token_req;
    LOG_D("[%s] enter", __func__);

    do
    {
        int32_t ret = 0;
        km_get_auth_token_rsp_t *token_rsp;

        if (NULL == rsp_buf)
        {
            LOG_E("[%s] args is null and invalid", __func__);
            err = GF_ERROR_HAL_GENERAL_ERROR;
            break;
        }

        // start keymaster
        ret = QSEECom_start_app(&g_keymaster_handle, "/firmware/image",
                                keymaster_name, comm_buf_size);

        if (ret)
        {
            LOG_E("[%s] keymaster start fail,ret=%d, errno=%d", __func__, ret, errno);
            err = GF_ERROR_HAL_GENERAL_ERROR;
            break;
        }
        else
        {
            if (NULL == g_keymaster_handle)
            {
                LOG_E("[%s] keymaster start fail,km handle=%p, errno=%d", __func__,
                      g_keymaster_handle, errno);
                err = GF_ERROR_HAL_GENERAL_ERROR;
                break;
            }
            else
            {
                LOG_D("[%s] keymaster start success,km handle=%p, errno=%d", __func__,
                      g_keymaster_handle, errno);
            }
        }

        // get key from keymaster
        memset(&token_req, 0, sizeof(token_req));
        token_req.cmd_id = KEYMASTER_GET_AUTH_TOKEN_KEY;
        token_req.auth_type = HW_AUTH_FINGERPRINT;
        ret = QSEECom_send_cmd(g_keymaster_handle, &token_req,
                               sizeof(token_req), rsp_buf, buf_len);
        token_rsp = (km_get_auth_token_rsp_t *) rsp_buf;
        LOG_D("[%s] km get key ret:%d, token_rsp:%p, rsp.status:0x%x, rsp.offset:%d, rsp.len:%d",
              __func__, ret, token_rsp, token_rsp->status,
              token_rsp->auth_token_key_offset,
              token_rsp->auth_token_key_len);

        if (ret || token_rsp->status)
        {
            LOG_E("[%s] get key failed.", __func__);
            err = GF_ERROR_HAL_GENERAL_ERROR;
        }
        else
        {
            LOG_D("[%s] get key success.", __func__);
        }

        // stop keymaster
        ret = QSEECom_shutdown_app(&g_keymaster_handle);

        if (ret)
        {
            LOG_E("[%s] Unload %s failed: ret=%d, errno=%d", __func__, keymaster_name, ret,
                  errno);
            err = GF_ERROR_HAL_GENERAL_ERROR;
        }
        else
        {
            g_keymaster_handle = NULL;
            LOG_D("[%s] Unload %s succeed.", __func__, keymaster_name);
        }
    } while (0);  // do...

    LOG_D("[%s] exit", __func__);
    return err;
}

