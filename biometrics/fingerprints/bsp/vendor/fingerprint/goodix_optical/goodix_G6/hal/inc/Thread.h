/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>
#include <utils/ThreadDefs.h>
#include "Condition.h"
#include "Mutex.h"

using  namespace android;

namespace goodix
{
    class Thread
    {
    public:
        Thread();
        virtual ~Thread();
        bool run(const char* name, int32_t priority = PRIORITY_DEFAULT, uint32_t stack = 0);
        void requestExit();
        void requestExitAndWait();
        bool join();
        bool isRunning();
        bool exitPending();

    protected:
        virtual bool threadLoop() = 0;

    private:
        Thread& operator=(const Thread&);
        static void* sthreadLoop(void* user);
        mutable Mutex mLock;
        Condition mThreadExitedCondition;
        volatile bool mExitPending;
        volatile bool mRunning;
        uint32_t mThreadId;
    };
}  // namespace goodix




#endif /* _THREAD_H_ */
