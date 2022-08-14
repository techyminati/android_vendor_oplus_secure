/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "QseeParams.h"


#define GOODIX_TA_NAME "gfp9558"
#define KEYMASTER_TA_NAME "keymaster64"
#define KEYMASTER_TA_PATH    "/firmware/image"
namespace goodix {
    // fingerprint ta path list
    const char *TA_PATH_LIST[] = {
        "/vendor/firmware_mnt/image",
        "/odm/vendor/firmware",
        "/system/etc/firmware",
        "/firmware/image",
        "/vendor/firmware",
    };

    void initCustomizedParams(QseeParams *params) {
        memcpy(params->goodixTaName, GOODIX_TA_NAME, strlen(GOODIX_TA_NAME));
        params->goodixTaNamePaths = TA_PATH_LIST;
        params->goodixTaPathCount = sizeof(TA_PATH_LIST) / sizeof(TA_PATH_LIST[0]);
        memcpy(params->keymasterTaName, KEYMASTER_TA_NAME, strlen(KEYMASTER_TA_NAME));
        memcpy(params->keymasterTaPath, KEYMASTER_TA_PATH, strlen(KEYMASTER_TA_PATH));
    }
}  // namespace goodix

