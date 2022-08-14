/******************************************************************************
 * @file   silead_fingerext_hub.c
 * @brief  Contains fingerprint extension hub operate functions implement file.
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

#define FILE_TAG "silead_ext_hub"
#include "log/logmsg.h"

#include <cutils/sockets.h>
#include <cutils/record_stream.h>
#include <sys/un.h>

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <pwd.h>

#include "silead_ext_skt.h"
#include "silead_ext_event.h"
#include "silead_ext_hub.h"
#include "silead_util.h"

#ifdef SIL_FP_EXT_SKT_SERVER_ENABLE

#define MAX_COMMAND_BYTES (150 * 1024)

#define SOCKET_NAME_DEFAULT "com_silead_fpext"

// function declare
static void _ext_hub_listen_callback(int fd, short flags, void *param);
static void _ext_hub_process_command_callback(int fd, short flags, void *param);

//=====================================================================
//wakeup event begin
static int32_t m_wakeup_read_fd = -1;
static int32_t m_wakeup_write_fd = -1;
static ext_event_t m_wakeupfd_event;
//wakeup event end
//=====================================================================

//=====================================================================
// eventloop begin
static pthread_t m_tid_dispatch = -1;
static int32_t m_started = 0;

static pthread_mutex_t m_startup_mutex;
static pthread_cond_t m_startup_cond;
// eventloop end
//=====================================================================

//=====================================================================
// sockets begin
static int32_t m_listen_fd = -1;
static ext_event_t m_listen_event;

static int32_t m_command_fd = -1;
static ext_event_t m_commands_event;
static RecordStream *p_rs = NULL;

static pthread_mutex_t m_write_mutex;
// sockets end
//=====================================================================

static void _ext_hub_process_wakeup_callback(int32_t __unused fd, short __unused flags, void __unused *param)
{
    int32_t ret = 0;
    char buff[16] = {0};

    do {
        ret = read(m_wakeup_read_fd, &buff, sizeof(buff));
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}

static void _ext_hub_trigger_event_loop(void)
{
    int32_t ret = 0;
    if (!pthread_equal(pthread_self(), m_tid_dispatch)) {
        do {
            ret = write(m_wakeup_write_fd, " ", 1);
        } while (ret < 0 && errno == EINTR);
    }
}

static void _ext_hub_event_add_wakeup(ext_event_t *ev)
{
    silfp_ext_event_add(ev);
    _ext_hub_trigger_event_loop();
}

static void *_ext_hub_event_loop(void __unused *param)
{
    int32_t ret = 0;
    int32_t filedes[2] = {0};

    silfp_ext_event_init();

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

    ret = fcntl(m_wakeup_read_fd, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        LOG_MSG_ERROR("Error in fcntl() errno:%d", errno);
        return NULL;
    }

    silfp_ext_event_set(&m_wakeupfd_event, m_wakeup_read_fd, 1, _ext_hub_process_wakeup_callback, NULL);
    _ext_hub_event_add_wakeup(&m_wakeupfd_event);

    silfp_ext_event_loop();

    LOG_MSG_ERROR("error in event_loop_base errno:%d", errno);

    if (m_listen_fd >= 0) {
        close(m_listen_fd);
        m_listen_fd = -1;
    }
    if (m_command_fd >= 0) {
        close(m_command_fd);
        m_command_fd = -1;
    }

    close(m_wakeup_read_fd);
    close(m_wakeup_write_fd);

    if (p_rs != NULL) {
        record_stream_free(p_rs);
        p_rs = NULL;
    }

    return NULL;
}

static void _ext_hub_event_loop_start(void)
{
    int32_t ret = 0;

    /* spin up eventLoop thread and wait for it to get started */
    m_started = 0;
    pthread_mutex_lock(&m_startup_mutex);

    ret = pthread_create(&m_tid_dispatch, NULL, _ext_hub_event_loop, NULL);

    while (m_started == 0) {
        pthread_cond_wait(&m_startup_cond, &m_startup_mutex);
    }

    pthread_mutex_unlock(&m_startup_mutex);

    if (ret < 0) {
        LOG_MSG_ERROR("Failed to create dispatch thread errno:%d", errno);
        return;
    }
}

static void _ext_hub_event_register(void)
{
    int32_t ret = 0;

    if (m_started == 0) {
        _ext_hub_event_loop_start();
    }

    m_listen_fd = socket_local_server(SOCKET_NAME_DEFAULT, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (m_listen_fd < 0) {
        LOG_MSG_ERROR("Failed to get socket %s", SOCKET_NAME_DEFAULT);
        return;
    }

    ret = listen(m_listen_fd, 4);
    LOG_MSG_DEBUG("register m_listen_fd=%d", m_listen_fd);

    if (ret < 0) {
        LOG_MSG_ERROR("Failed to listen on control socket '%d': %s", m_listen_fd, strerror(errno));
        return;
    }

    silfp_ext_event_set(&m_listen_event, m_listen_fd, 0, _ext_hub_listen_callback, NULL);
    _ext_hub_event_add_wakeup(&m_listen_event);
}

static void _ext_hub_listen_callback(int32_t fd, short __unused flags, void __unused *param)
{
    int32_t ret = 0;

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);
    struct passwd *pwd = NULL;

    int32_t is_silead_socket = 0;

    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof(peeraddr);

    if (fd != m_listen_fd) {
        LOG_MSG_ERROR("accept fd(%d) not equal with listen fd(%d)", fd, m_listen_fd);
        _ext_hub_event_add_wakeup(&m_listen_event);
        return;
    }

    fd = accept(m_listen_fd, (struct sockaddr *) &peeraddr, &socklen);
    m_command_fd = fd;

    if (m_command_fd < 0 ) {
        LOG_MSG_ERROR("Error on accept() errno:%d", errno);
        _ext_hub_event_add_wakeup(&m_listen_event);
        return;
    }

    ret = getsockopt(m_command_fd, SOL_SOCKET, SO_PEERCRED, &creds, &szCreds);
    if (ret == 0 && szCreds > 0) {
        errno = 0;
        pwd = getpwuid(creds.uid);
        if (pwd != NULL) {
            if (strcmp(pwd->pw_name, "system") == 0) {
                is_silead_socket = 1;
            } else {
                LOG_MSG_ERROR("can't accept socket from process %s", pwd->pw_name);
            }
        } else {
            LOG_MSG_ERROR("Error on getpwuid() errno: %d", errno);
        }
    }

    if (!is_silead_socket) {
        close(m_command_fd);
        m_command_fd = -1;

        _ext_hub_event_add_wakeup(&m_listen_event);
        return;
    }

    ret = fcntl(m_command_fd, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        LOG_MSG_ERROR("Error setting O_NONBLOCK errno:%d", errno);
    }

    LOG_MSG_DEBUG("new connection m_command_fd = %d", m_command_fd);

    p_rs = record_stream_new(m_command_fd, MAX_COMMAND_BYTES);

    silfp_ext_event_set(&m_commands_event, m_command_fd, 1, _ext_hub_process_command_callback, p_rs);
    _ext_hub_event_add_wakeup(&m_commands_event);
}

static void _ext_hub_process_command_callback(int32_t fd, short __unused flags, void *param)
{
    int32_t ret = 0;
    void *p_record = NULL;
    size_t recordlen = 0;

    LOG_MSG_DEBUG("receive some data");

    if(fd != m_command_fd) {
        LOG_MSG_ERROR("read fd(%d) not equal with accept fd(%d)", fd, m_command_fd);
        return;
    }

    p_rs = (RecordStream *)param;

    for (;;) {
        ret = record_stream_get_next(p_rs, &p_record, &recordlen);

        if (ret == 0 && p_record == NULL) {
            break;
        } else if (ret < 0) {
            break;
        } else if (ret == 0) {
            silfp_ext_skt_test_cmd(p_record, recordlen);
        }
    }

    LOG_MSG_DEBUG("ret = %d, errno = %d", ret, errno);

    if (ret == 0 || !(errno == EAGAIN || errno == EINTR)) {
        if (ret != 0) {
            LOG_MSG_ERROR("error on reading command socket errno:%d", errno);
        } else {
            LOG_MSG_INFO("EOS.  Closing command socket.");
        }

        if (m_command_fd >= 0) {
            close(m_command_fd);
            m_command_fd = -1;
        }

        silfp_ext_event_del(&m_commands_event);

        if (p_rs != NULL) {
            record_stream_free(p_rs);
            p_rs = NULL;
        }

        if (m_listen_fd >= 0) {
            _ext_hub_event_add_wakeup(&m_listen_event);
        }
    }
}

static int32_t _ext_hub_data_blocking_write(int32_t fd, const void *buffer, size_t len)
{
    size_t offset = 0;
    const uint8_t *wbuffer;

    wbuffer = (const uint8_t *)buffer;

    while (offset < len) {
        ssize_t written = 0;
        do {
            written = write(fd, wbuffer + offset, len - offset);
        } while (written < 0 && ((errno == EINTR) || (errno == EAGAIN)));

        if (written >= 0) {
            offset += written;
        } else {
            // written < 0
            LOG_MSG_ERROR("unexpected error on write errno:%d", errno);
            close(fd);
            return -1;
        }
    }

    return 0;
}

int32_t silfp_ext_hub_send_response_raw(uint32_t cmdid, const void *data, size_t data_size)
{
    int32_t ret = 0;
    uint32_t header = 0;
    char p[4] = {0};

    int32_t fd = m_command_fd;
    if (fd < 0) {
        return -1;
    }

    if (data_size + sizeof(uint32_t) > MAX_COMMAND_BYTES) {
        LOG_MSG_ERROR("packet larger than %u (%u)", MAX_COMMAND_BYTES, (uint32_t)data_size);
        return -1;
    }

    pthread_mutex_lock(&m_write_mutex);

    p[0] = (cmdid >> 24) & 0xFF;
    p[1] = (cmdid >> 16) & 0xFF;
    p[2] = (cmdid >> 8) & 0xFF;
    p[3] = cmdid & 0xFF;

    header = htonl(data_size + sizeof(uint32_t));
    ret = _ext_hub_data_blocking_write(fd, (void *)&header, sizeof(header));
    if (ret >= 0) {
        ret = _ext_hub_data_blocking_write(fd, p, sizeof(uint32_t));
        ret = _ext_hub_data_blocking_write(fd, data, data_size);
    }

    pthread_mutex_unlock(&m_write_mutex);

    return ret;
}

void silfp_ext_hub_init(void)
{
    LOG_MSG_VERBOSE("init");

    m_wakeup_read_fd = -1;
    m_wakeup_write_fd = -1;

    m_tid_dispatch = -1;
    m_started = 0;
    pthread_mutex_init(&m_startup_mutex, NULL);
    pthread_cond_init(&m_startup_cond, NULL);
    pthread_mutex_init(&m_write_mutex, NULL);

    m_listen_fd = -1;
    m_command_fd = -1;
    p_rs = NULL;

    _ext_hub_event_loop_start();
    _ext_hub_event_register();

    silfp_util_set_str_value("vendor.silead.fp.ext.enable", 1);
}

void silfp_ext_hub_deinit(void)
{
    if (m_started) {
        silfp_ext_event_exit();
        _ext_hub_trigger_event_loop();
        m_started = 0;

        pthread_join(m_tid_dispatch, NULL);
        silfp_ext_event_deinit();

        pthread_cond_destroy(&m_startup_cond);
        pthread_mutex_destroy(&m_startup_mutex);
        pthread_mutex_destroy(&m_write_mutex);

        LOG_MSG_VERBOSE("deinit");
    }
}

#endif // SIL_FP_EXT_SKT_SERVER_ENABLE