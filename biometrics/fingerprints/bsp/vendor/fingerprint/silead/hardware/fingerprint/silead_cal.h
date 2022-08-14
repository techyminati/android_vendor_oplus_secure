/******************************************************************************
 * @file   silead_calibrate.h
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

#ifndef __SILEAD_CALIBRATE_H__
#define __SILEAD_CALIBRATE_H__

#define FP_CONFIG_CHG_BIT   0x0001
#define FP_CONFIG_UP_BIT    0x0002
#define FP_CONFIG_DOWN_BIT  0x0004

void silfp_cal_set_path(const void *path, uint32_t len);
int32_t silfp_cal_init(uint32_t feature, int32_t is_optic);
int32_t silfp_cal_deinit(void);
int32_t silfp_cal_calibrate(void);
int32_t silfp_cal_update_cfg(uint32_t status, int32_t reset);
int32_t silfp_cal_need_nav_cal(void);
int32_t silfp_cal_reset(void);

int32_t silfp_cal_step(uint32_t step);
int32_t silfp_cal_base_sum();

#endif /* __SILEAD_CALIBRATE_H__ */
