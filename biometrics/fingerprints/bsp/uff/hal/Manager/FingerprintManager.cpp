/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Manager/FingerprintManager.cpp
 **
 ** Description:
 **      FingerprintManager for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][FingerprintManager]"

#include "FingerprintManager.h"
#include <cutils/properties.h>
#include <endian.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "DcsInfo.h"
#include "Device.h"
#include "HalContext.h"
#include "HalLog.h"
#include "fingerprint.h"
#include "FpCommon.h"
#include "FpType.h"
#include "Device.h"
#include "FingerprintMessage.h"
#include "Dump.h"
#include "Perf.h"

namespace android {
FingerprintManager::FingerprintManager(HalContext *context):
    FingerprintFunction(context),
    mNetlinkMessageToWait(0) {
    LOG_I(LOG_TAG, "enter to FingerprintManager");
    mManageHandler = new Handler("FingerprintManager", this);
}

FingerprintManager::~FingerprintManager() {
    LOG_I(LOG_TAG, "enter to ~FingerprintManager");
}

void FingerprintManager::handleMessage(FpMessage msg) {
    LOG_I(LOG_TAG, "[%s] what %d", __func__, msg.what);
    FUNC_ENTER();
    fp_return_type_t err = FP_SUCCESS;
    switch (msg.what) {
        case FP_INIT:
            LOG_D(LOG_TAG, "%s enter FP_INIT", __func__);
            //TBD
            break;

        case FP_SETNOTIFY:
            LOG_D(LOG_TAG, "%s enter FP_SETNOTIFY", __func__);
            //TBD
            break;

        case FP_SETACTIVEGROUP:
            LOG_D(LOG_TAG, "%s enter FP_SETACTIVEGROUP", __func__);
            setActiveGroup(msg.arg0, (const char*)msg.str1);
            break;

        case FP_ENUMERATE:
            LOG_D(LOG_TAG, "%s enter FP_ENUMERATE", __func__);
            enumerate();
            break;

        case FP_PREENROLL:
            LOG_D(LOG_TAG, "%s enter FP_PREENROLL", __func__);
            //TBD return uint64_t
            break;

        case FP_ENROLL_ACTION:
            LOG_D(LOG_TAG, "%s enter FP_ENROLl", __func__);
            enroll((const void*)msg.str1, msg.arg0, msg.arg1);
            break;

        case FP_POSTENROLL:
            LOG_D(LOG_TAG, "%s enter FP_POSTENROLL", __func__);
            postEnroll();
            break;

        case FP_REMOVE:
            LOG_D(LOG_TAG, "%s enter FP_REMOVE", __func__);
            remove(msg.arg0, msg.arg1);
            break;

        case FP_AUTH_ACTION:
            LOG_D(LOG_TAG, "%s enter FP_AUTH_ACTION", __func__);
            authenticateAsType(msg.arg3, msg.arg0, msg.arg1);
            break;

        case FP_CANCEL:
            LOG_D(LOG_TAG, "%s enter FP_CANCEL", __func__);
            cancel();
            break;

        case FP_SET_SCREEN_STATE:
            LOG_D(LOG_TAG, "%s enter FP_SET_SCREEN_STATE", __func__);
            screenState(msg.arg0);
            break;

        case FP_SETCONFIG:
            LOG_D(LOG_TAG, "%s enter FP_SETCONFIG", __func__);
            //TBD
            break;

        case FP_TOUCHDOWN:
            LOG_D(LOG_TAG, "%s enter FP_TOUCHDOWN", __func__);
            touchdown();
            break;

        case FP_TOUCHUP:
            LOG_D(LOG_TAG, "%s enter FP_TOUCHUP", __func__);
            touchup();
            break;

        case FP_SENDFPCMD:
            LOG_D(LOG_TAG, "%s enter FP_SENDFPCMD cmdid = %d", __func__, msg.arg0);
            sendFingerprintCmd(msg.arg0, (int8_t *)msg.str1, msg.arg1);
            break;

        case FP_EVENT_TP_TOUCHDOWN:
            LOG_E(LOG_TAG, "%s enter FP_EVENT_TP_TOUCHDOWN", __func__);
            doTouchDownEvent(FP_UNLOCK);
            break;

        default:
            LOG_D(LOG_TAG, "%s enter default", __func__);
            break;
    }

    FUNC_EXIT(err);
}

fp_return_type_t FingerprintManager::sendMessageToWorkThread(action_message_t action) {
    fp_return_type_t err = FP_SUCCESS;
    FpMessage        msg((int)FP_INIT);
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] action.type = %d", __func__, action.type);

    msg.what = action.type;
    if ((action.type >= FP_EVENT_BASE) && (action.type <= FP_EVENT_MAX)) {
        handleNetlinkMessage(action.type);
        goto fp_out;
    }
    switch (action.type) {
        case FP_INIT:
            LOG_D(LOG_TAG, "%s enter FP_INIT", __func__);
            //TBD
            mManageHandler->sendMessage(msg);
            break;

        case FP_SETNOTIFY:
            LOG_D(LOG_TAG, "%s enter FP_SETNOTIFY", __func__);
            //TBD
            mManageHandler->sendMessage(msg);
            break;

        case FP_SETACTIVEGROUP:
            LOG_D(LOG_TAG, "%s enter FP_SETACTIVEGROUP", __func__);
            msg.arg0 = action.data.setactivegroup.gid;
            msg.str1 = (void *)action.data.setactivegroup.path;
            mManageHandler->sendMessage(msg);
            break;

        case FP_ENUMERATE:
            LOG_D(LOG_TAG, "%s enter FP_ENUMERATE", __func__);
            mManageHandler->sendMessage(msg);
            break;

        case FP_PREENROLL:
            LOG_D(LOG_TAG, "%s enter FP_PREENROLL", __func__);
            //TBD
            mManageHandler->sendMessage(msg);
            break;

        case FP_ENROLL_ACTION:
            LOG_D(LOG_TAG, "%s enter FP_ENROLL", __func__);
            setWorkMode(FP_MODE_ENROLL);
            msg.arg0 = action.data.enroll.gid;
            msg.arg1 = action.data.enroll.timeoutSec;
            msg.str1 = (void *)action.data.enroll.authToken;
            mManageHandler->sendMessage(msg);
            break;

        case FP_POSTENROLL:
            LOG_D(LOG_TAG, "%s enter FP_POSTENROLL", __func__);
            mManageHandler->sendMessage(msg);
            break;

        case FP_REMOVE:
            LOG_D(LOG_TAG, "%s enter FP_REMOVE", __func__);
            msg.arg0 = action.data.remove.gid;
            msg.arg1 = action.data.remove.fid;
            mManageHandler->sendMessage(msg);
            break;

        case FP_AUTH_ACTION:
            LOG_D(LOG_TAG, "%s enter FP_AUTH_ACTION", __func__);
            setWorkMode(FP_MODE_AUTH);
            msg.arg3 = action.data.authenticate.operationId;
            msg.arg0 = action.data.authenticate.gid;
            msg.arg1 = action.data.authenticate.authtype;
            mManageHandler->sendMessage(msg);
            break;

        case FP_CANCEL:
            LOG_D(LOG_TAG, "%s enter FP_CANCEL", __func__);
            setWorkMode(FP_MODE_CANCEL);
            mManageHandler->sendMessage(msg);
            break;

        case FP_SET_SCREEN_STATE:
            LOG_D(LOG_TAG, "%s enter FP_SET_SCREEN_STATE", __func__);
            screenState(action.data.screenstate.state);
            break;

        case FP_SETCONFIG:
            LOG_D(LOG_TAG, "%s enter FP_SETCONFIG", __func__);
            //TBD
            mManageHandler->sendMessage(msg);
            break;

        case FP_TOUCHDOWN:
            LOG_D(LOG_TAG, "%s enter FP_TOUCHDOWN", __func__);
            //TBD
            mManageHandler->sendMessage(msg);
            break;

        case FP_TOUCHUP:
            LOG_D(LOG_TAG, "%s enter FP_TOUCHUP", __func__);
            //TBD
            mManageHandler->sendMessage(msg);
            break;

        case FP_SENDFPCMD:
            LOG_D(LOG_TAG, "%s enter FP_SENDFPCMD", __func__);
            msg.arg0 = action.data.sendfpcmd.cmdid;
            msg.arg1 = action.data.sendfpcmd.in_buff_size;
            msg.str1 = (void *)action.data.sendfpcmd.in_buff_data;
            mManageHandler->sendMessage(msg);
            break;

        default:
            LOG_D(LOG_TAG, "%s invalid action type: %d", __func__, action.type);
            break;
    }

fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintManager::setNetlinkMessageToWait(int32_t wait_msg) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    //TBD :add lock
    mNetlinkMessageToWait = wait_msg;
    LOG_D(LOG_TAG, "%s wait_msg: %d", __func__, wait_msg);
    FUNC_EXIT(err);
    return err;
}

int32_t FingerprintManager::getNetlinkMessageToBeWait(void) {
    ////TBD :add lock
    return mNetlinkMessageToWait;
}

fp_return_type_t FingerprintManager::handleNetlinkMessage(int32_t msg) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] msg = %d", __func__, msg);
    switch (msg) {
        case FP_EVENT_SCREEN_OFF:
            LOG_D(LOG_TAG, "%s enter FP_NET_EVENT_SCR_OFF", __func__);
            //TBD
            break;

        case FP_EVENT_SCREEN_ON:
            LOG_D(LOG_TAG, "%s enter FP_NET_EVENT_SCR_ON", __func__);
            //TBD
            break;

        case FP_EVENT_TP_TOUCHDOWN:
            LOG_D(LOG_TAG, "%s enter FP_NET_EVENT_TP_TOUCHDOWN", __func__);
            notifyTouch(FINGERPRINT_TOUCH_DOWN_STATUE);//TBD:put it in work thread??
            setFingerstatus(FP_FINGER_DOWN);
            mManageHandler->sendMessage(msg);
            break;

        case FP_EVENT_TP_TOUCHUP:
            LOG_D(LOG_TAG, "%s enter FP_NET_EVENT_TP_TOUCHUP", __func__);
            //TBD: cancel ??
            setFingerstatus(FP_FINGER_UP);
            notifyTouch(FINGERPRINT_TOUCH_UP_STATUE);
            break;

        case FP_EVENT_UI_READY:
            LOG_D(LOG_TAG, "%s enter FP_NET_EVENT_UI_READY", __func__);
            postUiready();
            break;

        case FP_EVENT_UI_DISAPPEAR:
            LOG_D(LOG_TAG, "%s enter FP_NET_EVENT_UI_DISAPPEAR", __func__);
            //TBD
            break;

        default:
            LOG_D(LOG_TAG, "%s invalid msg type: %d", __func__, msg);
            break;
    }

    FUNC_EXIT(err);
    return err;
}

fp_return_type_t FingerprintManager::simulatedEvent(SIMULATED_EVENT_T event) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] msg = %d", __func__, event);
    switch (event) {
        case DOWN_EVNT:
            setFingerstatus(FP_FINGER_DOWN);
            doTouchDownEvent(FP_AUTOSMOKING);
            break;
        case UI_EVNT:
            err = postUiready();
            break;
        case UP_EVNT:
            setFingerstatus(FP_FINGER_UP);
            notifyTouch(FINGERPRINT_TOUCH_UP_STATUE);
            break;
        default:
            LOG_D(LOG_TAG, "%s invalid msg type: %d", __func__, event);
            break;
    }
    FUNC_EXIT(err);
    return err;
}


}  // namespace android
