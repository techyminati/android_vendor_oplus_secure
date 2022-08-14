/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][Timer]"

#include <string.h>
#include <errno.h>
#include "Timer.h"
#include "HalLog.h"

namespace goodix {

    Timer *Timer::createTimer(timer_thread_t timerThread, void *ptrData) {
        Timer *t = NULL;

        do {
            if (NULL == timerThread) {
                break;
            }

            t = new Timer();
            struct sigevent evp;
            memset(&evp, 0, sizeof(struct sigevent));
            evp.sigev_value.sival_ptr = ptrData;
            evp.sigev_notify = SIGEV_THREAD;
            evp.sigev_notify_function = timerThread;

            if (timer_create(CLOCK_REALTIME, &evp, &(t->mTimerId)) != 0) {
                LOG_E(LOG_TAG, "[%s] timer_create failed", __func__);
                delete t;
                t = NULL;
            } else {
                LOG_D(LOG_TAG, "[%s] create timer success, timer_id=%p", __func__, t->mTimerId);
            }
        } while (0);

        return t;
    }

    Timer::Timer() {
        mTimerId = NULL;
    }

    Timer::~Timer() {
        destroy();
    }

    void Timer::destroy() {
        LOG_D(LOG_TAG, "[%s] destroy timer_id=%p", __func__, mTimerId);

        if (timer_delete(mTimerId) != 0) {
            LOG_E(LOG_TAG, "[%s] timer_delete failed", __func__);
        }
    }

    gf_error_t Timer::set(time_t interval_second, int64_t interval_nanosecond,
                          time_t value_second,
                          int64_t value_nanosecond) {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;

        do {
            if (NULL == mTimerId) {
                LOG_E(LOG_TAG, "[%s] timer not created", __func__);
                err = GF_ERROR_HAL_GENERAL_ERROR;
                break;
            }

            struct itimerspec ts;

            memset(&ts, 0, sizeof(struct itimerspec));

            ts.it_interval.tv_sec = interval_second;

            ts.it_interval.tv_nsec = interval_nanosecond;

            ts.it_value.tv_sec = value_second;

            ts.it_value.tv_nsec = value_nanosecond;

            if (timer_settime(mTimerId, 0, &ts, NULL) != 0) {
                LOG_E(LOG_TAG, "[%s] timer_settime failed errno=%d", __func__, errno);
                destroy();
                err = GF_ERROR_HAL_GENERAL_ERROR;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
