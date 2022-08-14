/*
 * Copyright (C) 2020, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _DATASYNCHRONIZER_H_
#define _DATASYNCHRONIZER_H_
#include "gf_error.h"
#include "gf_base_types.h"
#include "HalBase.h"
#include "AsyncQueue.h"

namespace goodix {
    class DataSynchronizer : public HalBase, public MsgBus::IMsgListener, public IAsyncTaskHandler {
    public:
        explicit DataSynchronizer(HalContext *context);
        explicit DataSynchronizer(HalContext *context, char* saveDir);
        virtual ~DataSynchronizer();
        gf_error_t init();
        gf_error_t postDataSyncMessage(AsyncMessage *e);
        gf_error_t getDDIC(int8_t *buf, int32_t buf_size, int32_t *len);

    protected:
        // override MsgBus::IMsgListener
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        // override IAsyncTaskHandler::doWork
        virtual gf_error_t doWork(AsyncMessage *message);

    private:
        gf_error_t injectFile(char *file_name);
        gf_error_t listFilesAndInject();
        gf_error_t onPersistDataChange();
        char *mDataRoot;
        AsyncQueue mAsyncQueue;
        bool mIsMtCaliSave;
    };
}  // namespace goodix

#endif /* _DATASYNCHRONIZER_H_ */
