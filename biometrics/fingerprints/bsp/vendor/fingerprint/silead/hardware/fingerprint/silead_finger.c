/******************************************************************************
 * @file   silead_finger.c
 * @brief  Contains fingerprint operate functions.
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

#define FILE_TAG "silead_finger"
#include "log/logmsg.h"

#include "silead_const.h"
#include "silead_error.h"
#include "silead_worker.h"
#include "silead_ext_skt.h"
#include "silead_ext_cb.h"
#include "silead_nav.h"
#include "silead_enroll.h"
#include "silead_auth.h"
#include "silead_common.h"
#include "silead_notify.h"
#include "silead_finger.h"
#include <cutils/properties.h>
#ifdef SIL_DEBUG_MEM_LEAK
#include "heaptracker.h"
#endif

uint64_t silfp_finger_pre_enroll()
{
    return silfp_enroll_pre_enroll();
}

int silfp_finger_enroll(const hw_auth_token_t *hat, uint32_t gid, uint32_t timeout_sec)
{
    return silfp_enroll_enroll(hat, gid, timeout_sec);
}

int silfp_finger_post_enroll()
{
    silfp_enroll_post_enroll();
    return SL_SUCCESS;
}

int silfp_finger_authenticate(uint64_t operation_id, uint32_t gid)
{
    return silfp_auth_authenticate(operation_id, gid);
}

int silfp_finger_authenticate_ext(uint64_t operation_id, uint32_t gid, uint32_t is_pay)
{
    return silfp_auth_authenticate_ext(operation_id, gid, is_pay);
}

uint64_t silfp_finger_get_auth_id()
{
    return silfp_auth_get_auth_id();
}

int silfp_finger_cancel()
{
    return silfp_common_cancel();
}

int silfp_finger_remove(uint32_t gid, uint32_t fid)
{
    return silfp_common_remove(gid, fid);
}

int silfp_finger_enumerate6(fingerprint_finger_id_t *results, uint32_t *max_size)
{
    return silfp_common_enumerate6(results, max_size);
}

int silfp_finger_enumerate()
{
    return silfp_common_enumerate();
}

int silfp_finger_set_active_group(uint32_t gid, const char *store_path)
{
    return silfp_common_set_active_group(gid, store_path);
}

int silfp_finger_set_notify_callback(fingerprint_notify_t notify)
{
    silfp_notify_set_notify_callback(notify);

    return SL_SUCCESS;
}

int silfp_finger_init()
{
    int ret = 0;

#ifdef SIL_DEBUG_MEM_LEAK
    heaptracker_init();
#endif

    silfp_common_sync_log_dump_path();
    silfp_log_dump_init(); // init log dump

    LOG_MSG_INFO("init");

    silfp_worker_init();
    ret = silfp_common_init();
    if (ret >= 0) {
        silfp_nav_init();
        silfp_ext_skt_init();
        silfp_ext_cb_init();

        ret = silfp_worker_run();
    }
	property_set("persist.sys.fp.vendor","silead");
	LOG_MSG_INFO("%s property_set persist.sys.fp.vendor = silead", __func__);
    return ret;
}

int silfp_finger_deinit()
{
    silfp_worker_deinit();

    silfp_ext_cb_deinit();
    silfp_ext_skt_deinit();
    silfp_nav_deinit();
    silfp_log_dump_deinit();
    silfp_common_deinit();

    LOG_MSG_VERBOSE("deinit");

#ifdef SIL_DEBUG_MEM_LEAK
    heaptracker_deinit();
#endif
    return 0;
}

void silfp_finger_set_dump_path(const char *path)
{
    silfp_common_set_dump_path(path, 1);
}

void silfp_finger_set_cal_path(const char *path)
{
    silfp_common_set_cal_path(path, 1);
}

void silfp_finger_set_ta_name(const char *taname)
{
    silfp_common_set_ta_name(taname, 1);
}

void silfp_finger_capture_disable(void)
{
    silfp_common_disable_capture(1);
}

void silfp_finger_set_key_mode(uint32_t enable)
{
    silfp_nav_set_key_mode(enable);
}