/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HALUTILS_H_
#define _HALUTILS_H_

#include "gf_error.h"

#define TIME_STAMP_LEN (128)
#define TIME_START(func) \
    long long start##func = HalUtils::getTimeUs()

#define TIME_END(func, cmd) \
    long long diff##func = (HalUtils::getTimeUs() - start##func) / 1000; \
    LOG_I(LOG_TAG,"cmd :%d, %s take time %llu ms", cmd, #func, diff##func); \
    diff##func

enum AndroidVersionType {
    FP_USERDEBG = 0,
    FP_PREVERSION = 1,
    FP_RELEASE = 2,
    FP_FACTORY = 3,
};

namespace goodix {
    class HalUtils {
    public:
        static int64_t getCurrentTimeMicrosecond();
        static int64_t getrealtime();
        static gf_error_t genTimestamp(char *timestamp, uint32_t len);
        static gf_error_t genTimestamp2(char *timestamp, uint32_t len);
        static gf_error_t mkdirs(const char *dir);
        static gf_error_t makedirsByFilePath(const char *filepath);
        static gf_error_t readFile(char *filepath, char *filename, uint8_t *read_buf,
                                   uint32_t read_buf_len);
        static gf_error_t writeFile(char *filepath, char *filename, uint8_t *write_buf,
                                    uint32_t write_buf_len);
        static gf_error_t genStringID(uint8_t *id, uint32_t len);
        static gf_error_t saveDataToSdcard(const char *filename, uint8_t *data, uint32_t len);
        static gf_error_t loadDataFromSdcard(const char *filename, uint8_t *data, uint32_t len);
        static bool isBlackImage(uint8_t *img, int32_t ImgSize);
        static long long getTimeUs();
        static AndroidVersionType getAndroidVersionType();
        static long long getFileSize(const char* path);
        static int getDirFileInfoInner(const char *path, int* totalCount,
            unsigned int* totalSize, char* earliestFile, char* fileFilter, long* mtime);
        static int getDirFileInfo(const char *path, int* totalCount,
            unsigned int* totalSize, char* earliestFile, char* fileFilter);
        static int removePath(const char* path);
    };
}  // namespace goodix



#endif /* _HALUTILS_H_ */
