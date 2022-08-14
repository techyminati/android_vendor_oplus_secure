/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/FingerprintMessage.cpp
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
#define LOG_TAG "[FP_HAL][FingerprintMessage]"

#include <errno.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "FingerprintMessage.h"
#include "ActionType.h"

#define FP_DIRVER_NETLINK (1)
#define FP_DRIVER_INTERRUPT (2)
#define MAX_MESSAGE_SIZE (128)

struct fingerprint_message_t {
    int module;
    int event;
    int in_size;
    char in_buf[MAX_MESSAGE_SIZE];
    int out_size;
    char out_buf[MAX_MESSAGE_SIZE];
};


enum fingerprint_event_module {E_FP_TP = 0, E_FP_LCD = 1, E_FP_HAL = 2 };

enum fingerprint_event {
    E_FP_EVENT_TEST = 0,
    E_FP_EVENT_IRQ = 1,
    E_FP_EVENT_SCR_OFF = 2,
    E_FP_EVENT_SCR_ON = 3,
    E_FP_EVENT_TP_TOUCHDOWN = 4,
    E_FP_EVENT_TP_TOUCHUP = 5,
    E_FP_EVENT_UI_READY = 6,
    E_FP_EVENT_UI_DISAPPEAR = 7,
    E_FP_EVENT_STOP_INTERRUPT = 8,
    E_FP_EVENT_MAX,
};

namespace android {
FingerprintMessage *FingerprintMessage::sInstance = nullptr;

FingerprintMessage::FingerprintMessage(HalContext *context)
    : mHalContext(context) {
    LOG_I(LOG_TAG, "enter to FingerprintMessage");
    mEventState.last_state = E_EVENT_STATE_NONE;
    mMessageHandler        = new Handler("FingerprintMessage", this);
    sInstance              = this;
}

FingerprintMessage::~FingerprintMessage() {
    if (HalContext::getInstance()->mDevice->mFd < 0) {
        LOG_E(LOG_TAG, "[%s], fd not exist", __func__);
        return;
    }
    HalContext::getInstance()->mDevice->stopWaitFingerprintEvent();
    HalContext::getInstance()->mDevice->mStopWaitInterrupt = true;
}

//WaitEvent(all event)
//doevent(all envent)
//screenon/off donothing

bool FingerprintMessage::handleTouchDownEvent() {
    bool isEventLegal = false;
    switch (mEventState.last_state) {
        case E_EVENT_STATE_NONE:
        case E_EVENT_STATE_TP_TOUCHDOWN:
        case E_EVENT_STATE_TP_TOUCHUP:
        case E_EVENT_STATE_UI_READY:
            isEventLegal = true;
            break;
        default:
            isEventLegal = false;
            break;
    }
    return isEventLegal;
}

bool FingerprintMessage::handleTouchUpEvent() {
    bool isEventLegal = false;
    switch (mEventState.last_state) {
        case E_EVENT_STATE_TP_TOUCHDOWN:
        case E_EVENT_STATE_UI_READY:
            isEventLegal = true;
            break;
        default:
            isEventLegal = false;
            break;
    }
    return isEventLegal;
}

bool FingerprintMessage::handleUireadyEvent() {
    bool isEventLegal = false;
    switch (mEventState.last_state) {
        case E_EVENT_STATE_TP_TOUCHDOWN:
            isEventLegal = true;
            break;
        default:
            isEventLegal = false;
            break;
    }
    return isEventLegal;
}

void FingerprintMessage::setCurrentEventState(int msg) {
    std::lock_guard<std::mutex> lock(mEventLock);
    switch (msg) {
        case NETLINK_EVENT_TP_TOUCHDOWN:
            mEventState.last_state = E_EVENT_STATE_TP_TOUCHDOWN;
            break;
        case NETLINK_EVENT_TP_TOUCHUP:
            mEventState.last_state = E_EVENT_STATE_TP_TOUCHUP;
            break;
        case NETLINK_EVENT_UI_READY:
            mEventState.last_state = E_EVENT_STATE_UI_READY;
            break;
        default:
            mEventState.last_state = E_EVENT_STATE_NONE;
            break;
    }
}

fp_return_type_t FingerprintMessage::handleNetlinkMessage(int32_t msg) {
    VOID_FUNC_ENTER();
    UNUSED(msg);
    UNUSED(mHalContext);
    bool isEventLegal = false;
    switch (msg) {
        case NETLINK_EVENT_TP_TOUCHDOWN:
            isEventLegal = handleTouchDownEvent();
            break;
        case NETLINK_EVENT_TP_TOUCHUP:
            isEventLegal = handleTouchUpEvent();
            break;
        case NETLINK_EVENT_UI_READY:
            isEventLegal = handleUireadyEvent();
            break;
        default:
            break;
    }

    if (isEventLegal) {
        LOG_E(LOG_TAG, "[%s] event accepted,last status:%d current event: %d", __func__,
            mEventState.last_state, msg);
        setCurrentEventState(msg);
        action_message_t       action;
        memset(&action, 0, sizeof(action_message_t));
        action.type = (action_type_t)(FP_EVENT_BASE + msg);
        mHalContext->mFingerprintManager->sendMessageToWorkThread(action);
    } else {
        LOG_E(LOG_TAG, "[%s] event reject,last status:%d current event: %d", __func__,
            mEventState.last_state, msg);
    }
    VOID_FUNC_EXIT();
    return FP_SUCCESS;
}


void FingerprintMessage::interruptProcessFpEvent()
{
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    if (HalContext::getInstance()->mDevice->mFd < 0) {
        LOG_E(LOG_TAG, "[%s], fd not exist", __func__);
        err = FP_DEVICE_FILE_DESCRIPTION_ERROR;
        return;
    }
    struct fingerprint_message_t msg;
    memset(&msg, 0, sizeof(msg));
    while (1) {
        if (HalContext::getInstance()->mDevice->mStopWaitInterrupt) {
            break;
        }
        memset(&msg, 0 , sizeof(msg));
        LOG_E(LOG_TAG, "start wait event\n");
        if (read(HalContext::getInstance()->mDevice->mFd, &msg, sizeof(msg)) == sizeof(msg)) {
            LOG_E(LOG_TAG, "module:%d, event:%d\n", msg.module, msg.event);
            if (msg.module == E_FP_HAL && msg.event == E_FP_EVENT_STOP_INTERRUPT) {
                break;
            }
            handleNetlinkMessage(msg.event);
        }
    }

    FUNC_EXIT(err);
    return;
}

fp_return_type_t FingerprintMessage::netlinkThreadLoop() {
    fp_return_type_t err     = FP_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] started", __func__);
    interruptProcessFpEvent();
    FUNC_EXIT(err);
    return err;
}

void FingerprintMessage::handleMessage(FpMessage msg) {
    VOID_FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] what %d", __func__, msg.what);

    switch (msg.what) {
        case netlink_creat_socket_and_start_loop:
            netlinkThreadLoop();
            break;
        default:
            LOG_E(LOG_TAG, "error: %s switch default msg.what =%d", __func__, msg.what);
            break;
    }
    VOID_FUNC_EXIT();
}

fp_return_type_t FingerprintMessage::sendToMessageThread(netlink_status_type_t status)
{
    fp_return_type_t err = FP_SUCCESS;
    FpMessage        msg((int)FP_INIT);
    switch (status) {
    case netlink_creat_socket_and_start_loop:
        msg.what = status;
        mMessageHandler->sendMessage(msg);
        break;
    default:
        break;
    }
    return err;
}
}  // namespace android
