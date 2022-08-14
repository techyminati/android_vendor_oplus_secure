/******************************************************************************
 * @file   silead_notify_cust.h
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
 * <author>        <date>   <version>     <desc>
 * Sileadinc      2019/1/2    0.1.0      Init version
 * Bangxiong.Wu   2019/03/18  1.0.0      Add for hypnusd
 * Bangxiong.Wu   2019/4/10   1.0.1      Delete unused code and rename function
 * Bangxiong.Wu   2019/04/14  1.0.2      Add for algo and power monitor
 *****************************************************************************/

#ifndef __SILEAD_NOTIFY_CUST_H__
#define __SILEAD_NOTIFY_CUST_H__

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE

#include "fingerprint.h"

void silfp_notify_set_cust_notify_callback(fingerprint_notify_t notify);
void silfp_cust_notify_send_fingerprint_cmd_notice(int32_t sub_id, int8_t *result, uint32_t len);
void silfp_cust_notify_send_fingerprint_cmd_3v_notice(int32_t sub_id, int32_t ret, int32_t v, int32_t v2, int32_t v3);
void silfp_cust_notify_send_fp_cmd_with_1v_notice(int32_t cmd_id, int32_t ret);
int32_t silfp_cust_notify_cal_test_result_notice(int32_t subcmd,int32_t __unused reserve);
void silfp_cust_notify_send_finger_press_status(int32_t status);
void silfp_cust_notify_send_fingerprint_img_quality_notice(int32_t result, int32_t quality);
void silfp_cust_notify_fingerprint_enumerate(void);

#ifdef FP_HYPNUSD_ENABLE
void silfp_cust_notify_send_hypnusdsetaction(int32_t action_type, int32_t action_timeout);
#endif

void silfp_cust_notify_send_auth_dcsmsg(fingerprint_auth_dcsmsg_t auth_context);
void silfp_cust_notify_send_monitor_power_notice(double battery);

#endif

#endif // __SILEAD_NOTIFY_H__
