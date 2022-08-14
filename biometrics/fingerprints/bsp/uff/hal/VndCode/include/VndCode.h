/*******************************************************************************************
 * Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
 *
 * File: VndCode.h
 * Description: implement the string function
 * Version: 1.0
 * Date : 2021-4-2
 * Author:  meiwei
 ** -----------------------------Revision History: -----------------------
 **  <author>      <date>            <desc>
 **  Wei.mei  2021/05/06        create the file
 *********************************************************************************************/

#ifndef VNDCODE_H
#define VNDCODE_H
// #define LOG_TAG "[FP_HAL][VND_CODE]"
#include <stdlib.h>
#include <string.h>
#include "FpCommon.h"
#include "FpType.h"
#include "HalLog.h"

typedef struct {
    vnd_cmd_t code;
    fp_return_type_t (*func)(vnd_code_t *);
} f_func_t;

void send_message_to_disaplay(vnd_code_t code);

extern f_func_t func_maps[];

#define excute_vnd_func(vnd_code) ({ \
    tmp = func_maps[vnd_code.code].func(&vnd_code); \
    tmp; \
})

#define __decorate(func, vnd_code, ...) ({\
    fp_return_type_t tmp; \
    memset(&vnd_code, 0, sizeof(vnd_code)); \
    vnd_code.code = E_CODE_NONE; \
    while (1) {\
    tmp = func(__VA_ARGS__); \
    LOG_E(LOG_TAG, "vnd_code:%s, %s, %d", vnd_code.apk_msg, vnd_code.vnd_description, vnd_code.need_reported); \
    if (vnd_code.need_reported) { \
        send_message_to_disaplay(vnd_code); \
    } \
    if (vnd_code.code == E_CODE_NONE || vnd_code.code >= E_MAX_CODE) { \
        LOG_D(LOG_TAG, "break for code none"); \
        break; \
    } \
    if (tmp != 0) { \
        LOG_D(LOG_TAG, "break for exception"); \
        break;\
    } \
    tmp = excute_vnd_func(vnd_code); \
    if (tmp) { \
        LOG_D(LOG_TAG, "break for excute_vnd_func exception");\
        break; \
    } \
    vnd_code.response_value = (int)tmp; \
    } \
    tmp; \
})

#endif //VNDCODE_H

