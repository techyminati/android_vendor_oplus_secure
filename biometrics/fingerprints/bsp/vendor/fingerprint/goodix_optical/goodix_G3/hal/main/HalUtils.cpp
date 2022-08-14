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
#include "FingerprintCore.h"

#define LOG_TAG "[HAL][HalUtils]"

#define MAX_FILE_NAME_LENGTH (256)

#define OP_TYPE_DEFAULT 0
#define OP_TYPE_ENROLL  1
#define OP_TYPE_AUTH    2
#define OP_TYPE_CALI    3

#define MAX_FILE_NUM 60

#define TYPE_FILE 8
#define TYPE_DIR 4
#define SUFFIX_WORD "dat"
#define DUMP_FILE_NAME_LEN 38

using android::String8;

extern uint32_t gDumpLevelVal;

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

    gf_error_t HalUtils::manage_finger_dump_dir(char *path, char *filename) {
        int i = 0;
        DIR *dir = NULL;
        struct dirent *entry;
        int file_len;
        int suffix_len = strlen(SUFFIX_WORD);
        std::vector <std::string> vec;
        FUNC_ENTER();
        (void)filename;
        if (path == NULL || strlen(path) == 0) {
            LOG_E(LOG_TAG, "%s, parameter error", __func__);
            return GF_ERROR_BAD_PARAMS;
        }
        // 1. open directory
        if ((dir = opendir(path)) == NULL)
        {
            LOG_E(LOG_TAG, "%s, opendir failed!, %s", __func__, path);
            return GF_ERROR_BAD_PARAMS;
        }
        // 2. read file in directory
        while ((entry = readdir(dir))) {
            file_len = strlen(entry->d_name);
            // 2.1 if file is not stantard face file, ignore
            if (file_len < suffix_len) {
                // LOGE("%s, filename length smaller than dat :%s\n", __func__, entry->d_name);
                continue;
            }
            // 2.2 if file is not end with "yuv", ignore
            if (entry->d_name[file_len -3] != 'd' || entry->d_name[file_len - 2] != 'a' || entry->d_name[file_len -1] != 't') {
                LOG_E(LOG_TAG, "%s, filename = (%s) is not dat formate\n", __func__, entry->d_name);
                continue;
            }
            // 2.3 push file to vector
            std::string file_name = entry->d_name;
            vec.push_back(file_name);
        }
        // 3. close dir
        closedir(dir);
        // 4. caculate if need to remove file
        LOG_D(LOG_TAG, "%s, file numbers, %d\n", __func__, vec.size());
        if (vec.size() < MAX_FILE_NUM) {
            return GF_SUCCESS;
        }
        // 5. sort the file from old to new
        std::sort(vec.begin(), vec.end());

        // 6.file number over than MAX_FILE_NUM, remove the old file
        for (i = 0; i <= (int)vec.size() - MAX_FILE_NUM; i++) {
            (void)remove((path + vec[i]).c_str());
            LOG_E(LOG_TAG, "%s, remove file, %s", __func__, vec[i].c_str());
        }
        return GF_SUCCESS;
    }


#define MAX_IMAGE_SIZE (1 * 1024 * 1024)
    gf_error_t HalUtils::customizedEncrypt(uint8_t* src, uint32_t src_len,
                                           uint8_t **encrypt, uint32_t *encrypt_len,
                                           char *filename, uint32_t operation)
    {
        gf_error_t err = GF_SUCCESS;
        uint8_t *encodeBuf = NULL;
        FUNC_ENTER();
        do
        {
            if (nullptr == src || nullptr == encrypt || nullptr == encrypt_len || nullptr == filename) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            LOG_E(LOG_TAG, "Running Client 2nd enc.. Cur OP is %d", operation);

            LOG_E(LOG_TAG, "[%s] gDumpLevelVal = %d", __func__, gDumpLevelVal);

            if ( gDumpLevelVal == 0 ) {
            // 1.filter directory no need to encrypted
            // if file in gf_cali or filesize > 1.2m, no need to process
            if (strstr(filename, "gf_cali") || (src_len > MAX_IMAGE_SIZE)) {
                encodeBuf = new uint8_t[src_len] { 0 };
                memcpy(encodeBuf, src, src_len);
                *encrypt = encodeBuf;
                *encrypt_len = src_len;
                break;
            }
            // 2. manager directory
            int index;
            struct stat file_prop;
            char file_path[128] = {'\0'};
            char file_name[128] = {'\0'};

            stat(filename, &file_prop);
            if (file_prop.st_mode & S_IFDIR) { // directory
                break;
            }

            for (index = strlen(filename); index >= 0; index--) {
                if (filename[index] == '/') {
                    break;
                }
            }
            if (index < 0) {
                LOG_E(LOG_TAG, "%s, path is illegal", __func__);
                break;
            }
            // seperate dir name
            memcpy(file_path, filename, index + 1);
            LOG_D(LOG_TAG, "%s, file_path:%s", __func__, file_path);

            // seperate file name
            memcpy(file_name, filename + index + 1, strlen(filename) - index);
            LOG_D(LOG_TAG, "%s, file_name:%s", __func__, file_name);

            manage_finger_dump_dir((char *)file_path, (char *)file_name);
            // 3.encrtrypted data
            // make sure the origin data size , and encrypted data size
            uint32_t origin_size = src_len;
            uint32_t pad_size =  (origin_size % 16 == 0) ? origin_size : ((int)(origin_size/16) + 1) * 16;
            uint32_t key_cipher_size = 256;
            uint32_t encrypted_total_size = pad_size + key_cipher_size + sizeof(origin_size);
            encodeBuf = new uint8_t[encrypted_total_size] { 0 };
            // padding encrypted image data and key cipher
            HalContext::getInstance()->mFingerprintCore->sendDumpdataToTa((uint8_t*)src, pad_size, (uint8_t*)encodeBuf, &encrypted_total_size);
            // padding imagesize
            memcpy(encodeBuf + pad_size + key_cipher_size, &origin_size, sizeof(origin_size));
            LOG_D(LOG_TAG, "%s, total size: %ld, filesize: %ld", __func__, encrypted_total_size, origin_size);

            // 4. set buffer to gf and save
            *encrypt = encodeBuf;
            *encrypt_len = encrypted_total_size;
            }
            else if ( gDumpLevelVal == 1 ) {
            if (operation == OP_TYPE_ENROLL) {
                // do nothing default
                encodeBuf = new uint8_t[src_len] { 0 };
                memcpy(encodeBuf, src, src_len);
                *encrypt = encodeBuf;
                *encrypt_len = src_len;
            }
            else if (operation == OP_TYPE_AUTH) {
                // do nothing default
                encodeBuf = new uint8_t[src_len] { 0 };
                memcpy(encodeBuf, src, src_len);
                *encrypt = encodeBuf;
                *encrypt_len = src_len;
            }
            else if (operation == OP_TYPE_CALI) {
                // do nothing default
                encodeBuf = new uint8_t[src_len] { 0 };
                memcpy(encodeBuf, src, src_len);
                *encrypt = encodeBuf;
                *encrypt_len = src_len;
            }
            else if (operation == OP_TYPE_DEFAULT) {
                break;
            }
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
