/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "Thread.h"
#include "HalLog.h"

#define LOG_TAG "[GF_HAL][Thread]"

typedef void *(*android_pthread_entry)(void *);

#ifndef UNUSED_VAR
#define UNUSED_VAR(X)   ((void)(X))
#endif  // UNUSED_VAR

namespace goodix {

    static bool createThread(android_pthread_entry entryFunction, void *userData,
                             const char *name,
                             int32_t priority, uint32_t stack, uint32_t *threadId) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        UNUSED_VAR(priority);
        UNUSED_VAR(name);

        if (stack > 0) {
            pthread_attr_setstacksize(&attr, stack);
        }

        pthread_t thread;
        int32_t result = pthread_create(&thread, &attr, entryFunction, userData);
        pthread_attr_destroy(&attr);

        if (result != 0) {
            LOG_E(LOG_TAG,
                  "[%s] create thread failed, result = %d, priority = %d, stack = %d.",
                  __func__, result, priority, stack);
            return false;
        }

        *threadId = (uint32_t) thread;
        LOG_D(LOG_TAG, "[%s] Thread id = %u", __func__, *threadId);
        return true;
    }

    Thread::Thread() :
        mExitPending(false), mRunning(false), mThreadId(-1) {
    }

    Thread::~Thread() {
    }

    void *Thread::sthreadLoop(void *user) {
        Thread *self = static_cast<Thread *>(user);
        bool result = false;

        while (true) {
            result = self->threadLoop();
            {  // NOLINT(660)
                AutoMutex _l(self->mLock);

                if (!result || self->mExitPending) {
                    self->mExitPending = true;
                    self->mRunning = false;
                    self->mThreadId = -1;
                    self->mThreadExitedCondition.broadcast();
                    break;
                }
            }
        }
        LOG_D(LOG_TAG, "[%s] EIXT Thread", __func__);
        return 0;
    }

    bool Thread::run(const char *name, int32_t priority, uint32_t stack) {
        AutoMutex _l(mLock);

        if (mRunning) {
            return false;
        }

        mExitPending = false;
        mRunning = true;
        bool ret = createThread(sthreadLoop, this, name, priority, stack, &mThreadId);

        if (!ret) {
            mRunning = false;
        }

        return ret;
    }

    void Thread::requestExit() {
        AutoMutex _l(mLock);
        mExitPending = true;
    }

    void Thread::requestExitAndWait() {
        AutoMutex _l(mLock);
        mExitPending = true;
        uint32_t tId = pthread_self();

        if (tId == mThreadId) {
            LOG_E(LOG_TAG, "[%s]Thread (this=%p): don't call wait exit() from this "
                  "Thread object's thread. It's a guaranteed deadlock!", __func__, this);
            return;
        }

        while (mRunning == true) {
            mThreadExitedCondition.wait(mLock);
        }
    }

    bool Thread::join() {
        AutoMutex _l(mLock);
        uint32_t tId = pthread_self();

        if (tId == mThreadId) {
            LOG_E(LOG_TAG, "[%s]Thread (this=%p): don't call join() from this "
                  "Thread object's thread. It's a guaranteed deadlock!", __func__, this);
            return false;
        }

        while (mRunning) {
            mThreadExitedCondition.wait(mLock);
        }

        return true;
    }

    bool Thread::exitPending() {
        AutoMutex _l(mLock);
        return mExitPending;
    }

    bool Thread::isRunning() {
        AutoMutex _l(mLock);
        return mRunning;
    }
}  // namespace goodix

