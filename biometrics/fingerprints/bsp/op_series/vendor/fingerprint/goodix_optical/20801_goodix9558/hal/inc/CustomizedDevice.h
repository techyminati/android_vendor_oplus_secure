/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CUSTOMIZEDDEVICE_H_
#define _CUSTOMIZEDDEVICE_H_

#include "Device.h"
#include "Timer.h"

namespace goodix {
    class CustomizedDevice : public Device {
    public:
        explicit CustomizedDevice(HalContext *context);
        virtual ~CustomizedDevice();

    protected:
        virtual gf_event_type_t mapMsgToEvent(int32_t msg);
    };
}  // namespace goodix

#endif /* _OPLUSDEVICE_H_ */
