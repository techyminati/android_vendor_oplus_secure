/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_INCLUDE_HARDWARE_FINGERPRINT_H
#define ANDROID_INCLUDE_HARDWARE_FINGERPRINT_H

#include "hw_auth_token.h"

#define MAX_ID_LIST_SIZE 5

#ifdef FP_HYPNUSD_ENABLE
#define ACTION_TYPE 12
#define ACTION_TIMEOUT_500  500
#endif

typedef enum fingerprint_msg_type {
    FINGERPRINT_ERROR = -1,
    FINGERPRINT_ACQUIRED = 1,
    FINGERPRINT_TEMPLATE_ENROLLING = 3,
    FINGERPRINT_TEMPLATE_REMOVED = 4,
    FINGERPRINT_AUTHENTICATED = 5,
    FINGERPRINT_GET_IMAGE_QUALITY = 6,
    FINGERPRINT_TOUCH_DOWN = 7,
    FINGERPRINT_TOUCH_UP = 8,
    FINGERPRINT_MONITOR = 9,
    FINGERPRINT_IMAGE_INFO = 10,
    FINGERPRINT_TEMPLATE_ENUMERATING = 11,
    FINGERPRINT_ENGINEERING_INFO = 12,
    FINGERPRINT_OPTICAL_SENDCMD = 13,
    FINGERPRINT_AUTHENTICATED_DCSSTATUS = 14,
    FINGERPRINT_HARDWARE = 15,
#ifdef FP_HYPNUSD_ENABLE
    FINGERPRINT_HYPNUSDSETACION = 200,
#endif
    FINGERPRINT_BINDCORE = 201,
} fingerprint_msg_type_t;

typedef enum fingerprint_screen_state {
    FINGERPRINT_SCREEN_OFF = 0,
    FINGERPRINT_SCREEN_ON = 1,
} fingerprint_screen_state_t;

/*
 * Fingerprint errors are meant to tell the framework to terminate the current operation and ask
 * for the user to correct the situation. These will almost always result in messaging and user
 * interaction to correct the problem.
 *
 * For example, FINGERPRINT_ERROR_CANCELED should follow any acquisition message that results in
 * a situation where the current operation can't continue without user interaction. For example,
 * if the sensor is dirty during enrollment and no further enrollment progress can be made,
 * send FINGERPRINT_ACQUIRED_IMAGER_DIRTY followed by FINGERPRINT_ERROR_CANCELED.
 */
typedef enum fingerprint_error {
    FINGERPRINT_ERROR_HW_UNAVAILABLE = 1, /* The hardware has an error that can't be resolved. */
    FINGERPRINT_ERROR_UNABLE_TO_PROCESS = 2, /* Bad data; operation can't continue */
    FINGERPRINT_ERROR_TIMEOUT = 3, /* The operation has timed out waiting for user input. */
    FINGERPRINT_ERROR_NO_SPACE = 4, /* No space available to store a template */
    FINGERPRINT_ERROR_CANCELED = 5, /* The current operation can't proceed. See above. */
    FINGERPRINT_ERROR_UNABLE_TO_REMOVE = 6, /* fingerprint with given id can't be removed */
    FINGERPRINT_ERROR_HAL_INITED = 998, /* hal init done */
    FINGERPRINT_ERROR_DO_RECOVER = 999, /* fingerprint recover */
    FINGERPRINT_ERROR_VENDOR_BASE = 1000 /* vendor-specific error messages start here */
} fingerprint_error_t;

/*
 * Fingerprint acquisition info is meant as feedback for the current operation.  Anything but
 * FINGERPRINT_ACQUIRED_GOOD will be shown to the user as feedback on how to take action on the
 * current operation. For example, FINGERPRINT_ACQUIRED_IMAGER_DIRTY can be used to tell the user
 * to clean the sensor.  If this will cause the current operation to fail, an additional
 * FINGERPRINT_ERROR_CANCELED can be sent to stop the operation in progress (e.g. enrollment).
 * In general, these messages will result in a "Try again" message.
 */
typedef enum fingerprint_acquired_info {
    FINGERPRINT_ACQUIRED_GOOD = 0,
    FINGERPRINT_ACQUIRED_PARTIAL = 1, /* sensor needs more data, i.e. longer swipe. */
    FINGERPRINT_ACQUIRED_INSUFFICIENT = 2, /* image doesn't contain enough detail for recognition*/
    FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3, /* sensor needs to be cleaned */
    FINGERPRINT_ACQUIRED_TOO_SLOW = 4, /* mostly swipe-type sensors; not enough data collected */
    FINGERPRINT_ACQUIRED_TOO_FAST = 5, /* for swipe and area sensors; tell user to slow down*/
    FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000, /* vendor-specific acquisition messages start here */
    FINGERPRINT_ACQUIRED_TOO_SIMILAR = 1001, /* for similar enrolled area */
    FINGERPRINT_ACQUIRED_ALREADY_ENROLLED = 1002, /* for the same */
} fingerprint_acquired_info_t;
#ifdef FP_HYPNUSD_ENABLE
typedef struct fingerprint_hypnusd_setting
{
    int32_t action_type;
    int32_t action_timeout;
} fingerprint_hypnusd_t;
#endif

typedef struct fingerprint_bindcore
{
    int32_t tid;
} fingerprint_bindcore_t;

typedef struct fingerprint_auth_dcsmsg
{
    uint32_t auth_result;
    uint32_t fail_reason;
    uint32_t quality_score;
    uint32_t match_score;
    uint32_t signal_value;   //grey1
    uint32_t img_area;
    uint32_t retry_times;
    char algo_version[32];
    uint32_t chip_ic;
    uint32_t factory_id;     //grey2
    uint32_t module_type;    //grey_diff
    uint32_t lense_type;     //deadpx
    uint32_t dsp_availalbe;  //badline
} fingerprint_auth_dcsmsg_t;

typedef struct fingerprint_finger_id {
    uint32_t gid;
    uint32_t fid;
} fingerprint_finger_id_t;

typedef struct fingerprint_enroll {
    fingerprint_finger_id_t finger;
    /* samples_remaining goes from N (no data collected, but N scans needed)
     * to 0 (no more data is needed to build a template). */
    uint32_t samples_remaining;
    uint64_t msg; /* Vendor specific message. Used for user guidance */
    uint32_t finger_count;
    fingerprint_finger_id_t total_fingers[MAX_ID_LIST_SIZE];
} fingerprint_enroll_t;

typedef struct fingerprint_enumerated {
    fingerprint_finger_id_t finger[MAX_ID_LIST_SIZE];
    uint32_t remaining_templates;
    uint32_t gid;
} fingerprint_enumerated_t;

typedef struct fingerprint_removed {
    fingerprint_finger_id_t finger;
    uint32_t remaining_templates; //finger count
    fingerprint_finger_id_t total_fingers[MAX_ID_LIST_SIZE];
} fingerprint_removed_t;

typedef struct fingerprint_acquired {
    fingerprint_acquired_info_t acquired_info; /* information about the image */
} fingerprint_acquired_t;

typedef struct fingerprint_authenticated {
    fingerprint_finger_id_t finger;
    hw_auth_token_t hat;
} fingerprint_authenticated_t;

typedef enum fingerprint_monitor_type {
    FINGERPRINT_POWER_MONITOR = 0,
    FINGERPRINT_ERROR_STATE =1,
    FINGERPRINT_TP_PROTECT_MONITOR = 2,
} fingerprint_monitor_type_t;

typedef enum fingerprint_tee_hal_mode {
    FINGER_TEE_HAL_IDENTIFY_MODE = 0,
    FINGER_TEE_HAL_ENROLL_MODE = 1,
    FINGER_TEE_HAL_SELFTEST_MODE= 2,
    FINGER_TEE_HAL_LOCK_MODE = 3,
} fingerprint_tee_hal_mode_t;

typedef struct fingerprint_monitor_power {
    double battery;
} fingerprint_monitor_power_t;

typedef struct fingerprint_monitor_tp_protect {
    fingerprint_tee_hal_mode_t mode;
} fingerprint_monitor_tp_protect_t;

typedef enum engineering_info_acquire_action {
    FINGERPRINT_GET_IMAGE_SNR = 0,
    FINGERPRINT_GET_IMAGE_QUALITYS = 1,
    FINGERPRINT_GET_BAD_PIXELS = 2,
    FINGERPRINT_SELF_TEST = 3,
} engineering_info_acquire_action_t;

typedef struct fingerprint_monitor {
    fingerprint_monitor_type_t type;
    union {
        fingerprint_monitor_power_t power;
        fingerprint_monitor_tp_protect_t tp_protect;
    } data;
} fingerprint_monitor_t;

typedef struct fingerprint_quality {
    uint32_t successed;
    uint32_t image_quality;
    uint32_t image_quality_pass;
} fingerprint_quality_t;

typedef enum fingerprint_engineering_info_type {
    FINGERPRINT_IMAGE_SNR = 1,
    FINGERPRINT_IMAGE_QUALITY = 2,
    FINGERPRINT_IMAGE_PIXEL = 3,
} fingerprint_engineering_info_type_t;

typedef struct engineering_info {
    fingerprint_engineering_info_type_t type;
    fingerprint_quality_t quality;
    //fingerprint_pixeltest_t pixel;
    //fingerprint_image_snr_t snr;
} engineering_info_t;

typedef struct fingerprint_image_info {
    fingerprint_acquired_info_t type; // PARTIAL, DIRTY, GOOD
    uint32_t quality;
    uint32_t match_score;
} fingerprint_image_info_t;

typedef struct fingerprint_enum_cust {
    fingerprint_finger_id_t finger[MAX_ID_LIST_SIZE];
    /* samples_remaining goes from N (no data collected, but N scans needed)
     * to 0 (no more data is needed to build a template). */
    uint32_t samples_remaining;
    uint64_t gid; /* Vendor specific message. Used for user guidance */
} fingerprint_enum_cust_t;

typedef struct fingerprint_test_cmd {
    int32_t cmd_id;
    int8_t *result;
    int32_t result_len;
} fingerprint_test_cmd_t;

typedef struct fingerprint_msg {
    fingerprint_msg_type_t type;
    union {
        fingerprint_error_t error;
        fingerprint_enroll_t enroll;
        fingerprint_enumerated_t enumerated;
        fingerprint_removed_t removed;
        fingerprint_acquired_t acquired;
        fingerprint_authenticated_t authenticated;

        fingerprint_monitor_t monitor;
        engineering_info_t engineering;
        fingerprint_image_info_t image_info;
        fingerprint_enum_cust_t enumerated_cust;
#ifdef FP_HYPNUSD_ENABLE
        fingerprint_hypnusd_t hypnusd_setting;
#endif
        fingerprint_bindcore_t bindcore_setting;
        fingerprint_auth_dcsmsg_t auth_dcsmsg;
        fingerprint_test_cmd_t test;
    } data;
} fingerprint_msg_t;

/* Callback function type */
typedef void (*fingerprint_notify_t)(const fingerprint_msg_t *msg);

#endif  /* ANDROID_INCLUDE_HARDWARE_FINGERPRINT_H */
