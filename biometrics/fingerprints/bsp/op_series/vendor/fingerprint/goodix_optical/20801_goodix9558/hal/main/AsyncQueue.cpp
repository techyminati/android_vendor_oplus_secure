/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[AsyncQueue]"

#include "HalLog.h"
#include "AsyncQueue.h"

namespace goodix {
    AsyncQueue::AsyncQueue(const char *name) :
        mpName(name) {
    }

    AsyncQueue::~AsyncQueue() {
        mQueue.clear();
    }

    gf_error_t AsyncQueue::start() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (!isRunning()) {
            mQueue.clear();
            run(mpName);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t AsyncQueue::stop() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        Mutex::Autolock _l(mMutex);
        mQueue.clear();
        requestExit();
        mCond.signal();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t AsyncQueue::postAsyncMessage(AsyncMessage *message) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            Mutex::Autolock _l(mMutex);

            if (nullptr == message) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            bool empty = mQueue.isEmpty();
            LOG_D(LOG_TAG, "[%s] enqueue message", __func__);
            mQueue.push_back(message);

            if (empty) {
                mCond.signal();
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    bool AsyncQueue::threadLoop() {
        while (!Thread::exitPending()) {
            AsyncMessage *e = nullptr;
            {  // NOLINT(660)
                LOG_D(LOG_TAG, "[%s] Wait get message", __func__);
                Mutex::Autolock _l(mMutex);

                while (mQueue.isEmpty() && !Thread::exitPending()) {
                    mCond.wait(mMutex);
                }

                if (!mQueue.isEmpty() && !Thread::exitPending()) {
                    e = mQueue[0];
                    mQueue.erase(mQueue.begin());
                }

                LOG_D(LOG_TAG, "[%s] Get message end", __func__);
            }

            if (nullptr != e) {
                if (nullptr != e->mpHandler) {
                    e->mpHandler->doWork(e);
                }

                delete e;
            }
        }

        return false;
    }
}  // namespace goodix
