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
        virtual gf_error_t open();
        virtual  void startSleepTimer(uint32_t timeoutSec);
        virtual void startKeyTimer(uint32_t timeoutSec);

        enum CUSTOMIZED_NETLINK_CMD {
            GF_NETLINK_TP_TOUCHDOWN = GF_NETLINK_SCREEN_ON + 1,
            GF_NETLINK_TP_TOUCHUP,
            GF_NETLINK_UI_READY,
            GF_NETLINK_UI_DISAPPEAR,
        };


        typedef enum GF_NAV_CODE {  // 20180917 add for quick pay
            GF_NAV_NONE = 0,
            GF_NAV_FINGER_UP,
            GF_NAV_FINGER_DOWN,
            GF_NAV_UP,
            GF_NAV_DOWN,
            GF_NAV_LEFT,
            GF_NAV_RIGHT,
            GF_NAV_CLICK,
            GF_NAV_HEAVY,
            GF_NAV_LONG_PRESS,
            GF_NAV_DOUBLE_CLICK,
            /*liuyan 2017/8/4 add for quick pay*/
            GF_NAV_F2,
            GF_NAV_MAX,
        } gf_nav_code_t;

        struct FP_UNDERSCREEN_INFO {
            uint8_t touch_state;
            uint8_t area_rate;
            uint16_t x;
            uint16_t y;
        };
        struct FP_UNDERSCREEN_INFO tp_info = {0, 0, 0, 0};

    protected:
        gf_event_type_t mapMsgToEvent(int32_t msg);
        // 20180917 add for quick pay
        virtual gf_error_t sendKeyEvent(gf_nav_code_t code);
        virtual void cancelKeyTimer();
        // 20180924 add for sensor sleep
        virtual gf_error_t sendSleepEvent();
        virtual  void cancelSleepTimer();
        // 20180917 add for quick pay
        Timer* pmKeyTimer = nullptr;
        static void keyTimerThread(union sigval v);
        // 20180924 add for sensor sleep
        Timer* pmSleepTimer = nullptr;
        static void sleepTimerThread(union sigval v);
        void handleNetlinkReceivedData(void* data, uint32_t len);
    };
}  // namespace goodix
#endif /* _CUSTOMIZEDDEVICE_H_ */
