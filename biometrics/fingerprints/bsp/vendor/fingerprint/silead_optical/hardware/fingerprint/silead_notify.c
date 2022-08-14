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

#define FILE_TAG "silead_notify"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>

#include "silead_notify.h"
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
#include "silead_notify_cust.h"
#endif
static fingerprint_notify_t m_notify_func = NULL;

void silfp_notify_set_notify_callback(fingerprint_notify_t notify)
{
    LOG_MSG_VERBOSE("set notify");

    if(NULL != notify) {
        m_notify_func = notify;
    }
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
    silfp_notify_set_cust_notify_callback(notify);
#endif
}

fingerprint_notify_t silfp_notify_get_notify_callback(void)
{
    return m_notify_func;
}

void silfp_notify_send_error_notice(int32_t error)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_ERROR;
    msg.data.error = (fingerprint_error_t)error;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_notify_send_enroll_notice(uint32_t fid, uint32_t gid, uint32_t remaining)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
    msg.data.enroll.finger.fid = fid;
    msg.data.enroll.finger.gid = gid;
    msg.data.enroll.samples_remaining = remaining;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_notify_send_removed_notice(uint32_t fid, uint32_t gid, uint32_t remaining)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_TEMPLATE_REMOVED;
    msg.data.removed.finger.fid = fid;
    msg.data.removed.finger.gid = gid;
    msg.data.removed.remaining_templates = remaining;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_notify_send_acquired_notice(int32_t acquired)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_ACQUIRED;
    msg.data.acquired.acquired_info = (fingerprint_acquired_info_t)acquired;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_notify_send_authenticated_notice(uint32_t fid, uint32_t gid, hw_auth_token_t *hat)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_AUTHENTICATED;
    msg.data.authenticated.finger.fid = fid;
    msg.data.authenticated.finger.gid = gid;

    if (fid != 0 && NULL != hat) {
        memcpy(&(msg.data.authenticated.hat), hat, sizeof(hw_auth_token_t));
    }

    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}

void silfp_notify_send_enumerate_notice(uint32_t fid, uint32_t gid, uint32_t remaining)
{
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
    msg.data.enumerated.finger.fid = fid;
    msg.data.enumerated.finger.gid = gid;
    msg.data.enumerated.remaining_templates = remaining;
    if (m_notify_func != NULL) {
        m_notify_func(&msg);
    }
}