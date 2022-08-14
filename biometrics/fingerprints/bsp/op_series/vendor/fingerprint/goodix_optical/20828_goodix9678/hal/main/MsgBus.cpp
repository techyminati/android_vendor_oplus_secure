/*
 * Copyright (C) 2013-2020, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][MsgBus]"

#include "MsgBus.h"
#include "HalLog.h"
#include "Mutex.h"

namespace goodix {

    void MsgBus::addMsgListener(IMsgListener *msgListener) {
        FUNC_ENTER();
        Mutex::Autolock _l(mMsgLock);
        mList.push_back(msgListener);
        VOID_FUNC_EXIT();
    }

    void MsgBus::removeMsgListener(IMsgListener *msgListener) {
        VOID_FUNC_ENTER();
        Mutex::Autolock _l(mMsgLock);
        Vector<IMsgListener *>::iterator it;

        for (it = mList.begin(); it != mList.end(); it++) {
            if (*it == msgListener) {
                mList.erase(it);
                break;
            }
        }
        VOID_FUNC_EXIT();
    }

    void MsgBus::sendMessage(int32_t msg, void *data, uint32_t dataLen) {
        Message message = { 0 };
        message.msg = msg;
        message.data = data;
        message.dataLen = dataLen;
        sendMessage(message);
    }

    void MsgBus::sendMessage(int32_t msg) {
        Message message = { 0 };
        message.msg = msg;
        sendMessage(message);
    }

    void MsgBus::sendMessage(const Message &msg) {
        VOID_FUNC_ENTER();
        Mutex::Autolock _l(mMsgLock);
        Vector<IMsgListener *>::iterator it;

        for (it = mList.begin(); it != mList.end(); it++) {
            IMsgListener *listener = (*it);
            if (nullptr != listener) {
                listener->onMessage(msg);
            } else {
                LOG_E(LOG_TAG, "[%s] listener pointer is null", __func__);
            }
        }
        VOID_FUNC_EXIT();
    }

}  // namespace goodix

