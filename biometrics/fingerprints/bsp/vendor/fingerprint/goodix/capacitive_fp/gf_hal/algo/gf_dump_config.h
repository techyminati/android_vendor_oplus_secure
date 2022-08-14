/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_DUMP_CONFIG_H_
#define _GF_DUMP_CONFIG_H_

#include <stdint.h>
#include "gf_common.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#define DUMP_VERSION_STR_MAX_LEN (50)

// support 32bits
// 0000 0000 0000 0000 0000 0000 0000 0000
typedef enum
{
    OP_DATA_NONE                   = 0,
    OP_DATA_RAW_DATA               = 1 << 0,
    OP_DATA_BROKEN_CHECK_RAW_DATA  = 1 << 1,
    OP_DATA_CALIBRATION_PARAMS     = 1 << 2,
    OP_DATA_CALIBRATION_RES        = 1 << 3,
    OP_DATA_DATA_BMP               = 1 << 4,
    OP_DATA_SITO_BMP               = 1 << 5,
    OP_DATA_NAV_ENHANCE            = 1 << 6,
    OP_DATA_TEMPLATE_DATA          = 1 << 7,
    OP_DATA_DEVICE_INFO            = 1 << 8,
} gf_dump_operation_data_type_t;

#define DUMP_ALL_OP_DATA (OP_DATA_RAW_DATA | OP_DATA_BROKEN_CHECK_RAW_DATA |       \
                          OP_DATA_CALIBRATION_PARAMS | OP_DATA_CALIBRATION_RES |   \
                          OP_DATA_DATA_BMP | OP_DATA_SITO_BMP |                    \
                          OP_DATA_NAV_ENHANCE | OP_DATA_TEMPLATE_DATA |            \
                          OP_DATA_DEVICE_INFO)

typedef enum
{
    OP_RESULT_ALL = 0,
    OP_RESULT_SUCCESS,
    OP_RESULT_FAIL
} gf_dump_operation_result_t;

typedef enum
{
    DUMP_OP_NOT_ALLOWED = -1,
    DUMP_OP_ENROLL,
    DUMP_OP_AUTHENTICATE,
    DUMP_OP_FINGER_BASE,
    DUMP_OP_NAV,
    DUMP_OP_NAV_BASE,
    DUMP_OP_TEST_PERFORMANCE,
    DUMP_OP_TEST_PIXEL,
    DUMP_OP_TEST_SENSOR_FINE,
    DUMP_OP_TEST_BAD_POINT,
    DUMP_OP_DEVICE_INFO,
    DUMP_OP_TEMPLATE,
    DUMP_OP_NAV_ENHANCE_DATA,
    DUMP_OP_CALIBRATION_PARAM_RETEST,
    DUMP_OP_TEST_DATA_NOISE,
    DUMP_OP_NUM,
} gf_dump_operation_type_t;

typedef struct
{
    gf_dump_operation_type_t operation;
    gf_dump_operation_result_t result;
    int32_t data_type;
} gf_dump_operation_t;

typedef struct
{
    uint8_t dump_version[DUMP_VERSION_STR_MAX_LEN];
    uint8_t dump_bigdata_version[DUMP_VERSION_STR_MAX_LEN];
    uint8_t dump_enabled;  // 0 disable, 1 enable
    uint8_t dump_encrypt_enabled;  // 0 disable, 1 enable
    uint8_t dump_big_data_enabled;  // 0 disable, 1 enable
    gf_dump_path_t dump_path;  // data to be stored in which directory, /data or /sdcard
    gf_dump_operation_t dump_operation[DUMP_OP_NUM];  // dump fingerprint operation
} gf_dump_config_t;

// get a copy of dump config, caller must free the copy
gf_dump_config_t* gf_get_dump_config();

// do mapping for dump_operation and irq_operation
gf_dump_operation_type_t gf_get_dump_op_by_irq_op(gf_operation_type_t irq_op);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_DUMP_CONFIG_H_

