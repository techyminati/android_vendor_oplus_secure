/******************************************************************************
 * @file   silead_calibrate.c
 * @brief  Contains fingerprint calibrate operate functions.
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

#define FILE_TAG "silead_cal"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_cal_normal.h"
#include "silead_cal_optic.h"
#include "silead_cal.h"

static int32_t m_is_optic = 0;

void silfp_cal_set_path(const void *path, uint32_t len)
{
    silfp_cal_optic_set_path(path, len);
    silfp_cal_normal_set_path(path, len);
}

int32_t silfp_cal_init(uint32_t feature, int32_t is_optic)
{
    int32_t ret = 0;

    m_is_optic = is_optic;

    if (!m_is_optic) {
        ret = silfp_cal_normal_init(feature);
    } else {
        ret = silfp_cal_optic_init(feature);
    }
    return ret;
}

int32_t silfp_cal_deinit(void)
{
    int32_t ret = 0;

    if (!m_is_optic) {
        ret = silfp_cal_normal_deinit();
    } else {
        ret = silfp_cal_optic_deinit();
    }
    return ret;
}

int32_t silfp_cal_calibrate(void)
{
    int32_t ret = 0;

    if (m_is_optic) {
        ret = silfp_cal_optic_calibrate();
    } else {
        ret = silfp_cal_normal_calibrate();
    }
    return ret;
}

int32_t silfp_cal_update_cfg(uint32_t status, int32_t reset)
{
    int32_t ret = 0;

    if (!m_is_optic) {
        ret = silfp_cal_normal_update_cfg(status, reset);
    }
    return ret;
}

int32_t silfp_cal_need_nav_cal(void)
{
    int32_t ret = 0;

    if (!m_is_optic) {
        ret = silfp_cal_normal_need_nav_cal();
    }
    return ret;
}

int32_t silfp_cal_reset(void)
{
    int32_t ret = -1;

    if (!m_is_optic) {
        ret = silfp_cal_normal_reset();
    }
    return ret;
}

int32_t silfp_cal_step(uint32_t step)
{
    int32_t ret = 0;

    if (m_is_optic) {
        ret = silfp_cal_optic_step(step);
    }
    return ret;
}

int32_t silfp_cal_base_sum()
{
    if (m_is_optic) {
        if (silfp_cal_optic_step(4) >= 0) {
            silfp_cal_optic_step(5);
        }
    }
    return 0;
}