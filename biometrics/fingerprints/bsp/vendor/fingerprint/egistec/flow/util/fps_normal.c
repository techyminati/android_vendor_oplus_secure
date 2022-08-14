#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include "fps_normal.h"
#include "response_def.h"
#include "plat_log.h"
#include "plat_time.h"

#ifndef DEVICE_DRIVER_HAS_SPI
struct egis_ioc_transfer {
	__u32 int_mode;
	__u32 detect_period;
	__u32 detect_threshold;
};
#else
struct egis_ioc_transfer {
	__u8 *tx_buf;
	__u8 *rx_buf;
	__u32 len;
	__u32 speed_hz;
	__u16 delay_usecs;
	__u8 bits_per_word;
	__u8 cs_change;
	__u8 opcode;
#ifdef PROJECT_AOP_DRIVER
	__u8 sector;
	__u32 umax;
	__u32 umin;
	__u8 flag;
#endif
	__u8 pad[3];
};

#define FP_SPI_SPEED_HZ 9700000
#define DELAY_USECS 0
#define BITS_PER_WORD 8
#define CS_CHANGE 1
#define SPI_IOC_MAGIC 'k'
#define SPI_MSGSIZE(N)                                                       \
	((((N) * (sizeof(struct egis_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
	     ? ((N) * (sizeof(struct egis_ioc_transfer)))                    \
	     : 0)
#define SPI_IOC_MESSAGE(N) _IOW(SPI_IOC_MAGIC, 0, char[SPI_MSGSIZE(N)])
#endif
#define DEVICE_PATH_NAME "/dev/esfp0"

#define FP_SENSOR_RESET 0x04
#define FP_POWER_CONTROL 0x05
#define INT_TRIGGER_INIT 0xa4   // Trigger signal initial routine
#define INT_TRIGGER_CLOSE 0xa5  // Trigger signal close routine
#define INT_TRIGGER_ABORT 0xa8
#define FP_SENSOR_PIN_INIT 0x08
#define FP_CREATE_DEVICE_FPID 0x09
#define FP_SCREEN_STATE		0xb3
#define FP_POWER			0xb0
// INT TRIGGER TYPE
#define EDGE_TRIGGER_FALLING 0x0
#define EDGE_TRIGGER_RAISING 0x1
#define LEVEL_TRIGGER_LOW 0x2
#define LEVEL_TRIGGER_HIGH 0x3
#ifdef  __TRUSTONIC__
#define FP_SPICLK_ENABLE  0xaa//0x81
#define FP_SPICLK_DISABLE 0xab//0x82
#define FP_REMOVE_DEVICE_NODE 0xac
#define FP_WAKELOCK_ENABLE 0xb1
#define FP_WAKELOCK_DISABLE 0xb2
#else
#define FP_SPICLK_ENABLE  0x81
#define FP_SPICLK_DISABLE 0x82
#endif

unsigned char pad_edge_f[3] = {EDGE_TRIGGER_FALLING, 1, 1};
unsigned char pad_edge_r[3] = {EDGE_TRIGGER_RAISING, 10, 6};
unsigned char pad_level_l[3] = {LEVEL_TRIGGER_LOW, 0, 0};

unsigned char * const default_pad = pad_edge_f;
const unsigned char *  g_pad = default_pad;

static unsigned int egis_call_driver(int device_handle, unsigned int ioctl_code,
				     __unused void *in_buffer,
				     __unused unsigned int in_buffer_size,
				     __unused void *out_buffer, void *pad,
				     __unused unsigned int speed_hz)
{
#ifndef DEVICE_DRIVER_HAS_SPI
	static struct egis_ioc_transfer xfer;

	if (NULL != pad) {
		xfer.int_mode = ((__u8 *)pad)[0];
		xfer.detect_period = ((__u8 *)pad)[1];
		xfer.detect_threshold = ((__u8 *)pad)[2];
	}

	if (-1 == ioctl(device_handle, ioctl_code, &xfer)) {
		return FINGERPRINT_RES_FAILED;
	}
#else
	static struct egis_ioc_transfer xfer = {
	    NULL,	NULL,		0,	FP_SPI_SPEED_HZ,
	    DELAY_USECS, BITS_PER_WORD, CS_CHANGE};
	xfer.tx_buf = (__u8 *)in_buffer;
	xfer.rx_buf = (__u8 *)out_buffer;
	xfer.len = (unsigned int)in_buffer_size;
	xfer.opcode = ioctl_code;
	if (pad) {
		xfer.pad[0] = ((__u8 *)pad)[0];
		xfer.pad[1] = ((__u8 *)pad)[1];
		xfer.pad[2] = ((__u8 *)pad)[2];
	}
	if (speed_hz > 0) xfer.speed_hz = (__u32)speed_hz;
	if (ioctl(device_handle, SPI_IOC_MESSAGE(1), &xfer) == -1)
		return FINGERPRINT_RES_FAILED;
#endif
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_open(int *device_handle)
{
#ifdef SIMULATE_NO_SENSOR
	return FINGERPRINT_RES_SUCCESS;
#endif

	if (NULL == device_handle) {
		return FINGERPRINT_RES_INVALID_PARAM;
	}

	*device_handle = open(DEVICE_PATH_NAME, O_RDWR);
	if(*device_handle < 0){
		*device_handle = 0;
		return FINGERPRINT_RES_HW_UNAVALABLE;
	}

	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_close(int device_handle)
{
	if (device_handle != 0) {
		int retval = close(device_handle);
		return retval == 0 ? FINGERPRINT_RES_SUCCESS
				   : FINGERPRINT_RES_FAILED;
	}
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_reset(int device_handle)
{
#ifdef SIMULATE_NO_SENSOR
	return FINGERPRINT_RES_SUCCESS;
#endif
	if (0 == device_handle) {
		return FINGERPRINT_RES_INVALID_PARAM;
	}
	int ret = FINGERPRINT_RES_SUCCESS;
	ret = egis_call_driver((int)device_handle, FP_SENSOR_RESET, NULL, 0,
				NULL, NULL, 0);
	plat_wait_time(15);//wait 15ms
	return ret;
}

unsigned int fp_device_poweron(int device_handle)
{
#ifdef SIMULATE_NO_SENSOR
	return FINGERPRINT_RES_SUCCESS;
#endif
	if (0 == device_handle) {
		return FINGERPRINT_RES_INVALID_PARAM;
	}
	int ret = FINGERPRINT_RES_SUCCESS;
	ret = egis_call_driver((int)device_handle, FP_POWER, NULL, 0,
				NULL, NULL, 0);
	plat_wait_time(5);//wait 5ms
	return ret;
}

unsigned int fp_device_clock_enable(__unused int device_handle,
				    __unused BOOL enable)
{
#ifdef DEVICE_DRIVER_NEED_SPI_ENALBE_DISABLE
	if (enable == TRUE) {
		return egis_call_driver((int)device_handle, FP_SPICLK_ENABLE,
					NULL, 0, NULL, NULL, 0);
	} else {
		return egis_call_driver((int)device_handle, FP_SPICLK_DISABLE,
					NULL, 0, NULL, NULL, 0);
	}
#else
	return FINGERPRINT_RES_SUCCESS;
#endif
}

void fp_set_interrupt_trigger_type(int trigger_type){

	if (trigger_type == TRIGGER_RESET){
		g_pad = default_pad;
		return;
	}
	
	if (trigger_type == EDGE_FALLING){
		g_pad = pad_edge_f;
	}else if (trigger_type == EDGE_RAISING){
		g_pad = pad_edge_r;
	}else if (trigger_type == LEVEL_LOW){
		g_pad = pad_level_l;
	}else
		g_pad = default_pad;
}

unsigned int fp_device_interrupt_enable(int device_handle, int cmd)
{
	unsigned int ret = 0;
	ex_log(LOG_DEBUG, "fp_device_interrupt_enable cmd:%d ,%d ,%d ,%d", cmd,g_pad[0],g_pad[1],g_pad[2]);
	const unsigned char in_buffer[1] = {0x0}, out_buffer[1] = {0x0};

#ifdef SIMULATE_NO_SENSOR
	return FINGERPRINT_RES_SUCCESS;
#endif

	if (cmd == FLAG_INT_INIT)
	{
		ret = egis_call_driver((int)device_handle, INT_TRIGGER_INIT, (void*)in_buffer, 1, (void*)out_buffer, (void*)g_pad, 0);
	}
	else if(cmd == FLAG_INT_ABORT)
	{
		ret = egis_call_driver((int)device_handle, INT_TRIGGER_ABORT, (void*)in_buffer, 1, (void*)out_buffer, NULL, 0);
	}
	else if(cmd == FLAG_INT_CLOSE)
	{
		ret = egis_call_driver((int)device_handle, INT_TRIGGER_CLOSE, (void*)in_buffer, 1, (void*)out_buffer, NULL, 0);
	}

	return ret;
}

unsigned int fp_device_interrupt_wait(int device_handle, unsigned int timeout)
{
	struct pollfd pollfds[2];
	int ret;

	pollfds[0].fd = (int)device_handle;
	pollfds[0].events = POLLIN;
#ifdef SIMULATE_NO_SENSOR
	usleep(1000 * 1000);
	return FINGERPRINT_RES_SUCCESS;
#endif
	ret = poll(pollfds, 1, (int)timeout);
	switch (ret) {
		case 0: {
			return FINGERPRINT_RES_TIMEOUT;
		}
		case -1: {
			return FINGERPRINT_RES_FAILED;
		}
		default: {
			if (pollfds[0].revents & POLLIN) {
				return FINGERPRINT_RES_SUCCESS;
			}
		}
	}

	return FINGERPRINT_RES_TIMEOUT;
}

unsigned int egisFpDeviceFingerOnZero(__unused int handle)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_power_control(__unused int device_handle, __unused BOOL enable)
{
	// 	return egis_call_driver((int)device_handle, FP_POWER_CONTROL, NULL,
	// 				(unsigned int)enable, NULL, NULL, 0);
	// 	return FINGERPRINT_RES_SUCCESS;
	// return egis_call_driver((int)device_handle, FP_POWER_CONTROL, NULL, 0, NULL, NULL, 0);
	return 0;
}

unsigned int fp_device_remove_node(__unused int device_handle)
{
#ifdef SIMULATE_NO_SENSOR
	return FINGERPRINT_RES_SUCCESS;
#endif

#ifdef __TRUSTONIC__
	if (0 == device_handle) {
		return FINGERPRINT_RES_INVALID_PARAM;
	}
	return egis_call_driver((int)device_handle, FP_REMOVE_DEVICE_NODE, NULL, 0,
				NULL, NULL, 0);
#else
	return FINGERPRINT_RES_SUCCESS;
#endif
}

unsigned int fp_device_wakelock_enable(__unused int device_handle,__unused BOOL enable) 
{
#ifdef __TRUSTONIC__
	if(enable) {
		return egis_call_driver((int)device_handle, FP_WAKELOCK_ENABLE, NULL, 0, NULL, NULL, 0);
	} else {
		return egis_call_driver((int)device_handle, FP_WAKELOCK_DISABLE, NULL, 0, NULL, NULL, 0);
	}	
#else
	return FINGERPRINT_RES_SUCCESS;
#endif
}

unsigned int fp_device_pin_init(int device_handle) 
{
	return egis_call_driver((int)device_handle, FP_SENSOR_PIN_INIT, NULL, 0, NULL, NULL, 0);
}

unsigned int fp_get_screen_state(int device_handle,int * screen_state)
{
	static struct egis_ioc_transfer xfer;
#ifndef DEVICE_DRIVER_HAS_SPI
	if (ioctl(device_handle, FP_SCREEN_STATE, &xfer) == -1)
		return FINGERPRINT_RES_FAILED;

	*screen_state = xfer.int_mode;

	ex_log(LOG_DEBUG, "fp_get_screen_state %d,xfer.int_mode %d ", *screen_state,xfer.int_mode);
#endif
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_create_fpid(int device_handle)
{
	return egis_call_driver((int)device_handle, FP_CREATE_DEVICE_FPID, NULL, 0, NULL, NULL, 0);
}
