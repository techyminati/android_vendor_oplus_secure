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

#ifndef __SILEAD_COMMON_H__
#define __SILEAD_COMMON_H__

#include "fingerprint.h"

int32_t silfp_common_init();
int32_t silfp_common_deinit();

int32_t silfp_common_cancel();
int32_t silfp_common_remove(uint32_t gid, uint32_t fid);
int32_t silfp_common_enumerate6(fingerprint_finger_id_t *results, uint32_t *max_size);
int32_t silfp_common_enumerate();
int32_t silfp_common_set_active_group(uint32_t gid, const char *store_path);

void silfp_common_set_dump_path(const char *path, int32_t force);
void silfp_common_set_cal_path(const char *path, int32_t force);
void silfp_common_set_ta_name(const char *taname, int32_t force);
void silfp_common_disable_capture(int32_t force);

#endif // __SILEAD_COMMON_H__