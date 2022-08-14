#ifndef __ANC_COMMAND_H__
#define __ANC_COMMAND_H__

#include "anc_type.h"
#include "sensor_command.h"
#include "algorithm_command.h"
#include "auth_token_command.h"
#include "extension_command.h"

typedef enum {
    ANC_CMD_NONE = 0,
    ANC_CMD_GET_VERSION,
    ANC_CMD_SENSOR,
    ANC_CMD_ALGORITHM,
    ANC_CMD_AUTH_TOKEN,
    ANC_CMD_EXTENSION,
    ANC_CMD_DO_TEST,
    ANC_CMD_TEST_SHARE_BUFFER,
    ANC_CMD_MAX
}ANC_COMMAND_TYPE;


typedef union{
    AncSensorCommand sensor;
    AncAlgorithmCommand algo;
    AncAuthTokenCommand token;
    AncExtensionCommand extension;
}AncCommandData;


typedef struct {
    uint32_t size;
    uint64_t share_buffer_ptr;  // must be 64 bit for both 32 bit and 64 bit TA
    uint32_t share_buffer_length;
    uint32_t id;
    AncCommandData data;
} __attribute__((packed)) AncSendCommand;



typedef union{
    uint32_t data;
    AncSensorCommandRespond sensor;
    AncAlgorithmCommandRespond algo;
    AncAuthTokenCommandRespond token;
    AncExtensionCommandRespond extension;
}AncCommandRespondData;

typedef struct {
    int32_t status;
    AncCommandRespondData respond;
} __attribute__((packed)) AncSendCommandRespond;


#endif
