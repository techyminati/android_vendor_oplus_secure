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

#ifndef __SILEAD_NAV_H__
#define __SILEAD_NAV_H__

int32_t silfp_nav_init(void);
int32_t silfp_nav_deinit(void);

int32_t silfp_nav_check_support(void);
int32_t silfp_nav_command(void);
int32_t silfp_nav_set_mode(uint32_t mode);

void silfp_nav_set_key_mode(uint32_t enable);

#endif // __SILEAD_NAV_H__