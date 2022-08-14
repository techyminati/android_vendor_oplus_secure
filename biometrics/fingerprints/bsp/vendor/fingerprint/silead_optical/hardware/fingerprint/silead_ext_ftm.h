/******************************************************************************
 * @file   silead_ext_ftm.h
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
 * calvin Wang  2018/10/30    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_EXT_FTM_H__
#define __SILEAD_EXT_FTM_H__

typedef enum _ext_ftm_type {
    EXT_FTM_TEST_SPI = 0,
    EXT_FTM_TEST_RESET,
    EXT_FTM_TEST_DEAD_PIXEL,
    EXT_FTM_TEST_VERSION,
    EXT_FTM_TEST_FLASH,
} ext_ftm_type_t;

// result: 0 pass, other failed
int32_t silfp_ext_ftm_test_entry(uint32_t item, uint32_t *result);
int32_t silfp_ext_ftm_init(void);
void silfp_ext_ftm_deinit(void);

#endif /* __SILEAD_EXT_FTM_H__ */

