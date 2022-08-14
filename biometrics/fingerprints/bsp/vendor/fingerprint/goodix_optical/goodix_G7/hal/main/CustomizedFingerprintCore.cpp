/*
 ** File: - CustomizedFingerprintCore.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      CustomizedFingerprintCore  fingerprint
 **
 ** Version: 1.0
 ** Date created: 10:58:11,16/08/2020
 ** Author: Chen.ran@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         2019/10/31        add for sensorRotation
 **  Bangxiong.Wu     2019/11/08        add for getAlgoVersion from ta and dcsmsg
 **  Ran.Chen         2019/10/31        add lcd_type for dcs cmd
 **  Ran.Chen         2019/10/31        add for enhanced auth in retry0
 **  Bangxiong.Wu     2020/03/02        add for calibrate check and properties set
 ***********************************************************************************/

#define LOG_TAG "[GF_HAL][CustomizedFingerprintCore]"

#include <stdlib.h>
#include <endian.h>
#include <cutils/properties.h>
#include <string.h>
#include "HalContext.h"
#include "CustomizedFingerprintCore.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "HalUtils.h"
#include "Sensor.h"
#include "Device.h"
#include "Algo.h"
#include "DelmarSensor.h"
#include "CustomizedSensor.h"
#include "DelmarHalUtils.h"
#include "gf_customized_types.h"

#ifdef FP_TRACE_TAG_ENABLE
#include <cutils/trace.h>
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#endif

// 0 cycle, 1 ellipse for icon
#define OPTICAL_ENROLLICON  "persist.vendor.fingerprint.optical.enrollicon"
#define OPTICAL_AUTHICON       "persist.vendor.fingerprint.optical.authicon"
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"
#define PROPERTY_SCREEN_TYPE   "persist.vendor.fingerprint.optical.lcdtype"
// 0 calibrate not pass, 1 calibrate pass
#define FINGERPRINT_CALIPROP      "vendor.fingerprint.cali"

#define MAX_CAPTURE_FAIL_RETRY 2

namespace goodix {

CustomizedFingerprintCore::CustomizedFingerprintCore(HalContext *context)
    : DelmarFingerprintCore(context),
      mCaptureFailRetryCount(0),
      mScreenState(0) {
    char iconconfig[PROPERTY_VALUE_MAX] = {0};
    gf_error_t err = GF_SUCCESS;
    err = loadSensorConfig();
    if (err != GF_SUCCESS) {
        LOG_E(LOG_TAG, "[%s] get sensor config fail.", __func__);
    }

    err = getAlgoVersion();
    if (err != GF_SUCCESS) {
        LOG_E(LOG_TAG, "[%s] get ALGO_VERSION fail.", __func__);
    }
    initcheckcalibration();

    property_get(OPTICAL_ENROLLICON, iconconfig, "0");
    if (!strncmp(iconconfig, "0", sizeof("0"))) {
        enrollicon_config = 0;
        LOG_E(LOG_TAG, "[%s] enrollicon_config %d.", __func__, enrollicon_config);
    } else {
        enrollicon_config = 1;
        LOG_E(LOG_TAG, "[%s] enrollicon_config %d.", __func__, enrollicon_config);
    }
    property_get(OPTICAL_AUTHICON, iconconfig, "0");
    if (!strncmp(iconconfig, "0", sizeof("0"))) {
        authicon_config = 0;
        LOG_E(LOG_TAG, "[%s] authicon_config %d.", __func__, authicon_config);
    } else {
        authicon_config = 1;
        LOG_E(LOG_TAG, "[%s] authicon_config %d.", __func__, authicon_config);
    }
    mContext->mMsgBus.addMsgListener(this);
}

CustomizedFingerprintCore::~CustomizedFingerprintCore() {
    mContext->mMsgBus.removeMsgListener(this);
}

gf_error_t CustomizedFingerprintCore::loadSensorConfig() {
    gf_error_t err = GF_SUCCESS;
    int32_t ret = 0;
    DelmarSensor *sensor = (DelmarSensor *)mContext->mSensor;
    int32_t sensorNum = (int32_t)((DelmarSensor *)mContext->mSensor)->getAvailableSensorNum();
        int32_t productScreenId = ((CustomizedSensor*)sensor)->getProductScreenId();
    int32_t i = 0;
    mPressInfo.pressX = 0;
    mPressInfo.pressY = 0;
    mPressInfo.touchMajor = 0;
    mPressInfo.touchMinor = 0;
    mPressInfo.touchOrientation = 0;
    ret = property_get_int32(OPTICAL_ENROLLICON, 0);
    LOG_D(LOG_TAG, "[%s] get sensor enrollicon ret=<%d>.", __func__, ret);
    mPressInfo.enrollicon = ret;
    ret = property_get_int32(OPTICAL_AUTHICON, 0);
    LOG_D(LOG_TAG, "[%s] get sensor authicon ret=<%d>.", __func__, ret);
    mPressInfo.authicon = ret;

    for (i = 0; i < sensorNum; i++) {
        mSensorConfig.sensorRotation[i] = 0;
        mSensorConfig.sensorX[i] = 0;
        mSensorConfig.sensorY[i] = 0;
    }
    mSensorConfig.radius = 0;
    mSensorConfig.radiusInMM = 0;
        ((CustomizedSensorConfigProvider *)sensor->getSensorConfigProvider())->setProductScreenId(productScreenId);
    err = sensor->getSensorConfigProvider()->getConfig(&mSensorConfig, sensorNum);

    for (i = 0; i < sensorNum; i++) {
        LOG_D(LOG_TAG, "[%s] get sensor<%d> sensorRotation=<%.1f>.", __func__,
              i, mSensorConfig.sensorRotation[i]);
        LOG_D(LOG_TAG, "[%s] get sensor<%d> sensorX=<%d>.", __func__,
              i, mSensorConfig.sensorX[i]);
        LOG_D(LOG_TAG, "[%s] get sensor<%d> sensorY=<%d>.", __func__,
              i, mSensorConfig.sensorY[i]);
    }
    LOG_D(LOG_TAG, "[%s] get sensor radius=<%d>.", __func__,
          mSensorConfig.radius);
    LOG_D(LOG_TAG, "[%s] get sensor radiusInMM=<%f>.", __func__,
          mSensorConfig.radiusInMM);
    return err;
}

void CustomizedFingerprintCore::onError(gf_error_t err) {
    if (err == GF_ERROR_NEED_CANCLE_ENROLL) {
        notifyErrorInfo(GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
    } else {
        DelmarFingerprintCore::onError(err);
    }
}
// notify message
gf_error_t CustomizedFingerprintCore::notifyAcquiredInfo(gf_fingerprint_acquired_info_t
        info) {
    gf_error_t err = GF_SUCCESS;
#if 1  // no customizedInfo by now
    gf_fingerprint_msg_t message;
    memset(&message, 0, sizeof(gf_fingerprint_msg_t));
    FUNC_ENTER();

    if (info > GF_FINGERPRINT_ACQUIRED_VENDOR_BASE) {
        switch (info) {
        case GF_FINGERPRINT_ACQUIRED_FINGER_DOWN: {
            notifyTouch(GF_FINGERPRINT_TOUCH_DOWN);
            break;
        }

        case GF_FINGERPRINT_ACQUIRED_FINGER_UP: {
            notifyTouch(GF_FINGERPRINT_TOUCH_UP);
            break;
        }

        default : {
            err = FingerprintCore::notifyAcquiredInfo(info);
            break;
        }
        }
    } else {
        err = FingerprintCore::notifyAcquiredInfo(info);
    }

#endif  // no customizedInfo by now

    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::notifyTouch(gf_fingerprint_msg_type_t type) {
    gf_error_t err = GF_SUCCESS;
    gf_fingerprint_msg_t message;
    memset(&message, 0, sizeof(gf_fingerprint_msg_t));
    FUNC_ENTER();
    if (nullptr != mNotify) {
        if (type == GF_FINGERPRINT_TOUCH_UP) {
            FingerprintCore::notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_FINGER_UP);
            setAuthUpNotified(true);
            LOG_I(LOG_TAG, "[%s] notifyTouch Up ", __func__);
        } else {
            message.data.tp_info.x = mPressInfo.pressX;
            message.data.tp_info.y = mPressInfo.pressY;
            if (((getWorkState() == STATE_ENROLL) && (enrollicon_config == 0))
                    || ((getWorkState() == STATE_AUTHENTICATE) && (authicon_config == 0))) {
                message.data.tp_info.touch_major = 0;
                message.data.tp_info.touch_minor = 0;
                message.data.tp_info.touch_angle = 0;
                LOG_I(LOG_TAG, "[%s] notifyTouch  DELMAR_SENSOR_UI_TYPE_CIRCLE ", __func__);
            } else if (((getWorkState() == STATE_ENROLL) && (enrollicon_config == 1))
                    || ((getWorkState() == STATE_AUTHENTICATE) && (authicon_config == 1))) {
                message.data.tp_info.touch_major = mPressInfo.touchMajor;
                message.data.tp_info.touch_minor = mPressInfo.touchMinor;
                message.data.tp_info.touch_angle = mPressInfo.touchOrientation;
                LOG_I(LOG_TAG, "[%s] notifyTouch DELMAR_SENSOR_UI_TYPE_ELLIPSE ", __func__);
            } else {
                LOG_E(LOG_TAG, "[%s] notifyTouch err  ", __func__);
            }
            LOG_D(LOG_TAG, "[%s] notifyTouch Down type=%d, pressX=%d ,pressY=%d ", __func__, type,
                  message.data.tp_info.x, message.data.tp_info.y);
            LOG_D(LOG_TAG, "[%s] notifyTouch Down touch_major=%d, touch_minor=%d ,touch_angle=%d ", __func__,
                  message.data.tp_info.touch_major, message.data.tp_info.touch_minor,
                  message.data.tp_info.touch_angle);
        }

        message.type = type;
        mNotify(&message);
    }

    FUNC_EXIT(err);
    return err;
}

void setTarceTag(const char * name , bool begin) {
#ifdef FP_TRACE_TAG_ENABLE
    if (begin) {
        atrace_begin(ATRACE_TAG, name);
    } else {
        atrace_end(ATRACE_TAG);
    }
#endif  // FP_TRACE_DEBUG
}


void CustomizedFingerprintCore::setSensorInfo() {
    VOID_FUNC_ENTER();
    //LOG_E(LOG_TAG, "[%s]  err WorkState =%d ", __func__,WorkState);
    gf_delmar_coordinate_info_t coordinateInfo = { 0 };
    gf_delmar_press_area_info_t ellipseInfo = { 0 };
    DelmarSensor *sensor = (DelmarSensor *)mContext->mSensor;
    int32_t sensorNum = (int32_t)sensor->getAvailableSensorNum();

    for (int i = 0; i < sensorNum; i++) {
        coordinateInfo.sensor_x[i] = mSensorConfig.sensorX[i];
        coordinateInfo.sensor_y[i] = mSensorConfig.sensorY[i];
        coordinateInfo.sensor_rotation[i] = mSensorConfig.sensorRotation[i];
    }

    coordinateInfo.press_x = mPressInfo.pressX;
    coordinateInfo.press_y = mPressInfo.pressY;
    coordinateInfo.radius = mSensorConfig.radius;
    sensor->setCoordinateInfo(&coordinateInfo);

    if ((getWorkState() == STATE_ENROLL && mPressInfo.enrollicon == 1) ||
            (getWorkState() == STATE_AUTHENTICATE && mPressInfo.authicon == 1)) {
        sensor->setSensorUIType(DELMAR_SENSOR_UI_TYPE_ELLIPSE);
        ellipseInfo.touch_major = mPressInfo.touchMajor;
        ellipseInfo.touch_minor = mPressInfo.touchMinor;
        ellipseInfo.touch_orientation = 90 - mPressInfo.touchOrientation;
        sensor->setSensorAreaInfo(&ellipseInfo);
    } else {
        sensor->setSensorUIType(DELMAR_SENSOR_UI_TYPE_CIRCLE);
    }

    sensor->updateSensorPriority();
    VOID_FUNC_EXIT();
    return;
}

gf_error_t CustomizedFingerprintCore::onEnrollStart(EnrollContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    mCaptureFailRetryCount = 0;
    setSensorInfo();
    err = DelmarFingerprintCore::onEnrollStart(context);

    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onAuthStart(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    mCaptureFailRetryCount = 0;
    setSensorInfo();
    mContext->mDevice->setAuthScreenState(mScreenState);
    FingerprintCore::setUxThread(1);
    setTarceTag("wakeupSensor", true);
    err = DelmarFingerprintCore::onAuthStart(context);
    setTarceTag("wakeupSensor", false);

    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onAuthStop(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = DelmarFingerprintCore::onAuthStop(context);
    FingerprintCore::setUxThread(0);
    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onBeforeAuthCapture(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    setTarceTag("waitUIready", true);
    err = DelmarFingerprintCore::onBeforeAuthCapture(context);
    setTarceTag("waitUIready", false);
    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onAfterAuthCapture(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    setTarceTag("readImage", true);
    err = DelmarFingerprintCore::onAfterAuthCapture(context);
    setTarceTag("readImage", false);
    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onBeforeAuthAlgo(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = DelmarFingerprintCore::onBeforeAuthAlgo(context);
    setTarceTag("authImage", true);
    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onAfterAuthAlgo(AuthenticateContext *context) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    setTarceTag("authImage", false);
    err = DelmarFingerprintCore::onAfterAuthAlgo(context);
    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onSensorPressInfo(int32_t touchCenterX, int32_t touchCenterY,
        int32_t touchMajor, int32_t touchMinor, int32_t touchOrientation) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    mPressInfo.pressX = touchCenterX;
    mPressInfo.pressY = touchCenterY;
    mPressInfo.touchMajor = touchMajor;
    mPressInfo.touchMinor = touchMinor;
    mPressInfo.touchOrientation = touchOrientation;

    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::onMessage(const MsgBus::Message &message) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    switch (message.msg) {
    case MsgBus::MSG_AUTHENTICATE_RETRYING:
    case MsgBus::MSG_AUTHENTICATE_END: {
        mCaptureFailRetryCount = 0;
        getAuthDetail(message);
        break;
    }
    case MsgBus::MSG_SCREEN_STATE: {
        mScreenState = (message.params1 > 0) ? 1 : 0;
        break;
    }
    default:
        break;
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::getAuthDetail(const MsgBus::Message &msg) {
    gf_error_t err = GF_SUCCESS;
    gf_error_t result = (gf_error_t)msg.params1;
    uint32_t retry = (uint32_t)msg.params2;
    FUNC_ENTER();
    do {
        LOG_D(LOG_TAG, "[%s] result=%d.", __func__, result);
        if (result != GF_ERROR_TOO_FAST) {
            detail = { { 0 } };
            detail.header.target = GF_TARGET_ALGO;
            detail.header.cmd_id = GF_CUSTOMIZED_CMD_GET_AUTH_DETAIL;
            detail.i_dsp_applied = mContext->isDSPEnabled();
            err = invokeCommand(&detail, sizeof(gf_customized_auth_detail_get_t));
            GF_ERROR_BREAK(err);
            LOG_D(LOG_TAG, "[%s] image_quality=%d.", __func__,
                  detail.o_image_quality);
            LOG_D(LOG_TAG, "[%s] valid_area=%d.", __func__,
                  detail.o_valid_area);
            LOG_D(LOG_TAG, "[%s] match_score=%d.", __func__,
                  (detail.o_match_score >= 0) ? (detail.o_match_score) : (0));
            LOG_D(LOG_TAG, "[%s] sig_val=%d.", __func__,
                  detail.o_sig_val);
            LOG_D(LOG_TAG, "[%s] fake_result=%d.", __func__,
                  detail.o_fake_result);
            LOG_D(LOG_TAG, "[%s] retry=%d.", __func__, retry);
            LOG_D(LOG_TAG, "[%s] gain=0x%02x.", __func__, detail.o_gain);
            LOG_D(LOG_TAG, "[%s] detect_fake_time=%dms.", __func__,
                  detail.o_detect_fake_time);
            LOG_D(LOG_TAG, "[%s] preprocess_time=%dms.", __func__,
                  detail.o_preprocess_time);
            LOG_D(LOG_TAG, "[%s] get_feature_time=%dms.", __func__,
                  detail.o_get_feature_time);
            LOG_D(LOG_TAG, "[%s] authenticate_time=%dms.", __func__,
                  detail.o_authenticate_time);
            // fake result
            for (uint8_t i = 0; i < ARRAY_LEN(detail.o_algo_fake_result); i++) {
                LOG_I(LOG_TAG, "[%s] algo point fake_result[%d] = %d.", __func__, i,
                        detail.o_algo_fake_result[i]);
            }
            LOG_I(LOG_TAG, "[%s] algo point core_result = %d.", __func__, detail.o_algo_core_result);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

bool CustomizedFingerprintCore::needRetryIfCaptureFailed(AuthenticateContext *context) {
    FUNC_ENTER();
    if (context->result == GF_ERROR_SPI_RAW_DATA_CRC_FAILED
            && mCaptureFailRetryCount < MAX_CAPTURE_FAIL_RETRY) {
        mCaptureFailRetryCount++;
        return true;
    }
    return false;
}

int32_t getLcdType(char *lcdtype) {
    int32_t ret = 0;
    char lcdtype_prop[PROPERTY_VALUE_MAX] = {0};
    ret = property_get(PROPERTY_SCREEN_TYPE, lcdtype_prop, "");
    if (ret) {
        const char * cutstr =  strstr(lcdtype_prop, "_");
        if (cutstr == NULL) {
            memcpy(lcdtype, lcdtype_prop, strlen(lcdtype_prop));
        } else {
            if (strlen(cutstr) - 1 > 0) {
                memcpy(lcdtype, cutstr+1, strlen(cutstr) - 1);
            } else {
                memcpy(lcdtype, lcdtype_prop, strlen(lcdtype_prop));
            }
        }
    }
    return ret;
}

gf_error_t CustomizedFingerprintCore::getAlgoVersion() {
    gf_error_t err = GF_SUCCESS;
    int ret = 0;
    char lcdtype_prop[PROPERTY_VALUE_MAX] = {0};
    FUNC_ENTER();
    do {
        gf_customized_algo_version_info_t cmd;
        memset(&cmd, 0, sizeof(gf_customized_algo_version_info_t));
        cmd.header.target = GF_TARGET_ALGO;
        cmd.header.cmd_id = GF_CUSTOMIZED_CMD_GET_ALGOVERSION;
        err = invokeCommand(&cmd, sizeof(cmd));
        GF_ERROR_BREAK(err);
        memcpy(mALGO_VERSION, cmd.version_info, sizeof(cmd.version_info));
    } while (0);
    ret = getLcdType(lcdtype_prop);
    if (ret) {
        strcat(mALGO_VERSION, "_");
        strncpy(mALGO_VERSION + strlen(mALGO_VERSION), lcdtype_prop, strlen(lcdtype_prop));
        LOG_E(LOG_TAG, "[%s] lcdtype_prop = %s", __func__, lcdtype_prop);
    } else {
        LOG_E(LOG_TAG, "[%s] no lcdtype_prop", __func__);
    }
    property_set(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, mALGO_VERSION);
    LOG_E(LOG_TAG, "[%s] ALGO_VERSION: %s.", __func__, mALGO_VERSION);
    FUNC_EXIT(err);
    return err;
}

gf_error_t CustomizedFingerprintCore::send_auth_dcsmsg(AuthenticateContext *context,
        bool notified) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (notified != true) {
        LOG_D(LOG_TAG, "[%s] no need to add this result to dcs. result = %d", __func__, context->result);
        return GF_SUCCESS;
    }

    // REMARK:OPLUS particular send dcsmsg
    notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_GOOD);
    if (mNotify != nullptr) {
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        message.type = GF_FINGERPRINT_AUTHENTICATED_DCSSTATUS;

        message.data.auth_dcsmsg.auth_result = (context->result == GF_SUCCESS) ? 1 : 0;

        message.data.auth_dcsmsg.fail_reason = context->result;
        message.data.auth_dcsmsg.quality_score = detail.o_image_quality;//invalid
        message.data.auth_dcsmsg.match_score = (detail.o_match_score >= 0) ? (detail.o_match_score) : (0);//invalid
        message.data.auth_dcsmsg.signal_value = detail.o_sig_val;//invalid
        message.data.auth_dcsmsg.img_area = 0;//invalid
        message.data.auth_dcsmsg.retry_times = context->retry;
        memcpy(message.data.auth_dcsmsg.algo_version, mALGO_VERSION, strlen(mALGO_VERSION));
        message.data.auth_dcsmsg.chip_ic = 0;//invalid
        message.data.auth_dcsmsg.module_type = 0;//invalid
        message.data.auth_dcsmsg.lense_type = 0;//invalid
        message.data.auth_dcsmsg.dsp_availalbe = 0;//invalid
        message.data.auth_dcsmsg.auth_total_time = FingerprintCore::authTotalTime / 1000.0;
        message.data.auth_dcsmsg.ui_ready_time = FingerprintCore::uiReadyTime / 1000.0;
        message.data.auth_dcsmsg.capture_time = FingerprintCore::captureTime / 1000.0;
        message.data.auth_dcsmsg.preprocess_time = detail.o_preprocess_time;
        message.data.auth_dcsmsg.get_feature_time = detail.o_get_feature_time;
        message.data.auth_dcsmsg.auth_time = detail.o_authenticate_time;
        message.data.auth_dcsmsg.detect_fake_time = detail.o_detect_fake_time;

        LOG_D(LOG_TAG, "[%s] Auth, fail_reason = %d", __func__, message.data.auth_dcsmsg.fail_reason);
        LOG_D(LOG_TAG, "[%s] Auth, quality_score = %d", __func__, message.data.auth_dcsmsg.quality_score);
        LOG_D(LOG_TAG, "[%s] Auth, match_score = %d", __func__,  message.data.auth_dcsmsg.match_score);
        LOG_D(LOG_TAG, "[%s] Auth, signal_value = %d", __func__,  message.data.auth_dcsmsg.signal_value);
        LOG_D(LOG_TAG, "[%s] Auth, img_area = %d", __func__, message.data.auth_dcsmsg.img_area);
        LOG_D(LOG_TAG, "[%s] Auth, retry_times = %d", __func__, message.data.auth_dcsmsg.retry_times);
        LOG_D(LOG_TAG, "[%s] Auth, algo_version = %s", __func__, message.data.auth_dcsmsg.algo_version);
        LOG_D(LOG_TAG, "[%s] Auth, chip_ic = %d", __func__,  message.data.auth_dcsmsg.chip_ic);
        LOG_D(LOG_TAG, "[%s] Auth, module_type = %d", __func__, message.data.auth_dcsmsg.module_type);
        LOG_D(LOG_TAG, "[%s] Auth, lense_type = %d", __func__, message.data.auth_dcsmsg.lense_type);
        LOG_D(LOG_TAG, "[%s] Auth, dsp_available = %d", __func__, message.data.auth_dcsmsg.dsp_availalbe);

        LOG_D(LOG_TAG, "[%s] Auth, auth_total_time = %d", __func__,
              message.data.auth_dcsmsg.auth_total_time);
        LOG_D(LOG_TAG, "[%s] Auth, ui_ready_time = %d", __func__, message.data.auth_dcsmsg.ui_ready_time);
        LOG_D(LOG_TAG, "[%s] Auth, capture_time = %d", __func__, message.data.auth_dcsmsg.capture_time);
        LOG_D(LOG_TAG, "[%s] Auth, preprocess_time = %d", __func__,
              message.data.auth_dcsmsg.preprocess_time);
        LOG_D(LOG_TAG, "[%s] Auth, get_feature_time = %d", __func__,
              message.data.auth_dcsmsg.get_feature_time);
        LOG_D(LOG_TAG, "[%s] Auth, auth_time = %d", __func__, message.data.auth_dcsmsg.auth_time);
        LOG_D(LOG_TAG, "[%s] Auth, detect_fake_time = %d", __func__,
              message.data.auth_dcsmsg.detect_fake_time);
        mNotify(&message);
    }

    FUNC_EXIT(err);
    return err;
}

void CustomizedFingerprintCore::initcheckcalibration() {
    VOID_FUNC_ENTER();
    gf_error_t ret = GF_SUCCESS;

    ret = DelmarFingerprintCore::checkEnrollAuthReady();
    if (GF_SUCCESS == ret) {
        property_set(FINGERPRINT_CALIPROP, "1");
        LOG_D(LOG_TAG, "[%s] spmt is pass", __func__);
    } else {
        property_set(FINGERPRINT_CALIPROP, "0");
        LOG_E(LOG_TAG, "[%s] spmt is not pass, please to do the process of spmt", __func__);
    }
    VOID_FUNC_EXIT();
}
static bool isForcePrintKpi() {
    return true;
}

gf_error_t CustomizedFingerprintCore::dumpKPI(const char *func_name) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do {
        if (nullptr == func_name) {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] func_name is nullptr", __func__);
            break;
        }
        /*dump kpi logs is not supported on release version*/
        if (GF_LOG_LEVEL > GF_LOG_INFO_LEVEL || isForcePrintKpi()) {
            // get kpi
            gf_delmar_algo_kpi_t kpi = { { 0 } };
            kpi.header.target = GF_TARGET_ALGO;
            kpi.header.cmd_id = GF_CMD_ALGO_KPI;
            gf_error_t err = invokeCommand(&kpi, sizeof(gf_delmar_algo_kpi_t));

            if (GF_SUCCESS != err) {
                break;
            }

            err = DelmarHalUtils::printKpiPerformance(getWorkState(),
                    mTotalKpiTime, &kpi.o_performance, mContext, func_name, isForcePrintKpi());
            if (err != GF_SUCCESS) {
                break;
            }
        }
    } while (0);
    FUNC_EXIT(err);
    return err;
}

}  // namespace goodix
