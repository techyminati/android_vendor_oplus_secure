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

#include <hardware/hardware.h>
#include <hardware/hw_auth_token.h>
#include "gf_fpcore_types.h"
#include "dcs_type.h"

#define FINGERPRINT_MODULE_API_VERSION_1_0 HARDWARE_MODULE_API_VERSION(1, 0)
#define FINGERPRINT_MODULE_API_VERSION_2_0 HARDWARE_MODULE_API_VERSION(2, 0)
#define FINGERPRINT_MODULE_API_VERSION_2_1 HARDWARE_MODULE_API_VERSION(2, 1)
#define FINGERPRINT_MODULE_API_VERSION_3_0 HARDWARE_MODULE_API_VERSION(3, 0)
#define FINGERPRINT_HARDWARE_MODULE_ID "fingerprint"

#define MAX_ID_LIST_SIZE 5
#define MAX_UVT_LENGTH   512

#ifdef FP_HYPNUSD_ENABLE
#define ACTION_TYPE 12
#define ACTION_TIMEOUT_500  500
#define ACTION_TIMEOUT_1000 1000
#define ACTION_TIMEOUT_2000 2000
#define ACTION_TIMEOUT_3000 3000
#define ACTION_TIMEOUT_6000 6000
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
    OPLUS_FINGRPRINT_DCS_INFO = 15,
#ifdef FP_HYPNUSD_ENABLE
    FINGERPRINT_HYPNUSSETACION = 200,
#endif
    FINGERPRINT_BINDCORE = 201,
    FINGERPRINT_GET_CONFIG_DATA = 202,
    FINGERPRINT_SETUXTHREAD = 203,
    FINGERPRINT_ACQUIRED_STABILITYTEST_DATA,
    // vendor extension
    GF_FINGERPRINT_MSG_VENDOR_BASE = 1000,
    GF_FINGERPRINT_TEST_CMD,
    GF_FINGERPRINT_DUMP_DATA,
    GF_FINGERPRINT_AUTHENTICATED_FIDO,
    GF_FINGERPRINT_MSG_TYPE_MAX,
} fingerprint_msg_type_t;

typedef enum fingerprint_screen_state {
    FINGERPRINT_SCREEN_OFF = 0,
    FINGERPRINT_SCREEN_ON = 1,
} fingerprint_screen_state_t;

typedef enum fingerprint_auth_type {
    AUTH_TYPE_KEYGUARD = 1,
    AUTH_TYPE_PAY = 2,
    AUTH_TYPE_OTHER = 3,
} fingerprint_auth_type_t;

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
    FINGERPRINT_ERROR_LOCKOUT = 7, /* the fingerprint hardware is in lockout due to too many attempts */
    FINGERPRINT_ERROR_VENDOR_BASE = 1000, /* vendor-specific error messages start here */
    FINGERPRINT_ERROR_HAL_INITED = FINGERPRINT_ERROR_VENDOR_BASE + 1, /* hal init done */
    FINGERPRINT_ERROR_DO_RECOVER = FINGERPRINT_ERROR_VENDOR_BASE + 2, /* fingerprint recover */
	GF_FINGERPRINT_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS = 1003,
    GF_FINGERPRINT_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS = 1004,
    GF_FINGERPRINT_ERROR_SPI_COMMUNICATION = 1005,
    GF_FINGERPRINT_ERROR_INVALID_PRESS_TOO_MUCH = 1006,
    GF_FINGERPRINT_ERROR_INCOMPLETE_TEMPLATE = 1007,
    GF_FINGERPRINT_ERROR_INVALID_DATA = 1008,
    //FINGERPRINT_ERROR_HAL_INITED = 998, /* hal init done */
    //FINGERPRINT_ERROR_DO_RECOVER = 999, /* fingerprint recover */
    //FINGERPRINT_ERROR_VENDOR_BASE = 1000 /* vendor-specific error messages start here */
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
    FINGERPRINT_ACQUIRED_DETECTED = 6, /* when the finger is first detected. Used to optimize wakeup.
                                          Should be followed by one of the above messages */
    FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000, /* vendor-specific acquisition messages start here */
    FINGERPRINT_ACQUIRED_VENDOR_SAME_AREA = 1001, /* for similar enrolled area */
    FINGERPRINT_ACQUIRED_VENDOR_DUPLICATE_FINGER = 1002, /* for the same */
	GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = 1003,
    GF_FINGERPRINT_ACQUIRED_FINGER_DOWN = 1004,
    GF_FINGERPRINT_ACQUIRED_FINGER_UP = 1005,
    GF_FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = 1006,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA = 1007,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = 1008,
    GF_FINGERPRINT_ACQUIRED_SIMULATED_FINGER = 1009,
    GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE = 1010,
    //FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000, /* vendor-specific acquisition messages start here */
    //FINGERPRINT_ACQUIRED_TOO_SIMILAR = 1001, /* for similar enrolled area */
    //FINGERPRINT_ACQUIRED_ALREADY_ENROLLED = 1002, /* for the same */
} fingerprint_acquired_info_t;

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
    uint32_t finger_count;
    fingerprint_finger_id_t total_fingers[MAX_ID_LIST_SIZE];
} fingerprint_removed_t;

typedef struct fingerprint_quality {
    uint32_t successed;
    uint32_t image_quality;
    uint32_t quality_pass;
} fingerprint_quality_t;

typedef struct fingerprint_image_info {
    fingerprint_acquired_info_t type; // PARTIAL, DIRTY, GOOD
    uint32_t quality;
    uint32_t match_score;
} fingerprint_image_info_t;

typedef enum engineering_info_acquire_action {
    FINGERPRINT_GET_IMAGE_SNR = 0,
    FINGERPRINT_GET_IMAGE_QUALITYS = 1,
    FINGERPRINT_GET_BAD_PIXELS = 2,
    FINGERPRINT_SELF_TEST = 3,
} engineering_info_acquire_action_t;

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

typedef struct fingerprint_acquired {
    fingerprint_acquired_info_t acquired_info; /* information about the image */
} fingerprint_acquired_t;

typedef struct fingerprint_authenticated {
    fingerprint_finger_id_t finger;
    hw_auth_token_t hat;
} fingerprint_authenticated_t;

typedef struct gf_fingerprint_test_cmd
{
    int32_t cmd_id;
    int8_t *result;
    int32_t result_len;
} gf_fingerprint_test_cmd_t;

typedef struct gf_fingerprint_authenticated_fido
{
    int32_t finger_id;
    uint32_t uvt_len;
    uint8_t uvt_data[MAX_UVT_LENGTH];
} gf_fingerprint_authenticated_fido_t;

typedef struct gf_fingerprint_acquired_test_info{
    uint8_t    auth_result;
    uint8_t    fail_reason;
    uint8_t    quality_score;
    uint8_t    match_score;
    uint8_t    signal_value;
    uint8_t    retry_times;
    uint8_t    algo_version;
    uint8_t    chip_ic;
    uint8_t    dsp_availalbe;
}fingerprint_acquired_test_info_t;


typedef struct fingerprint_auth_dcsmsg
{
    uint32_t auth_result;
    uint32_t fail_reason;
    uint32_t quality_score;
    uint32_t match_score;
    uint32_t signal_value;
    uint32_t img_area;
    uint32_t retry_times;
    char  algo_version[32];
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
} fingerprint_auth_dcsmsg_t;

#ifdef FP_HYPNUSD_ENABLE
typedef struct fingerprint_hypnus_setting
{
    int32_t action_type;
    int32_t action_timeout;
} fingerprint_hypnus_setting_t;
#endif

typedef struct fingerprint_bindcore
{
    int32_t tid;
} fingerprint_bindcore_t;

typedef struct fingerprint_settings_transfer
{
    void *para;
} fingerprint_settings_transfer_t;

typedef struct fingerprint_setuxthread
{
    int32_t pid;
    int32_t tid;
    int8_t enable;
} fingerprint_setuxthread_t;

typedef struct fingerprint_msg {
    fingerprint_msg_type_t type;
    union {
        fingerprint_error_t error;
        fingerprint_enroll_t enroll;
        fingerprint_enumerated_t enumerated;
        fingerprint_removed_t removed;
        fingerprint_acquired_t acquired;
        fingerprint_authenticated_t authenticated;
        engineering_info_t engineering;
        fingerprint_image_info_t image_info;
        gf_fingerprint_authenticated_fido_t authenticated_fido;
        gf_fingerprint_test_cmd_t test;
        fingerprint_auth_dcsmsg_t auth_dcsmsg;
        oplus_fingerprint_dcs_info_t dcs_info;
        fingerprint_acquired_test_info_t acquired_test_info;
#ifdef FP_HYPNUSD_ENABLE
        fingerprint_hypnus_setting_t hypnus_setting;
#endif
        fingerprint_bindcore_t bindcore_setting;
        fingerprint_settings_transfer_t data_config;
        fingerprint_setuxthread_t setuxthread_info;
    } data;
} fingerprint_msg_t;

/* Callback function type */
typedef void (*fingerprint_notify_t)(const fingerprint_msg_t *msg);

/* Synchronous operation */
typedef struct fingerprint_device {
    /**
     * Common methods of the fingerprint device. This *must* be the first member
     * of fingerprint_device as users of this structure will cast a hw_device_t
     * to fingerprint_device pointer in contexts where it's known
     * the hw_device_t references a fingerprint_device.
     */
    struct hw_device_t common;

    /*
     * Client provided callback function to receive notifications.
     * Do not set by hand, use the function above instead.
     */
    fingerprint_notify_t notify;

    /*
     * Set notification callback:
     * Registers a user function that would receive notifications from the HAL
     * The call will block if the HAL state machine is in busy state until HAL
     * leaves the busy state.
     *
     * Function return: 0 if callback function is successfuly registered
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_notify)(struct fingerprint_device *dev, fingerprint_notify_t notify);

    /*
     * Fingerprint pre-enroll enroll request:
     * Generates a unique token to upper layers to indicate the start of an enrollment transaction.
     * This token will be wrapped by security for verification and passed to enroll() for
     * verification before enrollment will be allowed. This is to ensure adding a new fingerprint
     * template was preceded by some kind of credential confirmation (e.g. device password).
     *
     * Function return: 0 if function failed
     *                  otherwise, a uint64_t of token
     */
    uint64_t (*pre_enroll)(struct fingerprint_device *dev);

    /*
     * Fingerprint enroll request:
     * Switches the HAL state machine to collect and store a new fingerprint
     * template. Switches back as soon as enroll is complete
     * (fingerprint_msg.type == FINGERPRINT_TEMPLATE_ENROLLING &&
     *  fingerprint_msg.data.enroll.samples_remaining == 0)
     * or after timeout_sec seconds.
     * The fingerprint template will be assigned to the group gid. User has a choice
     * to supply the gid or set it to 0 in which case a unique group id will be generated.
     *
     * Function return: 0 if enrollment process can be successfully started
     *                  or a negative number in case of error, generally from the errno.h set.
     *                  A notify() function may be called indicating the error condition.
     */
    int (*enroll)(struct fingerprint_device *dev, const hw_auth_token_t *hat,
                    uint32_t gid, uint32_t timeout_sec);

    /*
     * Finishes the enroll operation and invalidates the pre_enroll() generated challenge.
     * This will be called at the end of a multi-finger enrollment session to indicate
     * that no more fingers will be added.
     *
     * Function return: 0 if the request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*post_enroll)(struct fingerprint_device *dev);

    /*
     * get_authenticator_id:
     * Returns a token associated with the current fingerprint set. This value will
     * change whenever a new fingerprint is enrolled, thus creating a new fingerprint
     * set.
     *
     * Function return: current authenticator id or 0 if function failed.
     */
    uint64_t (*get_authenticator_id)(struct fingerprint_device *dev);

    /*
     * Cancel pending enroll or authenticate, sending FINGERPRINT_ERROR_CANCELED
     * to all running clients. Switches the HAL state machine back to the idle state.
     * Unlike enroll_done() doesn't invalidate the pre_enroll() challenge.
     *
     * Function return: 0 if cancel request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*cancel)(struct fingerprint_device *dev);

    /*
     * Enumerate all the fingerprint templates found in the directory set by
     * set_active_group()
     * For each template found notify() will be called with:
     * fingerprint_msg.type == FINGERPRINT_TEMPLATE_ENUMERATED
     * fingerprint_msg.data.enumerated.finger indicating a template id
     * fingerprint_msg.data.enumerated.remaining_templates indicating how many more
     * enumeration messages to expect.
     *
     * Function return: 0 if enumerate request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*enumerate)(struct fingerprint_device *dev);

    /*
     * Fingerprint remove request:
     * Deletes a fingerprint template.
     * Works only within the path set by set_active_group().
     * notify() will be called with details on the template deleted.
     * fingerprint_msg.type == FINGERPRINT_TEMPLATE_REMOVED and
     * fingerprint_msg.data.removed.finger indicating the template id removed.
     *
     * Function return: 0 if fingerprint template(s) can be successfully deleted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*remove)(struct fingerprint_device *dev, uint32_t gid, uint32_t fid);

    /*
     * Restricts the HAL operation to a set of fingerprints belonging to a
     * group provided.
     * The caller must provide a path to a storage location within the user's
     * data directory.
     *
     * Function return: 0 on success
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_active_group)(struct fingerprint_device *dev, uint32_t gid,
                            const char *store_path);

    /*
     * Authenticates an operation identifed by operation_id
     *
     * Function return: 0 on success
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*authenticate)(struct fingerprint_device *dev, uint64_t operation_id, uint32_t gid);
    int (*authenticateAsType)(struct fingerprint_device *dev, uint64_t operation_id, uint32_t gid, uint32_t authtype);
    #ifdef OPLUS_FEATURE_FINGERPRINT
    /*
     * fingerprint sensor selftest for factory test.
     */
    int (*selftest)(struct fingerprint_device *dev);

    /*
     * fingerprint get image quality for factory test.
     */
    void (*getImageQuality)(struct fingerprint_device *dev);

    /*
     * fingerprint sensor wait homekey abort for cannel wait homekey.
     */
    void (*homekey_abort)(void);

    /*
     * fingerprint sensor wait homekey down.
     * Blocking function, if wait error return false, homekey down return true.
     */
    void (*waitHomekeyDown)(struct fingerprint_device *dev);
    #endif /* OPLUS_FEATURE_FINGERPRINT */

    int (*pause_enroll)(struct fingerprint_device *dev);
    int (*continue_enroll)(struct fingerprint_device *dev);
    void (*wait_touch_down)(struct fingerprint_device *dev);
    int (*pause_identify)(struct fingerprint_device *dev);
    int (*continue_identify)(struct fingerprint_device *dev);

    #ifdef OPLUS_FEATURE_FINGERPRINT
    int (*getAlikeyStatus)(struct fingerprint_device *dev);   //  return  0 ok,  1 faile, -1 not support it..
    #endif /*OPLUS_FEATURE_FINGERPRINT*/

    #ifdef OPLUS_FEATURE_FINGERPRINT
    int (*get_enrollment_total_times)(struct fingerprint_device *dev);
    #endif /*OPLUS_FEATURE_FINGERPRINT*/

    #ifdef OPLUS_FEATURE_FINGERPRINT
    int (*getBadPixels)(struct fingerprint_device *dev);
    #endif /* OPLUS_FEATURE_FINGERPRINT */

    int (*setScreenState)(struct fingerprint_device *dev, uint32_t screen_state);
    int (*touchDown)(struct fingerprint_device *dev);
    int (*touchUp)(struct fingerprint_device *dev);
    int (*sendFingerprintCmd)(struct fingerprint_device *dev, int32_t cmd_id, int8_t* in_buf, uint32_t size);
    /* Reserved for backward binary compatibility */
    void *reserved[4];
} fingerprint_device_t;

typedef struct fingerprint_module {
    /**
     * Common methods of the fingerprint module. This *must* be the first member
     * of fingerprint_module as users of this structure will cast a hw_module_t
     * to fingerprint_module pointer in contexts where it's known
     * the hw_module_t references a fingerprint_module.
     */
    struct hw_module_t common;
} fingerprint_module_t;

#endif  /* ANDROID_INCLUDE_HARDWARE_FINGERPRINT_H */
