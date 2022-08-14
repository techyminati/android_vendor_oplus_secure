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

    gf_error_t HalBase::invokeCommand(void* cmd, int32_t size)
    {
        gf_error_t err = GF_SUCCESS;
        static uint32_t send_ta_cmd_error_times = 0;
        char retry_ta[PROPERTY_VALUE_MAX] = {0};
        GF_ASSERT_NOT_NULL(mContext->mCaEntry);
        GF_ASSERT_NOT_NULL(mContext->mDevice);
        Mutex::Autolock _l(mContext->mSensorLock);
        Device::AutoSpiClock _m(mContext->mDevice);
        do
        {
            gf_cmd_header_t* header = (gf_cmd_header_t*) cmd;
            header->reset_flag = 0;
            mContext->mDevice->hold_wakelock(GF_HOLD_WAKE_LOCK);
            err = mContext->mCaEntry->sendCommand(cmd, size);
            mContext->mDevice->hold_wakelock(GF_RELEASE_WAKE_LOCK);
            if (err == GF_ERROR_TA_DEAD)
            {
                LOG_E(LOG_TAG, "[%s] TA DEAD, ta_err_times =%d", __func__, send_ta_cmd_error_times);
                send_ta_cmd_error_times++;
                property_get(OPLUS_TA_FINGERPRINT_RETRY, retry_ta, "false");
                if((send_ta_cmd_error_times >= 5)&&(0 != strncmp(retry_ta, "true", sizeof("true")))){
                    LOG_E(LOG_TAG, "[%s] ta error_times = 5, restart ta", __func__);
                    property_set(OPLUS_TA_FINGERPRINT_RETRY, "true");
                    exit(0);
                }else{
                    sendMessage(MsgBus::MSG_TA_DEAD);
                    mContext->reInit();
                }
                break;
            }
            if (GF_SUCCESS == err)
            {
                err = header->result;
                send_ta_cmd_error_times = 0;
            }

            if (header->reset_flag > 0)
            {
                LOG_D(LOG_TAG, "[%s] reset_flag > 0, reset chip", __func__);
                mContext->mDevice->reset();
            }
        }
        while (0);

        LOG_D(LOG_TAG, "[%s] err = %d, errno = %s", __func__, err, gf_strerror(err));
        return err;
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

