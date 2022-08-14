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

#ifndef __SILEAD_NOTIFY_H__
#define __SILEAD_NOTIFY_H__

#include "fingerprint.h"

void silfp_notify_set_notify_callback(fingerprint_notify_t notify);
fingerprint_notify_t silfp_notify_get_notify_callback(void);

void silfp_notify_send_error_notice(int32_t error);
void silfp_notify_send_enroll_notice(uint32_t fid, uint32_t gid, uint32_t remaining);
void silfp_notify_send_removed_notice(uint32_t fid, uint32_t gid, uint32_t remaining);
void silfp_notify_send_acquired_notice(int32_t acquired);
void silfp_notify_send_authenticated_notice(uint32_t fid, uint32_t gid, hw_auth_token_t *hat);
void silfp_notify_send_enumerate_notice(uint32_t fid, uint32_t gid, uint32_t remaining);

#endif // __SILEAD_NOTIFY_H__