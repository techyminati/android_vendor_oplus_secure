/*******************************************************************************************
 * Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
 *
 * fp: Utils.cpp
 * Description: utils
 * Version: 1.0
 * Date : 2021-7-12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
*********************************************************************************************/
#include "Utils.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <fstream>
#include "HalLog.h"
#include <fcntl.h>
#include "FpType.h"

#define LOG_TAG "[HAL][Utils]"

int Utils::makePath(const char *path) {
    return makePath(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

int Utils::makePath(const char *path, mode_t mode) {
    char *dir, *delim;
    int rc = -1;

    delim = dir = strdup(path);
    if (dir == NULL) {
        fprintf(stderr, "strdup failed copying path");
        return -1;
    }

    /* skip '/' -- it had better exist */
    if (*delim == '/') {
        delim++;
    }

    while (1) {
        delim = strchr(delim, '/');
        if (delim) {
            *delim = '\0';
        }

        rc = mkdir(dir, mode);
        if (rc != 0 && errno != EEXIST) {
            LOG_E(LOG_TAG, "mkdir failed for %s: %s", dir, strerror(errno));
            goto fp_out;
        }

        if (delim == NULL) {
            break;
        }

        *delim = '/';
        delim++;
        if (*delim == '\0') {
            break;
        }
    }
    rc = 0;
fp_out:
    free(dir);
    return rc;
}

int Utils::readData(char *fileName, void *buf, unsigned int nbyte) {
    int o_size = -1;
    int fd = open(fileName, O_RDONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "open failed");
        return -1;
    }
    o_size = read(fd, buf, nbyte);
    close(fd);
    return o_size;
}


int Utils::writeData(const char* fileName, const char* data, int dataLen,
    int appendFlag, int newLineFlag) {
    int err = 0;
    FUNC_ENTER();
    char writeBuf[MAX_FILE_PATH_LEN+1] = {0};
    FILE *fp = NULL;
    size_t writeLen;
    char mode[8] = {0};
    LOG_D(LOG_TAG, "fileName:%s data:%s dataLen:%d", fileName, data, dataLen);

    if (fileName == NULL || data == NULL || dataLen <= 0) {
        LOG_E(LOG_TAG, "input error, fileName:%s data:%s dataLen:%d", fileName, data, dataLen);
        err = -1;
        goto fp_out;
    }

    if (appendFlag == 1) {
        sprintf(mode, "%s", "a");
    } else {
        sprintf(mode, "%s", "w");
    }

    if ((fp = fopen(fileName, mode)) == NULL) {
        LOG_E(LOG_TAG, "fail to open fp:%s (%d:%s)", fileName, errno, strerror(errno));
        err = -1;
        goto fp_out;
    }

    LOG_D(LOG_TAG, "open OK and write data");
    if (newLineFlag == 1) {
        strcat(writeBuf, data);
        sprintf(writeBuf + MAX_FILE_PATH_LEN - 1, "%s", "\n");
        writeLen = fwrite(writeBuf, sizeof(char), dataLen, fp);
    } else {
        writeLen = fwrite(data, sizeof(char), dataLen, fp);
    }
    if ((int)writeLen != dataLen) {
        LOG_E(LOG_TAG, "fwrite fail, writeLen:%zu (%d:%s)", writeLen, errno, strerror(errno));
        err = -2;
        goto fp_out;
    }

    /* make sure write the data into hard disk */
    fflush(fp);

    if (fp != NULL) {
        fclose(fp);
    }
fp_out:
    return err;
}

int Utils::getTimestamp(char* timestampBuf) {
    int err = 0;
    struct timeval tv;
    struct tm current_tm;
    memset(&tv, 0, sizeof(timeval));
    memset(&current_tm, 0, sizeof(tm));

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &current_tm);
    snprintf(timestampBuf, MAX_TIMESTAMP_LEN-1, "%04d%02d%02d_%02d%02d%02d",
        current_tm.tm_year + 1900, current_tm.tm_mon + 1, current_tm.tm_mday,
        current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec);

    return err;
}

long long Utils::getTimeUs() {
    struct timeval now;
    memset(&now, 0, sizeof(timeval));
    gettimeofday(&now, 0);
    return now.tv_sec * 1000000L + now.tv_usec;
}

int Utils::removePath(const char* path) {
    if (path == 0) {
        LOG_E(LOG_TAG, "path is NULL");
        return -1;
    }

    if (unlink(path) != 0) {
        LOG_E(LOG_TAG, "unlink failed for %s: %s", path, strerror(errno));
        return -1;
    }

    return 0;
}

int Utils::isFileExist(const char* path) {
    return ((access(path, F_OK)) != -1) ? FILE_DO_FOUND : FILE_NOT_FOUND;
}


long long Utils::getFileSize(const char* path) {
    LOG_D(LOG_TAG, "path:%s", path);
    if (path == 0) {
        LOG_D(LOG_TAG, "path is NULL");
        return -1;
    }

    struct stat st;
    stat(path, &st);

    return st.st_size;
}

int Utils::utilsRename(char *oldname, char *newname) {
    int ret = 0;
    ret = rename(oldname, newname);
    return ret;
}
