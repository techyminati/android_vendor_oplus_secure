#ifndef __PLAT_OTG__
#define __PLAT_OTG__

#include <stdint.h>

typedef void (*event_callback_t)(int event_id, int first_param, int second_param,
				unsigned char *data, int data_size);

typedef enum core_callback_event {
	// Used event id range is 10000 - 10999
	EVENT_IO_DISPATCH_WRITE_REGISTER = 10000,
	EVENT_IO_DISPATCH_READ_REGISTER  = 10001,
	EVENT_IO_DISPATCH_GET_FRAME      = 10002,
	EVENT_IO_DISPATCH_STANDBY        = 10003,
	EVENT_IO_DISPATCH_WAKEUP         = 10004,
	EVENT_IO_DISPATCH_GET_FRAME2 = 10005,
	EVENT_IO_DISPATCH_SENSOR_SELECT = 10006,
} core_callback_event_t;


int32_t io_7xx_dispatch_wakeup();
int32_t io_7xx_dispatch_standby();
int32_t io_7xx_dispatch_sensor_select(uint16_t sensor_sel);

#endif
