/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <utils/Timers.h>
#include "Mutex.h"

namespace goodix
{

    /*
     * Condition variable class.  The implementation is system-dependent.
     *
     * Condition variables are paired up with mutexes.  Lock the mutex,
     * call wait(), then either re-wait() if things aren't quite what you want,
     * or unlock the mutex and continue.  All threads calling wait() must
     * use the same mutex for a given Condition.
     *
     * On Android and Apple platforms, these are implemented as a simple wrapper
     * around pthread condition variables.  Care must be taken to abide by
     * the pthreads semantics, in particular, a boolean predicate must
     * be re-evaluated after a wake-up, as spurious wake-ups may happen.
     */
    class Condition
    {
    public:
        enum
        {
            PRIVATE = 0,
            SHARED = 1
        };

        enum WakeUpType
        {
            WAKE_UP_ONE = 0,
            WAKE_UP_ALL = 1
        };

        Condition();
        explicit Condition(int32_t type);
        ~Condition();
        // Wait on the condition variable.  Lock the mutex before calling.
        // Note that spurious wake-ups may happen.
        int32_t wait(Mutex& mutex);
        // same with relative timeout
        int32_t waitRelative(Mutex& mutex, nsecs_t reltime);
        // Signal the condition variable, allowing one thread to continue.
        void signal();
        // Signal the condition variable, allowing one or all threads to continue.
        void signal(WakeUpType type)
        {
            if (type == WAKE_UP_ONE)
            {
                signal();
            }
            else
            {
                broadcast();
            }
        }
        // Signal the condition variable, allowing all threads to continue.
        void broadcast();

    private:
        pthread_cond_t mCond;
    };

    // ---------------------------------------------------------------------------
    inline Condition::Condition() :
                Condition(PRIVATE)
    {
    }

    inline Condition::Condition(int32_t type)
    {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
#if defined(__linux__)
        pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
#endif  // defined(__linux__)

        if (type == SHARED)
        {
            pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        }

        pthread_cond_init(&mCond, &attr);
        pthread_condattr_destroy(&attr);
    }
    inline Condition::~Condition()
    {
        pthread_cond_destroy(&mCond);
    }

    inline int32_t Condition::wait(Mutex& mutex)
    {
        return -pthread_cond_wait(&mCond, &mutex.mMutex);
    }

    inline int32_t Condition::waitRelative(Mutex& mutex, nsecs_t reltime)
    {
        struct timespec ts;
#if defined(__linux__)
        clock_gettime(CLOCK_MONOTONIC, &ts);
#else  // defined(__linux__)
        // Apple doesn't support POSIX clocks.
        struct timeval t;
        gettimeofday(&t, NULL);
        ts.tv_sec = t.tv_sec;
        ts.tv_nsec = t.tv_usec * 1000;
#endif  // defined(__linux__)

        // On 32-bit devices, tv_sec is 32-bit, but `reltime` is 64-bit.
        int64_t reltime_sec = reltime / 1000000000;

        ts.tv_nsec += static_cast<int64_t>(reltime % 1000000000);
        if (reltime_sec < INT64_MAX && ts.tv_nsec >= 1000000000)
        {
            ts.tv_nsec -= 1000000000;
            ++reltime_sec;
        }

        int64_t time_sec = ts.tv_sec;
        if (time_sec > INT64_MAX - reltime_sec)
        {
            time_sec = INT64_MAX;
        }
        else
        {
            time_sec += reltime_sec;
        }

        ts.tv_sec = (time_sec > LONG_MAX) ? LONG_MAX : static_cast<int64_t>(time_sec);

        return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);
    }

    inline void Condition::signal()
    {
        pthread_cond_signal(&mCond);
    }

    inline void Condition::broadcast()
    {
        pthread_cond_broadcast(&mCond);
    }
};  // namespace goodix

#endif /* _Condition_H_ */
