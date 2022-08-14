/******************************************************************************
 * @file   silead_ext_cb_cust.h
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
 * Sileadinc  2019/1/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_EXT_CB_CUST_H__
#define __SILEAD_EXT_CB_CUST_H__
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
int32_t silfp_cust_ext_cb_spi_test(int32_t __unused cmd, int32_t __unused subcmd);
#endif

#endif /* __SILEAD_EXT_CB_H__ */

