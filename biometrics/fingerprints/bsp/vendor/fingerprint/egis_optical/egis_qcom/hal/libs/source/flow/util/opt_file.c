#include <stdio.h>
#include <sys/stat.h>
#include "egis_definition.h"
#include "opt_file.h"
#include "response_def.h"
#include "plat_log.h"
#include "plat_file.h"
#include "plat_mem.h"
#include "op_manager.h"
#include "core_config.h"
#include "egis_rbs_api.h"
#ifdef __TRUSTONIC__
#include "opt_file_trustonic.h"
#endif

app_instance_type_t g_app_type = APP_IS_BIOMETRIC;

char g_custom_ini_path[PATH_MAX] = {0};
static void _get_ini_config_path(char* path, const char* filename)
{
	if (strlen(g_custom_ini_path) > 0) {
		strncpy(path, g_custom_ini_path, PATH_MAX);
		strcat(path, filename);
		return;
	}
#if defined(ANDROID) || defined(__ANDROID__)
	switch (g_app_type)
	{
		case APP_IS_USING_JNI:
			mkdir("/sdcard/RbsG3Temp", 0700);
			snprintf(path, PATH_MAX, "/sdcard/RbsG3Temp/%s", filename);
			break;

		case APP_IS_USING_BINDER:
		case APP_IS_BIOMETRIC:
			mkdir("/data/vendor/optical_fingerprint", 0700);
                        strcpy(path, "/data/vendor/optical_fingerprint/rbs_config.ini");
                        break;
	}
#else
	strcpy(path, filename);
#endif
}

static int _save_ini_config_file(char* ini_config_path, unsigned char* in_data, int in_data_size, uint8_t** out_buf, unsigned int* real_size) {
	int retval = FINGERPRINT_RES_SUCCESS;
	uint8_t* buf = plat_alloc(INI_CONFING_FILE_MAX_SIZE);
	*out_buf = buf;
	if (buf == NULL) {
		return FINGERPRINT_RES_ALLOC_FAILED;
	}
	memset(buf, 0, INI_CONFING_FILE_MAX_SIZE);
	if (in_data != NULL && in_data_size > 0) {
		if (in_data_size > INI_CONFING_FILE_MAX_SIZE) {
			ex_log(LOG_ERROR, "config string size %d invalid", in_data_size);
			return FINGERPRINT_RES_INVALID_PARAM;
		}

		memcpy(buf, in_data, in_data_size);
		*real_size = in_data_size;
#ifndef RBS_EVTOOL
		int saveSize = plat_save_file(ini_config_path, in_data, in_data_size);
		if (saveSize > 0) {
			ex_log(LOG_DEBUG, "save ini config file: %s OK (%d)", ini_config_path, saveSize);
		} else {
			ex_log(LOG_ERROR, "Failed to save ini config file: %s (%d)", ini_config_path, saveSize);
			return FINGERPRINT_RES_FAILED;
		}
#endif
	} else {
		ex_log(LOG_DEBUG, "Open ini config file: %s", ini_config_path);
		retval = plat_load_file(ini_config_path, buf, INI_CONFING_FILE_MAX_SIZE, real_size);
		if (retval == PLAT_FILE_NOT_EXIST) {
			ex_log(LOG_DEBUG, "skipped ini config");
			return PLAT_FILE_NOT_EXIST;
		} else if (retval <= 0 || *real_size == INI_CONFING_FILE_MAX_SIZE) {
			ex_log(LOG_ERROR, "failed to open ini config file (%d)", retval);
			return FINGERPRINT_RES_FAILED;
		}
	}
	return FINGERPRINT_RES_SUCCESS;
}


int opt_send_data(int type, unsigned char* in_data, int in_data_size)
{
	int retval = FINGERPRINT_RES_SUCCESS;

	switch (type) {
		case TYPE_SEND_CALIBRATION_DATA:
		case TYPE_SEND_TEMPLATE: {
#ifdef __TRUSTONIC__
			retval = trustonic_send_data(type);
#endif
		} break;
		case TYPE_SEND_INI_CONFIG_PATH:
			if (in_data != NULL && in_data_size > 0) {
				ex_log(LOG_DEBUG, "ini config path: %s", in_data);
				strncpy(g_custom_ini_path, (char*)in_data, PATH_MAX);
			} else {
				ex_log(LOG_ERROR, "ini config path is not changed: %s", g_custom_ini_path);
			}
			break;
		case TYPE_SEND_INI_CONFIG: {
#ifdef DISABLE_SEND_INI
			if (in_data_size > 0) {
				retval = FINGERPRINT_RES_FAILED;
			}
#else
			unsigned int real_size = 0;
			char ini_config_path[PATH_MAX];
			_get_ini_config_path(ini_config_path, "rbs_config.ini");
			uint8_t* buf = NULL;
			retval = _save_ini_config_file(ini_config_path, in_data, in_data_size, &buf, &real_size);
			if(retval != FINGERPRINT_RES_SUCCESS || retval == PLAT_FILE_NOT_EXIST) {
				if (retval == PLAT_FILE_NOT_EXIST)
					retval = FINGERPRINT_RES_SUCCESS;
				PLAT_FREE(buf);
				break;
			}
			ex_log(LOG_DEBUG, "send ini config");
			retval = opm_set_data(type, buf, real_size);
			core_config_create(CONFIG_BUF_TYPE_INI, buf, real_size);
			
			PLAT_FREE(buf);
#endif
		} break;
		case TYPE_DESTROY_INI_CONFIG: {
			core_config_destroy();
			retval = opm_set_data(type, NULL, 0);
		}break;
		case TYPE_REMOVE_INI_FILE : {
			char ini_config_path[PATH_MAX];
			_get_ini_config_path(ini_config_path, "rbs_config.ini");
			char backup_ini_config_path[PATH_MAX];
			sprintf(backup_ini_config_path, "%srbs_config_backup.ini", g_custom_ini_path);
			FILE *fptr=fopen(ini_config_path,"r");
			if (fptr) {
				fclose(fptr);
				retval = rename(ini_config_path, backup_ini_config_path);
			} else 
				retval = FINGERPRINT_RES_SUCCESS;
		} break;
		case TYPE_SEND_DB_INI_CONFIG: {
			unsigned int real_size = 0;
			char ini_config_path[PATH_MAX];
			_get_ini_config_path(ini_config_path, "db_info.ini");
			uint8_t* buf = NULL;
			retval = _save_ini_config_file(ini_config_path, in_data, in_data_size, &buf, &real_size);
			if (retval == PLAT_FILE_NOT_EXIST)
				retval = FINGERPRINT_RES_SUCCESS;
			PLAT_FREE(buf);
		} break;
		default: {
			retval = opm_set_data(type, NULL, 0);
		} break;
	}

	return retval;
}

int opt_receive_data(int type, unsigned char* in_data, int in_data_size,
		     unsigned char* out_data, int* out_data_size)
{
	int retval = FINGERPRINT_RES_SUCCESS;

	switch (type) {
		case TYPE_RECEIVE_CALIBRATION_DATA:
		case TYPE_RECEIVE_TEMPLATE:
		case TYPE_DELETE_TEMPLATE: {
#ifdef __TRUSTONIC__
			retval =
			    trustonic_receive_data(type, in_data, in_data_size);
#endif
		} break;

		case TYPE_RECEIVE_IMAGE: {
			if (out_data == NULL || out_data_size == NULL) {
				retval = FINGERPRINT_RES_INVALID_PARAM;
				break;
			}

			retval =
			    opm_get_data(TYPE_RECEIVE_IMAGE, in_data,
					 in_data_size, out_data, out_data_size);
		} break;

		case TYPE_TEST_TRANSPORTER: {
			if (in_data == NULL || in_data_size <= 0 ||
			    out_data == NULL || out_data_size == NULL) {
				retval = FINGERPRINT_RES_INVALID_PARAM;
				break;
			}

			retval = opm_get_data(type, in_data, in_data_size,
					      out_data, out_data_size);
		} break;

		case TYPE_RECEIVE_MULTIPLE_IMAGE: {
			if (in_data == NULL || in_data_size <= 0) {
				retval = FINGERPRINT_RES_INVALID_PARAM;
				break;
			}
			
			retval = opm_get_data(type, in_data, in_data_size, out_data, out_data_size);
		} break;

		case TYPE_RECEIVE_LIVE_IMAGE: {
			 if (in_data == NULL || in_data_size <= 0) {
				 retval = FINGERPRINT_RES_INVALID_PARAM;
				 break;
			 }

			 retval = opm_get_data(type, in_data, in_data_size, out_data, out_data_size);
		} break;

		case TYPE_RECEIVE_INI_CONFIG: {
			if (out_data == NULL || out_data_size == NULL) {
				retval = FINGERPRINT_RES_INVALID_PARAM;
				break;
			}

			core_config_get_buf(CONFIG_BUF_TYPE_INI, out_data, out_data_size);
		} break;

		default: {
			retval = opm_get_data(type, in_data, in_data_size, out_data, out_data_size);
		} break;
	}

	return retval;
}
