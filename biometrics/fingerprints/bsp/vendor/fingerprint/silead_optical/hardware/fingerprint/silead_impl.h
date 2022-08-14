/******************************************************************************
 * @file   silead_impl.h
 * @brief  Contains CA interfaces header file.
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
 * David Wang  2018/5/15   0.1.1      Support get finger status
 * Jack Zhang  2018/5/17   0.1.2      Change test process to simplify app use
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * David Wang  2018/6/5    0.1.6      Support wakelock & pwdn
 * Rich Li     2018/6/7    0.1.7      Support dump image
 * Jack Zhang  2018/6/15   0.1.8      Add read OTP I/F.
 * Rich Li     2018/7/2    0.1.9      Add algo set param command ID.
 * Bangxiong.Wu 2019/04/20 1.0.0      Add wait finger up for enroll
 *
 *****************************************************************************/

#ifndef __SILEAD_IMPL_H__
#define __SILEAD_IMPL_H__

#include "silead_screen.h"

void sileadHypnusSetAction();
void silfp_impl_set_ta_name(const void *name, uint32_t len);
void silfp_impl_set_capture_dump_flag(int32_t addition);

int32_t silfp_impl_set_wait_finger_up_need(int32_t need);
int32_t silfp_impl_is_wait_finger_up_need(void);

int32_t silfp_impl_get_screen_status(uint8_t *status);
int32_t silfp_impl_set_screen_cb(screen_cb listen, void *param);
int32_t silfp_impl_set_finger_status_mode(int32_t mode);

void silfp_impl_wait_clean(void);
void silfp_impl_cancel(void);
void silfp_impl_sync_finger_status_optic(int32_t down);

int32_t silfp_impl_wait_finger_down(void);
int32_t silfp_impl_wait_finger_down_with_cancel(void);
int32_t silfp_impl_wait_finger_up(void);
int32_t silfp_impl_wait_finger_up_with_cancel(void);
int32_t silfp_impl_wait_finger_up_with_enroll(void);
int32_t silfp_impl_wait_finger_nav(void);
int32_t silfp_impl_wait_irq_with_cancel(void);

int32_t sl_init_tp_touch_info();
int32_t sl_get_tp_touch_info(uint8_t mode);

int32_t silfp_impl_get_finger_down_with_cancel(void);
int32_t silfp_impl_is_finger_down(void);

int32_t silfp_impl_capture_image(int32_t type, uint32_t step);
int32_t silfp_impl_nav_capture_image(void);

int32_t silfp_impl_auth_start(void);
int32_t silfp_impl_auth_step(uint64_t op_id, uint32_t step, uint32_t is_pay, uint32_t *fid);
int32_t silfp_impl_auth_end(void);

int32_t silfp_impl_get_enroll_num(uint32_t *num);
int32_t silfp_impl_enroll_start(void);
int32_t silfp_impl_enroll_step(uint32_t *remaining);
int32_t silfp_impl_enroll_end(uint32_t *fid);

int32_t silfp_impl_nav_support(uint32_t *type);
int32_t silfp_impl_nav_start(void);
int32_t silfp_impl_nav_step(uint32_t *key);
int32_t silfp_impl_nav_end(void);
int32_t silfp_impl_send_key(uint32_t key);
int32_t silfp_impl_nav_set_mode(uint32_t mode);

int32_t silfp_impl_download_normal();
int32_t silfp_impl_init();
int32_t silfp_impl_deinit(void);

int32_t silfp_impl_set_gid(uint32_t gid);
int32_t silfp_impl_load_user_db(const char *db_path);
int32_t silfp_impl_remove_finger(uint32_t fid);
int32_t silfp_impl_get_db_count(void);
int32_t silfp_impl_get_finger_prints(uint32_t *ids, uint32_t count);

int64_t silfp_impl_load_enroll_challenge(void);
int32_t silfp_impl_set_enroll_challenge(uint64_t challenge);
int32_t silfp_impl_verify_enroll_challenge(const void *hat, uint32_t size, int64_t sid);
int64_t silfp_impl_load_auth_id(void);
int32_t silfp_impl_get_hw_auth_obj(void *buffer, uint32_t length);
int64_t silfp_impl_get_sec_id(uint32_t fid);

int32_t silfp_impl_capture_image_pre(void);
int32_t silfp_impl_capture_image_raw(int32_t type, uint32_t step);
int32_t silfp_impl_capture_image_after(int32_t type, uint32_t step);

int32_t silfp_impl_set_touch_info(void *buffer, uint32_t len);
int32_t silfp_impl_get_otp(void);
int32_t silfp_impl_chip_pwdn(void);

int32_t silfp_impl_wakelock(uint8_t lock);

int32_t silfp_impl_is_optic(void);
int32_t silfp_impl_calibrate(void);
int32_t silfp_impl_cal_base_sum(void);
int32_t silfp_impl_cal_step(uint32_t step);
int32_t silfp_impl_cal_reset(void);
void silfp_impl_cal_set_path(const void *path, uint32_t len);

#endif /* __SILEAD_IMPL_H__ */
