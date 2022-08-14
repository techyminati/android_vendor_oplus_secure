//
//    Copyright 2017 Egis Technology Inc.
//
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Discretix in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//
#ifndef ETS_FINGERPRINT_HEADER
#define ETS_FINGERPRINT_HEADER
#include "egis_rbs_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fingerprint errors are meant to tell the framework to terminate the current
 * operation and ask
 * for the user to correct the situation. These will almost always result in
 * messaging and user
 * interaction to correct the problem.
 *
 * For example, FINGERPRINT_ERROR_CANCELED should follow any acquisition message
 * that results in
 * a situation where the current operation can't continue without user
 * interaction. For example,
 * if the sensor is dirty during enrollment and no further enrollment progress
 * can be made,
 * send FINGERPRINT_ACQUIRED_IMAGER_DIRTY followed by
 * FINGERPRINT_ERROR_CANCELED.
 */

typedef enum egis_fingerprint_error {
	EGIS_FINGERPRINT_ERROR_HW_UNAVAILABLE =
	    1, /* The hardware has an error that can't be resolved. */
	EGIS_FINGERPRINT_ERROR_UNABLE_TO_PROCESS =
	    2, /* Bad data; operation can't continue */
	EGIS_FINGERPRINT_ERROR_TIMEOUT =
	    3, /* The operation has timed out waiting for user input. */
	EGIS_FINGERPRINT_ERROR_NO_SPACE =
	    4, /* No space available to store a template */
	EGIS_FINGERPRINT_ERROR_CANCELED =
	    5, /* The current operation can't proceed. See above. */
	EGIS_FINGERPRINT_ERROR_UNABLE_TO_REMOVE =
	    6, /* fingerprint with given id can't be removed */
	EGIS_FINGERPRINT_ERROR_VENDOR_BASE =
	    1000 /* vendor-specific error messages start here */
} egis_fingerprint_error_t;

/*
 * Fingerprint acquisition info is meant as feedback for the current operation.
 * Anything but
 * FINGERPRINT_ACQUIRED_GOOD will be shown to the user as feedback on how to
 * take action on the
 * current operation. For example, FINGERPRINT_ACQUIRED_IMAGER_DIRTY can be used
 * to tell the user
 * to clean the sensor.  If this will cause the current operation to fail, an
 * additional
 * FINGERPRINT_ERROR_CANCELED can be sent to stop the operation in progress
 * (e.g. enrollment).
 * In general, these messages will result in a "Try again" message.
 */

typedef enum egis_fingerprint_acquired_info {
	EGIS_FINGERPRINT_ACQUIRED_GOOD = 0,
	EGIS_FINGERPRINT_ACQUIRED_PARTIAL =
	    1, /* sensor needs more data, i.e. longer swipe. */
	EGIS_FINGERPRINT_ACQUIRED_INSUFFICIENT =
	    2,					    /* image doesn't contain enough detail for recognition*/
	EGIS_FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3, /* sensor needs to be cleaned */
	EGIS_FINGERPRINT_ACQUIRED_TOO_SLOW =
	    4, /* mostly swipe-type sensors; not enough data collected */
	EGIS_FINGERPRINT_ACQUIRED_TOO_FAST =
	    5, /* for swipe and area sensors; tell user to slow down*/
	ACQUIRED_VENDOR_BASE =
	    1000,
	ACQUIRED_TOO_SIMILAR =
	    1001,
	/*for the same fingerprint as enrolled*/
	ACQUIRED_ALREADY_ENROLLED =
	    1002,

	EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE =
	    1100, /* vendor-specific acquisition messages start here */
	FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT =
	    EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 1,
	FINGERPRINT_ACQUIRED_FINGER_DOWN = EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 2,
	FINGERPRINT_ACQUIRED_FINGER_UP = EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 3,
	FINGERPRINT_ACQUIRED_INPUT_TOO_LONG =
	    EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 4,
	FINGERPRINT_ACQUIRED_DUPLICATE_FINGER =
	    EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 5,
	FINGERPRINT_ACQUIRED_DUPLICATE_AREA =
	    EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 6,
	FINGERPRINT_ACQUIRED_LOW_COVER = EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 7,
	FINGERPRINT_ACQUIRED_BAD_IMAGE = EGIS_FINGERPRINT_ACQUIRED_VENDOR_BASE + 8
} egis_fingerprint_acquired_info_t;

/*
 *Additional extended message definition for oplus
 *Used to report the status of the screen.
*/

typedef enum Egis_FingerprintScreenStatus {
	EGIS_FINGERPRINT_SCREEN_OFF = 0,
	EGIS_FINGERPRINT_SCREEN_ON = 1,
} Egis_FingerprintScreenState;

typedef enum egis_engineering_info_acquire_action {
	EGIS_FINGERPRINT_GET_IMAGE_SNR = 0,
	EGIS_FINGERPRINT_GET_IMAGE_QUALITY = 1,
	EGIS_FINGERPRINT_GET_BAD_PIXELS = 2,
	EGIS_FINGERPRINT_SELF_TEST = 3
} egis_engineering_info_acquire_action_t;

typedef enum egis_fingerprint_engineering_info_type {
	EGIS_FINGERPRINT_IMAGE_SNR = 1,
	EGIS_FINGERPRINT_IMAGE_QUALITY = 2,
	EGIS_FINGERPRINT_BAD_PIXELS = 3,
} egis_fingerprint_engineering_info_type_t;

typedef enum egis_engineering_parameter_group {
	EGIS_SUCCESSED = 0,
	EGIS_IMAGE_QUALITY = 1,
	EGIS_SNR_SUCCESSED = 2,
	EGIS_IMAGE_SNR = 3,
	EGIS_BAD_PIXEL_NUM = 4,
	EGIS_LOCAL_BAD_PIXEL_NUM = 5,
	EGIS_M_ALL_TILT_ANGLE = 6,
	EGIS_M_BLOCK_TILT_ANGLE_MAX = 7,
	EGIS_LOCAL_BIG_PIXEL_NUM = 8,
	EGIS_QUALITY_PASS = 9,
} egis_engineering_parameter_group_t;

typedef enum Operation_Type {
	FINGERPRINT_OPERATION_VERIFY,
	FINGERPRINT_OPERATION_OTHERS,
} Operation_Type;

/**
 * Data format for an authentication record used to prove successful
 * authentication.
 */
typedef struct __attribute__((__packed__)) {
	uint8_t version;  // Current version is 0
	uint64_t challenge;
	uint64_t user_id;	     // secure user ID, not Android user ID
	uint64_t authenticator_id;    // secure authenticator ID
	uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
	uint64_t timestamp;	   // in network order
	uint8_t hmac[32];
} egis_hw_auth_token_t;

typedef struct {
	void (*on_enroll_result)(void* context, uint32_t fid, uint32_t user_id,
				 uint32_t remaining);
	void (*on_acquired)(void* context, int info);
	void (*on_authenticated)(void* context, uint32_t fid, uint32_t user_id,
				 const uint8_t* token, uint32_t size_token);
	void (*on_error)(void* context, int code);
	void (*on_removed)(void* context, uint32_t fid, uint32_t user_id,
			   uint32_t remaining);
	void (*on_enumerate)(void* context, uint32_t fid, uint32_t user_id,
			     uint32_t remaining);
	//void (*trans_callback)(int event_id, int value1, int value2, uint8_t* buffer, int buffer_size);
	void (*on_sync_templates)(void* context, uint32_t* fids, uint32_t fids_count, uint32_t user_id);

	void (*on_touch_up)(void* context);
	void (*on_touch_down)(void* context);
	void (*on_monitor_event_triggered)(void* context, uint32_t type, char* data);
	void (*on_image_info_acquired)(void* context, uint32_t type, uint32_t quality, uint32_t match_score);
	void (*on_engineering_info_update)(void* context, uint32_t type, int result, int ext_info);
} fingerprint_callback_t;

typedef struct egis_fingerprint_hal_device_t {
	uint64_t op_id;
	uint32_t user_id;
	uint64_t challenge;
	uint64_t authenticator_id;
	pthread_mutex_t lock;
	egis_hw_auth_token_t hat;
	int matched_fid;
	fingerprint_callback_t* callback;
	void* callback_context;
} egis_fingerprint_hal_device_t;

typedef struct lib_handle_t {
	void* ets_fp_libhandle;
	int (*rbs_initialize)(unsigned char* in_data, unsigned int in_data_len);
	int (*rbs_uninitialize)(void);
	int (*rbs_cancel)(void);
	int (*rbs_pause)(void);
	int (*rbs_continue)(void);
	int (*rbs_clean_up)(void);
	int (*rbs_active_user_group)(unsigned int user_id,
				     const char* data_path);
	int (*rbs_set_data_path)(unsigned int data_type, const char* data_path,
				 unsigned int path_len);
	int (*rbs_chk_secure_id)(unsigned int user_id,
				 unsigned long long secure_id);
	int (*rbs_pre_enroll)(unsigned int user_id,
			      unsigned int fingerprint_id);
	int (*rbs_enroll)(void);
	int (*rbs_post_enroll)(void);
	int (*rbs_chk_auth_token)(unsigned char* token, unsigned int len);
	int (*rbs_get_authenticator_id)(unsigned long long* id);
	int (*rbs_authenticator)(unsigned int user_id, unsigned int* finger_ids,
				 unsigned int finger_count,
				 unsigned long long challenge);
	int (*rbs_remove_fingerprint)(unsigned int user_id,
				      unsigned int finger_id);
	int (*rbs_get_fingerprint_ids)(unsigned int user_id,
				       int* fingerprint_ids,
				       int* fingerprint_count);
	int (*rbs_navigation)(operation_callback_t operation_callback);
	int (*rbs_set_on_callback_proc)(
	    operation_callback_t do_native_callback);
	int (*rbs_extra_api)(int type, unsigned char* in_buffer,
			     int in_buffer_size, unsigned char* out_buffer,
			     int* out_buffer_size);
	int (*rbs_set_touch_event_listener)(void);
	int (*rbs_get_screen_state)(int* screen_state);
	int (*rbs_keymode_enable)(unsigned int enable);
} lib_handle_t;

int fingerprint_open(egis_fingerprint_hal_device_t** device);
void fingerprint_set_callback(egis_fingerprint_hal_device_t** device,
			      const fingerprint_callback_t* callback,
			      void* callback_context);
int fingerprint_close(egis_fingerprint_hal_device_t* dev);

uint64_t fingerprint_pre_enroll(egis_fingerprint_hal_device_t* device);
int fingerprint_enroll(egis_fingerprint_hal_device_t* device,
		       const uint8_t* hat_data, uint32_t user_id,
		       uint32_t timeout_sec);
int fingerprint_post_enroll(egis_fingerprint_hal_device_t* device);
int fingerprint_cancel(void);
int fingerprint_enumerate(void);

uint64_t fingerprint_get_auth_id(egis_fingerprint_hal_device_t* device);
int fingerprint_set_active_group(uint32_t user_id, const char* path);
int fingerprint_remove(uint32_t user_id, uint32_t fid);
int fingerprint_authenticate(egis_fingerprint_hal_device_t* device,
			     uint64_t operation_id, uint32_t user_id);
int do_extra_api_in(int cmd_id, const unsigned char* in_buf, size_t in_buf_size, unsigned char* out_data, int* out_data_len);

int extra_set_on_callback_proc(operation_callback_t callback);
int fingerprint_get_enrollment_total_times();
int fingerprint_clean_up();
int fingerprint_pause_enroll();
int fingerprint_continue_enroll();
int fingerprint_set_touch_event_listener();
int fingerprint_dynamically_config_log(uint32_t on);
int fingerprint_pause_identify();
int fingerprint_continue_identify();
int fingerprint_get_alikey_status();
int fingerprint_set_screen_state(int32_t screen_state);
int fingerprint_get_engineering_info(uint32_t type);
int fingerprint_keymode_enable(egis_fingerprint_hal_device_t* device, uint32_t enable);
int start_inline_service();
#ifdef __cplusplus
}
#endif

#endif  // ETS_FINGERPRINT_HEADER
