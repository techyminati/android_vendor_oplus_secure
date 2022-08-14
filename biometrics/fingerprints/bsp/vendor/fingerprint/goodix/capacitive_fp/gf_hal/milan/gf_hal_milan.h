/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: HAL function initialization for series
 * History: None
 * Version: 1.0
 */
#ifndef _GF_HAL_MILAN_H_
#define _GF_HAL_MILAN_H_

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "gf_error.h"
#include "gf_hal_common.h"

extern int64_t g_irq_time_milan;
extern timer_t g_key_long_pressed_timer_id;
extern uint8_t g_sensor_power_down;  // power down flag
extern uint8_t g_sensor_disable;

gf_error_t gf_hal_milan_function_customize(gf_hal_function_t
                                                    *hal_function);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_MILAN_H_
