/*
 * Copyright (C) 2013-2020, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _MSGBUS_H_
#define _MSGBUS_H_

#include <utils/Vector.h>
#include "gf_error.h"
#include "Mutex.h"

using ::android::Vector;

namespace goodix {
    class MsgBus {
    public:
        enum {
            MSG_DEVICE_INIT_END,
            MSG_HARDWARE_RESET,
            MSG_CAPTURE_IMAGE_END,
            MSG_ENROLL_REQUESTED,
            MSG_ENROLL_START,
            MSG_ENROLL_CONTINUE_CAPTURE,
            MSG_ENROLL_SAVE_TEMPLATE_END,
            MSG_ENROLL_END,
            MSG_AUTHENTICATE_REQUESTED,
            MSG_AUTHENTICATE_START,
            MSG_AUTHENTICATE_SAVE_TEMPLATE_END,
            MSG_AUTHENTICATE_ALGO_END,
            MSG_AUTHENTICATE_RETRYING,
            MSG_AUTHENTICATE_RETRY_END,
            MSG_AUTHENTICATE_END,
            MSG_WAIT_FOR_FINGER_INPUT,
            MSG_SCREEN_STATE,
            MSG_CANCELED,
            MSG_TEMPLATE_REMOVED,
            MSG_TA_DEAD,
            MSG_EVENT_QUEUED,
#ifdef SUPPORT_PERSIST_DATA_BACKUP
            MSG_PERSIST_DATA_CHANGE,
#endif  // SUPPORT_PERSIST_DATA_BACKUP
            MSG_MAX
        };

        class Message {
        public:
            int32_t msg;
            int32_t params1;
            int32_t params2;
            int32_t params3;
            void *data;
            uint32_t dataLen;
        };
        class IMsgListener {
        public:
            virtual ~IMsgListener() {
            }
            virtual gf_error_t onMessage(const Message &msg) = 0;
        };
        void addMsgListener(IMsgListener *msgListener);
        void removeMsgListener(IMsgListener *msgListener);
        void sendMessage(int32_t msg);
        void sendMessage(int32_t msg, void *data, uint32_t dataLen);
        void sendMessage(const Message &msg);

    private:
        Vector<IMsgListener *> mList;
        Mutex mMsgLock;
    };
}  // namespace goodix

#endif /* _MSGBUS_H_ */
