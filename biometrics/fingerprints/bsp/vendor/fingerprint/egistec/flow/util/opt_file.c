#include "opt_file.h"
#include "response_def.h"
#include "plat_log.h"
#include "plat_file.h"
#include "plat_mem.h"
#include "op_manager.h"
#include "core_config.h"

#define INI_CONFING_FILE_MAX_SIZE 10*1024
#if defined(ANDROID) || defined(__ANDROID__)
#define INI_CONFIG_FILE_PATH "/data/fpdata/rbs_config.ini"
#else
#define INI_CONFIG_FILE_PATH "rbs_config.ini"
#endif

int opt_send_data(int type, unsigned char* in_data, int in_data_size)
{
	int retval = FINGERPRINT_RES_SUCCESS;

	switch (type) {
		case TYPE_SEND_CALIBRATION_DATA:
		case TYPE_SEND_TEMPLATE:
		case TYPE_SEND_USER_INFO: 
		case TYPE_SEND_PATTERN_INFO:
		case TYPE_UPDATA_PATTERN_INFO_TO_USER: 
		case TYPE_SEND_ENROLL_MASK: {
#ifdef __TRUSTONIC__
			retval = trustonic_send_data(type, in_data, in_data_size);
#endif
		} break;
		case TYPE_SEND_INI_CONFIG: {
			unsigned int real_size = 0;
			uint8_t* buf = plat_alloc(INI_CONFING_FILE_MAX_SIZE);
			if (buf == NULL) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			memset(buf, 0, INI_CONFING_FILE_MAX_SIZE);
			if (in_data != NULL && in_data_size > 0) {
				if (in_data_size > INI_CONFING_FILE_MAX_SIZE) {
					ex_log(LOG_ERROR, "config string size %d invalid", in_data_size);
					retval = FINGERPRINT_RES_INVALID_PARAM;
					PLAT_FREE(buf);
					break;
				}

				memcpy(buf, in_data, in_data_size);
				real_size = in_data_size;
			} else {
				ex_log(LOG_DEBUG, "Open ini config file: %s", INI_CONFIG_FILE_PATH);
				retval = plat_load_file(INI_CONFIG_FILE_PATH, buf, INI_CONFING_FILE_MAX_SIZE, &real_size);
				if (retval == PLAT_FILE_NOT_EXIST) {
					ex_log(LOG_DEBUG, "skipped ini config");
					retval = FINGERPRINT_RES_SUCCESS;
					PLAT_FREE(buf);
					break;
				} else if (retval <= 0 || real_size == INI_CONFING_FILE_MAX_SIZE) {
					ex_log(LOG_ERROR, "failed to open ini config file (%d)", retval);
					retval = FINGERPRINT_RES_FAILED;
					PLAT_FREE(buf);
					break;
				}
			}
			
			ex_log(LOG_DEBUG, "send ini config");
			retval = opm_set_data(type, buf, real_size);
			core_config_create(CONFIG_BUF_TYPE_INI, buf, real_size);
			
			PLAT_FREE(buf);
		} break;
		case TYPE_DESTROY_INI_CONFIG: {
			core_config_destroy();
			retval = opm_set_data(type, NULL, 0);
		}break;
		default: {
			ex_log(LOG_ERROR, "invalid type");
			retval = FINGERPRINT_RES_INVALID_PARAM;
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
		case TYPE_DELETE_TEMPLATE:
		case TYPE_RECEIVE_USER_INFO: 
		case TYPE_RECEIVE_PATTERN_INFO:
		case TYPE_DELETE_PATTERN_INFO:
		case TYPE_RECEIVE_ENROLL_MASK:
		case TYPE_DELETE_ENROLL_MASK: {
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

		case TYPE_RECEIVE_FINGET_LOST_IMAGE: {
			if (in_data == NULL || in_data_size <= 0) {
				 retval = FINGERPRINT_RES_INVALID_PARAM;
				 break;
			}

			retval = opm_get_data(type, in_data, in_data_size, out_data, out_data_size);
		} break;

		default: {
			ex_log(LOG_ERROR, "invalid type");
			retval = FINGERPRINT_RES_INVALID_PARAM;
		} break;
	}

	return retval;
}