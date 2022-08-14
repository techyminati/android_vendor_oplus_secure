/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef _GF_HAL_OSWEGO_M_H_
#define _GF_HAL_OSWEGO_M_H_

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "gf_error.h"
#include "gf_hal_common.h"

extern int64_t g_irq_time;

gf_error_t gf_hal_oswego_m_function_customize(gf_hal_function_t *hal_function);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_OSWEGO_M_H_
