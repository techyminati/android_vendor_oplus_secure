#ifdef __ET0XX__
#include "fps_normal.h"
#include "response_def.h"
#include "plat_time.h"
unsigned int fp_device_open(int *device_handle)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_close(int device_handle)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_reset(int device_handle)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_clock_enable(int device_handle,
				    BOOL enable)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_interrupt_enable(int device_handle, int cmd)
{

	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_interrupt_wait(int device_handle, unsigned int timeout)
{
	plat_sleep_time(1000);
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int egisFpDeviceFingerOnZero(int handle)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_power_control(int device_handle, BOOL enable)
{
	return FINGERPRINT_RES_SUCCESS;
}

void fp_set_interrupt_trigger_type(int trigger_type)
{
	return ;
}	

unsigned int fp_device_remove_node(int device_handle)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_wakelock_enable(int device_handle, BOOL enable)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_pin_init(int device_handle) 
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_poweron(int device_handle)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_get_screen_state(int device_handle,int * screen_state)
{
	return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_create_fpid(int device_handle)
{
	return 0;
}
#endif