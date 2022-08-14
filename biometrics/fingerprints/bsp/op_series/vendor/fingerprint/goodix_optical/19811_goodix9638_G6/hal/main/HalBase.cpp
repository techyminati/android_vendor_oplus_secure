/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][HalBase]"

#include "HalContext.h"
#include "CaEntry.h"
#include "HalLog.h"
#include "Device.h"
#include "HalBase.h"

namespace goodix {
    HalBase::HalBase(HalContext *context) :
        mContext(context) {
        assert(context != nullptr);
    }

    HalBase::~HalBase() {
        mContext = nullptr;
    }

    gf_error_t HalBase::invokeCommand(void *cmd, int32_t size) {
        return mContext->invokeCommand(cmd, size);
    }

    void HalBase::sendMessage(int32_t msg) {
        mContext->mMsgBus.sendMessage(msg);
    }

    void HalBase::sendMessage(int32_t msg, void *data, uint32_t dataLen) {
        mContext->mMsgBus.sendMessage(msg, data, dataLen);
    }

    void HalBase::sendMessage(int32_t msg, int32_t params1) {
        MsgBus::Message message;
        message.msg = msg;
        message.params1 = params1;
        mContext->mMsgBus.sendMessage(message);
    }

    void HalBase::sendMessage(int32_t msg, int32_t params1, int32_t params2) {
        MsgBus::Message message;
        message.msg = msg;
        message.params1 = params1;
        message.params2 = params2;
        mContext->mMsgBus.sendMessage(message);
    }

    void HalBase::sendMessage(int32_t msg, int32_t params1, int32_t params2,
                              int32_t params3) {
        MsgBus::Message message;
        message.msg = msg;
        message.params1 = params1;
        message.params2 = params2;
        message.params3 = params3;
        mContext->mMsgBus.sendMessage(message);
    }
}  // namespace goodix

