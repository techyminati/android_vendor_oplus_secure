/*************************************************************
 ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
 ** OPLUS_FEATURE_SECURITY_COMMON
 ** File        : rpmbengclient.cpp
 ** Description : NULL
 ** Date        : 2018-12-04 14:39
 ** Author      : LONG.liu
 **
 ** ------------------ Revision History: ---------------------
 **      <author>        <date>          <desc>
 **      Long.Liu     2018/12/04      rpmbenable_client base on trustonic
 **      Bin.Li       2019/01/01      add for alikey write key rely on secure on.
 **      oujinrong    2019/12/09      add for reading secureType from oplus_secure_common
 **      Bin.Li       2020/06/30      add for reading rpmbeng_get_state for mtk
 **      Dongnan.Wu   2020/08/07      modify proc inode name
 **      zhoumeilin   2021/11/15      modify for rpmb key provision
 *************************************************************/
#define LOG_TAG "RPMBENG_CLIENT"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <android/log.h>
#include <utils/Log.h>
#include <sys/mman.h>
#include <getopt.h>
#include <rpmbengclient.h>

#define PROC_RPMB_ENABLE_FILE "/proc/oplusCustom/rpmb_enable"
#define PROC_MTK_RPMB_KEY_PATH "/proc/oplusCustom/rpmb_key_provisioned"
static int get_proc_file(const char *path, char *buf, uint32_t buf_size) {
    int fd = -1;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        ALOGE("Can't open %s\n", path);
        goto EXIT;
    }
    if (read(fd, buf, buf_size) == -1) {
        ALOGE("Can't read %s\n", path);
        goto EXIT;
    }
    close(fd);
    return 0;
EXIT:
    if (fd != -1) {
        close(fd);
    }
    return -1;
}

int32_t rpmbeng_get_state()
{
    char rpmb_key_status[8] = {'0'};
#ifdef CONFIG_ENABLE_RPMB_RELY_OPLUSRESERVE
    if (get_rpmb_key_provisioned_flag() == STATUS_RPMB_KEY_PROVISIONED) {
        ALOGI("RMPB Key status: RPMB_KEY_PROVISIONED_AND_OK.\n");
    } else if (get_rpmb_key_provisioned_flag() == STATUS_RPMB_KEY_NOT_PROVISIONED) {
        ALOGI("RMPB Key status: RPMB_KEY_NOT_PROVISIONED\n");
        return -1;
    } else {
        ALOGE("rpmb_key_status can not get\n");
        return -1;
    }
#else
    if (get_proc_file(PROC_MTK_RPMB_KEY_PATH, rpmb_key_status, 8)) {
        ALOGE("rpmb_key_status path not found\n");
        return -1;
    }
    if (strncmp("1", rpmb_key_status, 8)) {
        ALOGI("RMPB Key status: RPMB_KEY_NOT_PROVISIONED (%s)\n", rpmb_key_status);
        return -1;
    }
    ALOGI("RMPB Key status: RPMB_KEY_PROVISIONED_AND_OK.\n");
#endif
    return 0;
}

int32_t rpmbeng_get_enable_state()
{
    int32_t res = -1;
    int enable_rpmb_fd = -1;
    int enable_rpmb_status = -1;

    enable_rpmb_fd = open(PROC_RPMB_ENABLE_FILE, O_WRONLY);
    if (enable_rpmb_fd < 0) {
        ALOGE("rpmb_enable open error %d\n", enable_rpmb_fd);
        goto out;
    }

    read(enable_rpmb_fd, &enable_rpmb_status, sizeof(enable_rpmb_status));
    close(enable_rpmb_fd);

    if (enable_rpmb_status == 1) {
        res = 1;
        ALOGI("RPMB key provisioning status enabled.\n");
    } else {
        res = 0;
        ALOGI("RPMB key provisioning status disabled.\n");
    }

out:
    return res;

}

#ifdef CONFIG_WRITE_KEY_RELY_FUSE
#define SECURE_TYPE_PATH "/proc/oplus_secure_common/secureType"
static int get_secure_type(const char *path, char *buf, uint32_t buf_size) {
    int fd = -1;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        ALOGE("Can't open %s\n", path);
        goto EXIT;
    }
    if (read(fd, buf, buf_size) == -1) {
        ALOGE("Can't read %s\n", path);
        goto EXIT;
    }
    close(fd);
    return 0;
EXIT:
    if (fd != -1) {
        close(fd);
    }
    return -1;
}
#endif

int32_t rpmbeng_enable_rpmb()
{
    int32_t res = -1;
    int enable_rpmb_fd  = -1;
#ifdef CONFIG_WRITE_KEY_RELY_FUSE
    char fuse_buf[8] = {'0'};
    if (get_proc_file(SECURE_TYPE_PATH, fuse_buf, 8)) {
        ALOGE("securetype path not found\n");
        return -1;
    }
    if (strncmp("1", fuse_buf, 8)) {
        ALOGE("secure is close,Write key not allowed\n");
        return -1;
    }
#endif
#ifdef CONFIG_ENABLE_RPMB_RELY_OPLUSRESERVE
    if (set_enable_rpmb_flag()) {
        ALOGD("rpmb_enable success\n");
        res = 0;
    } else
        ALOGD("rpmb_enable fail\n");
    return res;
#else
    enable_rpmb_fd = open(PROC_RPMB_ENABLE_FILE, O_WRONLY);
    if (enable_rpmb_fd < 0) {
        ALOGE("rpmb_enable open error %d\n", enable_rpmb_fd);
    } else {
        write(enable_rpmb_fd, "1", 1);
        close(enable_rpmb_fd);
        res = 0;
        ALOGD("rpmb_enable success\n");
    }
    return res;
#endif
}
