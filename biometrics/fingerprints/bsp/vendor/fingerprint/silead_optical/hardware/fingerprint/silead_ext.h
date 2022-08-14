/******************************************************************************
 * @file   silead_ext.h
 * @brief  Contains fingerprint extension operate functions header file.
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
 * Jack Zhang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_EXT_H__
#define __SILEAD_EXT_H__

void silfp_ext_set_capture_image(int32_t enable);
void silfp_ext_set_path(const void *path, uint32_t len);

int32_t silfp_ext_test_spi(uint32_t *chipid, uint32_t *subid);
int32_t silfp_ext_test_deadpx(int32_t optic, uint32_t *result, uint32_t *deadpx, uint32_t *badline);
int32_t silfp_ext_test_self(int32_t optic, uint32_t *result);
int32_t silfp_ext_test_flash(void);
int32_t silfp_ext_test_otp(uint32_t *otp1, uint32_t *otp2, uint32_t *otp3);

int32_t silfp_ext_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *max_size, uint32_t *w_ori, uint32_t *h_ori, uint8_t *bitcount, uint8_t *bitcount_orig);
int32_t silfp_ext_test_image_test_init(uint32_t mode, uint32_t count, uint32_t step);
int32_t silfp_ext_test_image_test(uint32_t mode, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl, uint8_t *greyavg, uint8_t *greymax);
int32_t silfp_ext_test_send_group_image(uint32_t orig, uint32_t frr, uint32_t imgtype, void *buffer, uint32_t *len);
int32_t silfp_ext_test_image_finish(void);

int32_t silfp_ext_test_get_ta_ver(uint32_t *algoVer, uint32_t *taVer);

int32_t silfp_ext_optic_test_factory_quality(uint32_t *result, uint32_t *quality, uint32_t *length);
int32_t silfp_ext_optic_test_snr(uint32_t *result, uint32_t *snr, uint32_t *noise, uint32_t *signal);

int32_t silfp_ext_test_speed(void *buffer, uint32_t *len);
int32_t silfp_ext_test_reset(void);

#endif /* __SILEAD_EXT_H__ */

