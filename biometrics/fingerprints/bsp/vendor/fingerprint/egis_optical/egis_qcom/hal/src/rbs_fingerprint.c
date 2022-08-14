#include "plat_log.h"
#include <dlfcn.h>
#include <errno.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "android_fp_types.h"
#include "permission_ops.h"
#include "rbs_fingerprint.h"
#include "response_def.h"
#include "common_definition.h"
#include "rbs_oplus.h"
#include "rbs_hal.h"
#ifdef QSEE
#include "qsee_keymaster.h"
#endif

#define LOG_TAG "egis_fp_hal"

#define FINGERPRINT_VERSION "1.0.2"
extern int egis_tp_enable();
extern int egis_tp_disable();
#define ENROLLMENT_TOTAL_TIMES 17
#define EMPTY_USER_ID 9999
#define MAX_FP_PER_USER 5
#define MSG_QUEUE_SIZE 64
#define ENROLL_SAMPLES 16
#define FINGERPRINT_VTS_ERROR 0  //for VTS test. Do NOT modify.
#define FINGERPRINT_NORMAL_SCAN 701
#define FINGERPRINT_WKBOX_ON 703
#define FINGERPRINT_BKBOX_ON 704
#define FINGERPRINT_CHART_ON 705
#define FINGERPRINT_FLASH_TEST 717

optical_cali_result_t g_test_cali_result = {0};

unsigned int g_active_user_id = EMPTY_USER_ID;

egis_fingerprint_hal_device_t *g_dev = NULL;

lib_handle_t *g_lib_handle = NULL;

static int fp_get_lib_sym(lib_handle_t *lib_handle);

static int fp_get_lib_sym(lib_handle_t *lib_handle)
{
	lib_handle->ets_fp_libhandle = dlopen("libRbsFlow.so", RTLD_NOW);
	if (lib_handle->ets_fp_libhandle == NULL) {
		egislog_e("failed to load libRbsFlow.so");
		char* error_msg = dlerror();
		egislog_e("error_msg %s", error_msg);
		return -1;
	}
	// initialization
	*(void **)(&lib_handle->rbs_initialize) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_initialize");
	if (lib_handle->rbs_initialize == NULL) {
		egislog_e("dlsym: Error Loading rbs_initialize");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_uninitialize) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_uninitialize");
	if (lib_handle->rbs_uninitialize == NULL) {
		egislog_e("dlsym: Error Loading rbs_uninitialize");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_cancel) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_cancel");
	if (lib_handle->rbs_cancel == NULL) {
		egislog_e("dlsym: Error Loading rbs_cancel");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_pause) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_pause");
	if (lib_handle->rbs_pause == NULL) {
		egislog_e("dlsym: Error Loading rbs_pause");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_continue) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_continue");
	if (lib_handle->rbs_continue == NULL) {
		egislog_e("dlsym: Error Loading rbs_continue");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_active_user_group) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_active_user_group");
	if (lib_handle->rbs_active_user_group == NULL) {
		egislog_e("dlsym: Error Loading rbs_active_user_group");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_set_data_path) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_set_data_path");
	if (lib_handle->rbs_set_data_path == NULL) {
		egislog_e("dlsym: Error Loading rbs_set_data_path");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_chk_secure_id) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_chk_secure_id");
	if (lib_handle->rbs_chk_secure_id == NULL) {
		egislog_e("dlsym: Error Loading rbs_chk_secure_id");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_pre_enroll) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_pre_enroll");
	if (lib_handle->rbs_pre_enroll == NULL) {
		egislog_e("dlsym: Error Loading rbs_pre_enroll");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_enroll) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_enroll");
	if (lib_handle->rbs_enroll == NULL) {
		egislog_e("dlsym: Error Loading rbs_enroll");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_post_enroll) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_post_enroll");
	if (lib_handle->rbs_post_enroll == NULL) {
		egislog_e("dlsym: Error Loading rbs_post_enroll");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_chk_auth_token) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_chk_auth_token");
	if (lib_handle->rbs_chk_auth_token == NULL) {
		egislog_e("dlsym: Error Loading rbs_chk_auth_token");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_authenticator) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_authenticator");
	if (lib_handle->rbs_authenticator == NULL) {
		egislog_e("dlsym: Error Loading rbs_authenticator");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_remove_fingerprint) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_remove_fingerprint");
	if (lib_handle->rbs_remove_fingerprint == NULL) {
		egislog_e("dlsym: Error Loading rbs_remove_fingerprint");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_get_fingerprint_ids) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_get_fingerprint_ids");
	if (lib_handle->rbs_get_fingerprint_ids == NULL) {
		egislog_e("dlsym: Error Loading rbs_get_fingerprint_ids");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_get_authenticator_id) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_get_authenticator_id");
	if (lib_handle->rbs_get_authenticator_id == NULL) {
		egislog_e("dlsym: Error Loading rbs_get_authenticator_id");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_navigation) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_navigation");
	if (lib_handle->rbs_navigation == NULL) {
		egislog_e("dlsym: Error Loading rbs_navigation");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_set_on_callback_proc) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_set_on_callback_proc");
	if (lib_handle->rbs_set_on_callback_proc == NULL) {
		egislog_e("dlsym: Error Loading rbs_set_on_callback_proc");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_extra_api) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_extra_api");
	if (lib_handle->rbs_extra_api == NULL) {
		egislog_e("dlsym: Error Loading rbs_extra_api");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	return FINGERPRINT_RES_SUCCESS;
}

static unsigned long get_64bit_rand()
{
	return (((unsigned long)rand()) << 32) | ((unsigned long)rand());
}

static unsigned int generate_fingerprint_id() { return (rand() & 0x7FFFFFFFF); }

static void on_enroll_progress(unsigned int fid, int remaining)
{
	egislog_d("on_enroll_progress fid = %d, remaining = %d", fid, remaining);
	g_dev->callback->on_enroll_result(g_dev->callback_context, fid, g_dev->user_id, remaining);
}

static void on_acquired_msg(int info)
{
	egislog_d("on_acquired %d", info);
	g_dev->callback->on_acquired(g_dev->callback_context, info);
}

static void on_error_msg(int err_code)
{
	egislog_e("on_error %d", err_code);

	g_dev->callback->on_error(g_dev->callback_context, err_code);
}

static void on_touch_down_msg()
{
	egislog_d("on_touch_down");

	g_dev->callback->on_touch_down(g_dev->callback_context);
}

static void on_touch_up_msg()
{
	egislog_d("on_touch_up");

	g_dev->callback->on_touch_up(g_dev->callback_context);
}

static void on_monitor_event_triggered_msg(uint32_t type, char* data)
{
	egislog_d("on_monitor_event_triggered type = %d, data %s", type, data);

	g_dev->callback->on_monitor_event_triggered(g_dev->callback_context, type, data);
}

static void on_image_info_acquired_msg(uint32_t type, uint32_t quality, uint32_t match_score)
{
	egislog_d("on_image_info_acquired type = %d, quality = %d, match_score = %d", type, quality, match_score);

	g_dev->callback->on_image_info_acquired(g_dev->callback_context, type, quality, match_score);
}

static void on_sync_templates_msg(uint32_t* fingerIds, int fingerIds_size, uint32_t groupId)
{
	egislog_d("on_sync_templates fingerIds = %d, fingerIds_size = %d, groupId = %d", fingerIds, fingerIds_size, groupId);

	g_dev->callback->on_sync_templates(g_dev->callback_context, fingerIds, fingerIds_size, groupId);
}

static void on_engineering_info_updated_msg(uint32_t lenth, uint32_t* keys, int keys_size, uint32_t* values, int values_size)
{
	// egislog_d("on_engineering_info_updated fingerIds = %d, groupId = %d", fingerIds, groupId);

	g_dev->callback->on_engineering_info_updated(g_dev->callback_context, lenth, keys, keys_size, values, values_size);
}

static void on_fingerprint_cmd_msg(int32_t cmdId, int8_t* result, int result_size)
{
	// egislog_d("on_fingerprint_cmd cmdId = %d, fingerIds = %d, resultLen = %d", cmdId, fingerIds, resultLen);

	g_dev->callback->on_fingerprint_cmd(g_dev->callback_context, cmdId, result, (uint32_t)result_size);
}

int fingerprint_close(egis_fingerprint_hal_device_t *dev)
{
	egislog_i("fingerprint_close enter");
	if (dev) {
		free(dev);
		return g_lib_handle->rbs_uninitialize();
	} else {
		return -1;
	}
}

uint64_t fingerprint_pre_enroll(egis_fingerprint_hal_device_t *device)
{
	device->challenge = get_64bit_rand();

	egislog_i("fingerprint_pre_enroll challenge = %" PRIu64 "", device->challenge);

	return device->challenge;
}

int fingerprint_set_screen_state(int32_t screen_state)
{
	egislog_i("fingerprint_set_screen_state enter,%d" , screen_state);
	unsigned char in_data[sizeof(int)];
	int out_data_len = 0;
	int* ptr = (int*)in_data;

	int pid = PID_HOST_TOUCH;
	ptr[0] = screen_state? 202:201;
	do_extra_api(pid, in_data, sizeof(int), NULL, &out_data_len);
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_get_engineering_info(uint32_t type)
{
	egislog_i("fingerprint_get_engineering_info enter, type = %d", type);
	unsigned char in_data[sizeof(int) * 2];
	unsigned char out_data[64];
	int out_data_len = 64;
	int* ptr = (int*)in_data;
	int pid = 0;
	int retval = FINGERPRINT_RES_SUCCESS;

	switch (type)
    {
		case 0:

		case 1:  //  SENSORTEST_IMAGE_QTY
			pid = PID_INLINETOOL;
			ptr[0] = 1200;
			break;

		case 2:
		case 3:
			pid = PID_7XX_INLINETOOL;
			ptr[0] = FP_INLINE_7XX_NORMALSCAN;
			break;
		case 4:

		default:
			egislog_e("not support test cmd");
			break;
    }

	retval = do_extra_api(pid, in_data, sizeof(int), out_data, &out_data_len);
	egislog_i(" fingerprint_get_engineering_info end, retval = %d", retval);
	return 0;
}

int fingerprint_sendFingerprintCmd(uint32_t type, const signed char* in_buf, size_t in_buf_size)
{
	egislog_i("fingerprint_sendFingerprintCmd enter, type = %d", type);
	unsigned char in_data[sizeof(int) * 2];
	unsigned char out_data[64];
	int out_data_len = 64;
	int inlineTestToolCodes = PID_7XX_INLINETOOL;
	int* ptr = (int*)in_data;
	int retval = FINGERPRINT_RES_SUCCESS;

	switch (type)
    {
		case 0:
			inlineTestToolCodes = PID_7XX_INLINETOOL;
			ptr[0] = 701;
			break;

		case 1:
			inlineTestToolCodes = PID_7XX_INLINETOOL;
			ptr[0] = 703;
			break;

		case 2:
			inlineTestToolCodes = PID_7XX_INLINETOOL;
			ptr[0] = 704;
			break;

		case 3:
			inlineTestToolCodes = PID_7XX_INLINETOOL;
			ptr[0] = 717;
			break;

		case 257:
			inlineTestToolCodes = PID_7XX_INLINETOOL;
			ptr[0] = 705;
			break;

		case 513:
			inlineTestToolCodes = PID_INLINETOOL;
			ptr[0] = 1201; /* Aging test*/
			break;

		case 514:
			return fingerprint_cancel();

		case 600:
			inlineTestToolCodes = PID_SYSUNLOCKTOOL_NAME;
			retval = do_extra_api(inlineTestToolCodes, (unsigned char*)in_buf, in_buf_size, out_data, &out_data_len);
			goto EXIT;

		case 601:
			inlineTestToolCodes = PID_SYSUNLOCKTOOL_TOTAL_COUNT;
			retval = do_extra_api(inlineTestToolCodes, (unsigned char*)in_buf, in_buf_size, out_data, &out_data_len);
			goto EXIT;

		case 602:
			inlineTestToolCodes = PID_SYSUNLOCKTOOL_MATCH_COUNT;
			retval = do_extra_api(inlineTestToolCodes, (unsigned char*)in_buf, in_buf_size, out_data, &out_data_len);
			goto EXIT;

		case 603:
			inlineTestToolCodes = PID_SYSUNLOCKTOOL_NOT_MATCH_COUNT;
			retval = do_extra_api(inlineTestToolCodes, (unsigned char*)in_buf, in_buf_size, out_data, &out_data_len);
			goto EXIT;

		default:
			egislog_e("not support test cmd");
			retval = FINGERPRINT_RES_INVALID_PARAM;
			goto EXIT;
    }

	retval = do_extra_api(inlineTestToolCodes, in_data, sizeof(int), out_data, &out_data_len);
	egislog_i(" fingerprint_sendFingerprintCmd end, retval = %d", retval);

	if ((type==0) && (retval == 0))
	{
		ptr[0] = 702;
		retval = do_extra_api(inlineTestToolCodes, in_data, sizeof(int), out_data, &out_data_len);
		egislog_i(" fingerprint_sendFingerprintCmd end, retval = %d", retval);
	}

EXIT:
	return retval;
}

int fingerprint_enroll(egis_fingerprint_hal_device_t *device,
		       const uint8_t *hat_data, uint32_t __unused gid,
		       uint32_t __unused timeout_sec)
{
	int retval;
	unsigned int fid;
	egislog_i("fingerprint_enroll enter! fingerptint_version: %s",
		  FINGERPRINT_VERSION);
	hw_auth_token_t *hat = (hw_auth_token_t *)hat_data;

	egis_fingerprint_hal_device_t *dev =
	    (egis_fingerprint_hal_device_t *)device;

	egis_tp_enable();

	if ((hat == NULL) || (hat->timestamp == 0)) {
		egislog_e("hat is null");
		on_error_msg(ERROR_UNABLE_TO_PROCESS);
		return FINGERPRINT_VTS_ERROR;
	}

	if (hat && (hat->challenge == dev->challenge)) {
		egislog_d("challenge matched");
	} else {
		egislog_e("challenge not match");
		on_error_msg(ERROR_UNABLE_TO_PROCESS);
		return FINGERPRINT_VTS_ERROR;
	}

	if (hat->version != HW_AUTH_TOKEN_VERSION) {
		egislog_e("invalid hat->version = %d", hat->version);
		return -EPROTONOSUPPORT; /* Protocol not supported */
	}

	if (hat->challenge != dev->challenge &&
	    !(hat->authenticator_type & HW_AUTH_FINGERPRINT)) {
		egislog_e("Not super-user");
		return -EPERM; /* Not super-user */
	}

	retval = g_lib_handle->rbs_chk_auth_token((unsigned char *)hat,
						  sizeof(hw_auth_token_t));
	if (FINGERPRINT_RES_SUCCESS != retval) {
		egislog_e("authtoken check failed, retval = %d", retval);
		on_error_msg(FINGERPRINT_ERROR_TIMEOUT);
		return FINGERPRINT_RES_SUCCESS;
	}

	retval = g_lib_handle->rbs_chk_secure_id(gid, hat->user_id);
	if (retval != FINGERPRINT_RES_SUCCESS) {
		egislog_e("secureid check failed %d", retval);
		if (retval == FINGERPRINT_RES_SECURE_ID_NOTMATCH) {
			retval = g_lib_handle->rbs_remove_fingerprint(gid, 0);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				egislog_e("remove all fingerprints failed %d", retval);
				return -EPERM;
			}

			retval = g_lib_handle->rbs_chk_secure_id(gid, hat->user_id);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				egislog_e("secureid check failed %d", retval);
				return -EPERM;
			}

			egislog_d("Remove all fingerprints and secureid check OK");
		} else {
			return -EPERM;
		}
	}

	do {
		fid = generate_fingerprint_id();

		retval = g_lib_handle->rbs_pre_enroll(gid, fid);
	} while (FINGERPRINT_RES_SUCCESS != retval &&
		 FINGERPRINT_RES_NO_SPACE != retval);

	if (FINGERPRINT_RES_NO_SPACE == retval) {
		egislog_e("g_lib_handle->rbs_pre_enroll failed, no space");
		on_error_msg(FINGERPRINT_ERROR_CANCELED);
		return FINGERPRINT_RES_SUCCESS;
	}

	retval = do_extra_api_in(PID_SET_ENROLL_TIMEOUT, (unsigned char*)&timeout_sec, sizeof(uint32_t));
	if (FINGERPRINT_RES_SUCCESS != retval) {
		egislog_e("g_lib_handle->rbs_enroll return = %d", retval);
		on_error_msg(FINGERPRINT_ERROR_CANCELED);
	}

	retval = g_lib_handle->rbs_enroll();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		egislog_e("g_lib_handle->rbs_enroll return = %d", retval);
		on_error_msg(FINGERPRINT_ERROR_CANCELED);
	}

	if (FINGERPRINT_RES_HW_UNAVALABLE == retval) {
		on_error_msg(FINGERPRINT_ERROR_HW_UNAVAILABLE);
	}

	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_post_enroll(egis_fingerprint_hal_device_t *device)
{
	egislog_i("fingerprint_post_enroll enter");
	egis_fingerprint_hal_device_t *dev =
	    (egis_fingerprint_hal_device_t *)device;

	dev->challenge = 0;
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_enumerate()
{
	int retval;
	int tmp_ids[MAX_FP_PER_USER] = {0};
	int tmp_ids_count = 0;
	int loop_index;

	egislog_i("fingerprint_enumerate enter");

	if (EMPTY_USER_ID == g_dev->user_id) {
		egislog_e("fingerprint_enumerate user id empty");
		return -1;  // caller should call set_active_group first
	}

	retval = g_lib_handle->rbs_get_fingerprint_ids(g_dev->user_id, tmp_ids, &tmp_ids_count);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		egislog_e("g_lib_handle->rbs_get_fingerprint_ids return = %d", retval);
		return retval;
	}

	if (tmp_ids_count == 0) {
		egislog_e("finger list is null");
		g_dev->callback->on_enumerate(g_dev->callback_context, 0, g_dev->user_id, 0);
	}

	for (loop_index = 0; loop_index < tmp_ids_count; loop_index++) {
		g_dev->callback->on_enumerate(
		    g_dev->callback_context, tmp_ids[loop_index],
		    g_dev->user_id, tmp_ids_count - loop_index - 1);
	}

	return FINGERPRINT_RES_SUCCESS;
}

uint64_t fingerprint_get_auth_id(egis_fingerprint_hal_device_t *device)
{
	int retval;
	unsigned long long authenticator_id = 0;

	egislog_i("fingerprint_get_auth_id enter");
	if (device == NULL) {
		egislog_e("NULL device on open");
		return -EINVAL; /* Invalid argument */
	}

	retval = g_lib_handle->rbs_get_authenticator_id(&authenticator_id);

	egislog_d("rbs_get_authenticator_id retval = %d, authenticator_id =%llu", retval, authenticator_id);

	return authenticator_id;
}

int fingerprint_cancel()
{
	int retval;

	egislog_i("fingerprint_cancel enter");
	retval = g_lib_handle->rbs_cancel();
	on_error_msg(FINGERPRINT_ERROR_CANCELED);
	egis_tp_disable();
	egislog_d("g_lib_handle->rbs_cancel return = %d", retval);
	return retval;
}

int fingerprint_remove(uint32_t gid,
		       uint32_t fid)
{
	int retval;
	int fingercount = 0;
	int tmp_ids[MAX_FP_PER_USER] = {0};
	int tmp_ids_count = 0;

	egislog_i("fingerprint_remove enter, gid = %d, fid = %d", gid, fid);

	if (0 == fid) {
		retval = g_lib_handle->rbs_get_fingerprint_ids(gid, tmp_ids, &tmp_ids_count);
		egislog_d("remove all. retval=%d, finger_count=%d", retval, tmp_ids_count);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			on_error_msg(FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
		}
	}

	if (tmp_ids_count == 0 && 0 == fid) {
		egislog_e("finger list is null");
		g_dev->callback->on_removed(g_dev->callback_context, 0, gid, 0);
		return FINGERPRINT_RES_SUCCESS;
	}

	retval = g_lib_handle->rbs_remove_fingerprint(gid, fid);

	if (FINGERPRINT_RES_SUCCESS == retval) {
		if (0 == fid) {
			for (fingercount = 0; fingercount < tmp_ids_count; fingercount++) {
				g_dev->callback->on_removed(
				    g_dev->callback_context,
				    tmp_ids[fingercount], gid,
				    tmp_ids_count - fingercount - 1);
			}

			on_error_msg(FINGERPRINT_ERROR_CANCELED);
		} else {
			retval = g_lib_handle->rbs_get_fingerprint_ids(
			    gid, tmp_ids, &tmp_ids_count);
			if (FINGERPRINT_RES_SUCCESS != retval) {
				on_error_msg(FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
			}
			g_dev->callback->on_removed(g_dev->callback_context, fid, gid, tmp_ids_count);
		}
		// g_dev->callback->on_removed(g_dev->callback_context, 0, gid,
		// tmp_ids_count);
	} else {
		egislog_e("fingerprint_remove return = %d", retval);
		on_error_msg(FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
	}
	egislog_d("fingerprint remining = %d", tmp_ids_count);

	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_set_active_group(uint32_t gid, const char *store_path)
{
	int retval;

	egislog_i("fingerprint_set_active_group enter");

	g_dev->user_id = gid;

	retval =
	    g_lib_handle->rbs_active_user_group(gid, (const char *)store_path);

	egislog_d("fingerprint_set_active_group return = %d", retval);
	return retval;
}

int fingerprint_authenticate(egis_fingerprint_hal_device_t *device,
			     uint64_t operation_id, uint32_t gid)
{
	egislog_i("fingerprint_authenticate enter, operation_id = %" PRIu64 ", gid = %u", operation_id, gid);

	int retval;
	egis_tp_enable();
	egis_fingerprint_hal_device_t *dev =
	    (egis_fingerprint_hal_device_t *)device;
	dev->op_id = operation_id;

	g_dev->user_id = gid;

	retval = g_lib_handle->rbs_authenticator(gid, NULL, 0, operation_id);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		on_error_msg(FINGERPRINT_ERROR_CANCELED);
	}

	if (FINGERPRINT_RES_HW_UNAVALABLE == retval) {
		on_error_msg(FINGERPRINT_ERROR_HW_UNAVAILABLE);
	}

	egislog_d("fingerprint_authenticate return = %d", retval);
	return retval;
}

void event_handle(int event_id, int value1, int value2,
		   unsigned char *buffer, int buffer_size)
{
	egislog_i("event_handle event_id = %d", event_id);
			int len = 64;
			int32_t key[64];
			int32_t value[64];
			engineer_info_t* p_eng_info = NULL;
			int i = 0;

	switch (event_id) {
		case EVENT_ENROLL_OK: {
			egislog_d("Ignore legacy EVENT_ENROLL_OK. Wait for EVENT_ENROLL_REMAINING instead.");
		} break;

		case EVENT_ENROLL_REMAINING: {
			on_enroll_progress(value1, value2);
		} break;

		case EVENT_ENROLL_HIGHLY_SIMILAR: {
			on_acquired_msg(ACQUIRED_TOO_SIMILAR);
		} break;
		case EVENT_ENROLL_REDUNDANT: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_DUPLICATE_AREA);
		} break;

		case EVENT_ENROLL_DUPLICATE_FINGER: {
			on_acquired_msg(ACQUIRED_ALREADY_ENROLLED);
		} break;

		case EVENT_ENROLL_IMG_TOO_SMALL:
		case EVENT_ENROLL_FEATURE_LOW: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_TOO_SLOW);
		} break;

		case EVENT_ENROLL_TIMEOUT: {
			on_error_msg(FINGERPRINT_ERROR_TIMEOUT);
		} break;

		case EVENT_VERIFY_MATCHED:
		case EVENT_VERIFY_NOT_MATCHED: {
			g_dev->user_id = value1;
			g_dev->matched_fid = value2;
			if (sizeof(hw_auth_token_t) == buffer_size) {
				memcpy(&g_dev->hat, buffer, buffer_size);
			} else {
				egislog_e(
				    "received hat sizeof(hw_auth_token_t) = %u "
				    ", buffer_size = %d",
				    sizeof(hw_auth_token_t), buffer_size);
			}

			g_dev->callback->on_authenticated(
			    g_dev->callback_context, g_dev->matched_fid,
			    g_dev->user_id, &g_dev->hat,
			    sizeof(hw_auth_token_t));
		} break;

		case EVENT_FINGER_TOUCH: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_FINGER_DOWN);
		} break;

		case EVENT_FINGER_READY: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_GOOD);
		} break;

		case EVENT_FINGER_LEAVE: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_FINGER_UP);
		} break;

		case EVENT_IMG_BAD_QLTY: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_INSUFFICIENT);
		} break;

		case EVENT_IMG_PARTIAL:
		case EVENT_IMG_TOO_PARTIAL: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_PARTIAL);
		} break;

		case EVENT_IMG_FAST:
		case EVENT_IMG_COVER:
		case EVENT_IMG_WATER: {
			on_acquired_msg(FINGERPRINT_ACQUIRED_TOO_FAST);
		} break;

		case EVENT_FINGER_TOUCH_DOWN: {
			on_touch_down_msg();
		} break;

		case EVENT_FINGER_TOUCH_UP: {
			on_touch_up_msg();
		} break;

		// case EVENT_ENROLL_MAP:
			// g_dev->callback->trans_callback(event_id, value1, value2, buffer, buffer_size);

		case EVENT_SENSOR_OPTICAL_ENG_AGING_RESULT:
		case EVENT_SENSOR_OPTICAL_CALI_IMAGE_QTY: {
			p_eng_info = buffer; 
#if 0
			key[0] = 0;
            //value[0] = getHidlstring((value1 == 0 ? 1 : 0));
            value[0] = (value1 == 0 ? 1 : 0);

            key[1] = 1;
            //value[1] = getHidlstring(value2);
            value[1] = value2;

            key[2] = 0;
            key[3] = 0;

            value[2] = 0;
            value[3] = 0;
#endif
			len = buffer_size/(sizeof(engineer_info_t));
			for(i =0; i<len ; i++, p_eng_info++)
			{
				key[i] = p_eng_info->key;
				value[i] = p_eng_info->value;
			}
			
			on_engineering_info_updated_msg(len,
					key, len, value, len);
		} break;
		case EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT:
			egislog_d("EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT value1:%d, value2:%d", value1, value2);

			g_test_cali_result.cmd_id = value1;
			g_test_cali_result.result_length = value2;
			if (sizeof(cmd_test_result_t) == buffer_size) {
				memcpy(&g_test_cali_result.test_result, buffer, buffer_size);
			}
			on_fingerprint_cmd_msg(g_test_cali_result.cmd_id,
					g_test_cali_result.test_result.data,
					g_test_cali_result.result_length);
		default:
			break;
	};
}

int fingerprint_open(egis_fingerprint_hal_device_t **device)
{
	int retval = -1;
	int delay_offset = 0;
	unsigned long rand = 0;
	char cali_path[256] = "/data/system/users/0/fpdata\0";
	egislog_i("fingerprint_open enter");

	if (!device) {
		egislog_e("NULL device on open");
		return -EINVAL;
	}

	*(void **)&g_lib_handle = malloc(sizeof(lib_handle_t));
	if (g_lib_handle == NULL) return retval;
	retval = fp_get_lib_sym(g_lib_handle);
	if (retval != 0) {
		goto exit;
	}

	/* Workaround to open Fingerprint TA because request master key from keymaster TA */
	/* set calibration path */
	retval = g_lib_handle->rbs_set_data_path(1, (const char *)cali_path, strlen(cali_path));

	/* set callback */
	g_lib_handle->rbs_set_on_callback_proc(event_handle);

	/* initialize */
	while (delay_offset <= 9) {
		sleep(delay_offset);

#if defined(AUTHTOKEN_HMAC) && defined(QSEE)
		egislog_d("get_secure_key from Keymaster");
		uint8_t master_key[MAX_MASTERKEY_LEN] = {0};
		uint32_t master_key_len = sizeof(master_key) / sizeof(master_key[0]);

		retval = get_secure_key(master_key, &master_key_len);
		if (retval == FINGERPRINT_RES_SUCCESS)
			retval = g_lib_handle->rbs_initialize(master_key, master_key_len);
		else
			egislog_e("thread_initialize get master key fail");
#else
		retval = g_lib_handle->rbs_initialize(NULL, 0);
#endif
		if (FINGERPRINT_RES_SUCCESS == retval) {
			egislog_d("g_lib_handle->rbs_initialize success");
			break;
		}

		egislog_e("g_lib_handle->rbs_initialize failed, return = %d, offset = %d", retval, delay_offset);
		delay_offset += 3;
	}

	if (FINGERPRINT_RES_SUCCESS == retval) {
		egis_fingerprint_hal_device_t *dev = (egis_fingerprint_hal_device_t *)malloc(sizeof(egis_fingerprint_hal_device_t));
		if (dev == NULL) {
			egislog_e("Invalid argument, fingerprint_open malloc Fail, dev == NULL");
			retval = -EINVAL; /* Invalid argument */
			goto exit;
		}
		memset(dev, 0, sizeof(egis_fingerprint_hal_device_t));

		*device = dev;
		g_dev = dev;
	} else
		egislog_e("g_lib_handle->rbs_initialize = %d", retval);

exit:
	return retval;
}

void fingerprint_set_callback(egis_fingerprint_hal_device_t **device,
			      const fingerprint_callback_t *callback,
			      void *callback_context)
{
	if (*device == NULL) {
		egislog_e("NULL device on open");
		return;
	}

	(*device)->callback = (fingerprint_callback_t *)callback;
	(*device)->callback_context = callback_context;
	return;
}

#define EXTRA_OUT_SIZE 32
int do_extra_api_in(int cmd_id, const unsigned char *in_buf, size_t in_buf_size)
{
	int ret;
	int out_buf_size = EXTRA_OUT_SIZE;
	char out_buf[EXTRA_OUT_SIZE] = {0};

	egislog_i("do_extra_api_in [%d]", cmd_id);
	ret = g_lib_handle->rbs_extra_api(cmd_id, in_buf, in_buf_size, (unsigned char *)out_buf, &out_buf_size);

	return ret;
}

int do_extra_api(int cmd_id, const unsigned char* in_buf, size_t in_buf_size,
                 unsigned char* out_buf, size_t* out_buf_size) 
{
	egislog_i("do_extra_api_in [%d]", cmd_id);
    int ret = g_lib_handle->rbs_extra_api(cmd_id, in_buf, in_buf_size, out_buf, out_buf_size);
    return ret;
}

int extra_set_on_callback_proc(operation_callback_t callback)
{
	egislog_i("extra_set_on_callback_proc enter");
	g_lib_handle->rbs_set_on_callback_proc(callback);
	return 0;
}

int fingerprint_get_enrollment_total_times()
{
	egislog_i("fingerprint_get_enrollment_total_times enter");

	return ENROLLMENT_TOTAL_TIMES;
}

int fingerprint_pause_enroll()
{
	egislog_i("fingerprint_pause_enroll enter");
	egis_tp_disable();
	return g_lib_handle->rbs_pause();

}

int fingerprint_continue_enroll()
{
	egislog_i("fingerprint_continue_enroll enter");
	egis_tp_enable();
	return g_lib_handle->rbs_continue();
}

