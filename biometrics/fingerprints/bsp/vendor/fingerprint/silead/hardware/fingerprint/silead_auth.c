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

#define FILE_TAG "silead_auth"
#define LOG_DBG_VERBOSE 0
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <endian.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_worker.h"
#include "silead_impl.h"
#include "silead_cust.h"
#include "silead_stats.h"
#include "silead_util.h"
#include "silead_notify.h"
#include "silead_dump.h"

static uint32_t m_gid = 0;
static uint64_t m_auth_op_id = 0;
static uint64_t m_auth_id = 0;
static uint32_t m_is_pay_mode = 0;

uint64_t silfp_auth_get_auth_id()
{
    m_auth_id = silfp_impl_load_auth_id();
    m_auth_id = htobe64(m_auth_id);

    LOG_MSG_VERBOSE("m_auth_id = %" PRIu64, m_auth_id);

    return m_auth_id;
}

int32_t silfp_auth_authenticate(uint64_t operation_id, uint32_t gid)
{
    LOG_MSG_VERBOSE("(sid=%" PRIu64 ", gid=%d)", operation_id, gid);

    m_gid = gid;
    m_auth_op_id = operation_id;
    m_is_pay_mode = 0;
    silfp_worker_set_state(STATE_SCAN);

    return SL_SUCCESS;
}

int32_t silfp_auth_authenticate_ext(uint64_t operation_id, uint32_t gid, uint32_t is_pay)
{
    LOG_MSG_VERBOSE("(sid=%" PRIu64 ", gid=%d, is_pay=%d)", operation_id, gid, is_pay);

    m_gid = gid;
    m_auth_op_id = operation_id;
    m_is_pay_mode = is_pay;
    silfp_worker_set_state(STATE_SCAN);

    return SL_SUCCESS;
}

static void _auth_send_authenticated_notice(uint32_t fid, uint32_t gid)
{
    hw_auth_token_t hat;
    memset(&hat, 0, sizeof(hat));

    if (fid != 0) {
        hat.challenge = m_auth_op_id;
        hat.authenticator_type = htobe32(HW_AUTH_FINGERPRINT);
        hat.version = HW_AUTH_TOKEN_VERSION;
        hat.authenticator_id = m_auth_id;
        hat.user_id = silfp_impl_get_sec_id(fid);

        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        hat.timestamp = htobe64((uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

        silfp_impl_get_hw_auth_obj(&hat, sizeof(hat));
        LOG_MSG_VERBOSE("challenge=%" PRIu64 " op_id=%" PRIu64 " sec_id=%" PRIu64, hat.challenge, m_auth_op_id, hat.user_id);
    }

    silfp_notify_send_authenticated_notice(fid, gid, &hat);
}

int32_t _auth_check_mistouch(int32_t err)
{
    int32_t ret = 0;
    switch(err) {
    case -SL_ERROR_COVERAREA_FAILED:
    case -SL_ERROR_QUALITY_FAILED:
    case -SL_ERROR_QUALITY_COVERAREA_FAILED:
    case -SL_ERROR_FAKE_FINGER: {
        ret = 1;
        break;
    }
    default: {
        ret = 0;
        break;
    }
    }
    return ret;
}

int32_t silfp_auth_command(void)
{
    int32_t ret = SL_SUCCESS;
    int32_t fid = -1;
    int32_t status = -1;

    int32_t i = 0;
    int32_t retry_max = 0;
    int32_t auth_step = 0;
    int32_t ret_bak = SL_SUCCESS;
    int32_t retry_times = 0;
    int32_t auth_result = 0;
    int32_t fail_reason = FINGERPRINT_ACQUIRED_GOOD;
    int32_t fake_finger = 0;

    LOG_MSG_INFO("authenticate-------------");

    ret = silfp_impl_auth_start();
    if (ret >= 0) {
        do {
            silfp_cust_clear_ui_ready();
            if (silfp_impl_is_wait_finger_up_need() || silfp_impl_is_narrow_mode()) {
                ret = silfp_impl_wait_finger_up_with_cancel();
            }

            ret = silfp_impl_wait_finger_down_with_cancel();
            if(ret >= 0) {
                ret = silfp_cust_wait_ui_ready();
            }

            if (ret >= 0) {
                auth_step = 0;
                retry_max = silfp_impl_get_ta_retry_loop()?silfp_impl_get_ta_retry_loop():silfp_cust_auth_get_retry_times();
                if (retry_max < 1) {
                    retry_max = 1;
                }

                for (i = 0; i < retry_max; i++) {
                    retry_times = i;
                    silfp_stats_start();

                    ret = silfp_impl_capture_image(IMG_CAPTURE_AUTH, i);
                    if (ret >= 0) {
                        silfp_stats_capture_image();

                        ret = silfp_impl_auth_step(m_auth_op_id, auth_step, m_is_pay_mode, (uint32_t *)&fid);
                        auth_step++;

                        if (ret >= 0) {
                            silfp_stats_auth_match();
                        } else {
                            silfp_stats_auth_mismatch();
                        }
                    }

                    if (ret >= 0) { // matched
                        break;
                    } else {
                        if (ret_bak == -SL_ERROR_AUTH_MISMATCH && ret != -SL_ERROR_FAKE_FINGER) {
                            ret = ret_bak;
                        }
                    }

                    if (!silfp_impl_is_finger_down()) {
                        LOG_MSG_DEBUG("finger up, stop retry authenticate.");
                        break;
                    }

                    ret_bak = ret;
                }
                silfp_dump_data(DUMP_IMG_AUTH_MUL_RAW);
            }

            fake_finger = 0;
            fail_reason = FINGERPRINT_ACQUIRED_GOOD;
            if (ret == -SL_ERROR_AUTH_MISMATCH) { // mismatch
                LOG_MSG_DEBUG("authenticate: mismatch");
                silfp_cust_auth_mismatch();
                _auth_send_authenticated_notice(0, m_gid);
                fail_reason = FINGERPRINT_ACQUIRED_INSUFFICIENT;
                silfp_dump_data(DUMP_IMG_AUTH_FAIL);
            } else if (ret == -SL_ERROR_FAKE_FINGER) {
                fake_finger = 1;
                LOG_MSG_DEBUG("authenticate: mis-touch, fake finger");
                silfp_dump_data(DUMP_IMG_AUTH_FAIL);
            } else if (_auth_check_mistouch(ret)) { // mis touch
                if (silfp_worker_get_screen_state() || !silfp_cust_auth_mistouch_ignor_screen_off()) {
                    LOG_MSG_DEBUG("authenticate: mis-touch, but not ignor");
                    switch (ret) {
                    case -SL_ERROR_COVERAREA_FAILED: {
                        silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_PARTIAL);
                        fail_reason = FINGERPRINT_ACQUIRED_PARTIAL;
                        break;
                    }
                    case -SL_ERROR_QUALITY_COVERAREA_FAILED:
                    case -SL_ERROR_QUALITY_FAILED: {
                        silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                        fail_reason = FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
                        break;
                    }
                    default: {
                        silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                        fail_reason = FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
                        break;
                    }
                    }
                    _auth_send_authenticated_notice(0, m_gid);
                } else {
                    LOG_MSG_DEBUG("authenticate: mis-touch, ignor");
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_INSUFFICIENT);
                    fail_reason = FINGERPRINT_ACQUIRED_INSUFFICIENT;
                }
            } else if (ret < 0) {
                LOG_MSG_DEBUG("authenticate: err(%d)", -ret);
                if (ret != -SL_ERROR_CANCELED) {
                    switch (ret) {
                    case -SL_ERROR_DETECT_NO_FINGER:
                    case -SL_ERROR_MOVE_TOO_FAST: {
                        silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_TOO_FAST);
                        fail_reason = FINGERPRINT_ACQUIRED_TOO_FAST;
                        break;
                    }
                    default: {
                        _auth_send_authenticated_notice(0, m_gid);
                        silfp_dump_data(DUMP_IMG_AUTH_FAIL);
                    }
                    }
                } else { // cancel
                    break;
                }
            } else {
                LOG_MSG_DEBUG("authenticate: fid(%u) cancel:%d", fid, silfp_worker_is_canceled());
                if (!silfp_worker_is_canceled()) {
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    _auth_send_authenticated_notice(fid, m_gid);
                    silfp_worker_set_state_no_signal(STATE_IDLE);
                    auth_result = 1;
                    status = 0;
                }
                silfp_notify_send_dcsmsg(auth_result, fail_reason, retry_times, NULL);
                silfp_dump_data(DUMP_IMG_AUTH_SUCC);
                break;
            }

            if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
                ret = silfp_impl_wait_finger_up_with_cancel();
                if (ret == -SL_ERROR_CANCELED) {
                    status = 0;
                }
            }
            if(!fake_finger)
            silfp_notify_send_dcsmsg(auth_result, fail_reason, retry_times, NULL);
        } while (!silfp_worker_is_canceled());
    }

    LOG_MSG_DEBUG("authenticate finish-------------");
    silfp_impl_auth_end();

    silfp_impl_cal_base_sum();

    if (status >= 0) {
        silfp_impl_set_wait_finger_up_need(1);
        silfp_impl_wait_finger_up_with_cancel();
    }

    return ret;
}