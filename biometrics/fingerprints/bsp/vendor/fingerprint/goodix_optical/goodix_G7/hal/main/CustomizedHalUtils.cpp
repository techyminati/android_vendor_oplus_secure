/*
 * Copyright (C) 2013-2021, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CustomizedHalUtils.h"
#include "HalLog.h"
#include "HalUtils.h"
#include "CustomizedHalConfig.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define LOG_TAG "[GF_HAL][CustomizedHalUtils]"

#define PATH_LEN 256

namespace goodix {
    typedef enum {
        OP_TYPE_ENROLL,
        OP_TYPE_AUTH,
        OP_TYPE_CALI,
        OP_TYPE_PRODUCT,
    } GF_DUMP_OP_TYPE;

    int filter(const struct dirent *namelist) {
        return strcmp(namelist->d_name, ".") == 0 || strcmp(namelist->d_name, "..") == 0 ? 0 : 1;
    }

    gf_error_t manage_finger_dump_dir(char *path, int number) {
        int i = 0;
        DIR *dir = NULL;
        int count = 0;
        char abspath[PATH_LEN] = {'\0'};
        FUNC_ENTER();
        TIME_START(dump_sort_time);
        if (path == NULL || strlen(path) == 0) {
            LOG_E(LOG_TAG, "%s, parameter error", __func__);
            return GF_ERROR_BAD_PARAMS;
        }
        struct dirent **entry_list = NULL;
        count = scandir(path, &entry_list, filter, alphasort);
        if (count <= 0) {
            goto fp_out;
        }

        LOG_D(LOG_TAG, "%s file numbers, %d\n", path, count);
        if (count <= number) {
            goto fp_out;
        }

        for (i = 0; i < count - number; i++) {
            snprintf(abspath, PATH_LEN - 1, "%s/%s", path, entry_list[i]->d_name);
            remove((char *)abspath);
            LOG_D(LOG_TAG, "%s, remove file, %s", __func__, abspath);
            memset(abspath, '\0', sizeof(abspath));
            free(entry_list[i]);
        }
        fp_out:
        if (entry_list) {
            free(entry_list);
        }
        TIME_END(dump_sort_time, 0);
        return GF_SUCCESS;
    }
    /*check and remove the eraliest filt in 'auth' dir
     *
     *test performance:
     *   500 files, check cost 1ms
     *   500 files, remove cost 1-2ms
     */
    #define CALI_PATH "gf_data/encrypted_data/cali"
    #define AUTH_PATH "gf_data/encrypted_data/auth"
    #define AUTH_BMP_PATH "gf_data/auth/bmp"
    #define ENROLL_PATH "gf_data/encrypted_data/enroll"
    #define TEST_PATH "gf_data/encrypted_data/test"
    #define ENROLL_BMP_PATH "gf_data/enroll/bmp"
    #define WARNING_TOTAL_COUNT (450)
    void CustomizedHalUtils::checkDumpFileLimit() {
        int totalCount = 0;
        unsigned int totalSize = 0;
        int limitCount = 0;
        unsigned int limitSize = 0;
        int overlayFlag = 0;
        char earliestFile[256] = {0};
        char* basepath = (char*)getDumpRootDir();
        long long t0 = HalUtils::getTimeUs();
        long long cost = 0;
        char path[PATH_LEN] = {'\0'};
        if (HalUtils::getAndroidVersionType() == FP_PREVERSION) {
            LOG_I(LOG_TAG, "FP_PREVERSION user");
            limitCount = 500 - 5;
            /* 500M 512*1024*1024 - 5M */
            limitSize = 524288000 - 5242880;
            // remove enroll
            snprintf(path, PATH_LEN - 1, "%s/%s", basepath, ENROLL_PATH);
            manage_finger_dump_dir(path, 0);
            // manage calibration
            memset(path, '\0', PATH_LEN);
            snprintf(path, PATH_LEN - 1, "%s/%s", basepath, CALI_PATH);
            manage_finger_dump_dir(path, 12);
            // manage auth
            memset(path, '\0', PATH_LEN);
            snprintf(path, PATH_LEN - 1, "%s/%s", basepath, AUTH_PATH);
            manage_finger_dump_dir(path, WARNING_TOTAL_COUNT);
            // remove enroll bmp
            snprintf(path, PATH_LEN - 1, "%s/%s", basepath, ENROLL_BMP_PATH);
            manage_finger_dump_dir(path, 0);
            // remove auth bmp
            snprintf(path, PATH_LEN - 1, "%s/%s", basepath, AUTH_BMP_PATH);
            manage_finger_dump_dir(path, 0);
        } else {
            limitCount = 2000;
            /* 2G 1024*1024*1024 */
            limitSize = 2*1073741824;
            LOG_I(LOG_TAG, "not FP_PREVERSION user and not check");
            return;
        }

        HalUtils::getDirFileInfo(basepath, &totalCount, &totalSize, earliestFile, (char*)"auth");
        LOG_I(LOG_TAG, "custdump totalCount:%d (%d) totalSize:%u (%u)",
            totalCount, limitCount, totalSize, limitSize);

        LOG_D(LOG_TAG, "custdump the earliestFile:%s", earliestFile);

        /* 1st limit: dir file total size > mMaxDirFileSize */
        if (totalSize > limitSize) {
            LOG_I(LOG_TAG, "custdump file size exceed MAX size:%d", limitSize);
            overlayFlag = 1;
        }

        /* 2nd limit: dir file total count > mMaxDirFileCount */
        if (totalCount > limitCount) {
            LOG_I(LOG_TAG, "custdump file count exceed MAX count:%d", limitCount);
            overlayFlag = 1;
        }

        if (overlayFlag) {
            LOG_I(LOG_TAG, "custdump remove the earliestFile:%s", earliestFile);
            HalUtils::removePath(earliestFile);
        } else {
            LOG_D(LOG_TAG, "custdump check dump dir limit OK");
        }
        cost = (HalUtils::getTimeUs() - t0) / 1000;

        LOG_D(LOG_TAG, "custdump checkDumpFileLimit cost:%lld ms", cost);
    }

    gf_error_t CustomizedHalUtils::customizedEncrypt(uint8_t* src, uint32_t srcLen,
                            uint8_t **encrypt, uint32_t *encryptLen,
                            char *filename, uint32_t operation) {
        gf_error_t err = GF_SUCCESS;
        uint8_t *encodeBuf = NULL;

        /* check the version type */
        LOG_I(LOG_TAG, "custdump version type:%d filename:%s operation:%u",
            HalUtils::getAndroidVersionType(), filename, operation);

        FUNC_ENTER();
        do {
            if (nullptr == src || nullptr == encrypt || nullptr == encryptLen || nullptr == filename) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            checkDumpFileLimit();

            if (operation == OP_TYPE_ENROLL) {
                // do nothing default
                encodeBuf = new uint8_t[srcLen] { 0 };
                memcpy(encodeBuf, src, srcLen);
                *encrypt = encodeBuf;
                *encryptLen = srcLen;
            } else if (operation == OP_TYPE_AUTH) {
                // do nothing default
                encodeBuf = new uint8_t[srcLen] { 0 };
                memcpy(encodeBuf, src, srcLen);
                *encrypt = encodeBuf;
                *encryptLen = srcLen;
            } else if (operation == OP_TYPE_CALI) {
                // do nothing default
                encodeBuf = new uint8_t[srcLen] { 0 };
                memcpy(encodeBuf, src, srcLen);
                *encrypt = encodeBuf;
                *encryptLen = srcLen;
            } else {
                // do nothing default
                encodeBuf = new uint8_t[srcLen] { 0 };
                memcpy(encodeBuf, src, srcLen);
                *encrypt = encodeBuf;
                *encryptLen = srcLen;
                break;
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
