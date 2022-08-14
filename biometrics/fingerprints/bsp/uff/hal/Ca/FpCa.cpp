/*********************************************************************************************
* Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
*
* File: FpCa.cpp
* Description: Ca for clint
* Version: 1.0
* Date : 2021-5-12
* Author: wangzhi(Kevin) wangzhi12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/05/12        create the file
*********************************************************************************************/
#define LOG_TAG "[FP_HAL][FpCa]"

#include "FpCa.h"
#include <string>
#include "CaFactory.h"
#include "HalLog.h"
#include "FpType.h"
namespace android {

FpCa::FpCa(const std::string& teeType) {
    FUNC_ENTER();
    mICa = CaFactory::createCa(teeType);
    VOID_FUNC_EXIT();
}

FpCa::FpCa() {
    FUNC_ENTER();
    mICa = nullptr;
    VOID_FUNC_EXIT();
}

FpCa::~FpCa() {
    FUNC_ENTER();
    if (mICa) {
        delete mICa;
        mICa = nullptr;
    }
    VOID_FUNC_EXIT();
}

fp_return_type_t FpCa::setTeeType(const std::string& teeType) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    mICa = CaFactory::createCa(teeType);

    if (mICa == nullptr) {
        err = FP_ERR_NULL_PTR;
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FpCa::startTa(fp_ta_name_t name) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    if (mICa == nullptr) {
        err = FP_ERR_NULL_PTR;
        goto fp_out;
    }

    err = mICa->startTa(name);
fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FpCa::closeTa() {
    fp_return_type_t err = FP_SUCCESS;

    FUNC_ENTER();
    if (mICa == nullptr) {
        err = FP_ERR_NULL_PTR;
        goto fp_out;
    }

    err = mICa->closeTa();
fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FpCa::sendCommand(void* cmd, uint32_t len) {
    fp_return_type_t err = FP_SUCCESS;
    static uint32_t errcount = 0;
    FUNC_ENTER();
    if (mICa == nullptr) {
        err = FP_ERR_NULL_PTR;
        goto fp_out;
    }

    err = mICa->sendCommand(cmd, len);
    if (err == FP_ERR_TA_DEAD) {
        errcount++;
        LOG_I(LOG_TAG, "[%s] err =%d, errcount =%d", __func__, err, errcount);
        if (errcount >= SEND_TA_COMMAND_MAX_ERR_COUNT) {
            LOG_E(LOG_TAG, "[%s]errcount large thant: %d,exit!",
                __func__, SEND_TA_COMMAND_MAX_ERR_COUNT);
            exit(0);
        }
        goto fp_out;
    } else {
        errcount = 0;
    }
fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FpCa::sendHmacKeyToFpta(){
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    if (mICa == nullptr) {
        err = FP_ERR_NULL_PTR;
        goto fp_out;
    }
    err = mICa->sendHmacKeyToFpta();
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

};  // namespace android
