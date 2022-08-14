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
 **  oujinrong  2019/08/27           add TA path for product partition
 **  oujinrong  2019/09/06           add for product partition in recovery
 **  chenran    2019/09/06           add for ta patch(oplus_verison/vendor/firmware)
 ************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "QseeParams.h"

#define GOODIX_TA_NAME "goodixfp"
#define KEYMASTER_TA_NAME "keymaster64"
#define KEYMASTER_TA_PATH    "/firmware/image"

#ifndef MY_VERSION_ROOT
#define MY_VERSION_ROOT "/oplus_version"
#endif

#ifndef MY_PRODUCT_ROOT
#define MY_PRODUCT_ROOT "/oplus_product"
#endif

namespace goodix
{
    // fingerprint ta path list
    const char *TA_PATH_LIST[] =
    {
        "odm/vendor/firmware",
        "" MY_VERSION_ROOT "/vendor/firmware",
        "" MY_PRODUCT_ROOT "/vendor/firmware",
        "/data/vendor/euclid/product/vendor/firmware",
        "/data/vendor/euclid/version/vendor/firmware",
        "/vendor/firmware",
        "/system/etc/firmware",
        "/firmware/image",   
        "/vendor/firmware_mnt/image",
    };

    void initCustomizedParams(QseeParams* params)
    {
        memcpy(params->goodixTaName, GOODIX_TA_NAME, strlen(GOODIX_TA_NAME));
        params->goodixTaNamePaths = TA_PATH_LIST;
        params->goodixTaPathCount = sizeof(TA_PATH_LIST) / sizeof(TA_PATH_LIST[0]);
        memcpy(params->keymasterTaName, KEYMASTER_TA_NAME, strlen(KEYMASTER_TA_NAME));
        memcpy(params->keymasterTaPath, KEYMASTER_TA_PATH, strlen(KEYMASTER_TA_PATH));
    }
}  // namespace goodix

