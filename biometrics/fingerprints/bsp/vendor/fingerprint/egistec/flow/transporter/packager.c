#include "egis_definition.h"
#include "packager.h"
#include "transporter.h"
#include "response_def.h"
#include "thread_manager.h"
#include "stdlib.h"
#include "plat_log.h"
#include "plat_time.h"
#include "fps_normal.h"
#include "common_definition.h"

typedef struct transfer_package {
	unsigned int process;
	unsigned int command;
	unsigned int uid;
	unsigned int fid;
	unsigned int in_data_len;
	unsigned char in_data[0];
} transfer_package_t;

#define LOG_TAG "RBS"
#define SIZEOF_INT sizeof(unsigned int)
#define BASE_TRANSFER_SIZE 1024

#define EGIS_NEED_TO_RESET_RETRY_COUNT 15

#ifdef SUPPORT_EGIS_NEED_TO_RESET
#include "fps_normal.h"
extern int g_hdev;
#endif

extern int g_hdev;

BOOL is_sensor_command(unsigned int command)
{
	BOOL retval = FALSE;
	switch (command) {
		case EX_CMD_INIT_SENSOR:
		case EX_CMD_UNINIT_SENSOR:
		case EX_CMD_GET_IMAGE:
		case EX_CMD_SET_WORK_MODE:
		case EX_CMD_SENSOR_TEST:
		case EX_CMD_CHECK_FINGER_LOST:
		case EX_CMD_OPEN_SPI:
		case EX_CMD_CLOSE_SPI:
		case EX_CMD_CALIBRATION:
		case EX_CMD_EXTRA:
			retval = TRUE;
			break;
		default:
			break;
	}
	return retval;
}

/*
**	@transfer_data
**	Transmission information channel
**	@params
**	[ pid cid uid fid in_data_len ] transfer_header_t
**	[ in_data ] the pointer of the msg string
**	[ out_data_len ] the pointer of the @out_data length
**	[ out_data ] the pointer of the out info string
**	@other_explain
**	[ BASE_TRANSFER_SIZE ] align up the message length with a certain size
*/
int transfer_data(unsigned int pid, unsigned int cid, unsigned int uid, unsigned int fid,
		  unsigned int in_data_len, unsigned char *in_data,
		  unsigned int *out_data_len, unsigned char *out_data)
{
	int retval;
	unsigned int msg_data_len, rsp_data_len;
	transfer_package_t *msg_data = NULL;
	BOOL bsensor_command = FALSE;
#ifdef SUPPORT_EGIS_NEED_TO_RESET
	int retry_reset_count = 0;
#endif
	if (0 != trylock_operation(RBS_MUTEX_TRANSFER))  //To Do : use lock instead of trylock
		return FINGERPRINT_RES_NOT_IDLE;

	bsensor_command = is_sensor_command(cid);

	if (bsensor_command){
		retval = fp_device_clock_enable(g_hdev, TRUE);
		if (FINGERPRINT_RES_SUCCESS != retval) {
			ex_log(LOG_ERROR,"fp_device_clock_enable failed");
			retval = FINGERPRINT_RES_HW_UNAVALABLE;
			bsensor_command = FALSE;
			goto EXIT;
		}
	}

	msg_data_len = SIZEOF_INT * 5 + in_data_len;

	if (msg_data_len % BASE_TRANSFER_SIZE != 0) {
		msg_data_len = (msg_data_len / BASE_TRANSFER_SIZE + 1) * BASE_TRANSFER_SIZE;
	}

	msg_data = malloc(msg_data_len);
	if (NULL == msg_data) {
		retval = FINGERPRINT_RES_ALLOC_FAILED;
		goto EXIT;
	}

	for (;;) {
		/*can not remove this 'memset' calling because a lots of input strings do not have a '\0' ending such as most input paths*/
		/*or using memset(msg_data->in_data, 0, in_data_len + 1) as an alternative*/
		memset(msg_data, 0, msg_data_len);
		msg_data->process = pid;
		msg_data->command = cid;
		msg_data->uid = uid;
		msg_data->fid = fid;
		msg_data->in_data_len = 0;

		egislog_v("-- packager, process: %d, command: %d, uid: %d, fid:%d", msg_data->process, msg_data->command, msg_data->uid, msg_data->fid);

		if (in_data_len > 0 && NULL != in_data) {
			msg_data->in_data_len = in_data_len;
			memcpy(msg_data->in_data, in_data, in_data_len);
		}
		if (NULL != out_data_len) {
			rsp_data_len = *out_data_len;
		}

		fp_device_wakelock_enable(g_hdev, TRUE);
		retval = transporter((unsigned char *)msg_data, msg_data_len, out_data, out_data_len);
		fp_device_wakelock_enable(g_hdev, FALSE);

#ifdef SUPPORT_EGIS_NEED_TO_RESET
		if (retval != EGIS_ESD_NEED_RESET) break;
		else if (retry_reset_count > EGIS_NEED_TO_RESET_RETRY_COUNT) {
			egislog_e("-- packager, transporter Failed RESET (already retry %d)", retry_reset_count);
			break;
		}

		retry_reset_count++;
		egislog_e("-- packager, transporter (%d)", retry_reset_count);
#ifdef __OPLUS_K4__
		egislog_e("-- packager, fp_device_power_control");
		fp_device_power_control(g_hdev, FALSE);
		plat_sleep_time(50);
#endif
		retval = fp_device_reset(g_hdev);
		plat_wait_time(20);
		egislog_e("-- packager, transporter done RESET. retval=%d", retval);
		if (NULL != out_data_len) *out_data_len = rsp_data_len;
#else
		break;
#endif
	}

	if (NULL != msg_data) {
		free(msg_data);
	}

EXIT:
	if (bsensor_command){
		fp_device_clock_enable(g_hdev, FALSE);
	}

	unlock_operation(RBS_MUTEX_TRANSFER);

	return retval;
}
