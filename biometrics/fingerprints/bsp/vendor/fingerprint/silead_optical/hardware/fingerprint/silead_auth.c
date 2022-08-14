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
 * <author>      <date>      <version>     <desc>
 * David Wang    2018/7/2    0.1.0      Init version
 * Bangxiong.Wu  2019/04/01  1.0.0      add reverse gery time
 * Bangxiong.Wu  2019/04/01  1.0.1      notify when mismatch
 * Bangxiong.Wu  2019/04/02  1.0.2      Move getting timestamp to TA
 * Bangxiong.Wu  2019/04/03  1.0.3      Finger move too fast,reset retry time and auth step
 * Bangxiong.WU  2019/04/14  1.0.4      Add for algo and power monitor
 * Bangxiong.Wu  2019/04/17  1.0.5      Notify acquireInfo when move too fast
 * Bangxiong.Wu  2019/04/18  1.0.6      Add authenticate notice for other error
 * Bangxiong.Wu  2019/05/07  1.0.7      remove authenticate notice when other error
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
#include "silead_dump.h"
#include "silead_stats.h"
#include "silead_util.h"
#include "silead_notify.h"
#include "fingerprint.h"
#include "silead_notify_cust.h"

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

#if 0 //by bangxiong.wu,move getting timestamp to TA for fixing resetPassword timeout bug
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        hat.timestamp = htobe64((uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif

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
    int32_t retry_time = 0;
    int32_t reverse_grey_times = 0;
    int32_t auth_max_time = 0;
    fingerprint_auth_dcsmsg_t auth_context;
    memset(&auth_context, 0 ,sizeof(fingerprint_auth_dcsmsg_t));

    LOG_MSG_INFO("authenticate-------------");

    ret = silfp_impl_auth_start();
    if (ret >= 0) {
        do {
            silfp_cust_clear_ui_ready();
            if (silfp_impl_is_wait_finger_up_need()) {
                ret = silfp_impl_wait_finger_up_with_cancel();
            }

            ret = silfp_impl_wait_finger_down_with_cancel();
            if(ret >= 0) {
                ret = silfp_cust_wait_ui_ready();
            }

            if (ret >= 0) {
                auth_step = 0;
                retry_max = silfp_cust_auth_get_retry_times();
                reverse_grey_times = silfp_cust_auth_get_reverse_grey_times();
                if (retry_max < 1) {
                    retry_max = 1;
                }

                retry_time = ((reverse_grey_times | 0x80) << 16) & 0xFFFF0000; //0x80 mean enable reverse grep

                /*authenticate start*/
                for (i = 0; i < retry_max; i++) {
                    silfp_stats_start();

                    ret = silfp_impl_capture_image(IMG_CAPTURE_AUTH, i);
                    if (ret >= 0) {
                        silfp_stats_capture_image();

                        auth_max_time = retry_time | (auth_step & 0x0000FFFF);
                        ret = silfp_impl_auth_step(m_auth_op_id, auth_max_time, m_is_pay_mode, (uint32_t *)&fid);
                        auth_step++;

                        silfp_dump_data((ret >=0) ? DUMP_IMG_AUTH_SUCC : DUMP_IMG_AUTH_FAIL);
                        if (ret >= 0) {
                            silfp_stats_auth_match();
                        } else {
                            silfp_stats_auth_mismatch();
                        }
                    }

                    if (ret >= 0) { // matched
                        break;
                    } else if (ret == -SL_ERROR_MOVE_TOO_FAST) { //move too fast
                        LOG_MSG_DEBUG("move too fast, not count during retry authenticate!");
                        i--;
                        auth_step--;
                    }

                    if (!silfp_impl_is_finger_down()) {
                        LOG_MSG_DEBUG("finger up, stop retry authenticate.");
                        break;
                    }
                }
            }
            /*authenticate end*/

            silfp_cust_monitor_power_reset();
            auth_context.retry_times = auth_step;
            auth_context.fail_reason = -ret;

            /*notify authenticate result start*/
            if (ret == -SL_ERROR_AUTH_MISMATCH) { // mismatch
                LOG_MSG_DEBUG("authenticate: mismatch");
                silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                _auth_send_authenticated_notice(0, m_gid);
                status = 0;
                auth_context.auth_result = 0;
                silfp_cust_notify_send_auth_dcsmsg(auth_context);
            } else if (_auth_check_mistouch(ret)) { // mis touch
                silfp_cust_monitor_power_start();
                if (silfp_worker_get_screen_state() || !silfp_cust_auth_mistouch_ignor_screen_off()) { //mis touch not ignor
                    LOG_MSG_DEBUG("authenticate: mis-touch, but not ignor");
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    switch (ret) {
                        case -SL_ERROR_COVERAREA_FAILED: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_PARTIAL);
                            break;
                        }
                        case -SL_ERROR_QUALITY_COVERAREA_FAILED:
                        case -SL_ERROR_QUALITY_FAILED: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                            break;
                        }
                        default: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                            break;
                        }
                    }
                    _auth_send_authenticated_notice(0, m_gid);
                    auth_context.auth_result = 0;
                    silfp_cust_notify_send_auth_dcsmsg(auth_context);
                } else { //mis touch ignor
                    LOG_MSG_DEBUG("authenticate: mis-touch, ignor");
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_INSUFFICIENT);
                }
            } else if (ret == -SL_ERROR_MOVE_TOO_FAST) { //move too fast
                LOG_MSG_DEBUG("authenticate: move too fast");
                silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_TOO_FAST);
            } else if (ret < 0) { //other error
                LOG_MSG_DEBUG("authenticate: err(%d)", -ret);
                if (ret != -SL_ERROR_CANCELED) {
                    silfp_notify_send_acquired_notice((fingerprint_acquired_info_t)(-ret));
                } else { // cancel
                    break;
                }
            } else { //matched
                LOG_MSG_DEBUG("authenticate: fid(%u) cancel:%d", fid, silfp_worker_is_canceled());
                if (!silfp_worker_is_canceled()) {
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    _auth_send_authenticated_notice(fid, m_gid);
                    silfp_worker_set_state_no_signal(STATE_IDLE);
                    status = 0;
                    auth_context.auth_result = 1;
                    silfp_cust_notify_send_auth_dcsmsg(auth_context);
                }
                break;
            }
            /*notify authenticate result end*/

            silfp_log_dump_ta_log();
            if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
                silfp_impl_wait_finger_up_with_cancel();
                silfp_cust_monitor_power_end();
            }
        } while (!silfp_worker_is_canceled());
    }

    LOG_MSG_DEBUG("authenticate finish-------------");
    silfp_impl_auth_end();

    silfp_impl_cal_base_sum();
    silfp_log_dump_ta_log();

    if (status >= 0) {
        silfp_impl_set_wait_finger_up_need(1);
        silfp_impl_wait_finger_up_with_cancel();
    }

    return ret;
}
