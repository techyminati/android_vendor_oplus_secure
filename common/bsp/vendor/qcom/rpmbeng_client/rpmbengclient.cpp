/*************************************************************
 ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
 ** OPLUS_FEATURE_SECURITY_COMMON
 ** File        : rpmbengclient.cpp
 ** Description : NULL
 ** Date        : 2015-08-12 14:39
 ** Author      : Lycan.Wang
 **
 ** ------------------ Revision History: ---------------------
 **      <author>        <date>          <desc>
 **      Haitao.Zhou     2016/07/12      rpmbenable_client base on QSEE 4.0
 **      Dongnan.Wu      2018/11/20      change the rpmb status inode path
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
#include <linux/msm_ion.h>
#include <android/log.h>
#include <utils/Log.h>
#include "QSEEComAPI.h"
#include "common_log.h"
#include <sys/mman.h>
#include <getopt.h>
//#include <jni.h>

#define _PERSIST_RPMB_STATUS_PATH "/mnt/vendor/persist/rpmb_status"
#define _PERSIST_RPMB_STATUS_NAME "/rpmb_status"
#define _PERSIST_RPMB_STATUS_FILE _PERSIST_RPMB_STATUS_PATH _PERSIST_RPMB_STATUS_NAME
#define _PERSIST_RPMB_STATUS_MODE 0666

int32_t rpmbeng_get_state() {
        int ret = 0;
        uint32_t status;

        ret = QSEECom_send_service_cmd(NULL, 0, &status, sizeof(uint32_t),
                        QSEECOM_RPMB_CHECK_PROV_STATUS_COMMAND);

        if (ret) {
                ALOGE("Failed to check RPMB status, ret = %d\n", ret);
                return -1;
        } else {
                switch (status) {
                        case 0:
                                ALOGI("RMPB Key status: RPMB_KEY_PROVISIONED_AND_OK (%x)\n", status);
                                break;
                        case QSEECOM_RPMB_KEY_NOT_PROVISIONED:
                                ALOGI("RMPB Key status: RPMB_KEY_NOT_PROVISIONED (%x)\n", status);
                                break;
                        case QSEECOM_RPMB_KEY_PROVISIONED_BUT_MAC_MISMATCH:
                                ALOGI("RMPB Key status: RPMB_KEY_PROVISIONED_BUT_MAC_MISMATCH (%x)\n", status);
                                break;
                        default:
                                ALOGI("RPMB Key status: Others (%x)\n", status);
                                break;
                }
                return status;
        }
}

int32_t rpmbeng_get_enable_state() {
        int ret = 0;
        int fd;
        uint32_t enable_rpmb_status = 0;

        fd = open(_PERSIST_RPMB_STATUS_FILE, O_RDONLY);
        if (-1 == fd) {
                ALOGE("Couldn't open file: %s.(%s)\n", _PERSIST_RPMB_STATUS_FILE, strerror(errno));
                ret = -1;
                goto out;
        }
        read(fd, &enable_rpmb_status, sizeof(enable_rpmb_status));
        close(fd);

        if (enable_rpmb_status == 1) {
                ret = 0;
                ALOGI("RPMB key provisioning status enabled.\n");
        } else {
                ret = -1;
                ALOGI("RPMB key provisioning status disabled.\n");
        }

out:
        if (ret == 0) {
                return 0;
        }
        return -1;
}

int32_t rpmbeng_enable_rpmb() {
        int ret = 0;
        int fd;
        uint32_t enable_rpmb_status = 1;
        struct qseecom_rpmb_provision_key send_buf = {0};
        send_buf.key_type = 0;

        /*//creat folder in init.oplus.rc for keysoter group can not creat folder in persist
          if ((mkdir(_PERSIST_RPMB_STATUS_PATH, _PERSIST_RPMB_STATUS_MODE) != 0)
          && (errno != EEXIST)) {
          ALOGE("Couldn't create persist rpmb_status directory! (%s)", strerror(errno));
          return -1;
          }*/

        umask(0000);
        fd = open(_PERSIST_RPMB_STATUS_FILE, O_RDWR | O_CREAT, _PERSIST_RPMB_STATUS_MODE);
        if (-1 == fd) {
                ALOGE("Cannot open file: %s.(%s)\n", _PERSIST_RPMB_STATUS_FILE, strerror(errno));
                return -1;
        }
        write(fd, &enable_rpmb_status, sizeof(enable_rpmb_status));
        close(fd);
        chmod(_PERSIST_RPMB_STATUS_FILE, _PERSIST_RPMB_STATUS_MODE);

        if (0 == rpmbeng_get_state()) {
                ALOGI("RPMB have key provisioned\n");
                return 0;
        }

        ret = QSEECom_send_service_cmd((void*) &send_buf, sizeof(send_buf),
                        NULL, 0, QSEECOM_RPMB_PROVISION_KEY_COMMAND);

        if (!ret) {
                ALOGI("RPMB key provisioning completed\n");
                return 0;
        } else {
                ALOGE("RPMB key provisioning failed (%d)\n", ret);
                return -1;
        }
}
