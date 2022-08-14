/******************************************************************************
 * @file   silead_log.c
 * @brief  Contains dump ta log functions.
 *
 *
 * Copyright (c) 2016-2019 Silead Inc.
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
 * calvin wang     2018/8/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_log"
#include "log/logmsg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_util.h"
#include "silead_log.h"
#include "silead_cmd.h"

#ifndef SIL_LOG_DUMP_DATA_PATH
#define SIL_LOG_DUMP_DATA_PATH "/data/vendor/silead"
#endif

static char m_str_log_dump_path[MAX_PATH_LEN] = {0};
static const char *_log_dump_get_path(void)
{
    if (m_str_log_dump_path[0] != '\0') {
        return m_str_log_dump_path;
    } else {
        return SIL_LOG_DUMP_DATA_PATH;
    }
}

void silfp_log_dump_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_log_dump_path, sizeof(m_str_log_dump_path), path, len);
    if (ret < 0) {
        memset(m_str_log_dump_path, 0, sizeof(m_str_log_dump_path));
    }
    LOG_MSG_VERBOSE("path = %s", m_str_log_dump_path);
}

#ifndef SIL_DEBUG_LOG_DUMP_DYNAMIC
int32_t silfp_log_dump_init(void)
{
    return 0;
}

void silfp_log_dump_deinit(void)
{
}

#else /* SIL_DEBUG_LOG_DUMP_DYNAMIC */

#define LOG_DUMP_WAIT_TIMEOUT_SEC 2  // timeout: 2s

#define LOG_DUMP_FILE_BAK_COUNT 4
#define LOG_DUMP_FILE_LEN_MAX (2*1024*1024)

#define LOG_DUMP_FILE_SUFFIX "log"
#define LOG_DUMP_FILE_BAK_SUFFIX "old"

static pthread_t m_tid_dispatch = -1;
static int32_t m_started = 0;
static int32_t m_exit = 0;

static pthread_mutex_t m_startup_mutex;
static pthread_cond_t m_startup_cond;
static pthread_mutex_t m_list_mutex;

static int32_t m_wakeup_read_fd = -1;
static int32_t m_wakeup_write_fd = -1;
static log_event_t m_wakeupfd_event;

static log_event_t *m_watch_table[LOG_EVENTS_MAX];
static fd_set m_read_fds;
static int32_t m_nfds = 0;

void silfp_log_file_flush(const char *pname, int32_t *pbak_idx, const void *pbuf, uint32_t len)
{
    int32_t ret = 0;

    int32_t fd = -1;
    char path[MAX_PATH_LEN] = {0};
    char path_old[MAX_PATH_LEN] = {0};
    int32_t file_size = 0;

    if (pname == NULL || pbak_idx == NULL || pbuf == NULL || len == 0) {
        return;
    }

    snprintf(path, sizeof(path), "%s/%s.%s", _log_dump_get_path(), pname, LOG_DUMP_FILE_SUFFIX);

    file_size = silfp_util_file_get_size(path);
    if (file_size >= LOG_DUMP_FILE_LEN_MAX) {
        if (*pbak_idx > 0) {
            snprintf(path_old, sizeof(path_old), "%s/%s-%s%d.%s", _log_dump_get_path(), pname, LOG_DUMP_FILE_BAK_SUFFIX, *pbak_idx, LOG_DUMP_FILE_SUFFIX);
            if (*pbak_idx + 1 < LOG_DUMP_FILE_BAK_COUNT) {
                *pbak_idx = *pbak_idx + 1;
            } else {
                *pbak_idx = 0;
            }
        } else {
            snprintf(path_old, sizeof(path_old), "%s/%s-%s.%s", _log_dump_get_path(), pname, LOG_DUMP_FILE_BAK_SUFFIX, LOG_DUMP_FILE_SUFFIX);
            if (*pbak_idx + 1 < LOG_DUMP_FILE_BAK_COUNT) {
                *pbak_idx = 1;
            }
        }
        ret = silfp_util_file_rename(path, path_old);
        if (ret < 0) {
            LOG_MSG_ERROR("rename %s --> %s failed %d (%d:%s)", path, path_old, ret, errno, strerror(errno));
        }
    }

    fd = silfp_util_open_file(path, 1);
    if (fd >= 0) {
        ret = silfp_util_write_file(fd, pbuf, len);
        silfp_util_close_file(fd);
    }
}

int32_t silfp_log_is_event_thread(pthread_t tid)
{
    if (pthread_equal(tid, m_tid_dispatch)) {
        return 1;
    }
    return 0;
}

static void _log_wakeup_callback(int32_t fd, int32_t timeout)
{
    int32_t ret = 0;
    char buff[16] = {0};

    UNUSED(fd);
    UNUSED(timeout);

    if (m_wakeup_read_fd >= 0) {
        do {
            ret = read(m_wakeup_read_fd, &buff, sizeof(buff));
        } while (ret > 0 || (ret < 0 && errno == EINTR));
    }
}

static void _log_trigger_event_loop(void)
{
    int32_t ret = 0;
    if (m_wakeup_write_fd >= 0) {
        if (!silfp_log_is_event_thread(pthread_self())) {
            do {
                ret = write(m_wakeup_write_fd, " ", 1);
            } while (ret < 0 && errno == EINTR);
        }
    }
}

void silfp_log_event_set(log_event_t *ev, int32_t fd, log_event_cb func, log_event_timeout timeout)
{
    int32_t ret = 0;

    memset(ev, 0, sizeof(log_event_t));
    ev->fd = fd;
    ev->func = func;
    ev->timeout = timeout;

    ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (0 != ret) {
        LOG_MSG_ERROR("fcntl ret:%d", ret);
    }
}

void silfp_log_event_add(int32_t index, log_event_t *ev)
{
    if (index >= 0 && index < LOG_EVENTS_MAX) {
        pthread_mutex_lock(&m_list_mutex);
        m_watch_table[index] = ev;
        FD_SET(ev->fd, &m_read_fds);
        if (ev->fd >= m_nfds) {
            m_nfds = ev->fd + 1;
        }
        pthread_mutex_unlock(&m_list_mutex);

        _log_trigger_event_loop();
    }
}

void silfp_log_event_del(int32_t index)
{
    log_event_t *ev = NULL;
    int32_t n = 0;
    int32_t i = 0;

    if (index >= 0 && index < LOG_EVENTS_MAX) {
        pthread_mutex_lock(&m_list_mutex);
        ev = m_watch_table[index];
        if (ev != NULL) {
            m_watch_table[index] = NULL;

            FD_CLR(ev->fd, &m_read_fds);

            if (ev->fd + 1 == m_nfds) {
                for (i = 0; i < LOG_EVENTS_MAX; i++) {
                    log_event_t *rev = m_watch_table[i];
                    if ((rev != NULL) && (rev->fd > n)) {
                        n = rev->fd;
                    }
                }
                m_nfds = n + 1;
            }
        }
        pthread_mutex_unlock(&m_list_mutex);

        _log_trigger_event_loop();
    }
}

static int32_t _log_cal_timeout(struct timeval *tv)
{
    int32_t timeout = -1;
    int32_t i = 0;
    log_event_t *ev = NULL;

    pthread_mutex_lock(&m_list_mutex);
    for (i = LOG_EVENT_CA_LOG; i < LOG_EVENTS_MAX; i++) {
        ev = m_watch_table[i];
        if (ev != NULL && ev->timeout != NULL) {
            timeout = ev->timeout();
            if (timeout > 0) {
                break;
            }
        }
    }
    pthread_mutex_unlock(&m_list_mutex);

    if (timeout > 0) {
        if (tv != NULL) {
            tv->tv_sec = LOG_DUMP_WAIT_TIMEOUT_SEC;
            tv->tv_usec = 0;
            return 0;
        }
    }

    return -1;
}

static void _log_event_process(fd_set *rfds, int32_t n)
{
    int32_t i = 0;
    log_event_t *ev = NULL;

    pthread_mutex_lock(&m_list_mutex);
    for (i = 0; (i < LOG_EVENTS_MAX) && (n > 0); i++) {
        ev = m_watch_table[i];
        if (ev != NULL && FD_ISSET(ev->fd, rfds)) {
            if (ev->func != NULL) {
                ev->func(ev->fd, 0);
            }
            n--;
        }
    }
    pthread_mutex_unlock(&m_list_mutex);
}

static void _log_event_timeout(void)
{
    int32_t i = 0;
    log_event_t *ev = NULL;

    pthread_mutex_lock(&m_list_mutex);
    for (i = LOG_EVENT_CA_LOG; i < LOG_EVENTS_MAX; i++) {
        ev = m_watch_table[i];
        if (ev != NULL && ev->func != NULL) {
            ev->func(ev->fd, 1);
        }
    }
    pthread_mutex_unlock(&m_list_mutex);
}

void _log_event_loop()
{
    int32_t n = 0;
    fd_set rfds;
    struct timeval tv;
    struct timeval *ptv = NULL;

    for (;;) {
        memcpy(&rfds, &m_read_fds, sizeof(fd_set));

        if (_log_cal_timeout(&tv) < 0) {
            ptv = NULL;
            //LOG_MSG_VERBOSE("no timers; blocking indefinitely");
        } else {
            ptv = &tv;
            //LOG_MSG_VERBOSE("blocking for %ds + %dus", (int32_t)tv.tv_sec, (int32_t)tv.tv_usec);
        }

        n = select(m_nfds, &rfds, NULL, NULL, ptv);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }

            LOG_MSG_ERROR("event: select error (%d)", errno);
            break;
        }

        if (n == 0) { // timeout
            _log_event_timeout();
        } else {
            _log_event_process(&rfds, n);
        }

        if (m_exit) {
            break;
        }
    }
}

static void *_log_async_read_loop(void *param)
{
    int32_t ret = 0;
    int32_t filedes[2] = {0};

    UNUSED(param);

    pthread_mutex_lock(&m_startup_mutex);
    m_started = 1;
    pthread_cond_broadcast(&m_startup_cond);
    pthread_mutex_unlock(&m_startup_mutex);

    ret = pipe(filedes);
    if (ret < 0) {
        LOG_MSG_ERROR("Error in pipe() errno:%d", errno);
        return NULL;
    }

    m_wakeup_read_fd = filedes[0];
    m_wakeup_write_fd = filedes[1];

    silfp_log_event_set(&m_wakeupfd_event, m_wakeup_read_fd, _log_wakeup_callback, NULL);
    silfp_log_event_add(LOG_EVENT_WAKEUP, &m_wakeupfd_event);

    _log_event_loop();

    close(m_wakeup_read_fd);
    close(m_wakeup_write_fd);
    m_wakeup_read_fd = -1;
    m_wakeup_write_fd = -1;

    return NULL;
}

int32_t _log_dump_init(void)
{
    int32_t ret = 0;

    if (m_started) {
        return 0;
    }

    LOG_MSG_VERBOSE("init");

    pthread_mutex_lock(&m_startup_mutex);

    ret = pthread_create(&m_tid_dispatch, NULL, _log_async_read_loop, NULL);

    while (m_started == 0) {
        pthread_cond_wait(&m_startup_cond, &m_startup_mutex);
    }

    pthread_mutex_unlock(&m_startup_mutex);

    if (ret < 0) {
        LOG_MSG_ERROR("Failed to create dispatch thread errno:%d", errno);
    } else {
        silfp_log_ca_dump_init();
        silfp_log_ta_dump_init();
    }

    return ret;
}

int32_t silfp_log_dump_init(void)
{
    int32_t ret = 0;

    FD_ZERO(&m_read_fds);
    m_nfds = 0;
    m_exit = 0;
    memset(m_watch_table, 0, sizeof(m_watch_table));

    pthread_mutex_init(&m_startup_mutex, NULL);
    pthread_cond_init(&m_startup_cond, NULL);
    pthread_mutex_init(&m_list_mutex, NULL);

    m_started = 0;

    ret = silfp_dbg_check_ca_to_file_mode();
    if (ret >= 0) {
        silfp_log_dump_to_file(LOG_DUMP_TO_FILE_CA);
    }

    return 0;
}

void silfp_log_dump_deinit(void)
{
    LOG_MSG_VERBOSE("m_started: %d", m_started);

    if (m_started) {
        silfp_log_ca_dump_deinit();
        silfp_log_ta_dump_deinit();

        m_exit = 1;
        _log_trigger_event_loop();
        m_started = 0;

        pthread_join(m_tid_dispatch, NULL);
    }

    pthread_cond_destroy(&m_startup_cond);
    pthread_mutex_destroy(&m_startup_mutex);
    pthread_mutex_destroy(&m_list_mutex);

    LOG_MSG_VERBOSE("deinit");
}

void silfp_log_dump_to_file(int32_t enable)
{
    if (enable & (LOG_DUMP_TO_FILE_CA | LOG_DUMP_TO_FILE_TA)) {
        _log_dump_init();
    }

    silfp_log_ca_dump_to_file(enable);
    silfp_log_ta_dump_to_file(enable);
}

#endif /* SIL_DEBUG_LOG_DUMP_DYNAMIC */

void silfp_log_dump_cfg(uint32_t chipid, uint32_t subid, uint32_t vid)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = 128 * 1024;
    uint32_t result = 0;

    char name[MAX_PATH_LEN] = {0};

    buf = malloc(len);
    if (buf == NULL) {
        LOG_MSG_VERBOSE("malloc(%u) fail", len);
        return;
    }

    memset(buf, 0, len);
    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DBG_CFG, buf, &len, &result);
    if (ret >= 0) {
        snprintf(name, sizeof(name), "%08x-%08x-%08x.cfg", chipid, subid, vid);
        silfp_util_file_save(_log_dump_get_path(), name, buf, len);
    }

    free(buf);
    buf = NULL;
}