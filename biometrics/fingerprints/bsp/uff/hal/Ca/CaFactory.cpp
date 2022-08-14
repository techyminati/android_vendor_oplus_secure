/*********************************************************************************************
* Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
*
* File: CaFactory.cpp
* Description: implment the CaFactory
* Version: 1.0
* Date : 2021-5-12
* Author: wangzhi(Kevin) wangzhi12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/05/12        create the file
*********************************************************************************************/
#define LOG_TAG "[FP_HAL][CaFactory]"

#include "CaFactory.h"
#include "HalLog.h"
#include "ReeCa.h"
#include "QseeCa.h"
#include "TrustonicCa.h"

namespace android {

ICa* CaFactory::createCa(const std::string& teeType) {
    FUNC_ENTER();

    ICa* ca = nullptr;

    LOG_I(LOG_TAG, "teeType:%s", teeType.c_str());

    if (teeType == "REE") {
        ca = dynamic_cast<ICa*>(new ReeCa());
        goto fp_out;
    }

    if (teeType == "TBASE") {
        ca = dynamic_cast<ICa*>(new TrustonicCa());
        goto fp_out;
    }

    if (teeType == "QSEE") {
        ca = dynamic_cast<ICa*>(new QseeCa());
        goto fp_out;
    }

fp_out:
    VOID_FUNC_EXIT();
    return ca;
}
};  // namespace android
