/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - hal/health/HealthMonitor.cpp
 **
 ** Description:
 **      HealthMonitor for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#include "HealthMonitor.h"
#include <signal.h>
#include <unistd.h>
#include <sstream>

namespace android{

// --- HealthMonitor ---
HealthMonitor* HealthMonitor::sInstance = NULL;

HealthMonitor::HealthMonitor() {
    LOG_D(LOG_TAG, "HealthMonitor create");
    mHealthMonitorHandler = new Handler("HealthMonitor", this);
}

HealthMonitor::~HealthMonitor() {
    LOG_D(LOG_TAG, "~HealthMonitor");
    mHealthMonitorHandler = NULL;
}

void HealthMonitor::start(std::string startApiName, long delay, std::string session) {
    LOG_D(LOG_TAG, "start %s, session = %s, delay = %ld", startApiName.c_str(), session.c_str(), delay);
    FpMessage msg(EVENT_LIBRARY_CALL_CHECK, session);
    mHealthMonitorHandler->sendMessageDelayed(msg, delay);
}

void HealthMonitor::stop(std::string stopApiName, std::string session) {
    if (!mHealthMonitorHandler->hasMessage(EVENT_LIBRARY_CALL_CHECK, session)) {
        LOG_D(LOG_TAG, "no message, session = %s", session.c_str());
        return;
    }
    LOG_D(LOG_TAG, "stop %s, session = %s", stopApiName.c_str(), session.c_str());
    mHealthMonitorHandler->removeMessage(EVENT_LIBRARY_CALL_CHECK, session);
}

std::string HealthMonitor::toString(long long param) {
    std::string       result;
    std::stringstream ss;
    ss << param;
    ss >> result;
    return result;
}

void HealthMonitor::handleMessage(FpMessage msg) {
    LOG_D(LOG_TAG, "handleMessage ,what = %d, session = %s", msg.what, msg.str0.c_str());
    switch (msg.what) {
        case EVENT_LIBRARY_CALL_CHECK:
            LOG_E(LOG_TAG, "Library Call Check timeout, kill myself!");
            kill(getpid(), SIGKILL);
            break;
        default:
            LOG_E(LOG_TAG, "invalid msg type: %d", msg.what);
            return;
    }
}

};  // namespace android
