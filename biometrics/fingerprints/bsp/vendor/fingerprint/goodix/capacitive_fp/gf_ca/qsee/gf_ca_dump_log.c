/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_ca dump log file
 * History:
 * Version: 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <cutils/fs.h>
#include <android/log.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


#include "gf_error.h"
#include "gf_ca_dump_log.h"
#include "gf_type_define.h"

#define LOG_TAG "[gf_ca_entry]"
#define LOG_D(...) (__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOG_E(...) (__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#define PROPERTY_DUMP_TALOG_DATA "gf.debug.dump_talog_data"

#define GF_DUMP_DATA_ROOT_PATH "/data"
#define GF_DUMP_TALOG_DIR "/gf_data/talog"

#define GF_DUMP_FILE_PATH_MAX_LEN 256

typedef struct tm tm_t;

const uint8_t *g_log_level_strtable[] =  // log level strtable
{
    "GF_LOG_SILENT",
    "GF_LOG_ERROR",
    "GF_LOG_INFO",
    "GF_LOG_DEBUG",
    "GF_LOG_VERBOSE",
    "GF_LOG_UNKNOWN"
};

static uint8_t ta_log_file_path[GF_DUMP_FILE_PATH_MAX_LEN] = {0};  // ta log file path

volatile gf_log_level_t g_logdump_level = GF_LOG_SILENT;  // log dump level

/**
 * Function: gf_dump_data_to_file
 * Description: dump data to file
 * Input: file_path, data, length
 * Output: none
 * Return:
 * GF_SUCCESS  if succeed to dump data to file
 * others      if fail to dump data to file
 */
static gf_error_t gf_dump_data_to_file(const uint8_t *file_path, uint8_t *data,
                                       uint32_t length)
{
    FILE *file = NULL;
    uint32_t count = 0;
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == file_path || NULL == data)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        file = fopen(file_path, "a+");

        if (NULL == file)
        {
            break;
        }

        do
        {
            count += fwrite(data + count, sizeof(uint8_t), length - count, file);
        }
        while (count < length);

        fflush(file);
    }
    while (0);

    if (file != NULL)
    {
        fclose(file);
    }

    return err;
}

/**
 * Function: gf_dump_talog_to_file
 * Description: dump ta log to file
 * Input: logbuf, loglen
 * Output: none
 * Return:
 * GF_SUCCESS  if succeed to dump data to file
 * others      if fail to dump data to file
 */
static gf_error_t gf_dump_talog_to_file(uint8_t *logbuf, uint32_t loglen)
{
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t cur_time_str[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    time_t current_time = time(NULL);
    tm_t ptm = {0};
    tm_t *current_tm = NULL;
    gf_error_t err = GF_SUCCESS;
    int32_t ret = 0;

    do
    {
        if (0 == ta_log_file_path[0] || 0 != access(ta_log_file_path, F_OK))
        {
            current_tm = localtime_r(&current_time, &ptm);
            snprintf(cur_time_str, GF_DUMP_FILE_PATH_MAX_LEN,
                     "%04d-%02d-%02d-%02d-%02d-%02d",
                     current_tm->tm_year + 1900,
                     current_tm->tm_mon + 1, current_tm->tm_mday, current_tm->tm_hour,
                     current_tm->tm_min, current_tm->tm_sec);
            snprintf(dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s/",
                     GF_DUMP_DATA_ROOT_PATH, GF_DUMP_TALOG_DIR);
            ret = fs_mkdirs(dir, 0755);

            if (ret < 0)
            {
                LOG_E("[%s] make directory(%s) fail=%d", __func__, dir, ret);
                break;
            }

            memset(ta_log_file_path, 0x0, GF_DUMP_FILE_PATH_MAX_LEN);
            snprintf(ta_log_file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s/talog_%s.log", dir,
                     cur_time_str);
        }

        err = gf_dump_data_to_file(ta_log_file_path, (uint8_t *)logbuf, loglen);
    }
    while (0);

    return err;
}

#ifdef GF_TALOG_DUMP_TO_CA_SUPPORT

/**
 * Function: gf_ca_get_ta_logbuf_length
 * Description: get ta log buf length
 * Input: none
 * Output: none
 * Return: length of ta log buf
 */
uint32_t gf_ca_get_logbuf_length(void)
{
    uint32_t logbuf_len = 0;
    int8_t ret = 0;
    LOG_D("[%s] enter", __func__);

    do
    {
        ret = property_get_int32(PROPERTY_DUMP_TALOG_DATA, 0);

        if (ret >= 1)
        {
            if (ret > 1 && ret <= (uint32_t)GF_LOG_UNKNOWN)
            {
                g_logdump_level = (gf_log_level_t)(ret - 1);
            }
            else
            {
                g_logdump_level = GF_LOG_DEBUG;
            }

            logbuf_len = GF_EXTEND_SIZE + sizeof(gf_ta_log_info_t);
        }
        else
        {
            g_logdump_level = GF_LOG_SILENT;
        }
    }
    while (0);

    LOG_D("[%s] exit", __func__);
    return logbuf_len;
}

/**
 * Function: gf_ca_ta_logdump
 * Description: dump ta log
 * Input: log_ptr
 * Output: none
 * Return: none
 */
void gf_ca_logdump(gf_ta_log_info_t *log_ptr)
{
    gf_ta_log_info_t *log_info = log_ptr;
    LOG_D("[%s] enter", __func__);

    do
    {
        if (GF_LOG_SILENT != g_logdump_level && log_ptr != NULL)
        {
            uint32_t i = 0;
            uint32_t n = 0;
            uint8_t logbuf[GF_LOG_OUTPUT_LENGTH] = {0};
            LOG_D("[%s] TA log info, log length=%d, logbuf index=%d, log dump level=%s\n",
                  __func__, log_info->logbuf_sum, log_info->logbuf_index,
                  g_log_level_strtable[log_info->logbuf_dump_level]);

            if (log_info->logbuf_sum > log_info->logbuf_index)
            {
                n = log_info->logbuf_index / GF_LOG_OUTPUT_LENGTH +
                    (log_info->logbuf_index % GF_LOG_OUTPUT_LENGTH > 0 ? 1 : 0);

                for (i = n; i < GF_TA_LOG_BUF_LENGTH / GF_LOG_OUTPUT_LENGTH; i++)
                {
                    memset(logbuf, 0x0, GF_LOG_OUTPUT_LENGTH);
                    n = snprintf(logbuf, GF_LOG_OUTPUT_LENGTH, "%s",
                                 &log_info->logbuf[i * GF_LOG_OUTPUT_LENGTH]);
                    LOG_D("%s", logbuf);
                    gf_dump_talog_to_file(logbuf, n);
                }
            }

            for (i = 0; i < log_info->logbuf_index / GF_LOG_OUTPUT_LENGTH; i++)
            {
                memset(logbuf, 0x0, GF_LOG_OUTPUT_LENGTH);
                n = snprintf(logbuf, GF_LOG_OUTPUT_LENGTH, "%s",
                             &log_info->logbuf[i * GF_LOG_OUTPUT_LENGTH]);
                LOG_D("%s", logbuf);
                gf_dump_talog_to_file(logbuf, n);
            }

            if (log_info->logbuf_index % GF_LOG_OUTPUT_LENGTH)
            {
                memset(logbuf, 0x0, GF_LOG_OUTPUT_LENGTH);
                n = snprintf(logbuf, GF_LOG_OUTPUT_LENGTH, "%s",
                             &log_info->logbuf[i * GF_LOG_OUTPUT_LENGTH]);
                LOG_D("%s", logbuf);
                gf_dump_talog_to_file(logbuf, n);
            }
        }  // if...
    } while (0);  // do...

    LOG_D("[%s] exit", __func__);
}

#else  // ifndef GF_TALOG_DUMP_TO_CA_SUPPORT

/**
 * Function: gf_ca_get_ta_logbuf_length
 * Description: get ta log buf length
 * Input: none
 * Output: none
 * Return: 0 (QSEE_LOG_DUMP_SUPPORT is not defined)
 */
uint32_t gf_ca_get_logbuf_length(void)
{
    return 0;
}

/**
 * Function: gf_ca_ta_logdump
 * Description: dump ta log
 * Input: log_ptr
 * Output: none
 * Return: none (QSEE_LOG_DUMP_SUPPORT is not defined)
 */
void gf_ca_logdump(gf_ta_log_info_t *log_ptr)
{
    UNUSED_VAR(log_ptr);
    return;
}

#endif  // end ifdef GF_TALOG_DUMP_TO_CA_SUPPORT
