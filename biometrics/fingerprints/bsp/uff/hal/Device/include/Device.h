/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Device/include/Device.h
 **
 ** Description:
 **      Device define for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "HalContext.h"
#include "FpType.h"
#include "FingerprintMessage.h"

#define FP_ENABLE_TP_PATH "/proc/touchpanel/fp_enable"
#define FP_LCD_TOUCH_DOWN "/sys/kernel/oplus_display/notify_fppress"
#define FP_DEV_NODE "/dev/fingerprint_dev"

// for drive ioctl msg
typedef enum device_key_event {
    DEVICE_KEY_NONE = 0,
    DEVICE_KEY_HOME,
    DEVICE_KEY_POWER,
    DEVICE_KEY_MENU,
    DEVICE_KEY_BACK,
    DEVICE_KEY_CAMERA,
} device_key_event_t;

struct device_key {
    enum device_key_event key;
    uint32_t  value; /* key down = 1, key up = 0 */
};

typedef enum device_control_spi {
    DEVICE_ENABLE_SPI = 0,
    DEVICE_DISABLE_SPI = 1,
} device_control_spi_t;

typedef enum device_control_power {
    DEVICE_ENABLE_POWER = 0,
    DEVICE_DISABLE_POWER = 1,
} device_control_power_t;

typedef enum device_control_irq {
    DEVICE_ENABLE_IRQ = 0,
    DEVICE_DISABLE_IRQ = 1,
} device_control_irq_t;

typedef enum device_control_tp {
    DEVICE_DISABLE_TP = 0,
    DEVICE_ENABLE_TP = 1,
} device_control_tp_t;

typedef enum device_control_brightness {
    DEVICE_SET_LOW_BRIGHTNESS = 0,
    DEVICE_SET_MID_BRIGHTNES = 1,
    DEVICE_SET_HIGH_BRIGHTNES = 2,
} device_control_brightness_t;

typedef enum device_wakelock {
    DEVICE_RELEASE_WAKE_LOCK = 0,
    DEVICE_HOLD_WAKE_LOCK = 1,
}device_wakelock_t;

// ioctl function
/************************************************/
#define FP_IOC_MAGIC 'O'  // define magic number
#define FP_IOC_INIT _IOR(FP_IOC_MAGIC, 0, uint8_t)
#define FP_IOC_EXIT _IO(FP_IOC_MAGIC, 1)
#define FP_IOC_RESET _IO(FP_IOC_MAGIC, 2)
#define FP_IOC_ENABLE_IRQ _IO(FP_IOC_MAGIC, 3)
#define FP_IOC_DISABLE_IRQ _IO(FP_IOC_MAGIC, 4)
#define FP_IOC_ENABLE_SPI_CLK _IOW(FP_IOC_MAGIC, 5, uint32_t)
#define FP_IOC_DISABLE_SPI_CLK _IO(FP_IOC_MAGIC, 6)
#define FP_IOC_ENABLE_POWER _IO(FP_IOC_MAGIC, 7)
#define FP_IOC_DISABLE_POWER _IO(FP_IOC_MAGIC, 8)
#define FP_IOC_INPUT_KEY_EVENT _IOW(FP_IOC_MAGIC, 9, struct fp_key)
#define FP_IOC_ENTER_SLEEP_MODE _IO(FP_IOC_MAGIC, 10)
#define FP_IOC_GET_FW_INFO _IOR(FP_IOC_MAGIC, 11, uint8_t)
#define FP_IOC_REMOVE _IO(FP_IOC_MAGIC, 12)
#define FP_IOC_POWER_RESET _IO(FP_IOC_MAGIC, 17)
#define FP_IOC_WAKELOCK_TIMEOUT_ENABLE _IO(FP_IOC_MAGIC, 18)
#define FP_IOC_WAKELOCK_TIMEOUT_DISABLE _IO(FP_IOC_MAGIC, 19)
#define FP_IOC_CLEAN_TOUCH_FLAG _IO(FP_IOC_MAGIC, 20)
#define FP_IOC_AUTO_SEND_TOUCHDOWN        _IO(FP_IOC_MAGIC, 21)
#define FP_IOC_AUTO_SEND_TOUCHUP        _IO(FP_IOC_MAGIC, 22)
#define FP_IOC_STOP_WAIT_INTERRUPT_EVENT _IO(FP_IOC_MAGIC, 23)

/************************************************/

namespace android {
class HalContext;
class FingerprintMessage;

class Device {
public:
    explicit Device(HalContext* context);
    ~Device();
    fp_return_type_t open();
    fp_return_type_t close();
    fp_return_type_t reset();
    fp_return_type_t enable();
    fp_return_type_t disable();
    fp_return_type_t controlSpiClock(device_control_spi_t enable);
    fp_return_type_t controlPower(device_control_power_t enable);
    fp_return_type_t controlIrq(device_control_irq_t enable);
    fp_return_type_t controlTp(device_control_tp_t mode);
    fp_return_type_t cleanTouchStatus();
    fp_return_type_t holdWakeLock(device_wakelock_t mode);
    fp_return_type_t stopWaitFingerprintEvent();
public:
    int32_t mFd;
    bool mStopWaitInterrupt;
protected:
    HalContext* mHalContext;
    // bool mStopWaitInterrupt
};
}  // namespace android

#endif /* _DEVICE_H_ */
