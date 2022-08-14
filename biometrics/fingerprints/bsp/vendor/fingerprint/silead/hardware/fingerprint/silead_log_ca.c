/******************************************************************************
 * @file   silead_log_ca.c
 * @brief  Contains dump ca log functions.
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

#define FILE_TAG "silead_log_ca"
#include "log/logmsg.h"

#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <cutils/properties.h>

#include "silead_const.h"
#include "silead_log.h"
#include "silead_util.h"

#define LOG_CA_NAMESPACE ("log.tag." LOG_TAG)

#define LOG_DUMP_FILE_NAME "silca"
#define LOG_DUMP_BUF_SIZE 1024

static int32_t m_log_level = -1;

static int32_t _log_to_level(const char *value)
{
    switch (value[0]) {
    case 'V':
    case 'v':
        return 0;  // VERBOSE
    case 'D':
    case 'd':
        return 1;  // DEBUG
    case 'I':
    case 'i':
        return 2;  // INFO
    case 'W':
    case 'w':
        return 3;  // WARNING
    case 'E':
    case 'e':
        return 4;  // ERROR
    case 'A':
    case 'a':
        return 0;  // ALL
    case 'S':
    case 's':
        return 100; // SUPPRESS
    }
    return 2;
}

int32_t silfp_log_is_loggable(int32_t level)
{
    int32_t len = 0;
    char buf[PROPERTY_VALUE_MAX] = {0};
    int32_t log_level = -1;

    if (m_log_level == -1) {
        len = property_get(LOG_CA_NAMESPACE, buf, "");
        log_level = _log_to_level(buf);
    } else {
        log_level = m_log_level;
    }

    return (level >= log_level) ? 1 : 0;
}

void silfp_log_set_level(int32_t level)
{
    m_log_level = level;
}

#ifndef SIL_DEBUG_LOG_DUMP_DYNAMIC

void silfp_log_dump(const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_DUMP_BUF_SIZE] = {0};
    int32_t buf_size = sizeof(buf);

    va_start(ap, fmt);
    vsnprintf(buf, buf_size, fmt, ap);
    va_end(ap);
    ALOGD("%s", buf);
}

#else /* SIL_DEBUG_LOG_DUMP_DYNAMIC */

static pthread_mutex_t m_log_mutex;
static pthread_mutex_t m_log_write_mutex;
static int32_t m_log_read_fd = -1;
static int32_t m_log_write_fd = -1;
static log_event_t m_fd_event;

static int32_t m_log_bak_index = 0;
static int32_t m_log_dump_to_file = 0;

static uint8_t *m_buf = NULL;
static int32_t m_buf_pos = 0;

static int32_t _log_ca_dump_timeout(void)
{
    if ((m_buf != NULL) && (m_buf_pos > 0)) {
        return 1;
    }
    return 0;
}

static void _log_ca_dump_event_callback(int32_t fd, int32_t timeout)
{
    int32_t ret = 0;

    if (m_buf == NULL) {
        return;
    }

    if (timeout) {
        if (m_buf_pos > 0) {
            silfp_log_file_flush(LOG_DUMP_FILE_NAME, &m_log_bak_index, m_buf, m_buf_pos);
            m_buf_pos = 0;
        }
    } else {
        if (fd == m_log_read_fd) {
            ret = read(fd, &m_buf[m_buf_pos], LOG_DUMP_BUF_CACHE_SIZE - m_buf_pos);
            if (ret > 0) {
                m_buf_pos += ret;
                if (m_buf_pos > LOG_DUMP_BUF_FLUSH_SIZE) {
                    silfp_log_file_flush(LOG_DUMP_FILE_NAME, &m_log_bak_index, m_buf, m_buf_pos);
                    m_buf_pos = 0;
                }
            }
        }
    }
}

static void _log_ca_dump_write(int32_t fd, const void *pbuf, uint32_t size)
{
    int32_t ret = 0;
    int32_t count = size;
    const unsigned char *p = (const unsigned char *)pbuf;

    if ((fd < 0) || (NULL == pbuf) || (0 == size)) {
        return;
    }

    pthread_mutex_lock(&m_log_write_mutex);
    do {
        ret = write(fd, p, count);
        if (ret > 0)  {
            count -= ret;
            p += ret;
        } else if (!(ret < 0 && errno == EINTR)) {
            break;
        }
    } while(count > 0);
    pthread_mutex_unlock(&m_log_write_mutex);

    if (count > 0) {
        LOG_MSG_DEBUG("write fail (%d:%s)", errno, strerror(errno));
    }
}

static int32_t _log_ca_dump_start(void)
{
    int32_t ret = 0;
    int32_t filedes[2];

    LOG_MSG_VERBOSE("start");
    pthread_mutex_init(&m_log_write_mutex, NULL);

    ret = pipe(filedes);
    if (ret < 0) {
        LOG_MSG_ERROR("Error in pipe() errno:%d", errno);
        return -1;
    }

    m_log_read_fd = filedes[0];
    m_log_write_fd = filedes[1];

    m_buf = malloc(LOG_DUMP_BUF_CACHE_SIZE);
    if (m_buf == NULL) {
        LOG_MSG_ERROR("malloc(%d) failed", LOG_DUMP_BUF_CACHE_SIZE);
        return -1;
    }
    m_buf_pos = 0;

    silfp_log_event_set(&m_fd_event, m_log_read_fd, _log_ca_dump_event_callback, _log_ca_dump_timeout);
    silfp_log_event_add(LOG_EVENT_CA_LOG, &m_fd_event);

    return 0;
}

static void _log_ca_dump_stop(void)
{
    silfp_log_event_del(LOG_EVENT_CA_LOG);

    if (m_log_read_fd >= 0) {
        close(m_log_read_fd);
        m_log_read_fd = -1;
    }
    if (m_log_write_fd >= 0) {
        close(m_log_write_fd);
        m_log_write_fd = -1;
    }

    _log_ca_dump_event_callback(m_log_read_fd, 1);
    if (m_buf != NULL) {
        free(m_buf);
        m_buf = NULL;
    }
    m_buf_pos = 0;

    pthread_mutex_destroy(&m_log_write_mutex);

    LOG_MSG_VERBOSE("stop");
}

int32_t silfp_log_ca_dump_init(void)
{
    m_log_read_fd = -1;
    m_log_write_fd = -1;
    memset(&m_fd_event, 0, sizeof(m_fd_event));
    m_buf = NULL;
    m_buf_pos = 0;

    pthread_mutex_init(&m_log_mutex, NULL);

    return 0;
}

void silfp_log_ca_dump_deinit(void)
{
    _log_ca_dump_stop();
    pthread_mutex_destroy(&m_log_mutex);
}

static int32_t _log_ca_dump_is_async_mode(void)
{
    if ((m_log_write_fd >= 0) && (m_log_dump_to_file & LOG_DUMP_TO_FILE_CA)) {
        return 1;
    }
    return 0;
}

void silfp_log_ca_dump_to_file(int32_t enable)
{
    pthread_mutex_lock(&m_log_mutex);

    if (!(m_log_dump_to_file & LOG_DUMP_TO_FILE_CA) && (enable & LOG_DUMP_TO_FILE_CA)) {
        m_log_dump_to_file = enable;
        _log_ca_dump_start();
    } else if ((m_log_dump_to_file & LOG_DUMP_TO_FILE_CA) && !(enable & LOG_DUMP_TO_FILE_CA)) {
        m_log_dump_to_file = enable;
        _log_ca_dump_stop();
    }

    pthread_mutex_unlock(&m_log_mutex);
}

void silfp_log_dump(const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_DUMP_BUF_SIZE] = {0};
    int32_t buf_size = sizeof(buf);
    uint64_t msecond = 0;
    int32_t time_size = 0;
    int32_t log_size = 0;
    int32_t async_mode = 0;

    async_mode = _log_ca_dump_is_async_mode();
    if (async_mode) {
        if (silfp_log_is_event_thread(pthread_self())) {
            async_mode = 0;
        }
    }

    if (async_mode) {
        msecond = silfp_util_get_msec();
        time_size = silfp_util_msec_to_date(msecond, buf, buf_size, MODE_GET_SEC_TYPE_ALL, MODE_GET_SEC_FORMAT_TIMESTAMP);
        if (time_size < 0) { // format fail
            time_size = 0;
        }

        if (time_size < buf_size) {
            va_start(ap, fmt);
            log_size = vsnprintf(buf + time_size, buf_size - time_size, fmt, ap);
            va_end(ap);

            if (log_size < 0) {
                log_size = 0;
            } else if (log_size >= buf_size - time_size) {
                log_size = buf_size - time_size - 1;
            }
        } else {
            time_size = buf_size - 1;
        }

        log_size += time_size;
        if (log_size > 0) {
            if (log_size < buf_size - 1) {
                buf[log_size] = '\n';
                log_size += 1;
                buf[log_size] = '\0';
            } else if (log_size == buf_size - 1) {
                buf[log_size - 1] = '\n';
                buf[log_size] = '\0';
            }
            _log_ca_dump_write(m_log_write_fd, buf, log_size);
        }
        return;
    }

    if (!async_mode) {
        va_start(ap, fmt);
        vsnprintf(buf, buf_size, fmt, ap);
        va_end(ap);
        ALOGD("%s", buf);
    }
}

#endif /* SIL_DEBUG_LOG_DUMP_DYNAMIC */