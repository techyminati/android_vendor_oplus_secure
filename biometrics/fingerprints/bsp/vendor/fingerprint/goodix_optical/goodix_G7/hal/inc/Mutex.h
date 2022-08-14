/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <utils/Timers.h>

namespace goodix {

    class Condition;

    /*
     * Simple mutex class.  The implementation is system-dependent.
     *
     * Note: Goodix implemented Mutex is recursive, it is different from Android implementation.
     * The mutex must be unlocked by the thread that locked it.  They are
     * recursive, i.e. the same thread can lock it multiple times.
     */
    class Mutex {
    public:
        enum {
            PRIVATE = 0,
            SHARED = 1
        };
        Mutex();
        explicit Mutex(const char *name);
        explicit Mutex(int32_t type, const char *name);
        ~Mutex();
        // lock or unlock the mutex
        int32_t lock();
        void unlock();
        // lock if possible; returns 0 on success, error otherwise
        int32_t tryLock();
        int32_t timedLock(nsecs_t timeoutNs);
        // Manages the mutex automatically. It'll be locked when Autolock is
        // constructed and released when Autolock goes out of scope.
        class Autolock {
        public:
            inline explicit Autolock(Mutex &mutex) :
                mLock(mutex) {
                mLock.lock();
            }
            inline explicit Autolock(Mutex *mutex) :
                mLock(*mutex) {
                mLock.lock();
            }
            inline ~Autolock() {
                mLock.unlock();
            }
        private:
            Mutex &mLock;
        };

    private:
        friend class Condition;
        void initRecursiveMutex();
        // A mutex cannot be copied
        Mutex(const Mutex &);
        Mutex &operator =(const Mutex &);
        pthread_mutex_t mMutex;
    };

    // ---------------------------------------------------------------------------
    inline Mutex::Mutex() {
        initRecursiveMutex();
    }

    inline Mutex::Mutex(__attribute__((unused)) const char *name) {
        initRecursiveMutex();
    }

    inline Mutex::Mutex(int32_t type, __attribute__((unused)) const char *name) {
        if (type == SHARED) {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&mMutex, &attr);
            pthread_mutexattr_destroy(&attr);
        } else {
            initRecursiveMutex();
        }
    }

    inline void Mutex::initRecursiveMutex() {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    inline Mutex::~Mutex() {
        pthread_mutex_destroy(&mMutex);
    }

    inline int32_t Mutex::lock() {
        return -pthread_mutex_lock(&mMutex);
    }

    inline void Mutex::unlock() {
        pthread_mutex_unlock(&mMutex);
    }

    inline int32_t Mutex::tryLock() {
        return -pthread_mutex_trylock(&mMutex);
    }

    inline int32_t Mutex::timedLock(nsecs_t timeoutNs) {
        timeoutNs += systemTime(SYSTEM_TIME_REALTIME);
        const struct timespec ts = {
            static_cast<time_t>(timeoutNs / 1000000000),  // .tv_sec
                static_cast<int32_t>(timeoutNs % 1000000000),  // .tv_nsec
        };
        return -pthread_mutex_timedlock(&mMutex, &ts);
    }
    // ---------------------------------------------------------------------------
    /*
     * Automatic mutex.  Declare one of these at the top of a function.
     * When the function returns, it will go out of scope, and release the
     * mutex.
     */
    typedef Mutex::Autolock AutoMutex;
};  // namespace goodix

#endif /* _MUTEX_H_ */
