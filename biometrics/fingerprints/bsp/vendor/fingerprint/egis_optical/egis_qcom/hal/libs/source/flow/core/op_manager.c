#include <string.h>
#include "op_manager.h"
#include "packager.h"
#include "response_def.h"
#include "message_handler.h"
#include "plat_time.h"
#include "plat_log.h"
#include "fp_definition.h"

#define LOG_TAG "RBS-opm"

#ifdef _WINDOWS
#define __unused
#endif

#ifdef SIMULATE_NO_SENSOR
#include "finger_image_db.h"
#include "egis_io.h"
#include "constant_def.h"
#include "egis_sensormodule.h"
#include "testdb_cb.h"

static int g_db_fimage_type = 0;
static int g_db_enroll_finger_idx = -1;
static int g_db_verify_finger_idx = 0;
static int g_db_max_finger_idx = 0;
static int g_db_fingerpirnt_idx = 0;
static int g_db_total_fingerprint_for_this_finger = 0;
static char *image_path = "/sdcard/FP1";
#endif

int opm_initialize_sdk(unsigned char *in_data, unsigned int in_data_size)
{
	unsigned int length = 0;

	return transfer_data(OPERATION_TYPE, EX_CMD_INIT_SDK, 0, 0,
			     in_data_size, in_data, &length, NULL);
}

int opm_initialize_algo()
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_INIT_ALGO, 0, 0, 0, NULL,
			     &length, NULL);
}

int opm_initialize_sensor()
{
	unsigned int length = 0;
#ifdef SIMULATE_NO_SENSOR
	egis_initFakeSensor(70, 57);
	testdb_set_load_image_fn(loadImageFromFile);
	if (!fimage_db_is_created()) {
		fimage_db_create(image_path, DB_EVTOOL);
	}

	g_db_max_finger_idx = fimage_db_get_num_finger() - 1;
	return transfer_data(OPERATION_TYPE, EX_CMD_INIT_SENSOR, 70, 57, 0, NULL, &length, NULL);
#endif
	return transfer_data(OPERATION_TYPE, EX_CMD_INIT_SENSOR, 0, 0, 0, NULL,
			     &length, NULL);
}
int opm_uninitialize_sdk()
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_UNINIT_SDK, 0, 0, 0, NULL,
			     &length, NULL);
}

int opm_uninitialize_algo()
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_UNINIT_ALGO, 0, 0, 0, NULL,
			     &length, NULL);
}

int opm_uninitialize_sensor()
{
	unsigned int length = 0;
#ifdef SIMULATE_NO_SENSOR
    fimage_db_destroy();
#endif
	return transfer_data(OPERATION_TYPE, EX_CMD_UNINIT_SENSOR, 0, 0, 0,
			     NULL, &length, NULL);
}

int opm_set_active_group(unsigned int user_id, const char *data_path)
{
	unsigned int length = 0;
	unsigned int out_len = 0;

	if (NULL != data_path) {
		length = strlen(data_path);
	}

	return transfer_data(OPERATION_TYPE, EX_CMD_SET_ACTIVE_USER, user_id, 0,
			     length, (unsigned char *)data_path, &out_len,
			     NULL);
}

int opm_set_data_path(unsigned int data_type, const char *data_path,
		      unsigned int path_len)
{
	unsigned int length = 0;

	return transfer_data(OPERATION_TYPE, EX_CMD_SET_DATA_PATH, data_type, 0,
			     path_len, (unsigned char *)data_path, &length,
			     NULL);
}

int opm_set_work_mode(unsigned int mode)
{
#ifdef SIMULATE_NO_SENSOR
	return 0;
#endif
	unsigned int length = 0;

	return transfer_data(OPERATION_TYPE, EX_CMD_SET_WORK_MODE, mode, 0, 0,
			     NULL, &length, NULL);
}

int opm_calibration(int status, int type, unsigned char *calibration_data,
		    int data_size)
{
#ifdef SIMULATE_NO_SENSOR
	return 0;
#endif
	unsigned int length = sizeof(int);
	unsigned char sta = (unsigned char)status;

	return transfer_data(type, EX_CMD_CALIBRATION, status, 0, data_size,
			     calibration_data, &length, &sta);
}

int opm_get_fingerprint_ids(unsigned int user_id, fingerprint_ids_t *fps)
{
	unsigned int length = sizeof(fingerprint_ids_t);

	return transfer_data(OPERATION_TYPE, EX_CMD_GET_FP_IDS, user_id, 0, 0,
			     NULL, &length, (unsigned char *)fps);
}

int opm_remove_fingerprint(fingerprint_remove_info_t remove_info)
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_REMOVE,
			     remove_info.fingerprint_info.user_id,
			     remove_info.fingerprint_info.fingerprint_id, 0,
			     NULL, &length, NULL);
}

int opm_open_spi()
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_OPEN_SPI, 0, 0, 0, NULL, &length, NULL);
}

int opm_close_spi()
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_CLOSE_SPI, 0, 0, 0, NULL, &length, NULL);
}

int opm_get_image(unsigned int *quality, host_device_info_t __unused info)
{
	unsigned int len = sizeof(cmd_imagestatus_t);
	int retval;
	cmd_imagestatus_t image_status;

#ifdef SIMULATE_NO_SENSOR
	int finger_idx = g_db_fimage_type == FIMAGE_ENROLL ? g_db_enroll_finger_idx : g_db_verify_finger_idx;
	if (g_db_fingerpirnt_idx < g_db_total_fingerprint_for_this_finger)
		testdb_set_read_param(g_db_fimage_type, finger_idx, g_db_fingerpirnt_idx++, TRUE);
	else g_db_fingerpirnt_idx = 0;
	ex_log(LOG_DEBUG, "opm_get_image: fimage_type = %d; fingerprint_idx = %d\n", g_db_fimage_type, g_db_fingerpirnt_idx - 1);
#endif
	TIME_MEASURE_START(opm_get_image);
#ifdef SEND_HOST_TEMPERATURE_INFO
	unsigned int length = 0;
	retval = transfer_data(OPERATION_TYPE, EX_CMD_SET_DATA, TYPE_SEND_HOST_DEVICE_INFO, 0, sizeof(host_device_info_t), (unsigned char *)&info,
			       &length, NULL);
#endif
	retval = transfer_data(OPERATION_TYPE, EX_CMD_GET_IMAGE, 0, 0, 0, NULL,
			       &len, (unsigned char *)&image_status);

	if (0 == retval) {
		*quality = image_status.status;
	}

	TIME_MEASURE_STOP(opm_get_image, "Get image total");
	return retval;
}

int opm_check_finger_lost(int timeout, unsigned int *status)
{
#if defined(__ET0XX__) || defined(SIMULATE_NO_SENSOR)
	*status = FP_LIB_FINGER_LOST;
	return 0;
#endif
	int retval;
	unsigned int len = sizeof(unsigned int);

	TIME_MEASURE_START(opm_check_finger_lost);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_CHECK_FINGER_LOST, timeout,
			     0, 0, NULL, (unsigned int *)&len,
			     (unsigned char *)status);

	TIME_MEASURE_STOP(opm_check_finger_lost, "opm_check_finger_lost");
	return retval;
}

int opm_get_enrolled_count(unsigned int *count)
{
	unsigned int value;
	unsigned int len = sizeof(unsigned int);
	int retval;

	if (count == NULL) return -1;

	TIME_MEASURE_START(opm_get_enrolled_count);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_GET_ENROLLED_COUNT, 0, 0,
			       0, NULL, &len, (unsigned char *)&value);
	if (retval == 0) {
		*count = value;
	} else {
		*count = 0;
	}

	TIME_MEASURE_STOP(opm_get_enrolled_count, "opm_get_enrolled_count");
	return retval;
}

int opm_enroll_initialize()
{
#ifdef SIMULATE_NO_SENSOR
	g_db_fimage_type = FIMAGE_ENROLL;
	g_db_enroll_finger_idx++;
	g_db_fingerpirnt_idx = 0;
	g_db_total_fingerprint_for_this_finger = fimage_db_get_num_fingerprint(g_db_fimage_type, g_db_enroll_finger_idx);
#endif
	int retval;
	unsigned int length = 0;

	TIME_MEASURE_START(opm_enroll_initialize);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_ENROLL_INIT, 0, 0, 0, NULL,
			       &length, NULL);

	TIME_MEASURE_STOP(opm_enroll_initialize, "opm_enroll_initialize");
	return retval;
}

int opm_do_enroll(cmd_enrollresult_t * enroll_result, int enroll_option, int is_identify_count)
{
	unsigned int len = sizeof(cmd_enrollresult_t);
	int retval;

	TIME_MEASURE_START(opm_do_enroll);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_ENROLL, enroll_option, is_identify_count, 0, NULL,
		&len, (unsigned char *)enroll_result);

	TIME_MEASURE_STOP(opm_do_enroll, "opm_do_enroll");
	return retval;
}

int opm_save_enrolled_fingerprint(fingerprint_enroll_info_t enroll_info)
{
	unsigned int length = 0;

	return transfer_data(OPERATION_TYPE, EX_CMD_ENROLL_SAVE_TEMPLATE,
			     enroll_info.fingerprint_info.user_id,
			     enroll_info.fingerprint_info.fingerprint_id, 0,
			     NULL, &length, NULL);
}

int opm_enroll_uninitialize()
{
#ifdef SIMULATE_NO_SENSOR
	if (g_db_enroll_finger_idx == g_db_max_finger_idx) {
		g_db_enroll_finger_idx = -1;
	}
	g_db_fingerpirnt_idx = 0;
#endif
	unsigned int length = 0;
	int retval;

	TIME_MEASURE_START(opm_enroll_uninitialize);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_ENROLL_UNINIT, 0, 0, 0,
			       NULL, &length, NULL);

	TIME_MEASURE_STOP(opm_enroll_uninitialize, "opm_enroll_uninitialize");
	return retval;
}

int opm_chk_secure_id(unsigned int user_id, unsigned long long secure_id)
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_CHECK_SID, user_id, 0,
			     sizeof(unsigned long long),
			     (unsigned char *)&secure_id, &length, NULL);
}

int opm_chk_auth_token(unsigned char *token, unsigned int len)
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_CHECK_AUTH_TOKEN, 0, 0, len,
			     token, &length, NULL);
}

int opm_get_authenticator_id(unsigned long long *id)
{
	int retval;
	unsigned long long authenticator_id = 0;
	unsigned int out_data_len = sizeof(unsigned long long);

	*id = 0;
	retval =
	    transfer_data(OPERATION_TYPE, EX_CMD_GET_AUTH_ID, 0, 0, 0, NULL,
			  &out_data_len, (unsigned char *)&authenticator_id);
	if (0 == retval) {
		*id = authenticator_id;
	}
	return retval;
}

int opm_identify_start(unsigned int need_liveness_authentication)
{
#ifdef SIMULATE_NO_SENSOR
	g_db_fimage_type = FIMAGE_VERIFY;
	if (g_db_fingerpirnt_idx == g_db_total_fingerprint_for_this_finger) {
		g_db_fingerpirnt_idx = 0;
		g_db_verify_finger_idx++;
	}
	g_db_total_fingerprint_for_this_finger = fimage_db_get_num_fingerprint(g_db_fimage_type, g_db_verify_finger_idx);
#endif
	unsigned int length = 0;
	int retval;

	TIME_MEASURE_START(opm_identify_start);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_IDENTIFY_START,
			       need_liveness_authentication, 0, 0, NULL, &length,
			       NULL);

	TIME_MEASURE_STOP(opm_identify_start, "opm_identify_start");
	return retval;
}

int opm_identify_finish()
{
#ifdef SIMULATE_NO_SENSOR
	if (g_db_verify_finger_idx == g_db_max_finger_idx && g_db_fingerpirnt_idx == g_db_total_fingerprint_for_this_finger) {
		g_db_verify_finger_idx = 0;
	}
#endif
	unsigned int length = 0;
	int retval;

	TIME_MEASURE_START(opm_identify_finish);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_IDENTIFY_FINISH, 0, 0, 0,
			       NULL, &length, NULL);

	TIME_MEASURE_STOP(opm_identify_finish, "opm_identify_finish");
	return retval;
}

int opm_identify_template_update(unsigned char *is_update)
{
	int retval;
	unsigned char value;
	unsigned int len = sizeof(value);

	*is_update = 0;
	TIME_MEASURE_START(opm_identify_template_update);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_IDENTIFY_TMPL_UPDATE, 0,
			       0, 0, NULL, &len, &value);
	if (0 == retval) {
		*is_update = value;
	}

	TIME_MEASURE_STOP(opm_identify_template_update, "opm_identify_template_update");
	return retval;
}

int opm_identify_template_save()
{
	int retval;
	unsigned int len = 0;

	TIME_MEASURE_START(opm_identify_template_save);
	retval = transfer_data(OPERATION_TYPE, EX_CMD_IDENTIFY_TMPL_SAVE, 0, 0,
			       0, NULL, &len, NULL);

	TIME_MEASURE_STOP(opm_identify_template_save, "opm_identify_template_save");
	return retval;
}

int opm_identify(fingerprint_verify_info_t *verify_info, unsigned int *match_id,
		 unsigned int *status, unsigned char *out_token,
		 unsigned int *out_token_size, int *ext_feat_quality)
{
#define AUTH_TOKEN_MAX_LEN 512
#define AUTH_TOKEN_ENABLE 1
#define AUTH_TOKEN_DISABLE 0

	int retval;
	unsigned char out_data[AUTH_TOKEN_MAX_LEN] = {0};
	uint32_t *value = (uint32_t *)out_data;
	unsigned int out_data_len = AUTH_TOKEN_MAX_LEN;
	unsigned int token_len = 0;
	unsigned int auth_token_enable = AUTH_TOKEN_ENABLE;

	if (NULL == out_token || NULL == out_token_size ||
	    *out_token_size <= 0) {
		auth_token_enable = AUTH_TOKEN_DISABLE;
	}

	TIME_MEASURE_START(opm_identify);
	retval = transfer_data(
	    OPERATION_TYPE, EX_CMD_IDENTIFY, verify_info->user_id,
	    auth_token_enable, sizeof(verify_info->challenge),
	    (unsigned char *)&verify_info->challenge, &out_data_len, out_data);

	if (0 == retval) {
		*match_id = value[0];
		*status = value[1];
		*ext_feat_quality = value[3];

		if (AUTH_TOKEN_ENABLE == auth_token_enable && out_data_len > IDENTIFYRESULT_X_DATA_OFFSET) {
			token_len = value[4];
			egislog_d("opm_identify token_len = %d", token_len);
			if (token_len <= *out_token_size && token_len != 0) {
				egislog_d("before opm_identify memcpy");
				memcpy(out_token, &out_data[IDENTIFYRESULT_X_DATA_OFFSET], token_len);
				egislog_d("after opm_identify memcpy");
			}
		}
	}

	if (out_token_size) {
		*out_token_size = token_len;
	}

	TIME_MEASURE_STOP(opm_identify, "opm_identify");
	return retval;
}

int opm_get_navi_event(unsigned char *out_data, int *out_data_size)
{
	return transfer_data(OPERATION_TYPE, EX_CMD_NAVIGATION, 0, 0, 0, NULL,
			     (unsigned int *)out_data_size, out_data);
}

int opm_set_data(int file_type, unsigned char *data, int data_size)
{
	unsigned int length = 0;
	return transfer_data(OPERATION_TYPE, EX_CMD_SET_DATA, file_type, 0,
			     data_size, data, &length, NULL);
}

int opm_get_data(int file_type, unsigned char *in_data, int in_data_size,
		 unsigned char *out_data, int *out_data_size)
{
	return transfer_data(OPERATION_TYPE, EX_CMD_GET_DATA, file_type, 0,
			     in_data_size, in_data,
			     (unsigned int *)out_data_size, out_data);
}

int opm_extra_command(int type, unsigned char *in_data, int in_data_size,
		      unsigned char *out_buffer, int *out_buffer_size)
{
	return transfer_data(type, EX_CMD_EXTRA, 0, 0, in_data_size, in_data,
			     (unsigned int *)out_buffer_size, out_buffer);
}

int opm_navi_control(int type, unsigned char *in_data, int in_data_size,
	unsigned char *out_buffer, int *out_buffer_size)
{
	return transfer_data(type, EX_CMD_NAVI_CONTROL, 0, 0, in_data_size, in_data,
		(unsigned int *)out_buffer_size, out_buffer);
}

int opm_general_command(int type, int cmd, unsigned char *in_data,
			int in_data_size, unsigned char *out_buffer,
			int *out_buffer_size)
{
	return transfer_data(type, cmd, 0, 0, in_data_size, in_data,
			     (unsigned int *)out_buffer_size, out_buffer);
}
int opm_initialize_sensor_hw() {
    unsigned int length = 0;
    return transfer_data(OPERATION_TYPE, EX_CMD_INIT_SENSOR_HW, 0, 0, 0, NULL, &length, NULL);
}
