/******************************************************************************
 * @file   silead_cmd.h
 * @brief  Contains CA communication interfaces.
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
 * <author>     <date>      <version>     <desc>
 * David Wang   2018/7/20    0.1.0      Init version
 * Bangxiong.Wu 2019/06/21   1.0.0      modify for sync tpl to tee
 *
 *****************************************************************************/

#ifndef __SILEAD_COMMOND_H__
#define __SILEAD_COMMOND_H__

#include "silead_dev.h"

int32_t silfp_cmd_wait_finger_status(uint32_t status, uint32_t charging);
int32_t silfp_cmd_capture_image(int32_t type, uint32_t step);
int32_t silfp_cmd_nav_capture_image(void);

int32_t silfp_cmd_auth_start(void);
int32_t silfp_cmd_auth_step(uint64_t op_id, uint32_t step, uint32_t is_pay, uint32_t *fid);
int32_t silfp_cmd_auth_end(void);

int32_t silfp_cmd_get_enroll_num(uint32_t *num);
int32_t silfp_cmd_enroll_start(void);
int32_t silfp_cmd_enroll_step(uint32_t *remaining);
int32_t silfp_cmd_enroll_end(int32_t status, uint32_t *fid);

int32_t silfp_cmd_nav_support(uint32_t *type);
int32_t silfp_cmd_nav_start(void);
int32_t silfp_cmd_nav_step(uint32_t *key);
int32_t silfp_cmd_nav_end(void);

int32_t silfp_cmd_deep_sleep_mode(void);
int32_t silfp_cmd_download_normal(void);
int32_t silfp_cmd_set_env(const void *ta_name);
int32_t silfp_cmd_init(fp_dev_conf_t *dev_conf, uint32_t *chipid, uint32_t *subid, uint32_t *vid, uint32_t *update_cfg);
int32_t silfp_cmd_deinit(void);

int32_t silfp_cmd_set_gid(uint32_t gid, uint32_t serial);
int32_t silfp_cmd_load_user_db(const char *db_path, uint32_t *need_sync);
int32_t silfp_cmd_remove_finger(uint32_t fid);
int32_t silfp_cmd_get_db_count(void);
int32_t silfp_cmd_get_finger_prints(uint32_t *ids, uint32_t count);

int64_t silfp_cmd_load_enroll_challenge(void);
int32_t silfp_cmd_set_enroll_challenge(uint64_t challenge);
int32_t silfp_cmd_verify_enroll_challenge(const void *hat, uint32_t size);
int64_t silfp_cmd_load_auth_id(void);
int32_t silfp_cmd_get_hw_auth_obj(void *buffer, uint32_t length);

int32_t silfp_cmd_update_cfg(const void *buffer, uint32_t len);
int32_t silfp_cmd_init2(const void *buffer, uint32_t len, uint32_t *feature, uint32_t *tpl_size);

int32_t silfp_cmd_load_template(uint32_t fid, const void *buffer, uint32_t len);
int32_t silfp_cmd_save_template(void *buffer, uint32_t *plen);
int32_t silfp_cmd_update_template(void *buffer, uint32_t *plen, uint32_t *fid);

int32_t silfp_cmd_set_log_mode(uint8_t tam, uint8_t agm);
int32_t silfp_cmd_set_nav_mode(uint32_t mode);

int32_t silfp_cmd_capture_image_pre(void);
int32_t silfp_cmd_capture_image_raw(int32_t type, uint32_t step);
int32_t silfp_cmd_capture_image_after(int32_t type, uint32_t step);

int32_t silfp_cmd_check_esd(void);
int32_t silfp_cmd_get_finger_status(uint32_t *status, uint32_t *addition);
int32_t silfp_cmd_get_otp(uint32_t *otp1, uint32_t *otp2, uint32_t *otp3);
int32_t silfp_cmd_send_cmd_with_buf(uint32_t cmd, const void *buffer, uint32_t len);
int32_t silfp_cmd_send_cmd_with_buf_and_get(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result);

int32_t silfp_cmd_get_config(void *buffer, uint32_t *plen);
int32_t silfp_cmd_calibrate(void *buffer, uint32_t len);
int32_t silfp_cmd_calibrate2(void);
int32_t silfp_cmd_calibrate_optic(uint32_t step, void *buffer, uint32_t *plen, uint32_t flag);

int32_t silfp_cmd_get_ta_ver(uint32_t *algoVer, uint32_t *taVer);
int32_t silfp_cmd_get_chipid(uint32_t *chipId, uint32_t *subId);

int32_t silfp_cmd_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *max_size, uint32_t *w_ori, uint32_t *h_ori, uint8_t *bitcount, uint8_t *bitcount_orig);
int32_t silfp_cmd_test_dump_data(uint32_t type, uint32_t step, void *buffer, uint32_t len, uint32_t *remain, uint32_t *size, uint32_t *w, uint32_t *h, uint8_t *bitcount);

int32_t silfp_cmd_test_image_capture(uint32_t mode, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl, uint8_t *greyavg, uint8_t *greymax);
int32_t silfp_cmd_test_send_group_image(uint32_t orig, uint32_t frr, uint32_t imgtype, void *buffer, uint32_t *len);
int32_t silfp_cmd_test_image_finish(void);

int32_t silfp_cmd_test_deadpx(uint32_t *result, uint32_t *deadpx, uint32_t *badline);
int32_t silfp_cmd_test_speed(void *buffer, uint32_t *len);

#endif /* __SILEAD_COMMOND_H__ */

