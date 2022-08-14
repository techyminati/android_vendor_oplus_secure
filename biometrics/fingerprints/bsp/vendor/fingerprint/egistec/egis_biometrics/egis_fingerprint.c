#include "plat_log.h"
#include <dlfcn.h>
#include <errno.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <hardware/hardware.h>
#include <hardware/hw_auth_token.h>
#include "android_fp_types.h"
#include "egis_fingerprint.h"
#include "response_def.h"
#ifdef QSEE
#include "qsee_keymaster.h"
#endif

#include <cutils/properties.h>
#include <cutils/log.h>
#include <pthread.h>
#include "egis_service.h"

#define FINGERPRINT_VERSION "1.0.6"

#define EMPTY_USER_ID 9999
#define MAX_FP_PER_USER 5
#define MSG_QUEUE_SIZE 64
#define ENROLL_SAMPLES 16
#define FINGERPRINT_VTS_ERROR 0  //for VTS test. Do NOT modify.
#define CMD_REMOVE_DEVICE_NODE 1
#define PID_COMMAND 0
unsigned int g_active_user_id = EMPTY_USER_ID;
egis_fingerprint_hal_device_t *g_dev = NULL;
unsigned int g_need_check_timeout = 0;

lib_handle_t *g_lib_handle = NULL;
static int fp_get_lib_sym(lib_handle_t *lib_handle);

timer_t g_enroll_timer = NULL;
static Egis_FingerprintScreenState g_screen_state = EGIS_FINGERPRINT_SCREEN_OFF;
static unsigned int g_verify_opt = FINGERPRINT_OPERATION_OTHERS;
#define ENROLL_TIMEOUT 0x100
#define ENROLL_TIMEOUT_DEFAULT_VALUE 600
#define ENROLLMENT_TOTAL_TIMES  16

static uint64_t g_pre_enroll_time = 0; // seconds

static int timer_setup(int type);
static int timer_destroy(int type);
static int timer_run(int type, unsigned int timeout);
static int timer_stop(int type);
static uint64_t get_seconds(void);

static int fp_get_lib_sym(lib_handle_t *lib_handle)
{
	lib_handle->ets_fp_libhandle = dlopen("libRbsFlow_cap.so", RTLD_NOW);
	if (lib_handle->ets_fp_libhandle == NULL) {
		ALOGE("failed to load libRbsFlow_cap.so");
		return -1;
	}
	// initialization
	*(void **)(&lib_handle->rbs_initialize) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_initialize");
	if (lib_handle->rbs_initialize == NULL) {
		ALOGE("dlsym: Error Loading rbs_initialize");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_uninitialize) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_uninitialize");
	if (lib_handle->rbs_uninitialize == NULL) {
		ALOGE("dlsym: Error Loading rbs_uninitialize");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_cancel) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_cancel");
	if (lib_handle->rbs_cancel == NULL) {
		ALOGE("dlsym: Error Loading rbs_cancel");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_pause) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_pause");
	if (lib_handle->rbs_pause == NULL) {
		ALOGE("dlsym: Error Loading rbs_pause");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_continue) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_continue");
	if (lib_handle->rbs_continue == NULL) {
		ALOGE("dlsym: Error Loading rbs_continue");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_clean_up) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_clean_up");
	if (lib_handle->rbs_clean_up == NULL) {
		ALOGE("dlsym: Error Loading rbs_clean_up");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_set_touch_event_listener) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_set_touch_event_listener");
	if (lib_handle->rbs_set_touch_event_listener == NULL) {
		ALOGE("dlsym: Error Loading rbs_set_touch_event_listener");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_active_user_group) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_active_user_group");
	if (lib_handle->rbs_active_user_group == NULL) {
		ALOGE("dlsym: Error Loading rbs_active_user_group");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_set_data_path) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_set_data_path");
	if (lib_handle->rbs_set_data_path == NULL) {
		ALOGE("dlsym: Error Loading rbs_set_data_path");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_chk_secure_id) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_chk_secure_id");
	if (lib_handle->rbs_chk_secure_id == NULL) {
		ALOGE("dlsym: Error Loading rbs_chk_secure_id");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_pre_enroll) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_pre_enroll");
	if (lib_handle->rbs_pre_enroll == NULL) {
		ALOGE("dlsym: Error Loading rbs_pre_enroll");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_enroll) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_enroll");
	if (lib_handle->rbs_enroll == NULL) {
		ALOGE("dlsym: Error Loading rbs_enroll");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_post_enroll) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_post_enroll");
	if (lib_handle->rbs_post_enroll == NULL) {
		ALOGE("dlsym: Error Loading rbs_post_enroll");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_chk_auth_token) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_chk_auth_token");
	if (lib_handle->rbs_chk_auth_token == NULL) {
		ALOGE("dlsym: Error Loading rbs_chk_auth_token");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_authenticator) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_authenticator");
	if (lib_handle->rbs_authenticator == NULL) {
		ALOGE("dlsym: Error Loading rbs_authenticator");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_remove_fingerprint) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_remove_fingerprint");
	if (lib_handle->rbs_remove_fingerprint == NULL) {
		ALOGE("dlsym: Error Loading rbs_remove_fingerprint");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_get_fingerprint_ids) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_get_fingerprint_ids");
	if (lib_handle->rbs_get_fingerprint_ids == NULL) {
		ALOGE("dlsym: Error Loading rbs_get_fingerprint_ids");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_get_authenticator_id) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_get_authenticator_id");
	if (lib_handle->rbs_get_authenticator_id == NULL) {
		ALOGE("dlsym: Error Loading rbs_get_authenticator_id");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_navigation) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_navigation");
	if (lib_handle->rbs_navigation == NULL) {
		ALOGE("dlsym: Error Loading rbs_navigation");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_set_on_callback_proc) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_set_on_callback_proc");
	if (lib_handle->rbs_set_on_callback_proc == NULL) {
		ALOGE("dlsym: Error Loading rbs_set_on_callback_proc");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_extra_api) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_extra_api");
	if (lib_handle->rbs_extra_api == NULL) {
		ALOGE("dlsym: Error Loading rbs_extra_api");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_get_screen_state) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_get_screen_state");
	if (lib_handle->rbs_get_screen_state == NULL) {
		ALOGE("dlsym: Error Loading rbs_get_screen_state");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	*(void **)(&lib_handle->rbs_keymode_enable) =
	    dlsym(lib_handle->ets_fp_libhandle, "rbs_keymode_enable");
	if (lib_handle->rbs_keymode_enable == NULL) {
		ALOGE("dlsym: Error Loading rbs_keymode_enable");
		dlclose(lib_handle->ets_fp_libhandle);
		lib_handle->ets_fp_libhandle = NULL;
		return -1;
	}
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int g_has_enroll_count = 0;
int g_enroll_percentage_tmp = 0;

static unsigned long get_64bit_rand()
{
	unsigned long random = 0;
	int fd = open("/dev/urandom", O_RDONLY);
	if (-1 == fd) {
		ALOGE("open /dev/urandom failed, return 0");
	}

	if (-1 == read(fd, (void *)&random, sizeof(unsigned long))) {
		ALOGE("read /dev/urandom failed, return 0");
	}

	if (-1 == close(fd)) {
		ALOGE("close /dev/urandom failed, return 0");
	}

	//return (((unsigned long)rand()) << 32) | ((unsigned long)rand());
	return random;
}

static unsigned int generate_fingerprint_id() { return (get_64bit_rand() & 0x7FFFFFFF); }

static void on_enroll_progress(unsigned int fid, int percentage)
{
	ALOGD("on_enroll_progress fid = %d, percentage = %d", fid, percentage);

	if (g_enroll_percentage_tmp != percentage) {
		g_has_enroll_count++;
		g_enroll_percentage_tmp = percentage;
	}

	int remaining;
	if (percentage >= 100) {
		g_enroll_percentage_tmp = 0;
		g_has_enroll_count = 0;
		remaining = 0;
	} else if (percentage > 0) {
		int total = 100 / (float)percentage * g_has_enroll_count;
		remaining = total - g_has_enroll_count;
		ALOGD("on_enroll_progress has_enroll_count=%d, total=%d, remaining=%d",
			  g_has_enroll_count, total, remaining);
		if (remaining < 1) remaining = 1;
	} else {
		remaining = ENROLL_SAMPLES;
	}

	g_dev->callback->on_enroll_result(g_dev->callback_context, fid, g_dev->user_id, remaining);
}

static void on_acquired_msg(int info)
{
	ALOGD("on_acquired %d", info);
	g_dev->callback->on_acquired(g_dev->callback_context, info);
}

static void on_error_msg(int err_code)
{
	ALOGE("on_error %d", err_code);

	g_dev->callback->on_error(g_dev->callback_context, err_code);
}

int fingerprint_close(egis_fingerprint_hal_device_t *dev)
{
	ALOGI("fingerprint_close enter");
	if (dev) {
		int ret ;
		free(dev); dev = NULL;
		//timer_destroy(ENROLL_TIMEOUT);
		ret = g_lib_handle->rbs_uninitialize();

		if (g_lib_handle) free(g_lib_handle);
		if (g_dev) free(g_dev);

		return ret;
	} else {
		return -1;
	}
}

uint64_t fingerprint_pre_enroll(egis_fingerprint_hal_device_t *device)
{
	device->challenge = get_64bit_rand();
    g_need_check_timeout = 1;
    g_pre_enroll_time = get_seconds();
	ALOGI("fingerprint_pre_enroll challenge = %" PRIu64 "", device->challenge);

	return device->challenge;
}

int fingerprint_enroll(egis_fingerprint_hal_device_t *device,
		       const uint8_t *hat_data, uint32_t __unused gid,
		       uint32_t __unused timeout_sec)
{
	int retval;
	unsigned int fid;
	g_has_enroll_count = 0;
	g_enroll_percentage_tmp = 0;
    uint64_t enroll_time = 0;
	ALOGI("fingerprint_enroll enter! fingerptint_version: %s",
		  FINGERPRINT_VERSION);
	egis_hw_auth_token_t *hat = (egis_hw_auth_token_t *)hat_data;

	egis_fingerprint_hal_device_t *dev =
	    (egis_fingerprint_hal_device_t *)device;

	if (ENROLL_TIMEOUT_DEFAULT_VALUE > 0) {	 // need check timeout
		g_need_check_timeout = 1;
		ALOGI("check enroll timeout (%d)", ENROLL_TIMEOUT_DEFAULT_VALUE);
		enroll_time = get_seconds();
		if (enroll_time < g_pre_enroll_time || (int32_t)(enroll_time - g_pre_enroll_time) >= ENROLL_TIMEOUT_DEFAULT_VALUE) {
			on_error_msg(ERROR_TIMEOUT);
			ALOGI("enroll timeout, change to idle");
			return 0;
		}
	}

	if ((hat == NULL) || (hat->timestamp == 0)) {
		ALOGE("hat is null");
		on_error_msg(ERROR_UNABLE_TO_PROCESS);
		return FINGERPRINT_VTS_ERROR;
	}

	if (hat && (hat->challenge == dev->challenge)) {
		ALOGD("challenge matched");
	} else {
		ALOGE("challenge not match");
		on_error_msg(ERROR_UNABLE_TO_PROCESS);
		return FINGERPRINT_VTS_ERROR;
	}

	if (hat->version != HW_AUTH_TOKEN_VERSION) {
		ALOGE("invalid hat->version = %d", hat->version);
		return -EPROTONOSUPPORT; /* Protocol not supported */
	}

	if (hat->challenge != dev->challenge &&
	    !(hat->authenticator_type & HW_AUTH_FINGERPRINT)) {
		ALOGE("Not super-user");
		return -EPERM; /* Not super-user */
	}

	retval = g_lib_handle->rbs_chk_auth_token((unsigned char *)hat,
						  sizeof(egis_hw_auth_token_t));
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ALOGE("authtoken check failed");
		return -EPERM;
	}

	retval = g_lib_handle->rbs_chk_secure_id(gid, hat->user_id);
	if (retval != FINGERPRINT_RES_SUCCESS) {
		ALOGE("secureid check failed %d", retval);
		if (retval == FINGERPRINT_RES_SECURE_ID_NOTMATCH) {
			retval = g_lib_handle->rbs_remove_fingerprint(gid, 0);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				ALOGE("remove all fingerprints failed %d", retval);
				return -EPERM;
			}

			retval = g_lib_handle->rbs_chk_secure_id(gid, hat->user_id);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				ALOGE("secureid check failed %d", retval);
				return -EPERM;
			}

			ALOGD("Remove all fingerprints and secureid check OK");
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
		ALOGE("g_lib_handle->rbs_pre_enroll failed, no space");
		on_error_msg(EGIS_FINGERPRINT_ERROR_CANCELED);
		return FINGERPRINT_RES_SUCCESS;
	}

	retval = g_lib_handle->rbs_enroll();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ALOGE("g_lib_handle->rbs_enroll return = %d", retval);
		on_error_msg(EGIS_FINGERPRINT_ERROR_CANCELED);
	}

	if (FINGERPRINT_RES_HW_UNAVALABLE == retval) {
		on_error_msg(EGIS_FINGERPRINT_ERROR_HW_UNAVAILABLE);
	}

	//timer_run(ENROLL_TIMEOUT, ENROLL_TIMEOUT_DEFAULT_VALUE);
	g_verify_opt = FINGERPRINT_OPERATION_OTHERS;

	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_post_enroll(egis_fingerprint_hal_device_t *device)
{
	ALOGI("fingerprint_post_enroll enter");
	egis_fingerprint_hal_device_t *dev =
	    (egis_fingerprint_hal_device_t *)device;
	//timer_stop(ENROLL_TIMEOUT);
	dev->challenge = 0;
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_enumerate()
{
	int retval;
	int tmp_ids[MAX_FP_PER_USER] = {0};
	int tmp_ids_count = 0;
	int loop_index;

	ALOGI("fingerprint_enumerate enter");

	if (EMPTY_USER_ID == g_dev->user_id) {
		ALOGE("fingerprint_enumerate user id empty");
		return -1;  // caller should call set_active_group first
	}

	retval = g_lib_handle->rbs_get_fingerprint_ids(g_dev->user_id, tmp_ids, &tmp_ids_count);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ALOGE("g_lib_handle->rbs_get_fingerprint_ids return = %d", retval);
		return retval;
	}

	g_dev->callback->on_sync_templates(g_dev->callback_context, tmp_ids, tmp_ids_count, g_dev->user_id);

	return FINGERPRINT_RES_SUCCESS;
}

uint64_t fingerprint_get_auth_id(egis_fingerprint_hal_device_t *device)
{
	int retval;
	unsigned long long authenticator_id = 0;

	ALOGI("fingerprint_get_auth_id enter");
	if (device == NULL) {
		ALOGE("NULL device on open");
		return -EINVAL; /* Invalid argument */
	}

	retval = g_lib_handle->rbs_get_authenticator_id(&authenticator_id);

	ALOGD("rbs_get_authenticator_id retval = %d, authenticator_id =%llu", retval, authenticator_id);

	return authenticator_id;
}

int fingerprint_cancel()
{
	int retval;

	ALOGI("fingerprint_cancel enter");
	g_enroll_percentage_tmp = 0;
	g_has_enroll_count = 0;

	retval = g_lib_handle->rbs_cancel();
	on_error_msg(EGIS_FINGERPRINT_ERROR_CANCELED);

	ALOGD("g_lib_handle->rbs_cancel return = %d", retval);
	return retval;
}

int fingerprint_remove(uint32_t gid,
		       uint32_t fid)
{
	int retval;
	int fingercount = 0;
	int tmp_ids[MAX_FP_PER_USER] = {0};
	int tmp_ids_count = 0;

	ALOGI("fingerprint_remove enter, gid = %d, fid = %d", gid, fid);

	if (0 == fid) {
		retval = g_lib_handle->rbs_get_fingerprint_ids(gid, tmp_ids, &tmp_ids_count);
		ALOGD("remove all. retval=%d, finger_count=%d", retval, tmp_ids_count);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			on_error_msg(EGIS_FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
		}
	}

	if (tmp_ids_count == 0 && 0 == fid) {
		ALOGE("finger list is null");
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

			on_error_msg(EGIS_FINGERPRINT_ERROR_CANCELED);
		} else {
			g_dev->callback->on_removed(g_dev->callback_context, fid, gid, tmp_ids_count);
			ALOGI("[fingerprint_remove] sync fingerlist");
			if(retval == FINGERPRINT_RES_SUCCESS)
			{
				retval = fingerprint_enumerate();
			}
			ALOGI("[fingerprint_remove] sync fingerlist return =%d", retval);				
		}
		// g_dev->callback->on_removed(g_dev->callback_context, 0, gid,
		// tmp_ids_count);
	} else {
		ALOGE("fingerprint_remove return = %d", retval);
		on_error_msg(EGIS_FINGERPRINT_ERROR_UNABLE_TO_REMOVE);
	}
	ALOGD("fingerprint remining = %d", tmp_ids_count);

	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_set_active_group(uint32_t gid, const char *store_path)
{
	int retval;

	ALOGI("fingerprint_set_active_group enter");

	ALOGI("fingerprint_set_active_group g_dev->user_id= %d , gid = %d" , g_dev->user_id, gid);

	g_dev->user_id = gid;

	retval = g_lib_handle->rbs_active_user_group(gid, (const char *)store_path);

	ALOGI("[fingerprint_set_active_group] sync fingerlist");
	if(retval == FINGERPRINT_RES_SUCCESS)
	{
		retval = fingerprint_enumerate();
	}
	ALOGI("[fingerprint_set_active_group] sync fingerlist return =%d", retval);

	ALOGD("fingerprint_set_active_group return = %d", retval);
	return retval;
}

int fingerprint_authenticate(egis_fingerprint_hal_device_t *device,
			     uint64_t operation_id, uint32_t gid)
{
	ALOGI("fingerprint_authenticate enter, operation_id = %" PRIu64 ", gid = %u", operation_id, gid);

	int retval;
    //fingerprint authenticate do not check time out
    g_need_check_timeout = 0;
    
	egis_fingerprint_hal_device_t *dev =
	    (egis_fingerprint_hal_device_t *)device;
	dev->op_id = operation_id;

	retval = g_lib_handle->rbs_authenticator(gid, NULL, 0, operation_id);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		on_error_msg(EGIS_FINGERPRINT_ERROR_CANCELED);
	}

	if (FINGERPRINT_RES_HW_UNAVALABLE == retval) {
		on_error_msg(EGIS_FINGERPRINT_ERROR_HW_UNAVAILABLE);
	}

	g_verify_opt = FINGERPRINT_OPERATION_VERIFY;

	ALOGD("fingerprint_authenticate return = %d", retval);
	return retval;
}

void event_handle(int event_id, __unused int value1, __unused int value2,
		  __unused unsigned char *buffer, __unused int buffer_size)
{
    uint64_t enroll_time = 0;
	ALOGI("event_handle event_id = %d ", event_id);
    ALOGI("%s: g_need_check_timeout:%d",__func__,g_need_check_timeout);
    if (ENROLL_TIMEOUT_DEFAULT_VALUE > 0 && g_need_check_timeout) { // need check timeout
        ALOGI("check enroll timeout (%d)", ENROLL_TIMEOUT_DEFAULT_VALUE);
        enroll_time = get_seconds();
        if (enroll_time < g_pre_enroll_time || (int32_t)(enroll_time - g_pre_enroll_time) >= ENROLL_TIMEOUT_DEFAULT_VALUE) {
            on_error_msg(ERROR_TIMEOUT);
            ALOGI("enroll timeout, change to idle");
        }
    }

	switch (event_id) {
		case EVENT_ENROLL_OK: {
			//if (value2 == 100)
				//timer_stop(ENROLL_TIMEOUT);

			on_enroll_progress(value1, value2);

			if(value2 == 100)
			{
				ALOGI("[EVENT_ENROLL_OK] sync fingerlist");
				fingerprint_enumerate();
			}

		} break;
		case EVENT_ENROLL_HIGHLY_SIMILAR: {
			on_acquired_msg(ACQUIRED_TOO_SIMILAR);
		} break;

		case EVENT_ENROLL_DUPLICATE_FINGER: {
			on_acquired_msg(ACQUIRED_ALREADY_ENROLLED);
		} break;

		case EVENT_ENROLL_IMG_TOO_SMALL:
		case EVENT_ENROLL_FEATURE_LOW: {
			g_dev->callback->on_image_info_acquired(g_dev->callback_context, EGIS_FINGERPRINT_ACQUIRED_TOO_SLOW,0,0);
		} break;
		case EVENT_ENROLL_CANCELED: {
			//timer_stop(ENROLL_TIMEOUT);
			g_verify_opt = FINGERPRINT_OPERATION_OTHERS;
		} break;
		case EVENT_VERIFY_MATCHED:
		case EVENT_VERIFY_NOT_MATCHED: {
			//g_dev->user_id = value1;
			g_dev->matched_fid = value2;
			if (sizeof(egis_hw_auth_token_t) == buffer_size) {
				memcpy(&g_dev->hat, buffer, buffer_size);
			} else {
				ALOGE(
				    "received hat sizeof(hw_auth_token_t) = %u "
				    ", buffer_size = %d",
				    sizeof(egis_hw_auth_token_t), buffer_size);
			}

			g_dev->callback->on_authenticated(
			    g_dev->callback_context, g_dev->matched_fid,
			    /*g_dev->user_id*/value1, &g_dev->hat,
			    sizeof(egis_hw_auth_token_t));
		} break;

		case EVENT_FINGER_TOUCH: {
			g_dev->callback->on_touch_down(g_dev->callback_context);
		} break;

		case EVENT_FINGER_READY: {
			g_dev->callback->on_image_info_acquired(g_dev->callback_context, EGIS_FINGERPRINT_ACQUIRED_GOOD,0,0);
		} break;

		case EVENT_FINGER_LEAVE: {
			g_dev->callback->on_touch_up(g_dev->callback_context);
		} break;

		case EVENT_IMG_BAD_QLTY: {
			ALOGI("EVENT_IMG_BAD_QLTY g_verify_opt:%d, g_screen_state:%d", g_verify_opt, g_screen_state);
			if (g_verify_opt == FINGERPRINT_OPERATION_VERIFY) {
				if (g_screen_state == EGIS_FINGERPRINT_SCREEN_OFF){
					int dev_screan_state = EGIS_FINGERPRINT_SCREEN_OFF;

					g_lib_handle->rbs_get_screen_state(&dev_screan_state);
					ALOGI("EVENT_IMG_BAD_QLTY dev_screan_state:%d", dev_screan_state);
					if (dev_screan_state == EGIS_FINGERPRINT_SCREEN_OFF) {
						g_dev->callback->on_image_info_acquired(g_dev->callback_context, EGIS_FINGERPRINT_ACQUIRED_INSUFFICIENT, 0, 0);
						break;
					}
				}

				g_dev->callback->on_image_info_acquired(g_dev->callback_context, EGIS_FINGERPRINT_ACQUIRED_IMAGER_DIRTY, 0, 0);
				g_dev->callback->on_authenticated(g_dev->callback_context, 0, 0, &g_dev->hat, sizeof(egis_hw_auth_token_t));
			} else {
				g_dev->callback->on_image_info_acquired(g_dev->callback_context, EGIS_FINGERPRINT_ACQUIRED_IMAGER_DIRTY, 0, 0);
			}

			on_acquired_msg(ACQUIRED_IMAGER_DIRTY);
		} break;

		case EVENT_IMG_PARTIAL:
		case EVENT_IMG_TOO_PARTIAL: {
			g_dev->callback->on_image_info_acquired(g_dev->callback_context, EGIS_FINGERPRINT_ACQUIRED_PARTIAL,0,0);
			on_acquired_msg(ACQUIRED_PARTIAL);
		} break;

		case EVENT_IMG_FAST:
		case EVENT_IMG_COVER:
		case EVENT_IMG_WATER: {
			g_dev->callback->on_image_info_acquired(g_dev->callback_context, EGIS_FINGERPRINT_ACQUIRED_TOO_FAST,0,0);
			on_acquired_msg(ACQUIRED_TOO_FAST);
		} break;
		case EVENT_TOO_MAUCH_IMG_REJECTED:
			ALOGE("too much reject!");
			on_error_msg(ERROR_UNABLE_TO_PROCESS);
		break;
		case EVENT_SENSOR_IMAGE_QTY:
			ALOGD("EVENT_SENSOR_IMAGE_QTY value2:%d", value2);
			g_dev->callback->on_engineering_info_update(g_dev->callback_context, EGIS_FINGERPRINT_IMAGE_QUALITY, value1, value2);
			break;
		default:
			break;
	};
}
/*
static void *thread_server_init(void *__unused data)
{
	egislog_e("thread_server_init egis_server_init start");

	egis_server_init();

	return NULL;
}

int start_inline_service()
{	
	pthread_t thread_id;
	ALOGD("start_inline_service begin");
	pthread_create(&thread_id, NULL, thread_server_init, NULL);
	pthread_detach(thread_id);
	ALOGD("start_inline_service end");
	return 0;
}
*/
int fingerprint_open(egis_fingerprint_hal_device_t **device)
{
	int retval = -1;
	int delay_offset = 0;
	unsigned long rand = 0;
	char user_path[256] = "/data/vendor_de/0/fpdata";
	ALOGI("fingerprint_open enter");

//	pthread_t thread_id;

	if (!device) {
		ALOGE("NULL device on open");
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
	retval = g_lib_handle->rbs_set_data_path(1, user_path, 256);

	/* set callback */
	g_lib_handle->rbs_set_on_callback_proc(event_handle);

	/* initialize */
	while (delay_offset <= 5) {
		sleep(delay_offset);

#if defined(AUTHTOKEN_HMAC) && defined(QSEE)
		ALOGD("get_secure_key from Keymaster");
		uint8_t master_key[MAX_MASTERKEY_LEN] = {0};
		uint32_t master_key_len = sizeof(master_key) / sizeof(master_key[0]);

		retval = get_secure_key(master_key, &master_key_len);
		if (retval == FINGERPRINT_RES_SUCCESS)
			retval = g_lib_handle->rbs_initialize(master_key, master_key_len);
		else
			ALOGE("thread_initialize get master key fail");
#else
		retval = g_lib_handle->rbs_initialize(NULL, 0);
#endif
		ALOGE("rbs_initialize retval = %d" , retval);
		if (FINGERPRINT_RES_SUCCESS == retval) {
			ALOGE("g_lib_handle->rbs_initialize success");
			break;
		}

		ALOGE("g_lib_handle->rbs_initialize failed, return = %d, offset = %d", retval, delay_offset);
		delay_offset += 3;
	}

	if (FINGERPRINT_RES_SUCCESS == retval) {
		egis_fingerprint_hal_device_t *dev = (egis_fingerprint_hal_device_t *)malloc(sizeof(egis_fingerprint_hal_device_t));
		if (dev == NULL) {
			ALOGE("Invalid argument, fingerprint_open malloc Fail, dev == NULL");
			retval = -EINVAL; /* Invalid argument */
			goto exit;
		}
		memset(dev, 0, sizeof(egis_fingerprint_hal_device_t));

		*device = dev;
		g_dev = dev;

		ALOGE("thread_initialize success for fp mmi test,add hidl service");
//		pthread_create(&thread_id, NULL, thread_server_init, NULL);
//		pthread_detach(thread_id);
//		start_inline_service();

		// timer_setup(ENROLL_TIMEOUT);
	} else {
		ALOGE("g_lib_handle->rbs_initialize = %d", retval);
		int buffer[1] = {0};
		buffer[0] = CMD_REMOVE_DEVICE_NODE;
		int buffer_size = sizeof(buffer);
		g_lib_handle->rbs_extra_api(PID_COMMAND, (unsigned char *)buffer, buffer_size, NULL, 0);
	}

	ALOGE("%s property_set persist.sys.fp.vendor = et520", __func__);
	property_set("persist.sys.fp.vendor","et520");

exit:
	ALOGI("fingerprint_open end");
	return retval;
}

void fingerprint_set_callback(egis_fingerprint_hal_device_t **device,
			      const fingerprint_callback_t *callback,
			      void *callback_context)
{
	if (*device == NULL) {
		ALOGE("NULL device on open");
		return;
	}

	(*device)->callback = (fingerprint_callback_t *)callback;
	(*device)->callback_context = callback_context;
	return;
}

#define EXTRA_OUT_SIZE 32
int do_extra_api_in(int cmd_id, const unsigned char *in_buf, size_t in_buf_size, unsigned char* out_buf, int* out_buf_size)
{
	int ret;
	//int out_buf_size = EXTRA_OUT_SIZE;
	//char out_buf[EXTRA_OUT_SIZE] = {0};

	ALOGI("do_extra_api_in [%d]", cmd_id);
	ret = g_lib_handle->rbs_extra_api(cmd_id, in_buf, in_buf_size, (unsigned char *)out_buf, out_buf_size);

	return ret;
}

int extra_set_on_callback_proc(operation_callback_t callback)
{
	ALOGI("extra_set_on_callback_proc enter");
	g_lib_handle->rbs_set_on_callback_proc(callback);
	return 0;
}

int fingerprint_get_enrollment_total_times( )
{
	ALOGI("fingerprint_get_enrollment_total_times enter");

	return ENROLLMENT_TOTAL_TIMES;
}

int fingerprint_clean_up()
{
	ALOGI("fingerprint_clean_up enter");
	g_lib_handle->rbs_clean_up();
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_pause_enroll()
{
	ALOGI("fingerprint_pause_enroll enter");
	return g_lib_handle->rbs_pause();
}

int fingerprint_continue_enroll()
{
	ALOGI("fingerprint_continue_enroll enter");
	return g_lib_handle->rbs_continue();
}

int fingerprint_set_touch_event_listener()
{
	ALOGI("fingerprint_set_touch_event_listener enter");
	return g_lib_handle->rbs_set_touch_event_listener();
}

int fingerprint_dynamically_config_log(uint32_t on)
{
	ALOGI("fingerprint_dynamically_config_log enter");
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_pause_identify()
{
	ALOGI("fingerprint_pause_identify enter");
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_continue_identify()
{
	ALOGI("fingerprint_continue_identify enter");
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_get_alikey_status()
{
	ALOGI("fingerprint_get_alikey_status enter");
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_set_screen_state(int32_t screen_state)
{
	g_screen_state = screen_state;
	ALOGI("fingerprint_set_screen_state enter :%d",screen_state);
	return FINGERPRINT_RES_SUCCESS;
}

int fingerprint_get_engineering_info(uint32_t type)
{
	ALOGI("fingerprint_get_engineering_info enter, type = %d", type);
	unsigned char in_data[sizeof(int) * 2];
	unsigned char out_data[64];
	int out_data_len = 64;
	int snr_value = 0;
	int* ptr = (int*)in_data;
	int retval = FINGERPRINT_RES_SUCCESS;

	switch(type)
    {
		case EGIS_FINGERPRINT_GET_IMAGE_QUALITY:
			ptr[0] = 1104;
			ptr[1] = 2001;
			retval = do_extra_api_in(3, in_data, sizeof(int), out_data, &out_data_len);
			ptr = (int*)out_data;
			//g_dev->callback->on_engineering_info_update(g_dev->callback_context, EGIS_FINGERPRINT_IMAGE_QUALITY, retval, ptr[0]);

		break;

		case EGIS_FINGERPRINT_GET_BAD_PIXELS:
		case EGIS_FINGERPRINT_SELF_TEST:
			ptr[0] = 1100;
			retval = do_extra_api_in(3, in_data, sizeof(int), out_data, &out_data_len);
			if (retval == FINGERPRINT_RES_SUCCESS) {
				ptr[0] = 1102;
				retval = do_extra_api_in(3, in_data, sizeof(int), out_data, &out_data_len);
			}

			//g_dev->callback->on_engineering_info_update(g_dev->callback_context, type, retval, ptr[0]);
		break;

		case EGIS_FINGERPRINT_GET_IMAGE_SNR:
			ptr[0] = 1107;
			retval = do_extra_api_in(3, in_data, sizeof(int), out_data, &out_data_len);
			float *ft = (float *)out_data;
			snr_value = (int)*ft;
			//g_dev->callback->on_engineering_info_update(g_dev->callback_context, EGIS_FINGERPRINT_IMAGE_SNR, retval, snr_value);
		break;
		
		default:
		break;
    }
	return retval;
}

int fingerprint_keymode_enable(egis_fingerprint_hal_device_t* device, uint32_t enable)
{
	ALOGI("fingerprint_keymode_enable enter! enable:%d", enable);
	if (device == NULL) {
		ALOGE("NULL device on open");
		return -EINVAL; /* Invalid argument */
	}

	g_lib_handle->rbs_keymode_enable(enable);
	return 0;
}

static void enroll_timeout_handler()
{
	ALOGI("The operation has timed out waiting for user input");
	//g_lib_handle->rbs_cancel();
	//on_error_msg(ERROR_TIMEOUT);
	//timer_stop(ENROLL_TIMEOUT);
	//timer_destroy(ENROLL_TIMEOUT);
}

static int timer_setup(int type)
{
	int retval;
	timer_t timer;  // void*
	struct sigaction act;
	struct sigevent evp;

	if (type != ENROLL_TIMEOUT)
		return -1;

	memset(&act, 0, sizeof(act));
	act.sa_handler = enroll_timeout_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGUSR2, &act, NULL) == -1) {
		return -2;
	}

	memset(&evp, 0, sizeof(evp));
	evp.sigev_value.sival_ptr = &timer;
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR2;
	retval = timer_create(CLOCK_REALTIME, &evp, &timer);
	if (retval != 0) {
		return -3;
	}

	g_enroll_timer = timer;
	return 0;
}

static int timer_destroy(int type)
{
	int retval = 0;
	if (type != ENROLL_TIMEOUT) {
		return -1;
	}

	if (g_enroll_timer != NULL)
		retval = timer_delete(g_enroll_timer);
	g_enroll_timer = NULL;
	return retval;
}

static int timer_run(int type, unsigned int seconds)
{
	int retval;
	struct itimerspec ts;

	if (type != ENROLL_TIMEOUT) {
		return -1;
	}

	if (seconds > 1000 * 1000) {
		return -1;
	}

	if (g_enroll_timer == NULL) {
		timer_setup(type);
	}
	ts.it_value.tv_sec = seconds;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	retval = timer_settime(g_enroll_timer, 0, &ts, NULL);
	if (retval != 0) {
		return -4;
	}

	return 0;
}

static int timer_stop(int type)
{
	int retval;
	struct itimerspec ts;

	if (type != ENROLL_TIMEOUT) {
		return -1;
	}

	if (g_enroll_timer == NULL)
		return 0;

	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	retval = timer_settime(g_enroll_timer, 0, &ts, NULL);
	if (retval != 0) {
		return -4;
	}

	return 0;
}

static uint64_t get_seconds(void)
{
    uint64_t msec = 0;
    time_t timep;

    time(&timep);
    msec = (uint64_t)timep;

    return msec;
}
