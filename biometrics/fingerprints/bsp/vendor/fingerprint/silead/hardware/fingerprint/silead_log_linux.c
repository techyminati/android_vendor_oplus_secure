/******************************************************************************
 * @file   silead_log_linux.c
 * @brief  Contains dump log functions.
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
 * calvin wang     2018/8/2    0.1.0      Init version
 *
 *****************************************************************************/
#ifdef HOST_OS_LINUX

#define FILE_TAG "silead_log_linux"
#include "log/logmsg.h"

#ifndef LOG_TAG
#define LOG_TAG "log"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_DUMP_CA_BUF_SIZE 1024

static int32_t m_log_level = 1;

int32_t silfp_log_is_loggable(int32_t level)
{
    return (level >= m_log_level) ? 1 : 0;
}

void silfp_log_set_level(int32_t level)
{
    m_log_level = level;
}

void silfp_log_dump(const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_DUMP_CA_BUF_SIZE] = {0};
    int32_t buf_size = sizeof(buf);

    va_start(ap, fmt);
    vsnprintf(buf, buf_size, fmt, ap);
    va_end(ap);
    printf("%s %s\n", LOG_TAG, buf);
}

#endif /* HOST_OS_LINUX */