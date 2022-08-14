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

#define FILE_TAG "silead_nav"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_error.h"
#include "silead_impl.h"
#include "silead_key.h"
#include "silead_worker.h"

static uint32_t m_nav_type = NAV_TYPE_NONE;

int32_t silfp_nav_init(void)
{
    m_nav_type = NAV_TYPE_NONE;
    silfp_impl_nav_support(&m_nav_type);
    m_nav_type &= 0x7F;

    LOG_MSG_INFO("nav support:%d", m_nav_type);

    return 0;
}

int32_t silfp_nav_deinit(void)
{
    return 0;
}

int32_t silfp_nav_check_support(void)
{
    return ((m_nav_type > NAV_TYPE_NONE && m_nav_type < NAV_TYPE_MAX)) ? 1 : 0;
}

static int32_t _nav_send_key_for_lite(uint32_t key, int32_t *status)
{
    int32_t ret = SL_SUCCESS;
    uint32_t up_key = 0;

    if (key == NAV_KEY_CLICK_DOWN) {
        silfp_impl_send_key(key);
        do {
            ret = silfp_impl_wait_finger_up_with_cancel();
            if (ret >= 0) {
                ret = silfp_impl_nav_capture_image();
                if (ret >= 0) {
                    ret = silfp_impl_nav_step(&up_key);
                    if (ret >= 0) {
                        if (up_key == NAV_KEY_CLICK_UP) {
                            silfp_impl_send_key(up_key);
                            break;
                        }
                    }
                }
            }
        } while (!silfp_worker_is_canceled());
    }

    if (status != NULL) {
        *status = -1;
    }

    return ret;
}

int32_t silfp_nav_command(void)
{
    int32_t ret = SL_SUCCESS;
    uint32_t key = 0;
    int32_t status = -1;

    LOG_MSG_INFO("nav-------------");

    ret = silfp_impl_nav_start();
    if (ret >= 0) {
        if (silfp_impl_is_wait_finger_up_need()) {
            ret = silfp_impl_wait_finger_up_with_cancel();
        }
        ret = silfp_impl_wait_finger_nav();
    }

    if (ret >= 0) {
        ret = silfp_impl_nav_capture_image();
        if (ret >= 0) {
            do {
                ret = silfp_impl_nav_step(&key);
                if (ret >= 0) {
                    if (IS_KEY_VALID(key)) {
                        LOG_MSG_VERBOSE("*********report key = %s", silead_key_get_des((int32_t)key));
                        if (m_nav_type == NAV_TYPE_LITE) {
                            ret = _nav_send_key_for_lite(key, &status);
                        } else {
                            if (m_nav_type == NAV_TYPE_NORMAL) {
                                silfp_impl_send_key(key);
                            }
                            status = 0;
                        }
                        break;
                    }
                } else { // some error or cancel
                    break;
                }
            } while (!silfp_worker_is_canceled());
        }
    }

    silfp_impl_nav_end();

    if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
        silfp_worker_set_state_no_signal(STATE_IDLE);
    }

    silfp_log_dump_ta_log();

    LOG_MSG_DEBUG("nav finish------------- %d", ret);

    if (status >= 0) {
        silfp_impl_wait_finger_up_with_cancel();
    }

    return ret;
}

inline int32_t silfp_nav_set_mode(uint32_t mode)
{
    return silfp_impl_nav_set_mode(mode);
}