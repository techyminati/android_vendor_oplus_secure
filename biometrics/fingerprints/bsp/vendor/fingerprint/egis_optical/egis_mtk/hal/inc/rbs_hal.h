#ifndef ETS_HAL_HEADER
#define ETS_HAL_HEADER
#include "egis_rbs_api.h"
#include <hardware/hw_auth_token.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	void (*on_touch_down)(void* context);
	void (*on_touch_up)(void* context);
	void (*on_monitor_event_triggered)(void* context, uint32_t type, char* data);
	void (*on_image_info_acquired)(void* context, uint32_t type, uint32_t quality, uint32_t match_score);
	void (*on_sync_templates)(void* context, uint32_t* fingerIds, int fingerIds_size, uint32_t groupId);
	void (*on_engineering_info_updated)(void* context, uint32_t lenth, uint32_t* keys, int keys_size, uint32_t* values, int values_size);
	void (*on_fingerprint_cmd)(void* context, int32_t cmdId, int8_t* result, uint32_t resultLen);
	void (*on_send_dcsmsg)(void* context, int32_t cmdId, int8_t* dcsmsg, uint32_t dcsmsgLen);
	void (*set_action)(void* context, uint32_t type, uint32_t time_out);
} fingerprint_callback_t;

typedef struct egis_fingerprint_hal_device_t {
	uint64_t op_id;
	uint32_t user_id;
	uint64_t challenge;
	uint64_t authenticator_id;
	pthread_mutex_t lock;
	hw_auth_token_t hat;
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
	int (*rbs_set_on_callback_proc)(
	    operation_callback_t do_native_callback);
	int (*rbs_extra_api)(int type, unsigned char* in_buffer,
			     int in_buffer_size, unsigned char* out_buffer,
			     int* out_buffer_size);
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
int do_extra_api_in(int cmd_id, const unsigned char* in_buf, size_t in_buf_size);
int do_extra_api(int cmd_id, const unsigned char* in_buf, size_t in_buf_size,
                 unsigned char* out_buf, size_t* out_buf_size);

int extra_set_on_callback_proc(operation_callback_t callback);
int fingerprint_set_screen_state(int32_t screen_state);
int fingerprint_get_engineering_info(uint32_t type);
int fingerprint_sendFingerprintCmd(uint32_t type, const signed char* in_buf, size_t in_buf_size);
int fingerprint_get_enrollment_total_times();
int fingerprint_pause_enroll();
int fingerprint_continue_enroll();

#ifdef __cplusplus
}
#endif

#endif  // ETS_FINGERPRINT_HEADER
