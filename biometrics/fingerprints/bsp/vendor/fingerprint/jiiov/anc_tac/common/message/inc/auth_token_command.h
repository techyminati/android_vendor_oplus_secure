#ifndef __AUTH_TOKEN_COMMAND_H__
#define __AUTH_TOKEN_COMMAND_H__

#include "anc_type.h"

typedef enum {
    ANC_CMD_TOKEN_NONE = 0,
    ANC_CMD_TOKEN_SET_HMAC_KEY,
    ANC_CMD_TOKEN_GET_ENROLL_CHALLENGE,
    ANC_CMD_TOKEN_AUTHORIZE_ENROLL,
    ANC_CMD_TOKEN_SET_AUTHENTICATE_CHALLENGE,
    ANC_CMD_TOKEN_GET_AUTHENTICATE_RESULT,
    ANC_CMD_TOKEN_MAX
}ANC_COMMAND_TOKEN_TYPE;


typedef struct {
    uint32_t command;
    int32_t response;
    uint64_t challenge;
    uint32_t size;
    uint8_t array[256];
} __attribute__((packed)) AncAuthTokenCommand;


typedef struct {
    int32_t data;
    uint64_t challenge;
    uint32_t size;
    uint8_t array[256];
} __attribute__((packed)) AncAuthTokenCommandRespond;

#endif
