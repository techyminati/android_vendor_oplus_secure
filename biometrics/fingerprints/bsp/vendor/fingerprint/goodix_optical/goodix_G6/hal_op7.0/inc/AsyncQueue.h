/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _ASYNCQUEUE_H_
#define _ASYNCQUEUE_H_

#include <utils/Vector.h>
#include "Thread.h"
#include "gf_error.h"
#include "HalUtils.h"
#include "Mutex.h"
#include "Condition.h"

using ::android::Vector;

namespace goodix {
    class AsyncMessage;
    class IAsyncTaskHandler {
    public:
        virtual ~IAsyncTaskHandler() {
        }
        virtual gf_error_t doWork(AsyncMessage *message) = 0;
    };

    class AsyncMessage {
    public:
        uint8_t *data;
        uint32_t dataLen;
        IAsyncTaskHandler *mpHandler;
        int32_t params1;
        int32_t params2;
        int32_t params3;
        int32_t params4;
        char timestamp[TIME_STAMP_LEN];
    };

    class AsyncQueue: public Thread {
    public:
        explicit AsyncQueue(const char *name);
        virtual ~AsyncQueue();
        gf_error_t start();
        gf_error_t stop();
        gf_error_t postAsyncMessage(AsyncMessage *message);
    protected:
        // overide thread
        virtual bool threadLoop();
    private:
        Vector<AsyncMessage *> mQueue;
        Mutex mMutex;
        Condition mCond;
        const char *mpName;
    };
}  // namespace goodix

#endif /* _ASYNCQUEUE_H_ */
