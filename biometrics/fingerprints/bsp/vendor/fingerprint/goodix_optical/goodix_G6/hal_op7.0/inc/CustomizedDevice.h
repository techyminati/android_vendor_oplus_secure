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

namespace goodix {
    class CustomizedDevice : public Device {
    public:
        explicit CustomizedDevice(HalContext *context);
        virtual ~CustomizedDevice();
        virtual gf_error_t open();
    protected:
        virtual uint32_t getNetlinkMsgDataLen();
        virtual void handleNetlinkReceivedData(void* data, uint32_t len);
        virtual gf_event_type_t mapMsgToEvent(int32_t msg);
    private:
        bool mEnableNetlinkChannel;
    };
}  // namespace goodix



#endif /* _CUSTOMIZEDDEVICE_H_ */
