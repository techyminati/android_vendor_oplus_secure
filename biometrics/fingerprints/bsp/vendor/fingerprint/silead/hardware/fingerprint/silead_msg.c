/******************************************************************************
 * @file   silead_msg.c
 * @brief  Contains pipe communication functions.
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
 * Luke Ma     2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_msg"
#include "log/logmsg.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include "silead_msg.h"

#define STR_CANCEL "c"
#define STR_IRQ "i"
#define STR_DOWN "d"
#define STR_UP "u"
#define MSG_LEN 1

static int32_t s_fdWakeupRead = -1;
static int32_t s_fdWakeupWrite = -1;
static int32_t s_inited = 0;

static int32_t s_finger_down = 0;

struct pollfd mPollFds;

int32_t silfp_msg_init(void)
{
    int32_t ret = 0;
    int32_t filedes[2];

    LOG_MSG_VERBOSE("init");

    s_fdWakeupRead = -1;
    s_fdWakeupWrite = -1;
    s_inited = 0;
    s_finger_down = 0;

    ret = pipe(filedes);
    if (ret >= 0) {
        s_fdWakeupRead = filedes[0];
        s_fdWakeupWrite = filedes[1];

        fcntl(s_fdWakeupRead, F_SETFL, O_NONBLOCK | O_NOATIME);
        fcntl(s_fdWakeupWrite, F_SETFL, O_NONBLOCK);

        mPollFds.fd = s_fdWakeupRead;
        mPollFds.events = POLLIN;
        mPollFds.revents = 0;

        s_inited = 1;
    } else {
        LOG_MSG_ERROR("pipe failed (%d:%s)", errno, strerror(errno));
    }

    return 0;
}

void silfp_msg_deinit(void)
{
    if (s_inited) {
        close(s_fdWakeupRead);
        s_fdWakeupRead = -1;
        close(s_fdWakeupWrite);
        s_fdWakeupWrite = -1;
    }

    LOG_MSG_VERBOSE("deinit");
}

void silfp_msg_send(int32_t type)
{
    int32_t ret = 0;
    char *buf = NULL;

    if (!s_inited) {
        return;
    }

    if (SIFP_MSG_IRQ == type) {
        buf = STR_IRQ;
    } else if (SIFP_MSG_CANCEL == type) {
        buf = STR_CANCEL;
    } else if (SIFP_MSG_DOWN == type) {
        buf = STR_DOWN;
        s_finger_down = 1;
    } else if (SIFP_MSG_UP == type) {
        buf = STR_UP;
        s_finger_down = 0;
    } else {
        LOG_MSG_ERROR("invalid type=%d", type);
        return;
    }

    do {
        ret = write(s_fdWakeupWrite, buf, MSG_LEN);
        LOG_MSG_VERBOSE("write buf=%s, ret=%d", buf, ret);
    } while (ret < 0 && errno == EINTR);
}

static int32_t _msg_read_msg(void)
{
    ssize_t count = 0;
    char buff[16] = {0};

    do {
        count = read(s_fdWakeupRead, buff, MSG_LEN);
        LOG_MSG_VERBOSE("buff=%s, count=%zd", buff, count);
    } while (count < 0 && errno == EINTR);

    if (count > 0) {
        if (memcmp(buff, STR_CANCEL, MSG_LEN) == 0) {
            return SIFP_MSG_CANCEL;
        } else if (memcmp(buff, STR_IRQ, MSG_LEN) == 0) {
            return SIFP_MSG_IRQ;
        } else if (memcmp(buff, STR_DOWN, MSG_LEN) == 0) {
            return SIFP_MSG_DOWN;
        } else if (memcmp(buff, STR_UP, MSG_LEN) == 0) {
            return SIFP_MSG_UP;
        }
    }
    return SIFP_MSG_UNKNOW;
}

void silfp_msg_clean(void)
{
    char buff[1024] = {0};
    int32_t ret = 0;

    if (!s_inited) {
        return;
    }

    do {
        ret = read(s_fdWakeupRead, &buff, sizeof(buff));
        LOG_MSG_VERBOSE("buff=%s, ret=%d", buff, ret);
    } while (ret > 0 || (ret < 0 && errno == EINTR));
}

int32_t silfp_msg_wait_finger_status(int32_t irq, int32_t down, int32_t up, int32_t cancel)
{
    int32_t n = 0;
    int32_t ret = 0;

    if (!s_inited) {
        return SIFP_MSG_UNKNOW;
    }

    for (;;) {
        LOG_MSG_VERBOSE("poll iduc(%d,%d,%d,%d)", irq, down, up, cancel);

        n = poll(&mPollFds, 1, -1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOG_MSG_ERROR("poll error (%d)", errno);
        }

        if (mPollFds.revents & POLLIN) {
            mPollFds.revents = 0;
            ret = _msg_read_msg();
            if ((ret == SIFP_MSG_IRQ) && (irq || down || up)) {
                return ret;
            } else if ((ret == SIFP_MSG_DOWN) && (irq || down)) {
                return ret;
            } else if ((ret == SIFP_MSG_UP) && (irq || up)) {
                return ret;
            } else if (ret == SIFP_MSG_CANCEL && cancel) {
                return ret;
            }
        }
    }
    return SIFP_MSG_UNKNOW;
}


int32_t silfp_msg_is_finger_down(void)
{
    return s_finger_down;
}

void silfp_msg_reset_finger_status(void)
{
    s_finger_down = 0;
}