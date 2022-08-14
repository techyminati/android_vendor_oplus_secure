#include <stdlib.h>
#include "response_def.h"
#include "op_manager.h"
#include "plat_log.h"
#include "fps_normal.h"
#include "common_definition.h"
#include "fp_definition.h"
#include "op_sensortest.h"
#include "device_int.h"

#ifdef _WINDOWS
#define __unused
#endif

#define FP_SENSORTEST_FINGERON_COUNT 3
#define SENSORTEST_DEFAULT_TIEM_OUT	 6 * 1000
#define SENSOR_NEED_RESET 21
#define PID_SENSORTEST 3

int sensortest_send_cmd(int cmd, unsigned char* out_data, int* out_data_size)
{
	ex_log(LOG_DEBUG, "%s enter!", __func__);
	int retval = FP_LIB_ERROR_GENERAL;
	int in_data[2];
	memset(in_data, 0, 2);
	in_data[0] = cmd;
	int in_data_size = 2;
	//unsigned char out_data[64] = {0};
	//int out_data_size = 64;	

	retval = opm_extra_command(PID_SENSORTEST, (unsigned char*)in_data, in_data_size, out_data, out_data_size);
	ex_log(LOG_DEBUG, "%s opm_extra_command retval = %d", __func__, retval);
	if (cmd == FP_GET_MODULE_INFO || cmd == FP_GET_ALGO_VER || cmd == FP_GET_SENSOR_ID) {
		ex_log(LOG_DEBUG, "Get big data info! cmd: %d, out_data_size:%d", cmd, *out_data_size);
		goto EXIT;
	}

	if(FP_LIB_OK == retval || SENSOR_NEED_RESET == retval) {
		memcpy(&retval , out_data, sizeof(int));
	} else {
		retval = MMI_TEST_FAIL;
	}

EXIT:
	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
	return retval;
}

static int sensortest_check_finger_on(int idev)
{
	int retval = MMI_TEST_FAIL;
	int test_count = 0;
	unsigned char out_data[64];
	int out_data_size = 64;

	retval = sensortest_send_cmd(FP_MMI_FOD_TEST, out_data, &out_data_size);
	if(MMI_TEST_SUCCESS != retval) {
		retval =  MMI_TEST_FAIL;
		goto EXIT;
	}

	retval = FINGERPRINT_RES_TIMEOUT;
	do {
		retval = fp_device_interrupt_enable(idev, FALSE);
		retval = fp_device_interrupt_enable(idev, TRUE);
		retval = fp_device_interrupt_wait(idev, SENSORTEST_DEFAULT_TIEM_OUT);
		test_count++;
		if(FINGERPRINT_RES_SUCCESS == retval) break;
	}while(test_count < FP_SENSORTEST_FINGERON_COUNT);
	
	if(FINGERPRINT_RES_SUCCESS == retval)
	{
		retval = sensortest_send_cmd(FP_MMI_GET_FINGER_IMAGE, out_data, &out_data_size);
		if(MMI_TEST_SUCCESS != retval) 
			retval = MMI_TEST_FAIL;
	} else {
		retval = MMI_TEST_FAIL;
	}

EXIT:
	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
	return retval;
}


static int sensortest_wait_finger_on(__unused int idev,int times_ms)
{
	int retval = MMI_TEST_FAIL;
	int test_count = times_ms/30;
	unsigned char out_data[64];
	int out_data_size = 64;

	retval = sensortest_send_cmd(FP_MMI_FOD_TEST, out_data, &out_data_size);
	if (MMI_TEST_SUCCESS != retval) {
		retval = MMI_TEST_FAIL;
		goto EXIT;
	}

	if (!wait_trigger(test_count, 30))
		retval = MMI_TEST_FAIL;
	else
		retval = MMI_TEST_SUCCESS;

EXIT:
	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
	return retval;
}

static int sensortest_wait_interrupt(trigger_info_t * trigger_info)
{
	int retval = MMI_TEST_FAIL;
	int try_count = trigger_info->wait_time/trigger_info->time_interval;
	fp_set_interrupt_trigger_type(trigger_info->trigger_type);

	if (!wait_trigger(try_count, trigger_info->time_interval))
		retval = MMI_TEST_FAIL;
	else
		retval = MMI_TEST_SUCCESS;

	fp_set_interrupt_trigger_type(TRIGGER_RESET);
	ex_log(LOG_DEBUG, "%s end ,wait result %d ", __func__,retval);
	return retval;
}

static int sensortest_get_image(unsigned char *entry_data, unsigned char *buffer, int *buffer_size)
{
	if(NULL == buffer || NULL == buffer_size) {
		ex_log(LOG_DEBUG, "%s invalid param", __func__);
		return MMI_TEST_FAIL;
	}

	int image_test_type = ((int *)entry_data)[1];
	int retval = FINGERPRINT_RES_FAILED;
	int in_data[2] = {0};
	in_data[0] = FP_CAPTURE_IMG;
	in_data[1] = image_test_type;
	int in_data_size = sizeof(in_data);
	
	retval = opm_extra_command(PID_SENSORTEST, (unsigned char *)in_data, in_data_size, buffer, buffer_size);
	if(FP_LIB_OK == retval) {
		retval = MMI_TEST_SUCCESS;
	} else {
		retval = MMI_TEST_FAIL;
	}

	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return retval;
}

static int sensortest_set_crop_info(unsigned char *entry_data)
{
	int retval = FINGERPRINT_RES_FAILED;
	unsigned char out_data[64] = {0};
	int out_data_size = 64;	
	int in_data[4] = {0};
	in_data[0] = FP_MMI_SET_CROP_INFO;
	in_data[1] =((int *)entry_data)[1];
	in_data[2] =((int *)entry_data)[3];
	in_data[3] =((int *)entry_data)[4];
	int in_data_size = sizeof(in_data);
	
	retval = opm_extra_command(PID_SENSORTEST, (unsigned char *)in_data, in_data_size, out_data, &out_data_size);
	if(FP_LIB_OK == retval) {
		retval = MMI_TEST_SUCCESS;
	} else {
		retval = MMI_TEST_FAIL;
	}

	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return retval;
}

static int sensortest_get_image_snr(int cmd, unsigned char *buffer, int *buffer_size)
{
	int retval = opm_extra_command(PID_SENSORTEST, (unsigned char *)&cmd, sizeof(int), buffer, buffer_size);
	if(FP_LIB_OK == retval) {
		retval = MMI_TEST_SUCCESS;
	} else {
		retval = MMI_TEST_FAIL;
	}

	return retval;
}

static int sensortest_get_nvm_uid(__unused unsigned char *entry_data, unsigned char *buffer, int *buffer_size)
{
	if(NULL == buffer || NULL == buffer_size) {
		ex_log(LOG_DEBUG, "%s invalid param", __func__);
		return MMI_TEST_FAIL;
	}

	__unused int image_test_type = ((int *)entry_data)[1];
	int retval = FINGERPRINT_RES_FAILED;
	int in_data[2] = {0};
	
	memset(in_data, 0, 2);
	in_data[0] = FP_MMI_GET_NVM_UID;
	int in_data_size = sizeof(in_data);
	
	retval = opm_extra_command(PID_SENSORTEST, (unsigned char *)in_data, in_data_size, buffer, buffer_size);
	if(FP_LIB_OK == retval) {
		retval = MMI_TEST_SUCCESS;
	} else {
		retval = MMI_TEST_FAIL;
	}

	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return retval;
}

int sensor_test_opation( int cid, int idev, unsigned char *in_data, __unused int in_data_size,
                             unsigned char *buffer, int *buffer_size)
{
    ex_log(LOG_DEBUG, "%s enter, dev = %d!", __func__, idev);
	int retval = MMI_TEST_FAIL;
	__unused int image_score = 0;
	__unused unsigned char out_data[64] = {0};
	__unused int out_data_size = 64;
	trigger_info_t trigger;
	switch(cid) 
	{
	case SENSORTEST_DIRTYDOTS_TEST:
		retval = sensortest_send_cmd(FP_MMI_DIRTYDOTS_TEST, buffer, buffer_size);
	break;

	case SENSORTEST_READ_REV_TEST:
		retval = sensortest_send_cmd(FP_MMI_READ_REV_TEST, buffer, buffer_size);
	break;
    
	case SENSORTEST_REGISTER_RW_TEST:
	{
		retval = sensortest_send_cmd(FP_MMI_REGISTER_RW_TEST, buffer, buffer_size);
		if( SENSOR_NEED_RESET == retval) {
			fp_device_reset(idev);
			retval = sensortest_send_cmd(FP_MMI_REGISTER_RW_TEST, buffer, buffer_size);
		}
		fp_device_reset(idev);
		retval = sensortest_send_cmd(FP_MMI_REGISTER_RECOVERY, buffer, buffer_size);
	}
	break;

	case SENSORTEST_CHECK_FINGER_ON:
		retval = sensortest_check_finger_on(idev);
		break;
	case SENSORTEST_GET_IMAGE:
		retval = sensortest_get_image(in_data, buffer, buffer_size);
		break;
	case SENSORTEST_WAIT_INTERRUTP:
		{
			__unused int in_0 = ((int *)in_data)[0];
			int in_1 = ((int *)in_data)[1];

			retval = sensortest_wait_finger_on(idev, in_1);
		}
		break;
	case SENSORTEST_START_INTERRUTP:
		retval = sensortest_send_cmd(FP_MMI_FOD_TEST,buffer,buffer_size);
		break;
	case SENSORTEST_TEST_INTERRUTP:
		{
			trigger_info_t * trigger_info = (trigger_info_t *)&(((int *)in_data)[1]);
			
			retval = sensortest_wait_interrupt(trigger_info);
		}
		break;		 
	case SENSORTEST_STOP_INTERRUTP:
			
		break;
	case SENSORTEST_SET_CROP_INFO:
		retval = sensortest_set_crop_info(in_data);
		break;
	case SENSORTEST_GET_NVM_UID:
		retval = sensortest_get_nvm_uid(in_data, buffer, buffer_size);
		break;
	case SENSORTEST_GET_IMAGE_SNR:		
		trigger.time_interval = 30;
		trigger.wait_time = 0;
		trigger.trigger_type = LEVEL_LOW;
		if(sensortest_wait_interrupt(&trigger) == MMI_TEST_SUCCESS) {
			retval = sensortest_get_image_snr(FP_MMI_GET_IMAGE_SNR, buffer, buffer_size);
		}
		break;
	default:
		ex_log(LOG_DEBUG, "sensortest tool unkown cmd");
		break;
	}

	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return MMI_TEST_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS
						 : FINGERPRINT_RES_FAILED;
}
