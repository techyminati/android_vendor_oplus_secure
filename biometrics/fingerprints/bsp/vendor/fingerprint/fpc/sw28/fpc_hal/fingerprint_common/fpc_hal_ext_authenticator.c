/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include "fpc_hal_ext_authenticator.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "container_of.h"
#include "fpc_types.h"
#include "fpc_tee_fido_auth.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_hal.h"
#include "fpc_hal_private.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define FPC_FIDO_AUTH_HAL_EXT_CODE_OK (0)
#define FPC_FIDO_AUTH_HAL_EXT_CODE_ERR (1)
#define FPC_FIDO_AUTH_HAL_EXT_CODE_CANCEL (2)

typedef struct {
    fpc_authenticator_t authenticator;

    void *verify_user_cb_ctx;
    fpc_verify_user_cb_t verify_user_cb;
    fpc_verify_user_help_cb_t verify_user_help_cb;

    /*
     * fido_auth_input_1 is:
     * . used as 'nonce', for FPC_CONFIG_FIDO_AUTH_VER_GENERIC
     * . used as 'fido challenge', for FPC_CONFIG_FIDO_AUTH_VER_GMRZ
     */
    uint8_t fido_auth_input_1[FPC_FIDO_AUTH_INPUT_1_MAX_LEN];
    uint32_t fido_auth_input_1_len;

    /*
     * fido_auth_input_2 is:
     * . not used, for FPC_CONFIG_FIDO_AUTH_VER_GENERIC
     * . used as 'fido aaid', for FPC_CONFIG_FIDO_AUTH_VER_GMRZ
     */
    uint8_t fido_auth_input_2[FPC_FIDO_AUTH_INPUT_2_MAX_LEN];
    uint32_t fido_auth_input_2_len;

    char sec_app_name[FPC_FIDO_SEC_APP_NAME_MAX_LEN];
    uint32_t size_sec_app_name;

    fpc_hal_common_t *hal;
} authenticator_module_t;

static void authenticator_reset(authenticator_module_t *module)
{
    memset(module->fido_auth_input_1, 0, sizeof(module->fido_auth_input_1));
    module->fido_auth_input_1_len = 0;

    memset(module->fido_auth_input_2, 0, sizeof(module->fido_auth_input_2));
    module->fido_auth_input_2_len = 0;

    memset(module->sec_app_name, 0, sizeof(module->sec_app_name));
    module->size_sec_app_name = 0;

    module->verify_user_cb_ctx = NULL;
    module->verify_user_cb = NULL;
    module->verify_user_help_cb = NULL;
}

static void do_verify_user(void *data)
{
    LOGD("%s", __func__);
    int status = FPC_ERROR_NONE;
    authenticator_module_t *module = (authenticator_module_t *) data;
    fpc_tee_fido_auth_result_t *fido_auth_result;
    fpc_verify_user_data_t verify_user_data;

    fido_auth_result = malloc(sizeof(fpc_tee_fido_auth_result_t));

    if (!fido_auth_result) {
        LOGE("%s malloc failed", __func__);
        status = -ENOMEM;
        goto out;
    }

    memset(&verify_user_data, 0, sizeof(verify_user_data));
    memset(fido_auth_result, 0, sizeof(fpc_tee_fido_auth_result_t));

    status = fpc_tee_fido_auth_set_nonce(module->hal->tee_handle,
                                         module->fido_auth_input_1, module->fido_auth_input_1_len);
    if (status) {
        LOGE("%s fpc_tee_fido_auth_set_nonce returned status code: %i", __func__, status);
        goto out;
    }

    bool capture_ok = false;
    while (!capture_ok) {
        status = fpc_tee_capture_image(module->hal->sensor, 0, 0);

        switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
        case FPC_ERROR_NONE:
            capture_ok = true;
            break;
        case FPC_STATUS_FINGER_LOST:
            if (module->verify_user_help_cb) {
                module->verify_user_help_cb(module->verify_user_cb_ctx,
                                            HAL_COMPAT_ACQUIRED_TOO_FAST);
            }
            break;
        case FPC_STATUS_BAD_QUALITY:
            if (module->verify_user_help_cb) {
                module->verify_user_help_cb(module->verify_user_cb_ctx,
                                            HAL_COMPAT_ACQUIRED_INSUFFICIENT);
            }
            break;
        case FPC_STATUS_WAIT_TIME:
            LOGD("%s FPC_STATUS_WAIT_TIME", __func__);
            if (module->verify_user_help_cb) {
                module->verify_user_help_cb(module->verify_user_cb_ctx,
                                            HAL_COMPAT_ACQUIRED_INSUFFICIENT);
            }
            goto out;
        case -FPC_ERROR_IO:
            goto out;
        case -FPC_ERROR_CANCELLED:
            goto out;
        default:
            goto out;
        }
    }

    uint32_t id, update = 0;

    status = fpc_tee_identify(module->hal->bio, &id);
    if (status) {
        goto out;
    }

    status = fpc_tee_update_template(module->hal->bio, &update);
    if (status) {
        goto out;
    }

    if (update != 0) {
        fpc_tee_store_template_db(module->hal->bio, module->hal->current_db_file);
    }

    if (id != 0) {
        status = fpc_tee_fido_auth_get_result(module->hal->tee_handle,
                                              module->fido_auth_input_1,
                                              module->fido_auth_input_1_len,
                                              module->fido_auth_input_2,
                                              module->fido_auth_input_2_len,
                                              module->sec_app_name,
                                              module->size_sec_app_name,
                                              fido_auth_result);
        if (status) {
            LOGE("%s fpc_tee_fido_auth_get_result returned status code: %i", __func__, status);
            goto out;
        }

        verify_user_data.result = fido_auth_result->result;
        verify_user_data.user_id = fido_auth_result->user_id;
        verify_user_data.entity_id = fido_auth_result->user_entity_id;
        verify_user_data.size_result_blob = fido_auth_result->encapsulated_result_length;
        verify_user_data.result_blob = fido_auth_result->encapsulated_result;
        verify_user_data.finger_id = id;
    } else {
        // Asm map FPC_AUTH_CODE_ERR to NO_MATCH
        verify_user_data.result = FPC_FIDO_AUTH_HAL_EXT_CODE_ERR;
    }

out:
    if (status == -EINTR) {
        verify_user_data.result = FPC_FIDO_AUTH_HAL_EXT_CODE_CANCEL;
    } else if (status != 0) {
        verify_user_data.result = FPC_FIDO_AUTH_HAL_EXT_CODE_ERR;
    }

    if (module->verify_user_cb) {
        module->verify_user_cb(module->verify_user_cb_ctx, verify_user_data);
    }


    if (fido_auth_result) {
        free(fido_auth_result);
    }

    authenticator_reset(module);
}

static int verify_user(fpc_authenticator_t *self,
                       const uint8_t *fido_auth_input_1, uint32_t fido_auth_input_1_len,
                       const uint8_t *fido_auth_input_2, uint32_t fido_auth_input_2_len,
                       const char *sec_app_name, uint32_t size_sec_app_name,
                       void *ctx, fpc_verify_user_cb_t verify_user_cb,
                       fpc_verify_user_help_cb_t verify_user_help_cb)
{
    LOGD("%s", __func__);

    int status = 0;
    authenticator_module_t *module = (authenticator_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);

    // Check parameters
    if (fido_auth_input_1_len > FPC_FIDO_AUTH_INPUT_1_MAX_LEN) {
        LOGE("%s failed fido_auth_input_1_len is %d", __func__, fido_auth_input_1_len);
        status = -EINVAL;
        goto out;
    }

    if (fido_auth_input_2_len > FPC_FIDO_AUTH_INPUT_2_MAX_LEN) {
        LOGE("%s failed fido_auth_input_2_len is %d", __func__, fido_auth_input_2_len);
        status = -EINVAL;
        goto out;
    }

    if (size_sec_app_name > FPC_FIDO_SEC_APP_NAME_MAX_LEN) {
        LOGE("%s failed size_sec_app_name is %d", __func__, size_sec_app_name);
        status = -EINVAL;
        goto out;
    }

    fingerprint_hal_goto_idle(module->hal);

    module->fido_auth_input_1_len = fido_auth_input_1_len;
    module->fido_auth_input_2_len = fido_auth_input_2_len;
    memcpy(module->fido_auth_input_1, fido_auth_input_1, fido_auth_input_1_len);
    memcpy(module->fido_auth_input_2, fido_auth_input_2, fido_auth_input_2_len);

    // Copy sec_app_name and append null terminator
    memcpy(module->sec_app_name, sec_app_name, size_sec_app_name);
    module->sec_app_name[size_sec_app_name] = '\0';
    module->size_sec_app_name = size_sec_app_name;

    module->verify_user_cb_ctx = ctx;
    module->verify_user_cb = verify_user_cb;
    module->verify_user_help_cb = verify_user_help_cb;

    LOGD("%s setting fido_auth_input_1 0x%2x with size %i and app: %s with size %i",
         __func__, module->fido_auth_input_1[0], module->fido_auth_input_1_len,
         module->sec_app_name, size_sec_app_name);

    fingerprint_hal_do_async_work(module->hal, do_verify_user, module, FPC_TASK_HAL_EXT);

out:
    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

static void cancel(fpc_authenticator_t *self)
{
    LOGD("%s", __func__);
    authenticator_module_t *module = (authenticator_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
}

static int is_user_valid(fpc_authenticator_t *self,
                         uint64_t user_id,
                         bool *is_user_valid)
{
    LOGD("%s", __func__);
    int status = 0;
    uint32_t result = 0;
    authenticator_module_t *module = (authenticator_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    status = fpc_tee_fido_auth_is_user_valid(module->hal->tee_handle, user_id, &result);
    if (status) {
        LOGE("%s fpc_tee_fido_auth_is_user_valid returned error: %i", __func__, status);
        goto unlock;
    }
    *is_user_valid = (result != 0);

unlock:
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

fpc_authenticator_t *fpc_authenticator_new(fpc_hal_common_t *hal)
{
    authenticator_module_t *module = malloc(sizeof(authenticator_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(authenticator_module_t));
    module->hal = hal;
    module->authenticator.verify_user = verify_user;
    module->authenticator.cancel = cancel;
    module->authenticator.is_user_valid = is_user_valid;

    return (fpc_authenticator_t *) module;
}

void fpc_authenticator_destroy(fpc_authenticator_t *self)
{
    if (!self) {
        return;
    }

    free(self);
}

