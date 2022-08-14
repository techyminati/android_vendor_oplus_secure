/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _TIMER_H_
#define _TIMER_H_

#include <signal.h>
#include <stdint.h>
#include <time.h>
#include "gf_error.h"

namespace goodix {

    typedef void (*timer_thread_t)(union sigval v);

    class Timer {
    public:
        static Timer *createTimer(timer_thread_t timerThread, void *ptrData);
        virtual ~Timer();
        gf_error_t set(time_t interval_second, int64_t interval_nanosecond,
                       time_t value_second, int64_t value_nanosecond);
    private:
        Timer();
        void destroy();
        timer_t mTimerId;
    };

}  // namespace goodix

#endif /* _TIMER_H_ */
