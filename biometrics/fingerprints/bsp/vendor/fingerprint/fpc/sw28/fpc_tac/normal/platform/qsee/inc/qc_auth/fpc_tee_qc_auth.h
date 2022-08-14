/*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TEE_QC_AUTH_H
#define FPC_TEE_QC_AUTH_H

#include <stdint.h>
#include <errno.h>

#include "fpc_tee.h"
#include <auth.h>

// QSEE 4: QC_UVT_MAX_LEN = 5120. QSEE 3: QC_USER_VERIFICATION_TOKEN_LEN = 4272
#define UVT_MAX_LEN 5120

typedef struct {
    int result;
    uint64_t user_id;
    uint64_t user_entity_id;
    uint32_t encapsulated_result_length;
    uint8_t encapsulated_result[UVT_MAX_LEN];
} fpc_tee_qc_auth_data_t;

int fpc_tee_set_qc_auth_nonce(fpc_tee_t* tee,
        const uint8_t* nonce,
        const uint32_t nonce_size);

int fpc_tee_get_qc_auth_result(fpc_tee_t* tee,
        const uint8_t* nonce,
        const uint32_t nonce_size,
        char* sec_app_name,
        const uint32_t sec_app_name_size,
        fpc_tee_qc_auth_data_t* qc_auth_data);

int fpc_tee_is_user_valid(fpc_tee_t* tee, uint64_t user_id, uint32_t *result);

#endif // FPC_TEE_QC_AUTH_H
