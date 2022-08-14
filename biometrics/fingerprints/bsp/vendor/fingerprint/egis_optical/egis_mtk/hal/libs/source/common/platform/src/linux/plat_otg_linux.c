#ifdef __OTG_SENSOR__

#include <unistd.h>
#include "egis_definition.h"
#include "type_definition.h"
#include "plat_log.h"
#include "plat_heap.h"
#include "plat_otg.h"

#define LOG_TAG "RBS-PLAT-OTG"

event_callback_t g_event_callback = NULL;

static void event_callback(int event_id, int first_param, int second_param, unsigned char *data, int data_size)
{
	// egislog_v("event_callback enter g_event_callback = %p event_id = %d", g_event_callback, event_id);
	if (NULL != g_event_callback) {
		g_event_callback(event_id, first_param, second_param, data, data_size);
	}
}

int32_t io_7xx_dispatch_write_register(uint16_t addr, uint8_t value)
{
	// egislog_v("%s: is called", __func__);
	uint8_t data[2];
	data[0] = addr & 0xFF;
	data[1] = value;
	event_callback(EVENT_IO_DISPATCH_WRITE_REGISTER, 0, 0, data, 2);
	return EGIS_OK;
}

int32_t io_7xx_dispatch_read_register(uint16_t addr, uint8_t *value)
{
	// egislog_v("%s: is called", __func__);
	uint8_t data[1];
	data[0] = addr & 0xFF;
	event_callback(EVENT_IO_DISPATCH_READ_REGISTER, 0, 0, data, 1);
	*value = data[0];
	return EGIS_OK;
}

int32_t io_7xx_dispatch_get_frame(uint8_t* buffer, uint32_t length, uint32_t frames)
{
	// egislog_v("%s: is called", __func__);
	event_callback(EVENT_IO_DISPATCH_GET_FRAME, (int)frames, 0, buffer, (int)length);
	return EGIS_OK;
}

int32_t io_7xx_dispatch_wakeup()
{
	egislog_d("%s: is called", __func__);
	event_callback(EVENT_IO_DISPATCH_WAKEUP, 0, 0, NULL, 0);
	return EGIS_OK;
}

int32_t io_7xx_dispatch_standby()
{
	egislog_d("%s: is called", __func__);
	event_callback(EVENT_IO_DISPATCH_STANDBY, 0, 0, NULL, 0);
	return EGIS_OK;
}

enum io_dispatch_cmd {
	IOCMD_READ_ZONE_AVERAGE = 700,
	IOCMD_READ_HISTOGRAM,
	IOCMD_READ_EFUSE,
};

int32_t io_dispatch_command_read(enum io_dispatch_cmd cmd, int param1, int param2, uint8_t* out_buf, int* out_buf_size)
{
	switch (cmd) {
		case IOCMD_READ_ZONE_AVERAGE:
		case IOCMD_READ_HISTOGRAM:
		case IOCMD_READ_EFUSE:
			egislog_e("%s [%d] not supported yet", __func__, cmd);
			break;

		default:
			egislog_e("%s [%d] not supported", __func__, cmd);
			break;
	}
	return EGIS_OK;
}

int32_t io_7xx_dispatch_sensor_select(uint16_t sensor_sel)
{
	egislog_d("%s: is called", __func__);

	// io_7xx_dispatch_sensor_select(0xFF33, sensor_sel);
	// // io_7xx_dispatch_write_register(0xFF34, sensor_sel);

	uint8_t data[4];
	data[0] = 0xFF;
	data[1] = 0x33;
	data[2] = (sensor_sel >> 8) & 0xFF;
	data[3] = sensor_sel & 0xFF;
	event_callback(EVENT_IO_DISPATCH_SENSOR_SELECT, 0, 0, data, 4);
	return EGIS_OK;
}

int io_dispatch_connect(HANDLE *device_handle)
{
	return EGIS_OK;
}

#endif
