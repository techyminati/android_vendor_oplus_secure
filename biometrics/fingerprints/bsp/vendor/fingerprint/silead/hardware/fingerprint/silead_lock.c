/******************************************************************************
 * @file   silead_nav.c
 * @brief  Contains fingerprint navi operate functions.
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

#define FILE_TAG "silead_lock"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_cust.h"
#include "silead_error.h"
#include "silead_worker.h"
#include "silead_impl.h"

int32_t silfp_lock_mode_commond(void)
{
    int32_t ret = SL_SUCCESS;

    LOG_MSG_INFO("lock-------------");

    do {
        if (silfp_impl_is_wait_finger_up_need()) {
            ret = silfp_impl_wait_finger_up_with_cancel();
        }
        ret = silfp_impl_wait_finger_down_with_cancel();

        if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
            silfp_impl_wait_finger_up_with_cancel();
        }
    } while (!silfp_worker_is_canceled());

    LOG_MSG_DEBUG("lock finish-------------");

    if (ret != -SL_ERROR_CANCELED) {
        silfp_impl_set_wait_finger_up_need(1);
        silfp_impl_wait_finger_up_with_cancel();
    }

    return ret;
}

int32_t silfp_esd_mode_support(void)
{
    int32_t ret = silfp_cust_esd_support();
    if (ret < 0) {
        ret = !silfp_impl_is_optic() && !silfp_impl_is_unsupport_esd();
    }
    return ret;
}

int32_t silfp_esd_check_commond(void)
{
    int32_t ret = SL_SUCCESS;

    LOG_MSG_DEBUG("esd-------------");

    do {
        ret = silfp_impl_wait_irq_with_cancel();
    } while (!silfp_worker_is_canceled());

    LOG_MSG_DEBUG("esd finish-------------");

    return ret;
}