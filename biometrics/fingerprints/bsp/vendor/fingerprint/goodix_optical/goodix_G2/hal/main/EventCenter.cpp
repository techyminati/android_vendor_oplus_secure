/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][EventCenter]"

#include <utils/Log.h>
#include "EventCenter.h"
#include "HalLog.h"
#include "HalContext.h"
#include "Sensor.h"
#include "FingerprintCore.h"

namespace goodix
{

    EventCenter::EventCenter(HalContext* context)
          : mHandler(NULL),
            mFilter(NULL),
            mContext(context),
            mDownDetected(false)
    {
        registerFilter(this);
    }

    EventCenter::~EventCenter()
    {
        VOID_FUNC_ENTER();
        {
            Mutex::Autolock _l(mMutex);
            mEventQueue.clear();
            Thread::requestExit();
            mCond.signal();
            mHandler = nullptr;
            mFilter = nullptr;
            mContext = nullptr;
        }
        Thread::join();
        VOID_FUNC_EXIT();
    }

    gf_error_t EventCenter::init()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        if (!isRunning())
        {
            mEventQueue.clear();
            run("EventCenterThread");
        }
        FUNC_EXIT(err);
        return err;
    }

    void EventCenter::dispatchEvent(gf_event_type_t e)
    {
        VOID_FUNC_ENTER();
        do
        {
            if (nullptr == mContext)
            {
                LOG_E(LOG_TAG, "[%s] mContext is NULL", __func__);
                break;
            }
            Mutex::Autolock _l(mContext->mHalLock);

            if (EVENT_SCREEN_ON == e)
            {
                if (nullptr != mContext->mFingerprintCore)
                {
                    mContext->mFingerprintCore->screenState(1);
                }
                // handler not handle the message
                break;
            }
            else if (EVENT_SCREEN_OFF == e)
            {
                if (nullptr != mContext->mFingerprintCore)
                {
                    mContext->mFingerprintCore->screenState(0);
                }
                // handler not handle the message
                break;
            }
            else if (EVENT_IRQ == e)
            {
                if (nullptr != mContext->mSensor)
                {
                    e = mContext->mSensor->getIrqEventType();
                }
            }
            if (nullptr != mHandler)
            {
                LOG_E(LOG_TAG, "[%s] mHandler is NULL", __func__);
                mHandler->onEvent(e);
            }
        }
        while (0);
        VOID_FUNC_EXIT();
    }

    gf_error_t EventCenter::postEvent(gf_event_type_t e)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            Mutex::Autolock _l(mMutex);
            if (mFilter == NULL || mFilter->isValidEvent(e))
            {
                LOG_D(LOG_TAG, "[%s] enqueue event<%d>", __func__, e);
                bool empty = mEventQueue.isEmpty();
                mEventQueue.enqueue(e);
                if (empty)
                {
                    mCond.signal();
                }
            }
            else
            {
                LOG_D(LOG_TAG, "[%s] event<%d> is filtered", __func__, e);
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t EventCenter::registerHandler(IEventHandler* handler)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            GF_NULL_BREAK(handler, err);
            Mutex::Autolock _l(mMutex);
            mHandler = handler;
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t EventCenter::deregisterHandler(IEventHandler* handler)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            GF_NULL_BREAK(handler, err);
            Mutex::Autolock _l(mMutex);
            if (handler == mHandler)
            {
                mHandler = NULL;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t EventCenter::registerFilter(IEventFilter* filter)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            GF_NULL_BREAK(filter, err);
            Mutex::Autolock _l(mMutex);
            mFilter = filter;
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t EventCenter::deregisterFilter(IEventFilter* filter)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            GF_NULL_BREAK(filter, err);
            Mutex::Autolock _l(mMutex);
            if (filter == mFilter)
            {
                mFilter = this;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    bool EventCenter::isValidEvent(gf_event_type_t e)
    {
        bool valid = true;
        VOID_FUNC_ENTER();
        if (EVENT_FINGER_DOWN == e)
        {
            if (hasDownEvt())
            {
                valid  = false;
            }
            else
            {
                mDownDetected = true;
            }
        }
        else if (EVENT_FINGER_UP == e)
        {
            if (!mDownDetected)
            {
                valid = false;
            }
            else
            {
                mDownDetected = false;
            }
        }
        VOID_FUNC_EXIT();
        return valid;
    }

    bool EventCenter::hasUpEvt()
    {
        return mEventQueue.contains(EVENT_FINGER_UP);
    }

    bool EventCenter::hasDownEvt()
    {
        return mEventQueue.contains(EVENT_FINGER_DOWN);
    }


    bool EventCenter::threadLoop()
    {
        while (!Thread::exitPending())
        {
            gf_event_type_t e = EVENT_UNKNOWN;
            {
                LOG_D(LOG_TAG, "[%s] Wait get event", __func__);
                Mutex::Autolock _l(mMutex);
                while (mEventQueue.isEmpty() && !Thread::exitPending())
                {
                    mCond.wait(mMutex);
                }
                if (!mEventQueue.isEmpty() && !Thread::exitPending())
                {
                    e = mEventQueue.nextEvent();
                }
                LOG_D(LOG_TAG, "[%s] Get event %d end", __func__, e);
            }
            if (e != EVENT_UNKNOWN)
            {
                dispatchEvent(e);
            }
        }
        return false;
    }

    EventCenter::EventQueue::EventQueue()
    {
    }

    EventCenter::EventQueue::~EventQueue()
    {
        mList.clear();
    }

    void EventCenter::EventQueue::clear()
    {
        mList.clear();
    }

    gf_error_t EventCenter::EventQueue::enqueue(gf_event_type_t event)
    {
        mList.push_back(event);
        return GF_SUCCESS;
    }

    gf_error_t EventCenter::EventQueue::dequeue(gf_event_type_t event)
    {
        UNUSED_VAR(event);
        return GF_SUCCESS;
    }

    gf_event_type_t EventCenter::EventQueue::nextEvent()
    {
        gf_event_type_t e = EVENT_UNKNOWN;
        if (mList.size() != 0)
        {
            e = mList[0];
            mList.erase(mList.begin());
        }
        return e;
    }

    bool EventCenter::EventQueue::contains(gf_event_type_t event)
    {
        bool contains = false;

        Vector<gf_event_type_t>::iterator it;
        for (it = mList.begin(); it != mList.end(); it++)
        {
            if (event == *it)
            {
                contains = true;
                break;
            }
        }
        return contains;
    }

    bool EventCenter::EventQueue::isEmpty()
    {
        return mList.empty();
    }
}   // namespace goodix
