/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/AppData/AppData.cpp
 **
 ** Description:
 **      AppData.cpp
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          AppData.cpp
 **  WangTao    2021/10/11          capture tool dump fingerprint image data
 ************************************************************************************/
#include "AppData.h"
#include "FpType.h"
#include "HalLog.h"
#include <string>
#include "Utils.h"
#include "Dump.h"

using namespace std;

#define LOG_TAG "[FP_HAL][AppData]"

namespace android {

AppData::AppData() {
    mAppDumpPara = nullptr;
}

AppData::~AppData() {
}

void AppData::setParameters(void *buf, int len) {
    VOID_FUNC_ENTER();
    UNUSE(buf);
    UNUSE(len);
    if (buf == NULL) {
        LOG_E(LOG_TAG, "app dump data is NULL, %p", buf);
        return;
    }
    mAppDumpPara = (fp_user_data_t *)buf;

    VOID_FUNC_EXIT();
}

void AppData::getParameters(void *buf, int len) {
    VOID_FUNC_ENTER();
    UNUSE(buf);
    UNUSE(len);
    VOID_FUNC_EXIT();
}

fp_return_type_t AppData::saveDataTaToHal() {
    if (mAppDumpPara == nullptr) {
        return FP_ERR_NULL_PTR;
    }
    const char *rootpath = "/data/vendor/fingerprint/dump/app/";
    LOG_D(LOG_TAG, "file name:%s", mAppDumpPara->filePath);
    char full_path[128] = {'\0'};
    if (Utils::isFileExist(rootpath) == FILE_NOT_FOUND) {
        LOG_I(LOG_TAG, "dir not exist, create:%s", rootpath);
        if (Utils::makePath(rootpath)) {
            LOG_E(LOG_TAG, "not have right to create dir");
        }
    }
    sprintf(full_path, "%s%s", rootpath, mAppDumpPara->filePath);

    if (Utils::writeData(full_path, mAppDumpPara->data, mAppDumpPara->dataLen)) {
        LOG_E(LOG_TAG, "write factory dump fail:%s", full_path);
    }
    mAppDumpPara = nullptr;
    return FP_SUCCESS;
}

fp_return_type_t AppData::sendDataHalToTa(void) {
    VOID_FUNC_ENTER();
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}

}
