/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL layer simulator define type
 * History:
 * Version: 1.0
 */

#ifndef _GF_HAL_SIMULATOR_TYPE_H_
#define _GF_HAL_SIMULATOR_TYPE_H_

#include "gf_fingerprint.h"
#include "gf_common.h"
#include "gf_dump_data_buffer.h"

#define SIMULATOR_FINGER_DOWN 1000
#define SIMULATOR_FINGER_UP 1001

typedef struct gf_simulator_fingerprint_pre_enroll
{
    uint64_t challenge;
} gf_simulator_fingerprint_pre_enroll_t;

typedef struct gf_simulator_reply
{
    int32_t err_code;
    union
    {
        gf_fingerprint_enroll_t enroll;
        gf_fingerprint_authenticated_t authenticated;
        gf_fingerprint_authenticated_fido_t authenticated_fido;
        gf_enumerate_t enumerated;
        gf_fingerprint_removed_t removed;
        gf_get_auth_id_t authenticator_id;
        gf_fingerprint_enumerated_t enumerated_with_callback;
        gf_memory_info_t memory_info;
        gf_fingerprint_acquired_t acquired;
        gf_simulator_fingerprint_pre_enroll_t pre_enroll;
        dump_data_buffer_header_t dump_buf;
    };
} gf_simulator_reply_t;

typedef enum
{
    IRQ_TYPE = 1,
    NAV_ORIENTATION,
    ENROLL,
    AUTHENTICATE,
    REMOVE,
    ENUMERATE,
    SET_ACTIVE_GROUP,
    GET_AUTH_ID,
    ENUMERATE_WITH_CALLBACK,
    REMOVE_WITH_CALLBACK,
    MEMORY_CHECK,
    CANCEL,
    FINGER_QUICK_UP_OR_MISTAKE_TOUCH,
    AUTHENTICATE_FIDO,
    CLEAR,
    PRE_ENROLL,
    POST_ENROLL,
    SET_DUMP_CONFIG,
    DUMP_CMD,
    RESET_APK_ENABLED_DUMP,
    SIMULATE_DIRTY_DATA,
    DUMP_DEVICE_INFO,
    START_NAVIGATE,
} gf_data_type_t;

typedef enum
{
    SR_SHELL = 2000,
    SR_CLIENT,
} gf_client_type_t;

#endif  // _GF_HAL_SIMULATOR_TYPE_H_
