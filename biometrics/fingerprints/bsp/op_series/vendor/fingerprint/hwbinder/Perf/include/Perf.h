/**************************************************************************************
 ** File: - Perf.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2019-08-21
 ** Author: Ziqing.Guo@BSP.Fingerprint.Basic, Add for fingerprint performance module
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ziqing.Guo      2019/08/21       create file
 **  Ziqing.Guo      2019/08/22       add to distingue project
 ************************************************************************************/

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <hardware/hardware.h>
#include <string>
#include <log/log.h>
#ifdef FP_HYPNUSD_ENABLE
#include "OrmsHalClient.h"
#endif

#ifndef PERF_H_
#define PERF_H_

#define PATH_LEN 256
#define MAX_LEN 20
#define ACTION_INFO "/sys/kernel/hypnus/action_info"
#ifdef FP_HYPNUSD_ENABLE
#define ACTION_IO 15
#else
#define ACTION_IO 12
#endif
#define GPU_ACTION_IO 21
#define QCOM_OCL_ACTION_IO 27

extern bool DEBUG;
namespace android {
/* SenseTimeThreshold. */
class Hypnus : public virtual RefBase {
public:
    static Hypnus* getInstance() {
        if (sInstance == NULL) {
            sInstance = new Hypnus();
        }
        return sInstance;
    }

    void setAction(int action, int timeout);
    void bind_big_core(void);
    void bind_big_core_bytid(int tid);
    void setUxthread(int pid, int tid, uint8_t enable);
    void fp_tee_bind_core(uint8_t enable);

private:
    Hypnus();
    virtual ~Hypnus();
    static Hypnus* sInstance;
#ifdef FP_HYPNUSD_ENABLE
    OrmsHalClient* ormsHalClient = NULL;
    std::string identity = "biometrics.fingerprint";
    struct sa_param mSa;
#endif
};

} // namespace android

#define MAX_CORE_NUMBER 8

#endif // PERF_H_
