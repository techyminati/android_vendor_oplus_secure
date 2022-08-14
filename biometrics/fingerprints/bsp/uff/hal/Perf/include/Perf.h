/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Perf/include
 **
 ** Description:
 **      Perf for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _PERF_H_
#define _PERF_H_

#include <stddef.h>
#include <stdint.h>
#include "Handler.h"

typedef enum perf_event {
    PERF_SET_ACTION = 1,
    PERF_BIND_BIGCORE_BY_PID,
    PERF_BIND_BIGCORE_BY_TID,
    PERF_SET_UXTHREAD,
} perf_event_t;

typedef struct perf_action {
    perf_event_t event;
    int orms_type;
    int orms_timeout;
    int bind_pid;
    int bind_tid;
    int ux_enable;
} perf_action_t;

namespace android {
class Perf : public HandlerCallback {
public:
    static Perf* getInstance() {
        if (sInstance == NULL) {
            sInstance = new Perf();
        }
        return sInstance;
    }
    virtual void handleMessage(FpMessage msg);
    void improvePerf(perf_action_t action);

private:
    Perf();
    virtual ~Perf();
    void setAction(int action, int timeout);
    void bind_big_core(void);
    void bind_big_core_bytid(int tid);
    void setUxthread(int pid, int tid, int enable);
    static Perf* sInstance;
    sp<Handler> mPerfHandler = nullptr;
#ifdef FP_HYPNUSD_ENABLE
    OrmsHalClient*  ormsHalClient = NULL;
    std::string     identity      = "biometrics.fingerprint";
    struct sa_param mSa;
#endif
};
}  // namespace android
#endif /* _PERF_H_ */
