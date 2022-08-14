/******************************************************************************
 * @file   silead_worker.c
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

#define FILE_TAG "silead_worker"
#include "log/logmsg.h"

#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include "silead_const.h"
#include "silead_impl.h"
#include "silead_cust.h"
#include "silead_worker.h"

#include "silead_lock.h"
#include "silead_nav.h"
#include "silead_enroll.h"
#include "silead_auth.h"
#include "silead_ext_cb.h"
#include "silead_ext_skt.h"
#include "silead_debug.h"
#include "silead_cmd.h"

typedef struct {
    int32_t request;
    int32_t (*dispatch) ();
} worker_cmd_t;

static const worker_cmd_t m_worker[] = {
    {STATE_ENROLL,  silfp_enroll_command},
    {STATE_SCAN,    silfp_auth_command},
    {STATE_NAV,     silfp_nav_command},
    {STATE_TEST,    silfp_ext_skt_commond},
    {STATE_EXT_CB,  silfp_ext_cb_command},
    {STATE_LOCK,    silfp_lock_mode_commond},
    {STATE_ESD_CHK, silfp_esd_check_commond},
};

static pthread_t m_tid_workerthread;
static worker_state_t m_worker_state;
static uint8_t m_work_cancel = 0;

static pthread_mutex_t m_lock;
static pthread_cond_t m_worker_cond;

static uint8_t m_screen_status = 0;

static inline const char *_worker_get_state_string(int32_t state)
{
    static const char *stateString[] = {
        "STATE_IDLE",
        "STATE_ENROLL",
        "STATE_SCAN",
        "STATE_NAV",
        "STATE_EXIT",
        "STATE_TEST",
        "STATE_WAIT",
        "STATE_EXT_CB",
        "STATE_LOCK",
        "STATE_BREAK",
        "STATE_ESD_CHK",
    };

    if (state >= 0 && state < STATE_MAX) {
        return stateString[state];
    }

    return "UNKNOWN";
}

void silfp_worker_wait_condition(int32_t timeout_msec)
{
    struct timeval now;
    struct timespec outtime;

    if (timeout_msec == 0) {
        timeout_msec = 1000;
    }

    if (timeout_msec > 0) {
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + timeout_msec / 1000;
        outtime.tv_nsec = now.tv_usec * 1000 + (timeout_msec % 1000) * 1000000;
    }

    pthread_mutex_lock(&m_lock);
    if (timeout_msec > 0) {
        pthread_cond_timedwait(&m_worker_cond, &m_lock, &outtime);
    } else {
        pthread_cond_wait(&m_worker_cond, &m_lock);
    }
    pthread_mutex_unlock(&m_lock);
}

void silfp_worker_wakeup_condition(void)
{
    pthread_mutex_lock(&m_lock);
    pthread_cond_signal(&m_worker_cond);
    pthread_mutex_unlock(&m_lock);
}

static inline worker_state_t _worker_get_state_unlock(void)
{
    return m_worker_state;
}

worker_state_t silfp_worker_get_state(void)
{
    worker_state_t state = STATE_IDLE;

    pthread_mutex_lock(&m_lock);
    state = _worker_get_state_unlock();
    pthread_mutex_unlock(&m_lock);

    return state;
}

static void _worker_set_state_unlock(worker_state_t state)
{
    LOG_MSG_INFO("set to %s************", _worker_get_state_string(state));
    m_worker_state = state;
}

void silfp_worker_set_state_no_signal(worker_state_t state)
{
    pthread_mutex_lock(&m_lock);
    _worker_set_state_unlock(state);
    pthread_mutex_unlock(&m_lock);
}

void silfp_worker_set_state(worker_state_t state)
{
    pthread_mutex_lock(&m_lock);

    m_work_cancel = 1;
    silfp_impl_cancel();

    if ((_worker_get_state_unlock() != STATE_EXIT) && ((_worker_get_state_unlock() != STATE_BREAK) || (state == STATE_EXIT))) {
        _worker_set_state_unlock(state);
    }
    pthread_cond_signal(&m_worker_cond);
    pthread_mutex_unlock(&m_lock);
}

int32_t silfp_worker_get_screen_state(void)
{
    int32_t screenon = 0;

    pthread_mutex_lock(&m_lock);
    screenon = m_screen_status;
    pthread_mutex_unlock(&m_lock);

    return screenon;
}

void silfp_worker_set_screen_state_default(int32_t screenon)
{
    pthread_mutex_lock(&m_lock);
    m_screen_status = screenon;
    pthread_mutex_unlock(&m_lock);

    LOG_MSG_VERBOSE("screen status:%d", m_screen_status);
}

void silfp_worker_screen_state_callback(int32_t screenon, void *param)
{
    UNUSED(param);
    pthread_mutex_lock(&m_lock);

    m_screen_status = screenon;
    LOG_MSG_INFO("m_screen_status: %d", m_screen_status);

    worker_state_t state = _worker_get_state_unlock();
    if (silfp_nav_check_support() && state != STATE_EXIT && state != STATE_BREAK) {
        LOG_MSG_DEBUG("work status: %s", _worker_get_state_string(state));
        if (state == STATE_IDLE && m_screen_status) {
            LOG_MSG_DEBUG("should run navi, signal only*************");
            pthread_cond_signal(&m_worker_cond);
        } else if (state == STATE_NAV && !m_screen_status) {
            LOG_MSG_DEBUG("should goto idle, cancel navi************");
            _worker_set_state_unlock(STATE_IDLE);
            m_work_cancel = 1;
            silfp_impl_cancel();
        } else if (state == STATE_ESD_CHK) {
            LOG_MSG_DEBUG("should goto idle, cancel esd************");
            _worker_set_state_unlock(STATE_IDLE);
            m_work_cancel = 1;
            silfp_impl_cancel();
        }
    } else if (!silfp_nav_check_support() && (state == STATE_NAV)) {
        LOG_MSG_DEBUG("not support navi, cancel navi************");
        _worker_set_state_unlock(STATE_IDLE);
        m_work_cancel = 1;
        silfp_impl_cancel();
    } else if (/*(m_screen_status == 1) && */(state == STATE_ESD_CHK || state == STATE_SCAN)) {
        LOG_MSG_INFO("authenticate finish************");
        if ((m_screen_status == 1) || (silfp_impl_get_db_count() == 0 && state == STATE_ESD_CHK)){
            LOG_MSG_DEBUG("m_screen_status == 1 or db == 0************");
            _worker_set_state_unlock(state);
            m_work_cancel = 1;
            silfp_impl_cancel();
        }
    }
    pthread_mutex_unlock(&m_lock);
    silfp_dbg_update_all_log_level();
}

int32_t silfp_worker_is_canceled(void)
{
    int32_t cancel = 0;

    pthread_mutex_lock(&m_lock);
    cancel = m_work_cancel;
    pthread_mutex_unlock(&m_lock);

    return cancel;
}

static void* _worker_command_loop(void *arg)
{
    worker_state_t state;
    uint32_t i = 0;
    int32_t ret = 0;

    LOG_MSG_VERBOSE("enter");
    UNUSED(arg);

    silfp_impl_calibrate();

    while (silfp_worker_get_state() != STATE_EXIT) {
        silfp_dbg_update_all_log_level();
        LOG_MSG_INFO("work %s", _worker_get_state_string(silfp_worker_get_state()));
        pthread_mutex_lock(&m_lock);
        state = _worker_get_state_unlock();
#ifdef SILEAD_CHECK_BROKEN
        ret = silfp_cmd_check_broken(state);
        if (ret < 0) {
            LOG_MSG_ERROR("silfp_cmd_check_broken and set to STATE_BREAK************");
            m_worker_state = STATE_BREAK;
            state = STATE_BREAK;
        }
        if (STATE_WAIT == state || STATE_BREAK == state || (state == STATE_IDLE && (!silfp_nav_check_support() || !m_screen_status) && !silfp_esd_mode_support())) {
            LOG_MSG_INFO("wait----------");
            if (STATE_WAIT != state) {
                LOG_MSG_INFO("loop ---power off------------");
                silfp_impl_chip_pwoff();
            }
            pthread_cond_wait(&m_worker_cond, &m_lock);
        }
#else
        if (STATE_WAIT == state || STATE_BREAK == state || (state == STATE_IDLE && (!silfp_nav_check_support() || !m_screen_status) && !silfp_esd_mode_support())) {
            LOG_MSG_INFO("wait----------");
            pthread_cond_wait(&m_worker_cond, &m_lock);
        }
#endif
        m_work_cancel = 0;
        pthread_mutex_unlock(&m_lock);

        state = silfp_worker_get_state();
        for (i = 0; i < NUM_ELEMS(m_worker); i++) {
            if (m_worker[i].request == (int32_t)state) {
                LOG_MSG_INFO("dispatch (%d) %s", state, _worker_get_state_string(state));
                silfp_impl_wait_clean();
                silfp_cust_tp_irq_enable(1);
                m_worker[i].dispatch();
                silfp_cust_tp_irq_enable(0);
                break;
            }
        }

        pthread_mutex_lock(&m_lock);
        if (!m_work_cancel) {
            if (silfp_nav_check_support() && m_screen_status && _worker_get_state_unlock() == STATE_IDLE) {
                _worker_set_state_unlock(STATE_NAV);
            } else if (silfp_esd_mode_support() && _worker_get_state_unlock() == STATE_IDLE) {
                _worker_set_state_unlock(STATE_ESD_CHK);
            }
        }
        pthread_mutex_unlock(&m_lock);
    }

    LOG_MSG_VERBOSE("exit");

    return 0;
}

int32_t silfp_worker_init(void)
{
    LOG_MSG_VERBOSE("init");

    m_tid_workerthread = 0;
    m_worker_state = STATE_IDLE;
    m_work_cancel = 0;

    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_worker_cond, NULL);

    m_screen_status = 0;

    return 0;
}

int32_t silfp_worker_run(void)
{
    int32_t ret = 0;

    ret = pthread_create(&m_tid_workerthread, NULL, _worker_command_loop, NULL);
    if(ret != 0) {
        LOG_MSG_ERROR("can't create thread (%d:%s)", ret, strerror(ret));
        ret = -1;
    }

    return ret;
}

int32_t silfp_worker_deinit(void)
{
    silfp_worker_set_state(STATE_EXIT);
    pthread_join(m_tid_workerthread, NULL);

    pthread_cond_destroy(&m_worker_cond);
    pthread_mutex_destroy(&m_lock);

    LOG_MSG_VERBOSE("deinit");
    return 0;
}

void silfp_worker_set_to_break_mode(void)
{
    // ta crash, will come into break mode
    if (!pthread_equal(pthread_self(), m_tid_workerthread)) {
        silfp_worker_set_state(STATE_BREAK);
    } else {
        _worker_set_state_unlock(STATE_BREAK);
        m_work_cancel = 1;
        silfp_impl_cancel();
    }
}
