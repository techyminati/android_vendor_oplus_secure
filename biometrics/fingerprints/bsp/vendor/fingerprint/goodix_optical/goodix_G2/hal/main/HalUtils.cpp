/************************************************************************************
 ** File: - FingerprintCore.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      hal utils implementation for goodix optical fingerprint (android p)
 **
 ** Version: 1.0
 ** Date : 18:03:11,17/10/2018
 ** Author: wudongnan@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  Dongnan.Wu   2019/04/18      modify the way to get timestamp
 ************************************************************************************/

#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cutils/fs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <utils/String8.h>
#include "HalUtils.h"
#include "HalLog.h"
#include <utils/SystemClock.h>

#define LOG_TAG "[HAL][HalUtils]"

#define MAX_FILE_NAME_LENGTH (256)

using android::String8;

namespace goodix
{
    int64_t HalUtils::getCurrentTimeMicrosecond()
    {
        struct timeval now;
        memset(&now, 0, sizeof(timeval));
        gettimeofday(&now, 0);
        return now.tv_sec * 1000000L + now.tv_usec;
    }

    int64_t HalUtils::getrealtime()
    {
        int64_t current_time = android::elapsedRealtime();
        return current_time;
    }

    gf_error_t HalUtils::genTimestamp(char* timestamp, uint32_t len)
    {
        gf_error_t err = GF_SUCCESS;
        struct timeval tv;
        memset(&tv, 0, sizeof(timeval));
        struct tm current_tm;
        memset(&current_tm, 0, sizeof(tm));
        char cur_time_str[MAX_FILE_NAME_LENGTH] = { 0 };
        do
        {
            if (nullptr == timestamp || len < TIME_STAMP_LEN)
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            gettimeofday(&tv, NULL);
            localtime_r(&tv.tv_sec, &current_tm);
            snprintf(cur_time_str, sizeof(cur_time_str), "%04d-%02d-%02d-%02d-%02d-%02d-%06ld",
                    current_tm.tm_year + 1900, current_tm.tm_mon + 1, current_tm.tm_mday,
                    current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec, tv.tv_usec);
            if (len < strlen(cur_time_str))
            {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] timestamp len too short.", __func__);
                break;
            }
        }
        while (0);
        if (GF_SUCCESS == err)
        {
            memcpy(timestamp, cur_time_str, strlen(cur_time_str));
        }
        return err;
    }

    gf_error_t HalUtils::genTimestamp2(char* timestamp, uint32_t len)
    {
        gf_error_t err = GF_SUCCESS;
        struct timeval tv;
        memset(&tv, 0, sizeof(timeval));
        struct tm current_tm;
        memset(&current_tm, 0, sizeof(tm));
        char cur_time_str[MAX_FILE_NAME_LENGTH] = { 0 };
        do
        {
            if (nullptr == timestamp || len < TIME_STAMP_LEN)
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            gettimeofday(&tv, NULL);
            localtime_r(&tv.tv_sec, &current_tm);
            snprintf(cur_time_str, sizeof(cur_time_str), "%04d%02d%02d-%02d%02d%02d",
                    current_tm.tm_year + 1900, current_tm.tm_mon + 1, current_tm.tm_mday,
                    current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec);
            if (len < strlen(cur_time_str))
            {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] timestamp len too short.", __func__);
                break;
            }
        }
        while (0);
        if (GF_SUCCESS == err)
        {
            memcpy(timestamp, cur_time_str, strlen(cur_time_str));
        }
        return err;
    }

    gf_error_t HalUtils::mkdirs(const char* dir)
    {
        gf_error_t err = GF_SUCCESS;
        int32_t len = 0;
        int32_t i = 0;
        char _dir[MAX_FILE_NAME_LENGTH] = { 0 };

        do
        {
            if (nullptr == dir)
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            snprintf(_dir, sizeof(_dir), "%s", dir);

            if (_dir[strlen(_dir) - 1] != '/')
            {
                snprintf(_dir, sizeof(_dir), "%s/", dir);
            }

            len = strlen(_dir);

            for (i = 1; i < len; i++)
            {
                if (_dir[i] == '/')
                {
                    _dir[i] = 0;

                    if (access(_dir, F_OK) != 0)
                    {
                        if (mkdir(_dir, 0755) == -1)
                        {
                            LOG_E(LOG_TAG, "[%s] mkdir failed, errno = %d", __func__, errno);
                            err = GF_ERROR_MKDIR_FAILED;
                            break;
                        }
                    }

                    _dir[i] = '/';
                }
            }
        }
        while (0);
        return err;
    }

    gf_error_t HalUtils::readFile(char *filepath, char *filename, uint8_t *read_buf, uint32_t read_buf_len)
    {
        gf_error_t err = GF_SUCCESS;
        FILE *file = NULL;
        int32_t file_len = 0;
        char path[MAX_FILE_NAME_LENGTH] = {0};

        FUNC_ENTER();

        do
        {
            GF_NULL_BREAK(filepath, err);
            GF_NULL_BREAK(filename, err);
            GF_NULL_BREAK(read_buf, err);

            if (filepath[strlen(filepath) - 1] != '/')
            {
                snprintf(path, MAX_FILE_NAME_LENGTH, "%s/%s", filepath, filename);
            }
            else
            {
                snprintf(path, MAX_FILE_NAME_LENGTH, "%s%s", filepath, filename);
            }

            file = fopen(path, "rb");

            GF_NULL_BREAK(file, err);

            file_len = fread(read_buf, sizeof(uint8_t), read_buf_len, file);
            if (file_len <= 0)
            {
                LOG_E(LOG_TAG, "[%s] read file(%s) fail", __func__, path);
                err = GF_ERROR_FILE_READ_FAILED;
                break;
            }
        } while (0);

        if (NULL != file)
        {
            fclose(file);
            file = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalUtils::writeFile(char *filepath, char *filename, uint8_t *write_buf, uint32_t write_buf_len)
    {
        gf_error_t err = GF_SUCCESS;
        int32_t ret = 0;
        FILE *file = NULL;
        char dir[MAX_FILE_NAME_LENGTH] = {0};
        char path[MAX_FILE_NAME_LENGTH] = {0};
        FUNC_ENTER();

        do
        {
            GF_NULL_BREAK(filepath, err);
            GF_NULL_BREAK(filename, err);
            GF_NULL_BREAK(write_buf, err);

            if (filepath[strlen(filepath) - 1] != '/')
            {
                snprintf(dir, MAX_FILE_NAME_LENGTH, "%s/", filepath);
            }
            else
            {
                snprintf(dir, MAX_FILE_NAME_LENGTH, "%s", filepath);
            }

            if (access(dir, F_OK) != 0)
            {
                ret = mkdirs(dir);
                if (ret < 0)
                {
                    LOG_E(LOG_TAG, "[%s] make directory(%s) fail, ret=%d", __func__, dir, err);
                    err = GF_ERROR_MKDIR_FAILED;
                    break;
                }
            }

            snprintf(path, MAX_FILE_NAME_LENGTH, "%s%s", dir, filename);
            LOG_D(LOG_TAG, "[%s] write file (%s)", __func__, path);
            file = fopen(path, "wb");
            GF_NULL_BREAK(file, err);

            if (write_buf_len != fwrite(write_buf, sizeof(uint8_t),
                                                 write_buf_len, file))
            {
                LOG_E(LOG_TAG, "[%s] write file(%s) fail", __func__, path);
                err = GF_ERROR_FILE_WRITE_FAILED;
                break;
            }
        } while (0);

        if (NULL != file)
        {
            fclose(file);
            file = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalUtils::makedirsByFilePath(const char* filepath)
    {
        gf_error_t err = GF_SUCCESS;
        do
        {
            FUNC_ENTER();
            if (nullptr == filepath)
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            String8 str(filepath);
            String8 dir = str.getPathDir();
            err = HalUtils::mkdirs(dir.string());
            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] mkdirs (%s) error", __func__, dir.string());
                break;
            }
            FUNC_EXIT(err);
        }
        while (0);
        return err;
    }
}  // namespace goodix
