/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: milan chip particular dump
 * History:
 */
#ifndef _GF_MILAN_DUMP_H_
#define _GF_MILAN_DUMP_H_

#include "gf_hal_common.h"


#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus


bool gf_hal_milan_dump_chip_init_data();

bool gf_hal_milan_dump_chip_operation_data(gf_dump_data_t *dump_data,
                                       gf_operation_type_t operation,
                                       timeval_t* tv,
                                       gf_error_t error_code,
                                       gf_chip_type_t chip_type);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_MILAN_DUMP_H_

