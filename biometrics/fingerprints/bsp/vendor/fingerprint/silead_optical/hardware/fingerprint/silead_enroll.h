/******************************************************************************
 * @file   silead_nav.h
 * @brief  Contains fingerprint navi operate functions header file.
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

#ifndef __SILEAD_ENROLL_H__
#define __SILEAD_ENROLL_H__

#include "fingerprint.h"

uint64_t silfp_enroll_pre_enroll(void);
int32_t silfp_enroll_enroll(const hw_auth_token_t *hat, uint32_t gid, uint32_t timeout_sec);
int32_t silfp_enroll_post_enroll(void);

int32_t silfp_enroll_pause(int32_t pause);
int32_t silfp_enroll_command(void);

#endif // __SILEAD_ENROLL_H__