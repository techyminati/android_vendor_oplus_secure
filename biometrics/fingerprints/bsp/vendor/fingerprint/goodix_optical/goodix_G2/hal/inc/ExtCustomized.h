/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _EXTCUSTOMIZED_H_
#define _EXTCUSTOMIZED_H_


#include "fingerprint_oplus.h"
#include "HalBase.h"
#include "EventCenter.h"

namespace goodix
{

    class HalContext;

    class ExtCustomized : public HalBase, IEventHandler
    {
    public:
        // define command id
        explicit ExtCustomized(HalContext* context);
        virtual ~ExtCustomized();
        virtual gf_error_t executeCommand(uint32_t cmdId, uint8_t *in, uint32_t inLen, uint8_t **out,
                uint32_t *outLen);
        void setNotify(fingerprint_notify_t notify);
        // overide IEventHandler
        virtual gf_error_t onEvent(gf_event_type_t e);
    private:
        fingerprint_notify_t mNotify;
    };

    // The interface is implemented by project
    ExtCustomized* createExtCustomized(HalContext* context);
}  // namespace goodix


#endif /* _EXTCUSTOMIZED_H_ */
