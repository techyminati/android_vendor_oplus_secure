/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][CustomizedDevice]"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "CustomizedDevice.h"
#include "gf_event.h"
#include "Sensor.h"
#include "HalLog.h"
#include "HalContext.h"
#include "szMsgBus.h"
#include "CustomizedFingerprintCore.h"
#include "HalUtils.h"
#include "FingerprintCore.h"
#ifdef FP_TRACE_DEBUG
#include <cutils/trace.h>
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#endif

extern "C"{
#include "Fpsys.h"
}

namespace goodix {
    CustomizedDevice::CustomizedDevice(HalContext *context) : Device(context) {
    }

    CustomizedDevice::~CustomizedDevice() {
    }

    gf_event_type_t CustomizedDevice::mapMsgToEvent(int32_t msg) {
        gf_event_type_t e = EVENT_UNKNOWN;

        if (GF_NETLINK_TEST == msg) {
            // for netlink test use only
            LOG_D(LOG_TAG, "[%s] received GF_NETLINK_TEST command", __func__);
        } else if (GF_NETLINK_IRQ == msg || GF_NETLINK_SCREEN_OFF == msg || GF_NETLINK_SCREEN_ON == msg
                ||GF_NETLINK_UI_READY == msg || GF_NETLINK_UI_DISAPPEAR == msg
                || GF_NETLINK_TP_TOUCHDOWN == msg|| GF_NETLINK_TP_TOUCHUP == msg
            ) {
            if (GF_NETLINK_IRQ == msg) {
                e = EVENT_IRQ;
            } else if (GF_NETLINK_SCREEN_ON == msg) {
                e = EVENT_SCREEN_ON;
            } else if (GF_NETLINK_SCREEN_OFF == msg) {
                e = EVENT_SCREEN_OFF;
            }
            else if (GF_NETLINK_TP_TOUCHDOWN == msg) {
#ifdef FP_TRACE_DEBUG
                atrace_begin(ATRACE_TAG, "EVENT_FINGER_DOWN");
#endif  // fp trace debug
                e = EVENT_FINGER_DOWN;
                sendMessage(MSG_BIG_DATA_FINGER_DOWN);
                cancelSleepTimer();
#ifdef FP_TRACE_DEBUG
                atrace_end(ATRACE_TAG);
#endif  // fp trace debug
            } else if (GF_NETLINK_TP_TOUCHUP == msg) {
                e = EVENT_FINGER_UP;
                static_cast<CustomizedFingerprintCore*>(mContext->mFingerprintCore)->mUpTime = HalUtils::getCurrentTimeMicrosecond();
                cancelKeyTimer();
                if (nullptr != mContext && nullptr != mContext->mFingerprintCore) {
                    mContext->mFingerprintCore->notifyAuthUpEvt();
                }
            } else if (GF_NETLINK_UI_READY == msg) {
#ifdef FP_TRACE_DEBUG
                atrace_begin(ATRACE_TAG, "EVENT_UI_READY");
#endif  // fp trace debug
                e = EVENT_UI_READY;
                if (nullptr != mContext && nullptr != mContext->mSensor && (false == mContext->mFingerprintCore->isAuthDownDetected())) {
                    cancelSleepTimer();
                    mContext->mSensor->wakeupSensor();
                    startSleepTimer(1);
                    sendMessage(MSG_BIG_DATA_UI_READY);
                }
                LOG_I(LOG_TAG, "[%s] received uiread msg", __func__);  //syncbyllh frommaster
                /* sleep 15ms for UI Ready */
                usleep(15 * 1000);
                LOG_D(LOG_TAG, "[%s] uiready usleep end", __func__);

                notify_finger_ready();
#ifdef FP_TRACE_DEBUG
                atrace_end(ATRACE_TAG);
#endif  // fp trace debug
            } else if (GF_NETLINK_UI_DISAPPEAR == msg) {
#ifdef FP_TRACE_DEBUG
                atrace_begin(ATRACE_TAG, "EVENT_UI_DISAPPEAR");
#endif  // fp trace debug
                e = EVENT_UI_DISAPPEAR;
                clear_finger_ready_flag();
#ifdef FP_TRACE_DEBUG
                atrace_end(ATRACE_TAG);
#endif  // fp trace debug
            }
        } else {
            LOG_E(LOG_TAG, "[%s] wrong netlink command value=%u", __func__, msg);
        }

        return e;
    }
}  // namespace goodix

