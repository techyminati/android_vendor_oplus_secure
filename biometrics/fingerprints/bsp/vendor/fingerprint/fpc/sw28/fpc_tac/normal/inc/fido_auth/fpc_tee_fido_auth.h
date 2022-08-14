/*
* Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TEE_FIDO_AUTH_H
#define FPC_TEE_FIDO_AUTH_H

#include <stdint.h>
#include <errno.h>

#include "fpc_tee.h"
#include "fpc_fido_auth_types.h"

typedef struct {
    int result;
    uint64_t user_id;
    uint64_t user_entity_id;
    uint32_t encapsulated_result_length;
    uint8_t encapsulated_result[FPC_FIDO_UVT_MAX_LEN];
} fpc_tee_fido_auth_result_t;

int fpc_tee_fido_auth_set_nonce(fpc_tee_t *tee,
                                const uint8_t *nonce,
                                const uint32_t nonce_size);

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
                                 fpc_tee_fido_auth_result_t *fido_auth_result);

int fpc_tee_fido_auth_is_user_valid(fpc_tee_t *tee, uint64_t user_id, uint32_t *result);

#endif // FPC_TEE_FIDO_AUTH_H
