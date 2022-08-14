/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HALDUMP_H_
#define _HALDUMP_H_

#include <utils/Vector.h>
#include <utils/Thread.h>
#include "gf_error.h"
#include "HalBase.h"
#include "MsgBus.h"
#include "DumpTypeDefine.h"
#include "AsyncQueue.h"
#include "DumpEncoder.h"
#include "Mutex.h"
#include "Condition.h"

using ::android::Thread;
using ::android::Vector;

namespace goodix {
    class HalDump: public HalBase, public MsgBus::IMsgListener, public IAsyncTaskHandler {
    public:
        explicit HalDump(HalContext *context);
        // '/' should not be followed in dumpDir tail. such as '/data' not '/data/'
        explicit HalDump(HalContext *context, const char* dumpDir);
        explicit HalDump(HalContext *context, const char* dumpDir, const char* dumpControlProperty);
        virtual ~HalDump();
        virtual gf_error_t init();
        gf_error_t reInit();
        gf_error_t postDumpMessage(AsyncMessage *e);

    protected:
        // override MsgBus::IMsgListener
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        // override IAsyncTaskHandler::doWork
        virtual gf_error_t doWork(AsyncMessage *message);
        virtual gf_error_t onDeviceInitEnd(const MsgBus::Message &msg);
        virtual gf_error_t onEnrollSaveTemplatesEnd(const MsgBus::Message &msg);
        virtual gf_error_t onEnrollStart(const MsgBus::Message &msg);
        virtual gf_error_t onEnrollEnd(const MsgBus::Message &msg);
        virtual gf_error_t onAuthSaveTemplatesEnd(const MsgBus::Message &msg);
        virtual gf_error_t onAuthenticateStart(const MsgBus::Message &msg);
        virtual gf_error_t onAuthenticateEnd(const MsgBus::Message &msg);
        virtual gf_error_t onScreenStateChange(const MsgBus::Message &msg);
        virtual DumpConfig *getDumpConfig() = 0;
        virtual gf_error_t getDumpData(gf_data_info_t *cmd, uint8_t **data,
                                       uint32_t *data_len);
        virtual gf_error_t dumpDeviceInfo(char *timestamp, uint8_t *data,
                                          uint32_t data_len);
        virtual gf_error_t dumpTemplates(char *timestamp, uint8_t *data,
                                         uint32_t data_len);
        virtual gf_error_t dumpEnrollData(char *timestamp, gf_error_t result,
                                          uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpAuthData(char *timestamp, gf_error_t result,
                                        uint32_t retry, uint8_t *data, uint32_t data_len);
        gf_error_t genResultStrForFileName(char *resultStr, uint32_t dumpOp,
                                           gf_error_t result, uint32_t retry);
        bool isDumpEnable(gf_error_t result, uint32_t dumpOp);
        bool isDumpDataTypeAllowed(uint32_t dumpOp, uint32_t dataType);

    protected:
        DumpConfig *mDumpConfig;
        const char *mDumpDataRoot;
        uint32_t mSensorCol;
        uint32_t mSensorRow;
        DumpEncoder mDumpEncoder;
        uint64_t mTimestampFingerDown;
        uint64_t mTimestampStart;
        uint64_t mTimestampEnd;
        uint32_t mScreenStatus;
        const char *mDumpControlProperty;

    private:
        AsyncQueue mAsyncQueue;
    };

    // The interface is implemented by project
    HalDump *createHalDump(HalContext *context);
}  // namespace goodix

#endif /* _HALDUMP_H_ */
