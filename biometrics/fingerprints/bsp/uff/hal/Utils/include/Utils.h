/*******************************************************************************************
 * Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
 *
 * File: Utils.h
 * Description: Utils.h
 * Version: 1.0
 * Date : 2021-7-12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
*********************************************************************************************/
#ifndef HAL_UTILS_H
#define HAL_UTILS_H

#include <sys/types.h>
#include <sys/stat.h>

#define MAX_TIMESTAMP_LEN 32
#define MAX_FILE_PATH_LEN 256
#define MAX_FILE_COUNT    60
#define MAX_CA_TA_SIZE (1 * 1024 * 1024) // 1M
#define FILE_DO_FOUND (1)
#define FILE_NOT_FOUND (0)

#define TIME_START(func) \
    long long start##func = Utils::getTimeUs()

#define TIME_END(func) \
    long long diff##func = (Utils::getTimeUs() - start##func) / 1000; \
    LOG_I(LOG_TAG, "%s take time %llu ms", #func, diff##func); \
    diff##func


class Utils {
public:
    static int makePath(const char *path);
    static int makePath(const char *path, mode_t mode);
    static int writeData(const char* fileName, const char* data, int dataLen,
        int appendFlag = 0, int newLineFlag = 0);
    static int readData(char *fileName, void *buf, unsigned int nbyte);
    static int getTimestamp(char* timestampBuf);
    static long long getTimeUs();
    static int removePath(const char* path);
    static int isFileExist(const char* path);
    static long long getFileSize(const char* path);
    static int utilsRename(char *oldname, char *newname);
};
#endif //HAL_UTILS_H

