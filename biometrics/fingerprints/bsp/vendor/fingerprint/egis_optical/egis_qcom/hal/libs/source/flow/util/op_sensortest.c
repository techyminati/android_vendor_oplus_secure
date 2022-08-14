#include <stdlib.h>
#include "response_def.h"
#include "op_manager.h"
#include "plat_mem.h"
#include "plat_log.h"
#include "fps_normal.h"
#include "common_definition.h"
#include "fp_definition.h"
#include "op_sensortest.h"
#include "device_int.h"
#include "plat_file.h"
#include "plat_time.h"
#if defined(__ET7XX__) && !defined(__ET0XX__)
#include "7xx_sensor_test_definition.h"
#include "captain.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
extern event_callbck_t g_event_callback;
#endif
#define FP_SENSORTEST_FINGERON_COUNT 3
#define SENSORTEST_DEFAULT_TIEM_OUT	 6 * 1000
#define SENSOR_NEED_RESET 21
#define LITTLE_TO_BIG_ENDIAN(x) (((x >> 8) & 0xFF) + ((x << 8) & 0xFF00))

extern int g_hdev;
float g_inline_siganl = 0;
float g_inline_noise = 0;
float g_inline_snr = 0;
char g_UID[9];
float g_expo_step1 = 0;

#ifdef __INLINE_SAVE_LOG__
struct et7xx_inline_lmage_path {
char *lmage_path[INLINE_IMAGE_NUMBER];
};

struct et7xx_inline_normalscan_lmage_path {
char *lmage_path[1];
};

struct et7xx_inline_snr_lmage_path {
char *lmage_path[INLINE_IMAGE_SNR_NUMBER];
};

struct et7xx_inline_bp_lmage_path {
char *lmage_path[INLINE_IMAGE_BP_NUMBER];
};

struct et7xx_inline_snrct_lmage_path {
char *lmage_path[1];
};

struct et7xx_inline_expstep1_lmage_path {
char *lmage_path[INLINE_IMAGE_EXPSTEP1_NUMBER];
};

struct et7xx_inline_expstep2_lmage_path {
char *lmage_path[INLINE_IMAGE_EXPSTEP2_NUMBER];
};

struct et7xx_inline_lmage_path inline_image_path = {
 .lmage_path[0] = SNR_BK_LOG_DBG,
 .lmage_path[1] = MF_BK_LOG_DBG,
 .lmage_path[2] = MF_CT_LOG_DBG,
 .lmage_path[3] = FEA_BK_LOG_DBG,
 .lmage_path[4] = SP_BK_LOG_DBG,
};


struct et7xx_inline_snr_lmage_path inline_snr_image_path = {
 .lmage_path[0] = SNR_CT_LOG_DBG_1,
 .lmage_path[1] = SNR_CT_LOG_DBG_2,
 .lmage_path[2] = SNR_CT_LOG_DBG_3,
 .lmage_path[3] = SNR_CT_LOG_DBG_4,
 .lmage_path[4] = SNR_CT_LOG_DBG_5,
 .lmage_path[5] = SNR_CT_LOG_DBG_6,
 .lmage_path[6] = SNR_CT_LOG_DBG_7,
 .lmage_path[7] = SNR_CT_LOG_DBG_8,
 .lmage_path[8] = SNR_CT_LOG_DBG_9,
 .lmage_path[9] = SNR_CT_LOG_DBG_10,
};

struct et7xx_inline_bp_lmage_path inline_bp_image_path = {
 .lmage_path[0] = BP_CT_LOG_DBG_1,
 .lmage_path[1] = BP_CT_LOG_DBG_2,
 .lmage_path[2] = BP_CT_LOG_DBG_3,
 .lmage_path[3] = BP_CT_LOG_DBG_4,
 .lmage_path[4] = BP_CT_LOG_DBG_5,
 .lmage_path[5] = BP_CT_LOG_DBG_6,
 .lmage_path[6] = BP_CT_LOG_DBG_7,
 .lmage_path[7] = BP_CT_LOG_DBG_8,
 .lmage_path[8] = BP_CT_LOG_DBG_9,
};


struct et7xx_inline_snrct_lmage_path inline_snrct_image_path = {
 .lmage_path[0] = SBR_CT8_LOG_DBG
};

struct et7xx_inline_normalscan_lmage_path inline_image_normal_path = {
 .lmage_path[0] = TPT_CT_LOG_DBG
};

struct et7xx_inline_expstep1_lmage_path inline_expstep1_image_path = {
 .lmage_path[0] = EXP1_CT_LOG_DBG_1,
 .lmage_path[1] = EXP1_CT_LOG_DBG_2,
 .lmage_path[2] = EXP1_CT_LOG_DBG_3,
 .lmage_path[3] = EXP1_CT_LOG_DBG_4,
 .lmage_path[4] = EXP1_CT_LOG_DBG_5,
 .lmage_path[5] = EXP1_CT_LOG_DBG_6,
 .lmage_path[6] = EXP1_CT_LOG_DBG_7,
 .lmage_path[7] = EXP1_CT_LOG_DBG_8,
 .lmage_path[8] = EXP1_CT_LOG_DBG_9,
 .lmage_path[9] = EXP1_CT_LOG_DBG_10,
 .lmage_path[10] = EXP1_CT_LOG_DBG_11,
 .lmage_path[11] = EXP1_CT_LOG_DBG_12,
 .lmage_path[12] = EXP1_CT_LOG_DBG_13,
 .lmage_path[13] = EXP1_CT_LOG_DBG_14,
 .lmage_path[14] = EXP1_CT_LOG_DBG_15,
 .lmage_path[15] = EXP1_CT_LOG_DBG_16,
};

struct et7xx_inline_expstep2_lmage_path inline_expstep2_image_path = {
    .lmage_path[0] = EXP2_CT_LOG_DBG,
    .lmage_path[1] = EXP2_CT_LOG_DBG,
    .lmage_path[2] = EXP2_CT_LOG_DBG,
    .lmage_path[3] = EXP2_CT_LOG_DBG,
    .lmage_path[4] = EXP2_CT_LOG_DBG,
    .lmage_path[5] = EXP2_CT_LOG_DBG,
    .lmage_path[6] = EXP2_CT_LOG_DBG,
    .lmage_path[7] = EXP2_CT_LOG_DBG,
    .lmage_path[8] = EXP2_CT_LOG_DBG,
    .lmage_path[9] = EXP2_CT_LOG_DBG,
    .lmage_path[10] = EXP2_CT_LOG_DBG,
    .lmage_path[11] = EXP2_CT_LOG_DBG,
    .lmage_path[12] = EXP2_CT_LOG_DBG,
    .lmage_path[13] = EXP2_CT_LOG_DBG,
    .lmage_path[14] = EXP2_CT_LOG_DBG,
    .lmage_path[15] = EXP2_CT_LOG_DBG,
    .lmage_path[16] = EXP2_CT_LOG_DBG,
    .lmage_path[17] = EXP2_CT_LOG_DBG,
    .lmage_path[18] = EXP2_CT_LOG_DBG,
    .lmage_path[19] = EXP2_CT_LOG_DBG,
    .lmage_path[20] = EXP2_CT_LOG_DBG,
};
#endif

static int sensortest_send_cmd(int cmd)
{
	ex_log(LOG_DEBUG, "%s enter!", __func__);
	int retval = FP_LIB_ERROR_GENERAL;
	int in_data[2];
	memset(in_data, 0, 2);
	in_data[0] = cmd;
	int in_data_size = 2;
	unsigned char out_data[64] = {0};
	int out_data_size = 64;

	retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, out_data, &out_data_size);
	ex_log(LOG_DEBUG, "%s opm_extra_command retval = %d", __func__, retval);
	if(FP_LIB_OK == retval || SENSOR_NEED_RESET == retval) {
		memcpy(&retval , out_data, sizeof(int));
	} else {
		retval = MMI_TEST_FAIL;
	}
	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
	return retval;
}

static int sensortest_check_finger_on(int idev)
{
	int retval = MMI_TEST_FAIL;
	int test_count = 0;

	retval = sensortest_send_cmd(FP_MMI_FOD_TEST);
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
		retval = sensortest_send_cmd(FP_MMI_GET_FINGER_IMAGE);
		if(MMI_TEST_SUCCESS != retval)
			retval = MMI_TEST_FAIL;
	} else {
		retval = MMI_TEST_FAIL;
	}

EXIT:
	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
	return retval;
}


static int sensortest_wait_finger_on(int times_ms)
{
	int retval = MMI_TEST_FAIL;
	int test_count = times_ms/30;

	retval = sensortest_send_cmd(FP_MMI_FOD_TEST);
	if (MMI_TEST_SUCCESS != retval) {
		retval = MMI_TEST_FAIL;
		goto EXIT;
	}

	if (!wait_trigger(test_count, 30, TIMEOUT_WAIT_FOREVER))
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
	int try_count = 0;
	int trigger_type = TRIGGER_RESET;
	int time_interval = 30;

	if (trigger_info != NULL){
		try_count = trigger_info->wait_time/trigger_info->time_interval;
		trigger_type = trigger_info->trigger_type;
		time_interval = trigger_info->time_interval > 0 ? trigger_info->time_interval : 30;
	}
	ex_log(LOG_DEBUG, "%s start,try_count %d,type %d ,time_interval %d ", __func__,retval,trigger_type,time_interval);

	fp_set_interrupt_trigger_type(trigger_type);

	if (!wait_trigger(try_count, time_interval, TIMEOUT_WAIT_FOREVER))
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

	retval = opm_extra_command(PID_INLINETOOL, (unsigned char *)in_data, in_data_size, buffer, buffer_size);
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

	retval = opm_extra_command(PID_INLINETOOL, (unsigned char *)in_data, in_data_size, out_data, &out_data_size);
	if(FP_LIB_OK == retval) {
		retval = MMI_TEST_SUCCESS;
	} else {
		retval = MMI_TEST_FAIL;
	}

	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return retval;
}

static int sensortest_get_nvm_uid(unsigned char *buffer, int *buffer_size)
{
	if(NULL == buffer || NULL == buffer_size) {
		ex_log(LOG_DEBUG, "%s invalid param", __func__);
		return MMI_TEST_FAIL;
	}

	int retval = FINGERPRINT_RES_FAILED;
	int in_data[2] = {0};

	memset(in_data, 0, 2);
	in_data[0] = FP_MMI_GET_NVM_UID;
	int in_data_size = sizeof(in_data);

	retval = opm_extra_command(PID_INLINETOOL, (unsigned char *)in_data, in_data_size, buffer, buffer_size);
	if(FP_LIB_OK == retval) {
		retval = MMI_TEST_SUCCESS;
	} else {
		retval = MMI_TEST_FAIL;
	}

	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return retval;
}

int sensor_test_opation( int cid, int idev, unsigned char *in_data, int in_data_size,
                             unsigned char *buffer, int *buffer_size)
{
    ex_log(LOG_DEBUG, "%s enter, dev = %d, in_data_size %d", __func__, idev, in_data_size);
	int retval = MMI_TEST_FAIL;
	switch(cid)
	{
	case SENSORTEST_DIRTYDOTS_TEST:
		retval = sensortest_send_cmd(FP_MMI_DIRTYDOTS_TEST);
	break;

	case SENSORTEST_READ_REV_TEST:
		retval = sensortest_send_cmd(FP_MMI_READ_REV_TEST);
	break;

	case SENSORTEST_REGISTER_RW_TEST:
	{
		retval = sensortest_send_cmd(FP_MMI_REGISTER_RW_TEST);
		if( SENSOR_NEED_RESET == retval) {
			retval = fp_device_reset(idev);
			retval = sensortest_send_cmd(FP_MMI_REGISTER_RW_TEST);
		}
		retval = fp_device_reset(idev);
		retval = sensortest_send_cmd(FP_MMI_REGISTER_RECOVERY);
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
			// int in_0 = ((int *)in_data)[0];
			int in_1 = ((int *)in_data)[1];

			retval = sensortest_wait_finger_on(in_1);
		}
		break;
	case SENSORTEST_START_INTERRUTP:
		retval = sensortest_send_cmd(FP_MMI_FOD_TEST);
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
		retval = sensortest_get_nvm_uid(buffer, buffer_size);
		break;

	default:
		ex_log(LOG_DEBUG, "sensortest tool unkown cmd");
		break;
	}

	ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

	return MMI_TEST_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS
						 : FINGERPRINT_RES_FAILED;
}

int flow_inline_legacy_cmd(int cmd, int param1, int param2, int param3, unsigned char *out_buf, int *out_size)
{
	int retval = FP_LIB_ERROR_GENERAL;
	int in_data_size;
	int in_data[4];
	in_data[0] = cmd;
	in_data[1] = param1;
	in_data[2] = param2;
	in_data[3] = param3;
	unsigned char *out_data = NULL;
	int out_data_size;
	in_data_size = sizeof(in_data);
	unsigned long long time_diff =0;

	ex_log(LOG_DEBUG, "%s [%d] %d, %d, %d", __func__, cmd, param1, param2, param3);

	if (out_buf == NULL || out_size == NULL) {
        ex_log(LOG_DEBUG, "flow_inline_legacy_cmd out_data = plat_alloc(4);");
		out_data = plat_alloc(4);
		out_data_size = 4;
	} else {
		ex_log(LOG_DEBUG, "%s, out_size=%d", __func__, *out_size);
		out_data = out_buf;
		out_data_size = *out_size;
	}
        unsigned long long time_start= plat_get_time();
	ex_log(LOG_DEBUG, "flow_inline_legacy_cmd in");
	if (cmd == FP_INLINE_7XX_SNR_BKBOX_ON || cmd == FP_INLINE_7XX_SNR_BKBOX_ON_2 || cmd == FP_INLINE_7XX_FLASH_TEST || cmd == FP_INLINE_7XX_FLASH_TEST_2) {
		plat_sleep_time(50);
	}
	retval = opm_extra_command(PID_INLINETOOL, (unsigned char *)in_data, in_data_size, out_data, &out_data_size);
	time_diff = plat_get_diff_time(time_start);
	ex_log(LOG_DEBUG, "flow_inline_legacy_cmd out");
	ex_log(LOG_DEBUG, "flow_inline_legacy_cmd out CMD = %d spent %d ms\r\n", cmd, time_diff);

	if (FP_LIB_OK != retval) {
		ex_log(LOG_ERROR, "%s, retval = %d", __func__, retval);
	}
	ex_log(LOG_DEBUG, "%s, retval = %d", __func__, retval);
	if ((out_size != NULL) && (*out_size >= out_data_size)) {
		*out_size = out_data_size;
	}
	if (out_data != out_buf) {
		PLAT_FREE(out_data);
	}
	return retval;
}
#if defined(__ET7XX__) && !defined(__ET0XX__)

#define FACTORY_TEST_EVT_SNSR_TEST_SCRIPT_START 0x11000003
/* Output event to java AP. It means test is start. */
#define FACTORY_TEST_EVT_SNSR_TEST_SCRIPT_END 0x11000004
/* Output event to java AP. It means test is end. */
#define FACTORY_TEST_EVT_SNSR_TEST_PUT_WKBOX 0x11000006
#define FACTORY_TEST_EVT_SNSR_TEST_PUT_BKBOX 0x11000007
#define FACTORY_TEST_EVT_SNSR_TEST_PUT_CHART 0x11000008
/* Output event to java AP. It means put the robber on sensor. */
#define EVT_ERROR 2020
//Copy from samsung jni\common\platform\inc\egis_definition.h

/* Output event to java AP. It means test is error. */

//static void notify_callback(int event_id) int first_param, int second_param, unsigned char *data, int data_size)
static void notify_callback(int event_id, int first_param, int second_param, unsigned char *data, int data_size)
{
	ex_log(LOG_DEBUG, "%s, event_id = %d", __func__, event_id);

	if (NULL != g_event_callback) {
#ifndef EGIS_DBG
		if (event_id == EVENT_RETURN_IMAGE || event_id ==  EVENT_RETURN_LIVE_IMAGE)
			return;
#endif
		g_event_callback(event_id, first_param, second_param, data, data_size);
#if 0
		if (event_id == EVENT_ENROLL_OK) {
			int remaining = enroll_percentage_to_remaining(second_param);
			g_event_callback(EVENT_ENROLL_REMAINING, first_param, remaining, data, data_size);
		}
#endif
	}
#ifdef __ENABLE_NAVIGATION__
	if (event_id == EVENT_NAVIGATION) send_navi_event_to_driver(first_param);
#endif
}

static void __send_7XX_sensortest_event(int cmd, int errorcode)
{
	ex_log(LOG_INFO, "event_id = %d", cmd);
	/*
	switch (event_id)
	{
		case FP_INLINE_7XX_SNR_INIT:
			notify_callback(FACTORY_TEST_EVT_SNSR_TEST_PUT_WKBOX, 0, 0, NULL, 0);
			break;
		case FP_INLINE_7XX_SNR_WKBOX_ON:
			notify_callback(FACTORY_TEST_EVT_SNSR_TEST_PUT_BKBOX, 0, 0, NULL, 0);
			break;
		case FP_INLINE_7XX_SNR_BKBOX_ON:
			notify_callback(FACTORY_TEST_EVT_SNSR_TEST_PUT_CHART, 0, 0, NULL, 0);
			break;
		case FP_INLINE_7XX_SNR_CHART_ON:
		case FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID:
	    case FP_INLINE_7XX_TEST_MFACTOR:
			notify_callback(FACTORY_TEST_EVT_SNSR_TEST_SCRIPT_END, 0, 0, NULL, 0);
			break;
		case EVT_ERROR:
			notify_callback(EVT_ERROR, 0, 0, NULL, 0);
			break;
		default:
			notify_callback(EVT_ERROR, 0, 0, NULL, 0);
			break;
	}
	*/

	switch (cmd)
	{
		case FP_INLINE_7XX_SNR_WKBOX_ON:
			cmd = 1;
			break;
		case FP_INLINE_7XX_SNR_BKBOX_ON:
			cmd = 2;
			break;
		case FP_INLINE_7XX_FLASH_TEST:
			cmd = 3;
			break;
		case FP_INLINE_7XX_SNR_CHART_ON:
			cmd = 257;
			break;
		case EVT_ERROR:
			break;

	    case FP_INLINE_7XX_TEST_MFACTOR:
		case FP_INLINE_7XX_SNR_INIT:
		default:
			return;
	}

	cmd_test_result_t notify_result = {};
	notify_result.test_result_data[0] = 100;
	notify_result.test_result_data[1] = errorcode;
	notify_result.test_result_data[2] = 101;
	notify_result.test_result_data[3] = g_inline_siganl;
	notify_result.test_result_data[4] = 102;
	notify_result.test_result_data[5] = g_inline_noise;
	notify_result.test_result_data[6] = 103;
	notify_result.test_result_data[7] = g_inline_snr;


	notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, cmd, 32, (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
}

static void init_7xx_test(int *result)
{
	int ret = FP_LIB_ERROR_GENERAL, len=sizeof(int);

	*result = 0;
	ret = flow_inline_legacy_cmd(FP_INLINE_7XX_CASE_INIT, 0, 0, 0, (unsigned char *)result, &len);
	ex_log(LOG_INFO, "init_7xx_test ret=%d, buffer.result = %d", ret, *result);
}

static void uninit_7xx_test(int *result)
{
	int ret = FP_LIB_ERROR_GENERAL, len=sizeof(int);

	*result = 0;
	ret = flow_inline_legacy_cmd(FP_INLINE_7XX_CASE_UNINIT, 0, 0, 0, (unsigned char *)result, &len);
	ex_log(LOG_INFO, "uninit_7xx_test ret=%d, buffer.result = %d", ret, *result);
}

#ifdef __INLINE_SAVE_LOG__
#define SAVE_AND_PRINT_LOG(format, ...)				\
	do {											\
		ex_log(LOG_DEBUG, format, ##__VA_ARGS__);	\
		__inline_save_log_data(format, ##__VA_ARGS__);		\
	} while (0)

static void __inline_save_log_data(const char* pImage, ...){

	static char buf[1000]={0};
	if (pImage == NULL) {
		ex_log(LOG_INFO, "inline_save_log_data pImage = NULL");
	}
	va_list arg;
	va_start(arg, pImage);
	unsigned int pImage_size = vsnprintf((char* )buf, 100, pImage, arg);
	va_end(arg);

	FILE* fn = fopen(INLINE_LOG_PATH, "ab");
	if (fn != NULL) {
		unsigned int size = fwrite(buf, 1, pImage_size, fn);
		fclose(fn);
		ex_log(LOG_INFO, "inline_save_log_data done size = %d", size);
	}
}

static void __inline_print_log_file(struct sensor_test_output* buffer_out_data)
{
	ex_log(LOG_INFO, "[inline_print_log_file]");
	SAVE_AND_PRINT_LOG("WBOX [SNR_RESULT] expo:%.3f, hw_int:%d, [%d,%d]\r\n",
		   buffer_out_data->data.expo,
		   buffer_out_data->data.hw_int,
		   buffer_out_data->data.centroid_x,
		   buffer_out_data->data.centroid_y);
	SAVE_AND_PRINT_LOG("[SNR_RESULT] FEA, x1x2 = (%f,%f), y1y2=(%f,%f)\r\n",
		   buffer_out_data->data.fov_x1_result,
		   buffer_out_data->data.fov_x2_result,
		   buffer_out_data->data.fov_y1_result,
		   buffer_out_data->data.fov_y2_result);
	SAVE_AND_PRINT_LOG("[SNR_RESULT] M-FACTOR m%d.%d, r%d, v%d\r\n",
		   (int)buffer_out_data->data.mag_val,
		   (int)(buffer_out_data->data.mag_val * 1000) % 1000,
		   buffer_out_data->data.ridge,
		   buffer_out_data->data.valley);
	SAVE_AND_PRINT_LOG("[SNR_RESULT] PERIOD = % f\r\n",
		   buffer_out_data->data.period);
	SAVE_AND_PRINT_LOG("[SNR_RESULT] signal=%f, noise=%f, snr=%f\r\n",
		   buffer_out_data->data.signal, buffer_out_data->data.noise, buffer_out_data->data.snr);
	SAVE_AND_PRINT_LOG("EGFPS_RESULT %d, %d, %d\r\n",
		   buffer_out_data->data.bad_block_cnt,
		   buffer_out_data->data.bad_pxl_max_continu_cnt,
		   buffer_out_data->data.bad_pxl_cnt);

    ex_log(LOG_INFO, "[inline_save_csv]");
    FILE *fp = fopen(INLINE_CSV_DBG, "r");
    if (!fp) {
    	fp = fopen(INLINE_CSV_DBG, "w+");
    	fprintf(fp, " %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", "UID", "exposure", "hw_int", "bkg_cx", "bkg_cy", "fov_x1_result", "fov_x2_result", "fov_y1_result", "fov_y2_result", "period", "signal", "noise", "snr", "bad_block_cnt", "bad_pxl_max_continu_cnt", "bad_pxl_cnt");
    	fprintf(fp, " %x%x%x%x%x%x%x%x%x,%.3f,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d\n", g_UID[0], g_UID[1], g_UID[2], g_UID[3], g_UID[4], g_UID[5], g_UID[6], g_UID[7], g_UID[8], buffer_out_data->data.expo, buffer_out_data->data.hw_int, buffer_out_data->data.bkg_cx, buffer_out_data->data.bkg_cy, buffer_out_data->data.fov_x1_result, buffer_out_data->data.fov_x2_result, buffer_out_data->data.fov_y1_result, buffer_out_data->data.fov_y2_result, buffer_out_data->data.period, buffer_out_data->data.signal, buffer_out_data->data.noise, buffer_out_data->data.snr, buffer_out_data->data.bad_block_cnt,  buffer_out_data->data.bad_pxl_max_continu_cnt, buffer_out_data->data.bad_pxl_cnt);
    } else {
    	fp = fopen(INLINE_CSV_DBG, "ab");
         fprintf(fp, " %x%x%x%x%x%x%x%x%x,%.3f,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d\n", g_UID[0], g_UID[1], g_UID[2], g_UID[3], g_UID[4], g_UID[5], g_UID[6], g_UID[7], g_UID[8], buffer_out_data->data.expo, buffer_out_data->data.hw_int, buffer_out_data->data.bkg_cx, buffer_out_data->data.bkg_cy, buffer_out_data->data.fov_x1_result, buffer_out_data->data.fov_x2_result, buffer_out_data->data.fov_y1_result, buffer_out_data->data.fov_y2_result, buffer_out_data->data.period, buffer_out_data->data.signal, buffer_out_data->data.noise, buffer_out_data->data.snr, buffer_out_data->data.bad_block_cnt,  buffer_out_data->data.bad_pxl_max_continu_cnt, buffer_out_data->data.bad_pxl_cnt);
    }
    fclose(fp);
}
#endif
static int __inline_check_result(int cmd, struct sensor_test_output* buffer_out_data)
{
	ex_log(LOG_INFO, "[inline_check_result] cmd = %d", cmd);
	if (cmd == FP_INLINE_7XX_SNR_WKBOX_ON) {
		ex_log(LOG_INFO, "[__inline_check_result_wbox_data]");
		ex_log(LOG_INFO, "[EXPOSRE]%.3f, EXPO_MAX = %d, EXPO_MIN = %d", buffer_out_data->data.expo, EXPO_MAX, EXPO_MIN);
		if ((buffer_out_data->data.expo > EXPO_MAX) || (buffer_out_data->data.expo < EXPO_MIN))
			return INLINE_DATA_OUTOFRANGE;
		
		ex_log(LOG_INFO, "[HW_INT]%d, HW_INT_MAX = %d, HW_INT_MIN = %d", buffer_out_data->data.hw_int, HW_INT_MAX, HW_INT_MIN);
		if ((buffer_out_data->data.hw_int > HW_INT_MAX) || (buffer_out_data->data.hw_int < HW_INT_MIN))
			return INLINE_DATA_OUTOFRANGE;

		return INLINE_DATA_OK;
	}

	if (cmd == FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID) {
		ex_log(LOG_INFO, "[__inline_check_result_charter_data]");
		ex_log(LOG_INFO, "[FEA_X1]%.3f, FOVX1_MAX = %f, FOVX1_MIN = %f", buffer_out_data->data.fov_x1_result, FOVX1_MAX, FOVX1_MIN);	
		if ((buffer_out_data->data.fov_x1_result > FOVX1_MAX) || (buffer_out_data->data.fov_x1_result < FOVX1_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[FEA_X2]%.3f, FOVX2_MAX = %f, FOVX2_MIN = %f", buffer_out_data->data.fov_x2_result, FOVX2_MAX, FOVX2_MIN);		
		if ((buffer_out_data->data.fov_x2_result > FOVX2_MAX) || (buffer_out_data->data.fov_x2_result < FOVX2_MIN))
			return INLINE_DATA_OUTOFRANGE;
	
		ex_log(LOG_INFO, "[FEA_Y1]%.3f, FOVY1_MAX = %f, FOVY1_MIN = %f", buffer_out_data->data.fov_y1_result, FOVY1_MAX, FOVY1_MIN);		
		if ((buffer_out_data->data.fov_y1_result > FOVY1_MAX) || (buffer_out_data->data.fov_y1_result < FOVY1_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[FEA_Y2]%.3f, FOVY2_MAX = %f, FOVY2_MIN = %f", buffer_out_data->data.fov_y2_result, FOVY2_MAX, FOVY2_MIN);		
		if ((buffer_out_data->data.fov_y2_result > FOVY2_MAX) || (buffer_out_data->data.fov_y2_result < FOVY2_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[PERIOD]%.3f, PERIOD_MAX = %f, PERIOD_MIN = %f", buffer_out_data->data.period, PERIOD_MAX, PERIOD_MIN);
		if ((buffer_out_data->data.period > PERIOD_MAX) || (buffer_out_data->data.period < PERIOD_MIN))
			return INLINE_DATA_OUTOFRANGE;

	    ex_log(LOG_INFO, "[SIGNAL]%.3f, SIGNAL_MAX = %f, SIGNAL_MIN = %f", buffer_out_data->data.signal, SIGNAL_MAX, SIGNAL_MIN);
		if ((buffer_out_data->data.signal > SIGNAL_MAX) || (buffer_out_data->data.signal < SIGNAL_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[NOISE]%.3f, NOISE_MAX = %f, NOISE_MIN = %f", buffer_out_data->data.noise, NOISE_MAX, NOISE_MIN);	
		if ((buffer_out_data->data.noise > NOISE_MAX) || (buffer_out_data->data.noise < NOISE_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[SNR]%.3f, SNR_MAX = %f, SNR_MIN = %f", buffer_out_data->data.snr, SNR_MAX, SNR_MIN);
		if ((buffer_out_data->data.snr > SNR_MAX) || (buffer_out_data->data.snr < SNR_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[BAD_PIXEL_BLOCK_CNT_MAX]%d, SIGNAL_MAX = %d, SIGNAL_MIN = %d", buffer_out_data->data.bad_block_cnt, BAD_BLOCK_CNT_MAX, BAD_BLOCK_CNT_MIN);	
		if ((buffer_out_data->data.bad_block_cnt > BAD_BLOCK_CNT_MAX) || (buffer_out_data->data.bad_block_cnt < BAD_BLOCK_CNT_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[BAD_PIXEL_MAX_CON_CNT]%d, SIGNAL_MAX = %d, SIGNAL_MIN = %d", buffer_out_data->data.bad_pxl_max_continu_cnt, BAD_PIXEL_MAC_CONTINU_CNT_MAX, BAD_PIXEL_MAC_CONTINU_CNT_MIN);	
		if ((buffer_out_data->data.bad_pxl_max_continu_cnt > BAD_PIXEL_MAC_CONTINU_CNT_MAX) || (buffer_out_data->data.bad_pxl_max_continu_cnt < BAD_PIXEL_MAC_CONTINU_CNT_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[BAD_PIXEL_CNT]%d, SIGNAL_MAX = %d, SIGNAL_MIN = %d", buffer_out_data->data.bad_pxl_cnt, BAD_PIXEL_CNT_MAX, BAD_PIXEL_CNT_MIN);
		if ((buffer_out_data->data.bad_pxl_cnt > BAD_PIXEL_CNT_MAX) || (buffer_out_data->data.bad_pxl_cnt < BAD_PIXEL_CNT_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[BKG_CENTER_X]%d, BKG_CENTER_X_MAX = %d, BKG_CENTER_X_MIN = %d", buffer_out_data->data.bkg_cx, BKG_CENTER_X_MAX, BKG_CENTER_X_MIN);
		if ((buffer_out_data->data.bkg_cx > BKG_CENTER_X_MAX) || (buffer_out_data->data.bkg_cx < BKG_CENTER_X_MIN))
			return INLINE_DATA_OUTOFRANGE;

		ex_log(LOG_INFO, "[BKG_CENTER_Y]%d, BKG_CENTER_Y_MAX = %d, BKG_CENTER_Y_MIN = %d", buffer_out_data->data.bkg_cy, BKG_CENTER_Y_MAX, BKG_CENTER_Y_MIN);
		if ((buffer_out_data->data.bkg_cy > BKG_CENTER_Y_MAX) || (buffer_out_data->data.bkg_cy < BKG_CENTER_Y_MIN))
			return INLINE_DATA_OUTOFRANGE;		

		return FP_LIB_OK;
	}
	return FP_LIB_OK;
}
#ifdef __INLINE_SAVE_LOG__
static void __inline_get_snr_images(int cmd, int image_num, char** path, int image_start_number)
{
	char temp_path[260]={0};
	int ret = 0;
	struct sensor_test_output *buffer_out_data;
	int buffer_out_length = sizeof(struct sensor_test_output);
	buffer_out_data = (struct sensor_test_output *)plat_alloc(sizeof(struct sensor_test_output));
	if(buffer_out_data==NULL) {
		ex_log(LOG_ERROR, "plat_alloc ERROR");
		return;
	}
	if (cmd == FP_INLINE_7XX_SNRCT_GET_IMAGE) {
		sprintf(temp_path, "%s", path[0]);
		ex_log(LOG_ERROR, "FP_INLINE_7XX_SNRCT_GET_IMAGE");
		ret = flow_inline_legacy_cmd(cmd, image_start_number, 0, 0,
					     (unsigned char *)buffer_out_data, &buffer_out_length);
                if (ret != 0) ex_log(LOG_ERROR, "flow_inline_legacy_cmd ERROR, %d", ret);
                plat_save_raw_image(temp_path, (unsigned char *)buffer_out_data->picture_buffer_8, IMG_MAX_BUFFER_SIZE, 1);
                PLAT_FREE(buffer_out_data);
		return;
	}
	for (int i = 0; i < image_num; i++) {
	        if (cmd == FP_INLINE_7XX_EXPSTEP2_GET_IMAGE) {
            	    float expo_step1 = (g_expo_step1 - 1) + (0.1 * i);
            	    sprintf(temp_path, "%s%.3f.bin", path[i], expo_step1);
                } else {
            	    sprintf(temp_path, "%s", path[i]);
                }
                ret = flow_inline_legacy_cmd(cmd, i+image_start_number, 0, 0, 
					(unsigned char *)buffer_out_data, &buffer_out_length);
		if(ret!=0) 	ex_log(LOG_ERROR, "flow_inline_legacy_cmd ERROR, %d", ret);

		for (int j = 0; j < IMG_MAX_BUFFER_SIZE; j++) {
			buffer_out_data->picture_buffer_16[j] = LITTLE_TO_BIG_ENDIAN(buffer_out_data->picture_buffer_16[j]);
		}
		plat_save_raw_image(temp_path, (unsigned char *)buffer_out_data->picture_buffer_16, IMG_MAX_BUFFER_SIZE*2, 1);
	}
	PLAT_FREE(buffer_out_data);
}
#endif
static int __inline_get_snr_MT_cali_data(int cmd)
{
	int ret = 0;
	struct sensor_test_output *buffer_out_data;
	int buffer_out_length = sizeof(struct sensor_test_output);

	buffer_out_data = (struct sensor_test_output *)plat_alloc(sizeof(struct sensor_test_output));
	if(buffer_out_data==NULL) {
		ex_log(LOG_ERROR, "plat_alloc ERROR");
		return 80;
	}
	ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_DATA, 0, 0, 0, 
					(unsigned char *)buffer_out_data, &buffer_out_length);
	if(ret!=0) 	ex_log(LOG_ERROR, "flow_inline_legacy_cmd ERROR, %d", ret);

	int returncode = __inline_check_result(cmd, buffer_out_data);
	if (cmd == FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID) {
#ifdef __INLINE_SAVE_LOG__
	__inline_print_log_file(buffer_out_data);
        g_inline_siganl = buffer_out_data->data.signal;
            g_inline_noise = buffer_out_data->data.noise;
            g_inline_snr = buffer_out_data->data.snr;
#endif
	}
	if (cmd == FP_INLINE_7XX_SNR_WKBOX_ON) {
		g_expo_step1 = buffer_out_data->data.expo_step1;
        }
	PLAT_FREE(buffer_out_data);
	ex_log(LOG_ERROR, "__inline_get_snr_MT_cali_data returncode=%d", returncode);
	return returncode;
}

static int __inline_get_snr_data(int cmd)
{
	ex_log(LOG_INFO, "[inline_get_snr_data] cmd = %d", cmd);
#ifdef __INLINE_SAVE_LOG__
        int image_num=0, image_start_number=0;
#endif
	//char temp_path[260]={0};
	//mkdir(DEFINE_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	//time_t timep;
	//struct tm *ptime;

	if (cmd == FP_INLINE_7XX_SNR_WKBOX_ON) {
		ex_log(LOG_INFO, "FP_INLINE_7XX_SNR_WKBOX_ON");
		int ret = __inline_get_snr_MT_cali_data(cmd);
		return ret;
	}
	if (cmd == FP_INLINE_7XX_SNR_WKBOX_ON_IMAGE) {
#ifdef __INLINE_SAVE_LOG__			
		ex_log(LOG_INFO, "FP_INLINE_7XX_SNR_WKBOX_ON_SAVE_IMAGE");
		image_num = sizeof(struct et7xx_inline_expstep1_lmage_path) / sizeof(char *);
		ex_log(LOG_INFO, "FP_INLINE_7XX_EXPSTEP1_GET_IMAGE image_num = %d", image_num);
		__inline_get_snr_images(FP_INLINE_7XX_EXPSTEP1_GET_IMAGE, image_num,
					            inline_expstep1_image_path.lmage_path, image_start_number);

		image_num = sizeof(struct et7xx_inline_expstep2_lmage_path) / sizeof(char *);
		ex_log(LOG_INFO, "FP_INLINE_7XX_EXPSTEP2_GET_IMAGE image_num = %d expo_step1 = %f", image_num, g_expo_step1);
		__inline_get_snr_images(FP_INLINE_7XX_EXPSTEP2_GET_IMAGE, image_num,
					inline_expstep2_image_path.lmage_path, image_start_number);
#endif
		return FP_LIB_OK;	
        }

	if (cmd == FP_INLINE_7XX_SNR_BKBOX_ON) {
#ifdef __INLINE_SAVE_LOG__			
		image_num = sizeof(struct et7xx_inline_bp_lmage_path) / sizeof(char *);
		ex_log(LOG_INFO, "FP_INLINE_7XX_BP_GET_IMAGE image_num = %d", image_num);
		__inline_get_snr_images(FP_INLINE_7XX_BP_GET_IMAGE, image_num,
					inline_bp_image_path.lmage_path, image_start_number);
#endif						
	    return FP_LIB_OK;				
	}

	if (cmd == FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID) {
		ex_log(LOG_INFO, "FP_INLINE_7XX_SNR_GET_DATA");
		int ret = __inline_get_snr_MT_cali_data(cmd);
		return ret;
	}
#ifdef __INLINE_SAVE_LOG__	
	image_num = sizeof(struct et7xx_inline_lmage_path) / sizeof(char *);
	ex_log(LOG_INFO, "FP_INLINE_7XX_GET_IMAGE Start, image_num=%d", image_num);
	__inline_get_snr_images(FP_INLINE_7XX_GET_IMAGE, image_num, 
						inline_image_path.lmage_path, image_start_number+1 /*skip picture_temp8 in TA*/);

	image_num = sizeof(struct et7xx_inline_snr_lmage_path) / sizeof(char *);
	ex_log(LOG_INFO, "FP_INLINE_7XX_SNR_GET_IMAGE image_num = %d", image_num);
	__inline_get_snr_images(FP_INLINE_7XX_SNR_GET_IMAGE, image_num, 
						inline_snr_image_path.lmage_path, image_start_number);

	ex_log(LOG_INFO, "FP_INLINE_7XX_SNRCT_GET_IMAGE");
	__inline_get_snr_images(FP_INLINE_7XX_SNRCT_GET_IMAGE, 1,
						inline_snrct_image_path.lmage_path, image_start_number);					
#endif	
	return FP_LIB_OK;
}

#ifdef __INLINE_SAVE_LOG__		
static void __inline_get_norscan_data(int cmd)
{
	ex_log(LOG_INFO, "[inline_get_norscan_data]");
	int ret = 0;
	struct sensor_test_output *buffer_out_data = NULL;
	char temp_path[260];
	int buffer_out_length = sizeof(struct sensor_test_output);
	//mkdir(DEFINE_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    buffer_out_data = (struct sensor_test_output *)plat_alloc(sizeof(struct sensor_test_output));
	ex_log(LOG_INFO, "GET_NORMAL_IMAGE Start, size=%d", sizeof(struct sensor_test_output));
	sprintf(temp_path, "%s", inline_image_normal_path.lmage_path[0]);
	ret = flow_inline_legacy_cmd(cmd, 0, 0, 0,
				     (unsigned char *)buffer_out_data, &buffer_out_length);
	plat_save_raw_image(temp_path, (unsigned char *)buffer_out_data->picture_buffer_16, IMG_MAX_BUFFER_SIZE*2, 1);
        for (int i = 0; i < 9; i++) {
	    g_UID[i] = buffer_out_data->UID[i];
	    ex_log(LOG_INFO, "[__inline_get_norscan_data] UID = 0x%x ", g_UID[i]);
	}
	PLAT_FREE(buffer_out_data);
        ex_log(LOG_INFO, "[inline_get_norscan_data end] ret = %d", ret);
}
#endif //__INLINE_SAVE_LOG__

static void __inline_sensortest_reset_sensor()
{
	fp_device_reset(g_hdev);
	opm_initialize_sensor();
}

int do_7XX_spi_test(int cmd)
{
	int ret = FP_LIB_ERROR_GENERAL;
	int buffer_out_length = sizeof(int), test_result = 0;
    mkdir(DEFINE_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	ex_log(LOG_INFO, "cmd = %d", cmd);

	init_7xx_test(&test_result);
	fp_device_power_control(g_hdev, TRUE);
	fp_device_reset(g_hdev);
	usleep(500*000);
	ex_log(LOG_INFO, "FP_INLINE_7XX_SPI(NORSCAN) Start");
	do{
		ret = flow_inline_legacy_cmd(FP_INLINE_7XX_NORMALSCAN, 0, 0, 0, 
				(unsigned char *)&test_result, &buffer_out_length);
		ex_log(LOG_INFO, "ret=%d, test_result = %d", ret, test_result);
		if (ret != FP_LIB_OK) 
			ex_log(LOG_ERROR, "ret=%d, test_result = %d", ret, test_result);

		switch (test_result) {
			case EGIS_7XX_TEST_READ_SENSOR_INFO_START:
				ex_log(LOG_INFO, "not support EGIS_READ_SENSOR_INFO_START");
				break;
			case EGIS_7XX_TEST_READ_SENSOR_INFO_END:
				ex_log(LOG_INFO, "not support EGIS_READ_SENSOR_INFO_END");
				break;
			case EGIS_7XX_TEST_RESET:
				ex_log(LOG_INFO, "EGIS_7XX_TEST_RESET");
				fp_device_reset(g_hdev);
				break;
			case FP_LIB_OK:
			case EGIS_7XX_TEST_SENSOR_TEST_FAIL:
				break;
			default:
				ex_log(LOG_ERROR, "FAIL");
				break;
		}
	}while(!(test_result==FP_LIB_OK ||
			 test_result==EGIS_7XX_TEST_SENSOR_TEST_FAIL));
	fp_device_power_control(g_hdev, FALSE);
	ex_log(LOG_INFO, "FP_INLINE_7XX_SPI(NORSCAN) End");

#ifdef __INLINE_SAVE_LOG__
	__inline_get_norscan_data(FP_INLINE_7XX_NORMAL_GET_IMAGE);
#endif
	int spi_test_result = test_result;
	uninit_7xx_test(&test_result);
	__inline_sensortest_reset_sensor();

	ex_log(LOG_INFO, "spi_test_result = %d", spi_test_result);
	return spi_test_result;
}

int do_7XX_sensortest(int cmd, int param1, int param2, int param3,
						unsigned char *out_buf, int *out_size)
{
	int ret = FP_LIB_ERROR_GENERAL;
	int error_code = -1;
	static unsigned long long time_diff_total = 0;
	int buffer_out_length = sizeof(int), test_result = 0;
	unsigned long long time_start= plat_get_time();
	unsigned long long time_diff = 0;
    mkdir(DEFINE_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	ex_log(LOG_INFO, "cmd = %d", cmd);
    switch(cmd) {
		case FP_INLINE_7XX_CALI_SPI_TEST:
		    host_touch_set_hbm(1);
			ret = do_7XX_spi_test(cmd);
		    time_diff = plat_get_diff_time(time_start);
		    ex_log(LOG_INFO, "ret=%d, test_result = %d, %d spent %d ms", ret, test_result, cmd, time_diff);
			//break;
			goto SPI_TEST_END;

        case FP_INLINE_7XX_NORMALSCAN:
			init_7xx_test(&test_result);
			fp_device_power_control(g_hdev, TRUE);
			fp_device_reset(g_hdev);
			usleep(50*1000);
			ex_log(LOG_INFO, "FP_INLINE_7XX_NORMALSCAN Start");
			do{
				ret = flow_inline_legacy_cmd(cmd, param1, param2, param3,
						(unsigned char *)&test_result, &buffer_out_length);
				ex_log(LOG_INFO, "ret=%d, test_result = %d", ret, test_result);
				if (ret != FP_LIB_OK)
					ex_log(LOG_ERROR, "ret=%d, test_result = %d", ret, test_result);

				switch (test_result) {
					case EGIS_7XX_TEST_READ_SENSOR_INFO_START:
						ex_log(LOG_INFO, "not support EGIS_READ_SENSOR_INFO_START");
						break;
					case EGIS_7XX_TEST_READ_SENSOR_INFO_END:
						ex_log(LOG_INFO, "not support EGIS_READ_SENSOR_INFO_END");
						break;
					case EGIS_7XX_TEST_RESET:
						ex_log(LOG_INFO, "EGIS_7XX_TEST_RESET");
						fp_device_reset(g_hdev);
						break;
					case FP_LIB_OK:
					case EGIS_7XX_TEST_SENSOR_TEST_FAIL:
#ifdef __INLINE_SAVE_LOG__
						__inline_get_norscan_data(FP_INLINE_7XX_NORMAL_GET_IMAGE);
#endif
						uninit_7xx_test(&test_result);
						fp_device_reset(g_hdev);
						opm_initialize_sensor();
						break;
					default:
						uninit_7xx_test(&test_result);
						__send_7XX_sensortest_event(cmd, 1000);
						ex_log(LOG_ERROR, "FAIL");
						break;
				}
			}while(!(test_result==FP_LIB_OK ||
					 test_result==EGIS_7XX_TEST_SENSOR_TEST_FAIL));
			fp_device_power_control(g_hdev, FALSE);
			host_touch_set_hbm(0);
			ex_log(LOG_INFO, "FP_INLINE_7XX_NORMALSCAN End");
			break;
		/* no use*/
	    case FP_INLINE_7XX_SNR_INIT:
		    init_7xx_test(&test_result);
		    break;
	    /* no use end*/
	    case FP_INLINE_7XX_FLASH_TEST:
		    ret = FP_LIB_OK;
			ex_log(LOG_INFO, "FP_INLINE_7XX_FLASH_TEST here no use");
			__send_7XX_sensortest_event(cmd, ret);
		    break;
	    case FP_INLINE_7XX_SNR_WKBOX_ON:
        case FP_INLINE_7XX_SNR_BKBOX_ON:
        case FP_INLINE_7XX_SNR_CHART_ON:
			if (cmd==FP_INLINE_7XX_SNR_CHART_ON)
				cmd = FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID;
		case FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID:
		    host_touch_set_hbm(1);
		    fp_device_power_control(g_hdev, TRUE);
		    if (cmd == FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID) {
			    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_FLASH_TEST, param1, param2, param3,
							 (unsigned char *)&test_result, &buffer_out_length);
			    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_FLASH_TEST_2, param1, param2, param3,
							 (unsigned char *)&test_result, &buffer_out_length);
			    if (test_result == FP_LIB_OK) {
					ex_log(LOG_INFO, "Test Charter in");
				    ret = flow_inline_legacy_cmd(cmd, param1, param2, param3,
								 (unsigned char *)&test_result, &buffer_out_length);
			    } else {
					ex_log(LOG_INFO, "FP_INLINE_7XX_FLASH_TEST Flash test fail retrun = %d", test_result);
			    }
		    } else {
			    ret = flow_inline_legacy_cmd(cmd, param1, param2, param3,
							 (unsigned char *)&test_result, &buffer_out_length);
		    }
		    if (cmd == FP_INLINE_7XX_SNR_BKBOX_ON) {
                        ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_BKBOX_ON_2, param1, param2, param3,
							 (unsigned char *)&test_result, &buffer_out_length);
		    }
                    time_diff = plat_get_diff_time(time_start);
		    ex_log(LOG_INFO, "ret=%d, test_result = %d, %d spent %d ms", ret, test_result, cmd, time_diff);
#ifdef __INLINE_SAVE_LOG__
			SAVE_AND_PRINT_LOG("CMD = %d spent %d ms\r\n", cmd, time_diff);
#endif
		    time_diff_total += time_diff;

		    //if (ret != FP_LIB_OK) break;

		    switch (test_result) {
			    case EGIS_7XX_TEST_RESET:
					fp_device_reset(g_hdev);
					break;
				case FP_LIB_OK:
					if (cmd == FP_INLINE_7XX_SNR_WKBOX_ON) {
						error_code = __inline_get_snr_data(cmd);
						__send_7XX_sensortest_event(cmd, error_code);
						host_touch_set_hbm(0);
						__inline_get_snr_data(FP_INLINE_7XX_SNR_WKBOX_ON_IMAGE);
					}
					if (cmd == FP_INLINE_7XX_SNR_BKBOX_ON) {
						__send_7XX_sensortest_event(cmd, test_result);
						host_touch_set_hbm(0);
						__inline_get_snr_data(cmd);
					}
					if (cmd == FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID) {
						error_code = __inline_get_snr_data(cmd);
#ifdef __INLINE_SAVE_LOG__
						SAVE_AND_PRINT_LOG("Total Test spent %d ms\r\n", time_diff_total);
#endif
						cmd = FP_INLINE_7XX_SNR_CHART_ON;
						__send_7XX_sensortest_event(cmd, error_code);
						__inline_get_snr_data(0);
                        uninit_7xx_test(&test_result);
                        host_touch_set_hbm(0);
						time_diff_total = 0;
					}
					break;
				case EGIS_7XX_TEST_SENSOR_TEST_FAIL:
				default:
					if (cmd == FP_INLINE_7XX_SNR_WKBOX_ON) {
						__inline_get_snr_data(cmd);
						__inline_get_snr_data(FP_INLINE_7XX_SNR_WKBOX_ON_IMAGE);
					}
                    __send_7XX_sensortest_event(cmd, test_result);
					uninit_7xx_test(&test_result);
					host_touch_set_hbm(0);
                    ex_log(LOG_ERROR, "FAIL");
					break;
			}
			fp_device_power_control(g_hdev, FALSE);
        	break;
#ifdef __INLINE_SAVE_LOG__
		case FP_INLINE_7XX_TEST_MFACTOR:
	     	ex_log(LOG_INFO, "FP_INLINE_7XX_TEST_MFACTOR Start");
			init_7xx_test(&test_result);
			fp_device_power_control(g_hdev, TRUE);
			__inline_get_snr_data(cmd);
			__send_7XX_sensortest_event(cmd, 0);
			uninit_7xx_test(&test_result);
			fp_device_power_control(g_hdev, FALSE);			
			ret = FP_LIB_OK;
			break;
#endif
		case FP_INLINE_7XX_SNR_GET_DATA:
	     	ex_log(LOG_DEBUG, "FP_INLINE_7XX_SNR_GET_DATA Start");
	     	if(out_size) ex_log(LOG_DEBUG, "out_size=%d", *out_size);
			flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_DATA, param1, param2, param3,
					     (unsigned char *)out_buf, out_size);
			ret = FP_LIB_OK;
			return ret;
		case FP_INLINE_7XX_CASE_UNINIT:
			uninit_7xx_test(&test_result);
			ret = FP_LIB_OK;
			break;			
        default:
			uninit_7xx_test(&test_result);
			__send_7XX_sensortest_event(cmd, 1000);
			ex_log(LOG_ERROR, "not support, cmd = %d", cmd);
			break;
    }

SPI_TEST_END:
	if ((out_buf!=NULL) && (out_size!=NULL) && 
		(*out_size>0) && (*out_size>=buffer_out_length)){
		memcpy(out_buf, (unsigned char *)&test_result, buffer_out_length);
		ex_log(LOG_INFO, "copy to buffer_out, length=%d", buffer_out_length);
	}
	else{
		ex_log(LOG_INFO, "buffer_out_length == %d", buffer_out_length);
	}

	ex_log(LOG_INFO, "ret = %d", ret);
	return ret;
}
#endif	
