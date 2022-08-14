#ifndef __FPS_NORMAL_H__
#define __FPS_NORMAL_H__

#include "type_def.h"

typedef enum INTERRUPT_OPERATE {
	FLAG_INT_INIT,
	FLAG_INT_ABORT,
	FLAG_INT_CLOSE,
} interrupt_operate;

#define TRIGGER_RESET 0xFFFF
#define EDGE_FALLING 1
#define EDGE_RAISING 2
#define LEVEL_LOW 3

unsigned int fp_device_open(int *handle);

unsigned int fp_device_close(int handle);

unsigned int fp_device_reset(int handle);

unsigned int fp_device_reset_set(int handle, int high_low);

unsigned int fp_device_clock_enable(int handle, BOOL enable);

unsigned int fp_device_interrupt_enable(int handle, int cmd);

unsigned int fp_device_interrupt_wait(int handle, unsigned int timeout);

unsigned int egisFpDeviceFingerOnZero(int handle);

unsigned int fp_device_power_control(int device_handle, BOOL enable);

void fp_set_interrupt_trigger_type(int trigger_type);

unsigned int fp_device_touch_wait(int device_handle);

unsigned int fp_device_no_touch_wait(int device_handle);

unsigned int fp_device_ui_wait(int device_handle);

#endif

