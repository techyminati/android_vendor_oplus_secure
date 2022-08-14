/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GOODIXFINGERPRINT_H_
#define _GOODIXFINGERPRINT_H_

#include "gf_fpcore_types.h"
#include "dcs_type.h"

#define MAX_UVT_LENGTH   512
#define MAX_ID_LIST_SIZE 5

#ifdef FP_HYPNUSD_ENABLE
#define ACTION_TYPE 12
#define ACTION_TIMEOUT_500  500
#define ACTION_TIMEOUT_1000 1000
#define ACTION_TIMEOUT_2000 2000
#define ACTION_TIMEOUT_3000 3000
#endif

typedef enum gf_fingerprint_msg_type {
    GF_FINGERPRINT_ERROR = -1,
    GF_FINGERPRINT_ACQUIRED = 1,
    GF_FINGERPRINT_TEMPLATE_ENROLLING = 3,
    GF_FINGERPRINT_TEMPLATE_REMOVED = 4,
    GF_FINGERPRINT_AUTHENTICATED = 5,
    GF_FINGERPRINT_TOUCH_DOWN = 7,
    GF_FINGERPRINT_TOUCH_UP = 8,
    GF_FINGERPRINT_MONITOR = 9,
    GF_FINGERPRINT_IMAGE_INFO = 10,
    GF_FINGERPRINT_TEMPLATE_ENUMERATING = 11,
    GF_FINGERPRINT_ENGINEERING_INFO = 12,
    GF_FINGERPRINT_OPTICAL_SENDCMD = 13,
    GF_FINGERPRINT_AUTHENTICATED_DCSSTATUS = 14,
    GF_FINGRPRINT_DCS_INFO = 15,
#ifdef FP_HYPNUSD_ENABLE
    GF_FINGERPRINT_HYPNUSSETACION = 200,
#endif
    GF_FINGERPRINT_BINDCORE = 201,
    GF_FINGERPRINT_GET_CONFIG_DATA = 202,
    GF_FINGERPRINT_SETUXTHREAD = 203,
    GF_FINGERPRINT_HEART_RATE_INFO = 204,
    // vendor extension
    GF_FINGERPRINT_MSG_VENDOR_BASE = 1000,
    GF_FINGERPRINT_TEST_CMD,
    GF_FINGERPRINT_DUMP_DATA,
    GF_FINGERPRINT_AUTHENTICATED_FIDO,
    GF_FINGERPRINT_SKIN_STATUS,
    GF_FINGERPRINT_MSG_TYPE_MAX,
} gf_fingerprint_msg_type_t;

typedef enum gf_fingerprint_error {
    GF_FINGERPRINT_ERROR_HW_UNAVAILABLE = 1,
    GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS = 2,
    GF_FINGERPRINT_ERROR_TIMEOUT = 3,
    GF_FINGERPRINT_ERROR_NO_SPACE = 4,
    GF_FINGERPRINT_ERROR_CANCELED = 5,
    GF_FINGERPRINT_ERROR_UNABLE_TO_REMOVE = 6,
    GF_FINGERPRINT_ERROR_LOCKOUT = 7, /* the fingerprint hardware is in lockout due to too many attempts */
    // vendor extension
    GF_FINGERPRINT_ERROR_VENDOR_BASE = 1000,
    GF_FINGERPRINT_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS = 1001,
    GF_FINGERPRINT_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS = 1002,
    GF_FINGERPRINT_ERROR_SPI_COMMUNICATION = 1003,
    GF_FINGERPRINT_ERROR_INVALID_PRESS_TOO_MUCH = 1004,
    GF_FINGERPRINT_ERROR_INCOMPLETE_TEMPLATE = 1005,
    GF_FINGERPRINT_ERROR_INVALID_DATA = 1006,
    GF_FINGERPRINT_ERROR_NEED_CANCEL_ENROLL = 1007,
    GF_FINGERPRINT_ERROR_LOCKOUT_PERMANENT = 1008,
} gf_fingerprint_error_t;

typedef enum gf_fingerprint_acquired_info {
    GF_FINGERPRINT_ACQUIRED_GOOD = 0,
    GF_FINGERPRINT_ACQUIRED_PARTIAL = 1,
    GF_FINGERPRINT_ACQUIRED_INSUFFICIENT = 2,
    GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3,
    GF_FINGERPRINT_ACQUIRED_TOO_SLOW = 4,
    GF_FINGERPRINT_ACQUIRED_TOO_FAST = 5,
    GF_FINGERPRINT_ACQUIRED_DETECTED = 6,

    /* oujinrong@BSP.Fingerprint.Basic 2019/10/10, move defination for it */
    /* chenran@BSP.Fingerprint.Basic 2019/10/16, sync with fingerprint.h */
    GF_FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA = 1001,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = 1002,
    GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = 1101,
    GF_FINGERPRINT_ACQUIRED_FINGER_DOWN = 1102,
    GF_FINGERPRINT_ACQUIRED_FINGER_UP = 1103,
    GF_FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = 1104,
    GF_FINGERPRINT_ACQUIRED_SIMULATED_FINGER = 1105,
    GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE = 1106,
    GF_FINGERPRINT_ACQUIRED_NOT_LIVE_FINGER = 1107,
    GF_FINGERPRINT_ACQUIRED_ANTIPEEPING = 1108,
    GF_FINGERPRINT_ACQUIRED_SCREEN_STRUCT = 1109,
} gf_fingerprint_acquired_info_t;

typedef struct gf_fingerprint_finger_id {
    uint32_t gid;
    uint32_t fid;
} gf_fingerprint_finger_id_t;

typedef struct gf_fingerprint_fingers_id {
    uint32_t gid;
    uint32_t fid[MAX_FINGERS_PER_USER];
} gf_fingerprint_fingers_id_t;
typedef struct gf_fingerprint_enroll {
    gf_fingerprint_finger_id_t finger;
    uint32_t samples_remaining;
    uint64_t msg;
} gf_fingerprint_enroll_t;




//typedef gf_fingerprint_iterator_t gf_fingerprint_removed_t;
//typedef gf_fingerprint_iterator_t gf_fingerprint_enumerated_t;
typedef gf_fingerprint_fingers_id_t gf_fingerprint_remove_fingers_t;

typedef struct gf_fingerprint_enumerated {
    gf_fingerprint_finger_id_t fingers[MAX_ID_LIST_SIZE];
    uint32_t remaining_templates;
    uint32_t gid;
} gf_fingerprint_enumerated_t;

typedef struct gf_fingerprint_removed {
    gf_fingerprint_finger_id_t finger;
    #ifdef OPLUS_FEATURE_FINGERPRINT
    //use fingerprint_enumerated instead
    uint32_t fingers_count;
    gf_fingerprint_finger_id_t total_fingers[MAX_ID_LIST_SIZE];
    #endif /* OPLUS_FEATURE_FINGERPRINT */
} gf_fingerprint_removed_t;

typedef struct gf_fingerprint_quality {
    uint32_t successed;
    uint32_t image_quality;
    uint32_t quality_pass;
} gf_fingerprint_quality_t;

typedef enum gf_fingerprint_engineering_info_type {
    GF_FINGERPRINT_IMAGE_SNR = 1,
    GF_FINGERPRINT_IMAGE_QUALITY = 2,
    GF_FINGERPRINT_IMAGE_PIXEL = 3,
} gf_fingerprint_engineering_info_type_t;

typedef struct gf_fingerprint_engineering_info {
    gf_fingerprint_engineering_info_type_t type;
    gf_fingerprint_quality_t quality;
    //fingerprint_pixeltest_t pixel;
    //fingerprint_image_snr_t snr;
} gf_fingerprint_engineering_info_t;

typedef struct gf_fingerprint_acquired {
    gf_fingerprint_acquired_info_t acquired_info;
    int32_t pressX;
    int32_t pressY;
    int32_t touchMajor;
    int32_t touchMinor;
    int32_t touchOrientation;
} gf_fingerprint_acquired_t;

typedef struct gf_fingerprint_authenticated {
    gf_fingerprint_finger_id_t finger;
    gf_hw_auth_token_t hat;
} gf_fingerprint_authenticated_t;

typedef struct gf_fingerprint_authenticated_fido {
    int32_t finger_id;
    uint32_t uvt_len;
    uint8_t uvt_data[MAX_UVT_LENGTH];
} gf_fingerprint_authenticated_fido_t;

typedef struct gf_fingerprint_test_cmd {
    int32_t cmd_id;
    int8_t *result;
    int32_t result_len;
} gf_fingerprint_test_cmd_t;
typedef struct gf_fingerprint_skin_status {
    gf_fingerprint_finger_id_t finger;
    float score;
    float low_thres;
    float high_thres;
} gf_fingerprint_skin_status_t;

typedef struct gf_fingerprint_tp_info{
    uint8_t touch_state;
    uint8_t area_rate;  //for touch area
    uint16_t  x;
    uint16_t  y;
    uint16_t touch_major;
    uint16_t touch_minor;
    uint16_t touch_angle;
}gf_fingerprint_tp_info_t;

typedef struct gf_fingerprint_auth_dcsmsg
{
    uint32_t auth_result;
    uint32_t fail_reason;
    uint32_t quality_score;
    uint32_t match_score;
    uint32_t signal_value;
    uint32_t img_area;
    uint32_t retry_times;
    char algo_version[MAX_ALGO_VERSION_LEN];
    uint32_t chip_ic;
    uint32_t factory_id;
    uint32_t module_type;
    uint32_t lense_type;
    uint32_t dsp_availalbe;
    uint32_t auth_total_time;
    uint32_t ui_ready_time;
    uint32_t capture_time;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t auth_time;
    uint32_t detect_fake_time;
} gf_fingerprint_auth_dcsmsg_t;

#ifdef FP_HYPNUSD_ENABLE
typedef struct gf_fingerprint_hypnusd_setting
{
    int32_t action_type;
    int32_t action_timeout;
} gf_fingerprint_hypnusd_t;
#endif

typedef struct gf_fingerprint_bindcore
{
    int32_t tid;
} gf_fingerprint_bindcore_t;

typedef struct gf_fingerprint_settings_transfer
{
    void *para;
} gf_fingerprint_settings_transfer_t;

typedef struct gf_fingreprint_lockout {
    gf_fingerprint_error_t error;
    uint64_t duration_mills;
} gf_fingreprint_lockout_t;

typedef struct gf_fingerprint_setuxthread
{
    int32_t pid;
    int32_t tid;
    int8_t enable;
} gf_fingerprint_setuxthread_t;

typedef struct gf_fingerprint_msg {
    gf_fingerprint_msg_type_t type;
    union {
        gf_fingerprint_error_t error;
        gf_fingerprint_enroll_t enroll;
        gf_fingerprint_enumerated_t enumerated;
        gf_fingerprint_removed_t removed;
        gf_fingerprint_acquired_t acquired;
        gf_fingerprint_engineering_info_t engineering;
        gf_fingerprint_authenticated_t authenticated;
        gf_fingerprint_authenticated_fido_t authenticated_fido;
        gf_fingerprint_test_cmd_t test;
        gf_fingerprint_tp_info_t  tp_info;
        gf_fingerprint_auth_dcsmsg_t auth_dcsmsg;
        oplus_fingerprint_dcs_info_t dcs_info;
#ifdef FP_HYPNUSD_ENABLE
        gf_fingerprint_hypnusd_t hypnusd_setting;
#endif
        gf_fingerprint_bindcore_t bindcore_setting;
        gf_fingerprint_skin_status_t skin;  //liulihuaG70430
        gf_fingerprint_settings_transfer_t data_config;
        gf_fingreprint_lockout_t lockout;
        gf_fingerprint_remove_fingers_t removed_fingers;
        gf_fingerprint_setuxthread_t setuxthread_info;
    }
    data;
} gf_fingerprint_msg_t;

/* Callback function type */
typedef void (*gf_fingerprint_notify_t)(const gf_fingerprint_msg_t *msg);

#endif /* _GOODIXFINGERPRINT_H_ */
