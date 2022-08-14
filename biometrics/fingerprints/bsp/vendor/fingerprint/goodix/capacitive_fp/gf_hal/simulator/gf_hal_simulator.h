/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL layer simulator chip
 * History:
 * Version: 1.0
 */
#ifndef _GF_HAL_SIMULATOR_H_
#define _GF_HAL_SIMULATOR_H_

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "gf_error.h"
#include "gf_hal_common.h"

typedef void (*notify)(const gf_fingerprint_msg_t *msg);
extern timer_t g_key_long_pressed_timer_id_fps;

gf_error_t gf_hal_simulator_function_customize(gf_hal_function_t
                                                    *hal_function);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_SIMULATOR_H_
