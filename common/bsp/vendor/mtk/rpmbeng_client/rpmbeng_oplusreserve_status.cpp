/*************************************************************
 ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
 ** OPLUS_FEATURE_SECURITY_COMMON
 ** File        : rpmbeng_oplusreserve_status.cpp
 ** Description : read and write rpmb enable flag, rpmb provisioned flag from oplusreserve1 partition
 ** Date        : 2021-11-15 19:47
 ** Author      : Meilin.Zhou
 **
 ** ------------------ Revision History: ---------------------
 **      <author>        <date>          <desc>
 **      zhoumeilin   2021/11/15      modify for rpmb enable and rpmb key provision flag
 *************************************************************/
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <utils/Log.h>
#include <rpmbengclient.h>

bool is_ufs_storage(void) {
    if (access(UFS_DEVICE_BLOCK_PATH, F_OK) == 0) {
        return true;
    } else {
        ALOGE("access %s failed! err:%s", UFS_DEVICE_BLOCK_PATH, strerror(errno));
        return false;
    }
}

int readInterface(const char *path, int offset, void *buf, int len)
{
    int fd = -1;
    int ret = -1;
    int size = -1;

    fd = open(path, O_RDWR | O_NONBLOCK );
    if (fd == -1) {
        ALOGD("open %s failed! err:%s", path, strerror(errno));
        return ret;
    }

    size = lseek(fd, offset, SEEK_SET);
    if (size == -1){
        ALOGD("read failed! size:%d", size);
        goto end;
    }
    size = read(fd, buf, len);
    if (size == -1){
        ALOGD("read no content! size:%d", size);
        goto end;
    }
    ret = size;

end:
    if (fd != -1)
        close(fd);

    return ret;
}

int writeInterface(const char *path, int offset, const char *buf, int len)
{
    int fd = -1;
    int ret = -1;
    int size = -1;

    fd = open(path, O_RDWR | O_NONBLOCK );
    if (fd == -1) {
        ALOGD("open %s failed! err:%s", path, strerror(errno));
        return ret;
    }
    size = lseek(fd, offset, SEEK_SET);
    if (size == -1){
        ALOGD("lseek failed! size:%d", size);
        goto end;
    }

    size = write(fd, buf, len);
    if (size == -1){
        ALOGD("write failed! size:%d", size);
        goto end;
    }
    ALOGD("write %d bytes", size);

    ret = size;
    fsync(fd);
end:
    if (fd != -1)
        close(fd);

    return ret;
}

bool set_enable_rpmb_flag(void) {
    int ret = 0;
    Rpmb_flag_Config  mConfigInf = {0};
    if (!is_ufs_storage()) {
        ret = readInterface(OPLUS_RESERVE1_PATH, OPLUS_RESERVE1_EMMC_ENABLE_RPMB_FLAG_OFFSET, (char *)&mConfigInf, sizeof(mConfigInf));
        memset(mConfigInf.rpmb_enable, 0, OPLUSRESERVE1_RPMB_FLAG_LENGTH);
        memcpy(mConfigInf.rpmb_enable, RPMB_ENABLE_MAGIC_STRING, strlen(RPMB_ENABLE_MAGIC_STRING));
        ret = writeInterface(OPLUS_RESERVE1_PATH, OPLUS_RESERVE1_EMMC_ENABLE_RPMB_FLAG_OFFSET, (char *)&mConfigInf, sizeof(mConfigInf));
    } else {
        ret = readInterface(OPLUS_RESERVE1_PATH, OPLUS_RESERVE1_UFS_ENABLE_RPMB_FLAG_OFFSET, (char *)&mConfigInf, sizeof(mConfigInf));
        memset(mConfigInf.rpmb_enable, 0, OPLUSRESERVE1_RPMB_FLAG_LENGTH);
        memcpy(mConfigInf.rpmb_enable, RPMB_ENABLE_MAGIC_STRING, strlen(RPMB_ENABLE_MAGIC_STRING));
        ret = writeInterface(OPLUS_RESERVE1_PATH, OPLUS_RESERVE1_UFS_ENABLE_RPMB_FLAG_OFFSET, (char *)&mConfigInf, sizeof(mConfigInf));
    }
    ALOGD("set_enable_rpmb_flag ret=%d\n", ret);
    return ret ? true : false;
}

enum status_rpmb_key_provisioned get_rpmb_key_provisioned_flag(void) {
    int ret = 0;
    Rpmb_flag_Config  mConfigInf = {0};

    if (!is_ufs_storage())
        ret = readInterface(OPLUS_RESERVE1_PATH, OPLUS_RESERVE1_EMMC_ENABLE_RPMB_FLAG_OFFSET, (char *)&mConfigInf, sizeof(mConfigInf));
    else
        ret = readInterface(OPLUS_RESERVE1_PATH, OPLUS_RESERVE1_UFS_ENABLE_RPMB_FLAG_OFFSET, (char *)&mConfigInf, sizeof(mConfigInf));

    ALOGD("get_rpmb_key_provisioned_flag ret=%d\n", ret);
    if(ret) {
        if (0 == memcmp(mConfigInf.rpmb_keyprovision, RPMB_KEY_PROVISIONED_STRING, strlen(RPMB_KEY_PROVISIONED_STRING))) {
            return STATUS_RPMB_KEY_PROVISIONED;
        } else if(0 == memcmp(mConfigInf.rpmb_keyprovision, RPMB_KEY_NOT_PROVISIONED_STRING, strlen(RPMB_KEY_NOT_PROVISIONED_STRING))) {
            return STATUS_RPMB_KEY_NOT_PROVISIONED;
        } else {
            return STATUS_RPMB_KEY_PROVISIONED_ERROR;
        }
    } else {
        return STATUS_RPMB_KEY_PROVISIONED_ERROR;
    }
}
