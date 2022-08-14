/**************************************************************************************
 ** File: - HealthMonitor.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2019-08-29
 ** Author: Ziqing.Guo@BSP.Fingerprint.Basic, Add for fingerprint helathmonitor module
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ziqing.Guo       2019/08/29       create file
 ************************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <sstream>
#include "HealthMonitor.h"

namespace android {

// --- HealthMonitor ---
HealthMonitor* HealthMonitor::sInstance = NULL;

HealthMonitor::HealthMonitor() {
    ALOGD("HealthMonitor create");
    mHealthMonitorHandler = new Handler("HealthMonitor", this);
}

HealthMonitor::~HealthMonitor() {
    ALOGD("~HealthMonitor");
    mHealthMonitorHandler = NULL;
}

void HealthMonitor::start(std::string startApiName, long delay, std::string session) {
    ALOGD("start %s, session = %s, delay = %ld", startApiName.c_str(), session.c_str(), delay);
    FpMessage msg(EVENT_LIBRARY_CALL_CHECK, session);
    mHealthMonitorHandler->sendMessageDelayed(msg, delay);
}

void HealthMonitor::stop(std::string stopApiName, std::string session) {
    if (!mHealthMonitorHandler->hasMessage(EVENT_LIBRARY_CALL_CHECK, session)) {
        ALOGD("no message, session = %s", session.c_str());
        return;
    }
    ALOGD("stop %s, session = %s", stopApiName.c_str(), session.c_str());
    mHealthMonitorHandler->removeMessage(EVENT_LIBRARY_CALL_CHECK, session);
}

std::string HealthMonitor::toString(long long param) {
    std::string result;
    std::stringstream ss;
    ss <<  param;
    ss >> result;
    return result;
}

void HealthMonitor::handleMessage(FpMessage msg) {
    ALOGD("handleMessage ,what = %d, session = %s", msg.what, msg.str0.c_str());
    switch (msg.what) {
        case EVENT_LIBRARY_CALL_CHECK:
            ALOGE("Library Call Check timeout, kill myself!");
            kill(getpid(), SIGKILL);
            break;
        default:
            ALOGE("invalid msg type: %d", msg.what);
            return;
    }
}

}; // namespace android
