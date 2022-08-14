/******************************************************************************
 * @file   silead_fingerext_hub.h
 * @brief  Contains fingerprint extension hub operate functions header file.
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

#ifndef __SILEAD_EXT_HUB_H__
#define __SILEAD_EXT_HUB_H__

#ifdef SIL_FP_EXT_SKT_SERVER_ENABLE

void silfp_ext_hub_init(void);
void silfp_ext_hub_deinit(void);

int32_t silfp_ext_hub_send_response_raw(uint32_t cmdid, const void *data, size_t data_size);

#endif // SIL_FP_EXT_SKT_SERVER_ENABLE

#endif // __SILEAD_EXT_HUB_H__