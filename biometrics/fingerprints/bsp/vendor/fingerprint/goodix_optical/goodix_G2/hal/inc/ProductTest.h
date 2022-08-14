/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _PRODUCTTEST_H_
#define _PRODUCTTEST_H_

#include "HalBase.h"
#include "MsgBus.h"

namespace goodix
{
    typedef void (*product_test_callback)(int64_t devId, int32_t msgId, int32_t cmdId, const int8_t* data, uint32_t len);

    class HalContext;

    class ProductTest : public HalBase, public MsgBus::IMsgListener
    {
    public:
        explicit ProductTest(HalContext* contex);
        virtual ~ProductTest();
        gf_error_t onCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);
        void setNotify(product_test_callback notify);
        // override IMsgListener
        virtual gf_error_t onMessage(const MsgBus::Message& msg);
        virtual void initializePreviewWindowSize();
    protected:
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);
        virtual gf_error_t getConfig(int8_t** cfgBuf, uint32_t* bufLen) = 0;
        virtual void notifyTestCmd(int64_t devId, int32_t cmdId, const int8_t *result, int32_t resultLen);
        virtual void handleEnrollAuthEndMessage();
        virtual void handleWaitForFingerInputMessage();
        virtual void handleCanceledMessage();
        virtual bool isNeedLock(int32_t cmdId)
        {
            UNUSED_VAR(cmdId);
            return true;
        }
        product_test_callback mpNotify;
    };

    // The interface is implemented by project
    ProductTest* createProductTest(HalContext* context);
}  // namespace goodix


#endif /* _PRODUCTTEST_H_ */
