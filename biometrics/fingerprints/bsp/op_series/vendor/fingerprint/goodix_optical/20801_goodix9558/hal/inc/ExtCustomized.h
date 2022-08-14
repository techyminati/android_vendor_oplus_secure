/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _EXTCUSTOMIZED_H_
#define _EXTCUSTOMIZED_H_

#include "GoodixFingerprint.h"
#include "ExtModuleBase.h"
#include "HalBase.h"
#include "EventCenter.h"

namespace goodix {
    class HalContext;

    class ExtCustomized : public ExtModuleBase, public HalBase, public IEventHandler {
    public:
        explicit ExtCustomized(HalContext *context);
        virtual ~ExtCustomized();
        gf_error_t onCommand(int32_t cmdId, const int8_t *in, uint32_t inLen,
                             int8_t **out, uint32_t *outLen);

        // overide IEventHandler
        virtual gf_error_t onEvent(gf_event_type_t e);
    protected:
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen,
                                          int8_t **out, uint32_t *outLen);
    };

    // The interface is implemented by project
    ExtCustomized *createExtCustomized(HalContext *context);
}  // namespace goodix


#endif /* _EXTCUSTOMIZED_H_ */
