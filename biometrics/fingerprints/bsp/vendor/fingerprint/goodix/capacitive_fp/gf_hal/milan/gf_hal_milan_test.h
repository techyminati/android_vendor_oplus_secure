/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: IRQ command test for milan series
 * History: None
 * Version: 1.0
 */
#ifndef _GF_HAL_MILAN_TEST_H_
#define _GF_HAL_MILAN_TEST_H_

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "gf_error.h"
#include "gf_hal_common.h"

extern void gf_hal_milan_irq_test(gf_irq_t *cmd, gf_error_t *err_code);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_MILAN_TEST_H_
