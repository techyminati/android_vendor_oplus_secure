/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_keymaster header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_KEYMASTER_H_
#define _GF_KEYMASTER_H_

#include "gf_error.h"

#ifdef __cplusplus
extern "C"
{
#endif  // end ifdef __cplusplus

gf_error_t get_key_from_keymaster(uint8_t *rsp_buf, int32_t buf_len);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _GF_KEYMASTER_H_
