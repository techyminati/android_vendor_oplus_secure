/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/ActionType.h
 **
 ** Description:
 **     action type define for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _ACTIONTYPE_H_
#define _ACTIONTYPE_H_

#include <hardware/hw_auth_token.h>
#include <stddef.h>
#include <stdint.h>

typedef enum action_type {
    FP_INIT = 1,
    FP_SETNOTIFY,
    FP_SETACTIVEGROUP,
    FP_ENUMERATE,
    FP_PREENROLL,
    FP_ENROLL_ACTION,
    FP_POSTENROLL,
    FP_REMOVE,
    FP_AUTH_ACTION,
    FP_CANCEL,

    // ADD BY OPPO
    FP_GET_ENROLL_TOTALTIMES,
    FP_SET_SCREEN_STATE,
    FP_SETCONFIG,
    FP_TOUCHDOWN,
    FP_TOUCHUP,
    FP_SENDFPCMD,

    //DEVICE_EVENT
    FP_EVENT_BASE,
    FP_EVENT_IRQ,
    FP_EVENT_SCREEN_OFF,
    FP_EVENT_SCREEN_ON,
    FP_EVENT_TP_TOUCHDOWN,
    FP_EVENT_TP_TOUCHUP,
    FP_EVENT_UI_READY,
    FP_EVENT_UI_DISAPPEAR,
    FP_EVENT_EXIT,
    FP_EVENT_INVALID,
    FP_EVENT_MAX,

    FP_END,
} action_type_t;

typedef struct enroll_message {
    hw_auth_token_t* authToken;
    uint32_t gid;
    uint32_t timeoutSec;
} enroll_message_t;

typedef struct authenticate_message {
    uint64_t operationId;
    uint32_t gid;
    uint32_t authtype;
} authenticate_message_t;

typedef struct remove_message {
    uint32_t gid;
    uint32_t fid;
} remove_message_t;

typedef struct set_screenstate_message {
    uint32_t state;
} set_screenstate_message_t;

typedef struct send_fpcmd_message {
    uint32_t cmdid;
    int8_t*  in_buff_data;
    uint32_t in_buff_size;
} send_fpcmd_message_t;

typedef struct set_active_group_message {
    uint32_t gid;
    char*    path;
} set_active_group_message_t;

typedef struct action_message {
    action_type_t type;
    union {
        enroll_message_t           enroll;
        authenticate_message_t     authenticate;
        remove_message_t           remove;
        set_screenstate_message_t  screenstate;
        send_fpcmd_message_t       sendfpcmd;
        set_active_group_message_t setactivegroup;
    } data;
} action_message_t;

#endif  // _ACTIONTYPE_H_
