/******************************************************************************
 * @file   silead_cal_optic.h
 * @brief  Contains fingerprint calibrate operate functions header file.
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

#ifndef __SILEAD_CALIBRATE_NORMAL_H__
#define __SILEAD_CALIBRATE_NORMAL_H__

void silfp_cal_normal_set_path(const void *path, uint32_t len);
int32_t silfp_cal_normal_init(uint32_t feature);
int32_t silfp_cal_normal_deinit(void);
int32_t silfp_cal_normal_calibrate(void);
int32_t silfp_cal_normal_update_cfg(uint32_t status, int32_t reset);
int32_t silfp_cal_normal_need_nav_cal(void);
int32_t silfp_cal_normal_reset(void);

#endif /* __SILEAD_CALIBRATE_NORMAL_H__ */
