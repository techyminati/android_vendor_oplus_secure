/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "QseeParams.h"

// name to overide NEX Ta temporarily
#define GOODIX_TA_NAME "gfp9608"
#define KEYMASTER_TA_NAME "keymaster64"
#define KEYMASTER_TA_PATH    "/firmware/image"

#ifndef OPLUS_VERSION_ROOT
#define OPLUS_VERSION_ROOT "/oplus_version"
#endif

#ifndef OPLUS_PRODUCT_ROOT
#define OPLUS_PRODUCT_ROOT "/oplus_product"
#endif

#ifndef OPLUS_ODM_ROOT
#define OPLUS_ODM_ROOT "/odm"
#endif

namespace goodix {
    // fingerprint ta path list
    const char *TA_PATH_LIST[] = {
        "" OPLUS_ODM_ROOT "/vendor/firmware",
        "" OPLUS_VERSION_ROOT "/vendor/firmware",
        "" OPLUS_PRODUCT_ROOT "/vendor/firmware",
        "/data/vendor/euclid/version/vendor/firmware",
        "/data/vendor/euclid/product/vendor/firmware",
        "/vendor/firmware",
        "/system/etc/firmware",
        "/firmware/image",
        "/vendor/firmware_mnt/image",
    };

    void initCustomizedParams(QseeParams *params) {
        memcpy(params->goodixTaName, GOODIX_TA_NAME, strlen(GOODIX_TA_NAME));
        params->goodixTaNamePaths = TA_PATH_LIST;
        params->goodixTaPathCount = sizeof(TA_PATH_LIST) / sizeof(TA_PATH_LIST[0]);
        memcpy(params->keymasterTaName, KEYMASTER_TA_NAME, strlen(KEYMASTER_TA_NAME));
        memcpy(params->keymasterTaPath, KEYMASTER_TA_PATH, strlen(KEYMASTER_TA_PATH));
    }
}  // namespace goodix

