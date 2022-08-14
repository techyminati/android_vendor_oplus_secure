#ifndef __SENSOR_COMMAND_H__
#define __SENSOR_COMMAND_H__

#include "anc_type.h"
#include "sensor_command_param.h"

typedef enum {
    ANC_CMD_SENSOR_NONE = 0,
    ANC_CMD_SENSOR_INIT,
    ANC_CMD_SENSOR_DEINIT,
    ANC_CMD_SENSOR_SET_PARAM,
    ANC_CMD_SENSOR_GET_PARAM,
    ANC_CMD_SENSOR_CAPTURE_IMAGE,
    ANC_CMD_SENSOR_GET_CHIP_ID,
    ANC_CMD_SENSOR_SET_POWER_MODE,
    ANC_CMD_SENSOR_MAX
}ANC_COMMAND_SENSOR_TYPE;


typedef struct {
    uint32_t command;
    int32_t data;
    AncSensorCommandParam param;
    int32_t response;
} __attribute__((packed)) AncSensorCommand;


typedef struct {
    uint32_t data;
    AncSensorCommandParam param;
} __attribute__((packed)) AncSensorCommandRespond;

#endif
