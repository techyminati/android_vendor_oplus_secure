/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_FINGERPRINT_H_
#define _GF_FINGERPRINT_H_

#include "gf_type_define.h"
#include "gf_error.h"
#include <hardware/hardware.h>
#include "fingerprint.h"
#define MAX_ID_LIST_SIZE 5
#define MAX_UVT_LENGTH   512      // it should be according with gf_common.h

#ifdef FP_HYPNUSD_ENABLE
#define ACTION_TYPE 12
#define ACTION_TIMEOUT_500  500
#endif

typedef enum gf_fingerprint_msg_type
{
    GF_FINGERPRINT_ERROR = -1,
    GF_FINGERPRINT_ACQUIRED = 1,
    GF_FINGERPRINT_TEMPLATE_ENROLLING = 3,
    GF_FINGERPRINT_TEMPLATE_REMOVED = 4,
    GF_FINGERPRINT_AUTHENTICATED = 5,
    GF_FINGERPRINT_ENUMERATED = 6,
    GF_FINGERPRINT_TEST_CMD = 7,
    GF_FINGERPRINT_HBD = 8,
    GF_FINGERPRINT_AUTHENTICATED_FIDO = 9,
    GF_FINGERPRINT_SYNCED = 10,

} gf_fingerprint_msg_type_t;

typedef enum gf_fingerprint_error
{
    GF_FINGERPRINT_ERROR_HW_UNAVAILABLE = 1,
    GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS = 2,
    GF_FINGERPRINT_ERROR_TIMEOUT = 3,
    GF_FINGERPRINT_ERROR_NO_SPACE = 4,
    GF_FINGERPRINT_ERROR_CANCELED = 5,
    GF_FINGERPRINT_ERROR_UNABLE_TO_REMOVE = 6,
    GF_FINGERPRINT_ERROR_LOCKOUT = 7, /* the fingerprint hardware is in lockout due to too many attempts */
    GF_FINGERPRINT_ERROR_VENDOR_BASE = 1000,
    GF_FINGERPRINT_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS = 1001,
    GF_FINGERPRINT_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS = 1002,
    GF_FINGERPRINT_ERROR_SPI_COMMUNICATION = 1003,
    GF_FINGERPRINT_ERROR_INVALID_PRESS_TOO_MUCH = 1004,
    GF_FINGERPRINT_ERROR_INCOMPLETE_TEMPLATE = 1005,
    GF_FINGERPRINT_ERROR_INVALID_DATA = 1006,

    /*for oppo*/
    GF_FINGERPRINT_ERROR_HAL_INITED = 998,
    GF_FINGERPRINT_ERROR_DO_RECOVER = 999,
} gf_fingerprint_error_t;

typedef enum gf_fingerprint_acquired_info
{
    GF_FINGERPRINT_ACQUIRED_GOOD = 0,
    GF_FINGERPRINT_ACQUIRED_PARTIAL = 1,
    GF_FINGERPRINT_ACQUIRED_INSUFFICIENT = 2,
    GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3,
    GF_FINGERPRINT_ACQUIRED_TOO_SLOW = 4,
    GF_FINGERPRINT_ACQUIRED_TOO_FAST = 5,
    GF_FINGERPRINT_ACQUIRED_DETECTED = 6,
    GF_FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000,
    GF_FINGERPRINT_ACQUIRED_TOO_SIMLAR = 1001,
    GF_FINGERPRINT_ACQUIRED_ALREAD_ENROLLED=1002,

    _GX_FINGERPRINT_ACQUIRED_VENDOR_BASE = 1100,
    GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = 1101,
    GF_FINGERPRINT_ACQUIRED_FINGER_DOWN = 1102,
    GF_FINGERPRINT_ACQUIRED_FINGER_UP = 1103,
    GF_FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = 1104,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA = 1105,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = 1106,
    GF_FINGERPRINT_ACQUIRED_SIMULATED_FINGER = 1107,
    GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE = 1108,
    GF_FINGERPRINT_ACQUIRED_QUICK_LICK = 1109,
} gf_fingerprint_acquired_info_t;


typedef struct gf_fingerprint_finger_id
{
    uint32_t gid;
    uint32_t fid;
} gf_fingerprint_finger_id_t;

typedef struct gf_fingerprint_enroll
{
    gf_fingerprint_finger_id_t finger;
    uint32_t samples_remaining;
    uint64_t msg;
} gf_fingerprint_enroll_t;

typedef struct gf_fingerprint_sync
{
    int32_t groupId;
    int32_t count;
    int32_t fingerIds[MAX_FINGERS_PER_USER];
} gf_fingerprint_sync_t;

typedef struct gf_fingerprint_iterator
{
    gf_fingerprint_finger_id_t finger;
    uint32_t remaining_templates;
} gf_fingerprint_iterator_t;

typedef gf_fingerprint_iterator_t gf_fingerprint_removed_t;
typedef gf_fingerprint_iterator_t gf_fingerprint_enumerated_t;

typedef struct gf_fingerprint_acquired
{
    gf_fingerprint_acquired_info_t acquired_info;
} gf_fingerprint_acquired_t;

typedef struct gf_fingerprint_authenticated
{
    gf_fingerprint_finger_id_t finger;
    gf_hw_auth_token_t hat;
} gf_fingerprint_authenticated_t;

typedef struct gf_fingerprint_authenticated_fido
{
    int32_t finger_id;
    uint32_t uvt_len;
    uint8_t uvt_data[MAX_UVT_LENGTH];
} gf_fingerprint_authenticated_fido_t;

typedef struct gf_fingerprint_test_cmd
{
    int32_t cmd_id;
    int8_t *result;
    int32_t result_len;
} gf_fingerprint_test_cmd_t;

typedef enum gf_fingerprint_hbd_status
{
    GF_FINGERPRINT_HBD_UNTOUCHED,
    GF_FINGERPRINT_SIGNAL_SEARCHING,
    GF_FINGERPRINT_HBD_TESTING,
    GF_FINGERPRINT_HBD_TOOHARD,
    GF_FINGERPRINT_HBD_TEST_FAIL
} gf_fingerprint_hbd_status_t;

typedef struct gf_fingerprint_hbd
{
    int32_t heart_beat_rate;
    gf_fingerprint_hbd_status_t status;
    int32_t *display_data;
    int32_t data_len;
    int32_t *raw_data;
    int32_t raw_data_len;
} gf_fingerprint_hbd_t;

typedef struct gf_fingerprint_msg
{
    gf_fingerprint_msg_type_t type;
    union
    {
        gf_fingerprint_error_t error;
        gf_fingerprint_enroll_t enroll;
        gf_fingerprint_removed_t removed;
        gf_fingerprint_acquired_t acquired;
        gf_fingerprint_authenticated_t authenticated;
        gf_fingerprint_authenticated_fido_t authenticated_fido;
        gf_fingerprint_test_cmd_t test;
        gf_fingerprint_hbd_t hbd;
        gf_fingerprint_enumerated_t enumerated;
        gf_fingerprint_sync_t sync;
    }
    data;
} gf_fingerprint_msg_t;

typedef struct
{
    int32_t cmd_id;
    int8_t *data;
    int32_t data_len;
} gf_fingerprint_dump_msg_t;

typedef struct gf_fingerprint_device
{
    void (*notify)(const fingerprint_msg_t *msg);
    void (*test_notify)(const gf_fingerprint_msg_t *msg);
    void (*factory_test_notify)(uint32_t cmd_id, gf_error_t test_result,
                                void *test_cmd);
    void (*dump_notify)(const gf_fingerprint_dump_msg_t *msg);
} gf_fingerprint_device_t;

#endif  // _GF_FINGERPRINT_H_
