/******************************************************************************
 * @file   silead_log.c
 * @brief  Contains dump ta log functions.
 *
 *
 * Copyright (c) 2016-2018 Silead Inc.
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
 * Luke Ma     2018/8/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_log"
#include "log/logmsg.h"

#include <utils/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <cutils/properties.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_util.h"
#include "silead_cmd.h"

#define FP_FEATURE_DUMP_TA_LOG_MASK 0x0400
#define LOG_CA_NAMESPACE ("log.tag." LOG_TAG)

#ifndef SIL_LOG_DUMP_DATA_PATH
#define SIL_LOG_DUMP_DATA_PATH "/data/vendor/silead"
#endif

#define LOG_DUMP_FILE_LEN_MAX (4*1024*1024)
#define LOG_DUMP_TA_BUF_SIZE (100*1024)
#define LOG_DUMP_CA_BUF_SIZE 1024

#define LOG_DUMP_TA_FILE_NAME "ta_log"
#define LOG_DUMP_CA_FILE_NAME "ca_log"
#define LOG_DUMP_FILE_SUFFIX ".log"
#define LOG_DUMP_FILE_BAK_SUFFIX "-old"

#ifndef SIL_LOG_CA_TO_FILE_SUPPORT
#define SIL_LOG_CA_TO_FILE_SUPPORT 1
#endif
#if SIL_LOG_CA_TO_FILE_SUPPORT
#define LOG_DUMP_BUF_CACHE_SIZE (12*1024)
#define LOG_DUMP_BUF_FLUSH_SIZE (10*1024)
#define _log_ext_init _log_async_init
#define _log_ext_deinit _log_async_deinit

static int32_t m_log_async_ready = 0;
static int32_t m_log_async_exit = 0;
static pthread_t m_log_async_tid = -1;
static pthread_mutex_t m_log_write_mutex;
static int32_t m_log_read_fd = -1;
static int32_t m_log_write_fd = -1;
#else
#define _log_ext_init() ((void)0)
#define _log_ext_deinit() ((void)0)
#endif

static int32_t m_log_level = -1;
static int32_t m_log_dump_to_file = 0;
static int32_t m_log_dump_ta_support = 1;

static void *m_log_ta_buf = NULL;

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

static int32_t _log_to_level(const char* value)
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
    return 0;
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

static void _log_file_flush(char *name, const void *buf, uint32_t len)
{
    int32_t ret = 0;

    int32_t fd = -1;
    char path[MAX_PATH_LEN] = {0};
    char path_old[MAX_PATH_LEN] = {0};
    int32_t file_size = 0;

    if (name == NULL || buf == NULL || len == 0) {
        return;
    }

    snprintf(path, sizeof(path), "%s/%s%s", _log_dump_get_path(), name, LOG_DUMP_FILE_SUFFIX);

    file_size = silfp_util_file_get_size(path);
    if (file_size >= LOG_DUMP_FILE_LEN_MAX) {
        snprintf(path_old, sizeof(path_old), "%s/%s%s%s", _log_dump_get_path(), name, LOG_DUMP_FILE_BAK_SUFFIX, LOG_DUMP_FILE_SUFFIX);
        ret = silfp_util_file_rename(path, path_old);
        if (ret < 0) {
            LOG_MSG_ERROR("rename %s --> %s failed %d (%d:%s)", path, path_old, ret, errno, strerror(errno));
        }
    }

    fd = silfp_util_open_file(path, 1);
    if (fd >= 0) {
        ret = silfp_util_write_file(fd, buf, len);
        silfp_util_close_file(fd);
    }
}

#if SIL_LOG_CA_TO_FILE_SUPPORT
static int32_t _log_is_async_mode(void)
{
    if ((m_log_write_fd >= 0) && m_log_dump_to_file && m_log_async_ready && !m_log_async_exit) {
        return 1;
    }
    return 0;
}

static void _log_async_write(const void *buf, uint32_t size)
{
    int32_t ret = 0;
    int32_t count = size;
    const unsigned char *p = (const unsigned char *)buf;

    if ((m_log_write_fd < 0) || (NULL == buf) || (0 == size)) {
        return;
    }

    pthread_mutex_lock(&m_log_write_mutex);
    do {
        ret = write(m_log_write_fd, p, count);
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

static void *_log_async_read_loop(void __unused *param)
{
    int32_t ret = 0;
    struct pollfd fdp;
    char * buf = NULL;
    int32_t buf_pos = 0;
    int32_t buf_max = LOG_DUMP_BUF_CACHE_SIZE;
    int32_t timeout = -1;

    fdp.fd = m_log_read_fd;
    fdp.events = POLLIN;

    buf = (char *) malloc(buf_max);
    if (NULL == buf) {
        m_log_async_exit = 1;
        return NULL;
    }

    while (m_log_async_ready && !m_log_async_exit) {
        if (buf_pos > 0) {
            timeout = 2000;
        } else {
            timeout = -1;
        }
        ret = poll(&fdp, 1, timeout);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
        } else if (ret == 0) {
            if (buf_pos > 0) {
                _log_file_flush(LOG_DUMP_CA_FILE_NAME, buf, buf_pos);
                buf_pos = 0;
            }
        }

        if (fdp.revents & POLLIN) {
            fdp.revents = 0;

            ret = read(fdp.fd, &buf[buf_pos], buf_max - buf_pos);
            if (ret > 0) {
                buf_pos += ret;
                if (buf_pos > LOG_DUMP_BUF_FLUSH_SIZE) {
                    _log_file_flush(LOG_DUMP_CA_FILE_NAME, buf, buf_pos);
                    buf_pos = 0;
                }
            }
        }
    }

    if (NULL != buf) {
        if (buf_pos > 0) {
            _log_file_flush(LOG_DUMP_CA_FILE_NAME, buf, buf_pos);
            buf_pos = 0;
        }
        free(buf);
        buf = NULL;
    }

    return NULL;
}

static void _log_async_init(void)
{
    int32_t ret = 0;
    int32_t filedes[2];

    pthread_mutex_init(&m_log_write_mutex, NULL);

    if (m_log_read_fd < 0) {
        ret = pipe(filedes);
        if (ret < 0) {
            LOG_MSG_ERROR("Error in pipe() errno:%d", errno);
            return;
        }
        m_log_read_fd = filedes[0];
        m_log_write_fd = filedes[1];

        ret = fcntl(m_log_read_fd, F_SETFL, O_NONBLOCK);
        if (ret < 0) {
            LOG_MSG_ERROR("Error in fcntl() errno:%d", errno);
        }
    }

    if (!m_log_async_ready) {
        m_log_async_exit = 0;
        ret = pthread_create(&m_log_async_tid, NULL, _log_async_read_loop, NULL);
        if (ret < 0) {
            LOG_MSG_ERROR("Failed to create thread errno:%d", errno);
        } else {
            m_log_async_ready = 1;
        }
    }
}

static void _log_async_deinit(void)
{
    char exit_info[] = "async deinit";
    m_log_async_exit = 1;
    _log_async_write(exit_info, sizeof(exit_info));
    pthread_join(m_log_async_tid, NULL);
    m_log_async_tid = -1;
    m_log_async_ready = 0;

    if (m_log_read_fd >= 0) {
        close(m_log_read_fd);
        m_log_read_fd = -1;
    }
    if (m_log_write_fd >= 0) {
        close(m_log_write_fd);
        m_log_write_fd = -1;
    }
    pthread_mutex_destroy(&m_log_write_mutex);
}

static int32_t _log_async_get_time(void *buf, uint32_t size)
{
    int32_t len = 0;
    struct timeval now;
    struct tm tday;

    if (NULL == buf || size < 20) {
        return 0;
    }

    gettimeofday(&now, NULL);
    localtime_r(&now.tv_sec, &tday);

    len = snprintf(buf, size, "%02d-%02d %02d:%02d:%02d.%03d ", tday.tm_mon+1, tday.tm_mday, tday.tm_hour, tday.tm_min, tday.tm_sec, (uint32_t)(now.tv_usec/1000));
    if (len < 0) { // error
        len = 0;
    } else if (len >= (int32_t)size) { // not have enough buffer
        len = size - 1;
    }
    return len;
}
#endif

void silfp_log_dump_init(void)
{
    LOG_MSG_VERBOSE("init");
    _log_ext_init();
}

void silfp_log_dump_deinit(void)
{
    if (NULL != m_log_ta_buf) {
        free(m_log_ta_buf);
        m_log_ta_buf = NULL;
    }

    _log_ext_deinit();
    LOG_MSG_VERBOSE("deinit");
}

void silfp_log_dump_to_file(int32_t enable)
{
    m_log_dump_to_file = enable;
}

void silfp_log_dump_set_feature(uint32_t feature)
{
    m_log_dump_ta_support = !!(feature & FP_FEATURE_DUMP_TA_LOG_MASK);
    LOG_MSG_VERBOSE("dump_ta_log: %d", m_log_dump_ta_support);
}

void silfp_log_dump(const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_DUMP_CA_BUF_SIZE] = {0};
    int32_t buf_size = sizeof(buf);
    int32_t time_size = 0;
    int32_t log_size = 0;
    int32_t async_mode = 0;

#if SIL_LOG_CA_TO_FILE_SUPPORT
    async_mode = _log_is_async_mode();
    if (async_mode) {
        if (pthread_equal(pthread_self(), m_log_async_tid)) {
            async_mode = 0;
        }
    }
    if (async_mode) {
        time_size = _log_async_get_time(buf, buf_size);

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
            _log_async_write(buf, log_size);
        }
        return;
    }
#endif

    if (!async_mode) {
        va_start(ap, fmt);
        vsnprintf(buf, buf_size, fmt, ap);
        va_end(ap);
        ALOGD("%s", buf);
    }
}

void silfp_log_dump_ta_log(void)
{
    int32_t ret = 0;
    uint32_t result = 0;
    uint32_t len = 0;

    if ((!m_log_dump_ta_support) || (!m_log_dump_to_file)) {
        return;
    }

    if (NULL == m_log_ta_buf) {
        m_log_ta_buf = malloc(LOG_DUMP_TA_BUF_SIZE);
    }

    if (NULL != m_log_ta_buf) {
        len = LOG_DUMP_TA_BUF_SIZE;
        ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DUMP_TA_LOG, m_log_ta_buf, &len, &result);
        if (ret >= 0) {
            _log_file_flush(LOG_DUMP_TA_FILE_NAME, m_log_ta_buf, len);
        }

        if (ret == -SL_ERROR_REQ_CMD_UNSUPPORT) { // is not supported, disable it.
            m_log_dump_ta_support = 0;
        }
    }
}