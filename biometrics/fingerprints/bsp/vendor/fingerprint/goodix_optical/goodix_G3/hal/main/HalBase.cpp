/************************************************************************************
 ** File: - HalBase.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service Base for goodix optical fingerprint (android O)
 **
 ** Version: 1.0
 ** Date : 18:03:11,17/10/2018
 ** Author: oujinrong@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  luhongyu   2018/10/17           create file, and add for wakelock
 **  Ran.Chen  2019/03/06           add for fingerprint send ta cmd try
 ************************************************************************************/

#define LOG_TAG "[GF_HAL][HalBase]"
#include <cutils/properties.h>
#include "HalContext.h"
#include "CaEntry.h"
#include "HalLog.h"
#include "Device.h"
#include "HalBase.h"
#define OPLUS_TA_FINGERPRINT_RETRY "vendor.fingerprint.optical.goodix.ta.retry"

namespace goodix
{
    HalBase::HalBase(HalContext* context) :
            mContext(context)
    {
        assert(context != nullptr);
    }

    HalBase::~HalBase()
    {
        mContext = nullptr;
    }

    gf_error_t HalBase::invokeCommand(void *cmd, int32_t size) {
        return mContext->invokeCommand(cmd, size);
    }

    void HalBase::sendMessage(int32_t msg)
    {
        mContext->mMsgBus.sendMessage(msg);
    }

    void HalBase::sendMessage(int32_t msg, void* data, uint32_t dataLen)
    {
        mContext->mMsgBus.sendMessage(msg, data, dataLen);
    }

    void HalBase::sendMessage(int32_t msg, int32_t params1)
    {
        MsgBus::Message message;
        message.msg = msg;
        message.params1 = params1;
        mContext->mMsgBus.sendMessage(message);
    }

    void HalBase::sendMessage(int32_t msg, int32_t params1, int32_t params2)
    {
        MsgBus::Message message;
        message.msg = msg;
        message.params1 = params1;
        message.params2 = params2;
        mContext->mMsgBus.sendMessage(message);
    }

    void HalBase::sendMessage(int32_t msg, int32_t params1, int32_t params2, int32_t params3)
    {
        MsgBus::Message message;
        message.msg = msg;
        message.params1 = params1;
        message.params2 = params2;
        message.params3 = params3;
        mContext->mMsgBus.sendMessage(message);
    }
}  // namespace goodix

