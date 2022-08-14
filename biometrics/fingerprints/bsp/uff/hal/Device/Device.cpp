/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump.cpp
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
************************************************************************************/
#define LOG_TAG "[FP_HAL][Device]"

#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <linux/uinput.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "Device.h"
#include "FingerprintManager.h"
#include "FingerprintMessage.h"
#include "HalContext.h"
#include "HalLog.h"
#include "FpCommon.h"
#include "FpType.h"

namespace android {

Device::Device(HalContext* context) {
    mFd = -1;
    mHalContext = context;
    mStopWaitInterrupt = false;
}

Device::~Device() {
    close();
}

fp_return_type_t Device::open() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    mFd = ::open(FP_DEV_NODE, O_RDWR);
    if (mFd < 0) {
        err = FP_DEVICE_OPEN_ERROR;
        LOG_E(LOG_TAG, "[%s] Failed to open fd (%s),and reopen fd. g_fd=%u, errno =%d",
            __func__, FP_DEV_NODE, mFd, errno);
    }

    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::close() {
    fp_return_type_t err = FP_SUCCESS;
    int32_t ret = -1;
    FUNC_ENTER();
    stopWaitFingerprintEvent();
    mStopWaitInterrupt = true;
    err = disable();
    if (mFd >= 0) {
        ret = ::close(mFd);
        if (ret < 0) {
            LOG_E(LOG_TAG, "[%s] Failed to close fd (%s). g_fd=%u, errno =%d",
                __func__, FP_DEV_NODE, mFd, errno);
            err = FP_DEVICE_CLOSE_ERROR;
        }
        mFd = -1;
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::reset() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    if (mFd < 0) {
        LOG_E(LOG_TAG, "[%s], no fd=%s", __func__, FP_DEV_NODE);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }

    if (ioctl(mFd, FP_IOC_RESET) != 0) {
        LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
        err = FP_DEVICE_IOTCL_ERROR;
        goto fp_out;
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::enable() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    if (mFd < 0) {
        LOG_E(LOG_TAG, "[%s], no fd=%s", __func__, FP_DEV_NODE);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }

    if (ioctl(mFd, FP_IOC_ENABLE_IRQ) != 0) {
        LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
        err = FP_DEVICE_IOTCL_ERROR;
        goto fp_out;
    }

    err = mHalContext->mFingerprintMessage->sendToMessageThread(netlink_creat_socket_and_start_loop);
    CHECK_RESULT_SUCCESS(err);

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::stopWaitFingerprintEvent()
{
    fp_return_type_t err = FP_SUCCESS;
    if (mFd < 0) {
        LOG_E(LOG_TAG, "[%s], fd not exist", __func__);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        return err;
    }

    if (ioctl(mFd, FP_IOC_STOP_WAIT_INTERRUPT_EVENT) != 0) {
        LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
        err = FP_DEVICE_IOTCL_ERROR;
        return err;
    }
    return err;
}


fp_return_type_t Device::disable() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    if (mFd < 0) {
        LOG_E(LOG_TAG, "[%s], no fd=%s", __func__, FP_DEV_NODE);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }

    if (ioctl(mFd, FP_IOC_DISABLE_IRQ) != 0) {
        LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
        err = FP_DEVICE_IOTCL_ERROR;
        goto fp_out;
    }
    if (ioctl(mFd, FP_IOC_EXIT) != 0) {
        LOG_E(LOG_TAG, "[%s] GF_IOC_EXIT ioctl failed", __func__);
        err = FP_DEVICE_IOTCL_ERROR;
        goto fp_out;
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::controlSpiClock(device_control_spi_t enable) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    if (mFd < 0) {
        LOG_E(LOG_TAG, "[%s] no fd is opened", __func__);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }

    if (enable == DEVICE_ENABLE_SPI) {
        if (ioctl(mFd, FP_IOC_ENABLE_SPI_CLK) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    } else {
        if (ioctl(mFd, FP_IOC_DISABLE_SPI_CLK) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::controlPower(device_control_power_t enable) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    if (mFd < 0) {
        LOG_E(LOG_TAG, "[%s] no fd is opened", __func__);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }

    if (enable == DEVICE_ENABLE_POWER) {
        if (ioctl(mFd, FP_IOC_ENABLE_POWER) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    } else {
        if (ioctl(mFd, FP_IOC_DISABLE_POWER) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::controlIrq(device_control_irq_t enable) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    if (mFd < 0) {
        LOG_E(LOG_TAG, "[%s] no fd is opened", __func__);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }

    if (enable == DEVICE_ENABLE_IRQ) {
        if (ioctl(mFd, FP_IOC_ENABLE_IRQ) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    } else {
        if (ioctl(mFd, FP_IOC_DISABLE_IRQ) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::controlTp(device_control_tp_t mode) {
    fp_return_type_t err = FP_SUCCESS;
    char buff[2] = {0};
    int fd = -1;
    FUNC_ENTER();

    holdWakeLock(DEVICE_HOLD_WAKE_LOCK);
    LOG_I(LOG_TAG, "[%s] mode = %d.", __func__, mode);
    fd = ::open(FP_ENABLE_TP_PATH, O_WRONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "[%s] open fp_enale fail.", __func__);
        err =  FP_DEVICE_OPEN_TP_ERROR;
        goto fp_out;
    }
    snprintf(buff, sizeof(buff), "%d", (int)mode);
    write(fd, buff, 2);
    ::close(fd);

fp_out:
    holdWakeLock(DEVICE_RELEASE_WAKE_LOCK);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::cleanTouchStatus() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    if (mFd< 0) {
        LOG_E(LOG_TAG, "[%s], no fd=%s", __func__, FP_DEV_NODE);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }

    if (ioctl(mFd, FP_IOC_CLEAN_TOUCH_FLAG) != 0) {
        LOG_E(LOG_TAG, "[%s] GF_IOC_CLEAN_TOUCH_FLAG ioctl failed", __func__);
        err = FP_DEVICE_IOTCL_ERROR;
        goto fp_out;
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t Device::holdWakeLock(device_wakelock_t mode) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    if (mFd< 0) {
        LOG_E(LOG_TAG, "[%s], no fd=%s", __func__, FP_DEV_NODE);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        goto fp_out;
    }
    if (DEVICE_HOLD_WAKE_LOCK == mode) {
        if (ioctl(mFd, FP_IOC_WAKELOCK_TIMEOUT_ENABLE) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_HOLD_WAKE_LOCK ioctl failed", __func__);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    } else {
        if (ioctl(mFd, FP_IOC_WAKELOCK_TIMEOUT_DISABLE) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_RELEASE_WAKE_LOCK ioctl failed", __func__);
            err = FP_DEVICE_IOTCL_ERROR;
            goto fp_out;
        }
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

}  // namespace android
