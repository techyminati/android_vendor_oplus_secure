/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "Thread.h"
#include "gf_error.h"
#include "gf_base_types.h"
#include "HalBase.h"
#include "Timer.h"
#include "gf_event.h"

namespace goodix {

    enum gf_spi_speed {
        GF_SPI_SPEED_LOW = 0,
        GF_SPI_SPEED_HIGH = 1,
    };

    enum gf_tp_status
    {
        GF_TP_ENABLE = 1,
        GF_TP_DISENABLE = 0,
    };

    enum gf_lcd_status
    {
        GF_LCD_ENABLE = 1,
        GF_LCD_DISENABLE = 0,
    };

    enum gf_wakelock_status
    {
        GF_RELEASE_WAKE_LOCK = 0,
        GF_HOLD_WAKE_LOCK = 1,
    };


    enum NETLINK_CMD {
        GF_NETLINK_TEST = 0,  //
        GF_NETLINK_IRQ = 1,
        GF_NETLINK_SCREEN_OFF,
        GF_NETLINK_SCREEN_ON,
        GF_NETLINK_TP_TOUCHDOWN,
        GF_NETLINK_TP_TOUCHUP,
        GF_NETLINK_UI_READY,
        GF_NETLINK_UI_DISAPPEAR,
        GF_NETLINK_MAX
    };

    typedef enum {
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

    class Device : public HalBase {
    public:
        explicit Device(HalContext *context);
        virtual ~Device();
        virtual gf_error_t open();
        virtual void close();
        virtual gf_error_t reset();
        virtual gf_error_t enable();
        virtual gf_error_t disable();
        virtual gf_error_t controlSpiClock(uint8_t enable);
        virtual gf_error_t enablePower();
        virtual gf_error_t disablePower();
        virtual gf_error_t enableIrq();
        virtual gf_error_t disableIrq();
        virtual gf_error_t remove();
        //20180917 add for quick pay
        virtual gf_error_t sendKeyEvent(gf_nav_code_t code);
        virtual void startKeyTimer(uint32_t timeoutSec);
        virtual void cancelKeyTimer();
        //20180924 add for sensor sleep
        virtual gf_error_t sendSleepEvent();
        virtual void startSleepTimer(uint32_t timeoutSec);
        virtual void cancelSleepTimer();
        virtual gf_error_t getFirmwareInfo(uint8_t* buf);
		virtual gf_error_t enable_tp(gf_tp_status mode);
        virtual gf_error_t hold_wakelock(gf_wakelock_status mode);
        virtual gf_error_t enable_lcd(gf_lcd_status mode);
        virtual gf_error_t clear_kernel_touch_flag();
        inline void setSpiSpeed(int32_t speed) {
            mSpiSpeed = speed;
        }
        class AutoSpiClock {
        public:
            inline explicit AutoSpiClock(Device *device) : mDevice(device) {
                mDevice->controlSpiClock(1);
            }
            inline ~AutoSpiClock() {
                mDevice->controlSpiClock(0);
            }

        private:
            Device *mDevice;
        };
        class NetlinkRoutingThread : public Thread {
        public:
            explicit NetlinkRoutingThread(Device *device);
            virtual ~NetlinkRoutingThread();
        private:
            virtual bool threadLoop();
            Device *mDevice;
        };
		class FpPerfThread : public Thread {
        public:
            explicit FpPerfThread(Device *device);
            virtual ~FpPerfThread();
        private:
            virtual bool threadLoop();
            Device *mDevice;
        };

    protected:
        virtual void startNetlinkRoutingThread();
        virtual uint32_t getNetlinkMsgDataLen();
        virtual void handleNetlinkReceivedData(void* data, uint32_t len);
        virtual void handleNetlinkMessage(int32_t msg);
        virtual gf_event_type_t mapMsgToEvent(int32_t msg);
        //20180917 add for quick pay
        Timer* mKeyTimer;
        static void keyTimerThread(union sigval v);
        //20180924 add for sensor sleep
        Timer* mSleepTimer;
        static void sleepTimerThread(union sigval v);

        int32_t mFD;
        int32_t mSpiSpeed;
        int32_t mNetlinkRoute;
        NetlinkRoutingThread mThread;
		FpPerfThread fpThread;
    };
}  // namespace goodix



#endif  /* _DEVICE_H_ */
