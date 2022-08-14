/******************************************************************************
 * @file   silead_ext_cb_cust.c
 * @brief  Contains fingerprint extension operate functions.
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
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
#define FILE_TAG "silead_ext_cb_cust"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_impl.h"
#include "silead_worker.h"
#include "silead_cust.h"
#include "silead_ext.h"
#include "silead_ext_cb.h"
#include "silead_notify_cust.h"

//Silead <SIL_FP> <lyman.xue> <2018-12-29> Added Begin
int32_t silfp_cust_ext_cb_spi_test(int32_t __unused cmd, int32_t __unused subcmd)
{
    int32_t ret = 0;
    uint32_t chipid = 0, subid = 0;
    uint32_t result = 0;
    uint32_t deadpixelnum = 0;
    uint32_t badlinenum = 0;

    ret = silfp_ext_test_spi(&chipid, &subid);
    if (ret >= 0) {
        LOG_MSG_ERROR("silead main chip id = %#08x, sub chip id = %#08x", chipid, subid);
        int32_t optic = silfp_impl_is_optic();
        ret = silfp_ext_test_deadpx(optic, &result, &deadpixelnum, &badlinenum);
        if (ret >= 0) {
            if (!result) {
                LOG_MSG_ERROR("silead deadpixel test success");
            } else {
                LOG_MSG_ERROR("silead deadpixel test fail, deadpixelnum = %d, badlinenum = %d", deadpixelnum, badlinenum);
                ret = -1;
            }
        } else {
            LOG_MSG_ERROR("silead deadpixel test error");
        }
    } else {
        LOG_MSG_ERROR("silead spi test error");
    }
    if (!silfp_worker_is_canceled()) {
        silfp_worker_set_state_no_signal(STATE_IDLE);
    }
    return ret;
}
#endif
//Silead <SIL_FP> <lyman.xue> <2018-12-29> Added End

