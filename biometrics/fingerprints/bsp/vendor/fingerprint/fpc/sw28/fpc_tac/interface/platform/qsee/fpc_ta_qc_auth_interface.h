/*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TA_QC_AUTH_INTERFACE_H
#define FPC_TA_QC_AUTH_INTERFACE_H

#include "fpc_ta_interface.h"
#include <auth.h>

// QSEE 4: QC_UVT_MAX_LEN = 5120. QSEE 3: QC_USER_VERIFICATION_TOKEN_LEN = 4272
#define UVT_MAX_LEN 5120

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    uint8_t nonce[QC_AUTH_NONCE];
    char sec_app_name[QC_SEC_APP_NAME_LEN];
    uint64_t user_id;
    uint64_t user_entity_id;
    uint32_t encapsulated_result_length;
    uint8_t encapsulated_result[UVT_MAX_LEN];
} fpc_ta_qc_auth_get_auth_data_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    uint64_t user_id;
    uint32_t result;
} fpc_ta_qc_auth_is_user_id_valid_cmd_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_byte_array_msg_t set_nonce;
    fpc_ta_qc_auth_get_auth_data_cmd_t get_auth_data;
    fpc_ta_qc_auth_is_user_id_valid_cmd_t user_id_valid;
} fpc_ta_qc_auth_command_t;

enum {
    FPC_TA_QC_AUTH_SET_NONCE        = 1,
    FPC_TA_QC_AUTH_GET_RESULT       = 2,
    FPC_TA_QC_AUTH_IS_USER_ID_VALID = 3,
};
#endif // FPC_TA_QC_AUTH_INTERFACE_H
