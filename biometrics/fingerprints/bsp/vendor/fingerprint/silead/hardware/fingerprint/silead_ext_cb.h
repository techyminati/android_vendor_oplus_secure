/******************************************************************************
 * @file   silead_ext_cb.h
 * @brief  Contains fingerprint extension operate functions header file.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Jack Zhang  2018/5/18    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_EXT_CB_H__
#define __SILEAD_EXT_CB_H__

typedef int32_t (*func_cb) (int32_t cmd, int32_t subcmd);

int32_t silfp_ext_cb_init(void);
int32_t silfp_ext_cb_deinit(void);

int32_t silfp_ext_cb_request(int32_t cmd, int32_t subcmd, func_cb cb);
int32_t silfp_ext_cb_command(void);
int32_t silfp_ext_cb_request_sync(int32_t cmd, int32_t timeout_sec);

#endif /* __SILEAD_EXT_CB_H__ */

