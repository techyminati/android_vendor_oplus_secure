/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/AutoTestData/AutoTestData.cpp
 **
 ** Description:
 **      AutoTestData.cpp
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          AutoTestData.cpp
 ************************************************************************************/
#include "AutoTestData.h"
#include "FpType.h"
#include "HalLog.h"
#include <string>
#include "Utils.h"
#include "Dump.h"

using namespace std;

#define LOG_TAG "[FP_HAL][AutoTestData]"

namespace android {

AutoTestData::AutoTestData() {
}

AutoTestData::~AutoTestData() {
}

void AutoTestData::setParameters(void *buf, int len) {
    VOID_FUNC_ENTER();
    UNUSE(buf);
    UNUSE(len);
    VOID_FUNC_EXIT();
}

void AutoTestData::getParameters(void *buf, int len) {
    VOID_FUNC_ENTER();
    UNUSE(buf);
    UNUSE(len);
    VOID_FUNC_EXIT();
}

fp_return_type_t AutoTestData::saveDataTaToHal() {
    VOID_FUNC_ENTER();
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}

fp_return_type_t AutoTestData::sendDataHalToTa(void) {
    VOID_FUNC_ENTER();
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}

}
