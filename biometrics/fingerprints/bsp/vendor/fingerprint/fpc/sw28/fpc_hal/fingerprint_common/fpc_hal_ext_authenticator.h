/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_AUTHENTICATOR_H
#define FPC_HAL_EXT_AUTHENTICATOR_H

#include <inttypes.h>
#include <stdbool.h>
#include "fpc_tee_hal.h"

typedef struct {
    int32_t result;
    int64_t user_id;
    int64_t entity_id;
    uint8_t *result_blob;
    uint32_t size_result_blob;
    uint32_t finger_id;
} fpc_verify_user_data_t;

typedef void (*fpc_verify_user_cb_t)(void *ctx, fpc_verify_user_data_t verify_user_data);
typedef void (*fpc_verify_user_help_cb_t)(void *ctx, int help_code);

typedef struct fpc_authenticator fpc_authenticator_t;

struct fpc_authenticator {
    /**
     * NOTE! Data for fido_auth_input_x and sec_app_name are copied but ctx and callbacks need
     *       to be kept by the client after this call has been made.
     */
    int (*verify_user)(fpc_authenticator_t *self,
                       const uint8_t *fido_auth_input_1, uint32_t fido_auth_input_1_len,
                       const uint8_t *fido_auth_input_2, uint32_t fido_auth_input_2_len,
                       const char *sec_app_name, uint32_t size_sec_app_name, void *ctx,
                       fpc_verify_user_cb_t verify_user_cb,
                       fpc_verify_user_help_cb_t verify_user_help_cb);

    void (*cancel)(fpc_authenticator_t *self);

    int (*is_user_valid)(fpc_authenticator_t *self, uint64_t user_id, bool *is_user_valid);

};

fpc_authenticator_t *fpc_authenticator_new(fpc_hal_common_t *hal);

void fpc_authenticator_destroy(fpc_authenticator_t *self);

#endif // FPC_HAL_EXT_AUTHENTICATOR_H
