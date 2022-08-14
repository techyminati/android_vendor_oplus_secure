/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _TRACER_H_
#define _TRACER_H_

#include <stdlib.h>
#include "Thread.h"
#include "MsgBus.h"
#include "DumpEncoder.h"
#include "HalContext.h"
#include "AsyncQueue.h"
#include "HalBase.h"

namespace goodix
{
    class Tracer : public Thread, public MsgBus::IMsgListener, public IAsyncTaskHandler,  public HalBase
    {
    public:
        static Tracer* getInstance(HalContext* context);

        ~Tracer();
        void start(void);
        void stop(void);

    protected:
        // override MsgBus::IMsgListener
        virtual gf_error_t onMessage(const MsgBus::Message& msg);
        // overide thread
        virtual bool threadLoop();
        // overide IAysncTaskHandler
        gf_error_t doWork(AsyncMessage* e);

    private:
        explicit Tracer(HalContext* context);
        gf_error_t getTraceDataFromTa(gf_data_info_t* cmd, uint8_t** data, uint32_t* data_len);

        class LogManager
        {
        public:
            LogManager();
            void startCatchQseeLog();
            bool appendLog(const char* log, uint32_t log_len);
        private:
            FILE* mLogFile;
            uint32_t mCurrentLogFileBytes;
            char mLogFileName[GF_MAX_FILE_NAME_LEN];
        };

    private:
        static Tracer* mTracer;
        AsyncQueue mAsyncQueue;
        DumpEncoder mEncoder;
        LogManager mLogManager;
    };
    gf_error_t startTracer(HalContext* context);
    void stopTracer(HalContext* context);
};  // namespace goodix

#endif   /* _TRACER_H_ */
