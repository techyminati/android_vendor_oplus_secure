/**************************************************************************************
 ** File: - Handler.h
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

#ifndef FACE_HANDLER_H_
#define FACE_HANDLER_H_

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <utils/Timers.h>
#include <utils/String8.h>
#include <utils/Looper.h>
#include <utils/threads.h>
#include <utils/Vector.h>
#include <string>

#include "FpMessage.h"
#define MAX_SUPPORT_MESSAGE_COUNT (100)
extern bool DEBUG;
namespace android {

class HandlerCallback;
class HandlerThread;

/*
 * Constants of  Handler Support tyeps events.
 */
enum {
    EVENT_INIT = 101,

    /* use for health monitor. */
    EVENT_LIBRARY_CALL_CHECK = 102,
};

/* Handler */
class Handler : public virtual RefBase {
protected:
    virtual ~Handler();

public:
    explicit Handler(const char* name, const sp<HandlerCallback>& callback);
    virtual void dispatchOnce();
    virtual void wake(String8 wakeSource);
    virtual bool hasMessage(int what);
    virtual bool hasMessage(int what, std::string str0);
    virtual void sendMessage(int what);
    virtual void sendMessage(FpMessage msg);
    virtual void sendMessageDelayed(int what, long delayMillis);
    virtual void sendMessageDelayed(FpMessage msg, long delayMillis);
    virtual void removeMessage(int what);
    virtual void removeMessage(int what, std::string str0);
    virtual void clear();

private:
    virtual int findMessage(int what);
    virtual int findMessage(int what, std::string str0);
    Mutex mLock;
    sp<Looper> mLooper;
    String8 mWakeSourceName;
    int32_t mWakeupCount;

    sp<HandlerThread> mHandlerThread;
    sp<HandlerCallback> mHandlerCallback;
    Vector<FpMessage> mMessageQueue;
};

/* HandlerThread. */
class HandlerThread : public Thread {
protected:
    virtual ~HandlerThread();

public:
    explicit HandlerThread(const sp<Handler>& handler);

private:
    virtual bool threadLoop();
    sp<Handler> mHandler;
};

/*
 * The callback used by the Handler to notify the about new msg event.
 */
class HandlerCallback : public virtual RefBase {
protected:
    HandlerCallback() { }
    virtual ~HandlerCallback() { }

public:
    virtual void handleMessage(FpMessage msg) = 0;
};

} // namespace android

#endif // FACE_HANDLER_H_
