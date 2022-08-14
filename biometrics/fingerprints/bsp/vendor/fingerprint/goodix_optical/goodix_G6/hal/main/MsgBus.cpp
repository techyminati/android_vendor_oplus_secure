/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "MsgBus.h"
#include "HalLog.h"

namespace goodix {

    void MsgBus::addMsgListener(IMsgListener *msgListener) {
        mList.push_back(msgListener);
    }

    void MsgBus::removeMsgListener(IMsgListener *msgListener) {
        Vector<IMsgListener *>::iterator it;

        for (it = mList.begin(); it != mList.end(); it++) {
            if (*it == msgListener) {
                mList.erase(it);
                break;
            }
        }
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
        Vector<IMsgListener *>::iterator it;

        for (it = mList.begin(); it != mList.end(); it++) {
            IMsgListener *listener = (*it);
            if (nullptr != listener) {
                listener->onMessage(msg);
            } else {
                LOG_E(LOG_TAG, "[%s] listener pointer is null", __func__);
            }
        }
    }

}  // namespace goodix

