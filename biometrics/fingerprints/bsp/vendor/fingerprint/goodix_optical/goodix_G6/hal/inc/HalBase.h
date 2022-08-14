/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HALBASE_H_
#define _HALBASE_H_

#include "gf_base_types.h"
#include "MsgBus.h"

namespace goodix {

    class HalContext;

    class HalBase {
    public:
        explicit HalBase(HalContext *context);
        ~HalBase();
    protected:
        gf_error_t invokeCommand(void *cmd, int32_t size);
        void sendMessage(int32_t msg);
        void sendMessage(int32_t msg, void *data, uint32_t dataLen);
        void sendMessage(int32_t msg, int32_t params1);
        void sendMessage(int32_t msg, int32_t params1, int32_t params2);
        void sendMessage(int32_t msg, int32_t params1, int32_t params2,
                         int32_t params3);
        HalContext *mContext;
    private:
        HalBase(const HalBase &);
        HalBase &operator = (const HalBase &);
    };

}  // namespace goodix


#endif /* _HALBASE_H_ */
