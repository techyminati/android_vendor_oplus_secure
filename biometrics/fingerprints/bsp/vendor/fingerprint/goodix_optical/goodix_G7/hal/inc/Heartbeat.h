/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HEARTBEAT_H_
#define _HEARTBEAT_H_

#include "GoodixFingerprint.h"
#include "ExtModuleBase.h"
#include "HalBase.h"
#include "EventCenter.h"

namespace goodix {
    class HalContext;

    class HeartbeatRate : public ExtModuleBase, public HalBase {
    public:
        explicit HeartbeatRate(HalContext *context);
        virtual ~HeartbeatRate();
        gf_error_t onCommand(int32_t cmdId, const int8_t *in, uint32_t inLen,
                             int8_t **out, uint32_t *outLen);

    protected:
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen,
                                          int8_t **out, uint32_t *outLen);
    };

    // The interface is implemented by project
    HeartbeatRate *createHeartbeatRate(HalContext *context);
}  // namespace goodix


#endif /* _HEARTBEAT_H_  */
