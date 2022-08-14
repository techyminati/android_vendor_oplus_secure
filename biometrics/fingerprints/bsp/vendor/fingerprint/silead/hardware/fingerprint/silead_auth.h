/******************************************************************************
 * @file   silead_finger.h
 * @brief  Contains fingerprint operate functions header file.
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
 * David Wang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_AUTH_H__
#define __SILEAD_AUTH_H__

uint64_t silfp_auth_get_auth_id();
int32_t silfp_auth_authenticate(uint64_t operation_id, uint32_t gid);
int32_t silfp_auth_authenticate_ext(uint64_t operation_id, uint32_t gid, uint32_t is_pay);
int32_t silfp_auth_command(void);

#endif // __SILEAD_AUTH_H__