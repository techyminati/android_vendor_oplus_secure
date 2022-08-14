/************************************************************************************
 ** File: - CustomizedDevice.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      CustomizedDevice for goodix fingerprint
 **
 ** Version: 1.0
 ** Date created: 10:58:11,12/10/2020
 ** Author: Chen.ran@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         2019/10/12        modify for touchup notify
 **  Ran.Chen         2019/10/24        add  for hypnus
 **  Bangxiong.Wu     2020/02/24        fix set_hypnus block thread problem
************************************************************************************/

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
#include "DelmarSensor.h"
#include "CustomizedFingerprintCore.h"
#include "GoodixFingerprint.h"
#include "to_string.h"

#ifndef CUSTOMIZE_DEV_NAME
#define CUSTOMIZE_DEV_NAME "/dev/goodix_fp"
#endif  // CUSTOMIZE_DEV_NAME

#define NL_MSG_LEN (32)

namespace goodix {
    enum OPPO_NETLINK_CMD {
        GF_NETLINK_TP_TOUCH_DOWN = GF_NETLINK_MAX,
        GF_NETLINK_TP_TOUCH_UP,
        GF_NETLINK_UI_READY,
        GF_NETLINK_UI_DISAPPEAR
    };

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
        LOG_I(LOG_TAG, "[%s] receive msg from kernel: %d", __func__, msg);
        if (GF_NETLINK_TP_TOUCH_UP == msg) {
            LOG_I(LOG_TAG, "[%s] receive msg from kernel:GF_NETLINK_TP_TOUCH_UP", __func__);
            e = EVENT_FINGER_UP;
        } else if (GF_NETLINK_TP_TOUCH_DOWN == msg) {
            e = EVENT_FINGER_DOWN;
            LOG_I(LOG_TAG, "[%s] receive msg from kernel:GF_NETLINK_TP_TOUCH_DOWN", __func__);
        } else if (GF_NETLINK_UI_READY == msg) {
            e = EVENT_UNKNOWN;
            LOG_I(LOG_TAG, "[%s] receive msg from kernel:GF_NETLINK_UI_READY", __func__);

            if (mContext != nullptr && mContext->mSensor != nullptr) {
                #ifdef FP_HYPNUSD_ENABLE
                mContext->mFingerprintCore->hypnus_request_change(ACTION_TYPE, ACTION_TIMEOUT_2000);
                #endif
                usleep(15*1000);
                LOG_D(LOG_TAG, "[%s] UI ready 15 delay end", __func__);
                mContext->mSensor->setSensorUIReady();
            }
        } else if (GF_NETLINK_UI_DISAPPEAR == msg) {
            e = EVENT_UNKNOWN;
            LOG_I(LOG_TAG, "[%s] GF_NETLINK_UI_DISAPPEAR ignored, %d", __func__, msg);
        } else {
            e = Device::mapMsgToEvent(msg);
        }

        return e;
    }

    uint32_t CustomizedDevice::getNetlinkMsgDataLen() {
        return NL_MSG_LEN;
    }

    void CustomizedDevice::handleNetlinkReceivedData(void* data, uint32_t len) {
        uint8_t value = 0;
        gf_event_type_t e = EVENT_UNKNOWN;
        gf_netlink_msg_info_t gf_netlink_info;
        UNUSED_VAR(len);
        if (nullptr == data) {
            return;
        }
        value = *((uint8_t*)data);
        LOG_I(LOG_TAG, "[%s] GF netlink thread received msg %u %s.",
             __func__, value, netlink_to_str(int(value)));
        e = mapMsgToEvent(value);
        if (EVENT_FINGER_DOWN == e) {
            // parse sensor location data
            memcpy((uint8_t*)&gf_netlink_info, ((char *) data), sizeof(gf_netlink_info));
            LOG_D(LOG_TAG, "[%s] GF handleNetlinkReceivedData x =%d.y =%d.", __func__, gf_netlink_info.tp_info.x, gf_netlink_info.tp_info.y);
            int32_t sensorX = gf_netlink_info.tp_info.x;
            int32_t sensorY = gf_netlink_info.tp_info.y;
            int32_t touchMajor = 0;// mm turn to piexll chenran
            int32_t touchMinor = 0;// mm turn to piexll chenran
            int32_t touchOrientation = 0;
            ((CustomizedFingerprintCore*)mContext->mFingerprintCore)->onSensorPressInfo(sensorX, sensorY,
                touchMajor, touchMinor, touchOrientation);
        } else if (EVENT_FINGER_UP == e) {
            ((CustomizedFingerprintCore*)mContext->mFingerprintCore)->notifyTouch(GF_FINGERPRINT_TOUCH_UP);
        }
        if (e != EVENT_UNKNOWN && nullptr != mContext && nullptr != mContext->mCenter) {
            mContext->mCenter->postEvent(e);
        }
        return;
    }

}  // namespace goodix

