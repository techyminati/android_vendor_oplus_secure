/******************************************************************************
 * @file   silead_util_android.c
 * @brief  Contains fingerprint utilities functions file.
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

#define FILE_TAG "silead_util"
#include "log/logmsg.h"

#ifdef __unused
#undef __unused
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "silead_error.h"
#include "silead_const.h"

#define BAK_SUFFIX ".bak"

#ifdef SIL_USE_SELF_SEC_TO_DATE

#define SL_BEGIN_YEAR                   1969
#define SL_SEC_PER_MINITE               60
#define SL_SEC_PER_HOUR                 (60*60)
#define SL_SEC_PER_DAY                  (60*60*24)
#define SL_MIN_PER_HOUR                 60
#define SL_ZONE_OFFSET_8                (8*60*60)
#define SL_DAYS_PER_YEAR                365
#define SL_DAYS_3_YEARS_1_LEAP_YEAR     1461
#define SL_DAYS_3_YEARS                 1095
#define SL_DAYS_PER_MONTH_31            31
#define SL_DAYS_PER_MONTH_30            30
#define SL_DAYS_JAN_FEB                 59
#define SL_DAYS_PER_2_MONTH             61
#define SL_DAYS_BEFORE_JUL              181
#define SL_DAYS_BEFORE_SEP              243
#define SL_START_MONTH_1                1
#define SL_START_MONTH_3                3
#define SL_START_MONTH_7                7
#define SL_START_MONTH_9                9
#define SL_LEAP_YEAR_CIRCLE             4

int32_t silfp_util_seconds_to_date(uint64_t seconds, char *datastr, uint32_t len)
{
    uint32_t year = SL_BEGIN_YEAR;
    uint32_t month = 1;
    uint32_t date = 1;
    uint32_t hour = 0;
    uint32_t minite = 0;
    uint32_t second = 0;

    uint32_t days = 0;
    uint32_t seds = 0;
    uint32_t leapyear = 0;

    if (datastr == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    seconds += SL_ZONE_OFFSET_8;
    days = seconds / SL_SEC_PER_DAY + SL_DAYS_PER_YEAR;
    seds = seconds % SL_SEC_PER_DAY;

    hour = seds / SL_SEC_PER_HOUR;
    minite = (seds % SL_SEC_PER_HOUR) / SL_SEC_PER_MINITE;
    second =  seds % SL_SEC_PER_MINITE;

    year += days / SL_DAYS_3_YEARS_1_LEAP_YEAR * SL_LEAP_YEAR_CIRCLE;
    days = days % SL_DAYS_3_YEARS_1_LEAP_YEAR;
    year += (days <= SL_DAYS_3_YEARS) ? (days / SL_DAYS_PER_YEAR) : (SL_LEAP_YEAR_CIRCLE - 1);
    leapyear = (days > SL_DAYS_3_YEARS) ? 1 : 0;
    days = leapyear ? (days - SL_DAYS_3_YEARS) : (days % SL_DAYS_PER_YEAR);

    if (days <= (SL_DAYS_JAN_FEB + leapyear)) {
        month = SL_START_MONTH_1 + days / SL_DAYS_PER_MONTH_31;
        date = (days % SL_DAYS_PER_MONTH_31) + 1;
    } else if (days <= (SL_DAYS_BEFORE_JUL + leapyear)) {
        month = SL_START_MONTH_3 + (days - SL_DAYS_JAN_FEB - leapyear) / SL_DAYS_PER_2_MONTH * 2 + 
            ((days - SL_DAYS_JAN_FEB - leapyear) % SL_DAYS_PER_2_MONTH) / SL_DAYS_PER_MONTH_31;
        date = ((days - SL_DAYS_JAN_FEB - leapyear) % SL_DAYS_PER_2_MONTH) % SL_DAYS_PER_MONTH_31 + 1;
    } else if (days <= (SL_DAYS_BEFORE_SEP + leapyear)) {
        month = SL_START_MONTH_7 + (days - SL_DAYS_BEFORE_JUL - leapyear) / SL_DAYS_PER_MONTH_31;
        date = ((days - SL_DAYS_BEFORE_JUL - leapyear) % SL_DAYS_PER_MONTH_31) + 1;
    } else if (days <= (SL_DAYS_PER_2_MONTH + leapyear)) {
        month = SL_START_MONTH_9 + (days - SL_DAYS_BEFORE_SEP - leapyear) / SL_DAYS_PER_2_MONTH * 2 +
            ((((days - SL_DAYS_BEFORE_SEP - leapyear) % SL_DAYS_PER_2_MONTH) > SL_DAYS_PER_MONTH_31) ? 1 : 0);
        days = (days - SL_DAYS_BEFORE_SEP - leapyear) % SL_DAYS_PER_2_MONTH;
        date = ((days > SL_DAYS_PER_MONTH_30) ? days - SL_DAYS_PER_MONTH_30 : days) - 1;
    }

    snprintf(datastr, len, "%04d%02d%02d-%02d%02d%02d", year % 100, month, date, hour, minite, second);

    return 0;
}

#else

int32_t silfp_util_seconds_to_date(uint64_t seconds, char *datastr, uint32_t len)
{
    time_t timep = (time_t)seconds;
    struct tm *p;

    if (datastr == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    p = localtime(&timep);
    snprintf(datastr, len, "%04d%02d%02d-%02d%02d%02d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    return 0;
}

#endif /* SIL_USE_SELF_SEC_TO_DATE */

uint64_t silfp_util_get_seconds(void)
{
    uint64_t msec = 0;
    time_t timep;

    time(&timep);
    msec = (uint64_t)timep;

    return msec;
}

int32_t silfp_util_open_file(const char *path, int32_t append)
{
    int32_t fd = -1;

    if (path == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (append) {
        fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0644);
    } else {
        fd = open(path, O_RDWR | O_CREAT, 0644);
    }

    return fd;
}

int32_t silfp_util_write_file(int32_t fd, const void *buf, uint32_t len)
{
    int32_t ret = 0;

    const unsigned char *p = (const unsigned char *)buf;
    int32_t count = len;

    if (fd < 0) {
        LOG_MSG_ERROR("file not open");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (buf == NULL || len == 0) {
        LOG_MSG_ERROR("content empty");
        return 0;
    }

    do {
        ret = write(fd, p, count);
        if (ret > 0)  {
            count -= ret;
            p += ret;
        } else if (!(ret < 0 && errno == EINTR)) {
            break;
        }
    } while(count > 0);

    if (count > 0) {
        LOG_MSG_DEBUG("write fail (%d:%s)", errno, strerror(errno));
    }

    return ret;
}

int32_t silfp_util_close_file(int32_t fd)
{
    if (fd < 0) {
        LOG_MSG_ERROR("file not open");
        return -SL_ERROR_BAD_PARAMS;
    }
    return close(fd);
}

int32_t silfp_util_strcpy(void *dst, uint32_t size, const void *src, uint32_t len)
{
    uint32_t count = len;
    char *p = (char *)dst;

    if (dst == NULL || size == 0 || src == NULL || len == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (count > size - 1) {
        count = size - 1;
    }

    strncpy(p, src, count);
    p[count] = '\0';

    return count;
}

int32_t silfp_util_path_copy(void *dst, uint32_t size, const void *src, uint32_t len)
{
    int32_t count = 0;
    char *p = (char *)dst;

    count = silfp_util_strcpy(dst, size, src, len);
    if (count > 0) {
        if (p[count - 1] == '\\') {
            p[count - 1] = '\0';
        }
    }

    return count;
}

int32_t silfp_util_file_get_size(const char *fname)
{
    struct stat st;

    if (fname == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    LOG_MSG_VERBOSE("getsize %s", fname);

    if (stat(fname, &st) < 0) {
        LOG_MSG_ERROR("getsize %s failed (%d:%s)", fname, errno, strerror(errno));
        return -1;
    }

    return st.st_size;
}

int32_t silfp_util_file_remove(const char *fname)
{
    if (fname == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -1;
    }

    LOG_MSG_VERBOSE("remove %s", fname);

    return unlink(fname);
}

int32_t silfp_util_file_load(const char *path, const char *name, char *buf, uint32_t len)
{
    int32_t ret = 0;
    FILE *fp = NULL;
    char *p = NULL;
    int32_t count = 0;
    char fname[MAX_PATH_LEN] = {0};

    if (name == NULL || buf == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (path != NULL) {
        snprintf(fname, sizeof(fname), "%s/%s", path, name);
    } else {
        snprintf(fname, sizeof(fname), "%s", name);
    }

    LOG_MSG_VERBOSE("load %s", fname);

    if ((fp = fopen(fname, "rb")) == NULL) {
        LOG_MSG_ERROR("open %s failed (%d:%s)", fname, errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    p = buf;
    count = len;

    fseek(fp, 0, SEEK_SET);
    do {
        ret = fread(p, sizeof(char), count, fp);
        if (ret > 0) {
            count -= ret;
            p += ret;
        } else if (!(ret < 0 && errno == EINTR)) {
            break;
        }
    } while(count > 0);

    fclose(fp);

    if (count > 0) {
        LOG_MSG_ERROR("read %s fail (%d:%s)", fname, errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    ret = len;

    return ret;
}

int32_t silfp_util_file_save(const char *path, const char *name, const char *buf, uint32_t len)
{
    int32_t ret = 0;
    FILE *fp = NULL;
    const char *p = NULL;
    int32_t count = 0;
    char fname[MAX_PATH_LEN] = {0};
    char fname_bak[MAX_PATH_LEN] = {0};

    if (name == NULL || buf == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (path != NULL) {
        snprintf(fname, sizeof(fname), "%s/%s", path, name);
    } else {
        snprintf(fname, sizeof(fname), "%s", name);
    }

    snprintf(fname_bak, sizeof(fname_bak), "%s%s", fname, BAK_SUFFIX);
    LOG_MSG_VERBOSE("save %s", fname);

    if ((fp = fopen(fname_bak, "wb")) == NULL) {
        LOG_MSG_ERROR("open %s fail (%d:%s)", fname_bak, errno, strerror(errno));
        return -SL_ERROR_STO_OP_FAILED;
    }

    p = (char *)buf;
    count = len;

    do {
        ret = fwrite(p, sizeof(char), count, fp);
        if (ret > 0) {
            count -= ret;
            p += ret;
        } else if (!(ret < 0 && errno == EINTR)) {
            break;
        }
    } while(count > 0);

    fclose(fp);

    if (count > 0) {
        LOG_MSG_ERROR("write %s fail (%d:%s)", fname_bak, errno, strerror(errno));
        silfp_util_file_remove(fname_bak);
        return -SL_ERROR_STO_OP_FAILED;
    }

    ret = rename(fname_bak, fname);
    if (ret < 0) {
        LOG_MSG_ERROR("rename %s --> %s failed (%d:%s)", fname_bak, fname, errno, strerror(errno));
    } else {
        ret = len;
    }

    return ret;
}

int32_t silfp_util_file_rename(const char *srcname, const char *dstname)
{
    if (NULL == srcname || NULL == dstname) {
        LOG_MSG_ERROR("param invalid");
        return -1;
    }

    LOG_MSG_VERBOSE("rename %s --> %s", srcname, dstname);

    return rename(srcname, dstname);
}