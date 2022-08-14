/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_HAL_DUMP_H_
#define _GF_HAL_DUMP_H_

#include "gf_error.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

gf_error_t gf_hal_dump_cmd(void *dev, uint32_t cmd_id, const uint8_t *param,
                           uint32_t param_len);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_DUMP_H_
