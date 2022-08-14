/**************************************************************************************
 ** File: - Handler.cpp
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

#include <inttypes.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <utils/String16.h>
#include <utils/Looper.h>
#include <utils/SystemClock.h>

#include <hardware/hardware.h>
#include <hardware/hw_auth_token.h>
#include "Handler.h"

namespace android {

// --- Handler ---

Handler::Handler(const char* name, const sp<HandlerCallback>& callback):
mHandlerCallback(callback) {
    mLooper = new Looper(false);
    mWakeupCount = 0;
    mWakeSourceName = String8("noWakeSource");
    mMessageQueue.setCapacity(MAX_SUPPORT_MESSAGE_COUNT);
    mHandlerThread = new HandlerThread(this);
    status_t result = mHandlerThread->run(name, PRIORITY_URGENT_DISPLAY);
    ALOGD("mHandlerThread run, result = %d" , result);
    if (result) {
        ALOGE("Could not start HandlerThread(%s) thread due to error %d.", name, result);
    }
    ALOGD("Handler create success");
}

Handler::~Handler() {
    ALOGD("HandlerThread destroy");
    mHandlerThread = NULL;
}

void Handler::dispatchOnce() {
    int64_t nextPollTimeoutMillis = -1;
    { // acquire lock
        AutoMutex _l(mLock);
        //size_t count = mMessageQueue.size();
        //LOGD("dispatchOnce getLock enter, wakeSource = %s, count = %d", mWakeSourceName.string(), count);
        Vector<FpMessage>::iterator it;
        int64_t now = uptimeMillis();
        for (it = mMessageQueue.begin(); it != mMessageQueue.end();) {
            if (now >= (*it).when) {
                mHandlerCallback->handleMessage(*it);
                it = mMessageQueue.erase(it);
            } else {
                if (nextPollTimeoutMillis == -1) {
                    nextPollTimeoutMillis = (*it).when - now;
                } else {
                    nextPollTimeoutMillis = (((*it).when - now) < nextPollTimeoutMillis) ? ((*it).when - now) : nextPollTimeoutMillis;
                }
                ++it;
            }
        }
    } // release lock
    mLooper->pollOnce(nextPollTimeoutMillis);
}

void Handler::wake(String8 wakeSource) {
    { // acquire lock
        AutoMutex _l(mLock);
        mWakeSourceName.setTo(wakeSource);
        mWakeupCount += 1;
    } // release lock
    mLooper->wake();
}

int Handler::findMessage(int what) {
    size_t count = mMessageQueue.size();
    //LOGD("findMessage, size = %d, what = %d", count, what);
    for (size_t index = 0; index < count; index++) {
        if (mMessageQueue[index].what == what) {
            return index;
        }
    }
    return -1;
}

int Handler::findMessage(int what, std::string str0) {
    size_t count = mMessageQueue.size();
    //LOGD("findMessage, size = %d, what = %d", count, what);
    for (size_t index = 0; index < count; index++) {
        if (mMessageQueue[index].what == what && mMessageQueue[index].str0 == str0) {
            return index;
        }
    }
    return -1;
}

bool Handler::hasMessage(int what) {
    { // acquire lock
        AutoMutex _l(mLock);
        int index = findMessage(what);
        return (index >= 0);
    } // release lock
}

bool Handler::hasMessage(int what, std::string str0) {
    { // acquire lock
        AutoMutex _l(mLock);
        int index = findMessage(what, str0);
        return (index >= 0);
    } // release lock
}

void Handler::sendMessage(int what) {
    sendMessageDelayed(what, 0);
}

void Handler::sendMessage(FpMessage msg) {
    sendMessageDelayed(msg, 0);
}

void Handler::sendMessageDelayed(int what, long delayMillis) {
    { // acquire lock
        AutoMutex _l(mLock);
        //LOGD("sendMessageDelayed, what = %d, size = %d, delayMillis = %ld", what, mMessageQueue.size(), delayMillis);
        FpMessage msg(what);
        msg.when = uptimeMillis() + delayMillis;
        size_t count = mMessageQueue.size();
        if (count <= 0) {
            mMessageQueue.push_back(msg);
        } else {
            size_t i = 0;
            while (i < count) {
                if (mMessageQueue[i].when > msg.when) {
                    mMessageQueue.insertAt(msg, i);
                    break;
                } else {
                    i++;
                }
            }
            if (i >= count) {
                mMessageQueue.push_back(msg);
            }
        }
    } // release lock
    wake(String8(__FUNCTION__));
}

void Handler::sendMessageDelayed(FpMessage msg, long delayMillis) {
    { // acquire lock
        AutoMutex _l(mLock);
        //LOGD("sendMessageDelayed, size = %d, what = %d, delayMillis = %ld", mMessageQueue.size(), msg.what, delayMillis);
        msg.when = uptimeMillis() + delayMillis;
        size_t count = mMessageQueue.size();
        if (count <= 0) {
            mMessageQueue.push_back(msg);
        } else {
            size_t i = 0;
            while (i < count) {
                if (mMessageQueue[i].when > msg.when) {
                    mMessageQueue.insertAt(msg, i);
                    break;
                } else {
                    i++;
                }
            }
            if (i >= count) {
                mMessageQueue.push_back(msg);
            }
        }
    } // release lock
    wake(String8(__FUNCTION__));
}

void Handler::removeMessage(int what) {
    {
        AutoMutex _l(mLock);// acquire lock
        int index = findMessage(what);
        if (index >= 0) {
            mMessageQueue.removeAt(index);
        }
    }//release lock
}

void Handler::removeMessage(int what, std::string str0) {
    {
        AutoMutex _l(mLock);// acquire lock
        int index = findMessage(what, str0);
        if (index >= 0) {
            mMessageQueue.removeAt(index);
        }
    }// release lock
}

void Handler::clear() {
    { // acquire lock
        AutoMutex _l(mLock);
        mMessageQueue.clear();
    }// release lock
}

// --- HandlerThread ---

HandlerThread::HandlerThread(const sp<Handler>& handler) :
        Thread(/*canCallJava*/ true), mHandler(handler) {
    ALOGD("HandlerThread create");
}

HandlerThread::~HandlerThread() {
}

bool HandlerThread::threadLoop() {
    mHandler->dispatchOnce();
    return true;
}
}; // namespace android
