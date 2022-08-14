/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/DataInfo.cpp
 **
 ** Description:
 **      DataInfo.cpp
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          DataInfo.cpp
 ************************************************************************************/
#include "DataInfo.h"
#include <string>
#include "CreateData.h"
#include "Dump.h"
#include "FpType.h"
#include "Utils.h"

#define LOG_TAG "[FP_HAL][DataInfo]"

namespace android {
DataInfo::DataInfo(const std::string &dataType) {
    LOG_D(LOG_TAG, "DataInfo create");
    mIData = CreateData::createDataInfo(dataType);
    VOID_FUNC_EXIT();
}

DataInfo::~DataInfo() {
    LOG_D(LOG_TAG, "DataInfo destory");
    if (mIData) {
        delete mIData;
        mIData = nullptr;
    }
    VOID_FUNC_EXIT();
}

fp_return_type_t DataInfo::getDataFromTaUnlimit(char *buf,
    uint32_t len, int index, long long dataAddr) {
    fp_return_type_t err = FP_SUCCESS;
    int dumpTimes        = 0;
    int offset           = 0;
    int readSize         = 0;
    fp_dump_t *cmd       = nullptr;

    FUNC_ENTER();
    if (buf == nullptr) {
        err = FP_ERR_NULL_PTR;
        goto fp_out;
    }

    dumpTimes = len / MAX_CA_TA_SIZE + 1;
    if ((len % MAX_CA_TA_SIZE) == 0) {
        if (dumpTimes >= 1) {
            dumpTimes -= 1;
        }
    }

    offset = 0;
    for (int j = 0; j < dumpTimes; j++) {
        readSize = (j + 1) == dumpTimes ? (len - j * MAX_CA_TA_SIZE) : MAX_CA_TA_SIZE;

        int size = sizeof(fp_dump_t);
        size += readSize;
        cmd = (fp_dump_t *)malloc(size * sizeof(char));
        if (cmd == NULL) {
            err = FP_TA_ERR_NULL_PTR;
            goto fp_out;
        }

        err = Dump::getDataFromTa(cmd, size, readSize, index, offset, dataAddr);
        if (err != FP_SUCCESS) {
            LOG_E(LOG_TAG, "getDataFromTa Fail");
            goto fp_out;
        }
        char* data = (char*)cmd + cmd->config.cmd_to_data_offset;
        memcpy(buf + offset, data, readSize);
        offset += readSize;

        if (cmd != nullptr) {
            free(cmd);
            cmd = nullptr;
        }
    }

    err = Dump::dumpFreeData();
    if (err != FP_SUCCESS) {
        LOG_E(LOG_TAG, "dumpFreeData Fail");
        goto fp_out;
    }

fp_out:
    if (cmd != nullptr) {
        free(cmd);
        cmd = nullptr;
    }

    FUNC_EXIT(err);
    return err;
}

void DataInfo::setParameters(void *buf, int len) {
    mIData->setParameters(buf, len);
}

void DataInfo::getParameters(void *buf, int len) {
    VOID_FUNC_ENTER();
    mIData->getParameters(buf, len);
    VOID_FUNC_EXIT();
}

fp_return_type_t DataInfo::saveDataTaToHal() {
    mIData->saveDataTaToHal();
    return FP_SUCCESS;
}

fp_return_type_t DataInfo::sendDataHalToTa() {
    mIData->sendDataHalToTa();
    return FP_SUCCESS;
}

}
