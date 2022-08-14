/******************************************************************************
 * @file   silead_dump.h
 * @brief  Contains dump image header file.
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
 * Luke Ma         2018/7/2    0.1.0      Init version
 * Bangxiong.Wu    2019/3/10   1.0.0      Add for saving calibrate data
 *****************************************************************************/

#ifndef __SILEAD_DUMP_H__
#define __SILEAD_DUMP_H__

#ifdef SIL_DUMP_IMAGE

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
#include "silead_cmd_cust.h"
#include "silead_txt.h"
#endif

typedef enum {
    DUMP_IMG_AUTH_SUCC = 0,
    DUMP_IMG_AUTH_FAIL,
    DUMP_IMG_ENROLL_SUCC,
    DUMP_IMG_ENROLL_FAIL,
    DUMP_IMG_NAV_SUCC,
    DUMP_IMG_NAV_FAIL,
    DUMP_IMG_SHOT_SUCC,
    DUMP_IMG_SHOT_FAIL,
    DUMP_IMG_RAW,
    DUMP_IMG_CAL,
    DUMP_IMG_FT_QA,
    DUMP_IMG_AUTH_ORIG,
    DUMP_IMG_ENROLL_ORIG,
    DUMP_IMG_OTHER_ORIG,
    DUMP_IMG_SNR,
    DUMP_IMG_ENROLL_NEW,
    DUMP_IMG_MAX,
} e_mode_dump_img_t;

void silfp_dump_data(e_mode_dump_img_t type);
void silfp_dump_deinit(void);

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
void silfp_dump_algorithm_parameter(uint32_t step, algotirhm_paramater_t *para, int32_t reserve);
#endif

#else /* undef SIL_DUMP_IMAGE */

#define silfp_dump_data(a) ((void)0)
#define silfp_dump_deinit() ((void)0)

#endif /* SIL_DUMP_IMAGE */

void silfp_dump_set_path(const void *path, uint32_t len);
void silfp_dump_deadpx_result(const char *desc, uint32_t value);

#endif /* __SILEAD_DUMP_H__ */
