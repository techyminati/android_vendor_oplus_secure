/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/Dump.cpp
 **
 ** Description:
 **      Dump for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26      create the file
 **  WangTao    2021/05/25      Dump.cpp
 **  WangTao    2021/08/10      Complete function ok on the phone;
 **                             auth、enroll、calidata dump;
 **  WangTao    2021/08/15      add UserData CaliData FactoryTestData
 **                             AutoTestData AppData
 ************************************************************************************/
#include <cutils/properties.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include "DataInfo.h"
#include "Dump.h"
#include "FpCommon.h"
#include "FpType.h"
#include "HalContext.h"
#include "HalLog.h"
#include "Utils.h"

using namespace std;

namespace android {
#define FULLNAME_MAX 255
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "[FP_HAL][DUMP]"
#endif

static fp_mode_t      mFpCurrentMode;
static char           mDumpSupport    = 0;
static unsigned char  mPreversionFlag = 0;
static fp_user_data_t mUserData;

#define USER_DATA_MODE_NAME "UserData"
#define APP_DATA_MODE_NAME  "AppData"
#define NONE_DATA_MODE_NAME ""

Dump::Dump() { LOG_D(LOG_TAG, "Dump create"); }

Dump::~Dump() { LOG_D(LOG_TAG, "Dump destory"); }

fp_return_type_t Dump::getTaFileCount(int *cnt) {
    fp_return_type_t ret = FP_SUCCESS;
    fp_dump_t cmd;
    int       size = sizeof(fp_dump_t);
    memset(&cmd, 0, size);
    cmd.header.module_id       = FP_MODULE_DUMP;
    cmd.header.cmd_id          = FP_CMD_DUMP_GET_DATA;
    cmd.config.fp_current_mode = mFpCurrentMode;
    cmd.config.dump_stage      = DUMP_STAGE_1_GET_FILE_COUNT;
    ret = HalContext::getInstance()->mCaEntry->sendCommand(&cmd, sizeof(cmd));
    if (ret != FP_SUCCESS) {
        LOG_E(LOG_TAG, "getTaFileCount sendCommand fail")
        goto fp_out;
    }
    *cnt = cmd.result.file_count;
    LOG_D(LOG_TAG, "dump file count:%d", *cnt);
fp_out:
    return ret;
}

fp_return_type_t Dump::getTaDataSize(int fileIndex, long long *dataAddr, char *file_path, int *dataSize) {
    fp_return_type_t ret = FP_SUCCESS;
    fp_dump_t cmd;
    int       size = sizeof(fp_dump_t);
    memset(&cmd, 0, size);
    cmd.header.module_id       = FP_MODULE_DUMP;
    cmd.header.cmd_id          = FP_CMD_DUMP_GET_DATA;
    cmd.config.fp_current_mode = mFpCurrentMode;
    cmd.config.dump_stage      = DUMP_STAGE_2_GET_DATA_INFO;
    cmd.config.dump_file_index = fileIndex;
    cmd.config.preversionFlag  = getPreversionFlag();
    ret = HalContext::getInstance()->mCaEntry->sendCommand(&cmd, sizeof(cmd));
    if (ret != FP_SUCCESS) {
        LOG_E(LOG_TAG, "getTaDataSize sendCommand Fail");
        goto fp_out;
    }
    *dataSize = cmd.result.data_size;
    *dataAddr    = cmd.result.data_addr;
    memcpy(file_path, cmd.result.file_path, sizeof(cmd.result.file_path));
fp_out:
    return ret;
}

fp_return_type_t Dump::dumpFreeData() {
    fp_return_type_t ret = FP_SUCCESS;
    fp_dump_t cmd;
    int       size = sizeof(fp_dump_t);
    memset(&cmd, 0, size);
    cmd.header.module_id       = FP_MODULE_DUMP;
    cmd.header.cmd_id          = FP_CMD_DUMP_GET_DATA;
    cmd.config.fp_current_mode = mFpCurrentMode;
    cmd.config.dump_stage      = DUMP_STAGE_4_FREE_DATA;
    ret = HalContext::getInstance()->mCaEntry->sendCommand(&cmd, sizeof(cmd));
    if (ret != FP_SUCCESS) {
        LOG_E(LOG_TAG, "dumpFreeData sendCommand Fail");
    }

    return ret;
}

fp_return_type_t Dump::getDataFromTa(
    fp_dump_t *cmd, int size, int readSize, int file_index, int offset, long long dataAddr) {
    fp_return_type_t ret = FP_SUCCESS;
    memset(cmd, 0, size);
    cmd->header.module_id             = FP_MODULE_DUMP;
    cmd->header.cmd_id                = FP_CMD_DUMP_GET_DATA;
    cmd->config.fp_current_mode       = mFpCurrentMode;
    cmd->config.cmd_to_data_offset    = sizeof(fp_dump_t);
    cmd->config.result_to_data_offset = sizeof(fp_dump_data_result_t);
    cmd->config.data_size             = readSize;
    cmd->config.dump_stage            = DUMP_STAGE_3_GET_DATA_BUF;
    cmd->config.dump_file_index       = file_index;
    cmd->result.offset                = offset;
    cmd->result.data_addr             = dataAddr;
    ret = HalContext::getInstance()->mCaEntry->sendCommand(cmd, size);
    if (ret != FP_SUCCESS) {
        LOG_E(LOG_TAG, "getDataFromTa sendCommand Fail");
    }

    return ret;
}

void Dump::setCurrentMode(fp_mode_t mode) { mFpCurrentMode = mode; }

fp_mode_t Dump::getCurrentMode() { return mFpCurrentMode; }

void Dump::setPreversionFlag(unsigned char flag) { mPreversionFlag = flag; }

unsigned char Dump::getPreversionFlag() { return mPreversionFlag; }

int Dump::dumpFileManager(const char *dir, const char *path) {
    int              err      = 0;
    static long long totalCnt = 0;
    FUNC_ENTER();
    FILE * fp = NULL;
    size_t writeLen;
    char   mode[8]                     = {0};
    char   fileName[MAX_FILE_PATH_LEN] = {0};
    char   buf[MAX_FILE_PATH_LEN + 1]  = {0};
    strcat(fileName, dir);
    strcat(fileName, "/index.txt");

    ifstream fin(fileName);
    string   str;

    if (fin.is_open()) {
        getline(fin, str);
        totalCnt = atoi(str.c_str());
        fin.close();
    } else {
        LOG_E(LOG_TAG, "index.txt open fail");
    }

    if (access(fileName, 0) != 0) {
        sprintf(mode, "%s", "a");
    } else {
        sprintf(mode, "%s", "r+");
    }

    totalCnt = totalCnt + 1;
    if (totalCnt > 1000000) {
        totalCnt = 0;
        err = Utils::removePath(fileName);
        if (err != 0) {
            LOG_E(LOG_TAG, "removePath fail");
            goto fp_out;
        }
    }
    sprintf(buf, "%lld", totalCnt);
    if ((fp = fopen(fileName, mode)) == NULL) {
        LOG_E(LOG_TAG, "fail to open fp: %s (%d:%s)", fileName, errno, strerror(errno));
        err = -1;
        goto fp_out;
    }
    fseek(fp, 0, SEEK_SET);

    LOG_D(LOG_TAG, "open OK and write data");
    sprintf(buf + MAX_FILE_PATH_LEN - 1, "%s", "\n");
    writeLen = fwrite(buf, sizeof(char), MAX_FILE_PATH_LEN, fp);
    fflush(fp);
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    memset(buf, 0, sizeof(buf));
    if (totalCnt <= MAX_FILE_COUNT) {
        err = Utils::writeData(fileName, path, MAX_FILE_PATH_LEN, 1, 1);
        if (err != 0) {
            LOG_E(LOG_TAG, "writeData fail");
            goto fp_out;
        }
    } else {
        int offset = totalCnt % (MAX_FILE_COUNT + 0);
        if (offset == 0) {
            offset = 60;
        }

        strcat(buf, path);
        if ((fp = fopen(fileName, "r+")) == NULL) {
            LOG_E(LOG_TAG, "fail to open fp: %s (%d:%s)", fileName, errno, strerror(errno));
            err = -1;
            goto fp_out;
        }
        fseek(fp, 0 + (offset + 0) * MAX_FILE_PATH_LEN, SEEK_SET);
        sprintf(buf + MAX_FILE_PATH_LEN - 1, "%s", "\n");
        writeLen = fwrite(buf, sizeof(char), MAX_FILE_PATH_LEN, fp);
        fflush(fp);
        fseek(fp, 0 - MAX_FILE_PATH_LEN, SEEK_CUR);
        fgets(buf, sizeof(buf), fp);
        err = Utils::removePath(buf);
        if (err != 0) {
            LOG_E(LOG_TAG, "removePath fail");
            goto fp_out;
        }
        LOG_D(LOG_TAG, "removePath:%s", buf);

        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
    }

fp_out:
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    LOG_D(LOG_TAG, "OUT");
    return FP_SUCCESS;
}

string Dump::getDataType(fp_mode_t mode) {
    switch (mode) {
        case FP_MODE_ENROLL:
        case FP_MODE_AUTH:
        case FP_MODE_CALI:
            return USER_DATA_MODE_NAME;
            break;
        case FP_MODE_FACTORY_TEST:
            return "FactoryTestData";
        case FP_MODE_APP_DATA:
            return APP_DATA_MODE_NAME;
        default:
            return NONE_DATA_MODE_NAME;
            break;
    }

    return NONE_DATA_MODE_NAME;
}

void Dump::getEngineeringType() {
    int  err = FP_SUCCESS;
    char engineeringType[PROPERTY_VALUE_MAX];

    err = property_get("ro.oplus.image.my_engineering.type", engineeringType, "preversion");
    if (err < 0) {
        LOG_E(LOG_TAG, "get ro.oplus.image.my_engineering.type error, default is preversion");
    }
    if (strcmp(engineeringType, "preversion") == 0) {
        LOG_I(LOG_TAG, "preversion version");
        setPreversionFlag(1);
    } else {
        setPreversionFlag(0);
    }
}

void *Dump::generateParameters(char *file_path, char *buf, int len, int format, unsigned int gid,
    unsigned int result, unsigned int fingerId, fp_mode_t mode) {
    memcpy(mUserData.filePath, file_path, sizeof(mUserData.filePath));
    mUserData.data       = buf;
    mUserData.dataLen    = len;
    mUserData.withFormat = format;
    mUserData.gid        = gid;
    mUserData.result     = result;
    mUserData.fingerId   = fingerId;
    mUserData.mode       = mode;
    return reinterpret_cast<void *>(&mUserData);
}

int Dump::getGenerateParametersLength(fp_mode_t mode) {
    int len = 0;
    switch (mode) {
        case FP_MODE_AUTH:
        case FP_MODE_ENROLL:
        case FP_MODE_CALI:
            len = sizeof(fp_user_data_t);
            break;
        default:
            len = 0;
            break;
    }

    return len;
}

int Dump::dumpProcess(fp_mode_t fpMode, uint32_t gid, unsigned int fingerId, int result) {
    int err = FP_SUCCESS;

    FUNC_ENTER();
    int       cnt      = 0;
    int       dataSize = 0;
    long long dataAddr = 0;
    char      file_path[MAX_FP_DUMP_FILE_PATH_LENGTH];
    DataInfo *mDataInfo = nullptr;
    void *    para      = nullptr;
    char *    caBuf     = nullptr;
    TIME_START(dumpProcess);
    if (mDumpSupport == 0) {
        LOG_I(LOG_TAG, "Dump is Disable");
        goto fp_out;
    } else {
        LOG_D(LOG_TAG, "Dump is Enable");
    }

    setCurrentMode(fpMode);

    LOG_I(LOG_TAG, "CurMode:%d, fingerId:%x, result:%d", getCurrentMode(), fingerId, result);
    getEngineeringType();

    err = (int)getTaFileCount(&cnt);
    if (err != FP_SUCCESS) {
        LOG_E(LOG_TAG, "getTaFileCount Fail");
        goto fp_out;
    }

    if (cnt == 0) {
        LOG_E(LOG_TAG, "cnt:%d is zero, no need dump data", cnt);
        goto fp_out;
    }

    mDataInfo = new DataInfo(getDataType(getCurrentMode()));
    if (mDataInfo == nullptr) {
        LOG_E(LOG_TAG, "mDataInfo is nullptr");
        goto fp_out;
    }

    for (int i = 0; i < cnt; i++) {
        err = (int)getTaDataSize(i, &dataAddr, file_path, &dataSize);
        if (err != FP_SUCCESS) {
            LOG_E(LOG_TAG, "getTaDataSize Fail");
            goto fp_out;
        }

        if (dataSize == 0) {
            LOG_D(LOG_TAG, "dataSize = 0");
            continue;
        }
        LOG_I(LOG_TAG, "file_path:%s", file_path);
        caBuf = (char *)malloc(dataSize * sizeof(char));
        if (caBuf == NULL) {
            err = FP_TA_ERR_NULL_PTR;
            goto fp_out;
        }
        err = mDataInfo->getDataFromTaUnlimit(caBuf, dataSize, i, dataAddr);
        if (err != FP_SUCCESS) {
            LOG_E(LOG_TAG, "getDataFromTaUnlimit fail");
            goto fp_out;
        }

        switch (getCurrentMode()) {
            case FP_MODE_AUTH:
            case FP_MODE_CALI:
            case FP_MODE_ENROLL:
            case FP_MODE_FACTORY_TEST:
            case FP_MODE_APP_DATA:
                para = (void *)generateParameters(
                    file_path, caBuf, dataSize, 0, gid, result, fingerId, fpMode);
                if (para == nullptr) {
                    LOG_E(LOG_TAG, "generateParameters fail, para is nullptr");
                    goto fp_out;
                }
                break;
            default:
                para = nullptr;
                break;
        }

        mDataInfo->setParameters(para, getGenerateParametersLength(getCurrentMode()));
        mDataInfo->saveDataTaToHal();
        if (caBuf != nullptr) {
            free(caBuf);
            caBuf = nullptr;
        }
        switch (getCurrentMode()) {
            case FP_MODE_AUTH:
            case FP_MODE_CALI:
            case FP_MODE_ENROLL:
                mDataInfo->getParameters(&mUserData, sizeof(mUserData));
                mUserData.mEnrollPath = string(mUserData.enrollRenamePath);
                LOG_I(LOG_TAG, "mEnrollPath:%s", mUserData.mEnrollPath.c_str());
                break;
            default:
                break;
        }
    }

fp_out:
    TIME_END(dumpProcess);
    if (mDataInfo) {
        delete mDataInfo;
    }

    if (caBuf != nullptr) {
        free(caBuf);
        caBuf = nullptr;
    }

    FUNC_EXIT(FP_SUCCESS);
    return err;
}

void Dump::dumpRename(unsigned int finger_id) {
    FUNC_ENTER();
    int err = FP_SUCCESS;
    char fingerId[64] = {0};
    sprintf(fingerId, "%u", finger_id);
    string::size_type index   = mUserData.mEnrollPath.rfind("temp");
    string            newPath = mUserData.mEnrollPath.substr(0, index);

    if (index == string::npos) {
        LOG_E(LOG_TAG, "mUserData.mEnrollPath no temp str");
        goto fp_out;
    }

    if (mDumpSupport == 0) {
        LOG_I(LOG_TAG, "%s Dump is Disable", __func__);
        goto fp_out;
    } else {
        LOG_I(LOG_TAG, "%s Dump is Enable", __func__);
    }

    newPath += string(fingerId);
    LOG_I(LOG_TAG, "%s newPath:%s, oldPath:%s", __func__, newPath.c_str(), mUserData.mEnrollPath.c_str());
    err = Utils::utilsRename((char *)(mUserData.mEnrollPath.c_str()), (char *)newPath.c_str());
    if (err != FP_SUCCESS) {
        LOG_E(LOG_TAG, "rename fail");
    }
fp_out:
    FUNC_EXIT(FP_SUCCESS);
}

void Dump::setDumpSupport(char dumpSupport) {
    mDumpSupport = dumpSupport;
    LOG_I(LOG_TAG, "mDumpSupport:%d", mDumpSupport);
}

};  // namespace android
