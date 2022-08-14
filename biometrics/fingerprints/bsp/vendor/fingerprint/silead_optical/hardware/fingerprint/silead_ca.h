/******************************************************************************
 * @file   silead_ca.h
 * @brief  Contains CA functions header file.
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

#ifndef __SILEAD_CA_H__
#define __SILEAD_CA_H__

int32_t silfp_ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t flag, uint32_t v1, uint32_t v2, uint32_t *data1, uint32_t *data2);
int32_t silfp_ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4, uint32_t *data1, uint32_t *data2, uint32_t *data3);
int32_t silfp_ca_open(const void *ta_name);
int32_t silfp_ca_close(void);
int32_t silfp_ca_keymaster_get(void **buffer);

#endif /* __SILEAD_CA_H__ */

