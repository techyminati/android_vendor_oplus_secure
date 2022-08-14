/*
* Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TA_FIDO_AUTH_INTERFACE_H
#define FPC_TA_FIDO_AUTH_INTERFACE_H

#include "fpc_ta_interface.h"
#include "fpc_fido_auth_types.h"

typedef struct {
    fpc_ta_cmd_header_t header;
    uint8_t fido_auth_input_1[FPC_FIDO_AUTH_INPUT_1_MAX_LEN];
    uint32_t fido_auth_input_1_len;
    uint8_t fido_auth_input_2[FPC_FIDO_AUTH_INPUT_2_MAX_LEN];
    uint32_t fido_auth_input_2_len;
    char sec_app_name[FPC_FIDO_SEC_APP_NAME_MAX_LEN];
    uint32_t sec_app_name_size;
    uint64_t user_id;
    uint64_t user_entity_id;
    uint32_t encapsulated_result_length;
    uint8_t encapsulated_result[FPC_FIDO_UVT_MAX_LEN];
} fpc_ta_fido_auth_get_result_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint8_t nonce[FPC_FIDO_AUTH_NONCE_MAX_SIZE];
    uint32_t nonce_size;
} fpc_ta_fido_auth_set_nonce_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint64_t user_id;
    uint32_t result;
} fpc_ta_fido_auth_is_user_id_valid_cmd_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_fido_auth_set_nonce_cmd_t set_nonce;
    fpc_ta_fido_auth_get_result_cmd_t get_result;
    fpc_ta_fido_auth_is_user_id_valid_cmd_t user_id_valid;
} fpc_ta_fido_auth_command_t;

enum {
    FPC_TA_FIDO_AUTH_SET_NONCE        = 1,
    FPC_TA_FIDO_AUTH_GET_RESULT       = 2,
    FPC_TA_FIDO_AUTH_IS_USER_ID_VALID = 3,
};
#endif // FPC_TA_FIDO_AUTH_INTERFACE_H
