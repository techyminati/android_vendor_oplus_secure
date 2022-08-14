/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_ca entry header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_CA_ENTRY_H_
#define _GF_CA_ENTRY_H_

#include <gf_error.h>
#include <gf_type_define.h>
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

gf_error_t gf_ca_open_session(void);
void gf_ca_close_session(void);
gf_error_t gf_ca_invoke_command(uint32_t operation_id, uint32_t cmd_id, void *cmd, int32_t len);
void gf_ca_set_handle(int32_t fd);
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_CA_ENTRY_H_
