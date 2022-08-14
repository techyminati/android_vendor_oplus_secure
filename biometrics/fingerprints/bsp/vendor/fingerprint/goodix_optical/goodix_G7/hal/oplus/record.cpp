/************************************************************************************
 ** File: - record.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2021-2025, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      record data
 **
 ** Version: 1.0
 ** Date created: 11:00,11/19/2021
 ** Author: Zhi.Wang@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Zhi.Wang
 ************************************************************************************/
#define LOG_TAG "[GF_HAL][oplusrecord]"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "HalLog.h"
#include "HalUtils.h"
#include "CustomizedHalConfig.h"
#include "record.h"

namespace goodix {

Record::Record()
{
    /* 2M */
    mMaxRecordFileSize = 2*1024*1024;
    LOG_I(LOG_TAG, "mMaxRecordFileSize:%d Byte", mMaxRecordFileSize);

    snprintf(mRecRootPath, PATH_LEN, "%s/gf_data/record", getDumpRootDir());
    LOG_I(LOG_TAG, "mRecRootPath:%s", mRecRootPath);
}

Record::~Record()
{
}

/**
 * k=v,k1=v1, ->
 * key[] = {k, k1}
 * val[] = {v, v1}
*/
int Record::parseKV(char* data, char* key, char* val) {
    char* p = data;
    char* q = p;
    while (*p != '\0') {
        if (*p == ':') {
            *p = '\0';

            /* remove the blank ' ' */
            while (*q == ' ') {
                q++;
            }
            strcat(key, q);
            strcat(key, ",");
            q = p + 1;
        }

        if (*p == ',') {
            *p = '\0';

            /* remove the blank ' ' */
            while (*q == ' ') {
                q++;
            }

            strcat(val, q);
            strcat(val, ",");
            q = p + 1;
        }
        p++;
    }

    if (strlen(key) <= 0 || strlen(val) <= 0) {
        LOG_E(LOG_TAG, "key and value error, check: k=v,k1=v1");
        return -1;
    }

    strcat(key, "\n");
    strcat(val, "\n");

    return 0;
}

int Record::setFilename(int record_type, char* filename) {
    switch (record_type) {
        case RECORD_INIT:
            strcat(filename, "fp_init_data.csv");
        break;
        case RECORD_ENROLL:
            strcat(filename, "fp_enroll_data.csv");
        break;
        case RECORD_ENROLL_FINISH:
            strcat(filename, "fp_enroll_finish_data.csv");
        break;
        case RECORD_AUTH:
            strcat(filename, "fp_auth_data.csv");
        break;
        default:
            LOG_E(LOG_TAG, "not support the type:%d", record_type);
            return -1;
        break;
    }

    return 0;
}

int Record::saveCsvFile(char* path, char* key, char* val) {
    int header_flag = 1;
    int err = 0;
    FILE *fp = NULL;

    /* check root path */
    if (access(mRecRootPath, F_OK) != 0) {
        err = (int)HalUtils::mkdirs(mRecRootPath);
        LOG_I(LOG_TAG, "make directory(%s), ret=%d",  mRecRootPath, err);
    }

    /* judge the file exist, and set the header_flag */
    if (access(path, F_OK) == 0) {
        header_flag = 0;
        if ((int)HalUtils::getFileSize(path) > mMaxRecordFileSize) {
            LOG_E(LOG_TAG, "record file reach the max size:%d", mMaxRecordFileSize);
            return -1;
        }
    }

    fp = fopen(path, "a+");
    if (!fp) {
        LOG_E(LOG_TAG, "open file:%s failed", path);
        return -1;
    }

    /* write the header:key */
    if (header_flag) {
        fwrite(key, sizeof(char), strlen(key), fp);
    }
    /* write the body:val */
    fwrite(val, sizeof(char), strlen(val), fp);

    fflush(fp);
    fclose(fp);
    return 0;
}

void Record::recordCsvData(char* data, int record_type) {
    LOG_D(LOG_TAG, "enter record_type:%d", record_type);
    //LOG_D(LOG_TAG, "data:%s", data);

    /* check the dump support */
    if (HalUtils::getAndroidVersionType() == FP_RELEASE) {
        LOG_I(LOG_TAG, "FP_RELEASE user and return");
        return;
    }

    /* 1st check the type */
    if (record_type < RECORD_INIT || record_type >= RECORD_NONE) {
        LOG_E(LOG_TAG, "error record_type");
        return;
    }

    int len = (int)strlen(data);
    if (len <= 0) {
        LOG_E(LOG_TAG, "error data len <= 0");
        return;
    }

    char* tmp_data = (char*)malloc(len+2);
    char* key = (char*)malloc(len);
    char* val = (char*)malloc(len);
    char filename[32] = {0};
    char path[PATH_LEN] = {0};

    if (!tmp_data || !key || !val) {
        LOG_E(LOG_TAG, "malloc failed:%p %p %p", tmp_data, key, val);
        goto fp_out;
    }

    /* pre-deal the string
     * k=v,k1=v1 -> k=v,k1=v1,
     */
    memset(key, 0, len);
    memset(val, 0, len);
    memset(tmp_data, 0, len+2);
    memcpy(tmp_data, data, len);
    if (tmp_data[len] != ',') {
        tmp_data[len] = ',';
    }

    /* 2nd parse the data and check */
    if (parseKV(tmp_data, key, val) != 0) {
        goto fp_out;
    }

    /* 3rd decide the file name */
    if (setFilename(record_type, filename) != 0) {
        goto fp_out;
    }

    /* 4th save the data */
    snprintf(path, PATH_LEN, "%s/%s", mRecRootPath, filename);
    if (saveCsvFile(path, key, val) != 0) {
        goto fp_out;
    }

fp_out:
    /* free the buffer */
    if (tmp_data) {
        free(tmp_data);
    }

    if (key) {
        free(key);
    }

    if (val) {
        free(val);
    }
}

}   // namespace goodix
