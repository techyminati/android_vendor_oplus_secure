/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_BASE_TYPES_H_
#define _GF_BASE_TYPES_H_

#include <stdint.h>
#include "gf_error.h"

#ifndef GF_SDCARD_PATH
#define GF_SDCARD_PATH "/data/media/0/"
#endif  // GF_SDCARD_PATH
#ifndef GF_DUMP_DATA_ROOT_PATH
#define GF_DUMP_DATA_ROOT_PATH "/data"
#endif  // GF_DUMP_DATA_ROOT_PATH

#define GF_MAX_FILE_NAME_LEN        (256)

#ifdef UNUSED_VAR
#undef UNUSED_VAR
#endif  // UNUSED_VAR
#define UNUSED_VAR0()
#define UNUSED_VAR1(x) (void)(x)
#define UNUSED_VAR2(x, y) UNUSED_VAR1(x), UNUSED_VAR1(y)
#define UNUSED_VAR3(x, y, z) UNUSED_VAR2(x, y), UNUSED_VAR1(z)
#define UNUSED_VAR4(x, y, z, a) UNUSED_VAR3(x, y, z), UNUSED_VAR1(a)
#define UNUSED_VAR5(x, y, z, a, b) UNUSED_VAR4(x, y, z, a), UNUSED_VAR1(b)
#define UNUSED_VAR6(x, y, z, a, b, c) UNUSED_VAR5(x, y, z, a, b), UNUSED_VAR1(c)
#define UNUSED_VAR7(x, y, z, a, b, c, d) UNUSED_VAR6(x, y, z, a, b, c), UNUSED_VAR1(d)
#define UNUSED_VAR8(x, y, z, a, b, c, d, e) UNUSED_VAR7(x, y, z, a, b, c, d), UNUSED_VAR1(e)
#define UNUSED_VAR9(x, y, z, a, b, c, d, e, f) UNUSED_VAR8(x, y, z, a, b, c, d, e), UNUSED_VAR1(f)
#define UNUSED_VAR10(x, y, z, a, b, c, d, e, f, g) UNUSED_VAR9(x, y, z, a, b, c, d, e, f), UNUSED_VAR1(g)
#define VA_NUM_ARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(0, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define UNUSED_VAR_IMPL_(nargs) UNUSED_VAR ## nargs
#define UNUSED_VAR_IMPL(nargs) UNUSED_VAR_IMPL_(nargs)
#define UNUSED_VAR(...) UNUSED_VAR_IMPL(VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define EXPORT __attribute__((visibility("default")))

#ifndef NULL
#define NULL  0
#endif  // NULL

#define GF_HW_AUTH_TOKEN_VERSION (0)

#define GF_TOKEN_LEN                                            (sizeof(uint8_t))

#define GF_CRC32_TOKEN                                          (0xF0)
#define GF_CRC32_CONTENT_LEN                                    (sizeof(uint32_t))
#define GF_CRC32_LEN                                            \
    (GF_TOKEN_LEN + GF_CRC32_CONTENT_LEN)

#define CA_TO_TA_MAX_BUFFER                 (400*1024)
#ifndef TA_LOG_DUMP_BUFFER_SIZE
#define TA_LOG_DUMP_BUFFER_SIZE             (32 * 1024)
#endif  //  TA_LOG_DUMP_BUFFER_SIZE

#define MAX_FINGERS_PER_USER (10)

#define QSEE_HMAC_KEY_MAX_LEN (256)

#define MAX_RESERVE_SIZE (64)

#define FW_VERSION_INFO_LEN    (64)
#define TEE_VERSION_INFO_LEN   (72)
#define TA_VERSION_INFO_LEN    (64)
#define ANDROID_VERSION_LEN    (20)
#define SERVICE_VERSION_LEN    (20)
#define PLATFORM_LEN           (20)
#define TARGET_PLATFORM_LEN    (20)
#define PRODUCTION_DATE_LEN    (32)
#define GF_VERSION_INFO_LEN       (64)

#define TAG_NAME_LEN_MAX            (64)
#define FUNC_NAME_LEN_MAX            (64)
#define LINE_NAME_LEN_MAX            (8)

#define MAX_FILE_ROOT_PATH_LEN (256)

#define MAX_MODULES_SIZE    (20)
#define MAX_MODULE_NAME_SIZE (64)

#define MAX_ALGO_VERSION_LEN (32)

enum {
    GF_TARGET_BIO = 1000,
    GF_TARGET_ALGO,
    GF_TARGET_SENSOR,
    GF_TARGET_PRODUCT_TEST,
    GF_TARGET_DUMP,
    GF_TARGET_INJECTION,
    GF_TARGET_AUTO_TEST,
    GF_TARGET_TRACE,
    GF_TARGET_FRR_FAR_TEST,
    GF_TARGET_EXT_TOOLS,
    GF_TARGET_DEBUG_TOOLS,
    GF_TARGET_REPLAY_TEST,
    GF_TARGET_MAX
};

typedef enum {
    GF_SHENZHEN,
    GF_BAIKAL,
    GF_ALASKA,
    GF_DELMAR,
    GF_LIMA,
    GF_UNKNOWN_SERIES,
} gf_chip_series_t;

typedef enum {
    GF_HW_AUTH_NONE = 0,
    GF_HW_AUTH_PASSWORD = (int32_t)(1 << 0),
    GF_HW_AUTH_FINGERPRINT = (int32_t)(1 << 1),
    // Additional entries should be powers of 2.
    GF_HW_AUTH_ANY = (int32_t)UINT32_MAX,
} gf_hw_authenticator_type_t;

typedef enum {
    GF_OPERATION_ENROLL = 1,
    GF_OPERATION_AUTHENTICATE,
    GF_OPERATION_FAR_FRR,
    GF_OPERATION_OTHER,
    GF_OPERATION_MAX
} gf_operation_t;

typedef enum {
    GF_SAFE_CLASS_HIGHEST = 0,
    GF_SAFE_CLASS_HIGH,
    GF_SAFE_CLASS_MEDIUM,
    GF_SAFE_CLASS_LOW,
    GF_SAFE_CLASS_LOWEST,
    GF_SAFE_CLASS_MAX,  // The number of safe class. can't set this value.
} gf_safe_class_t;

typedef struct {
    uint8_t reset_flag;
    int32_t target;
    uint32_t cmd_id;
    gf_error_t result;
    uint8_t reserved[12];  // reserved for extend
} gf_cmd_header_t;

typedef gf_cmd_header_t gf_base_cmd_t;
typedef gf_cmd_header_t gf_cancel_cmd_t;
typedef gf_cmd_header_t gf_init_finish_cmd_t;

typedef struct {
    uint8_t version;  // Current version is 0
    uint64_t challenge;
    uint64_t user_id;  // secure user ID, not Android user ID
    uint64_t authenticator_id;  // secure authenticator ID
    uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
    uint64_t timestamp;  // in network order
    uint8_t hmac[32];
} __attribute__((packed)) gf_hw_auth_token_t;

// for dump cmd
typedef enum {
    GF_CMD_DUMP_DATA,
    GF_CMD_DUMP_MAX
} gf_dump_cmd_id_t;

// dump op type
typedef enum {
    DUMP_OP_DEVICE,
    DUMP_OP_ENROLL,
    DUMP_OP_AUTHENTICATE,
    DUMP_OP_TEMPLATE,
    DUMP_OP_SPMT,
    DUMP_OP_MAX
} gf_dump_op_t;

typedef struct gf_data_info {
    gf_cmd_header_t header;
    uint32_t i_first_fetch_data;  // first fetch data
    uint32_t i_dump_op;
    uint8_t o_buffer[CA_TO_TA_MAX_BUFFER];
    uint32_t o_actual_len;
    uint32_t o_total_len;
    uint32_t o_remaining_len;  // indicate the remaining data will  be transfered from ta to ca
    uint8_t io_extend[MAX_RESERVE_SIZE];
} gf_data_info_t;

// it's must be compatible with bigdata server, cannot be modified
typedef enum {
    ENROLL = 0,
    AUTHENTICATE,
    NAVIGATION,
    FINGER_BASE,
    NAV_BASE,
    DEVICE,
    HBD,
    GSC,
    TEMPLATE,
    INVALID,
    TYPE_MAX
} gf_bigdata_type_t;

typedef struct {
    gf_bigdata_type_t type;
    uint32_t scene;  // application scene, reserved
    uint32_t charge;  // charge status, 0:no charging, 1:charging
    uint32_t screen_status;  // screen status, 0:off, 1:on
    uint32_t lock_status;  // lock status, 0:unlocked, 1:locked
} gf_bigdata_scene_t;

typedef struct gf_trace_data {
    int8_t fun_name[FUNC_NAME_LEN_MAX];
    int8_t line[LINE_NAME_LEN_MAX];
    int8_t tag[TAG_NAME_LEN_MAX];
    uint32_t type;  // 0: string 1: binary
    uint32_t data_size;
    uint8_t data[];
} gf_trace_data_t;

#endif  // _GF_BASE_TYPES_H_
