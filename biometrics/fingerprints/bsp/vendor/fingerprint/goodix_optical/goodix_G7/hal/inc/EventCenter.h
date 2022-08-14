/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _EVENTCENTER_H_
#define _EVENTCENTER_H_

#include <stdlib.h>
#include <utils/Vector.h>
#include "Thread.h"
#include "gf_error.h"
#include "gf_event.h"
#include "Mutex.h"
#include "Condition.h"

using ::android::Vector;

namespace goodix {
    class HalContext;

    class IEventHandler {
    public:
        virtual ~IEventHandler() {
        }
        virtual gf_error_t onEvent(gf_event_type_t e) = 0;
    };

    class IEventFilter {
    public:
        virtual ~IEventFilter() {
        }
        virtual bool isValidEvent(gf_event_type_t e) = 0;
    };

    class EventCenter : public Thread, public IEventFilter {
    public:
        explicit EventCenter(HalContext *context);
        ~EventCenter();
        gf_error_t init();
        gf_error_t postEvent(gf_event_type_t e);
        gf_error_t registerHandler(IEventHandler *handler);
        gf_error_t deregisterHandler(IEventHandler *handler);
        gf_error_t registerFilter(IEventFilter *filter);
        gf_error_t deregisterFilter(IEventFilter *filter);
        bool hasUpEvt();
        bool hasDownEvt();
        virtual bool isValidEvent(gf_event_type_t e);
        // overide thread
        virtual bool threadLoop();

    private:
        class EventQueue {
        public:
            EventQueue();
            ~EventQueue();
            gf_error_t enqueue(gf_event_type_t event);
            gf_error_t dequeue(gf_event_type_t event);
            gf_event_type_t nextEvent();
            void clear();
            bool contains(gf_event_type_t event);
            bool isEmpty();
        private:
            Vector<gf_event_type_t> mList;
        };
        void dispatchEvent(gf_event_type_t e);
        IEventHandler *mHandler;
        IEventFilter *mFilter;
        EventQueue mEventQueue;
        Mutex mQueueMutex;
        Mutex mHandlerMutex;
        Condition mCond;
        HalContext *mContext;
        bool mDownDetected;
    };

}  // namespace goodix
#endif  // _EVENTCENTER_H_
