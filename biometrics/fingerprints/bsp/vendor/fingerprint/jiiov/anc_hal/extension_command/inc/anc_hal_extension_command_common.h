#ifndef __ANC_HAL_EXTENSION_COMMAND_COMMON_H__
#define __ANC_HAL_EXTENSION_COMMAND_COMMON_H__

#include <stdint.h>




typedef struct {
    int32_t command_id;

    struct Command{
        uint8_t *p_buffer;
        uint32_t buffer_length;
    }command;

    struct CommandRespond{
        int32_t argument;
        uint8_t *p_buffer;
        uint32_t buffer_length;
    }command_respond;
}AncHalExtensionCommand;

#endif
