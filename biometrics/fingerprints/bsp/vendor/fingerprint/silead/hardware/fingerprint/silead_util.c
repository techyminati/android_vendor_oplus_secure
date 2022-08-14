/******************************************************************************
 * @file   silead_util_android.c
 * @brief  Contains fingerprint utilities functions file.
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
 * calvin wang  2018/1/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_util"
#include "log/logmsg.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "silead_error.h"
#include "silead_const.h"
#include "silead_util.h"
#include "silead_util_ext.h"

#define BAK_SUFFIX ".bak"

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

#define SIL_USE_SELF_SEC_TO_DATE
#ifdef SIL_USE_SELF_SEC_TO_DATE
int32_t silfp_util_msec_to_date(uint64_t msec, char *datastr, uint32_t len, uint32_t mode, uint32_t format)
{
    int32_t ret = 0;
    uint32_t year = SL_BEGIN_YEAR;
    uint32_t month = 1;
    uint32_t date = 1;
    uint32_t hour = 0;
    uint32_t minite = 0;
    uint32_t second = 0;

    uint32_t days = 0;
    uint32_t seds = 0;
    uint32_t leapyear = 0;
    uint64_t seconds = msec / 1000;

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

    if (format == MODE_GET_SEC_FORMAT_TIMESTAMP) {
        if (mode == MODE_GET_SEC_TYPE_TIME) {
            ret = snprintf(datastr, len, "%02d:%02d:%02d.%03d", hour, minite, second, (int32_t)(msec % 1000));
        } else if (mode == MODE_GET_SEC_TYPE_DATE) {
            ret = snprintf(datastr, len, "%02d-%02d-%02d", year % 100, month, date);
        } else {
            ret = snprintf(datastr, len, "%02d-%02d-%02d %02d:%02d:%02d.%03d", year % 100, month, date, hour, minite, second, (int32_t)(msec % 1000));
        }
    } else {
        if (mode == MODE_GET_SEC_TYPE_TIME) {
            ret = snprintf(datastr, len, "%02d%02d%02d%03d", hour, minite, second, (int32_t)(msec % 1000));
        } else if (mode == MODE_GET_SEC_TYPE_DATE) {
            ret = snprintf(datastr, len, "%02d%02d%02d", year % 100, month, date);
        } else {
            ret = snprintf(datastr, len, "%02d%02d%02d-%02d%02d%02d%03d", year % 100, month, date, hour, minite, second, (int32_t)(msec % 1000));
        }
    }

    return ret;
}
#else
int32_t silfp_util_msec_to_date(uint64_t msec, char *datastr, uint32_t len, uint32_t mode, uint32_t format)
{
    int32_t ret = 0;
    uint64_t seconds = msec / 1000;
    time_t timep = (time_t)seconds;
    struct tm tday;

    if (datastr == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    localtime_r(&timep, &tday);

    if (format == MODE_GET_SEC_FORMAT_TIMESTAMP) {
        if (mode == MODE_GET_SEC_TYPE_TIME) {
            ret = snprintf(datastr, len, "%02d:%02d:%02d.%03d", tday.tm_hour, tday.tm_min, tday.tm_sec, (int32_t)(msec % 1000));
        } else if (mode == MODE_GET_SEC_TYPE_DATE) {
            ret = snprintf(datastr, len, "%04d-%02d-%02d", (1900 + tday.tm_year), (1 + tday.tm_mon), tday.tm_mday);
        } else {
            ret = snprintf(datastr, len, "%04d-%02d-%02d %02d:%02d:%02d.%03d", (1900 + tday.tm_year), (1 + tday.tm_mon), tday.tm_mday, tday.tm_hour, tday.tm_min, tday.tm_sec, (int32_t)(msec % 1000));
        }
    } else {
        if (mode == MODE_GET_SEC_TYPE_TIME) {
            ret = snprintf(datastr, len, "%02d%02d%02d%03d", tday.tm_hour, tday.tm_min, tday.tm_sec, (int32_t)(msec % 1000));
        } else if (mode == MODE_GET_SEC_TYPE_DATE) {
            ret = snprintf(datastr, len, "%04d%02d%02d", (1900 + tday.tm_year), (1 + tday.tm_mon), tday.tm_mday);
        } else {
            ret = snprintf(datastr, len, "%04d%02d%02d-%02d%02d%02d%03d", (1900 + tday.tm_year), (1 + tday.tm_mon), tday.tm_mday, tday.tm_hour, tday.tm_min, tday.tm_sec, (int32_t)(msec % 1000));
        }
    }

    return ret;
}
#endif /* SIL_USE_SELF_SEC_TO_DATE */

int32_t silfp_util_msec_to_hours(uint64_t msec, uint8_t zone_fix)
{
    uint64_t seconds = msec / 1000;

    return silfp_util_seconds_to_hours(seconds, zone_fix);
}

int32_t silfp_util_seconds_to_hours(uint64_t seconds, uint8_t zone_fix)
{
    uint32_t hour = 0;
    uint32_t seds = 0;

    if (zone_fix) {
        seconds += SL_ZONE_OFFSET_8;
    }
    seds = seconds % SL_SEC_PER_DAY;

    hour = seds / SL_SEC_PER_HOUR;

    return hour;
}

uint64_t silfp_util_get_seconds(void)
{
    uint64_t msec = 0;
    time_t timep;

    time(&timep);
    msec = (uint64_t)timep;

    return msec;
}

int64_t silfp_util_get_milliseconds(void)
{
    struct timespec ts;
    int64_t timestamp = 0;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    timestamp = ((int64_t)ts.tv_sec * 1000) + ((int64_t)ts.tv_nsec / 1000000);
    LOG_MSG_DEBUG("timestamp = %" PRIu64, timestamp);
    return timestamp;
}

uint64_t silfp_util_get_msec(void)
{
    struct timeval now;
    uint64_t msec = 0;

    gettimeofday(&now, NULL);
    msec = now.tv_sec;
    msec *= 1000;
    msec += now.tv_usec/1000;

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

    memcpy(p, src, count);
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

    if (path != NULL && path[0] != '\0') {
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
        LOG_MSG_ERROR("param invalid (%u)", len);
        return -SL_ERROR_BAD_PARAMS;
    }

    if (path != NULL && path[0] != '\0') {
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

int32_t silfp_util_make_dirs(const char *path)
{
    int32_t i = 0;
    int32_t len = 0;
    char dir_name[MAX_PATH_LEN] = {0};

    if (path == NULL || path[0] == '\0') {
        return -SL_ERROR_BAD_PARAMS;
    }

    silfp_util_path_copy(dir_name, sizeof(dir_name), path, strlen(path));
    len = strlen(dir_name);

    if (access(dir_name, F_OK) == 0) {
        return 0;
    }

    for (i = 1; dir_name[i] != '\0'; i++) {
        if (dir_name[i] == '/') {
            dir_name[i] = '\0';
            if (access(dir_name, F_OK) != 0) {
                if (silfp_util_make_dir(dir_name) == -1) {
                    LOG_MSG_DEBUG("mkdir error (%d:%s)", errno, strerror(errno));
                    return -1;
                }
            }
            dir_name[i] = '/';
        }
    }

    if (access(dir_name, F_OK) != 0) {
        if (silfp_util_make_dir(dir_name) == -1) {
            LOG_MSG_DEBUG("mkdir error (%d:%s)", errno, strerror(errno));
        }
    }

    return 0;
}

int32_t silfp_util_file_save_dump_img(const char *path, const char *name, const char *buf, uint32_t len)
{
    if (path != NULL) {
        silfp_util_make_dirs(path);
    }
    return silfp_util_file_save(path, name, buf, len);
}