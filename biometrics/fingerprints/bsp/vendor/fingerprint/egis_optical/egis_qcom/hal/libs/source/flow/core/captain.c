#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "captain.h"
#include "device_int.h"
#include "constant_def.h"
#include "response_def.h"
#include "ex_define.h"
#include "type_def.h"
#include "extra_def.h"
#include "op_manager.h"
#include "plat_log.h"
#include "plat_time.h"
#include "plat_file.h"
#include "plat_mem.h"
#include "plat_thread.h"
#include "object_def_image.h"
#include "fps_normal.h"
#include "opt_file.h"
#include "egis_rbs_api.h"
#include "thread_manager.h"
#include "fp_definition.h"
#include "common_definition.h"
#include "transporter_test.h"
#include "op_sensortest.h"
#include "core_config.h"
#include "op_manager.h"
#include "ini_definition.h"
#ifdef __SDK_SAVE_IMAGE__
#include "save_image.h"
#endif
#ifdef __SDK_SAVE_IMAGE_V2__
#include "save_image_v2.h"

extern char g_username[256];
extern char g_fingerid[256];
extern char g_case[256];
extern char g_test_case_id[256];
#endif
#define LOG_TAG "RBS-CAPTAIN"

#define AUTH_TOKEN_LEN 256

#define FPS_CALI_ET7XX_BKG 7
#define FPS_SAVE_ET7XX_CALI_DATA 8
#define FPS_LOAD_ET7XX_CALI_DATA 9
#define FPS_REMOVE_ET7XX_CALI_DATA 10
#define FPS_CALI_UNINIT 11
#define FPS_REMOVE_ET7XX_BDS 12

#ifdef __ET7XX__
#define MAX_IMAGE_BUFFER_SIZE (224 * 224 * 2)
#else
#define MAX_IMAGE_BUFFER_SIZE (120 * 120 * 2)
#endif
#define MULTI_MAX_IMAGE_BUFFER_SIZE (MAX_IMAGE_BUFFER_SIZE * EGIS_TRANSFER_FRAMES_PER_TIME)

cache_info_t g_cache_info;
fingerprint_enroll_info_t g_enroll_info;
fingerprint_verify_info_t g_verify_info;
static enroll_config_t g_enroll_config;
static navigation_config_t g_navi_config;
static cut_img_config_t g_crop_config;

static unsigned int g_has_enroll_count = 0;
static int g_enroll_percentage_tmp = 0;

static BOOL g_need_cancel = FALSE;
static BOOL g_need_pause = FALSE;
static BOOL g_hardware_ready = FALSE;
static BOOL g_screen_state = FALSE;
extern BOOL g_navi_need_cancel;

int g_hdev = 0;
int g_temp_remaining = 0;
char g_user_path[MAX_PATH_LEN] = {0};
int g_enroll_timeout = TIMEOUT_WAIT_FOREVER;

event_callbck_t g_event_callback;

typedef enum verify_state {
	VSTATE_DETECT_MODE,
	VSTATE_WAIT_INT,
	VSTATE_GET_IMG,
	VSTATE_VERIFY,
	VSTATE_FINGER_OFF
} verify_state_t;

typedef enum enroll_state {
	ESTATE_DETECT_MODE,
	ESTATE_WAIT_INT,
	ESTATE_GET_IMG,
	ESTATE_CHECK_DUPLICATE,
	ESTATE_ENROLL,
	ESTATE_FINGER_OFF,
	ESTATE_CHECK_FINGER_OFF_ONCE,
	ESTATE_CAPTURE_DELAY,
	ESTATE_PAUSE,
} enroll_state_t;

typedef enum navi_state {
	NSTATE_WAIT_INT,
	NSTATE_NAVI,
	NSTATE_FINGER_OFF,
} navi_state_t;

typedef struct {
	int  info_version;
	int  is_new_finger_on;
	int  try_count;
	int  match_score;
	int  save_index;
} return_image_info_t;

typedef enum pwr_status {
    SENSOR_PWR_OFF = 1,
    SENSOR_PWR_ON = 2,
} pwr_status_t;

typedef struct {
    pwr_status_t pwr_status;
    BOOL is_fast_finger_on;
} power_management_t;

static power_management_t g_power_managemen = {
    .pwr_status = SENSOR_PWR_OFF,
    .is_fast_finger_on = false,
};

static mutex_handle_t pwr_lock;

static host_device_info_t g_host_device = { .temperature_x10 = 250, };
void set_host_device_info(host_device_info_t device) { g_host_device = device; }
host_device_info_t get_host_device_info(void) { return g_host_device; }

#if defined (__SHOW_LIVEIMAGE__) || defined (__SUPPORT_SAVE_IMAGE__)
static int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good, int match_result);
#endif
static int do_sensortest(int cid, unsigned char *in_data, int in_data_size,
			 unsigned char *out_buffer, int *out_buffer_size);
void captain_cancel(BOOL cancel_flag);
static void get_ca_version(unsigned char *version, int *len);
static int enroll_percentage_to_remaining(int percentage);
#if defined(__ET7XX__) && !defined(__ET0XX__)
int get_sys_temp(int *temperature);
int set_brightness(int level);
long getCurrentTime_ms(void);
void get_time_now(char* str_time);
long get_ca_time_ms(void);
static int do_power_on(pwr_status_t pwr);

#endif
static void notify(int event_id, int first_param, int second_param,
		   unsigned char *data, int data_size)
{
	if (NULL != g_event_callback) {
#ifndef EGIS_DBG
		if (event_id == EVENT_RETURN_IMAGE || event_id ==  EVENT_RETURN_LIVE_IMAGE)
			return;
#endif
		g_event_callback(event_id, first_param, second_param, data, data_size);
		if (event_id == EVENT_ENROLL_OK) {
			int remaining = enroll_percentage_to_remaining(second_param);
#ifdef __OPLUS__
		/* Report remaining after templates.*/
			if (remaining == 0) {
				egislog_d("skip report remaining = 0");
				return;
			}
#endif
			if(g_temp_remaining != remaining)
				g_temp_remaining = remaining;
			else
				return;
			g_event_callback(EVENT_ENROLL_REMAINING, first_param, remaining, data, data_size);
		}
#ifdef __OPLUS__
		/* Report remaining after templates.*/
		else if (event_id == EVENT_CUSTOM_ENROLL_OK_FINAL) {
			g_event_callback(EVENT_ENROLL_REMAINING, first_param, 0, data, data_size);
		}
#endif
	}
#ifdef __ENABLE_NAVIGATION__
	if (event_id == EVENT_NAVIGATION) send_navi_event_to_driver(first_param);
#endif
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

	g_enroll_config.capture_delay_enroll_enable = core_config_get_int(INI_SECTION_ENROLL, KEY_CAPTURE_DELAY_ENABLE, 1);
	g_enroll_config.capture_delay_enroll_start_progress =
		    core_config_get_int(INI_SECTION_ENROLL, KEY_CAPTURE_DELAY_START_PROGRESS, 80);  // last 5 of total 24, ~= 20%

	g_enroll_config.enroll_extra_1st_enable = core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLL_EXTRA_THE_FIRST_ENABLE,
		INID_ENROLL_EXTRA_THE_FIRST_ENABLE);
	g_enroll_config.enroll_extra_1st_before_progress = core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLL_EXTRA_THE_FIRST_BEFORE_PROGRESS, 90);
	g_enroll_config.enroll_max_count = core_config_get_int(INI_SECTION_ENROLL, KEY_MAX_ENROLL_COUNT, INID_MAX_ENROLL_COUNT);

	g_navi_config.navi_mode = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_MODE, 1);
	g_navi_config.change_x_y = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_CHANGE_X_Y, 0);
	g_navi_config.change_up_down = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_CHANGE_UP_DOWN, 0);
	g_navi_config.change_left_right = core_config_get_int(INI_SECTION_NAVI, KEY_NAVI_CHANGE_LEFT_RIGHT, 0);

	cut_img_config_t crop_config = { 0 };
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

#ifdef _INLINE_MERGE_MAP
#include "MergeMapCreator.h"
static void map_callback(unsigned int result, unsigned int percentage,
			 unsigned char *map, unsigned int width,
			 unsigned int height, int sum_x, int sum_y)
{
	if (NULL != g_event_callback) {
		switch (result) {
			case ECALLBACK_ENROLL_MAP:
				g_event_callback(EVENT_ENROLL_MAP, sum_x, sum_y, map, width * height);
				ex_log(LOG_DEBUG, "ECALLBACK_ENROLL_MAP sum_y = %d. percentage=%d", sum_y, percentage);
				break;

			case ECALLBACK_ENROLL_TOO_FAST:
				notify(EVENT_ENROLL_TOO_FAST, 0, 0, NULL, 0);
				break;

			default:
				ex_log(LOG_ERROR, "map_callback error, default case");
				break;
		}
	}
}
#endif

static void update_map(BOOL finger_on_again, BOOL fast, int percentage, algo_swipe_info_t swipe_info, BOOL finish)
{
#ifdef _INLINE_MERGE_MAP
	static BOOL started = FALSE;
	struct MergeMapUpdateInfo map_info;
	struct MergeMapConfig config;
	int map_size_data[4] = {0};
	int swipe_direction;
	ex_log(LOG_DEBUG, "update_map enter");
	if (!started) {
		config.MergeMapWidth = MERGE_MAP_WIDTH;
		config.MergeMapHeight = MERGE_MAP_HEIGHT;
		config.BackgroundHeight = MERGE_MAP_HEIGHT;
		ex_log(LOG_DEBUG, "swipe_dir = %d", swipe_info.swipe_dir);
		if (swipe_info.swipe_dir == 1) {
			swipe_direction = SWIPE_DIRECTION_X;
		} else if (swipe_info.swipe_dir == 2) {
			swipe_direction = SWIPE_DIRECTION_Y;
		} else {
			swipe_direction = SWIPE_DIRECTION_AUTO;
		}

		config.EnrollMethod = g_enroll_config.enroll_method;
		config.LoopCountX = g_enroll_config.swipe_count_x;
		config.LoopCountY = g_enroll_config.swipe_count_y;

		if (config.EnrollMethod == ENROLL_METHOD_PAINT) {
			config.BackgroundHeight = BACKGROUND_HEIGHT_PAINT;
		} else if (config.EnrollMethod == ENROLL_METHOD_SWIPE) {
			config.BackgroundHeight = BACKGROUND_HEIGHT_SWIPE;
		}

		config.SwipeDirection = swipe_direction;
		config.MergeMap_HW_Width = swipe_info.mergemap_hw_width;
		config.MergeMap_HW_Height = swipe_info.mergemap_hw_height;

		if ((swipe_direction != SWIPE_DIRECTION_AUTO) || (config.EnrollMethod == ENROLL_METHOD_PAINT)) {
			map_size_data[0] = config.MergeMapWidth;
			map_size_data[1] = config.BackgroundHeight;

			if (config.EnrollMethod == ENROLL_METHOD_SWIPE) {
				map_size_data[2] = swipe_direction == SWIPE_DIRECTION_X ? config.LoopCountX : config.LoopCountY;
				map_size_data[3] = swipe_direction == SWIPE_DIRECTION_X ? config.MergeMap_HW_Width : config.MergeMap_HW_Height;
			} else if (config.EnrollMethod == ENROLL_METHOD_PAINT) {
				map_size_data[2] = config.MergeMapWidth / 3;
				map_size_data[3] = config.MergeMapWidth / 3;
			}

			notify(EVENT_MERGE_MAP_SIZE, 4, sizeof(int), (unsigned char *)map_size_data, sizeof(int) * 4);

			mmap_set_config(&config);
			mmap_start(map_callback);
			finger_on_again = TRUE;
			started = TRUE;
			ex_log(LOG_DEBUG, "update_map start");
		}
	}

	if (finish) {
		mmap_end(percentage == 100 ? 0 : 1);
		started = FALSE;
	}

	if (started) {
		ex_log(LOG_DEBUG, "update_map update mergemap");
		map_info.finger_on_again = finger_on_again;
		map_info.rollback = g_enroll_config.enroll_too_fast_rollback ? fast : FALSE;
		map_info.percentage = percentage;
		map_info.similarity_score = swipe_info.similarity_score;
		map_info.delta_x = swipe_info.dx;
		map_info.delta_y = swipe_info.dy;

		mmap_update(&map_info);
	}

#else
	notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, percentage, NULL, 0);
#endif
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
		g_cache_info.authenticator_id = (unsigned long long)-1;
	}

	ex_log(LOG_DEBUG, "opm_get_fingerprint_ids return = %d", retval);
	return FINGERPRINT_RES_SUCCESS;
}

BOOL check_cancelable() {
	if (g_need_cancel) host_touch_set_hbm_system(FALSE);
	return g_need_cancel;
}
BOOL check_need_pause(void) {
	if (g_need_pause) host_touch_set_hbm_system(FALSE);
	return g_need_pause;
}

static void power_off(void)
{
	int ret = 0;
	ret = opm_set_work_mode(POWEROFF_MODE);
	if (ret != 0) {
		ex_log(LOG_ERROR, "power_off Fail = %d", ret);
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

const int TIMEOUT_FOR_ONE_TRY = 500;
extern oplus_trigger_case_t g_trigger_case;
static int touch_enroll(unsigned int *percentage, unsigned int enrolled_count)
{
	int retval, ret2 = 0;
	int enroll_option = ENROLL_OPTION_NORMAL;
	unsigned int status = FP_LIB_FINGER_LOST;
	unsigned int estate = ESTATE_WAIT_INT;
	unsigned int img_quality;
	cmd_enrollresult_t enroll_result;
	memset(&enroll_result, 0, sizeof(cmd_enrollresult_t));
	g_need_pause = FALSE;

	host_touch_set_finger_on(FALSE);
	host_touch_set_finger_off(FALSE);

	do {
		switch (estate) {
			case ESTATE_WAIT_INT:
				ex_log(LOG_DEBUG, "estate == ESTATE_WAIT_INT");
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "ESTATE_WAIT_INT set detect mode failed return = %d", retval);
				}
				notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);

				g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;
				ret2 = wait_trigger(0, TIMEOUT_FOR_ONE_TRY, g_enroll_timeout);
				if (ret2 == FINGERPRINT_RES_SECURE_ENROLL_TIMEOUT)
				{
					notify(EVENT_ENROLL_TIMEOUT, 0, 0, NULL, 0);
					ret2 = 0;
				}
				if (!ret2) {
					ex_log(LOG_DEBUG, "wait_trigger cancel, %d", ret2);
					return FINGERPRINT_RES_CANCEL;
				}
				host_touch_set_hbm_evalute_always();

				if ((enroll_result.percentage > (uint32_t)g_enroll_config.capture_delay_enroll_start_progress) && g_enroll_config.capture_delay_enroll_enable) {
					ex_log(LOG_INFO, "percentage = %d , enroll still triggered at %d",
					       enroll_result.percentage, g_enroll_config.capture_delay_enroll_enable);
					estate = ESTATE_CAPTURE_DELAY;
					break;
				}

				if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "power on sensor fail");
					retval = FINGERPRINT_RES_FAILED;
					goto EXIT;
				}
				estate = ESTATE_CAPTURE_DELAY;
				break;

			case ESTATE_GET_IMG:
				ex_log(LOG_DEBUG, "estate == ESTATE_GET_IMG");

				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

				int sys_temp;
				get_sys_temp(&sys_temp);
				host_device_info_t info;
				info.temperature_x10 = sys_temp;
				set_host_device_info(info);
				retval = opm_get_image(&img_quality, get_host_device_info());
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
#ifdef __SHOW_LIVEIMAGE__
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, img_quality, FALSE, 0);
#endif

				ex_log(LOG_DEBUG, "opm_get_image retval = %d", retval);
				ex_log(LOG_DEBUG, "opm_get_image quality = %d", img_quality);
				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				if (host_touch_is_finger_off()) {
					estate = ESTATE_FINGER_OFF;
					notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
					break;  // don't output image

				} else if (img_quality == FP_LIB_ENROLL_SUCCESS) {
					estate = ESTATE_ENROLL;
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);

				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY ||
					   img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE) {
					estate = ESTATE_FINGER_OFF;
					notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);
				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE || host_touch_is_low_coverage()) {
					estate = ESTATE_FINGER_OFF;
					host_touch_set_low_coverage(FALSE);
					notify(EVENT_IMG_PARTIAL, 0, 0, NULL, 0);
				} else if (img_quality == FP_LIB_ENROLL_HELP_TOO_WET) {
					estate = ESTATE_FINGER_OFF;
					notify(EVENT_IMG_WATER, 0, 0, NULL, 0);
				} else
					estate = ESTATE_WAIT_INT;

				break;

			case ESTATE_ENROLL:
				ex_log(LOG_DEBUG, "estate == ESTATE_ENROLL");

				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				retval = opm_do_enroll(&enroll_result, enroll_option, enrolled_count);
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_identify_enroll failed, return %d", retval);
					break;
				}

				ex_log(LOG_DEBUG, "opm_do_enroll status %u, (%d,%d) score=%d", enroll_result.status,
				       enroll_result.swipe_info.dx, enroll_result.swipe_info.dy,
				       enroll_result.swipe_info.similarity_score);

				*percentage = enroll_result.percentage;

				estate = ESTATE_FINGER_OFF;
				if (enroll_result.status == FP_LIB_ENROLL_HELP_SAME_AREA) {
					notify(EVENT_ENROLL_HIGHLY_SIMILAR, 0, 0, NULL, 0);

				} else if (enroll_result.status == FP_LIB_ENROLL_SUCCESS) {
#ifdef __ET7XX__  // ENROLL_THE_FIRST
					if (enroll_result.percentage < 100) {
						ex_log(LOG_DEBUG, "enroll_extra_1st (%d) %d", g_enroll_config.enroll_extra_1st_enable,
							g_enroll_config.enroll_extra_1st_before_progress);
						if (g_enroll_config.enroll_extra_1st_enable) {
#ifdef RBS_EVTOOL
							// Skip the first reject-retry image of DQE flow
							enroll_option = ENROLL_OPTION_REJECT_RETRY;
#endif
							if (enroll_result.percentage >= (uint32_t) g_enroll_config.enroll_extra_1st_before_progress) {
								opm_do_enroll(&enroll_result, ENROLL_OPTION_STOP_DQE, 0);
							} else if (enroll_option == ENROLL_OPTION_REJECT_RETRY) {
								opm_do_enroll(&enroll_result, ENROLL_OPTION_ENROLL_THE_FIRST, 0);
								notify(EVENT_ENROLL_LQ, g_enroll_info.fingerprint_info.fingerprint_id,
								enroll_result.percentage, NULL, 0);
							}
						}
					}
#endif
					if (enroll_option != ENROLL_OPTION_STOP_DQE) {
						notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id,
							enroll_result.percentage, NULL, 0);
					}
				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_QUALITY) {
					notify(EVENT_IMG_BAD_QLTY, g_enroll_info.fingerprint_info.fingerprint_id,
					       enroll_result.percentage, NULL, 0);
#ifdef __ET7XX__
				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_QUALITY_DYNAMIC_REJECT_LEVEL) {
					// notify(EVENT_IMG_BAD_QLTY, g_enroll_info.fingerprint_info.fingerprint_id,
					//    enroll_result.percentage, NULL, 0);
					estate = ESTATE_GET_IMG;
					enroll_option = ENROLL_OPTION_REJECT_RETRY;
					break;
#endif
				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
					notify(EVENT_IMG_PARTIAL, g_enroll_info.fingerprint_info.fingerprint_id,
					       enroll_result.percentage, NULL, 0);

				} else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
					//Todo
				} else if (enroll_result.status == FP_LIB_ENROLL_HELP_SCRATCH_DETECTED) {
					ex_log(LOG_DEBUG, "SCRATCH_DETECTED");
					notify(EVENT_ENROLL_SCRATCH_DETECTED, 0, 0, NULL, 0);
				} else if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
					notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);

				} else {
					ex_log(LOG_ERROR, "FINGERPRINT_RES_ALGORITHM_ERROR");
					retval = FINGERPRINT_RES_ALGORITHM_ERROR;
				}

#ifdef __SUPPORT_SAVE_IMAGE__
				transfer_frames_to_client(TRANSFER_ENROLL_IMAGE, img_quality, FALSE, 0);
#endif
#ifdef __ET7XX__ // KEEP_ENROLL_RETRY_IMAGES
				opm_do_enroll(&enroll_result, ENROLL_OPTION_CLEAR_IMAGES, 0);
#endif
				break;

			case ESTATE_CAPTURE_DELAY:
				ex_log(LOG_DEBUG, "estate == ESTATE_CAPTURE_DELAY");
				event_check_fd check[2];

				/*ESTATE_CAPTURE_DELAY will delay for awhile.*/
				setSpiState(FALSE, FALSE);

				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				check[0].checkerFunc = &check_cancelable;
				check[1].checkerFunc = &host_touch_is_finger_off;
				event_poll_wait(check, 2,
						core_config_get_int(INI_SECTION_ENROLL, KEY_CAPTURE_DELAY_WAITING_TIME,
								    400));
				if (check[0].revent) return FINGERPRINT_RES_CANCEL;

				if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "power on sensor fail");
					retval = FINGERPRINT_RES_FAILED;
					goto EXIT;
				}
				estate = ESTATE_GET_IMG;

				if (check[1].revent) {
					estate = ESTATE_WAIT_INT;
					notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
					break;
				}

				break;

			case ESTATE_PAUSE:
				ex_log(LOG_INFO, "estate == ESTATE_PAUSE");
				/*
				if (check_need_pause() && !is_power_off_mode) {
					opm_set_work_mode(POWEROFF_MODE);
					is_power_off_mode = TRUE;
				}
				*/

				if (!check_need_pause()) {
					estate = ESTATE_WAIT_INT;
				}

				if (check_cancelable()) {
					g_need_pause = FALSE;
					return FINGERPRINT_RES_CANCEL;
				}

				plat_sleep_time(20);
				break;

			case ESTATE_FINGER_OFF:
				ex_log(LOG_DEBUG, "estate == ESTATE_FINGER_OFF");
				enroll_option = ENROLL_OPTION_NORMAL;
				/*ESTATE_FINGER_OFF will be excuted many times until the finger leaving
				calling setSpiState(on) to keep the spi open for a while to avoid continual opening or closing the spi */
#ifdef __ET7XX__
                                if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                                        ex_log(LOG_ERROR, "power off sensor fail");
                                        retval = FINGERPRINT_RES_FAILED;
                                        goto EXIT;
                                }
#else
                                setSpiState(TRUE, FALSE);
#endif

				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
				if (host_touch_is_enable()) {
					/** OPLUS Flow **/
					if (host_touch_is_using_oplus_flow())
					{
						g_trigger_case = TRIGGER_WAIT_TOUCH_UP;
						if (!wait_trigger(0, TIMEOUT_FOR_ONE_TRY, g_enroll_timeout)) {

							notify(EVENT_ENROLL_TIMEOUT, 0, 0, NULL, 0);
							return FINGERPRINT_RES_CANCEL;
						}
					}
					/** OPLUS Flow End **/
					status = FP_LIB_FINGER_LOST;
				} else {
					opm_check_finger_lost(30, &status);
				}

				if (check_need_pause()) { estate = ESTATE_PAUSE; break; }
				if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

				// When finger is not lost, CORE change to detect mode already
				// opm_set_work_mode(DETECT_MODE);

				if (status == FP_LIB_FINGER_LOST) {
					estate = ESTATE_WAIT_INT;
					notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
					setSpiState(FALSE, FALSE);
				}
				break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS && enroll_result.percentage < 100);

EXIT:
#ifdef __ET7XX__
    	if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
        	ex_log(LOG_ERROR, "power off sensor fail");
        	retval = FINGERPRINT_RES_FAILED;
    	}
#else
    	setSpiState(FALSE, FALSE);
#endif	
	return retval;
}

static int paint_enroll(unsigned int *percentage, unsigned int enrolled_count)
{
	int retval;
	unsigned int estate = ESTATE_WAIT_INT;
	unsigned int img_quality = 0;
	cmd_enrollresult_t enroll_result;
	BOOL is_too_fast = FALSE;
	BOOL finger_on_again = TRUE;
	int SWIPE_FAST_THREASHOLD = 500;
	int SWIPE_FAST_CONTROL = FALSE;
	int enroll_option = ENROLL_OPTION_NORMAL;
	memset(&enroll_result, 0, sizeof(cmd_enrollresult_t));

	SWIPE_FAST_THREASHOLD = core_config_get_int(INI_SECTION_ENROLL, KEY_SWIPE_QUICK_TIMEOUT, 500);
	SWIPE_FAST_CONTROL = core_config_get_int(INI_SECTION_ENROLL, KEY_CHECK_TOO_FAST, TRUE);

	do {
		switch (estate) {
			case ESTATE_WAIT_INT:
				ex_log(LOG_DEBUG, "estate == ESTATE_WAIT_INT");

				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "ESTATE_WAIT_INT set detect mode failed return = %d", retval);
				}

				notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);

				setSpiState(FALSE, FALSE);
				if (!wait_trigger(0, TIMEOUT_FOR_ONE_TRY, g_enroll_timeout)) {
					notify(EVENT_ENROLL_TIMEOUT, 0, 0, NULL, 0);
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				setSpiState(TRUE, TRUE);

				enroll_option = ENROLL_OPTION_FINGERON;

			case ESTATE_GET_IMG:
				ex_log(LOG_DEBUG, "estate == ESTATE_GET_IMG");

				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}

				retval = opm_get_image(&img_quality, get_host_device_info());
#ifdef __SHOW_LIVEIMAGE__
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, img_quality, FALSE, 0);
#endif
				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				ex_log(LOG_DEBUG, "opm_get_image retval = %d", retval);
				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				ex_log(LOG_DEBUG, "opm_get_image img_quality = %d", img_quality);
				if (img_quality == FP_LIB_ENROLL_SUCCESS) {
					estate = ESTATE_ENROLL;
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);

				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY ||
					   img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE) {
					estate = ESTATE_FINGER_OFF;

				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
					estate = ESTATE_ENROLL;

				} else if (img_quality == FP_LIB_ENROLL_HELP_TOO_WET) {
					estate = ESTATE_FINGER_OFF;
					notify(EVENT_IMG_WATER, 0, 0, NULL, 0);
				} else
					estate = ESTATE_WAIT_INT;

				break;

			case ESTATE_ENROLL:
				ex_log(LOG_DEBUG, "estate == ESTATE_ENROLL");

				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				retval = opm_do_enroll(&enroll_result, enroll_option, enrolled_count);
				notify(EVENT_SWIPE_DX_DY, enroll_result.swipe_info.dx,
				       enroll_result.swipe_info.dy, NULL, 0);
				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				ex_log(LOG_DEBUG, "opm_do_enroll retval = %d", retval);
				if (retval != FINGERPRINT_RES_SUCCESS) {
					break;
				}

				notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, enroll_result.percentage, NULL, 0);

				enroll_option = ENROLL_OPTION_NORMAL;
				*percentage = enroll_result.percentage;
				if (enroll_result.percentage >= 100) {
					enroll_option = ENROLL_OPTION_MERGE;
					opm_do_enroll(&enroll_result, enroll_option, 0);
					if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
						notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);
						estate = ESTATE_FINGER_OFF;
						break;
					} else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
						enroll_option = ENROLL_OPTION_MERGE;
						opm_do_enroll(&enroll_result, enroll_option, 0);

						notify(EVENT_ENROLL_TOO_FAST, 0, 0, NULL, 0);
						estate = ESTATE_FINGER_OFF;
						break;
					}

					retval = FINGERPRINT_RES_SUCCESS;
					goto exit;
				}

				ex_log(LOG_DEBUG, "percentage = %d, status = %d, swipe_dir = %d",
				       enroll_result.percentage, enroll_result.status, enroll_result.swipe_info.swipe_dir);

				ex_log(LOG_DEBUG, "opm_do_enroll dx = %d, dy = %d, score = %d",
				       enroll_result.swipe_info.dx, enroll_result.swipe_info.dy, enroll_result.swipe_info.similarity_score);

				update_map(finger_on_again, FALSE, enroll_result.percentage, enroll_result.swipe_info, FALSE);
				finger_on_again = FALSE;

				if (enroll_result.status == FP_LIB_ENROLL_SUCCESS ||
				    enroll_result.status == FP_LIB_ENROLL_HELP_SAME_AREA) {
					estate = ESTATE_GET_IMG;

				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_QUALITY) {
					estate = ESTATE_FINGER_OFF;

				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
					estate = ESTATE_GET_IMG;

				} else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
					estate = ESTATE_GET_IMG;
					enroll_option = ENROLL_OPTION_MERGE;
					opm_do_enroll(&enroll_result, enroll_option, 0);

					ex_log(LOG_ERROR, "opm_identify_enroll status %u", enroll_result.status);
					if (SWIPE_FAST_CONTROL) {
						is_too_fast = TRUE;
						notify(EVENT_ENROLL_TOO_FAST, 0, 0, NULL, 0);
						estate = ESTATE_FINGER_OFF;
					}

				} else if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
					notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);
				} else {
					retval = FINGERPRINT_RES_ALGORITHM_ERROR;
				}

				break;

			case ESTATE_FINGER_OFF:
				ex_log(LOG_DEBUG, "estate == ESTATE_FINGER_OFF");

				opm_set_work_mode(DETECT_MODE);

				if (!wait_trigger(3, 30, TIMEOUT_WAIT_FOREVER)) {
					estate = ESTATE_WAIT_INT;

					if (is_too_fast) {
						update_map(finger_on_again, TRUE, enroll_result.percentage, enroll_result.swipe_info, FALSE);
						is_too_fast = FALSE;
					}

					if (!finger_on_again)
						update_map(TRUE, FALSE, enroll_result.percentage, enroll_result.swipe_info, FALSE);

					notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
				} else {
					estate = ESTATE_GET_IMG;
				}
				break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS);

exit:
	//flow exit, close spi
	setSpiState(FALSE, FALSE);
	update_map(FALSE, FALSE, enroll_result.percentage, enroll_result.swipe_info, TRUE);

#ifdef __SUPPORT_SAVE_IMAGE__
	transfer_frames_to_client(TRANSFER_ENROLL_IMAGE, img_quality, FALSE, 0);
#endif

	return retval;
}

static int swipe_enroll(unsigned int *percentage, unsigned int enrolled_count)
{
	int retval;
	unsigned int status = 0;
	unsigned int estate = ESTATE_WAIT_INT;
	unsigned int img_quality;
	cmd_enrollresult_t enroll_result;
	BOOL is_need_merge_enroll = FALSE;
	BOOL is_too_fast = FALSE;
	BOOL finger_on_again = TRUE;
	int SWIPE_FAST_THREASHOLD = 500;
	int SWIPE_FAST_CONTROL = FALSE;
	int enroll_option = ENROLL_OPTION_NORMAL;

	memset(&enroll_result, 0, sizeof(cmd_enrollresult_t));

	SWIPE_FAST_THREASHOLD = core_config_get_int(INI_SECTION_ENROLL, KEY_SWIPE_QUICK_TIMEOUT, 500);
	SWIPE_FAST_CONTROL = core_config_get_int(INI_SECTION_ENROLL, KEY_CHECK_TOO_FAST, TRUE);

	do {
		switch (estate) {
			case ESTATE_WAIT_INT:
				ex_log(LOG_DEBUG, "estate == ESTATE_WAIT_INT");

				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}

				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "ESTATE_WAIT_INT set detect mode failed return = %d", retval);
				}
				notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);

				setSpiState(FALSE, FALSE);
				if (!wait_trigger(0, TIMEOUT_FOR_ONE_TRY, g_enroll_timeout)) {
					notify(EVENT_ENROLL_TIMEOUT, 0, 0, NULL, 0);
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				setSpiState(TRUE, TRUE);

				enroll_option = ENROLL_OPTION_FINGERON;

			case ESTATE_GET_IMG:
				ex_log(LOG_DEBUG, "estate == ESTATE_GET_IMG");

				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}

				retval = opm_get_image(&img_quality, get_host_device_info());
#ifdef __SHOW_LIVEIMAGE__
				transfer_frames_to_client(TRANSFER_LIVE_IMAGE, img_quality, FALSE, 0);
#endif
				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}

				ex_log(LOG_DEBUG, "opm_get_image retval = %d", retval);
				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				ex_log(LOG_DEBUG, "opm_get_image img_quality = %d", img_quality);

				if (img_quality == FP_LIB_ENROLL_SUCCESS) {
					estate = ESTATE_ENROLL;
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY ||
					   img_quality == FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE) {
					estate = ESTATE_CHECK_FINGER_OFF_ONCE;

				} else if (img_quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
					estate = ESTATE_ENROLL;

				} else if (img_quality == FP_LIB_ENROLL_HELP_TOO_WET) {
					estate = ESTATE_FINGER_OFF;
				} else
					estate = ESTATE_WAIT_INT;

				break;

			case ESTATE_ENROLL:
				ex_log(LOG_DEBUG, "estate == ESTATE_ENROLL");

				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}
				retval = opm_do_enroll(&enroll_result, enroll_option, enrolled_count);
				notify(EVENT_SWIPE_DX_DY, enroll_result.swipe_info.dx, enroll_result.swipe_info.dy, NULL, 0);
				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}

				ex_log(LOG_DEBUG, "opm_do_enroll retval = %d", retval);
				if (retval != FINGERPRINT_RES_SUCCESS) {
					break;
				}

				ex_log(LOG_DEBUG, "percentage = %d, status = %d, swipe_dir = %d,height %d",
				       enroll_result.percentage, enroll_result.status,
				       enroll_result.swipe_info.swipe_dir, enroll_result.swipe_info.mergemap_hw_height);

				ex_log(LOG_DEBUG, "opm_do_enroll dx = %d, dy = %d, score = %d",
				       enroll_result.swipe_info.dx, enroll_result.swipe_info.dy,
				       enroll_result.swipe_info.similarity_score);

				enroll_option = ENROLL_OPTION_NORMAL;
				is_need_merge_enroll = TRUE;

				if (enroll_result.swipe_info.dx != 0 || enroll_result.swipe_info.dy != 0) {
					update_map(finger_on_again, FALSE, enroll_result.percentage, enroll_result.swipe_info, FALSE);
					finger_on_again = FALSE;
				}

				*percentage = enroll_result.percentage;
				if (*percentage >= 100) {
					*percentage = 99;
				}

				notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, *percentage, NULL, 0);

#ifdef SIMULATE_NO_SENSOR
				if (enroll_result.percentage >= 100) {
					estate = ESTATE_FINGER_OFF;
					break;
				}
#endif

				if (enroll_result.status == FP_LIB_ENROLL_SUCCESS ||
				    enroll_result.status == FP_LIB_ENROLL_HELP_SAME_AREA) {
					estate = ESTATE_GET_IMG;

				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_QUALITY) {
					estate = ESTATE_CHECK_FINGER_OFF_ONCE;

				} else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
					estate = ESTATE_GET_IMG;

				} else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
					estate = ESTATE_FINGER_OFF;
					ex_log(LOG_ERROR, "opm_identify_enroll too fast");

					if (SWIPE_FAST_CONTROL) {
						notify(EVENT_ENROLL_TOO_FAST, 0, 0, NULL, 0);
						is_too_fast = TRUE;
					}

				} else if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
					notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);
				} else {
					retval = FINGERPRINT_RES_ALGORITHM_ERROR;
				}

				break;

			case ESTATE_FINGER_OFF:
				ex_log(LOG_DEBUG, "estate == ESTATE_FINGER_OFF");

				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}

				opm_check_finger_lost(30, &status);
				if (check_cancelable()) {
					retval = FINGERPRINT_RES_CANCEL;
					break;
				}

				if (status == FP_LIB_FINGER_LOST || !wait_trigger(3, 30, TIMEOUT_WAIT_FOREVER)) {
					estate = ESTATE_WAIT_INT;
					if (is_too_fast) {
						update_map(finger_on_again, TRUE, enroll_result.percentage, enroll_result.swipe_info, FALSE);
						is_too_fast = FALSE;
					}

					finger_on_again = TRUE;

					if (enroll_result.percentage >= 100) {
						notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, enroll_result.percentage, NULL, 0);
					}

					notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);

#ifdef __SUPPORT_SAVE_IMAGE__
					transfer_frames_to_client(TRANSFER_ENROLL_IMAGE, img_quality, FALSE, 0);
#endif
					if (is_need_merge_enroll) {
						enroll_option = ENROLL_OPTION_MERGE;
						opm_do_enroll(&enroll_result, enroll_option, 0);
						if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
							notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);
						} else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
							notify(EVENT_ENROLL_TOO_FAST, 0, 0, NULL, 0);
							update_map(finger_on_again, TRUE, enroll_result.percentage, enroll_result.swipe_info, FALSE);
							notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, enroll_result.percentage, NULL, 0);
						}

						is_need_merge_enroll = FALSE;
					}

					if (enroll_result.percentage >= 100) {
						*percentage = 100;
						retval = FINGERPRINT_RES_SUCCESS;
						goto exit;
					}
				}

				break;

			case ESTATE_CHECK_FINGER_OFF_ONCE: {
				//only if get bad image. check finger off once, should to get image & enroll if still finger on
				ex_log(LOG_DEBUG, "estate == ESTATE_CHECK_FINGER_OFF_ONCE");

				opm_set_work_mode(DETECT_MODE);

				if (!wait_trigger(3, 30, TIMEOUT_WAIT_FOREVER)) {
					estate = ESTATE_WAIT_INT;

					if (enroll_result.percentage >= 100) {
						notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, enroll_result.percentage, NULL, 0);
					}

					notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);

#ifdef __SUPPORT_SAVE_IMAGE__
					transfer_frames_to_client(TRANSFER_ENROLL_IMAGE, img_quality, FALSE, 0);
#endif
					if (is_need_merge_enroll) {
						enroll_option = ENROLL_OPTION_MERGE;
						opm_do_enroll(&enroll_result, enroll_option, 0);
						if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
							notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);
						} else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
							notify(EVENT_ENROLL_TOO_FAST, 0, 0, NULL, 0);
							update_map(finger_on_again, TRUE, enroll_result.percentage, enroll_result.swipe_info, FALSE);
							notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id, enroll_result.percentage, NULL, 0);
						}

						is_need_merge_enroll = FALSE;
					}

					if (!finger_on_again)
						update_map(TRUE, FALSE, enroll_result.percentage, enroll_result.swipe_info, FALSE);

					if (enroll_result.percentage >= 100) {
						*percentage = 100;
						retval = FINGERPRINT_RES_SUCCESS;
						goto exit;
					}
				} else {
					estate = ESTATE_GET_IMG;
				}
			} break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS);

exit:
	setSpiState(FALSE, FALSE);
	update_map(FALSE, FALSE, enroll_result.percentage, enroll_result.swipe_info, TRUE);

	return retval;
}

static int do_enroll()
{
	int retval;
	unsigned int percentage = 0, enrolled_count = 0;
	int fid_size;

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

	switch (g_enroll_config.enroll_method) {
		case ENROLL_METHOD_TOUCH:
			retval = touch_enroll(&percentage, enrolled_count);
			break;
		case ENROLL_METHOD_PAINT:
			retval = paint_enroll(&percentage, enrolled_count);
			break;
		case ENROLL_METHOD_SWIPE:
			retval = swipe_enroll(&percentage, enrolled_count);
			break;
	}

	ex_log(LOG_DEBUG, "percentage = %d,enroll mode 0x%x", percentage, g_enroll_config.enroll_method);
	ex_log(LOG_DEBUG, "ENROLL loop end!!!!!! retval = %d", retval);

finish:
	opm_enroll_uninitialize();

	if (percentage >= 100) {
		retval = opm_save_enrolled_fingerprint(g_enroll_info);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			ex_log(LOG_ERROR, "saved fingerprint failed");
		} else {
			fid_size = sizeof(g_enroll_info.fingerprint_info.fingerprint_id);
			opt_receive_data(TYPE_RECEIVE_TEMPLATE,
					 (unsigned char *)&g_enroll_info.fingerprint_info.fingerprint_id,
					 fid_size, NULL, 0);
			g_cache_info.user_id = -1;
			sync_user_cache(g_enroll_info.fingerprint_info.user_id);
#ifdef __OPLUS__
			notify(EVENT_CUSTOM_ENROLL_OK_FINAL, g_enroll_info.fingerprint_info.fingerprint_id, 0, NULL, 0);
#endif
			notify(EVENT_ENROLL_SUCCESS, 0, 0, NULL, 0);
		}
	}

	if (retval == FINGERPRINT_RES_CANCEL) {
		notify(EVENT_ENROLL_CANCELED, 0, 0, NULL, 0);
	} else if (retval != FINGERPRINT_RES_SUCCESS) {
		notify(EVENT_ERR_ENROLL, 0, 0, NULL, 0);
	}
#ifdef __ET7XX__
	retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_SAVE_ET7XX_CALI_DATA, 0, 0, NULL, NULL);
	if (retval != FP_LIB_OK) ex_log(LOG_ERROR, "%s, Save BDS Pool Fail", __func__);
#endif
	return FINGERPRINT_RES_SUCCESS;
}

static int do_verify()
{
	const uint32_t LIVENESS_AUTHENTICATION = 0;
	int retval;
	int fid_size;
	unsigned int status;
	unsigned int match_id;
	BOOL has_result = FALSE;
	unsigned int quality;
	unsigned int vstate = VSTATE_WAIT_INT;
	unsigned char auth_token[AUTH_TOKEN_LEN];
	unsigned int auth_token_len = AUTH_TOKEN_LEN;
	int flow_trymatch_count = core_config_get_int(INI_SECTION_VERIFY, KEY_FLOW_TRY_MATCH, INID_FLOW_TRY_MATCH);
	int flow_try_match = flow_trymatch_count;
	int ext_feat_quality_trymatch_th = core_config_get_int(INI_SECTION_VERIFY,
			KEY_EXT_FEAT_QUALITY_TRYMATCH_THRESHOLD,
			INID_EXT_FEAT_QUALITY_TRYMATCH_THRESHOLD);
	int ext_feat_quality_badimg_th = core_config_get_int(INI_SECTION_VERIFY, KEY_EXT_FEAT_QUALITY_BADIMG_THRESHOLD, 10);
	unsigned int enrolled_count = 0;
	unsigned char is_tmpl_updated = 0;
	int ext_feat_quality = 0;

	ex_log(LOG_DEBUG, "%s, flow_try_match=%d", __func__, flow_try_match);

	if (!host_touch_is_using_oplus_flow())
	{
		host_touch_set_finger_on(FALSE);
		host_touch_set_finger_off(FALSE);
	}

	retval = opm_identify_start(LIVENESS_AUTHENTICATION);
	if (retval != FINGERPRINT_RES_SUCCESS) return retval;

	do {
		switch (vstate) {
			case VSTATE_WAIT_INT: {
				ex_log(LOG_DEBUG, "vstate == VSTATE_WAIT_INT");
				flow_try_match = flow_trymatch_count;
				if (check_cancelable()) goto EXIT;
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) goto EXIT;
				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "VSTATE_WAIT_INT set detect mode failed, retval = %d", retval);
					break;
				}
				notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);
				if (host_touch_is_using_oplus_flow())
				{
					g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;
				}

				if (!wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) goto EXIT;
				host_touch_set_hbm_evalute_always();
				if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "power on sensor fail");
					retval = FINGERPRINT_RES_FAILED;
				}
				vstate = VSTATE_GET_IMG;
				break;
			}

			case VSTATE_GET_IMG:
				ex_log(LOG_DEBUG, "vstate == VSTATE_GET_IMG");
				TIME_MEASURE_START(total_verify);

				if (check_cancelable()) goto EXIT;

				int sys_temp;
				get_sys_temp(&sys_temp);
				host_device_info_t info;
				info.temperature_x10 = sys_temp;
				set_host_device_info(info);

				retval = opm_get_image(&quality, get_host_device_info());
				if (check_cancelable()) goto EXIT;

				ex_log(LOG_DEBUG, "opm_get_image retval = %d, quality = %d", retval, quality);

				if (retval != FINGERPRINT_RES_SUCCESS) {
					ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
					break;
				}

				if (host_touch_is_finger_off()) {
					vstate = VSTATE_WAIT_INT;
					notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
					opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);
					break;  // don't output image

				} else if (FP_LIB_ENROLL_SUCCESS == quality) {
					vstate = VSTATE_VERIFY;
					notify(EVENT_FINGER_READY, 0, 0, NULL, 0);

				} else if (
				    FP_LIB_ENROLL_FAIL_LOW_QUALITY == quality ||
				    FP_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE == quality) {
					vstate = VSTATE_FINGER_OFF;
					notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);

				} else if (FP_LIB_ENROLL_FAIL_LOW_COVERAGE == quality) {
					vstate = VSTATE_VERIFY;
					break;
				} else if (FP_LIB_ENROLL_HELP_TOO_WET == quality) {
					vstate = VSTATE_FINGER_OFF;
					notify(EVENT_IMG_WATER, 0, 0, NULL, 0);

				} else if (FP_LIB_ENROLL_FAIL_SPOOF_FINGER == quality) {
					ex_log(LOG_INFO, "verify, finger is fake.");
					vstate = VSTATE_FINGER_OFF;
					notify(EVENT_IMG_FAKE_FINGER, 0, 0, NULL, 0);

				} else {
					vstate = VSTATE_WAIT_INT;
					break;
				}
#ifdef __SUPPORT_SAVE_IMAGE__
				if (vstate == VSTATE_FINGER_OFF) {
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, FALSE, 0);
				}
#endif
			break;

			case VSTATE_VERIFY:
				ex_log(LOG_DEBUG, "vstate == VSTATE_VERIFY");

				retval = opm_get_enrolled_count(&enrolled_count);
				if (retval) {
					ex_log(LOG_ERROR, "opm_get_enrolled_count failed, return %d", retval);
				}
				if (enrolled_count == 0) {
					ex_log(LOG_ERROR, "no fingerprints, identify will be failed");
				}

				if (check_cancelable()) goto EXIT;
				auth_token_len = AUTH_TOKEN_LEN;
				retval = opm_identify(&g_verify_info, &match_id, &status, auth_token, &auth_token_len, &ext_feat_quality);
				ex_log(LOG_DEBUG, "opm_do_verify ret=%d, status=%u, match_id=%d, ext_feat_quality=%d", retval, status, match_id, ext_feat_quality);
				if (check_cancelable()) goto EXIT;

				if (retval != FINGERPRINT_RES_SUCCESS) {
					break;
				}

				int is_good_extract_qty = (ext_feat_quality > ext_feat_quality_trymatch_th) ? 1 : 0;
				if (FP_LIB_IDENTIFY_NO_MATCH == status || FP_LIB_IDENTIFY_RESIDUAL == status) {
					ex_log(LOG_DEBUG, "(%d) threshold %d, %d", is_good_extract_qty, ext_feat_quality_trymatch_th, ext_feat_quality_badimg_th);
					if (FP_LIB_IDENTIFY_RESIDUAL == status) {
						flow_try_match = 0;
						ex_log(LOG_DEBUG, "FP_LIB_IDENTIFY_RESIDUAL");
					}

					if ((flow_try_match-- > 0) && !is_good_extract_qty) {
						ex_log(LOG_DEBUG, "flow_try_match=%d", flow_try_match);
						vstate = VSTATE_GET_IMG;
						break;
					} else {
						TIME_MEASURE_STOP(total_verify,"match Fail");
						host_touch_set_hbm_system(FALSE);
						if (quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
							notify(EVENT_IMG_PARTIAL, 0, 0, NULL, 0);
#ifdef __ET7XX__
						} else if (ext_feat_quality < ext_feat_quality_badimg_th) {
							ex_log(LOG_DEBUG, "bad ext_feat_quality %d < %d", ext_feat_quality, ext_feat_quality_badimg_th);
							quality = FP_LIB_ENROLL_FAIL_LOW_QUALITY;
							notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);
#endif
						} else {
							notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
						}
#if defined(SIMULATE_NO_SENSOR) || defined(RBS_EVTOOL)
						ex_log(LOG_DEBUG, "evtool has_result");
						has_result = TRUE;
#endif
					}
				} else if (FP_LIB_IDENTIFY_MATCH == status || FP_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE == status) {
					TIME_MEASURE_STOP(total_verify,"matched");
					host_touch_set_hbm_system(FALSE);
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

				vstate = VSTATE_FINGER_OFF;
#ifdef __SUPPORT_SAVE_IMAGE__
				if (quality == FP_LIB_ENROLL_FAIL_LOW_COVERAGE || !is_good_extract_qty) {
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, !(FP_LIB_IDENTIFY_NO_MATCH == status), status);
				} else {
					transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, quality, FALSE, status);
				}
#endif
			break;

			case VSTATE_FINGER_OFF: {
				ex_log(LOG_DEBUG, "vstate == VSTATE_FINGER_OFF");

				setSpiState(TRUE, FALSE);

				if (check_cancelable()) goto EXIT;
				if (host_touch_is_enable()) {
					status = FP_LIB_FINGER_LOST;
					opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);
				} else {
					opm_check_finger_lost(30, &status);

				}
#ifdef __ET7XX__
                                if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                                        ex_log(LOG_ERROR, "power off sensor fail");
                                        retval = FINGERPRINT_RES_FAILED;
                                        goto EXIT;
                                }
#else
                                setSpiState(TRUE, FALSE);
#endif
				if (check_cancelable()) goto EXIT;

				// When finger is not lost, CORE change to detect mode already
				// opm_set_work_mode(DETECT_MODE);
				if (host_touch_is_using_oplus_flow())
				{
					g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;
				}

				/** OPLUS Flow **/
				if (host_touch_is_using_oplus_flow())
				{
					g_trigger_case = TRIGGER_WAIT_TOUCH_UP;
					if (wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) {
						vstate = ESTATE_WAIT_INT;
						notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
						setSpiState(FALSE, FALSE);
						if (has_result) {
							goto EXIT;
						}
					}
				} else  
				/** OPLUS Flow End **/
				{
					if (status == FP_LIB_FINGER_LOST || !wait_trigger(3, 30, TIMEOUT_WAIT_FOREVER)) {
						vstate = ESTATE_WAIT_INT;
						notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
						setSpiState(FALSE, FALSE);
						if (has_result) {
							goto EXIT;
						}
					}
				}
			} break;
		}
	} while (retval == FINGERPRINT_RES_SUCCESS);
EXIT:
	setSpiState(FALSE, FALSE);
	opm_identify_finish();

	if (retval != FINGERPRINT_RES_SUCCESS) {
		notify(EVENT_ERR_IDENTIFY, 0, 0, NULL, 0);
	}
	notify(EVENT_IDENTIFY_FINISH, 0, 0, NULL, 0);
#ifdef __ET7XX__
	retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_SAVE_ET7XX_CALI_DATA, 0, 0, NULL, NULL);
	if (retval != FP_LIB_OK) ex_log(LOG_ERROR, "%s, Save BDS Pool Fail", __func__);
	if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
		ex_log(LOG_ERROR, "power off sensor fail");
    	}
#endif
	return FINGERPRINT_RES_SUCCESS;
}

#ifdef __ENABLE_NAVIGATION__
static int do_navigation_event_in_flow()
{
	int retval;
	int event_size;
	unsigned int nstate = NSTATE_WAIT_INT;

#define OUT_COUNT 5
#define CLICK_TIME 700
#define LONG_CLICK 700
#define DOUBLE_INTERVAL 200

	struct NaviStatusInfo event;
	event_size = sizeof(struct NaviStatusInfo);
	int navi_up = 0;
	int navi_down = 0;
	int navi_left = 0;
	int navi_right = 0;
	int navi_mode = NAVI_MODE_NORMAL;
	int navi_status = NAVI_CMD_START;

	unsigned long long time_start_waitfinger;
	unsigned long cost_fingeron;
	BOOL has_fingeron = FALSE;
	BOOL is_check_click = TRUE;
	BOOL is_check_long_click = FALSE;
	BOOL is_check_double_wait = FALSE;

	navi_mode = g_navi_config.navi_mode;

	retval = opm_navi_control(0, (unsigned char *)&navi_status, sizeof(int), NULL, &event_size);

	do {
		switch (nstate) {
			case NSTATE_WAIT_INT: {
				if (check_cancelable()) goto exit;
				retval = opm_set_work_mode(DETECT_MODE);
				if (check_cancelable()) goto exit;

				if (FINGERPRINT_RES_SUCCESS != retval) {
					ex_log(LOG_ERROR, "ESTATE_WAIT_INT set detect mode failed return = %d", retval);
				}

				navi_up = 0;
				navi_down = 0;
				navi_left = 0;
				navi_right = 0;

				ex_log(LOG_ERROR, "USE_CORE_CONFIG_INI status_fingeron %d,is_check_click %d,is_check_double_wait %d,longlick %d ", has_fingeron, is_check_click, is_check_double_wait, is_check_long_click);

				if (has_fingeron && is_check_click && !is_check_long_click) {
					cost_fingeron = plat_get_diff_time(time_start_waitfinger);
					ex_log(LOG_ERROR, "do_navigation fingeron time %d ", cost_fingeron);
					if (cost_fingeron < CLICK_TIME) {
						if (is_check_double_wait) {
							ex_log(LOG_ERROR, "do_navigation notify double click");
							notify(EVENT_NAVIGATION, EVENT_DCLICK, 0, NULL, 0);
							is_check_double_wait = FALSE;
						} else {
							if (wait_trigger(DOUBLE_INTERVAL / 30, 30, TIMEOUT_WAIT_FOREVER)) {
								ex_log(LOG_ERROR, "do_navigation ready for double click");
								time_start_waitfinger = plat_get_time();
								is_check_double_wait = TRUE;
								has_fingeron = FALSE;
							} else {
								ex_log(LOG_ERROR, "do_navigation notify click");
								notify(EVENT_NAVIGATION, EVENT_CLICK, 0, NULL, 0);
							}
						}
					} else {
						is_check_double_wait = FALSE;
					}
				} else {
					is_check_double_wait = FALSE;
				}

				if (!is_check_double_wait) {
					has_fingeron = FALSE;
					is_check_click = TRUE;
					is_check_long_click = FALSE;
					notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);
					if (!wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) goto exit;

					time_start_waitfinger = plat_get_time();
				}

				nstate = NSTATE_NAVI;
			} break;

			case NSTATE_NAVI: {
				event_size = sizeof(struct NaviStatusInfo);
				if (check_cancelable()) goto exit;
				retval = opm_get_navi_event((unsigned char *)&event, &event_size);
				if (check_cancelable()) goto exit;

				if (!event.is_finger) {
					nstate = NSTATE_WAIT_INT;
					break;
				}
				ex_log(LOG_DEBUG, "EGIS_NAVIGATION NAVI_DX:%d, NAVI_DY:%d", event.navi_dx, event.navi_dy);
				if (navi_mode == NAVI_MODE_NORMAL) {
					has_fingeron = TRUE;
					if (abs(event.navi_dx) > abs(event.navi_dy)) {
						if (event.navi_dx > 0) {
							navi_right++;
						} else {
							navi_left++;
						}
					} else if (abs(event.navi_dx) < abs(event.navi_dy)) {
						if (event.navi_dy > 0) {
							navi_up++;
						} else {
							navi_down++;
						}
					}

					if (navi_up >= OUT_COUNT) {
						ex_log(LOG_ERROR, "do_navigation notify up");
						notify(EVENT_NAVIGATION, EVENT_UP, 0, NULL, 0);
						navi_up = 0;
						is_check_click = FALSE;
					} else if (navi_down >= OUT_COUNT) {
						ex_log(LOG_ERROR, "do_navigation notify down");
						notify(EVENT_NAVIGATION, EVENT_DOWN, 0, NULL, 0);
						navi_down = 0;
						is_check_click = FALSE;
					} else if (navi_left >= OUT_COUNT) {
						ex_log(LOG_ERROR, "do_navigation notify left");
						notify(EVENT_NAVIGATION, EVENT_LEFT, 0, NULL, 0);
						navi_left = 0;
						is_check_click = FALSE;
					} else if (navi_right >= OUT_COUNT) {
						ex_log(LOG_ERROR, "do_navigation notify right");
						notify(EVENT_NAVIGATION, EVENT_RIGHT, 0, NULL, 0);
						navi_right = 0;
						is_check_click = FALSE;
					}
					if (is_check_click) {
						cost_fingeron = plat_get_diff_time(time_start_waitfinger);
						if (cost_fingeron > LONG_CLICK) {
							ex_log(LOG_ERROR, "do_navigation notify hold");
							notify(EVENT_NAVIGATION, EVENT_HOLD, 0, NULL, 0);
							time_start_waitfinger = plat_get_time();
							is_check_long_click = TRUE;
							ex_log(LOG_ERROR, "do_navigation notify long click ");
						}
					}
				} else {
					notify(EVENT_NAVI_DX_DY, event.navi_dx, event.navi_dy, NULL, 0);
				}

			} break;
		}
	} while (1);

exit:
	ex_log(LOG_DEBUG, "navigation loop end!!!!!! retval = %d", retval);

	navi_status = NAVI_CMD_STOP;
	retval = opm_navi_control(0, (unsigned char *)&navi_status, sizeof(int), NULL, &event_size);

	opm_set_work_mode(POWEROFF_MODE);
	return FINGERPRINT_RES_SUCCESS;
}
#if 0
static int do_navigation()
{
	int retval;
	int event, event_size;
	unsigned int status;
	ex_log(LOG_DEBUG, "do_navigation enter");

	do {
		if (check_cancelable()) goto exit;
		retval = opm_set_work_mode(NAVI_DETECT_MODE);
		if (check_cancelable()) goto exit;
		if (FINGERPRINT_RES_SUCCESS != retval) {
			ex_log(LOG_ERROR, "set detect mode failed, retval = %d",
			       retval);
			break;
		}

		ex_log(LOG_DEBUG, "waiting for interrupt");
		if (!wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) goto exit;

		ex_log(LOG_DEBUG, "finger touch");

		event = 0;
		event_size = sizeof(event);
		if (check_cancelable()) goto exit;
		retval =
		    opm_get_navi_event((unsigned char *)&event, &event_size);
		if (check_cancelable()) goto exit;

		ex_log(LOG_DEBUG, "opm_get_navi_event retval = %d, event = %d", retval, event);
		notify(EVENT_NAVIGATION, event, 0, NULL, 0);

		while (1) {
			if (check_cancelable()) goto exit;
			opm_check_finger_lost(30, &status);
			if (check_cancelable()) goto exit;

			if (status == FP_LIB_FINGER_LOST ||
			    !wait_trigger(3, 30, TIMEOUT_WAIT_FOREVER)) {
				ex_log(LOG_DEBUG, "finger leave");
				break;
			}
		}
	} while (1);

exit:
	ex_log(LOG_DEBUG, "navigation loop end!!!!!! retval = %d", retval);

	return FINGERPRINT_RES_SUCCESS;
}
#endif
#endif  // #ifdef __ENABLE_NAVIGATION__

static int do_navigation_setting()
{
#ifdef __ENABLE_NAVIGATION__
	if (navi_is_enable()) {
		do_navigation_event_in_flow();
	} else {
		power_off();
	}
#else
	power_off();
#endif
	return FINGERPRINT_RES_SUCCESS;
}

int cpt_initialize(unsigned char *in_data, unsigned int in_data_len)
{
	int retval;
	g_hardware_ready = FALSE;
    uint64_t current_time = get_ca_time_ms();

    opm_set_data(TYPE_SEND_CA_TIME, (unsigned char*)&current_time, sizeof(uint64_t));

#ifdef HOST_TOUCH_CONTROL
	host_touch_set_enable(TRUE);
#endif

	if (g_hdev <= 0) {
		retval = fp_device_open(&g_hdev);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			ex_log(LOG_ERROR,
			       "cpt_initialize fp_device_open failed");
			retval = FINGERPRINT_RES_HW_UNAVALABLE;
			goto EXIT;
		}
	}

	retval = fp_device_power_control(g_hdev, TRUE);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize fp_device_power_control failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}
	g_power_managemen.pwr_status = SENSOR_PWR_ON;
	retval = fp_device_reset(g_hdev);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize fp_device_reset failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}

	retval = fp_device_clock_enable(g_hdev, TRUE);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize fp_device_clock_enable failed");
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}

	retval = opm_initialize_sdk(in_data, in_data_len);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize opm_initialize_sdk return = %d", retval);
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}

	retval = create_ini_config(TRUE, NULL, 0);
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize create_ini_config return = %d", retval);

		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}

	retval = opm_initialize_sensor();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize opm_initialize_sensor return = %d",
		       retval);
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
	}

	retval = opm_initialize_algo();
	if (FINGERPRINT_RES_SUCCESS != retval) {
		ex_log(LOG_ERROR,
		       "cpt_initialize opm_initialize_algo return = %d",
		       retval);
		retval = FINGERPRINT_RES_HW_UNAVALABLE;
		goto EXIT;
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
	host_touch_set_hbm_evalute_always();
	thread_manager_init();
	thread_manager_set_cancel_func((rbsCancelFunc)captain_cancel);
	thread_manager_set_idle_task((do_operation_callback)do_navigation_setting);
#ifdef __OPLUS__
	if (host_touch_is_using_oplus_flow())
	{
		//host_touch_set_callback(notify);
		init_oplus_event_listener(g_hdev);
	}
#endif
EXIT:
        if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_ERROR, "power off sensor fail");
                retval = FINGERPRINT_RES_FAILED;
        }
	return retval;
}

int cpt_uninitialize()
{
	int retval;
	cpt_cancel();

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

	if (0 != g_hdev) {
		fp_device_clock_enable(g_hdev, FALSE);
		fp_device_power_control(g_hdev, FALSE);
		fp_device_close(g_hdev);
		g_hdev = 0;
	}

	thread_manager_uninitialize();

	return 0;
}

int cpt_cancel()
{
	ex_log(LOG_DEBUG, "cpt_cancel enter");
	g_has_enroll_count = 0;
	g_enroll_percentage_tmp = 0;
	g_temp_remaining = 0;
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

	retval = opm_set_active_group(user_id, data_path);
	ex_log(LOG_DEBUG, "opm_set_active_group return = %d", retval);
	if (FINGERPRINT_RES_SUCCESS == retval) {
		g_cache_info.user_id = -1;

		strncpy(g_user_path, data_path, MAX_PATH_LEN);
		opt_send_data(TYPE_SEND_TEMPLATE, NULL, 0);

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
	return opm_chk_secure_id(user_id, secure_id);
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

	if (host_touch_is_using_oplus_flow())
		host_touch_set_callback(notify);
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

unsigned long g_first_auth;

#define SKIP_FIST_TOUCH_INTERVAL 1000

static void auth_notify(int event_id, int first_param, int second_param,
		unsigned char *data, int data_size)
{
	if (event_id == EVENT_FINGER_TOUCH_DOWN && g_screen_state == 0) {
	    if (plat_get_diff_time(g_first_auth) < SKIP_FIST_TOUCH_INTERVAL) {
	        egislog_d("skip first touch");
                return;
	    }
	}
	return notify(event_id, first_param,second_param, data, data_size);
}

int cpt_authenticate(fingerprint_verify_info_t verify_info)
{
	if (g_cache_info.fingerprint_ids_count <= 0) {
		return FINGERPRINT_RES_SUCCESS;
	}

	if (TRUE != g_hardware_ready) {
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}
	g_verify_info = verify_info;
	memcpy(g_verify_info.fingerprints.fingerprint_ids,
	       verify_info.fingerprints.fingerprint_ids,
	       sizeof(int) * verify_info.fingerprints.fingerprint_ids_count);

	g_first_auth = plat_get_time();  

	if (host_touch_is_using_oplus_flow())
		host_touch_set_callback(auth_notify);


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
	opt_receive_data(
	    TYPE_DELETE_TEMPLATE,
	    (unsigned char *)&remove_info.fingerprint_info.fingerprint_id,
	    fid_size, NULL, 0);
	thread_unlock_operation();

	return retval;
}

int cpt_get_fingerprint_ids(unsigned int user_id, fingerprint_ids_t *fps)
{
	if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_get_fingerprint_ids another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}

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
	thread_unlock_operation();

	return FINGERPRINT_RES_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS
						 : FINGERPRINT_RES_FAILED;
}
#if 0
// ToDo:
void cpt_disable_navigation(BOOL onoff)
{
	if (onoff) thread_manager_set_idle_task(do_navigation_event_in_flow);
	else       thread_manager_set_idle_task(power_off);
}
#endif

int cpt_navigation()
{
	ex_log(LOG_ERROR, "cpt_navigation not support this interface");

	return FINGERPRINT_RES_SUCCESS;
}

void cpt_set_event_callback(event_callbck_t on_event_callback)
{
	g_event_callback = on_event_callback;
	ex_log(LOG_DEBUG, "cpt_set_event_callback = %p, size = %d", g_event_callback, sizeof(event_callbck_t));
	opm_set_data(TYPE_SEND_CALLBACK_FUNCTION, (unsigned char*)&g_event_callback, sizeof(event_callbck_t));
}

static void get_ca_version(unsigned char *version, int *len)
{
	if (version == NULL || len == NULL) return;

	int version_len = 0;
	version_len = sizeof("br_ipp6_pre-verify_2e89c8c");
	if (version_len <= *len) {
		*len = version_len;
		memcpy(version, "br_ipp6_pre-verify_2e89c8c", version_len);
	}

	ex_log(LOG_DEBUG, "version_len = %d , version = %s", *len, version);
}

int cpt_extra_api(int type, unsigned char *in_data, int in_data_size,
		  unsigned char *out_buffer, int *out_buffer_size)
{
	int retval = FINGERPRINT_RES_SUCCESS, cid = 0;

	if (in_data == NULL || in_data_size < 1) {
		// Bypass to  opm_extra_command();
		ex_log(LOG_DEBUG, "%s [%d] in_data_size %d", __func__, type, in_data_size);
		retval = thread_try_lock_operation();
		if (retval != 0) {
			ex_log(LOG_ERROR, "Failed try lock. retval=%d", retval);
			return FINGERPRINT_RES_NOT_IDLE;
		}
		retval = opm_extra_command(type, in_data, in_data_size, out_buffer, out_buffer_size);
		thread_unlock_operation();
		ex_log(retval == 0 ? LOG_DEBUG : LOG_ERROR, "cpt_extra_api end, cid = %d, retval=%d", cid, retval);
		return retval;
	}

	uint8_t* extra_data = NULL;
	int extra_data_size = 0;
	if (in_data_size < 4) {
		ex_log(LOG_ERROR, "%s, in_data_size %d < 4", __func__, in_data_size);
		cid = in_data[0];
	} else {
		cid = *(int *)in_data;
		if (in_data_size > 4) {
			extra_data = in_data + 4;
			extra_data_size = in_data_size - 4;
		}
	}
	ex_log(LOG_DEBUG, "cpt_extra_api [%d], cid = %d, in_data_size = %d", type, cid, in_data_size);

	BOOL needLockThread;
	switch (type) {
		case PID_HOST_TOUCH:
			needLockThread = FALSE;
			break;
		case PID_INLINETOOL:
			if (SENSORTEST_IMAGE_QTY == cid || cid == SENSORTEST_AGING_TEST)
				needLockThread = FALSE;
			else
				needLockThread = TRUE;
			break;
		default:
			needLockThread = TRUE;
			break;
	}

	if (needLockThread && FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
		ex_log(LOG_ERROR, "cpt_extra_api another operation is doing ,return not idle");
		return FINGERPRINT_RES_NOT_IDLE;
	}

	switch (type) {
#if defined(__ET7XX__) && !defined(__ET0XX__)
		case PID_7XX_INLINETOOL: {
			int *param = (int *)in_data;
                        if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                                ex_log(LOG_ERROR, "power on sensor fail");
                                retval = FINGERPRINT_RES_FAILED;
                                goto EXIT;
                        }
			retval = do_7XX_sensortest(cid, param[1], param[2], param[3], out_buffer, out_buffer_size);
			ex_log(LOG_DEBUG, "cid=%d, retval=%d", cid, retval);
                        if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                                ex_log(LOG_ERROR, "power off sensor fail");
                                retval = FINGERPRINT_RES_FAILED;
                                goto EXIT;
                        }
                        goto EXIT;
		}
#endif
		case PID_INLINETOOL:
		case PID_FAETOOL: {
                        if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                                ex_log(LOG_ERROR, "power on sensor fail");
                                retval = FINGERPRINT_RES_FAILED;
                                goto EXIT;
                        }
			retval = do_sensortest(cid, in_data, in_data_size, out_buffer, out_buffer_size);
                        if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                                ex_log(LOG_ERROR, "power off sensor fail");
                                retval = FINGERPRINT_RES_FAILED;
                                goto EXIT;
                        }
			goto EXIT;
		}
		case PID_COMMAND: {
			if (CMD_UPDATE_CONFIG == cid) {
				destroy_ini_config();
				if ((uint32_t)in_data_size > sizeof(int)) {
					retval = create_ini_config(FALSE, in_data + sizeof(int), in_data_size - sizeof(int));
				} else {
					retval = create_ini_config(FALSE, NULL, 0);
				}

				goto EXIT;
			} else if (CMD_GET_CONFIG == cid) {
				if (out_buffer != NULL && out_buffer_size != NULL) {
					retval = opt_receive_data(TYPE_RECEIVE_INI_CONFIG, NULL, 0, out_buffer, out_buffer_size);
				}
				goto EXIT;
			} else if (CMD_UPDATE_DB_CONFIG == cid) {
				if ((uint32_t)in_data_size > sizeof(int))
					retval = opt_send_data(TYPE_SEND_DB_INI_CONFIG, in_data + sizeof(int), in_data_size - sizeof(int));
				else {
					retval = FINGERPRINT_RES_FAILED;
					ex_log(LOG_ERROR, "Failed to update db config ini");
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

			if (CMD_REMOVE_INI_FILE == cid) {
				retval = opt_send_data(TYPE_REMOVE_INI_FILE, NULL, 0);
				goto EXIT;
			}
		}

		case PID_DEMOTOOL: {
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
				case CMD_VERSION_IP:
					retval = opm_get_data(TYPE_RECEIVE_IP_VERSION, NULL, 0, out_buffer, out_buffer_size);
					break;
				case EXTRA_DT_SET_INI_CONFIG_PATH:
					retval = opt_send_data(TYPE_SEND_INI_CONFIG_PATH, extra_data, extra_data_size);
					break;
				default:
					break;
			}
			goto EXIT;
		}
		case PID_BKG_IMG: {
			switch (cid) {
				case CMD_BKG_IMG_RUN_CALI_PROC:
					ex_log(LOG_DEBUG, "CMD_BKG_IMG_RUN_CALI_PROC");
					retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_CALI_ET7XX_BKG, 0, 0, NULL, NULL);
					if (retval == FP_LIB_OK) {
						retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_SAVE_ET7XX_CALI_DATA, 0, 0, NULL, NULL);
					}
					break;
				case CMD_BKG_IMG_SAVE_CALI_DATA:
					ex_log(LOG_DEBUG, "CMD_BKG_IMG_SAVE_CALI_DATA file path = %s", in_data + sizeof(int));
					retval = opm_set_data(TYPE_IC_SAVE_CALIBRATION_DATA, in_data + sizeof(int), in_data_size - sizeof(int));
					break;
				case CMD_BKG_IMG_LOAD_CALI_DATA:
					ex_log(LOG_DEBUG, "CMD_BKG_IMG_LOAD_CALI_DATA file path = %s", in_data + sizeof(int));
					retval = opm_set_data(TYPE_IC_LOAD_CALIBRATION_DATA, in_data + sizeof(int), in_data_size - sizeof(int));
					break;
				case CMD_BKG_IMG_GET_CALI_IMAGE: {
					ex_log(LOG_DEBUG, "CMD_BKG_IMG_GET_CALI_IMAGE");
					int cali_image_size = sizeof(liver_image_out_header_t) + MAX_IMAGE_BUFFER_SIZE * 2;
					unsigned char *cali_image = plat_alloc(cali_image_size);
					if (cali_image != NULL) {
						retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_GET_CALI_IMAGE, 0, 0, 0, cali_image, &cali_image_size);
						notify(EVENT_RETURN_LIVE_IMAGE_OUT, 0, 0, cali_image, cali_image_size);
						PLAT_FREE(cali_image);
					} else {
						ex_log(LOG_ERROR, "failed to allocate %d", cali_image_size);
						retval = FINGERPRINT_RES_ALLOC_FAILED;
					}
					break;
				}
				case CMD_BKG_IMG_RETURN_IMAGE: {
					ex_log(LOG_DEBUG, "CMD_BKG_IMG_RETURN_IMAGE");
					retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_GET_IMAGE, 0, 0, 0, out_buffer, out_buffer_size);
					break;
				}
				case CMD_BKG_IMG_REMOVE_CALI_DATA:
					ex_log(LOG_DEBUG, "CMD_BKG_IMG_REMOVE_CALI_DATA");
					retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_REMOVE_ET7XX_CALI_DATA, 0, 0, NULL, NULL);
					break;
				case CMD_BKG_IMG_REMOVE_BDS:
					ex_log(LOG_DEBUG, "CMD_BKG_IMG_REMOVE_BDS");
					retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_REMOVE_ET7XX_BDS, 0, 0, NULL, NULL);
					break;
				default:
					ex_log(LOG_ERROR, "unsupported cid : %d", cid);
					break;
			}
			goto EXIT;
		}
		case PID_HOST_TOUCH: {
			switch (cid) {
				case CMD_HOST_TOUCH_PARTIAL_TOUCHING:
					host_touch_set_finger_on(TRUE);
					host_touch_set_low_coverage(TRUE);
					break;
				case CMD_HOST_TOUCH_SET_TOUCHING: {
#if defined(__ET7XX__) && !defined(__ET0XX__)
					int sys_temp;
					get_sys_temp(&sys_temp);
					host_device_info_t info;
					info.temperature_x10 = sys_temp;
					set_host_device_info(info);
					if (out_buffer != NULL && out_buffer_size != NULL && *out_buffer_size >= 4) {
						memcpy(out_buffer, &sys_temp, sizeof(int));
					}
#endif
					host_touch_set_finger_on(TRUE);
					break;
				}
				case CMD_HOST_TOUCH_RESET_TOUCHING:
					host_touch_set_finger_off(TRUE);
					break;
				case CMD_HOST_TOUCH_SEND_TEMPERATURE: {
					host_device_info_t info;
					info.temperature_x10 = *((int *)in_data + 1);
					ex_log(LOG_DEBUG, "ignore app temperature: %d", info.temperature_x10);
					// set_host_device_info(info);
					break;
				}
				case CMD_HOST_TOUCH_SET_UI_READY: {
					host_touch_set_ui_ready(TRUE);
					break;
				}
				case CMD_HOST_TOUCH_CUSTOM_SCREEN_STATE_ON: {
					g_screen_state = TRUE ;
					ex_log(LOG_DEBUG, "g_screen_state %d", g_screen_state);
					break;
				}
				case CMD_HOST_TOUCH_CUSTOM_SCREEN_STATE_OFF: {
					g_screen_state = FALSE ;
					ex_log(LOG_DEBUG, "g_screen_state %d", g_screen_state);
					break;
				}
				case CMD_DEMOTOOL_SET_SCREEN_BRIGHTNESS: {
#if defined(__ET7XX__) && !defined(__ET0XX__)
					int value = *((int *)in_data + 1);
					ex_log(LOG_DEBUG, "set brightness: %d", value);
					retval = set_brightness(value);
#endif
					goto EXIT;
				}
				default:
					ex_log(LOG_ERROR, "unsupported cid : %d", cid);
					break;
			}
			retval = FINGERPRINT_RES_SUCCESS;
			goto EXIT;
		}
		case PID_SET_ENROLL_TIMEOUT: {
			g_enroll_timeout = cid;
			ex_log(LOG_DEBUG, "set enroll timeout = %d ", g_enroll_timeout);
			goto EXIT;
		}
		case PID_EVTOOL: {
			const int PARAM_BUF_ENROLLED_PATH = 1000;
			const int PARAM_BUF_VERIFIED_PATH = 1001;
			const int PARAM_INT_DB_TOTAL_FINGER = 730;
			const int PARAM_BUF_DB_TOTAL_FINGERPRINT = 731;
			switch(cid) {
				case CMD_GET_ENROLL_FILE_NAME: {
					int param = PARAM_BUF_ENROLLED_PATH;
					retval = opm_get_data(TYPE_RECEIVE_SENSOR_BUF, (uint8_t*) &param, sizeof(param), out_buffer, out_buffer_size);
				} break;
				case CMD_GET_VERIFY_FILE_NAME: {
					int param = PARAM_BUF_VERIFIED_PATH;
					retval = opm_get_data(TYPE_RECEIVE_SENSOR_BUF, (uint8_t*) &param, sizeof(param), out_buffer, out_buffer_size);
				} break;
				case CMD_GET_MATCH_SCORE: {
					retval = opm_get_data(TYPE_EVTOOL_RECEIVE_MATCH_SCORE, NULL, 0, out_buffer, out_buffer_size);
				} break;
				case CMD_INT_DB_TOTAL_FINGER: {
					int param = PARAM_INT_DB_TOTAL_FINGER;
					retval = opm_get_data(TYPE_RECEIVE_SENSOR_INT_PARAM, (uint8_t*) &param, sizeof(param), out_buffer, out_buffer_size);
				} break;
				case CMD_BUF_DB_TOTAL_FINGERPRINT: {
					int param = PARAM_BUF_DB_TOTAL_FINGERPRINT;
					retval = opm_get_data(TYPE_RECEIVE_SENSOR_BUF, (uint8_t*) &param, sizeof(param), out_buffer, out_buffer_size);
				} break;
				default:
					ex_log(LOG_ERROR, "unsupported pid : %d, cid : %d", type, cid);
					break;
			}
			goto EXIT;
		}
#ifdef __SDK_SAVE_IMAGE_V2__
		case PID_SYSUNLOCKTOOL_NAME: {
			char *str, *delim = "/";
			char tmp[64];

			memset(g_username, 0, sizeof(g_username));
			memset(g_fingerid, 0, sizeof(g_fingerid));
			memset(g_case, 0, sizeof(g_case));
			memset(g_test_case_id, 0, sizeof(g_test_case_id));
			memset(tmp, 0, sizeof(tmp));
			ex_log(LOG_DEBUG, "in_data = %s", in_data);
			memcpy(tmp, in_data, strlen((const char*)in_data)+1);
			ex_log(LOG_DEBUG, "tmp = %s", tmp);

			str = strtok(tmp, delim);
			ex_log(LOG_DEBUG, "str = %s", str);
			if(str)
			{
				memcpy(g_test_case_id, str, strlen(str)+1);
				ex_log(LOG_DEBUG, "g_test_case_id = %s", g_test_case_id);
				str = strtok(NULL, delim);

				memcpy(g_username, str, strlen(str)+1);
				str = strtok(NULL, delim);

				memcpy(g_fingerid, str, strlen(str)+1);
				ex_log(LOG_DEBUG, "g_fingerid = %s", g_fingerid);
				str = strtok(NULL, delim);
				
				memcpy(g_case, str, strlen(str)+1);
				ex_log(LOG_DEBUG, "g_case = %s", g_case);
				reset_finger_count();
			}

			goto EXIT;
		}
		case PID_SYSUNLOCKTOOL_TOTAL_COUNT: {
			retval = get_total_count();
			goto EXIT;		
		}
		case PID_SYSUNLOCKTOOL_MATCH_COUNT: {
			retval = get_match_count();
			goto EXIT;		
		}
		case PID_SYSUNLOCKTOOL_NOT_MATCH_COUNT: {
			retval = get_not_match_count();
			goto EXIT;		
		}
#endif
	}

	ex_log(LOG_VERBOSE, "opm_extra_command");
	retval = opm_extra_command(type, in_data, in_data_size, out_buffer, out_buffer_size);

EXIT:
	if (needLockThread) {
		thread_unlock_operation();
	}
	ex_log(LOG_DEBUG, "cpt_extra_api end, cid = %d, retval=%d", cid, retval);

	return retval;
}

#if defined (__SHOW_LIVEIMAGE__) || defined (__SUPPORT_SAVE_IMAGE__)
static BOOL set_img_state(transfer_image_type_t type, int img_quality, BOOL force_to_good)
{
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

	ex_log(LOG_ERROR, "invalid parameter, type = %d", type);
	return FALSE;
}
#ifdef __ET7XX__
static int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good, int match_result)
{
	const int buffer_size = sizeof(rbs_obj_array_request_t) + MULTI_MAX_IMAGE_BUFFER_SIZE;
	int out_size;
	int retval = FINGERPRINT_RES_FAILED, i, j;

	if (type == TRANSFER_LIVE_IMAGE) {
		ex_log(LOG_ERROR, "%s Need to implement getting the last image of IMGTYPE_8BIT", __func__);
		return FINGERPRINT_RES_FAILED;
	}

	ex_log(LOG_VERBOSE, "%s (%d) buf_size=%d, match_result=%d", __func__, type, buffer_size, match_result);
	ex_log(LOG_VERBOSE, "%s, img_quality=%d, force_to_good=%d", __func__, img_quality, force_to_good);
	int is_good_img = set_img_state(type, img_quality, force_to_good);
	ex_log(LOG_VERBOSE, "%s, is_good_img=%d", __func__, is_good_img);
	unsigned char *buffer = plat_alloc(buffer_size);
	if (buffer == NULL) {
		return FINGERPRINT_RES_ALLOC_FAILED;
	}
	rbs_obj_array_request_t array_req;
	rbs_obj_array_t* out_obj_array;
	uint32_t obj_id, obj_total_size;
	int img_type_request[3] = {IMGTYPE_8BIT, IMGTYPE_RAW, IMGTYPE_BKG};
#ifdef ONLY_REQUEST_IMGTYPE_BKG // temporary use define to switch
	int start_request_idx = 2;
#else
	int start_request_idx = 0;
#endif
	for (i = 0; ; i++) {
		for (j = start_request_idx; j < (int)(sizeof(img_type_request) / sizeof(*img_type_request)); j++) {
			out_size = buffer_size;
			array_req.param1 = img_type_request[j];
			RBSOBJ_set_ARRAY_REQ_v1_0((&array_req), i, 1, RBSOBJ_ID_IMAGE);
			retval = opt_receive_data(TYPE_RECEIVE_IMAGE_OBJECT_ARRAY, (unsigned char *)&array_req, sizeof(array_req), buffer, &out_size);
			out_obj_array = (rbs_obj_array_t *)buffer;
			if (out_obj_array->count == 0) {
				ex_log(LOG_ERROR, "OBJECT_ARRAY count is 0");
				break;
			}
			obj_id = RBSOBJ_get_objarray_obj_id(out_obj_array);
			if (obj_id == RBSOBJ_ID_IMAGE) {
				rbs_obj_image_v1_0_t *img_obj = (rbs_obj_image_v1_0_t *)RBSOBJ_get_payload_pointer(out_obj_array);
				if (!is_good_img) {
					ex_log(LOG_VERBOSE, "bad image [%d][%d]", i, j);
					RBSOBJ_set_IMAGE_param(img_obj, is_bad, 1);
				}
				obj_total_size = RBSOBJ_get_obj_total_size(out_obj_array);
				ex_log(LOG_DEBUG, "OBJECT_ARRAY total_size(%d) image %d:%d, bpp=%d", obj_total_size, img_obj->width, img_obj->height, img_obj->bpp);
				notify(EVENT_RETURN_IMAGEOBJ, 0, 0, buffer, obj_total_size);
#ifdef __SDK_SAVE_IMAGE_V2__
				save_image_info_t save_image_info;
				save_image_info.img_type = type;
				save_image_info.rbs_obj_array = out_obj_array;
				save_image_info.rbs_obj_image = img_obj;

				if (type == TRANSFER_VERIFY_IMAGE_V2) {
					if (is_good_img == FALSE) {
						save_image_info.img_state = SAVE_BAD;
					} else if (match_result == FP_LIB_IDENTIFY_NO_MATCH) {
						save_image_info.img_state = SAVE_GOOD_NOT_MATCH;
					} else {
						save_image_info.img_state = SAVE_GOOD_MATCH;
					}
				}
				else {
					if (is_good_img == FALSE) {
						save_image_info.img_state = SAVE_BAD;
					} else {
						save_image_info.img_state = SAVE_GOOD_ENROLL;
					}
				}
				debug_save_image(save_image_info);
#endif
			} else {
				ex_log(LOG_ERROR, "OBJECT_ARRAY unexpected obj_id %d", obj_id);
			}
		}
		if (!out_obj_array->has_more_object) break;
	}
	opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);

	PLAT_FREE(buffer);
	return retval;
}
#else
static int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good, int match_result)
{
	const int buffer_size = sizeof(receive_images_out_t) + MULTI_MAX_IMAGE_BUFFER_SIZE;
	int out_size;
	int retval = FINGERPRINT_RES_FAILED, width, height,bpp, i,img_size;
	int type_id;
	return_image_info_t return_image_info = {0};
	BOOL is_good_img = FALSE;
	BOOL is_new_finger_on = TRUE;

	receive_images_in_t in_data;
	receive_images_out_t *out_buffer = NULL;

#ifndef EGIS_DBG
	return FINGERPRINT_RES_FAILED;
#endif

#ifdef __SDK_SAVE_IMAGE__
	save_image_info_t save_image_info;
#endif

	ex_log(LOG_DEBUG, "%s (buffer size=%d) match_result=%d", __func__, buffer_size, match_result);
	unsigned char *buffer = (unsigned char *)plat_alloc(buffer_size);
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
		type_id = TYPE_RECEIVE_LIVE_IMAGE;
	} else {
		type_id = TYPE_RECEIVE_MULTIPLE_IMAGE;
	}

	do {
		out_size = buffer_size;
		retval = opt_receive_data(type_id, (unsigned char *)&in_data, sizeof(in_data), buffer, &out_size);
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
				save_image_info.img_width = width;
				save_image_info.img_height = height;
				save_image_info.img_cnt = out_buffer->image_count_included;
				save_image_info.select_index = out_buffer->identify_info.save_index;
				save_image_info.img_data_size = img_size * out_buffer->image_count_included;
				save_image_info.img_buf = image_buf;
				debug_save_image(save_image_info);
#endif
			} else {
				if (out_buffer->image_count_included <= 0) {
					ex_log(LOG_ERROR, "no output image !");
				}
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
					save_image_info.img_width = width;
					save_image_info.img_height = height;
					save_image_info.is_new_finger_on = is_new_finger_on;
					save_image_info.img_cnt = 1;
					save_image_info.select_index = 0;
					save_image_info.img_data_size = img_size;
					save_image_info.img_buf = image_buf + i * width * height;
					debug_save_image(save_image_info);
#endif
				}
			}
			is_new_finger_on = FALSE;

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
	} while (1);

	PLAT_FREE(buffer);
	return retval;
}
#endif // __ET7XX__
#endif

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

static int touch_enroll_remaining_no_percentage(int percentage)
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
	}else
	{
		remaining = g_enroll_config.enroll_max_count - g_has_enroll_count;
		if (remaining < 1) remaining = 1;
		if (remaining > g_enroll_config.enroll_max_count)
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
			retval = host_touch_is_using_oplus_flow() ?
			   	touch_enroll_remaining(percentage)
			   	: touch_enroll_remaining_no_percentage(percentage);
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

int do_sensor_aging_test()
{
	int retval = FINGERPRINT_RES_SUCCESS;
	ex_log(LOG_DEBUG, "%s enter !!!", __func__);
	
	while (!(check_cancelable()))
	{
#if defined(__ET7XX__) && !defined(__ET0XX__)		
                if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                        ex_log(LOG_ERROR, "power on sensor fail");
                        retval = FINGERPRINT_RES_FAILED;
                        break;
                }
		retval = do_7XX_spi_test(FP_INLINE_7XX_CALI_SPI_TEST);
                if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                        ex_log(LOG_ERROR, "power on sensor fail");
                        retval = FINGERPRINT_RES_FAILED;
                        break;
                }

#endif		
		cmd_test_result_t notify_result = {};
		notify_result.test_result_data[0] = 100;
		notify_result.test_result_data[1] = (retval == FP_LIB_OK) ? 0 : 1;
		//notify_result.test_result_data[1] = 0;
		notify(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, 513, 2 * sizeof(uint32_t), (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
	}
	return 0;
}

int do_image_qty_test()
{
	int in_data[2];
	static int image_score = 100;
	int buffer_size = (MAX_IMAGE_BUFFER_SIZE *2) + sizeof(liver_image_out_header_t);
	int retval = FINGERPRINT_RES_SUCCESS;
	liver_image_out_header_t *pheader ;
	unsigned char * buffer = NULL ;
	engineer_info_t info[3] = {};

	memset(in_data, 0x00, sizeof(in_data));
	in_data[1] = 4;

	ex_log(LOG_DEBUG, "%s enter !!!", __func__);
	if (check_cancelable()) return FINGERPRINT_RES_CANCEL;
	retval = opm_set_work_mode(DETECT_MODE);
	if (check_cancelable()) return FINGERPRINT_RES_CANCEL;

	buffer = (unsigned char *)malloc(buffer_size);	
	if (buffer == NULL) {
		return FINGERPRINT_RES_ALLOC_FAILED;
	}


	while (!(check_cancelable()))
	{
		host_touch_set_finger_on(FALSE);
		host_touch_set_finger_off(FALSE);
		memset(buffer, 0x00, buffer_size);
		if (wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) 
		{
			ex_log(LOG_DEBUG, "%s enter has finger on sensor", __func__);
                        if (check_cancelable()) {
                                retval = FINGERPRINT_RES_CANCEL;
                                break;
                        }
		}
		//if (!wait_trigger(0, INTERRUPT_TIMEOUT))
		/*
		if (host_touch_is_finger_off())
		{
			ex_log(LOG_DEBUG, "%s finger Leaved", __func__);
			continue;
			//return FINGERPRINT_RES_CANCEL;
		}
		*/
		ex_log(LOG_DEBUG, "%s image test finger touch !!!", __func__);
		
		notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);
                if (check_cancelable()) {
                        retval = FINGERPRINT_RES_CANCEL;
                        break;
                }
                if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                        ex_log(LOG_ERROR, "power on sensor fail");
                        retval = FINGERPRINT_RES_FAILED;
                        break;
                }
		
		retval = sensor_test_opation(SENSORTEST_GET_IMAGE, g_hdev, (unsigned char *)in_data, sizeof(in_data), buffer, &buffer_size);
                if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                        ex_log(LOG_ERROR, "power on sensor fail");
                        retval = FINGERPRINT_RES_FAILED;
                        break;
                }

		pheader = (liver_image_out_header_t *)buffer ;
		image_score = pheader->image_par_t.optical_quality_data.partial_score;

		info[0].key = 0;
		info[0].value = 1;

		info[1].key = 1;
		info[1].value = image_score;
		info[2].key = 9;
		info[2].value = 1;

		notify(	EVENT_SENSOR_OPTICAL_CALI_IMAGE_QTY, retval, image_score--, (unsigned char*)info, 3 * sizeof(engineer_info_t) );
	}

	free(buffer);
	return retval;
}

static int do_sensortest(int cid, unsigned char *in_data, int in_data_size,
			 unsigned char *out_buffer, int *out_buffer_size)
{
	ex_log(LOG_DEBUG, "%s enter! cid = %d", __func__, cid);
	int retval = FINGERPRINT_RES_FAILED;
	int buffer_size = MAX_IMAGE_BUFFER_SIZE * 2;

	ex_log(LOG_DEBUG, "%s, out buffer (%p) %d", __func__, out_buffer, out_buffer_size ? *out_buffer_size : 0);

	unsigned char *buffer;
	BOOL use_local_buffer;
	if (out_buffer != NULL && out_buffer_size != NULL && *out_buffer_size > 0) {
		buffer = out_buffer;
		buffer_size = *out_buffer_size;
		use_local_buffer = FALSE;
	} else {
		buffer = plat_alloc(MAX_IMAGE_BUFFER_SIZE * 2);
		use_local_buffer = TRUE;
	}

	if (buffer != NULL) {
		g_need_cancel = FALSE;

		if (SENSORTEST_IMAGE_QTY == cid) {
			thread_manager_run_task((do_operation_callback)do_image_qty_test, TASK_PROCESS);
			retval = FINGERPRINT_RES_SUCCESS;
			goto EXIT;
		}else if (SENSORTEST_AGING_TEST  == cid) {
			thread_manager_run_task((do_operation_callback)do_sensor_aging_test, TASK_PROCESS);
			retval = FINGERPRINT_RES_SUCCESS;
			goto EXIT;
		}

		retval = sensor_test_opation(cid, g_hdev, in_data, in_data_size, buffer, &buffer_size);
		if (SENSORTEST_GET_IMAGE == cid && FINGERPRINT_RES_SUCCESS == retval) {
			if (use_local_buffer) {
				notify(EVENT_RETURN_IMAGE, 0, 0, buffer, buffer_size);
			}
		}

		if(SENSORTEST_GET_NVM_UID == cid && FINGERPRINT_RES_SUCCESS == retval) {
			notify(EVENT_SENSROR_TEST_NVM_UID, buffer_size, 0, buffer, buffer_size);
		}
	}

EXIT:
	if (use_local_buffer) {
		PLAT_FREE(buffer);
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

int get_sys_temp(int* temperature) {
    int bat_temp;
    char temp_buf[5];

    memset(temp_buf, 0x00, sizeof(temp_buf));
    FILE* fdd = fopen("/sys/class/power_supply/battery/temp", "r");

    if (fdd == NULL) {
        ex_log(LOG_ERROR, "can not open the file");
        *temperature = 250;
        return -1;
    }
    fread(temp_buf, 4, 1, fdd);
    fclose(fdd);

    bat_temp = atoi(temp_buf);
    if (bat_temp > 650) {
        *temperature = 650;
    } else if (bat_temp < -200) {
        *temperature = -200;
    } else {
        *temperature = bat_temp;
    }
    ex_log(LOG_DEBUG, "sys temperature = %d", *temperature);
    return 0;
}

long getCurrentTime_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void get_time_now(char* str_time) {
    time_t timep;
    struct tm* p;

    time(&timep);
    p = gmtime(&timep);
    sprintf(str_time, "%02d-%02d-%02d.%02d.%02d.%03d", (1 + p->tm_mon), p->tm_mday, p->tm_hour + 8,
            p->tm_min, p->tm_sec, (int)(getCurrentTime_ms() % 1000));
}

long get_ca_time_ms(void) {
    time_t timep;
    struct tm* p;
    long time_ms;

    time(&timep);
    p = gmtime(&timep);
    time_ms = ((p->tm_hour + 8) * 60 * 60 * 1000) + (p->tm_min * 60 * 1000) + (p->tm_sec * 1000) +
              (getCurrentTime_ms() % 1000);

    return time_ms;
}

static int do_power_on(pwr_status_t pwr) {
    plat_mutex_lock(pwr_lock);
    int ret = FINGERPRINT_RES_SUCCESS;

    ex_log(LOG_DEBUG, "set pwr = %s", pwr == SENSOR_PWR_ON ? "SENSOR_PWR_ON" : "SENSOR_PWR_OFF");

    if (pwr == g_power_managemen.pwr_status) {
        ex_log(LOG_DEBUG, "already %s go exit!!",
        pwr == SENSOR_PWR_ON ? "SENSOR_PWR_ON" : "SENSOR_PWR_OFF");
        ret = FINGERPRINT_RES_SUCCESS;
        goto EXIT;
    }

    if (pwr == SENSOR_PWR_ON) {
        ret = fp_device_power_control(g_hdev, true);
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_power_control on failed");
            goto EXIT;
        }
        setSpiState(TRUE, FALSE);
	fp_device_reset_set(g_hdev, 1);
	fp_device_reset(g_hdev);
	ret = opm_initialize_sensor_hw();
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "opm_initialize_sensor_hw return = %d", ret);
            fp_device_power_control(g_hdev, false);
            setSpiState(FALSE, FALSE);
            goto EXIT;
        }
    } else {
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "opm_uninitialize_sensor_hw return = %d", ret);
            goto EXIT;
        }
        setSpiState(FALSE, FALSE);
	ret = fp_device_reset_set(g_hdev,0);
        ret = fp_device_power_control(g_hdev, false);
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_power_control off failed");
            goto EXIT;
        }
    }
    g_power_managemen.pwr_status = pwr;

EXIT:
    ex_log(LOG_DEBUG, "g_power_managemen.pwr_status = %s",
    g_power_managemen.pwr_status == SENSOR_PWR_ON ? "SENSOR_PWR_ON" : "SENSOR_PWR_OFF");
    plat_mutex_unlock(pwr_lock);
    return ret;
}
