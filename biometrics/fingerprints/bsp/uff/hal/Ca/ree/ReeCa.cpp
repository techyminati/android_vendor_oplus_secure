/*********************************************************************************************
* Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
*
* File: ReeCa.cpp
* Description: ree
* Version: 1.0
* Date : 2021-5-12
* Author: wangzhi(Kevin) wangzhi12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/05/12        create the file
*********************************************************************************************/
#define LOG_TAG "[FP_HAL][ReeCa]"

#ifdef TEE_REE

#include "ReeCa.h"
#include "HalLog.h"
#include "ree_com_api.h"

namespace android {

ReeCa::ReeCa() {
    FUNC_ENTER();
    VOID_FUNC_EXIT();
}

ReeCa::~ReeCa() {
    FUNC_ENTER();
    VOID_FUNC_EXIT();
}

fp_return_type_t ReeCa::startTa(fp_ta_name_t name) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    (void)name;
    LOG_I(LOG_TAG, "call ree_ta_init");
    err = (fp_return_type_t)ree_ta_init();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t ReeCa::closeTa() {
    fp_return_type_t err = FP_SUCCESS;

    FUNC_ENTER();
    LOG_I(LOG_TAG, "call ree_ta_shutdown");
    err = (fp_return_type_t)ree_ta_shutdown();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t ReeCa::sendCommand(void *cmd, uint32_t len) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    err = (fp_return_type_t)ree_ta_cmd_handler(cmd, len);

    FUNC_EXIT(err);
    return err;
}

fp_return_type_t ReeCa::sendHmacKeyToFpta() {
    return FP_SUCCESS;
}

}  // namespace android

#else

#include "ReeCa.h"

namespace android {
ReeCa::ReeCa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
}

ReeCa::~ReeCa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
}

fp_return_type_t ReeCa::startTa(fp_ta_name_t name) {
    (void)name;
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t ReeCa::closeTa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t ReeCa::sendCommand(void* cmd, uint32_t size)  {
    (void)cmd;
    (void)size;
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t ReeCa::sendHmacKeyToFpta() {
    return FP_SUCCESS;
}

}
#endif
