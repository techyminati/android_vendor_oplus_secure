/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/Handler.h
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef FACE_HANDLER_H_
#define FACE_HANDLER_H_

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <utils/Looper.h>
#include <utils/String8.h>
#include <utils/Timers.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include <string>
#include "Message.h"
#include "HalLog.h"
#define MAX_SUPPORT_MESSAGE_COUNT (100)
extern bool DEBUG;
namespace android {

class HandlerCallback;
class HandlerThread;

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
    HandlerCallback() {}
    virtual ~HandlerCallback() {}

   public:
    virtual void handleMessage(FpMessage msg) = 0;
};

}  // namespace android

#endif  // FACE_HANDLER_H_
