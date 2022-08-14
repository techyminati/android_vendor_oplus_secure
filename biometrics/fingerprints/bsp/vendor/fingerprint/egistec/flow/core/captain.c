#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "captain.h"
#include "device_int.h"
#include "navi_operator.h"
#include "constant_def.h"
#include "response_def.h"
#include "ex_define.h"
#include "type_def.h"
#include "op_manager.h"
#include "plat_log.h"
#include "plat_time.h"
#include "fps_normal.h"
#include "opt_file.h"
#include "egis_rbs_api.h"
#include "thread_manager.h"
#include "fp_definition.h"
#include "common_definition.h"
#include "op_sensortest.h"
#include "core_config.h"
#include "op_manager.h"
#include "plat_mem.h"

#if defined (__SDK_SAVE_IMAGE__) || defined (__SDK_SAVE_FINGER_OFF_IMAGE__)
#include "save_image.h"
#endif

#define CANCEL_TIMEOUT 1  // seconds
#define AUTH_TOKEN_LEN 256

#define CMD_UPDATE_CONFIG 2002
#define CMD_SET_NAVI_CONFIG 2003

#define PID_DEMOTOOL 5
#define CMD_VERSION_ALGO 1000
#define CMD_VERSION_TA 1001
#define CMD_VERSION_CA 1002
#define CMD_SET_CALLBACK_FLAG 1003
#define CMD_RESET_CALLBACK_FLAG 1004

#define CMD_REMOVE_DEVICE_NODE 1

#ifndef MAX_PATH
#define MAX_PATH 256
#endif
#ifndef __unused
#define __unused
#endif

cache_info_t g_cache_info;
fingerprint_enroll_info_t g_enroll_info;
fingerprint_verify_info_t g_verify_info;
static enroll_config_t g_enroll_config;
static navigation_config_t g_navi_config;
static cut_img_config_t g_crop_config;

#ifdef __SDK_SAVE_IMAGE__
static char g_match_info[MAX_PATH];
#endif

#ifdef __SDK_SAVE_FINGER_OFF_IMAGE__
static char g_finger_lost_info[MAX_PATH];
#endif

static unsigned int g_has_enroll_count = 0;
static int g_enroll_percentage_tmp = 0;

static BOOL g_need_cancel = FALSE;
static BOOL g_need_pause = FALSE;
static BOOL g_hardware_ready = FALSE;
#ifdef __USE_EGIS_DEMOTOOL__
static BOOL g_use_extra = FALSE;
#endif
extern BOOL g_navi_need_cancel;

typedef enum {
	LOCKER_STATUS_UNLOCK,
	LOCKER_STATUS_PREPARE_LOCK,
	LOCKER_STATUS_LOCK,
	LOCKER_STATUS_PREPARE_UNLOCK,
} locker_status_t;

int g_hdev = 0;
char g_user_path[MAX_PATH_LEN] = {0};
int g_img_qty_score = 0;

event_callbck_t g_event_callback;

#ifdef __USE_EGIS_DEMOTOOL__
event_callbck_t g_extra_callback;
#endif

#define __USE_AUTH_K4__

#ifdef __USE_AUTH_K4__
typedef enum verify_state {
	VSTATE_DETECT_MODE,
	VSTATE_CHECK_FINGER_ON,
	VSTATE_WAIT_INT,
	VSTATE_GET_IMG,
	VSTATE_VERIFY,
	VSTATE_FINGER_OFF_INTERRUPT,
	VSTATE_FINGER_OFF_IMAGE,
	VSTATE_ENTER_LOCKER_MODE,
	VSTATE_LEAVE_LOCKER_MODE
} verify_state_t;
#else
typedef enum verify_state {
	VSTATE_DETECT_MODE,
	VSTATE_WAIT_INT,
	VSTATE_GET_IMG,
	VSTATE_VERIFY,
	VSTATE_FINGER_OFF,
	VSTATE_ENTER_LOCKER_MODE,
	VSTATE_LEAVE_LOCKER_MODE
} verify_state_t;
#endif

typedef enum enroll_state {
	ESTATE_DETECT_MODE,
	ESTATE_WAIT_INT,
	ESTATE_GET_IMG,
	ESTATE_CHECK_DUPLICATE,
	ESTATE_ENROLL,
	ESTATE_FINGER_OFF,
	ESTATE_CHECK_FINGER_OFF_ONCE,
	ESTATE_PAUSE,
} enroll_state_t;

typedef enum navi_state {
	NSTATE_WAIT_INT,
	NSTATE_NAVI,
	NSTATE_FINGER_OFF,
} navi_state_t;

typedef enum camera_state {
	CSTATE_WAIT_INT,
	CSTATE_GET_IMG,
	CSTATE_FINGER_OFF,
} camera_state_t;

typedef struct {
	int  info_version;
	int  is_new_finger_on;
	int  try_count;
	int  match_score;
	int  save_index;
} return_image_info_t;

#ifdef __USE_AUTH_K4__
typedef enum fingerprint_option {
	FINGERPRINT_OPTION_ENROLL,
	FINGERPRINT_OPTION_VERIFY,
	FINGERPRINT_OPTION_OTHERS,
} fingerprint_option_t;
static int g_fingerprint_opt = FINGERPRINT_OPTION_OTHERS;
#endif

#ifdef __TRUSTONIC__
fix_pattern_data g_fix_data;
#endif

static int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good, int match_result);
static int do_sensortest(int cid, unsigned char *in_data, int in_data_size,
			 unsigned char *out_buffer, int *out_buffer_size);
static void power_consumption(void);
void captain_cancel(BOOL cancel_flag);
static void get_ca_version(unsigned char *version, int *len);
static int enroll_percentage_to_remaining(int percentage);

static void notify(int event_id, int first_param, int second_param,
		   unsigned char *data, int data_size)
{
#ifdef __USE_EGIS_DEMOTOOL__	
	if(g_extra_callback != NULL && g_use_extra) {
		ex_log(LOG_DEBUG, "g_extra_callback enter, event_id: %d", event_id);
		g_extra_callback(event_id, first_param, second_param, data, data_size);
		return;
	} 
#endif	
	
	if (NULL != g_event_callback) {
		ex_log(LOG_DEBUG, "g_event_callback enter, event_id: %d", event_id);
#ifndef EGIS_DBG
		if (event_id == EVENT_RETURN_IMAGE || event_id ==  EVENT_RETURN_LIVE_IMAGE)
			return;
#endif
		g_event_callback(event_id, first_param, second_param, data, data_size);

#ifdef __OPLUS_K4__
		ex_log(LOG_DEBUG, "EGIS_NOTIFY, g_fingerprint_opt: %d", g_fingerprint_opt);
		if (g_fingerprint_opt == FINGERPRINT_OPTION_ENROLL 
			&& (event_id == EVENT_IMG_PARTIAL || event_id == EVENT_IMG_FAST || event_id == EVENT_IMG_BAD_QLTY)) {
			g_event_callback(EVENT_ENROLL_OK, first_param, second_param, data, data_size);
		}
#else		
		if (event_id == EVENT_ENROLL_OK) {
			int remaining = enroll_percentage_to_remaining(second_param);
			g_event_callback(EVENT_ENROLL_REMAINING, first_param, remaining, data, data_size);
		}
#endif
		if (event_id == EVENT_NAVIGATION) send_navi_event_to_driver(first_param);
	}
}

static void get_module_info(unsigned char* module_type) {
    unsigned char buf[1] = {0};
    int ret = FP_LIB_ERROR_GENERAL;
    int buf_len = sizeof(buf);

    ret = sensortest_send_cmd(FP_GET_MODULE_INFO, buf, &buf_len);
    if (ret == FP_LIB_OK) {
	    ex_log(LOG_DEBUG, "buf[0] = 0x%02x", buf[0] & 0xF);
	    *module_type = buf[0] & 0xF;
    }
    ex_log(LOG_DEBUG, "module_type = 0x%02x", *module_type);
}

void get_algorithm_version(unsigned char *algo_ver)
{
	ex_log(LOG_DEBUG, "%s enter", __func__);
	unsigned char buf[32] = {0};
	int ret = FP_LIB_ERROR_GENERAL;
	int buf_len = sizeof(buf);
	ret = sensortest_send_cmd(FP_GET_ALGO_VER, buf, &buf_len);
	if (ret == FP_LIB_OK) {
		ex_log(LOG_DEBUG, "algo version= %s", buf);
		memcpy(algo_ver, buf, sizeof(buf));
	}
}

int get_sensor_id()
{
	ex_log(LOG_DEBUG, "%s enter", __func__);
	unsigned char buf[4] = {0};
	int ret = FP_LIB_ERROR_GENERAL;
	int buf_len = sizeof(buf);
	ret = sensortest_send_cmd(FP_GET_SENSOR_ID, buf, &buf_len);
	if (ret == FP_LIB_OK) {
		ex_log(LOG_DEBUG, "buf[0] = 0x%02x-%d, buf[10] = 0x%02x-%d", buf[0], buf[0], buf[1], buf[1]);
		return (int)buf[0] * 100 + buf[1];
	}
	return FP_LIB_ERROR_GENERAL;
}

static void get_dcs_msg(dcs_msg_t *msg, int size)
{
	ex_log(LOG_DEBUG, "%s enter", __func__);
	unsigned char module_type = 0;
	char algo_ver[32] = {0};

	if (msg != NULL) {
		memset(msg, 0x00, size);

		msg->auth_result = g_verify_info.is_passed;

		if (g_verify_info.is_passed) {
			msg->fail_reason = FP_LIB_VERIFY_MATCHED;
		} else {
			msg->fail_reason = g_verify_info.dcsmsg.fail_reason;
		}

		msg->quality_score = g_verify_info.dcsmsg.quality_score;
		msg->match_score = g_verify_info.dcsmsg.match_score;
		msg->signal_value = 0;
		msg->img_area = 0;
		msg->retry_times = g_verify_info.dcsmsg.retry_times;

		get_algorithm_version((unsigned char *)algo_ver);
		memcpy(msg->algo_version, algo_ver, sizeof(msg->algo_version));

		msg->chip_ic = get_sensor_id();
		msg->factory_id = 0;

		get_module_info(&module_type);
		msg->module_type = module_type;

		msg->lense_type = 0;
		msg->dsp_availalbe = 0;

		ex_log(LOG_DEBUG, "[%s] Auth, fail_reason = %d", __func__, msg->auth_result);
		ex_log(LOG_DEBUG, "[%s] Auth, fail_reason = %d", __func__, msg->fail_reason);
		ex_log(LOG_DEBUG, "[%s] Auth, quality_score = %d", __func__, msg->quality_score);
		ex_log(LOG_DEBUG, "[%s] Auth, match_score = %d", __func__,  msg->match_score);
		ex_log(LOG_DEBUG, "[%s] Auth, signal_value = %d", __func__,  msg->signal_value);
		ex_log(LOG_DEBUG, "[%s] Auth, img_area = %d", __func__, msg->img_area);
		ex_log(LOG_DEBUG, "[%s] Auth, retry_times = %d", __func__, msg->retry_times);
		ex_log(LOG_DEBUG, "[%s] Auth, algo_version = %s", __func__, msg->algo_version);
		ex_log(LOG_DEBUG, "[%s] Auth, chip_ic = %d", __func__,  msg->chip_ic);
		ex_log(LOG_DEBUG, "[%s] Auth, module_type = %d", __func__, msg->module_type);
		ex_log(LOG_DEBUG, "[%s] Auth, lense_type = %d", __func__, msg->lense_type);
		ex_log(LOG_DEBUG, "[%s] Auth, dsp_availalbe = %d", __func__, msg->dsp_availalbe);
	} else {
		ex_log(LOG_ERROR, "msg is NULL pointer");
	}
}

static int create_ini_config(BOOL need_reinit, unsigned char* in_data, int in_data_size)
{
	int retval = FINGERPRINT_RES_SUCCESS;
	
	retval = opt_send_data(TYPE_SEND_INI_CONFIG, in_data, in_data_size);

	if (retval != FINGERPRINT_RES_SUCCESS) {
		ex_log(LOG_ERROR, "create_ini_config return %d", retval);
		return FINGERPRINT_RES_FAILED;
	}

	g_enroll_config.enroll_method = core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLL_METHOD, ENROLL_METHOD_TOUCH);
	g_enroll_config.swipe_dir = core_config_get_int(INI_SECTION_ENROLL, KEY_SWIPE_DIRECTION_MODE, SWIPE_DIRECTION_AUTO);
	g_enroll_config.swipe_count_y = core_config_get_int(INI_SECTION_ENROLL, KEY_SWIPE_COUNT_Y, DEFAULT_SWIPE_COUNT_Y);
	g_enroll_config.swipe_count_x = core_config_get_int(INI_SECTION_ENROLL, KEY_SWIPE_COUNT_X, DEFAULT_SWIPE_COUNT_X);
	g_enroll_config.enroll_too_fast_rollback = core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLLEX_TOO_FAST_ROLLBACK, 0);

	g_navi_config.navi_mode = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_MODE, 1);
	g_navi_config.change_x_y = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_CHANGE_X_Y, 0);
	g_navi_config.change_up_down = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_CHANGE_UP_DOWN, 0);
	g_navi_config.change_left_right = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_CHANGE_LEFT_RIGHT, 0);

	cut_img_config_t crop_config;
	crop_config.enable_cut_img = core_config_get_int(INI_SECTION_SENSOR, KEY_CUT_IMAGE, 0);
	crop_config.algo_sensor_type = core_config_get_int(INI_SECTION_SENSOR, KEY_ALGO_INIT_SENSOR_TYPE, 0);
	crop_config.crop_width = core_config_get_int(INI_SECTION_SENSOR, KEY_CUT_IMAGE_WIDTH, 0);
	crop_config.crop_height = core_config_get_int(INI_SECTION_SENSOR, KEY_CUT_IMAGE_HEIGHT, 0);

	if (memcmp(&crop_config, &g_crop_config, sizeof(cut_img_config_t))) {
		if (!need_reinit) {
			opm_uninitialize_sensor();
			retval = opm_initialize_sensor();
			if (FINGERPRINT_RES_SUCCESS != retval) {
				return FINGERPRINT_RES_FAILED;
			}

			opm_uninitialize_algo();
			retval = opm_initialize_algo();
			if (FINGERPRINT_RES_SUCCESS != retval) {
				return FINGERPRINT_RES_FAILED;
			}
		}
		
		memcpy(&g_crop_config, &crop_config, sizeof(cut_img_config_t));
	}

	ex_log(LOG_DEBUG, "enroll_method = 0x%x", g_enroll_config.enroll_method);
	return retval;
}

static int destroy_ini_config(void)
{
	return opt_send_data(TYPE_DESTROY_INI_CONFIG, NULL, 0);
}

static BOOL check_fingerprint_id_available(unsigned int fingerprint_id)
{
	unsigned int index;

	ex_log(LOG_DEBUG, "check_fingerprint_id_available enter");

	for (index = 0; index < g_cache_info.fingerprint_ids_count; index++) {
		if (g_cache_info.fingerprint_ids[index] == fingerprint_id) {
			ex_log(LOG_DEBUG,
			       "check_fingerprint_id_available return false, "
			       "the same fingerprint id alredy exist");
			return FALSE;
		}
	}

	ex_log(LOG_DEBUG, "check_fingerprint_id_available return true");
	return TRUE;
}

static BOOL check_enrollment_space_available()
{
	return g_cache_info.fingerprint_ids_count < __SINGLE_UPPER_LIMITS__
		   ? TRUE
		   : FALSE;
}

static int sync_user_cache(unsigned int user_id)
{
	int retval;
	unsigned int i;
	fingerprint_ids_t fps;

	ex_log(LOG_DEBUG, "sync_user_cache enter");

	if (user_id == g_cache_info.user_id) {
		return FINGERPRINT_RES_SUCCESS;
	}

	fps.fingerprint_ids_count = 0;
	retval = opm_get_fingerprint_ids(user_id, &fps);
	if (EX_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR, "opm_get_fingerprint_ids return = %d", retval);
		return FINGERPRINT_RES_FAILED;
	}

	memcpy(g_cache_info.fingerprint_ids, fps.fingerprint_ids,
	       fps.fingerprint_ids_count * sizeof(unsigned int));
	g_cache_info.fingerprint_ids_count = fps.fingerprint_ids_count;
	g_cache_info.user_id = user_id;

	if (fps.fingerprint_ids_count > 0) {
		g_cache_info.authenticator_id = 123;  // hard code magic number
		for (i = 0; i < fps.fingerprint_ids_count; i++) {
			g_cache_info.authenticator_id += fps.fingerprint_ids[i];
		}
	} else {
		g_cache_info.authenticator_id = 0;
	}

	ex_log(LOG_DEBUG, "opm_get_fingerprint_ids return = %d, %d", retval, fps.fingerprint_ids_count);
	return FINGERPRINT_RES_SUCCESS;
}

BOOL check_cancelable() { return g_need_cancel; }

BOOL check_need_pause(void) { return g_need_pause; }

static void power_consumption(void)
{
	int ret = 0;
	ret = opm_set_work_mode(POWEROFF_MODE);
	if (ret != 0) {
		ex_log(LOG_ERROR, "power_consumption Fail = %d", ret);
	}
}

static BOOL g_spi_connected = FALSE;
static void setSpiState(BOOL spi_on, BOOL forced)
{
	if (!spi_on && (forced || g_spi_connected)) {
		opm_close_spi();
		g_spi_connected = FALSE;
		ex_log(LOG_VERBOSE, "setSpiState: spi_on = %d, forced = %d, g_spi_connected = %d", spi_on, forced, g_spi_connected);
	} else if (spi_on && (forced || !g_spi_connected)) {
		opm_open_spi();
		g_spi_connected = TRUE;
		ex_log(LOG_VERBOSE, "setSpiState: spi_on = %d, forced = %d, g_spi_connected = %d", spi_on, forced, g_spi_connected);
	} else {
		ex_log(LOG_DEBUG, "setSpiState do nothing, spi_on = %d, forced = %d, g_spi_connected = %d", spi_on, forced, g_spi_connected);
	}
}
const int INTERRUPT_TIMEOUT = 30;
const int MAX_CONTINUOUS_IMG_REJECTED_COUNT = 10;
const int MAX_IMG_REJECTED_COUNT = 20;
static int touch_enroll(unsigned int *percentage, unsigned int enrolled_count)
{
	int retval = FINGERPRINT_RES_SUCCESS;
	unsigned int status;
	unsigned int estate = ESTATE_WAIT_INT;
	unsigned int img_quality;
	cmd_enrollresult_t enroll_result;
	BOOL is_power_off_mode = FALSE;
	memset(&enroll_result, 0, sizeof(cmd_enrollresult_t));
	g_need_pause = FALSE;
	int img_rejected_count = 0;
	int continuous_img_rejected_count = 0;
#ifdef __OPLUS_K4__
	g_fingerprint_opt = FINGERPRINT_OPTION_ENROLL;
#endif	

#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__) || defined (__SHOW_LIVEIMAGE__)
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, 6, FALSE, 0);
#endif

	do {
		switch (estate) {
			case ESTATE_PAUSE:
				ex_log(LOG_INFO, "estate == ESTATE_PAUSE");
				if (check_need_pause() && !is_power_off_mode) {
					opm_set_work_mode(POWEROFF_MODE);
					is_power_off_mode = TRUE;
				}
				
				if (!check_need_pause()) {
					estate = ESTATE_WAIT_INT;
					is_power_off_mode = FALSE;
				}

				if (check_cancelable()) {
					g_need_pause = FALSE;
					return FINGERPRINT_RES_CANCEL;
				}

				plat_sleep_time(20);
				break;

			case ESTATE_WAIT_INT:
				ex_log(LOG_INFO, "estate == ESTATE_WAIT_INT");
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "ESTATE_WAIT_INT set detect mode failed return = %d", retval);
				}
				notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);

				if(wait_trigger(0, INTERRUPT_TIMEOUT)) {
					estate = ESTATE_GET_IMG;
					ex_log(LOG_DEBUG, "notify finger touch");
					notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);
					break;
				}

				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }

				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

			case ESTATE_GET_IMG:
				ex_log(LOG_INFO, "estate == ESTATE_GET_IMG");

				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				retval = opm_get_image(OPTION_NORMAL, &img_quality);
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__) || defined (__SHOW_LIVEIMAGE__)
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, img_quality, FALSE, 0);
#endif

				ex_log(LOG_INFO, "opm_get_image retval = %d, quality = %d", retval,img_quality);
				
				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				if (img_quality == FP_LIB_ENROLL_SUCCESS) {
					estate = ESTATE_ENROLL;
					continuous_img_rejected_count = 0;
					notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);
#ifdef __OPLUS_K5__
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
#else
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
#endif

				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY ||
					   img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE) {
					estate = ESTATE_FINGER_OFF;
					img_rejected_count++;
					continuous_img_rejected_count++;
					if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
					retval = opm_set_work_mode(DETECT_MODE);
					if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

					if (!wait_trigger(1, 30)) {
						notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
						estate = ESTATE_WAIT_INT;
#ifdef __OPLUS_K4__
						notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);					
#endif						
					} else {
#ifdef __OPLUS_K4__
						notify(EVENT_IMG_BAD_QLTY, g_enroll_info.fingerprint_info.fingerprint_id, enroll_result.percentage, NULL, 0);
#else
						notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);
#endif						
					}
				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
					estate = ESTATE_FINGER_OFF;
					img_rejected_count++;
					continuous_img_rejected_count++;
#ifdef __OPLUS_K4__
					notify(EVENT_IMG_PARTIAL, g_enroll_info.fingerprint_info.fingerprint_id, enroll_result.percentage, NULL, 0);
#else					
					notify(EVENT_IMG_PARTIAL, 0, 0, NULL, 0);
#endif					
				} else if (img_quality == FP_LIB_ENROLL_HELP_TOO_WET) {
					estate = ESTATE_FINGER_OFF;
					img_rejected_count++;
					continuous_img_rejected_count++;
					notify(EVENT_IMG_WATER, 0, 0, NULL, 0);
				} else
					estate = ESTATE_WAIT_INT;

				if(img_rejected_count >= MAX_IMG_REJECTED_COUNT || continuous_img_rejected_count >= MAX_CONTINUOUS_IMG_REJECTED_COUNT) {
					ex_log(LOG_ERROR, "imag rejected count >= max value img_rejected_count:%d continuous_count:%d", img_rejected_count, continuous_img_rejected_count);
					notify(EVENT_TOO_MAUCH_IMG_REJECTED, 0, 0, NULL, 0);
#ifdef __OPLUS_K4__
					retval = FINGERPRINT_RES_ENROLL_REJECTED ;
#endif
				}
				break;

			case ESTATE_ENROLL:
				ex_log(LOG_INFO, "estate == ESTATE_ENROLL");

				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				retval = opm_do_enroll(&enroll_result, ENROLL_OPTION_NORMAL, enrolled_count);
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_identify_enroll failed, return %d", retval);
					break;
				}

				ex_log(LOG_INFO, "opm_do_enroll status %u", enroll_result.status);
				ex_log(LOG_INFO, "opm_do_enroll dx = %d, dy = %d, score = %d",
				       enroll_result.swipe_info.dx, enroll_result.swipe_info.dy,
				       enroll_result.swipe_info.similarity_score);

				*percentage = enroll_result.percentage;

#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__)
				transfer_frames_to_client(TRANSFER_ENROLL_IMAGE, img_quality, FALSE, 0);
#endif

				estate = ESTATE_FINGER_OFF;
				if (enroll_result.status == FP_LIB_ENROLL_HELP_SAME_AREA) {
					notify(EVENT_ENROLL_HIGHLY_SIMILAR, 0, 0, NULL, 0);

				} else if (enroll_result.status == FP_LIB_ENROLL_SUCCESS) {
					if (enroll_result.percentage < 100) {
						notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id,
					       enroll_result.percentage, NULL, 0);
					}

				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_QUALITY) {
					notify(EVENT_IMG_BAD_QLTY, g_enroll_info.fingerprint_info.fingerprint_id,
					       enroll_result.percentage, NULL, 0);

				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
					notify(EVENT_IMG_TOO_PARTIAL, g_enroll_info.fingerprint_info.fingerprint_id,
					       enroll_result.percentage, NULL, 0);

				} else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
					//Todo

				} else if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
					notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);

				} else {
					retval = FINGERPRINT_RES_ALGORITHM_ERROR;
				}

				break;

			case ESTATE_FINGER_OFF:
				ex_log(LOG_INFO, "estate == ESTATE_FINGER_OFF");

				retval = opm_set_work_mode(FOD_MODE);
#ifdef __OPLUS_K4__
				wait_trigger(1,30);
#endif
				if (!wait_trigger(0, 30)) {
					if (check_need_pause()) {
						estate = ESTATE_PAUSE;
						break;
					}
					if (check_cancelable()) 
					{
						retval = FINGERPRINT_RES_CANCEL;
						goto Exit;
					}
				}

				if (check_cancelable()) 
				{
					retval = FINGERPRINT_RES_CANCEL;
					goto Exit;
				}
				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				opm_check_finger_lost(30, &status);
				if (check_cancelable()) 
				{
					retval = FINGERPRINT_RES_CANCEL;
					goto Exit;
				}
				ex_log(LOG_DEBUG, "[EGIS_ENROLL] opm_check_finger_lost status:%d", status);

#ifdef __SDK_SAVE_FINGER_OFF_IMAGE__
				if (status == FP_LIB_FINGER_LOST) {
					sprintf(g_finger_lost_info, "%s", "finger_lost");
				} else {
					sprintf(g_finger_lost_info, "%s", "finger_touch");
				}

				transfer_frames_to_client(TRANSFER_ENROLL_FINGER_LOST_IMAGE, 0, FALSE, 0);
#endif
				if (status == FP_LIB_FINGER_LOST) {
					notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
					estate = ESTATE_WAIT_INT;
					ex_log(LOG_INFO, "EVENT_FINGER_LEAVE ======");
				}
				break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS && enroll_result.percentage < 100);

Exit:
	setSpiState(FALSE, FALSE);
	return retval;
}

static int do_enroll()
{
	int retval;
	unsigned int percentage = 0, enrolled_count = 0;
	int fid_size;
	receive_template_in_t in_data;

	retval = opm_enroll_initialize();
	if (retval != 0) {
		notify(EVENT_ENROLL_FAILED, 0, 0, NULL, 0);
		return retval;
	}

	if (core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLL_DUPLICATE, 1)) {
		retval = opm_get_enrolled_count(&enrolled_count);
		if (retval) {
			ex_log(LOG_ERROR, "opm_get_enrolled_count failed, return %d", retval);
			goto finish;
		}
	}

	retval = touch_enroll(&percentage, enrolled_count);

	ex_log(LOG_INFO, "percentage = %d,enroll mode 0x%x", percentage, g_enroll_config.enroll_method);
	ex_log(LOG_INFO, "ENROLL loop end!!!!!! retval = %d", retval);

finish:
	opm_enroll_uninitialize();

	if (percentage >= 100) {
		percentage = 100;
		retval = opm_save_enrolled_fingerprint(g_enroll_info);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			ex_log(LOG_ERROR, "saved fingerprint failed");
		} else {
			fid_size = sizeof(receive_template_in_t);
			in_data.receive_type = ENROLL_RECEIVE_TEMPLATE;
			in_data.fid = g_enroll_info.fingerprint_info.fingerprint_id;

			g_cache_info.user_id = -1;
			sync_user_cache(g_enroll_info.fingerprint_info.user_id);
			notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, percentage, NULL, 0);
			
			opt_receive_data(TYPE_RECEIVE_USER_INFO, (unsigned char*)&g_enroll_info.fingerprint_info.user_id, sizeof(g_enroll_info.fingerprint_info.user_id), NULL, 0);
			opt_receive_data(TYPE_RECEIVE_TEMPLATE, (unsigned char *)&in_data, fid_size, NULL, 0);
			opt_receive_data(TYPE_RECEIVE_ENROLL_MASK, NULL, 0, NULL, 0);

			notify(EVENT_ENROLL_SUCCESS, 0, 0, NULL, 0);
		}
	}

	if (retval == FINGERPRINT_RES_CANCEL) {
		notify(EVENT_ENROLL_CANCELED, 0, 0, NULL, 0);
	} else if (retval != FINGERPRINT_RES_SUCCESS) {
		notify(EVENT_ERR_ENROLL, 0, 0, NULL, 0);
	}
	ex_log(LOG_INFO, "do enroll end!!!!!! retval = %d", retval);
	return FINGERPRINT_RES_SUCCESS;
}

#define VERIFY_RETRY_COUNT 1
#ifdef __USE_AUTH_K4__
static int do_verify()
{
	const uint32_t LIVENESS_AUTHENTICATION = 0;
	int retval = 0;
	int fid_size;
	unsigned int status;
	unsigned int match_id;
	BOOL has_result = FALSE;
	unsigned int quality;
	unsigned int vstate = VSTATE_CHECK_FINGER_ON;
	unsigned char auth_token[AUTH_TOKEN_LEN];
	unsigned int auth_token_len = AUTH_TOKEN_LEN;
	int get_image_option = OPTION_NORMAL;
	int flow_try_match = core_config_get_int(INI_SECTION_VERIFY, KEY_FLOW_TRY_MATCH, VERIFY_RETRY_COUNT);
	g_fingerprint_opt = FINGERPRINT_OPTION_VERIFY;
	BOOL first_loop_not_match = FALSE;
	BOOL bFirst_Call = TRUE;
	dcs_msg_t dcsmsg;

	ex_log(LOG_INFO, "do_verify : wait trigger once before real wait trigger");
	ex_log(LOG_INFO, "%s, flow_try_match=%d", __func__, flow_try_match);

	if (g_cache_info.fingerprint_ids_count > 0){
		retval = opm_identify_start(LIVENESS_AUTHENTICATION);
		if (retval != FINGERPRINT_RES_SUCCESS) return retval;
	}
	// int verify_retry = VERIFY_RETRY_COUNT;

#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__) || defined (__SHOW_LIVEIMAGE__)
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, 6, FALSE, 0);
#endif

	BOOL bFingerOff_Image = FALSE;

	do {
		switch (vstate) {
			case VSTATE_CHECK_FINGER_ON:{
				ex_log(LOG_INFO, "vstate == VSTATE_CHECK_FINGER_ON");
				
				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) goto EXIT;
				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "VSTATE_CHECK_FINGER_ON set detect mode failed, retval = %d", retval);
					break;
				}

				if (wait_trigger(1, INTERRUPT_TIMEOUT)) {
					ex_log(LOG_DEBUG, "verify time out has trigger!");
					vstate = VSTATE_FINGER_OFF_IMAGE;
					break;
				}

				vstate = VSTATE_WAIT_INT;
			}
			case VSTATE_WAIT_INT: {
				ex_log(LOG_INFO, "vstate == VSTATE_WAIT_INT");
				bFirst_Call = FALSE;
				flow_try_match = VERIFY_RETRY_COUNT;
				first_loop_not_match = FALSE;

				mem_set(&dcsmsg, 0, sizeof(dcsmsg));
				mem_set(&(g_verify_info.dcsmsg), 0, sizeof(dcs_msg_t));
				g_verify_info.dcsmsg.retry_times = -1; //init = -1, because retry_times need start 0
				g_verify_info.is_passed = FALSE;

				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) goto EXIT;
				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "VSTATE_WAIT_INT set detect mode failed, retval = %d", retval);
					break;
				}
				notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);

				if (!wait_trigger(0, INTERRUPT_TIMEOUT)) goto EXIT;
				notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);
			}

			case VSTATE_GET_IMG: {
				ex_log(LOG_INFO, "vstate == VSTATE_GET_IMG");

				if (flow_try_match == VERIFY_RETRY_COUNT) {
					ex_log(LOG_DEBUG, "empty image buffer option!");
					get_image_option = OPTION_EMPTY;
				} else {
					ex_log(LOG_DEBUG, "normal image buffer option!");
					get_image_option = OPTION_NORMAL;
				}

				if (check_cancelable()) goto EXIT;
				retval = opm_get_image(get_image_option, &quality);
				if (check_cancelable()) goto EXIT;

#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__) || defined (__SHOW_LIVEIMAGE__)
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, quality, FALSE, 0);
#endif
				ex_log(LOG_INFO, "opm_get_image retval = %d, quality = %d", retval, quality);

				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				if (FP_LIB_ENROLL_SUCCESS == quality) {
					vstate = VSTATE_VERIFY;
					//notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
					//notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);
				} else if (
				    FP_LIB_ENROLL_FAIL_LOW_QUALITY == quality ||
				    FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE == quality) {
					
					if (check_cancelable()) goto EXIT;
					retval = opm_set_work_mode(DETECT_MODE);
					if (check_cancelable()) goto EXIT;

					if (!wait_trigger(1, 30)) {
						if (flow_try_match <= 0) {
							if (first_loop_not_match) {
								notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
								g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAIL;
							} else {
								notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
								g_verify_info.dcsmsg.quality_score = g_img_qty_score;
								g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_TOO_FAST;
							}
							get_dcs_msg(&dcsmsg, sizeof(dcsmsg));
							notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char *)&dcsmsg, sizeof(dcsmsg));

							vstate = VSTATE_FINGER_OFF_IMAGE;
						} else {
							vstate = VSTATE_GET_IMG;
						}
					} else {
						if (flow_try_match <= 0) {
							if (first_loop_not_match) {
								notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
								g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAIL;
							} else {
								notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);
								g_verify_info.dcsmsg.quality_score = g_img_qty_score;
								g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_LOW_QTY;
							}
							get_dcs_msg(&dcsmsg, sizeof(dcsmsg));
							notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char *)&dcsmsg, sizeof(dcsmsg));

							vstate = VSTATE_FINGER_OFF_IMAGE;
						} else {
							vstate = VSTATE_GET_IMG;
						}
					}

					flow_try_match--;
					
				} else if (FP_LIB_ENROLL_FAIL_LOW_COVERAGE == quality) {
					vstate = VSTATE_VERIFY;
					break;
				} else if (FP_LIB_ENROLL_HELP_TOO_WET == quality) {
					vstate = VSTATE_FINGER_OFF_IMAGE;
					notify(EVENT_IMG_WATER, 0, 0, NULL, 0);
				} else {
					vstate = VSTATE_WAIT_INT;
					break;
				}
#if defined (__USE_EGIS_DEMOTOOL__) || defined (__SDK_SAVE_IMAGE__)				
				if (FP_LIB_ENROLL_FAIL_LOW_QUALITY == quality 
					|| FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE == quality) {
#ifdef __USE_EGIS_DEMOTOOL__					
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, FALSE, 0);
#elif defined (__SDK_SAVE_IMAGE__)
					sprintf(g_match_info, "%s", "bad");
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, TRUE, 0);
#endif
				}
#endif
			} break;

			case VSTATE_VERIFY: {
				unsigned int enrolled_count = 0;
				unsigned char is_tmpl_updated = 0;
				ex_log(LOG_INFO, "vstate == VSTATE_VERIFY");

				retval = opm_get_enrolled_count(&enrolled_count);
				if (retval) {
					ex_log(LOG_ERROR, "opm_get_enrolled_count failed, return %d", retval);
				}
				if (enrolled_count == 0) {
					ex_log(LOG_ERROR, "no fingerprints, identify will be failed");

					notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
					vstate = VSTATE_FINGER_OFF_IMAGE;
					break;
				}

				if (check_cancelable()) goto EXIT;

				auth_token_len = AUTH_TOKEN_LEN;
				ex_log(LOG_DEBUG, "befor opm_do_verify auth_token_len = %d", auth_token_len);
				retval = opm_identify(&g_verify_info, &match_id, &status, auth_token, &auth_token_len);
				ex_log(LOG_INFO, "opm_do_verify retval = %d, status = %u, match_id = %d", retval, status, match_id);
				//if (check_cancelable()) goto EXIT;
				if (retval != FINGERPRINT_RES_SUCCESS) {
					break;
				}

				if (FP_LIB_IDENTIFY_NO_MATCH == status) {
					ex_log(LOG_INFO, "flow_try_match=%d", flow_try_match);
					if ((flow_try_match--) > 0) {
						if (quality == FP_LIB_ENROLL_SUCCESS) {
							first_loop_not_match = TRUE;
						}
						vstate = VSTATE_GET_IMG;
						break;
					} else {
						if (quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
							if (first_loop_not_match) {
								notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
								g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAIL;
							} else {
								notify(EVENT_IMG_PARTIAL, 0, 0, NULL, 0);
								g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_LOW_COVERAGE;
							}
						} else {
							notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
							g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAIL;
						}
					}
				} else if (FP_LIB_IDENTIFY_MATCH == status || FP_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE == status) {
					notify(EVENT_VERIFY_MATCHED, g_verify_info.user_id, match_id,
					       auth_token, auth_token_len);
					has_result = TRUE;
					g_verify_info.is_passed = TRUE;
					g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_MATCHED;
					retval = opm_identify_template_update(&is_tmpl_updated);
					if (retval == FINGERPRINT_RES_SUCCESS && is_tmpl_updated) {
						opm_identify_template_save();
						fid_size = sizeof(match_id);
						opt_receive_data(TYPE_RECEIVE_TEMPLATE, (unsigned char *)&match_id,
								 fid_size, NULL, 0);
					}
				} else {
					ex_log(LOG_ERROR, "unknown identify result %d", status);
				}

				ex_log(LOG_INFO, "===Verify End!===");

				vstate = VSTATE_FINGER_OFF_IMAGE;
				get_dcs_msg(&dcsmsg, sizeof(dcsmsg));
				notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char *)&dcsmsg, sizeof(dcsmsg));
#ifdef __SDK_SAVE_IMAGE__
				if (FP_LIB_IDENTIFY_NO_MATCH == status) {
					sprintf(g_match_info, "%d_%d_%s", g_verify_info.user_id, match_id, "fail");
				} else if (FP_LIB_IDENTIFY_MATCH == status || FP_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE == status) {
					sprintf(g_match_info, "%d_%d_%s", g_verify_info.user_id, match_id, "success");
				}
#endif

#if defined (__USE_EGIS_DEMOTOOL__) || defined (__SDK_SAVE_IMAGE__)
				if (quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
#ifdef __USE_EGIS_DEMOTOOL__
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, !(FP_LIB_IDENTIFY_NO_MATCH == status), status);
#elif defined (__SDK_SAVE_IMAGE__)
					if (FP_LIB_IDENTIFY_NO_MATCH == status) {
						sprintf(g_match_info, "%s", "partial");
					}
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, TRUE, status);
#endif
				} else {
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, FALSE, status);
				}
#endif
			} break;

			case VSTATE_FINGER_OFF_IMAGE:{
				ex_log(LOG_INFO, "vstate == VSTATE_FINGER_OFF_IMAGE");

				if (check_cancelable()) goto EXIT;
				opm_check_finger_lost(30, &status);
				if (check_cancelable()) goto EXIT;
				ex_log(LOG_DEBUG, "[EGIS_VERIFY] opm_check_finger_lost status:%d", status);

#ifdef __SDK_SAVE_FINGER_OFF_IMAGE__
				if (status == FP_LIB_FINGER_LOST) {
					sprintf(g_finger_lost_info, "%s", "finger_lost");
				} else {
					sprintf(g_finger_lost_info, "%s", "finger_touch");
				}

				transfer_frames_to_client(TRANSFER_VERIFY_FINGER_LOST_IMAGE, 0, FALSE, 0);
#endif				

				if (status == FP_LIB_ERROR_GENERAL){
#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__) || defined (__SHOW_LIVEIMAGE__)
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, 6, FALSE, 0);
#endif
				}

				if ((status == FP_LIB_FINGER_LOST || status == FP_LIB_ERROR_GENERAL) && bFirst_Call) {
					bFingerOff_Image = FALSE;
					vstate = VSTATE_WAIT_INT;
					break;
				}

				if(status == FP_LIB_FINGER_LOST)
					bFingerOff_Image = TRUE;
				else
					bFingerOff_Image = FALSE;

				vstate = VSTATE_FINGER_OFF_INTERRUPT;				
			}

			case VSTATE_FINGER_OFF_INTERRUPT: {
				ex_log(LOG_INFO, "vstate == VSTATE_FINGER_OFF_INTERRUPT, bFingerOff_Image = %d", bFingerOff_Image);

				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) goto EXIT;

				if ( bFingerOff_Image == TRUE ||!wait_trigger(3, INTERRUPT_TIMEOUT) ) {
					if (check_cancelable()) goto EXIT;

					vstate = VSTATE_WAIT_INT;
					notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
					if (has_result) {
						goto EXIT;
					}
				} else {
					vstate = VSTATE_FINGER_OFF_IMAGE;
				}
			} break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS);
EXIT:
	setSpiState(FALSE, FALSE);

	if (g_cache_info.fingerprint_ids_count > 0){
		opm_identify_finish();
	}
	
	if (retval != FINGERPRINT_RES_SUCCESS) {
		notify(EVENT_ERR_IDENTIFY, 0, 0, NULL, 0);
	}

#ifdef __USE_EGIS_DEMOTOOL__
	notify(EVENT_IDENTIFY_FINISH, 0, 0, NULL, 0);
#endif

	return FINGERPRINT_RES_SUCCESS;
}
#else
static int do_verify()
{
	const uint32_t LIVENESS_AUTHENTICATION = 0;
	int retval = FINGERPRINT_RES_SUCCESS;
	int fid_size;
	unsigned int status;
	unsigned int match_id;
	BOOL has_result = FALSE;
	unsigned int quality;
	unsigned int vstate = VSTATE_WAIT_INT;
	unsigned char auth_token[AUTH_TOKEN_LEN];
	unsigned int auth_token_len = AUTH_TOKEN_LEN;
	int get_image_option = OPTION_NORMAL;
	int flow_try_match = core_config_get_int(INI_SECTION_VERIFY, KEY_FLOW_TRY_MATCH, 1);
	BOOL first_loop_not_match = FALSE;

	ex_log(LOG_INFO, "do_verify : wait trigger once before real wait trigger");
	ex_log(LOG_INFO, "%s, flow_try_match=%d", __func__, flow_try_match);

	if (g_cache_info.fingerprint_ids_count > 0){
		retval = opm_identify_start(LIVENESS_AUTHENTICATION);
		if (retval != FINGERPRINT_RES_SUCCESS) return retval;
	}

	do {
		switch (vstate) {
			case VSTATE_WAIT_INT: {
				ex_log(LOG_INFO, "vstate == VSTATE_WAIT_INT");
#ifdef __ET0XX__
				flow_try_match = 0;
#else
				flow_try_match = 1;
#endif
				first_loop_not_match = FALSE;

				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) goto EXIT;
				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "VSTATE_WAIT_INT set detect mode failed, retval = %d", retval);
					break;
				}
				notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);
#ifndef __ET0XX__

				if (wait_trigger(1, INTERRUPT_TIMEOUT)) {
					ex_log(LOG_DEBUG, "verify time out has trigger!");
#ifndef __OPLUS_K5__
					vstate = VSTATE_FINGER_OFF;
#endif
					break;
				}
#endif
				if (!wait_trigger(0, INTERRUPT_TIMEOUT)) goto EXIT;
				notify(EVENT_FINGER_TOUCH,0,0,NULL,0);
			}

			case VSTATE_GET_IMG: {
				ex_log(LOG_INFO, "vstate == VSTATE_GET_IMG");

				if (flow_try_match == 1) {
					ex_log(LOG_DEBUG, "empty image buffer option!");
					get_image_option = OPTION_EMPTY;
				} else {
					ex_log(LOG_DEBUG, "normal image buffer option!");
					get_image_option = OPTION_NORMAL;
				}

				if (check_cancelable()) goto EXIT;
				retval = opm_get_image(get_image_option, &quality);
				if (check_cancelable()) goto EXIT;

				ex_log(LOG_INFO, "opm_get_image retval = %d, quality = %d", retval, quality);

				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				if (FP_LIB_ENROLL_SUCCESS == quality) {
					vstate = VSTATE_VERIFY;
#ifdef __OPLUS_K5__
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
#else
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
#endif
				} else if (
				    FP_LIB_ENROLL_FAIL_LOW_QUALITY == quality ||
				    FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE == quality) {
					
					if (check_cancelable()) goto EXIT;
					retval = opm_set_work_mode(DETECT_MODE);
					if (check_cancelable()) goto EXIT;

					if (!wait_trigger(1, 30)) {
						if (flow_try_match <= 0) {
							if (first_loop_not_match) {
								notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
							} else {
								notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
							}

							vstate = VSTATE_FINGER_OFF;
						} else {
							vstate = VSTATE_GET_IMG;
						}
					} else {
						if (flow_try_match <= 0) {
							if (first_loop_not_match) {
								notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
							} else {
								notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);
							}

							vstate = VSTATE_FINGER_OFF;
						} else {
							vstate = VSTATE_GET_IMG;
						}
					}

					flow_try_match--;
					
				} else if (FP_LIB_ENROLL_FAIL_LOW_COVERAGE == quality) {
					vstate = VSTATE_VERIFY;
					break;
				} else if (FP_LIB_ENROLL_HELP_TOO_WET == quality) {
					vstate = VSTATE_FINGER_OFF;
					notify(EVENT_IMG_WATER, 0, 0, NULL, 0);
				} else {
					vstate = VSTATE_WAIT_INT;
					break;
				}
#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__)				
				if (FP_LIB_ENROLL_FAIL_LOW_QUALITY == quality 
					|| FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE == quality) {
#ifdef __SUPPORT_SAVE_IMAGE__					
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, FALSE, 0);
#elif defined (__SDK_SAVE_IMAGE__)
					sprintf(g_match_info, "%s", "bad");
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, TRUE, 0);
#endif
				}
#endif
			} break;

			case VSTATE_VERIFY: {
				unsigned int enrolled_count = 0;
				unsigned char is_tmpl_updated = 0;
				ex_log(LOG_INFO, "vstate == VSTATE_VERIFY");

				retval = opm_get_enrolled_count(&enrolled_count);
				if (retval) {
					ex_log(LOG_ERROR, "opm_get_enrolled_count failed, return %d", retval);
				}
				if (enrolled_count == 0) {
					ex_log(LOG_ERROR, "no fingerprints, identify will be failed");

					notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
					vstate = VSTATE_FINGER_OFF;
					break;
				}

				if (check_cancelable()) goto EXIT;
				
				auth_token_len = AUTH_TOKEN_LEN;
				ex_log(LOG_DEBUG, "befor opm_do_verify auth_token_len = %d", auth_token_len);
				retval = opm_identify(&g_verify_info, &match_id, &status, auth_token, &auth_token_len);
				ex_log(LOG_INFO, "opm_do_verify retval = %d, status = %u, match_id = %d", retval, status, match_id);
				//if (check_cancelable()) goto EXIT;
				if (retval != FINGERPRINT_RES_SUCCESS) {
					break;
				}

				if (FP_LIB_IDENTIFY_NO_MATCH == status) {
					ex_log(LOG_INFO, "flow_try_match=%d", flow_try_match);
					if ((flow_try_match--) > 0) {
						if (quality == FP_LIB_ENROLL_SUCCESS) {
							first_loop_not_match = TRUE;
						}
						vstate = VSTATE_GET_IMG;
						break;
					} else {
						if (quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
							if (first_loop_not_match) {
								notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
							} else {
								notify(EVENT_IMG_PARTIAL, 0, 0, NULL, 0);
							}
						} else {
							notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
						}
					}
				} else if (FP_LIB_IDENTIFY_MATCH == status || FP_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE == status) {
					notify(EVENT_VERIFY_MATCHED, g_verify_info.user_id, match_id,
					       auth_token, auth_token_len);
					has_result = TRUE;
					retval = opm_identify_template_update(&is_tmpl_updated);
					if (retval == FINGERPRINT_RES_SUCCESS && is_tmpl_updated) {
						opm_identify_template_save();
						fid_size = sizeof(match_id);
						opt_receive_data(TYPE_RECEIVE_TEMPLATE, (unsigned char *)&match_id,
								 fid_size, NULL, 0);
					}
				} else {
					ex_log(LOG_ERROR, "unknown identify result %d", status);
				}

				ex_log(LOG_INFO, "===Verify End!===");

				opt_receive_data(TYPE_RECEIVE_PATTERN_INFO, NULL, 0, NULL, 0);
				vstate = VSTATE_FINGER_OFF;
#ifdef __ET0XX__
				has_result = TRUE;
#endif
#ifdef __SDK_SAVE_IMAGE__
				if (FP_LIB_IDENTIFY_NO_MATCH == status) {
					sprintf(g_match_info, "%d_%d_%s", g_verify_info.user_id, match_id, "fail");
				} else if (FP_LIB_IDENTIFY_MATCH == status || FP_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE == status) {
					sprintf(g_match_info, "%d_%d_%s", g_verify_info.user_id, match_id, "success");
				}
#endif

#if defined (__SUPPORT_SAVE_IMAGE__) || defined (__SDK_SAVE_IMAGE__)
				if (quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
#ifdef __SUPPORT_SAVE_IMAGE__					
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, !(FP_LIB_IDENTIFY_NO_MATCH == status), status);
#elif defined (__SDK_SAVE_IMAGE__)				
					if (FP_LIB_IDENTIFY_NO_MATCH == status) {
						sprintf(g_match_info, "%s", "partial");
					}
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, TRUE, status);
#endif
				} else {
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, FALSE, status);
				}
#endif
			} break;

			case VSTATE_FINGER_OFF: {
				ex_log(LOG_INFO, "vstate == VSTATE_FINGER_OFF");

				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(FOD_MODE);
				if (check_cancelable()) goto EXIT;

				if (!wait_trigger(0, 30)) {					
					goto EXIT;
				}

				if (check_cancelable()) goto EXIT;
				opm_check_finger_lost(30, &status);
				if (check_cancelable()) goto EXIT;
				ex_log(LOG_DEBUG, "[EGIS_VERIFY] opm_check_finger_lost status:%d", status);

#ifdef __SDK_SAVE_FINGER_OFF_IMAGE__
				if (status == FP_LIB_FINGER_LOST) {
					sprintf(g_finger_lost_info, "%s", "finger_lost");
				} else {
					sprintf(g_finger_lost_info, "%s", "finger_touch");
				}

				transfer_frames_to_client(TRANSFER_VERIFY_FINGER_LOST_IMAGE, 0, FALSE, 0);
#endif
				if (status == FP_LIB_FINGER_LOST) {
					vstate = ESTATE_WAIT_INT;
					notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
					if (has_result) {
						goto EXIT;
					}
				}
				
			} break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS);
EXIT:
	setSpiState(FALSE, FALSE);

	if (g_cache_info.fingerprint_ids_count > 0){
		opm_identify_finish();
	}

	if (retval != FINGERPRINT_RES_SUCCESS) {
		notify(EVENT_ERR_IDENTIFY, 0, 0, NULL, 0);
	}

#ifdef __USE_EGIS_DEMOTOOL__
	notify(EVENT_IDENTIFY_FINISH, 0, 0, NULL, 0);
#endif

	return FINGERPRINT_RES_SUCCESS;
}
#endif

int cpt_initialize(unsigned char *in_data, unsigned int in_data_len)
{
	int retval;
	g_hardware_ready = FALSE;

	if (g_hdev <= 0) {
		retval = fp_device_open(&g_hdev);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			ex_log(LOG_ERROR,
			       "cpt_initialize fp_device_open failed");
			retval = FINGERPRINT_RES_HW_UNAVALABLE;
			goto EXIT;
		}
	}


#ifdef __OPLUS_K4__
	retval = fp_device_poweron(g_hdev);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize fp_device_power_control failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}
#else
	retval = fp_device_power_control(g_hdev, TRUE);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize fp_device_power_control failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}
#endif

#ifdef __OPLUS_K5__
	if (!isPinInit) {
		retval = fp_device_pin_init(g_hdev);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			ex_log(LOG_ERROR,
			       "cpt_initialize fp_device_pin_init failed");
			retval = FINGERPRINT_RES_HW_UNAVALABLE;
			goto EXIT;
		}
	}else
		ex_log(LOG_DEBUG,
			       "cpt_initialize has pin_init ,skiped");

	isPinInit = TRUE;
#endif

	retval = fp_device_reset(g_hdev);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize fp_device_reset failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}
	
	retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_INIT);
	if (FINGERPRINT_RES_SUCCESS == retval) {
		retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_CLOSE);
	}
	
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR, "cpt_initialize fp_device_interrupt_enable failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}
	
	retval = opm_initialize_sdk(in_data, in_data_len);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize opm_initialize_sdk return = %d", retval);
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

	retval = create_ini_config(TRUE, NULL, 0);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize create_ini_config return = %d", retval);

		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

	retval = opm_initialize_sensor();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize opm_initialize_sensor return = %d",
		       retval);
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

#ifdef __OPLUS_K5__
	retval = fp_device_create_fpid(g_hdev);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize fp_device_create_fpid failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}
#endif
	retval = opm_initialize_algo();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize opm_initialize_algo return = %d",
		       retval);
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

	retval = opm_calibration(0, PID_COMMAND, NULL, 0);
	if (EX_RES_SUCCESS == retval ||
	    EX_RES_NOT_NEED_RECALIBRATION == retval ||
	    EX_RES_USE_OLD_CALIBRATION_DATA == retval) {
		g_hardware_ready = TRUE;
		retval = FINGERPRINT_RES_SUCCESS;
	} else {
		ex_log(LOG_ERROR,
		       "cpt_initialize opm_calibration return = %d\n", retval);
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}

#ifdef __TRUSTONIC__
	memset(&g_fix_data, 0, sizeof(g_fix_data));
#endif

	thread_manager_init();
	thread_manager_set_cancel_func((rbsCancelFunc)captain_cancel);
#ifdef __ENABLE_NAVIGATION__
	thread_manager_set_idle_task((do_operation_callback)do_navigation);
#else
	thread_manager_set_idle_task((do_operation_callback)power_consumption);
#endif
EXIT:
	return retval;
}

int cpt_uninitialize()
{
	int retval;
	cpt_cancel();

	if (0 != g_hdev) {
		fp_device_power_control(g_hdev, FALSE);
		fp_device_close(g_hdev);
		g_hdev = 0;
	}

	g_hardware_ready = FALSE;

	destroy_ini_config();

	retval = opm_uninitialize_sensor();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR, "cpt_uninitialize opm_uninitialize_sensor return = %d", retval);
	}
	retval = opm_uninitialize_algo();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR, "cpt_uninitialize opm_uninitialize_algo return = %d", retval);
	}
	retval = opm_uninitialize_sdk();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR, "cpt_uninitialize opm_uninitialize_sdk return = %d", retval);
	}

	thread_manager_uninitialize();

	return 0;
}

int cpt_cancel()
{
	ex_log(LOG_DEBUG, "cpt_cancel enter");
	g_has_enroll_count = 0;
	g_enroll_percentage_tmp = 0;
#ifdef __USE_AUTH_K4__
	g_fingerprint_opt = FINGERPRINT_OPTION_OTHERS;
#endif	
	thread_manager_cancel_task();

	ex_log(LOG_DEBUG, "cpt_cancel end");

	return FINGERPRINT_RES_SUCCESS;
}

void captain_cancel(BOOL cancel_flag)
{
	ex_log(LOG_DEBUG, "captain_cancel enter [%d]", cancel_flag);

	g_need_cancel = cancel_flag;
}

int cpt_set_active_group(unsigned int user_id, const char *data_path)
{
	int retval, result_sync_cache;

	ex_log(LOG_DEBUG, "cpt_set_active_group user_id = %u, data_path = %s",
	       user_id, data_path);

	if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_set_active_group another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}

	strncpy(g_user_path, data_path, MAX_PATH_LEN - 1);
	opt_send_data(TYPE_SEND_USER_INFO, (unsigned char*)&user_id, sizeof(user_id));
	opt_send_data(TYPE_SEND_TEMPLATE, NULL, 0);
	opt_send_data(TYPE_UPDATA_PATTERN_INFO_TO_USER, NULL, 0);
	
	retval = opm_set_active_group(user_id, data_path);
	ex_log(LOG_DEBUG, "opm_set_active_group return = %d", retval);
	if (FINGERPRINT_RES_SUCCESS == retval) {	
		opt_send_data(TYPE_SEND_PATTERN_INFO, NULL, 0);
		opt_send_data(TYPE_SEND_ENROLL_MASK, NULL, 0);
		
		g_cache_info.user_id = -1;
		result_sync_cache = sync_user_cache(user_id);
		ex_log(LOG_DEBUG, "sync_user_cache return = %d",
		       result_sync_cache);
	}

	thread_unlock_operation();
	return FINGERPRINT_RES_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS
						 : FINGERPRINT_RES_FAILED;
}

int cpt_set_data_path(unsigned int data_type, const char *data_path,
		      unsigned int path_len)
{
	int retval;

	ex_log(
	    LOG_DEBUG,
	    "cpt_set_data_path data_type = %u, data_path = %s, path_len = %u",
	    data_type, data_path, path_len);

	if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_set_data_path another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}

	if (1 == data_type) {
		ex_log(LOG_DEBUG, "cpt_set_data_path set_log_path");
	}

	retval = opm_set_data_path(data_type, data_path, path_len);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR, "opm_set_data_path return = %d", retval);
	}

	thread_unlock_operation();

	return FINGERPRINT_RES_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS
						 : FINGERPRINT_RES_FAILED;
}

int cpt_chk_secure_id(unsigned int user_id, unsigned long long secure_id)
{
	int retval = 0;
	retval = opm_chk_secure_id(user_id, secure_id);
	if(retval != FINGERPRINT_RES_SUCCESS) {
		ex_log(LOG_DEBUG, "check secure id failed!");
		return retval;
	}

	retval = opt_receive_data(TYPE_RECEIVE_USER_INFO, (unsigned char*)&g_cache_info.user_id, sizeof(g_cache_info.user_id), NULL, 0);
	ex_log(LOG_DEBUG, "receive user info return %d", retval);
	return retval;
}

int cpt_pre_enroll(fingerprint_enroll_info_t enroll_info)
{
	BOOL check_res;
	int retval;

	if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_pre_enroll another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}

	retval = sync_user_cache(enroll_info.fingerprint_info.user_id);
	if (retval != FINGERPRINT_RES_SUCCESS) {
		goto EXIT;
	}

	check_res = check_fingerprint_id_available(
	    enroll_info.fingerprint_info.fingerprint_id);
	if (TRUE != check_res) {
		ex_log(LOG_ERROR, "fingerprint_id is not available");
		retval = FINGERPRINT_RES_DUPLICATE_ID;
		goto EXIT;
	}

	check_res = check_enrollment_space_available();
	if (TRUE != check_res) {
		ex_log(LOG_ERROR, "there's no space for enrollment");
		retval = FINGERPRINT_RES_NO_SPACE;
	}

	g_enroll_info = enroll_info;

EXIT:
	thread_unlock_operation();

	return retval;
}

int cpt_enroll()
{
	if (TRUE != g_hardware_ready) {
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

	thread_manager_run_task((do_operation_callback)do_enroll, TASK_PROCESS);

	return FINGERPRINT_RES_SUCCESS;
}

int cpt_post_enroll()
{
	if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_post_enroll another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}
	thread_unlock_operation();

	return FINGERPRINT_RES_SUCCESS;
}

int cpt_chk_auth_token(unsigned char *token, unsigned int len)
{
	return opm_chk_auth_token(token, len);
}

int cpt_get_authenticator_id(unsigned long long *id)
{
	int retval = opm_get_authenticator_id(id);
	if (retval != FINGERPRINT_RES_SUCCESS || *id == 0) {
		ex_log(LOG_DEBUG, "opm_get_authenticator_id %d", retval);
		*id = g_cache_info.authenticator_id;
	}
	return FINGERPRINT_RES_SUCCESS;
}

int cpt_authenticate(fingerprint_verify_info_t verify_info)
{
	if (TRUE != g_hardware_ready) {
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

	g_verify_info = verify_info;
	memcpy(g_verify_info.fingerprints.fingerprint_ids,
	       verify_info.fingerprints.fingerprint_ids,
	       sizeof(int) * verify_info.fingerprints.fingerprint_ids_count);

	thread_manager_run_task((do_operation_callback)do_verify, TASK_PROCESS);

	return FINGERPRINT_RES_SUCCESS;
}

int cpt_remove_fingerprint(fingerprint_remove_info_t remove_info)
{
	int retval, result_sync_cache, fid_size;

	if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_remove_fingerprint another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}

	retval = opm_remove_fingerprint(remove_info);

	if ((retval == EX_RES_SUCCESS) || (retval == FP_LIB_ERROR_INVALID_FINGERID))
		retval = FINGERPRINT_RES_SUCCESS;
	else
		retval = FINGERPRINT_RES_FAILED;

	/*
	** reset cache user_id to default
	** then sync the fingerprints info to the local cache
	*/
	g_cache_info.user_id = -1;

	result_sync_cache =
	    sync_user_cache(remove_info.fingerprint_info.user_id);
	if (FINGERPRINT_RES_SUCCESS != result_sync_cache) {
		ex_log(LOG_ERROR,
		       "cpt_remove_fingerprint sync_user_cache return = %d",
		       result_sync_cache);
	}

	fid_size = sizeof(remove_info.fingerprint_info.fingerprint_id);
	opt_receive_data(TYPE_RECEIVE_USER_INFO, (unsigned char*)&remove_info.fingerprint_info.user_id, sizeof(remove_info.fingerprint_info.user_id), NULL, 0);
	opt_receive_data(TYPE_DELETE_TEMPLATE, (unsigned char *)&remove_info.fingerprint_info.fingerprint_id, fid_size, NULL, 0);
	thread_unlock_operation();

	return retval;
}

int cpt_get_fingerprint_ids(unsigned int user_id, fingerprint_ids_t *fps)
{
	/*if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_get_fingerprint_ids another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}*/

	/* checks if local cache is updated
	** if it isnt updated, make it synchronized*/
	int retval = sync_user_cache(user_id);
	if (retval != FINGERPRINT_RES_SUCCESS) {
		ex_log(LOG_ERROR,
		       "cpt_get_fingerprint_ids sync_user_cache return = %d",
		       retval);
		goto EXIT;
	}

	fps->fingerprint_ids_count = g_cache_info.fingerprint_ids_count;
	memcpy(fps->fingerprint_ids, g_cache_info.fingerprint_ids,
	       g_cache_info.fingerprint_ids_count * sizeof(unsigned int));
EXIT:
	//thread_unlock_operation();

	return FINGERPRINT_RES_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS
						 : FINGERPRINT_RES_FAILED;
}
#if 0
// ToDo:
void cpt_disable_navigation(BOOL onoff)
{
	if (onoff) thread_manager_set_idle_task(do_navigation);
	else       thread_manager_set_idle_task(power_consumption);
}
#endif

int cpt_navigation()
{
	ex_log(LOG_ERROR, "cpt_navigation not support this interface");

	return FINGERPRINT_RES_SUCCESS;
}

void cpt_set_event_callback(event_callbck_t on_event_callback)
{
#ifdef __USE_EGIS_DEMOTOOL__	
	if (g_use_extra) {
		g_extra_callback = on_event_callback;	
	} else 
#endif	
	{
		g_event_callback = on_event_callback;
	}	
}

static void get_ca_version(unsigned char *version, int *len)
{
	if (version == NULL || len == NULL) return;

	int version_len = 0;
	version_len = sizeof("pascal_a_and_c_with_coco_v0.2_43a1b37");
	if (version_len <= *len) {
		*len = version_len;
		memcpy(version, "pascal_a_and_c_with_coco_v0.2_43a1b37" , version_len);
	}

	ex_log(LOG_DEBUG, "version_len = %d , version = %s", *len, version);
}

int evtool_extra(unsigned char *in_data, int in_data_size, unsigned char *out_buffer, int *out_buffer_size)
{
	int retval = FINGERPRINT_RES_SUCCESS;
	int cid  = *((int*)in_data);
	BOOL has_int = FALSE;
	
	
	switch(cid)
	{
		case CMD_CONNECT:
			retval = fp_device_open(&g_hdev);
			if (FINGERPRINT_RES_SUCCESS != retval) {
				ex_log(LOG_ERROR, "cpt_initialize fp_device_open failed");
				return FINGERPRINT_RES_HW_UNAVALABLE;
			}

			retval = fp_device_power_control(g_hdev, TRUE);
			if (FINGERPRINT_RES_SUCCESS != retval) {
				ex_log(LOG_ERROR, "cpt_initialize fp_device_power_control failed");
				return FINGERPRINT_RES_HW_UNAVALABLE;
			}

			retval = fp_device_reset(g_hdev);
			if (FINGERPRINT_RES_SUCCESS != retval) {
				ex_log(LOG_ERROR, "cpt_initialize fp_device_reset failed");
				return FINGERPRINT_RES_HW_UNAVALABLE;
			}

			retval = fp_device_clock_enable(g_hdev, TRUE);
			if (FINGERPRINT_RES_SUCCESS != retval) {
				ex_log(LOG_ERROR, "cpt_initialize fp_device_clock_enable failed");
				return FINGERPRINT_RES_HW_UNAVALABLE;
			}

			retval = opm_extra_command(PID_EVTOOL,in_data, in_data_size, out_buffer, out_buffer_size);
			break;
		case CMD_CALIBRATION:
			retval = opm_extra_command(PID_EVTOOL,in_data, in_data_size, out_buffer, out_buffer_size);
			break;
		case CMD_INTERRUPT_POLLING:
			retval = opm_extra_command(PID_EVTOOL,in_data, in_data_size, NULL, NULL);
			has_int = wait_trigger(10, 500);
			
			memcpy(out_buffer, &has_int, sizeof(BOOL));
			*out_buffer_size = sizeof(BOOL);
			break;
		case CMD_INTERRUPT_REGISTER:
			retval = opm_extra_command(PID_EVTOOL,in_data, in_data_size, out_buffer, out_buffer_size);
			break;
		case CMD_RESET:
			retval = fp_device_reset(g_hdev);
			ex_log(LOG_ERROR, "fp_device_reset retval = %d" , retval);
			break;
		case CMD_GET_IMAGE:
			retval = opm_extra_command(PID_EVTOOL,in_data, in_data_size, out_buffer, out_buffer_size);
			break;
		case CMD_REGISTER_RW:
			do{
				retval = opm_extra_command(PID_EVTOOL, in_data, in_data_size, out_buffer, out_buffer_size);
				ex_log(LOG_DEBUG, "CMD_REGISTER_RW retval:%d", retval);
				if (retval == EV_REGISTER_RW_RESET) {
					retval = fp_device_reset(g_hdev);
				}
			} while (retval == EV_REGISTER_RW_RESET);
			break;
		case CMD_DISCONNECT:
			retval = fp_device_close(g_hdev);
			if(retval != FINGERPRINT_RES_SUCCESS) {
				ex_log(LOG_ERROR, "fp_device_close failed, retval = %d" , retval);
				break;
			}

			retval = opm_extra_command(PID_EVTOOL,in_data, in_data_size, out_buffer, out_buffer_size);
			break;
		default:
			break;
	}

	return retval;
}

int cpt_extra_api(int type, unsigned char *in_data, int in_data_size,
		  unsigned char *out_buffer, int *out_buffer_size)
{
	int retval = FINGERPRINT_RES_SUCCESS, cid = 0;

	if (in_data != NULL) {
		cid = *(int *)in_data;
		
		if(type == PID_EVTOOL)
		{
			retval = evtool_extra(in_data, in_data_size, out_buffer, out_buffer_size);
			ex_log(LOG_DEBUG, "evtool_extra end, retval = %d", retval);
			return retval;
		}

		if (PID_COMMAND == type && CMD_REMOVE_DEVICE_NODE == *in_data) {
			if (g_hdev <= 0) {
				retval = fp_device_open(&g_hdev);
				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "cpt_extra_api fp_device_open failed");
					return FINGERPRINT_RES_HW_UNAVALABLE;
				}
			}

			if (g_hdev > 0) {
				ex_log(LOG_DEBUG, "fp_device_remove_node g_hdev=%d", g_hdev);
				retval = fp_device_close(g_hdev);
				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "cpt_extra_api fp_device_open failed");
					return FINGERPRINT_RES_HW_UNAVALABLE;
				}

				retval = fp_device_remove_node(g_hdev);
				if(FINGERPRINT_RES_SUCCESS == retval) {
					g_hdev = 0;
				}
			}
			ex_log(LOG_DEBUG, "fp_device_remove_node end retval =%d", retval);
			return retval;
		}

		/*if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
			ex_log(LOG_ERROR, "cpt_extra_api another operation is doing ,return not idle");
			return FINGERPRINT_RES_NOT_IDLE;
		}*/

		if (PID_INLINETOOL == type || PID_FAETOOL == type) {
			retval = do_sensortest(cid, in_data, in_data_size, out_buffer, out_buffer_size);
			goto EXIT;
		}

		if (CMD_UPDATE_CONFIG == cid) {
			destroy_ini_config();
			if (in_data_size > (int)sizeof(int)) {
				retval = create_ini_config(FALSE, in_data + sizeof(int), in_data_size - sizeof(int));
			} else {
				retval = create_ini_config(FALSE, NULL, 0);
			}

			goto EXIT;
		}

		if (CMD_SET_NAVI_CONFIG == cid) {
			g_navi_config.navi_mode = ((int *)in_data)[0];
			g_navi_config.change_x_y = ((int *)in_data)[1];
			g_navi_config.change_up_down = ((int *)in_data)[2];
			g_navi_config.change_left_right = ((int *)in_data)[3];
			goto EXIT;
		}

		if (PID_DEMOTOOL == type) {
			switch (cid) {
				case CMD_VERSION_CA:
					get_ca_version(out_buffer, out_buffer_size);
					break;
				case CMD_VERSION_TA:
					retval = opm_get_data(TYPE_RECEIVE_TA_VERSION, NULL, 0, out_buffer, out_buffer_size);
					break;
				case CMD_VERSION_ALGO:
					retval = opm_get_data(TYPE_RECEIVE_ALGO_VERSION, NULL, 0, out_buffer, out_buffer_size);
					break;
#ifdef __USE_EGIS_DEMOTOOL__					
				case CMD_SET_CALLBACK_FLAG:
					g_use_extra = TRUE;
					break;
				case CMD_RESET_CALLBACK_FLAG:
					g_use_extra = FALSE;
					break;
#endif					
				default:
					break;
			}
			goto EXIT;
		}
	}

	retval = opm_extra_command(type, in_data, in_data_size, out_buffer, out_buffer_size);
EXIT:

	//thread_unlock_operation();

	return retval;
}

#define MAX_IMAGE_BUFFER_SIZE (60 * 60 * EGIS_TRANSFER_FRAMES_PER_TIME)

static int set_img_state(transfer_image_type_t type, int img_quality, BOOL force_to_good)
{	
	ex_log(LOG_DEBUG, "set_img_state, type = %d ,img_qty %d", type,img_quality);
	if (type == TRANSFER_ENROLL_IMAGE || type == TRANSFER_ENROLL_RAW) {
		return TRUE;
	}
	else if (type == TRANSFER_VERIFY_IMAGE_V2 || type == TRANSFER_VERIFY_RAW) {
		if (img_quality == FP_LIB_ENROLL_SUCCESS || force_to_good == TRUE) {
			return TRUE;
		} else {
			return FALSE;
		}
	}

	if (img_quality == FP_LIB_ENROLL_SUCCESS)
		return 1;

	if (img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE || img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY)
		return 2;

	if (img_quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE)
		return 3;

	if (img_quality == FP_LIB_ENROLL_FAIL_NONE)
		return 5;

	ex_log(LOG_ERROR, "invalid parameter, type = %d %d", type,img_quality);
	return FALSE;
}

static int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good,__unused int match_result)
{
	int buffer_size = sizeof(receive_images_out_t) + MAX_IMAGE_BUFFER_SIZE;
	int retval = FINGERPRINT_RES_FAILED, width, height,bpp, i,img_size;
	int image_type = TYPE_RECEIVE_MULTIPLE_IMAGE;
	return_image_info_t return_image_info;
	int is_good_img = FALSE;
	BOOL is_new_finger_on = TRUE;

	receive_images_in_t in_data;
	receive_images_out_t *out_buffer = NULL;

#ifndef EGIS_DBG
	return FINGERPRINT_RES_FAILED;
#endif

#if defined (__SDK_SAVE_IMAGE__) || defined (__SDK_SAVE_FINGER_OFF_IMAGE__)
	save_image_info_t save_image_info;
#endif

	ex_log(LOG_DEBUG, "%s (buffer size=%d)", __func__, buffer_size);
	unsigned char *buffer = (unsigned char *)malloc(buffer_size);
	if (buffer == NULL) {
		return retval;
	}

	out_buffer = (receive_images_out_t *)buffer;
	unsigned char *image_buf = buffer + sizeof(receive_images_out_t);

	memset(&in_data, 0, sizeof(receive_images_in_t));
	in_data.image_count_request = EGIS_TRANSFER_FRAMES_PER_TIME;
	in_data.image_type = type;
	in_data.reset_mode = FRAMES_RESET_AUTO;

	return_image_info.info_version = 0x01;

	if (type == TRANSFER_LIVE_IMAGE) {
		in_data.reset_mode = FRAMES_RESET_NEVER;
		image_type = TYPE_RECEIVE_LIVE_IMAGE;
	}

#ifdef __SDK_SAVE_FINGER_OFF_IMAGE__
	if (type == TRANSFER_ENROLL_FINGER_LOST_IMAGE || type == TRANSFER_VERIFY_FINGER_LOST_IMAGE) {
		in_data.reset_mode = FRAMES_RESET_NEVER;
		image_type = TYPE_RECEIVE_FINGET_LOST_IMAGE;
	}
#endif	

	do {
		buffer_size = sizeof(receive_images_out_t) + MAX_IMAGE_BUFFER_SIZE;
		retval = opt_receive_data(image_type, (unsigned char *)&in_data, sizeof(in_data), buffer, &buffer_size);
		if (retval == FINGERPRINT_RES_SUCCESS) {
			width = out_buffer->format.width;
			height = out_buffer->format.height;
			bpp = out_buffer->format.bpp;
			img_size = width * height * bpp / 8;

			//call back images
			if (type == TRANSFER_LIVE_IMAGE) {
				notify(EVENT_RETURN_LIVE_IMAGE, width, height, image_buf, width * height);
				break;
			} else if (type == TRANSFER_VERIFY_IMAGE_V2) {
				ex_log(LOG_DEBUG, "==TRANSFER_VERIFY_IMAGE_V2==");
				is_good_img = set_img_state(type, img_quality, force_to_good);

				return_image_info.is_new_finger_on = is_new_finger_on;
				return_image_info.try_count = out_buffer->identify_info.try_count;
				return_image_info.match_score = out_buffer->identify_info.match_score;
				return_image_info.save_index = out_buffer->identify_info.save_index;

				notify(EVENT_RETURN_IMAGE_INFO, out_buffer->image_count_included, is_good_img, (unsigned char *)&return_image_info, sizeof(return_image_info));
				notify(EVENT_RETURN_IMAGE, width, height, image_buf, img_size * out_buffer->image_count_included);

#ifdef __SDK_SAVE_IMAGE__
				save_image_info.img_type = type;
				if (is_good_img == FALSE) {
					save_image_info.img_state = SAVE_BAD;
				} else if (match_result == FP_LIB_IDENTIFY_NO_MATCH) {
					save_image_info.img_state = SAVE_GOOD_NOT_MATCH;
				} else {
					save_image_info.img_state = SAVE_GOOD_MATCH;
				}
				save_image_info.is_new_finger_on = is_new_finger_on;
				save_image_info.single_img_size = width * height;
				save_image_info.img_cnt = out_buffer->image_count_included;
				save_image_info.select_index = out_buffer->identify_info.save_index;
				save_image_info.img_buf = image_buf;
				//debug_save_image(save_image_info);
				debug_save_image_oplus_k4(save_image_info, g_match_info);
#endif
				is_new_finger_on = FALSE;
			} else if (type == TRANSFER_ENROLL_FINGER_LOST_IMAGE || type == TRANSFER_VERIFY_FINGER_LOST_IMAGE) {
#ifdef __SDK_SAVE_FINGER_OFF_IMAGE__
				ex_log(LOG_DEBUG, "==TRANSFER_FINGER_LOST_IMAGE==");
				save_image_info.img_type = type;
				save_image_info.img_state = SAVE_GOOD_ENROLL;
				save_image_info.is_new_finger_on = TRUE;
				save_image_info.single_img_size = width * height;
				save_image_info.img_cnt = out_buffer->image_count_included;
				save_image_info.select_index = 0;
				save_image_info.img_buf = image_buf;
				debug_save_image_oplus_k4(save_image_info, g_finger_lost_info);
#endif
			}else if(type == TRANSFER_LIVE_IMAGE){
				ex_log(LOG_DEBUG, "%s get image count %d  ,qutality %d ", __func__, out_buffer->image_count_included,img_quality);
				for (i = 0; i < out_buffer->image_count_included; i++) {
					is_good_img = set_img_state(type, img_quality, force_to_good);

					return_image_info.is_new_finger_on = is_new_finger_on;
					// notify(EVENT_RETURN_IMAGE_INFO, 1, is_good_img, (unsigned char*)&return_image_info, sizeof(return_image_info));
					// notify(EVENT_RETURN_IMAGE, width, height, image_buf + i * img_size, img_size);
					notify(EVENT_RETURN_LIVE_IMAGE, width, height, image_buf, width * height);
#ifdef __SDK_SAVE_IMAGE__
					save_image_info.img_type = type;
					if (is_good_img == 2) {
						save_image_info.img_state = SAVE_BAD;
						sprintf(g_match_info, "%d_%s", g_verify_info.user_id,  "bad");
					}else if (is_good_img == 3) {
						save_image_info.img_state = SAVE_PARTIAL;
						sprintf(g_match_info, "%d_%s", g_verify_info.user_id,  "partial");
					}else if (is_good_img == 4) {
						save_image_info.img_state = SAVE_FAKE;
						sprintf(g_match_info, "%d_%s", g_verify_info.user_id,  "fake");
					}else if (is_good_img == 5) {
						save_image_info.img_state = SAVE_FAIL;
						sprintf(g_match_info, "%d_%s", g_verify_info.user_id,  "imagefail");
					}else {
						break;
					}
					save_image_info.single_img_size = width * height;
					save_image_info.is_new_finger_on = is_new_finger_on;
					save_image_info.img_cnt = out_buffer->image_count_included;
					save_image_info.select_index = out_buffer->image_count_included - 1;
					save_image_info.img_buf = image_buf + i * width * height;
					//debug_save_image(save_image_info);
					debug_save_image_oplus_k4(save_image_info, g_match_info);
#endif
				}
			} else {
				for (i = 0; i < out_buffer->image_count_included; i++) {
					is_good_img = set_img_state(type, img_quality, force_to_good);

					return_image_info.is_new_finger_on = is_new_finger_on;
					notify(EVENT_RETURN_IMAGE_INFO, 1, is_good_img, (unsigned char*)&return_image_info, sizeof(return_image_info));
					notify(EVENT_RETURN_IMAGE, width, height, image_buf + i * img_size, img_size);

#ifdef __SDK_SAVE_IMAGE__
					save_image_info.img_type = type;
					if (is_good_img == FALSE) {
						save_image_info.img_state = SAVE_BAD;
					} else {
						save_image_info.img_state = SAVE_GOOD_ENROLL;
					}
					save_image_info.single_img_size = width * height;
					save_image_info.is_new_finger_on = is_new_finger_on;
					save_image_info.img_cnt = 1;
					save_image_info.select_index = 0;
					save_image_info.img_buf = image_buf + i * width * height;
					//debug_save_image(save_image_info);
					debug_save_image_oplus_k4(save_image_info, g_match_info);
#endif
				}
			}

			in_data.image_index_start = out_buffer->image_index_end;
			if (out_buffer->has_more_image == TRANSFER_MORE_NONE) {
				break;
			} else if (out_buffer->has_more_image == TRANSFER_MORE_NEXT_RAW) {
				ex_log(LOG_DEBUG, "to continue to get RAW image");
				in_data.image_index_start = 0;
				if (type == TRANSFER_VERIFY_IMAGE_V2) {
					in_data.image_type = TRANSFER_VERIFY_RAW;
				} else if (type == TRANSFER_ENROLL_IMAGE) {
					in_data.image_type = TRANSFER_ENROLL_RAW;
				} else {
					ex_log(LOG_ERROR, "unexpected type %d", type);
					break;
				}
			}
		} else {
			ex_log(LOG_ERROR, "receive image failed! retval : %d", retval);
			break;
		}
	} while (TRUE);

	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}

	return retval;
}

static int touch_enroll_remaining(int percentage)
{
	ex_log(LOG_DEBUG, "%s enter! percentage = %d", __func__, percentage);

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
		int total = (int)(100 / (float)percentage * g_has_enroll_count);
		remaining = total - g_has_enroll_count;
		ex_log(LOG_DEBUG, "%s has_enroll_count=%d, total=%d, remaining=%d", __func__, g_has_enroll_count, total, remaining);
		if (remaining < 1) remaining = 1;
	} else {
		remaining = g_enroll_config.enroll_max_count;
	}

	return remaining;
	
}

static int swipe_enroll_remaining(int percentage)
{
    ex_log(LOG_DEBUG, "%s enter! percentage = %d", __func__, percentage);

	int remaining;
	if (percentage >= 100) {
		remaining = 0;
	} else if (percentage > 0) {
		remaining = 100 - percentage;
		ex_log(LOG_DEBUG, "%s = %d", __func__, remaining);
		if (remaining < 1) remaining = 1;
	} else {
		remaining = 100;
	}

	return remaining;
}

static int enroll_percentage_to_remaining(int percentage)
{
    int retval = -1;
	switch (g_enroll_config.enroll_method) {
		case ENROLL_METHOD_TOUCH:
			retval = touch_enroll_remaining(percentage);
			break;
		case ENROLL_METHOD_PAINT:
		case ENROLL_METHOD_SWIPE:
			retval = swipe_enroll_remaining(percentage);
			break;
		default:
			break;
	}
	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return retval;
}

int do_image_qty_test()
{
	int in_data[2], image_score;
	int buffer_size = MAX_IMAGE_BUFFER_SIZE + sizeof(liver_image_out_header_t);
	int retval = FINGERPRINT_RES_SUCCESS;
	liver_image_out_header_t *pheader ;
	unsigned char * buffer = NULL ;
	unsigned int status;

	memset(in_data, 0x00, sizeof(in_data));
	in_data[1] = 2001;

	ex_log(LOG_DEBUG, "%s check finger lost enter", __func__);
	do {
		if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
		opm_check_finger_lost(30, &status);
		if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

		if (status == FP_LIB_FINGER_LOST) {
			break;
		}
	} while (TRUE);
	ex_log(LOG_DEBUG, "%s check finger lost end", __func__);

	if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
	retval = opm_set_work_mode(DETECT_MODE);
	if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

	// if (wait_trigger(1,INTERRUPT_TIMEOUT)){
	// 	ex_log(LOG_DEBUG, "%s enter has finger on sensor", __func__);
	// 	if (check_cancelable()) return FINGERPRINT_RES_CANCEL;	
	// }

	if (!wait_trigger(0, INTERRUPT_TIMEOUT)) {
		return FINGERPRINT_RES_CANCEL;
	}

	ex_log(LOG_DEBUG, "%s image test finger touch !!!", __func__);
	
	notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);

	if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
	
	buffer = (unsigned char *)malloc(buffer_size);	
	if (buffer == NULL) {
		return FINGERPRINT_RES_ALLOC_FAILED;
	}
	
	memset(buffer, 0x00, buffer_size);
	
	retval = sensor_test_opation(SENSORTEST_GET_IMAGE, g_hdev, (unsigned char *)in_data, sizeof(in_data), buffer, &buffer_size);

	pheader = (liver_image_out_header_t *)buffer ;

	image_score = pheader->image_par_t.qm_parameter.qty;
	image_score = image_score * 100 / 256;

	notify(EVENT_SENSOR_IMAGE_QTY, retval, image_score, NULL, 0);

	if (NULL != buffer) {
		free(buffer);
		buffer = NULL;
	}
	return retval;
}

static int do_sensortest(int cid, unsigned char *in_data, int in_data_size,
			 unsigned char *out_buffer, int *out_buffer_size)
{
	int retval = FINGERPRINT_RES_FAILED;
	int buffer_size = MAX_IMAGE_BUFFER_SIZE + sizeof(liver_image_out_header_t);

	unsigned char *buffer = (unsigned char *)malloc(buffer_size);
	if (buffer != NULL) {
		g_need_cancel = FALSE;

		if (SENSORTEST_GET_IMAGE == cid) {
			thread_manager_run_task((do_operation_callback)do_image_qty_test, TASK_PROCESS);
			retval = FINGERPRINT_RES_SUCCESS;
			goto EXIT;
		}

		retval = sensor_test_opation(cid, g_hdev, in_data, in_data_size, buffer, &buffer_size);
		if (SENSORTEST_GET_IMAGE_SNR == cid && FINGERPRINT_RES_SUCCESS == retval) {
			memcpy(out_buffer, buffer, buffer_size);
			*out_buffer_size = buffer_size;
		}
		if(SENSORTEST_GET_NVM_UID == cid && FINGERPRINT_RES_SUCCESS == retval) {
			notify(EVENT_SENSROR_TEST_NVM_UID, buffer_size, 0, buffer, buffer_size);
		}
	}

EXIT:
	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}
	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return retval;
}

int cpt_pause(void)
{
	g_need_pause = TRUE;
	return FINGERPRINT_RES_SUCCESS;
}

int cpt_continue(void)
{
	g_need_pause = FALSE;
	return FINGERPRINT_RES_SUCCESS;
}

int cpt_clean_up(void)
{
	return FINGERPRINT_RES_SUCCESS;
}

int touch_lock_mode()
{
	int retval;

	ex_log(LOG_INFO, "%s start ", __func__);

	while (TRUE) {
		retval = opm_set_work_mode(LOCKER_MODE);
		ex_log(LOG_DEBUG, "set lock mode ret %d", retval);
		if (retval != FINGERPRINT_RES_SUCCESS) {
			fp_device_reset(g_hdev);
			plat_sleep_time(20);
			ex_log(LOG_DEBUG, "set lock mode reset and continue");
			continue;			
		}
		
		if (!wait_trigger(0, 30)) {					
			return FINGERPRINT_RES_CANCEL;
		}
		ex_log(LOG_INFO, "wait finger on int");
		notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);
		
		retval = opm_set_work_mode(FOD_MODE);
		if (!wait_trigger(0, 30)) {					
			return FINGERPRINT_RES_CANCEL;
		}
		ex_log(LOG_INFO, "wait finger level int");
		notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);

		if (check_cancelable()) {
			return FINGERPRINT_RES_SUCCESS;
		}
	}

	ex_log(LOG_INFO, "%s end ", __func__);

	return FINGERPRINT_RES_SUCCESS;
}

//OPLUS SET LOCKER MODE
int cpt_set_locker_mode(void)
{
	//lock mode flow
	//1.stop auth
	//2.start lock mode
	//3.loop finger on/off
	//4.cancel lock mode
	//5.start auth

	if (TRUE != g_hardware_ready) {
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

	thread_manager_run_task((do_operation_callback)touch_lock_mode, TASK_PROCESS);

	return FINGERPRINT_RES_SUCCESS;	
}

int cpt_get_screen_state(int * screen_state)
{	
	return fp_get_screen_state(g_hdev,screen_state);
}
static BOOL g_do_camera = FALSE;
static int do_camera()
{
	int retval;
	unsigned int status;
	unsigned int quality;
	unsigned int cstate = CSTATE_WAIT_INT;
	g_do_camera = TRUE;
#ifdef __OPLUS_K4__
	g_fingerprint_opt = FINGERPRINT_OPTION_OTHERS;
#endif
	do {
		switch (cstate) {
			case CSTATE_WAIT_INT: {
				ex_log(LOG_INFO, "cstate == CSTATE_WAIT_INT");

				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) goto EXIT;

				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "CSTATE_WAIT_INT set detect mode failed, retval = %d", retval);
					break;
				}

				if (!wait_trigger(0, INTERRUPT_TIMEOUT)) goto EXIT;
			}

			case CSTATE_GET_IMG: {
				ex_log(LOG_INFO, "cstate == CSTATE_GET_IMG");

				if (check_cancelable()) goto EXIT;
				retval = opm_get_image(OPTION_EMPTY, &quality);
				if (check_cancelable()) goto EXIT;

				ex_log(LOG_INFO, "opm_get_image retval = %d, quality = %d", retval, quality);

				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				if (FP_LIB_ENROLL_SUCCESS == quality || FP_LIB_ENROLL_FAIL_LOW_COVERAGE == quality) {
					ex_log(LOG_DEBUG, "=====EVENT CLICK=====");
					notify(EVENT_NAVIGATION, EVENT_FINGERON, 0, NULL, 0);
				}

				cstate = CSTATE_FINGER_OFF;
			} break;

			case CSTATE_FINGER_OFF: {
				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(FOD_MODE);
				if (check_cancelable()) goto EXIT;

				if (!wait_trigger(0, 30)) {					
					goto EXIT;
				}

				if (check_cancelable()) goto EXIT;
				opm_check_finger_lost(30, &status);
				if (check_cancelable()) goto EXIT;
				ex_log(LOG_DEBUG, "[EGIS_CAMERA] opm_check_finger_lost status:%d", status);

				if (status == FP_LIB_FINGER_LOST) {
					notify(EVENT_NAVIGATION, EVENT_FINGEROFF, 0, NULL, 0);
					cstate = CSTATE_WAIT_INT;
				}
			} break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS);

EXIT:
	setSpiState(FALSE, FALSE);
	g_do_camera = FALSE;
	return FINGERPRINT_RES_SUCCESS;	
}
int cpt_keymode_enable(unsigned int enable)
{
	if (TRUE != g_hardware_ready) {
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}
    if (g_do_camera)
	    cpt_cancel();

	if (enable) {
		thread_manager_run_task((do_operation_callback)do_camera, TASK_PROCESS);
	}
	return FINGERPRINT_RES_SUCCESS;
}