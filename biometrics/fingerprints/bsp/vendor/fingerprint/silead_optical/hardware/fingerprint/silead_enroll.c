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
 * Bangxiong.Wu 2019/03/23 1.0.0      Clear err_count when enroll success
 * Bangxiong.Wu 2019/04/20 1.0.1      Twice notify touch up once enroll step
 *****************************************************************************/

#define FILE_TAG "silead_enroll"
#define LOG_DBG_VERBOSE 0
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_worker.h"
#include "silead_impl.h"
#include "silead_cust.h"
#include "silead_dump.h"
#include "silead_stats.h"
#include "silead_util.h"
#include "silead_notify.h"

static int32_t m_enroll_pause = 0;
static uint64_t m_pre_enroll_time = 0; // seconds

static uint32_t m_gid = 0;

uint64_t silfp_enroll_pre_enroll()
{
    uint64_t challenge = 0;

    challenge = silfp_impl_load_enroll_challenge();
    LOG_MSG_VERBOSE("challenge = %" PRIu64, challenge);

    m_enroll_pause = 0;
    m_pre_enroll_time = silfp_util_get_seconds();

    return challenge;
}

int32_t silfp_enroll_enroll(const hw_auth_token_t *hat, uint32_t gid, uint32_t __unused timeout_sec)
{
    int32_t ret = 0;

    LOG_MSG_DEBUG("enroll(gid=%u, timeout_sec=%u)", gid, timeout_sec);

    if (hat == NULL) {
        silfp_notify_send_error_notice(FINGERPRINT_ERROR_HW_UNAVAILABLE);
        return SL_SUCCESS;
    }

    LOG_MSG_VERBOSE("hat(%" PRIu64 ":%" PRIu64 ":%" PRIu64 ")", hat->challenge, hat->user_id, hat->authenticator_id);

    ret = silfp_impl_verify_enroll_challenge((const void *)hat, sizeof(hw_auth_token_t), hat->user_id);
    if (ret < 0) {
        silfp_notify_send_error_notice(FINGERPRINT_ERROR_HW_UNAVAILABLE);
        return SL_SUCCESS;
    }

    m_gid = gid;
    silfp_worker_set_state(STATE_ENROLL);

    return SL_SUCCESS;
}

int32_t silfp_enroll_post_enroll(void)
{
    silfp_impl_set_enroll_challenge(0);

    LOG_MSG_DEBUG("post_enroll");
    return SL_SUCCESS;
}

int32_t silfp_enroll_pause(int32_t pause)
{
    LOG_MSG_VERBOSE("pause=%d, m_enroll_pause=%d", pause, m_enroll_pause);
    if (pause && (STATE_ENROLL == silfp_worker_get_state())) {
        m_enroll_pause = 1;
        silfp_worker_wakeup_condition();
    } else if ((!pause) && m_enroll_pause) {
        m_enroll_pause = 0;
        silfp_worker_wakeup_condition();
    }
    return 0;
}

int32_t silfp_enroll_command(void)
{
    int32_t ret = SL_SUCCESS;
    uint32_t remaining = 0;
    uint32_t fid = 0;
    int32_t status = -1;
    int32_t notice_code = 0;

    int32_t err_cont = 0;
    int32_t report_err_cont_times = 0;
    int32_t report_remain_when_error = 0;

    uint64_t enroll_time = 0;
    int32_t enroll_timeout_sec = 0;

    LOG_MSG_DEBUG("enroll-------------");

    silfp_impl_get_enroll_num(&remaining);
    if (remaining == 0) { // should not happend, just in case
        remaining = 999;
    }

    report_remain_when_error = silfp_cust_enroll_report_remain_when_error();
    report_err_cont_times = silfp_cust_enroll_cont_err_times();
    enroll_timeout_sec = silfp_cust_enroll_timeout_sec();

    if (enroll_timeout_sec > 0) { // need check timeout
        LOG_MSG_DEBUG("check enroll timeout (%d)", enroll_timeout_sec);
        enroll_time = silfp_util_get_seconds();
        if (enroll_time < m_pre_enroll_time || (int32_t)(enroll_time - m_pre_enroll_time) >= enroll_timeout_sec) {
            silfp_notify_send_error_notice(FINGERPRINT_ERROR_TIMEOUT);
            silfp_worker_set_state_no_signal(STATE_IDLE);
            LOG_MSG_DEBUG("enroll timeout, change to idle");
            return 0;
        }
    }

    ret = silfp_impl_enroll_start();
    if (ret >= 0) {
        do {
            if (m_enroll_pause) { // check enroll pause
                LOG_MSG_DEBUG("enroll pause, wait continue enroll");
                silfp_cust_tp_irq_enable(0);
                silfp_worker_wait_condition(-1);
                silfp_cust_tp_irq_enable(1);
                m_enroll_pause = 0;
                if (silfp_worker_is_canceled()) {
                    break;
                }
            }

            silfp_cust_clear_ui_ready();
            ret = silfp_impl_get_finger_down_with_cancel();
            if (ret >= 0) {
                if (m_enroll_pause) {
                    continue;
                }
            }

            if (ret >= 0) {
                ret = silfp_cust_wait_ui_ready();
                if (m_enroll_pause) {
                    continue;
                }
            }

            if (ret >= 0) {
                ret = silfp_impl_capture_image(IMG_CAPTURE_ENROLL, 0);
            }

            if (ret >= 0) {
                ret = silfp_impl_enroll_step(&remaining);
                silfp_dump_data((ret >= 0) ? DUMP_IMG_ENROLL_SUCC : DUMP_IMG_ENROLL_FAIL);
            }

            if (ret < 0) {
                if (ret != -SL_ERROR_MOVE_TOO_FAST) {
                    err_cont++;
                }

                LOG_MSG_DEBUG("enroll: err(%d)", -ret);
                if (ret == -SL_ERROR_CANCELED) { // canceled
                    break;
                } else if (report_err_cont_times > 0 && err_cont >= report_err_cont_times) {
                    LOG_MSG_DEBUG("err cont, report error notify");
                    silfp_notify_send_error_notice(FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
                } else {
                    notice_code = silfp_cust_trans_notice_code(ret);
                    switch (ret) {
                        case -SL_ERROR_EROLL_DUPLICATE: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_VENDOR_DUPLICATE_FINGER);
                            break;
                        }
                        case -SL_ERROR_SAME_AREA: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_VENDOR_SAME_AREA);
                            break;
                        }
                        case -SL_ERROR_COVERAREA_FAILED: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_PARTIAL);
                            break;
                        }
                        case -SL_ERROR_DETECT_NO_FINGER:
                        case -SL_ERROR_MOVE_TOO_FAST: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_TOO_FAST);
                            break;
                        }
                        case -SL_ERROR_FAKE_FINGER:
                        case -SL_ERROR_GAIN_IMPROVE_TIMEOUT:
                        case -SL_ERROR_SPI_TIMEOUT:
                        case -SL_ERROR_QUALITY_COVERAREA_FAILED:
                        case -SL_ERROR_QUALITY_FAILED: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                            break;
                        }
                        default: {
                            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_INSUFFICIENT);
                            break;
                        }
                    }
                    if ((report_remain_when_error > 0) && (ret != -SL_ERROR_EROLL_DUPLICATE)) {
                        LOG_MSG_DEBUG("need report remain when error (remaining:%d)", remaining);
                        silfp_notify_send_enroll_notice(0, m_gid, remaining);
                    }
                }
            } else {
                err_cont = 0;
                if (remaining > 0) {
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    silfp_notify_send_enroll_notice(0, m_gid, remaining);
                } else if (remaining == 0) { // enroll finish
                    break;
                }
            }

            silfp_log_dump_ta_log();
            if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
                silfp_impl_wait_finger_up_with_enroll();
            }
        } while (!silfp_worker_is_canceled());
    }

    if (silfp_impl_enroll_end(&fid) >= 0) { // return < 0, if not finish enroll or save tpl failed
        LOG_MSG_DEBUG("enroll: fid(%u) cancel:%d", fid, silfp_worker_is_canceled());
        //if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
            silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
            silfp_notify_send_enroll_notice(fid, m_gid, 0);

            silfp_cust_tpl_change_action();

            silfp_worker_set_state_no_signal(STATE_IDLE);
            status = 0;
        //}
    }
    LOG_MSG_DEBUG("enroll finish-------------");

    silfp_impl_cal_base_sum();
    silfp_log_dump_ta_log();

    if (status >= 0) {
        silfp_impl_set_wait_finger_up_need(1);
        silfp_impl_wait_finger_up_with_cancel();
    }

    return ret;
}
