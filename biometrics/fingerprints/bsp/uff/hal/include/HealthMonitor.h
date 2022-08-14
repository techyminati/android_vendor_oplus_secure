/**************************************************************************************
 ** File: - HealthMonitor.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2019-08-29
 ** Author: Ziqing.Guo@BSP.Fingerprint.Basic, Add for fingerprint helathmonitor module
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>          <data>           <desc>
 **  Ziqing.Guo       2019/08/29       create file
 ************************************************************************************/
#ifndef HEALTH_MONITOR_H_
#define HEALTH_MONITOR_H_

#include <log/log.h>
#include <string>
#include "Handler.h"
#include "Message.h"
#include "HalLog.h"

typedef enum healthMonitor_type {
    /* use for health monitor. */
    EVENT_LIBRARY_CALL_CHECK = 100,
}healthMonitor_type_t;

namespace android {
class HealthMonitor : public HandlerCallback {
   public:
    static HealthMonitor* getInstance() {
        if (sInstance == NULL) {
            sInstance = new HealthMonitor();
        }
        return sInstance;
    }

    void        start(std::string startApiName, long delay, std::string session);
    void        stop(std::string stopApiName, std::string session);
    std::string toString(long long param);

   private:
    HealthMonitor();
    virtual ~HealthMonitor();
    void                  handleMessage(FpMessage msg);
    sp<Handler>           mHealthMonitorHandler;
    static HealthMonitor* sInstance;
};

}  // namespace android

#endif  // HEALTH_MONITOR_H_
