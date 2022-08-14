/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_hal timer header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_HAL_TIMER_H_
#define _GF_HAL_TIMER_H_

#include <signal.h>
#include <sys/time.h>

#include "gf_error.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

typedef void (*timer_thread_t)(union sigval v);

gf_error_t gf_hal_create_timer(timer_t *timer_id, timer_thread_t timer_thread);
gf_error_t gf_hal_set_timer(timer_t *timer_id,
                            time_t interval_second, time_t value_second, int64_t value_nanosecond);
gf_error_t gf_hal_destroy_timer(timer_t *timer_id);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_TIMER_H_
