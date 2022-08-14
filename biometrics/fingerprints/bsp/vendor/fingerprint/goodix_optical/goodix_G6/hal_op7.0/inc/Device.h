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
#include "gf_event.h"

namespace goodix {

    enum gf_spi_speed {
        GF_SPI_SPEED_LOW = 0,
        GF_SPI_SPEED_HIGH = 1,
    };

    enum gf_tp_status
    {
        GF_TP_DISENABLE = 0,
        GF_TP_ENABLE = 1,
        GP_TP_OTHER=2,
    };

    enum gf_lcd_status
    {
        GF_LCD_DISENABLE = 0,
        GF_LCD_ENABLE = 1,
    };

    enum gf_tp_authtype
    {
        GF_TP_KEYGUARD = 1,
        GF_TP_PAY = 2,
        GF_TP_OTHER = 3,
        GF_TP_AUTHQKTYPE = 4,
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
        GF_NETLINK_MAX
    };

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
        virtual gf_error_t getFirmwareInfo(uint8_t *buf);
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

#ifdef FP_HYPNUSD_ENABLE
        class FpPerfThread : public Thread {
        public:
            explicit FpPerfThread(Device *device);
            virtual ~FpPerfThread();
        private:
            virtual bool threadLoop();
            Device *mDevice;
        };
#endif

    protected:
        virtual void startNetlinkRoutingThread();
        virtual uint32_t getNetlinkMsgDataLen();
        virtual void handleNetlinkReceivedData(void* data, uint32_t len);
        virtual void handleNetlinkMessage(int32_t msg);
        virtual gf_event_type_t mapMsgToEvent(int32_t msg);
        int32_t mFD;
        int32_t mSpiSpeed;
        int32_t mNetlinkRoute;
        NetlinkRoutingThread mThread;
#ifdef FP_HYPNUSD_ENABLE
        FpPerfThread fpThread;
#endif
    };
}  // namespace goodix



#endif  /* _DEVICE_H_ */
