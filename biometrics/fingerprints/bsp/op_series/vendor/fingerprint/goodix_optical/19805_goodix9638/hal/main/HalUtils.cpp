/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
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
#include <utils/SystemClock.h>
#include "HalUtils.h"
#include "HalLog.h"
#include "gf_algo_crc32.h"
#include "gf_base_types.h"

#define LOG_TAG "[HAL][HalUtils]"

using android::String8;

namespace goodix {
    int64_t HalUtils::getCurrentTimeMicrosecond() {
        struct timeval now = { 0 };
        gettimeofday(&now, 0);
        return now.tv_sec * 1000000L + now.tv_usec;
    }

    int64_t HalUtils::getrealtime() {
        int64_t current_time = android::elapsedRealtime();
        return current_time;
    }

    gf_error_t HalUtils::genTimestamp(char *timestamp, uint32_t len) {
        gf_error_t err = GF_SUCCESS;
        struct timeval tv = { 0 };
        struct tm current_tm = { 0 };
        char cur_time_str[GF_MAX_FILE_NAME_LEN] = { 0 };

        do {
            if (nullptr == timestamp || len < TIME_STAMP_LEN) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            gettimeofday(&tv, NULL);
            localtime_r(&tv.tv_sec, &current_tm);
            snprintf(cur_time_str, sizeof(cur_time_str),
                     "%04d-%02d-%02d-%02d-%02d-%02d-%06ld",
                     current_tm.tm_year + 1900, current_tm.tm_mon + 1, current_tm.tm_mday,
                     current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec, tv.tv_usec);

            if (len < strlen(cur_time_str)) {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] timestamp len too short.", __func__);
                break;
            }
        } while (0);

        if (GF_SUCCESS == err) {
            memcpy(timestamp, cur_time_str, strlen(cur_time_str) + 1);
        }

        return err;
    }

    gf_error_t HalUtils::genTimestamp2(char *timestamp, uint32_t len) {
        gf_error_t err = GF_SUCCESS;
        struct timeval tv = { 0 };
        struct tm current_tm = { 0 };
        char cur_time_str[GF_MAX_FILE_NAME_LEN] = { 0 };

        do {
            if (nullptr == timestamp || len < TIME_STAMP_LEN) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            gettimeofday(&tv, NULL);
            localtime_r(&tv.tv_sec, &current_tm);
            snprintf(cur_time_str, sizeof(cur_time_str), "%04d%02d%02d-%02d%02d%02d",
                     current_tm.tm_year + 1900, current_tm.tm_mon + 1, current_tm.tm_mday,
                     current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec);

            if (len < strlen(cur_time_str)) {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] timestamp len too short.", __func__);
                break;
            }
        } while (0);

        if (GF_SUCCESS == err) {
            memcpy(timestamp, cur_time_str, strlen(cur_time_str) + 1);
        }

        return err;
    }

    gf_error_t HalUtils::mkdirs(const char *dir) {
        gf_error_t err = GF_SUCCESS;
        int32_t len = 0;
        int32_t i = 0;
        char _dir[GF_MAX_FILE_NAME_LEN] = { 0 };

        do {
            if (nullptr == dir) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            snprintf(_dir, sizeof(_dir), "%s", dir);

            if (_dir[strlen(_dir) - 1] != '/') {
                snprintf(_dir, sizeof(_dir), "%s/", dir);
            }

            len = strlen(_dir);

            for (i = 1; i < len; i++) {
                if (_dir[i] == '/') {
                    _dir[i] = 0;

                    if (access(_dir, F_OK) != 0) {
                        if (mkdir(_dir, 0755) == -1) {
                            LOG_E(LOG_TAG, "[%s] mkdir failed, errno = %d", __func__, errno);
                            err = GF_ERROR_MKDIR_FAILED;
                            break;
                        }
                    }

                    _dir[i] = '/';
                }
            }
        } while (0);

        return err;
    }

    gf_error_t HalUtils::readFile(char *filepath, char *filename, uint8_t *read_buf,
                                  uint32_t read_buf_len) {
        gf_error_t err = GF_SUCCESS;
        FILE *file = NULL;
        int32_t file_len = 0;
        char path[GF_MAX_FILE_NAME_LEN] = {0};
        FUNC_ENTER();

        do {
            GF_NULL_BREAK(filepath, err);
            GF_NULL_BREAK(filename, err);
            GF_NULL_BREAK(read_buf, err);

            if (filepath[strlen(filepath) - 1] != '/') {
                snprintf(path, GF_MAX_FILE_NAME_LEN, "%s/%s", filepath, filename);
            } else {
                snprintf(path, GF_MAX_FILE_NAME_LEN, "%s%s", filepath, filename);
            }

            file = fopen(path, "rb");
            GF_NULL_BREAK(file, err);
            file_len = fread(read_buf, sizeof(uint8_t), read_buf_len, file);

            if (file_len <= 0) {
                LOG_E(LOG_TAG, "[%s] read file(%s) fail", __func__, path);
                err = GF_ERROR_FILE_READ_FAILED;
                break;
            }
        } while (0);

        if (NULL != file) {
            fclose(file);
            file = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalUtils::writeFile(char *filepath, char *filename,
                                   uint8_t *write_buf, uint32_t write_buf_len) {
        gf_error_t err = GF_SUCCESS;
        int32_t ret = 0;
        FILE *file = NULL;
        char dir[GF_MAX_FILE_NAME_LEN] = {0};
        char path[GF_MAX_FILE_NAME_LEN] = {0};
        FUNC_ENTER();

        do {
            GF_NULL_BREAK(filepath, err);
            GF_NULL_BREAK(filename, err);
            GF_NULL_BREAK(write_buf, err);

            if (filepath[strlen(filepath) - 1] != '/') {
                snprintf(dir, GF_MAX_FILE_NAME_LEN, "%s/", filepath);
            } else {
                snprintf(dir, GF_MAX_FILE_NAME_LEN, "%s", filepath);
            }

            if (access(dir, F_OK) != 0) {
                ret = mkdirs(dir);

                if (ret < 0) {
                    LOG_E(LOG_TAG, "[%s] make directory(%s) fail, ret=%d", __func__, dir, err);
                    err = GF_ERROR_MKDIR_FAILED;
                    break;
                }
            }

            snprintf(path, GF_MAX_FILE_NAME_LEN, "%s%s", dir, filename);
            LOG_D(LOG_TAG, "[%s] write file (%s)", __func__, path);
            file = fopen(path, "wb");
            GF_NULL_BREAK(file, err);

            if (write_buf_len != fwrite(write_buf, sizeof(uint8_t),
                                        write_buf_len, file)) {
                LOG_E(LOG_TAG, "[%s] write file(%s) fail", __func__, path);
                err = GF_ERROR_FILE_WRITE_FAILED;
                break;
            }
        } while (0);

        if (NULL != file) {
            fclose(file);
            file = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalUtils::makedirsByFilePath(const char *filepath) {
        gf_error_t err = GF_SUCCESS;

        do {
            FUNC_ENTER();

            if (nullptr == filepath) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            String8 str(filepath);
            String8 dir = str.getPathDir();
            err = HalUtils::mkdirs(dir.string());

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] mkdirs (%s) error", __func__, dir.string());
                break;
            }

            FUNC_EXIT(err);
        } while (0);

        return err;
    }

    gf_error_t HalUtils::genStringID(uint8_t *id, uint32_t len) {
        gf_error_t err = GF_SUCCESS;
        struct timeval now = { 0 };
        uint32_t val = 0;
        uint32_t i = 0;

        do {
            if (id == nullptr || len == 0) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            id[len - 1] = '\0';
            gettimeofday(&now, 0);
            srand((int32_t) now.tv_usec);
            for (i = 0; i < len - 1; i++) {
                if (val == 0) {
                    val = (uint32_t) rand();
                }
                uint32_t hexVal = val & 0x0F;
                if (hexVal < 10) {
                    id[i] = '0' + hexVal;
                } else {
                    id[i] = 'a' + (hexVal - 10);
                }
                val = val >> 4;
            }
        } while (0);

        return err;
    }

    gf_error_t HalUtils::saveDataToSdcard(const char* filename, uint8_t *data, uint32_t len) {
        gf_error_t err = GF_SUCCESS;
        uint32_t newLen = 0;
        uint8_t* newBuf = NULL;
        uint32_t calculateResult = 0;
        FILE* file = nullptr;
        uint32_t writelen = 0;
        char fullpath[GF_MAX_FILE_NAME_LEN] = { 0 };
        FUNC_ENTER();
        do {
            if (nullptr == filename || nullptr == data || len == 0) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            snprintf(fullpath, sizeof(fullpath), "%s%s", GF_SDCARD_PATH, filename);
            // data + crc
            newLen = len + sizeof(uint32_t);
            newBuf = new uint8_t[newLen];
            // calculate crc
            uint8_t crc32[sizeof(uint32_t)] = { 0 };
            gf_crc32_context_t ctx = { 0 };
            gf_algo_crc32_init(&ctx);
            gf_algo_crc32_update(&ctx, data, len);
            gf_algo_crc32_final(&ctx, crc32);
            memcpy(&calculateResult, crc32, sizeof(crc32));

            memcpy(newBuf, data, len);
            memcpy(newBuf + len, &calculateResult, sizeof(uint32_t));

            file = fopen(fullpath, "wb");
            if (NULL == file) {
                LOG_E(LOG_TAG, "[%s] save  file(%s) error.", __func__, fullpath);
                err = GF_ERROR_FILE_OPEN_FAILED;
                break;
            }
            LOG_D(LOG_TAG, "[%s] open file(%s) success", __func__, fullpath);
            writelen = fwrite(newBuf, sizeof(uint8_t), len + sizeof(uint32_t), file);
            if (writelen != (len + sizeof(uint32_t))) {
                LOG_E(LOG_TAG, "[%s] (%s)write INFO fail", __func__, fullpath);
                err = GF_ERROR_FILE_WRITE_FAILED;
                break;
            }
        } while (0);
        if (nullptr != file) {
            fclose(file);
        }
        if (newBuf != nullptr) {
            delete []newBuf;
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalUtils::loadDataFromSdcard(const char* filename, uint8_t *data, uint32_t len) {
        gf_error_t err = GF_SUCCESS;
        uint32_t fileLen = 0;
        uint8_t* fileBuf = NULL;
        uint32_t calculateCrc = 0;
        uint32_t readLen = 0;
        FILE* file = nullptr;
        char fullpath[GF_MAX_FILE_NAME_LEN] = { 0 };
        FUNC_ENTER();
        do {
            if (nullptr == filename || nullptr == data) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            snprintf(fullpath, sizeof(fullpath), "%s%s", GF_SDCARD_PATH, filename);
            file = fopen(fullpath, "rb");
            if (NULL == file) {
                LOG_E(LOG_TAG, "[%s] load  file(%s) is not exist", __func__, fullpath);
                err = GF_ERROR_FILE_NOT_EXIST;
                break;
            }

            fileLen = len + sizeof(uint32_t);
            fileBuf = new uint8_t[fileLen];
            readLen = fread(fileBuf, sizeof(uint8_t), fileLen, file);
            if (readLen != fileLen) {
                LOG_E(LOG_TAG, "[%s] read  file(%s) failue, read len = %u, actual len = %u", __func__,
                        fullpath, readLen, fileLen);
                err = GF_ERROR_FILE_READ_FAILED;
            }

            // crc
            uint8_t crc32[sizeof(uint32_t)] = { 0 };
            gf_crc32_context_t ctx = { 0 };
            gf_algo_crc32_init(&ctx);
            gf_algo_crc32_update(&ctx, fileBuf, len);
            gf_algo_crc32_final(&ctx, crc32);
            memcpy(&calculateCrc, crc32, sizeof(crc32));
            uint32_t backupCrc = *(uint32_t*)(fileBuf + len);
            if (backupCrc != calculateCrc) {
                LOG_E(LOG_TAG,
                        "[%s] CRC8 check fail, backupCrc8=0x%04X, calcualteCrc8=0x%04X",
                        __func__, backupCrc, calculateCrc);
                err = GF_ERROR_HAL_GENERAL_ERROR;
                break;
            }
            memcpy(data, fileBuf, len);
        } while (0);
        if (nullptr != file) {
            fclose(file);
        }
        if (fileBuf != nullptr) {
            delete []fileBuf;
        }
        FUNC_EXIT(err);
        return err;
    }

    bool HalUtils::isBlackImage(uint8_t *img, int32_t ImgSize) {
        for (int i = 0; i < ImgSize; i++) {
            if (img[i] > 0) {
                return false;
            }
        }
        return true;
    }

}  // namespace goodix
