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

#define FILE_TAG "silead_comm"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_impl.h"
#include "silead_cust.h"
#include "silead_worker.h"
#include "silead_notify.h"
#include "silead_util.h"
#include "silead_dump.h"
#include "silead_ext.h"
#include "silead_common.h"

#define ANDROID_6_SDK_VER 23
#define ANDROID_8_SDK_VER 26
#define BELOW_8_DEFAULT_PATH "/data/system/silead"

static pthread_mutex_t m_lock;

uint32_t m_gid = 0;
static int32_t m_need_send_cancel_notice = 0;
static int32_t m_below_android_8_platform = 0;

#define CFG_TYPE_DUMP_PATH_MASK 0x01
#define CFG_TYPE_CAL_PATH_MASK  0x02
#define CFG_TYPE_TA_NAME_MASK   0x04
#define CFG_TYPE_CAPTURE_MASK   0x08

static uint32_t m_cfg_set_type = 0;

static int32_t _common_is_mask_unset(int32_t mask)
{
    return (((m_cfg_set_type & mask) == 0) ? 1 : 0);
}

static inline void _common_set_mask(int32_t mask)
{
    m_cfg_set_type |= mask;
}

static void _common_set_finger_status_mode(void)
{
    int32_t mode = silfp_cust_get_finger_status_mode();
    if (mode < 0) {
        if (silfp_impl_is_optic()) {
            mode = SIFP_FINGER_STATUS_ANDROID;
        } else {
            mode = SIFP_FINGER_STATUS_IRQ;
        }
    }
    LOG_MSG_VERBOSE("finger status mode: %d", mode);

    silfp_impl_set_finger_status_mode(mode);
}

static void _common_set_screen_callback_mode(void)
{
    uint8_t status = 0;

    if (silfp_cust_is_screen_from_drv()) {
        silfp_impl_get_screen_status(&status);
        silfp_worker_set_screen_state_default(status);

        LOG_MSG_VERBOSE("get screen status from drv");
        silfp_impl_set_screen_cb(silfp_worker_screen_state_callback, NULL);
    } else {
        /* wrong screen status after kill bio service, if not get screen status from kernel (oplus) */
        silfp_impl_get_screen_status(&status);
        silfp_worker_set_screen_state_default(status);
    }
}

static int32_t _common_need_cancel_notice(void)
{
    int32_t ret = 0;
    int32_t prop = 0;

    ret = silfp_cust_need_cancel_notice();
    if (ret < 0) {
        prop = silfp_util_get_str_value("ro.build.version.sdk", 0xFF);
        if (prop != ANDROID_6_SDK_VER) {
            ret = 1;
        }
    }

    return ret;
}

static int32_t _common_is_below_8_platform(void)
{
    int32_t ret = 0;
    int32_t prop = 0;

    prop = silfp_util_get_str_value("ro.build.version.sdk", 0xFF);
    if (prop < ANDROID_8_SDK_VER) {
        ret = 1;
    }

    return ret;
}

static void _common_set_default_path(void)
{
    const char *path = NULL;
    int32_t capture_disable = 0;

    if (_common_is_mask_unset(CFG_TYPE_DUMP_PATH_MASK)) {
        path = silfp_cust_get_dump_path();
        if (path != NULL) {
            silfp_common_set_dump_path(path, 0);
        } else {
            if (m_below_android_8_platform) {
                silfp_common_set_dump_path(BELOW_8_DEFAULT_PATH, 0);
            }
        }
    }

    if (_common_is_mask_unset(CFG_TYPE_CAL_PATH_MASK)) {
        path = silfp_cust_get_cal_path();
        if (path != NULL) {
            silfp_common_set_cal_path(path, 0);
        } else {
            if (m_below_android_8_platform) {
                silfp_common_set_cal_path(BELOW_8_DEFAULT_PATH, 0);
            }
        }
    }

    if (_common_is_mask_unset(CFG_TYPE_CAPTURE_MASK)) {
        capture_disable = silfp_cust_is_capture_disable();
        if (capture_disable) {
            silfp_common_disable_capture(0);
        }
    }
}

static void _common_set_default_ta_name(void)
{
    const char *name = NULL;

    if (_common_is_mask_unset(CFG_TYPE_TA_NAME_MASK)) {
        name = silfp_cust_get_ta_name();
        if (name != NULL) {
            silfp_common_set_ta_name(name, 0);
        }
    }
}

int32_t silfp_common_init()
{
    int32_t ret = 0;

    LOG_MSG_VERBOSE("init");

    pthread_mutex_init(&m_lock, NULL);
    m_need_send_cancel_notice = _common_need_cancel_notice();
    m_below_android_8_platform = _common_is_below_8_platform();
    LOG_MSG_INFO("cancel notice (%d), below android8 (%d)", m_need_send_cancel_notice, m_below_android_8_platform);

    _common_set_default_ta_name();
    ret = silfp_impl_init();
    if (ret >= 0) {
        _common_set_default_path();
        _common_set_screen_callback_mode();
        _common_set_finger_status_mode();
    }

    return ret;
}

int32_t silfp_common_deinit()
{
    silfp_impl_deinit();

    pthread_mutex_destroy(&m_lock);
    LOG_MSG_VERBOSE("deinit");

    return 0;
}

int32_t silfp_common_cancel()
{
    LOG_MSG_VERBOSE("cancel()");

    silfp_worker_set_state(STATE_IDLE);

    if (m_need_send_cancel_notice) {
        silfp_notify_send_error_notice(FINGERPRINT_ERROR_CANCELED);
    }

    return SL_SUCCESS;
}

int32_t silfp_common_remove(uint32_t gid, uint32_t fid)
{
    int32_t ret = 0;
    int32_t i = 0;
    int32_t count = 0;
    uint32_t *ids = NULL;

    LOG_MSG_VERBOSE("remove(fid=%u, gid=%u)", fid, gid);

    pthread_mutex_lock(&m_lock);
    if (fid == 0) {
        count = silfp_impl_get_db_count();
        if (count > 0) {
            ids = (uint32_t *)malloc(sizeof(uint32_t) * count);
            if (NULL != ids) {
                memset(ids, 0, sizeof(uint32_t) * count);
                count = silfp_impl_get_finger_prints(ids, count);
                for (i = 0; i < count; i++) {
                    ret = silfp_impl_remove_finger(ids[i]);
                    LOG_MSG_DEBUG("removeall(%u, %d)", ids[i], ret);
                    silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                    silfp_notify_send_removed_notice(ids[i], gid, count - i - 1);
                }
                free(ids);
                if (m_below_android_8_platform) {
                    silfp_notify_send_removed_notice(fid, gid, 0);
                }
            } else {
                silfp_notify_send_removed_notice(fid, gid, 0);
            }
        } else {
            silfp_notify_send_removed_notice(fid, gid, 0);
        }
    } else {
        ret = silfp_impl_remove_finger(fid);
        LOG_MSG_DEBUG("remove(%u, %d)", fid, ret);
        if (ret < 0) {
            silfp_notify_send_error_notice(-ret);
        } else {
            silfp_notify_send_removed_notice(fid, gid, 0);
            if (m_below_android_8_platform) {
                silfp_notify_send_removed_notice(0, gid, 0);
            }
        }
    }
    pthread_mutex_unlock(&m_lock);

    silfp_impl_cal_reset();
    silfp_cust_tpl_change_action();

    return SL_SUCCESS;
}

int32_t silfp_common_enumerate6(fingerprint_finger_id_t *results, uint32_t *max_size)
{
    int32_t ret = 0;
    int32_t i = 0;

    int32_t count = 0;
    uint32_t *ids = NULL;

    LOG_MSG_VERBOSE("enumerate()");

    if (results == NULL || max_size == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    pthread_mutex_lock(&m_lock);
    count = silfp_impl_get_db_count();
    if (*max_size == 0) {
        *max_size = count;
        ret = count;
    } else if (count > 0) {
        ids = (uint32_t *)malloc(sizeof(uint32_t) * count);
        if (NULL != ids) {
            memset(ids, 0 , sizeof(uint32_t) * count);
            count = silfp_impl_get_finger_prints(ids, count);
            for (i = 0; i < count && i < (int)(*max_size); i++) {
                results[i].fid = ids[i];
                results[i].gid = m_gid;
            }
            ret = i;
            *max_size = i;
            free(ids);
        } else {
            ret = -SL_ERROR_OUT_OF_MEMORY;
        }
    } else {
        *max_size = 0;
    }
    pthread_mutex_unlock(&m_lock);

    return ret;
}

int32_t silfp_common_enumerate()
{
    LOG_MSG_VERBOSE("silfp_common_enumerate enter");
    silfp_cust_tpl_change_action();
#if 0
    int32_t i = 0;

    int32_t count = 0;
    uint32_t *ids = NULL;

    LOG_MSG_VERBOSE("enumerate()");

    pthread_mutex_lock(&m_lock);
    count = silfp_impl_get_db_count();
    if (count > 0) {
        ids = (uint32_t *)malloc(sizeof(uint32_t) * count);
        if (NULL != ids) {
            memset(ids, 0 , sizeof(uint32_t) * count);
            count = silfp_impl_get_finger_prints(ids, count);
            for (i = 0; i < count; i++) {
                silfp_notify_send_acquired_notice(FINGERPRINT_ACQUIRED_GOOD);
                silfp_notify_send_enumerate_notice(ids[i], m_gid, (uint32_t)(count - i - 1));
            }
            free(ids);
        }
    } else {
        silfp_notify_send_enumerate_notice(0, m_gid, 0);
    }
    pthread_mutex_unlock(&m_lock);
#endif
    return SL_SUCCESS;
}

int32_t silfp_common_set_active_group(uint32_t gid, const char *store_path)
{
    int32_t err = FINGERPRINT_ERROR;

    LOG_MSG_VERBOSE("set_active_group(%d, %s)", gid, store_path);

    pthread_mutex_lock(&m_lock);
    m_gid = gid;

    err = silfp_impl_set_gid(gid);
    if (err >= 0) {
        err = silfp_impl_load_user_db((char *)store_path);
    }
    pthread_mutex_unlock(&m_lock);

    if (err >= 0) {
        err = SL_SUCCESS;
    }
    LOG_MSG_DEBUG("err(%d)", err);

    silfp_cust_tpl_change_action();

    return err;
}

// _common_set_default_path invoked later, so here just set log dump path in the beginning.
void silfp_common_sync_log_dump_path(void)
{
    const char *path = NULL;

    if (_common_is_mask_unset(CFG_TYPE_DUMP_PATH_MASK)) {
        path = silfp_cust_get_dump_path();
        if (path != NULL) {
            silfp_log_dump_set_path(path, strlen(path));
        } else {
            if (_common_is_below_8_platform()) { // can't use m_below_android_8_platform, not be initialized
                silfp_log_dump_set_path(BELOW_8_DEFAULT_PATH, strlen(BELOW_8_DEFAULT_PATH));
            }
        }
    }
}

void silfp_common_set_dump_path(const char *path, int32_t force)
{
    if ((path != NULL) && (force || _common_is_mask_unset(CFG_TYPE_DUMP_PATH_MASK))) {
        LOG_MSG_VERBOSE("set dump path(%s)", path);
        silfp_dump_set_path(path, strlen(path));
        silfp_log_dump_set_path(path, strlen(path));
        _common_set_mask(CFG_TYPE_DUMP_PATH_MASK);
    }
}

void silfp_common_set_cal_path(const char *path, int32_t force)
{
    if ((path != NULL) && (force || _common_is_mask_unset(CFG_TYPE_CAL_PATH_MASK))) {
        LOG_MSG_VERBOSE("set cal&ext path(%s)", path);
        silfp_impl_cal_set_path(path, strlen(path));
        silfp_ext_set_path(path, strlen(path));
        _common_set_mask(CFG_TYPE_CAL_PATH_MASK);
    }
}

void silfp_common_set_ta_name(const char *taname, int32_t force)
{
    if (taname != NULL && (force || _common_is_mask_unset(CFG_TYPE_TA_NAME_MASK))) {
        LOG_MSG_VERBOSE("set ta name(%s)", taname);
        silfp_impl_set_ta_name(taname, strlen(taname));
        _common_set_mask(CFG_TYPE_TA_NAME_MASK);
    }
}

void silfp_common_disable_capture(int32_t force)
{
    if (force || _common_is_mask_unset(CFG_TYPE_CAPTURE_MASK)) {
        LOG_MSG_VERBOSE("disable capture image feature");
        silfp_ext_set_capture_image(0);
        _common_set_mask(CFG_TYPE_CAPTURE_MASK);
    }
}