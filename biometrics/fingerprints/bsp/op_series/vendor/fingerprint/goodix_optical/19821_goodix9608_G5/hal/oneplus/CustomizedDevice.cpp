/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][CustomizedDevice]"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "CustomizedDevice.h"
#include "HalLog.h"
#include "HalContext.h"
#include "CaEntry.h"
#include "CustomizedDelmarCommon.h"
#include "CustomizedDelmarSensor.h"

extern "C" {
#include "Fpsys.h"
}
#include "CustomizedFingerprintCore.h"

#ifdef FP_TRACE_DEBUG
#include <cutils/trace.h>
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#endif  // FP_TRACE_DEBUG

#ifndef CUSTOMIZE_DEV_NAME
#define CUSTOMIZE_DEV_NAME "/dev/goodix_fp_9518"
#endif  // CUSTOMIZE_DEV_NAME

#ifndef GF_DEV_NAME
#define GF_DEV_NAME "/dev/goodix_fp"
#endif  // GF_DEV_NAME

#ifndef GF_IOC_MAGIC
#define GF_IOC_MAGIC    'g'      // define magic number
#endif  // GF_IOC_MAGIC

#ifndef GF_IOC_NAV_EVENT
#define GF_IOC_NAV_EVENT        (_IOW(GF_IOC_MAGIC, 14, gf_nav_code_t))
#endif  // GF_IOC_NAV_EVENT

typedef struct gf_netlink_tp_info {
    uint8_t touch_state;
    uint8_t area_rate;
    uint16_t x;
    uint16_t y;
    //uint16_t touchMajor;
    //uint16_t touchMinor;
    //uint16_t touchOrientation;
}gf_netlink_tp_info_t;

typedef struct gf_netlink_msg_info {
    uint8_t netlink_cmd;
    gf_netlink_tp_info_t tp_info;
}gf_netlink_msg_info_t;

namespace goodix {
    CustomizedDevice::CustomizedDevice(HalContext *context) : Device(context) {
    }

    CustomizedDevice::~CustomizedDevice() {
    }
    gf_error_t CustomizedDevice::open() {
        gf_error_t err = GF_SUCCESS;
        uint32_t i = 0;
        FUNC_ENTER();

        do {
            while (i < 10) {
                mFD = ::open(CUSTOMIZE_DEV_NAME, O_RDWR);

                if (mFD < 0) {
                    LOG_E(LOG_TAG,
                          "[%s] Failed to open device (%s),and reopen device. g_fd=%u, errno =%d",
                          __func__, CUSTOMIZE_DEV_NAME, mFD, errno);
                    err = GF_ERROR_OPEN_DEVICE_FAILED;
                    mFD = -1;
                    i++;
                } else {
                    LOG_E(LOG_TAG, "[%s] Success to open device (%s) g_fd=%u", __func__,
                          CUSTOMIZE_DEV_NAME, mFD);
                    err = GF_SUCCESS;
                    break;
                }

                usleep(200 * 1000);
            }

            // Note: The method can't access mCaEntry data, or crash wil happend. Now only ree use the method.
            mContext->mCaEntry->setDeviceHandle(mFD);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_event_type_t CustomizedDevice::mapMsgToEvent(int32_t msg) {
        gf_event_type_t e = EVENT_UNKNOWN;

        if (GF_NETLINK_TEST == msg) {
            // for netlink test use only
            LOG_D(LOG_TAG, "[%s] received GF_NETLINK_TEST command", __func__);
        } else if (GF_NETLINK_IRQ == msg || GF_NETLINK_SCREEN_OFF == msg || GF_NETLINK_SCREEN_ON == msg
                ||GF_NETLINK_UI_READY == msg || GF_NETLINK_UI_DISAPPEAR == msg
                || GF_NETLINK_TP_TOUCHDOWN == msg|| GF_NETLINK_TP_TOUCHUP == msg) {
            if (GF_NETLINK_IRQ == msg) {
                e = EVENT_IRQ;
            } else if (GF_NETLINK_SCREEN_ON == msg) {
                e = EVENT_SCREEN_ON;
            } else if (GF_NETLINK_SCREEN_OFF == msg) {
                e = EVENT_SCREEN_OFF;
            } else if (GF_NETLINK_TP_TOUCHDOWN == msg) {
#ifdef FP_TRACE_DEBUG
                atrace_begin(ATRACE_TAG, "EVENT_FINGER_DOWN");
#endif  // FP_TRACE_DEBUG
                e = EVENT_FINGER_DOWN;
                sendMessage(MSG_BIG_DATA_FINGER_DOWN);
                cancelSleepTimer();
#ifdef FP_TRACE_DEBUG
                atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG
            } else if (GF_NETLINK_TP_TOUCHUP == msg) {
                e = EVENT_FINGER_UP;
                cancelKeyTimer();
                if (nullptr != mContext && nullptr != mContext->mFingerprintCore) {
                    ((CustomizedFingerprintCore*)(mContext->mFingerprintCore))->notifyAuthUpEvt();
                }
            } else if (GF_NETLINK_UI_READY == msg) {
#ifdef FP_TRACE_DEBUG
                atrace_begin(ATRACE_TAG, "EVENT_UI_READY");
#endif  // FP_TRACE_DEBUG
                e = (gf_event_type_t)EVENT_UI_READY;
                if (nullptr != mContext && nullptr != mContext->mSensor && (false == ((CustomizedFingerprintCore*)(mContext->mFingerprintCore))->customizedIsAuthDownDetected())) {
                    cancelSleepTimer();
                    mContext->mSensor->wakeupSensor();
                    startSleepTimer(1);
                    sendMessage(MSG_BIG_DATA_UI_READY);
                }
                LOG_D(LOG_TAG, "[%s] UI ready 15 delay start", __func__);
                usleep(15*1000);
                notify_finger_ready();
#ifdef FP_TRACE_DEBUG
                atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG
            } else if (GF_NETLINK_UI_DISAPPEAR == msg) {
#ifdef FP_TRACE_DEBUG
                atrace_begin(ATRACE_TAG, "EVENT_UI_DISAPPEAR");
#endif  // FP_TRACE_DEBUG
                e = (gf_event_type_t)EVENT_UI_DISAPPEAR;
                clear_finger_ready_flag();
#ifdef FP_TRACE_DEBUG
                atrace_end(ATRACE_TAG);
#endif  // FP_TRACE_DEBUG
            }
        } else {
            LOG_E(LOG_TAG, "[%s] wrong netlink command value=%u", __func__, msg);
        }
        return e;
    }

    // 20180917 add for quick pay
    gf_error_t CustomizedDevice::sendKeyEvent(gf_nav_code_t code) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (mFD < 0) {
                LOG_E(LOG_TAG, "[%s], no dev=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_NAV_EVENT, &code) != 0) {
                LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    void CustomizedDevice::keyTimerThread(union sigval v) {
        VOID_FUNC_ENTER();

        do {
            if (NULL == v.sival_ptr) {
                LOG_E(LOG_TAG, "[%s] invalid timer signal data", __func__);
                break;
            }

            CustomizedDevice* device = static_cast<CustomizedDevice*>(v.sival_ptr);
            device->sendKeyEvent(GF_NAV_F2);
            device->cancelKeyTimer();
        }
        while (0);
        // TODO implement
        VOID_FUNC_EXIT();
    }

    void CustomizedDevice::startKeyTimer(uint32_t timeoutSec) {
        VOID_FUNC_ENTER();
        do {
            gf_error_t err = GF_SUCCESS;

            if (NULL == pmKeyTimer) {
                pmKeyTimer = Timer::createTimer((timer_thread_t) keyTimerThread, this);
                if (NULL == pmKeyTimer) {
                    LOG_E(LOG_TAG, "[%s] create key timer failed", __func__);
                    break;
                }
            }

            err = pmKeyTimer->set(timeoutSec, 0, timeoutSec, 0);
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] start key timer failed", __func__);
            }
        }
        while (0);

        VOID_FUNC_EXIT();
    }

    void CustomizedDevice::cancelKeyTimer() {
        VOID_FUNC_ENTER();
        if (pmKeyTimer != NULL) {
            delete pmKeyTimer;
            pmKeyTimer = NULL;
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t CustomizedDevice::sendSleepEvent() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (nullptr != mContext && nullptr != mContext->mSensor && (false == ((CustomizedFingerprintCore*)(mContext->mFingerprintCore))->customizedIsAuthDownDetected())) {
            mContext->mSensor->sleepSensor();
        }

        FUNC_EXIT(err);
        return err;
    }

    void CustomizedDevice::sleepTimerThread(union sigval v) {
        VOID_FUNC_ENTER();

        do {
            if (NULL == v.sival_ptr) {
                LOG_E(LOG_TAG, "[%s] invalid timer signal data", __func__);
                break;
            }

            CustomizedDevice* device = static_cast<CustomizedDevice*>(v.sival_ptr);
            device->sendSleepEvent();
            device->cancelSleepTimer();
        }
        while (0);
        // TODO implement
        VOID_FUNC_EXIT();
    }

    void CustomizedDevice::startSleepTimer(uint32_t timeoutSec) {
        VOID_FUNC_ENTER();

        do {
            gf_error_t err = GF_SUCCESS;

            if (NULL == pmSleepTimer) {
                pmSleepTimer = Timer::createTimer((timer_thread_t) sleepTimerThread, this);
                if (NULL == pmSleepTimer) {
                    LOG_E(LOG_TAG, "[%s] create sleep timer failed", __func__);
                    break;
                }
            }

            err = pmSleepTimer->set(timeoutSec, 0, timeoutSec, 0);
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] start sleep timer failed", __func__);
            }
        }
        while (0);

        VOID_FUNC_EXIT();
    }

    void CustomizedDevice::cancelSleepTimer() {
        VOID_FUNC_ENTER();
        if (pmSleepTimer != NULL) {
            delete pmSleepTimer;
            pmSleepTimer = NULL;
        }

        VOID_FUNC_EXIT();
    }

    void CustomizedDevice::handleNetlinkReceivedData(void* data, uint32_t len) {
        uint8_t value = 0;
        gf_event_type_t e = EVENT_UNKNOWN;
        gf_netlink_msg_info_t gf_netlink_info;
        UNUSED_VAR(len);
        LOG_D(LOG_TAG, "[%s] receive length =%d", __func__, len);
        if (NULL == data) {
            return;
        }
        value = *((uint8_t*)data);
        LOG_D(LOG_TAG, "[%s] GF netlink thread received msg %u.", __func__, value);
        e = mapMsgToEvent(value);
        if (EVENT_FINGER_DOWN == e) {
            // parse sensor location data
            memcpy((uint8_t*)&gf_netlink_info, ((char *) data), sizeof(gf_netlink_info));
            memset(&tp_info, 0, sizeof(tp_info));
            memcpy((uint8_t*)&tp_info, ((uint8_t *)&(gf_netlink_info.tp_info)), sizeof(FP_UNDERSCREEN_INFO));
            LOG_D(LOG_TAG, "[%s] GF handleNetlinkReceivedData:%d %d x =%d.y =%d.", __func__,tp_info.touch_state,
                tp_info.area_rate, tp_info.x, tp_info.y);
        }
        handleNetlinkMessage(value);
        return;
    }
}  // namespace goodix

