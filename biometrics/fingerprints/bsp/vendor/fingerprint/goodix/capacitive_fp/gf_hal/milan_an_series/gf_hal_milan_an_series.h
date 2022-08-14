/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: the header file for milan an series
 * History:
 * Version: 1.0
 */
#ifndef _GF_HAL_MILAN_AN_SERIES_H_
#define _GF_HAL_MILAN_AN_SERIES_H_

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "gf_error.h"
#include "gf_hal_common.h"

extern int64_t g_irq_time_milan_an;

gf_error_t gf_hal_milan_an_series_function_customize(gf_hal_function_t
                                                     *hal_function);
gf_error_t hal_milan_an_series_irq();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_MILAN_AN_SERIES_H_
