/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef _GF_HAL_MILAN_A_SERIES_H_
#define _GF_HAL_MILAN_A_SERIES_H_

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "gf_error.h"
#include "gf_hal_common.h"

extern int64_t g_irq_time_milan_a;

gf_error_t gf_hal_milan_a_series_function_customize(gf_hal_function_t
                                                    *hal_function);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_MILAN_A_SERIES_H_
