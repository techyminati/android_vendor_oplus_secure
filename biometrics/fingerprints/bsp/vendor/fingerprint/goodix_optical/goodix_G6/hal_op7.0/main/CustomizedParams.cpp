/************************************************************************************
 ** File: - CustomizedParams.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix optical fingerprint (android O)
 **
 ** Version: 1.0
 ** Date : 18:03:11,27/08/2018
 ** Author: oujinrong@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  chenran    2019/09/06           add for ta patch(oppo_verison/vendor/firmware)
 ************************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "QseeParams.h"

// name to overide NEX Ta temporarily
#ifdef FP_MULTI_TA
#define GOODIX_TA_NAME "goodixfp_G6"
#else
#define GOODIX_TA_NAME "goodixfp"
#endif

#define KEYMASTER_TA_NAME "keymaster64"
#define KEYMASTER_TA_PATH    "/firmware/image"

#ifndef MY_VERSION_ROOT
#define MY_VERSION_ROOT "/oppo_version"
#endif

#ifndef MY_PRODUCT_ROOT
#define MY_PRODUCT_ROOT "/oppo_product"
#endif

#ifndef OPLUS_ODM_ROOT
#define OPLUS_ODM_ROOT "/odm"
#endif

namespace goodix {
    // fingerprint ta path list
    const char *TA_PATH_LIST[] = {
        "" OPLUS_ODM_ROOT "/vendor/firmware",
        "" MY_VERSION_ROOT "/vendor/firmware",
        "" MY_PRODUCT_ROOT "/vendor/firmware",
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
