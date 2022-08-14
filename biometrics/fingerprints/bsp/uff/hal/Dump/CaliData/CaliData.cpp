/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/CaliData/CaliData.cpp
 **
 ** Description:
 **      CaliData.cpp
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          CaliData.cpp
 ************************************************************************************/
#include "CaliData.h"
#include "FpType.h"
#include "HalLog.h"
#include <string>
#include "Utils.h"
#include "Dump.h"

using namespace std;

#define LOG_TAG "[FP_HAL][CaliData]"

namespace android {

CaliData::CaliData() {
}

CaliData::~CaliData() {
}

void CaliData::setParameters(void *buf, int len) {
    VOID_FUNC_ENTER();
    UNUSE(buf);
    UNUSE(len);
    VOID_FUNC_EXIT();
}

void CaliData::getParameters(void *buf, int len) {
    VOID_FUNC_ENTER();
    UNUSE(buf);
    UNUSE(len);
    VOID_FUNC_EXIT();
}

fp_return_type_t CaliData::saveDataTaToHal() {
    VOID_FUNC_ENTER();
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}

fp_return_type_t CaliData::sendDataHalToTa(void) {
    VOID_FUNC_ENTER();
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}

}
