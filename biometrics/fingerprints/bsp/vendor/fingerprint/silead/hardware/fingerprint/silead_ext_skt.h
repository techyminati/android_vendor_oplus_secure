/******************************************************************************
 * @file   silead_fingerext.h
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
 * David Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_EXT_SOCKET_H__
#define __SILEAD_EXT_SOCKET_H__

#ifdef SIL_FP_EXT_SKT_SERVER_ENABLE

int32_t silfp_ext_skt_init(void);
int32_t silfp_ext_skt_deinit(void);

int32_t silfp_ext_skt_test_cmd(const uint8_t *param, int32_t len);
int32_t silfp_ext_skt_commond(void);

int32_t silfp_ext_skt_capture_dump(int32_t orig, int32_t enroll, uint32_t step, int32_t result);

#else

#define silfp_ext_skt_init(void) (0)
#define silfp_ext_skt_deinit(void) (0)
int32_t silfp_ext_skt_commond(void);

#define silfp_ext_skt_capture_dump(o, e, s, r) (0)

#endif // SIL_FP_EXT_SKT_SERVER_ENABLE

#endif // __SILEAD_EXT_SOCKET_H__