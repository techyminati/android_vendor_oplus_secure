#ifndef FP_MONITOR_TOOL_H
#define FP_MONITOR_TOOL_H
#include "Sensor.h"
#include "Device.h"

using namespace goodix;

/* data */
bool is_rawdata_mode();
void fp_monitor_init(Device *dev);
void save_rawdata(int rawdata, int retry_count, int round, int screen_state);
#endif
