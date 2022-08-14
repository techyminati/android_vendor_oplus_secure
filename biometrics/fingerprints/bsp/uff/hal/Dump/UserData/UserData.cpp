/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/UserData/UserData.cpp
 **
 ** Description:
 **      UserData.cpp
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          UserData.cpp
 ************************************************************************************/
#include "UserData.h"
#include "FpType.h"
#include "HalLog.h"
#include <string>
#include "Utils.h"
#include "Dump.h"

using namespace std;

#define LOG_TAG "[FP_HAL][UserData]"
#define USER_DATA_BASE_PATH "/data/vendor/fingerprint/dump"

namespace android {

UserData::UserData() {
}

UserData::~UserData() {
}

void UserData::setParameters(void *buf, int len) {
    FUNC_ENTER();
    fp_user_data_t *user_data = (fp_user_data_t *)buf;
    user_data = SAFE_CAST_CMD_BUF(fp_user_data_t, buf, len);
    if (user_data == NULL) {
        LOG_E(LOG_TAG, "user_data is NULL, invalid buf");
        goto fp_out;
    }
    LOG_I(LOG_TAG, "fileP:%s,fingerId:%d, gid:%d,result:%d,format:%d,mode:%d",
        user_data->filePath, user_data->fingerId, user_data->gid,
        user_data->result, user_data->withFormat, user_data->mode);
    mCurrentMode = user_data->mode;
    sprintf(mGidName, "%d", user_data->gid);
    LOG_I(LOG_TAG, "mGidName:%s", mGidName);
    mFilePath = string(user_data->filePath);
    mResult = user_data->result;
    mFingerId = user_data->fingerId;
    mGid = user_data->gid;
    mFormat = user_data->withFormat;
    mData = user_data->data;
    mDataLen = user_data->dataLen;
fp_out:
    VOID_FUNC_EXIT();
}

void UserData::getParameters(void *buf, int len) {
    VOID_FUNC_ENTER();

    fp_user_data_t *user_data = (fp_user_data_t *)buf;
    user_data = SAFE_CAST_CMD_BUF(fp_user_data_t, buf, len);
    if (user_data == NULL) {
        LOG_E(LOG_TAG, "user_data is NULL, invalid buf");
        goto fp_out;
    }

    memcpy(user_data->enrollRenamePath, mEnrollRenamePath, sizeof(mEnrollRenamePath));

fp_out:
    VOID_FUNC_EXIT();
}

bool UserData::checkBmpFileName() {
    bool ret = 0;
    FUNC_ENTER();
    string path;
    string fileName;
    string suffixName;
    string::size_type index;
    LOG_D(LOG_TAG, "mFilePath:%s", mFilePath.c_str());
    index = mFilePath.rfind('/');
    if (index != string::npos) {
        fileName = mFilePath.substr(index+1);
    } else {
        LOG_D(LOG_TAG, "index == string::npos");
        fileName = mFilePath;
    }

    index = fileName.rfind('.');
    if (index != string::npos) {
        suffixName = mFilePath.substr(index+1);
    } else {
        LOG_D(LOG_TAG, "index == string::npos");
        suffixName = mFilePath;
    }
    LOG_D(LOG_TAG, "suffixName:%s", suffixName.c_str());
    ret = strcmp(suffixName.c_str(), "bmp");
    if (ret == 0) {
        LOG_D(LOG_TAG, "suffixName is .bmp");
        return true;
    } else {
        return false;
    }

    return false;
}

void UserData::setUserDataDirName() {
    string userDataBasePath = USER_DATA_BASE_PATH;

    switch (mCurrentMode) {
        case FP_MODE_CALI:
            sprintf(mCurModeName, "%s", "cali");
            sprintf(mFingerIdName, "%s", "/");
        break;
        case FP_MODE_ENROLL:
            sprintf(mCurModeName, "%s", "enroll");
            switch (mResult) {
                case 0:
                    sprintf(mFingerIdName, "%s", "temp");
                break;
                case 1:
                    sprintf(mFingerIdName, "%s", "fail");
                break;
                default:
                    sprintf(mFingerIdName, "%s", "fail");
                break;
            }
        break;
        case FP_MODE_AUTH:
            sprintf(mCurModeName, "%s", "auth");
            switch (mResult) {
                case 0:
                    sprintf(mFingerIdName, "%u", mFingerId);
                break;
                case 1:
                    sprintf(mFingerIdName, "%s", "fail");
                break;
                default:
                    sprintf(mFingerIdName, "%s", "fail");
                break;
            }
        break;
        default:
            sprintf(mCurModeName, "%s", "temp");
            sprintf(mFingerIdName, "%s", "temp");
        break;
    }

    if (checkBmpFileName() == true) {
        mBaseDir = userDataBasePath + "/" + string(mGidName) + "/" + string(mCurModeName)
            + "/" + string(mFingerIdName) + "/";
        if ((mCurrentMode == FP_MODE_ENROLL) && (mResult == 0)) {
            memcpy(mEnrollRenamePath, mBaseDir.c_str(), mBaseDir.length());
            LOG_I(LOG_TAG, "mEnrollRenamePath:%s", mEnrollRenamePath);
        }
    } else {
        mBaseDir = userDataBasePath + "/vendor/";
    }

    LOG_I(LOG_TAG, "%s,gid:%d, baseDir %s, mEnrollRenamePath:%s", __func__,
          mGid, mBaseDir.c_str(), mEnrollRenamePath);
}

int UserData::save() {
    int err = 0;
    FUNC_ENTER();
    string path;
    string fileName;
    string vendorDir;
    string::size_type index;

    index = mFilePath.rfind('/');
    if (index != string::npos) {
        fileName = mFilePath.substr(index+1);
        vendorDir = mFilePath.substr(0, index);

    } else {
        LOG_D(LOG_TAG, "index == string::npos");
        fileName = mFilePath;
    }

    setUserDataDirName();

    mBaseDir += vendorDir + "/";

    if (mFormat) {
        Utils::getTimestamp(mTimestamp);
        path = mBaseDir + string(mTimestamp) + "_" + fileName;
    } else {
        path = mBaseDir + fileName;
    }

    err = Utils::makePath(mBaseDir.c_str());
    if (err != 0) {
        LOG_E(LOG_TAG, "dumpMkdir error, err:%d", err);
        goto fp_out;
    }
    err = Dump::dumpFileManager(mBaseDir.c_str(), path.c_str());
    if (err != FP_SUCCESS) {
        LOG_E(LOG_TAG, "dumpFileManager error, err:%d", err);
        goto fp_out;
    }
    err = Utils::writeData(path.c_str(), mData, mDataLen);
    if (err != 0) {
        LOG_E(LOG_TAG, "writeData error, err:%d", err);
        goto fp_out;
    }
fp_out:
    return err;
}

fp_return_type_t UserData::saveDataTaToHal() {
    VOID_FUNC_ENTER();
    save();
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}

fp_return_type_t UserData::sendDataHalToTa(void) {
    VOID_FUNC_ENTER();
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}

}
