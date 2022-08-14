/******************************************************************************
 * @file   silead_fingerext_event.c
 * @brief  Contains fingerprint extension event operate functions.
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
 * David Wang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_ext_event"
#include "log/logmsg.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include "silead_ext_event.h"

#ifdef SIL_FP_EXT_SKT_SERVER_ENABLE

static pthread_mutex_t m_list_mutex;
#define MUTEX_ACQUIRE() pthread_mutex_lock(&m_list_mutex)
#define MUTEX_RELEASE() pthread_mutex_unlock(&m_list_mutex)
#define MUTEX_INIT() pthread_mutex_init(&m_list_mutex, NULL)
#define MUTEX_DESTROY() pthread_mutex_destroy(&m_list_mutex)

static fd_set m_read_fds;
static int32_t m_nfds = 0;
static int32_t m_exit = 0;

static ext_event_t * m_watch_table[MAX_FD_EVENTS];
static ext_event_t m_pending_list;

static void _ext_event_init_list(ext_event_t *list)
{
    memset(list, 0, sizeof(ext_event_t));
    list->next = list;
    list->prev = list;
    list->fd = -1;
}

static void _ext_event_add_to_list(ext_event_t *ev, ext_event_t *list)
{
    ev->next = list;
    ev->prev = list->prev;
    ev->prev->next = ev;
    list->prev = ev;
}

static void _ext_event_remove_from_list(ext_event_t *ev)
{
    ev->next->prev = ev->prev;
    ev->prev->next = ev->next;
    ev->next = NULL;
    ev->prev = NULL;
}

static void _ext_event_remove_watch(ext_event_t *ev, int32_t index)
{
    int32_t i;
    m_watch_table[index] = NULL;
    ev->index = -1;

    FD_CLR(ev->fd, &m_read_fds);

    if (ev->fd + 1 == m_nfds) {
        int32_t n = 0;

        for (i = 0; i < MAX_FD_EVENTS; i++) {
            ext_event_t * rev = m_watch_table[i];
            if ((rev != NULL) && (rev->fd > n)) {
                n = rev->fd;
            }
        }
        m_nfds = n + 1;
    }
}

static void _ext_event_process_read_readies(fd_set *rfds, int32_t n)
{
    int32_t i;

    MUTEX_ACQUIRE();

    for (i = 0; (i < MAX_FD_EVENTS) && (n > 0); i++) {
        ext_event_t *rev = m_watch_table[i];
        if (rev != NULL && FD_ISSET(rev->fd, rfds)) {
            _ext_event_add_to_list(rev, &m_pending_list);
            if (!rev->persist) {
                _ext_event_remove_watch(rev, i);
            }
            n--;
        }
    }

    MUTEX_RELEASE();
}

static void _ext_event_fire_pending()
{
    ext_event_t *ev = m_pending_list.next;
    while (ev != &m_pending_list) {
        ext_event_t * next = ev->next;
        _ext_event_remove_from_list(ev);
        ev->func(ev->fd, 0, ev->param);
        ev = next;
    }
}

void silfp_ext_event_init()
{
    MUTEX_INIT();

    FD_ZERO(&m_read_fds);
    m_nfds = 0;
    m_exit = 0;
    _ext_event_init_list(&m_pending_list);
    memset(m_watch_table, 0, sizeof(m_watch_table));
}

void silfp_ext_event_deinit()
{
    FD_ZERO(&m_read_fds);
    _ext_event_init_list(&m_pending_list);
    memset(m_watch_table, 0, sizeof(m_watch_table));

    MUTEX_DESTROY();
}

void silfp_ext_event_set(ext_event_t *ev, int32_t fd, int32_t persist, event_cb func, void *param)
{
    int32_t ret = 0;
    memset(ev, 0, sizeof(ext_event_t));
    ev->fd = fd;
    ev->index = -1;
    ev->persist = persist;
    ev->func = func;
    ev->param = param;
    ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (0 != ret) {
        LOG_MSG_ERROR("silfp_ext_event_set fcntl ret:%d", ret);
    }
}

void silfp_ext_event_add(ext_event_t * ev)
{
    int32_t i;

    MUTEX_ACQUIRE();

    for (i = 0; i < MAX_FD_EVENTS; i++) {
        if (m_watch_table[i] == NULL) {
            m_watch_table[i] = ev;
            ev->index = i;

            FD_SET(ev->fd, &m_read_fds);
            if (ev->fd >= m_nfds) {
                m_nfds = ev->fd + 1;
            }

            break;
        }
    }

    MUTEX_RELEASE();
}

void silfp_ext_event_del(ext_event_t *ev)
{
    MUTEX_ACQUIRE();

    if (ev->index >= 0 && ev->index < MAX_FD_EVENTS) {
        _ext_event_remove_watch(ev, ev->index);
    }

    MUTEX_RELEASE();
}

void silfp_ext_event_loop()
{
    int32_t n;
    fd_set rfds;

    for (;;) {
        memcpy(&rfds, &m_read_fds, sizeof(fd_set));

        n = select(m_nfds, &rfds, NULL, NULL, NULL);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }

            LOG_MSG_ERROR("event: select error (%d)", errno);
            break;
        }

        _ext_event_process_read_readies(&rfds, n);
        _ext_event_fire_pending();

        if (m_exit) {
            break;
        }
    }
}

void silfp_ext_event_exit()
{
    m_exit = 1;
}

#endif // SIL_FP_EXT_SKT_SERVER_ENABLE