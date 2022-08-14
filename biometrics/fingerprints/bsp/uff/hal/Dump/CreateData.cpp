/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/CreateData.cpp
 **
 ** Description:
 **      CreateData.cpp
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          CreateData.cpp
 ************************************************************************************/
#include "CreateData.h"
#include "UserData.h"
#include "AppData.h"
#include "AutoTestData.h"
#include "CaliData.h"
#include "FactoryTestData.h"
#include "HalLog.h"
#include "IData.h"

#define LOG_TAG "[FP_HAL][CreateData]"

namespace android {
IData *CreateData::createDataInfo(const std::string& dataType) {
    VOID_FUNC_ENTER();
    IData *data = nullptr;
    LOG_I(LOG_TAG, "dataType:%s", dataType.c_str());

    if (dataType == "UserData") {
        data = dynamic_cast<IData *>(new UserData());
        goto fp_out;
    } else if (dataType == "AppData") {
        data = dynamic_cast<IData *>(new AppData());
        goto fp_out;
    } else if (dataType == "AutoTestData") {
        data = dynamic_cast<IData *>(new AutoTestData());
        goto fp_out;
    } else if (dataType == "CaliData") {
        data = dynamic_cast<IData *>(new CaliData());
        goto fp_out;
    } else if (dataType == "FactoryTestData") {
        data = dynamic_cast<IData *>(new FactoryTestData());
        goto fp_out;
    }

fp_out:
    VOID_FUNC_EXIT();
    return data;
}
}
