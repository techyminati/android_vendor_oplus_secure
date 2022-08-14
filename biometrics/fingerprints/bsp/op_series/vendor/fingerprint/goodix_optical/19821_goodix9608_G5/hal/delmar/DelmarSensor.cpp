/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][DelmarSensor]"

#include <stdlib.h>
#include <string.h>
#include <cutils/fs.h>
#include <cutils/properties.h>

#include "HalContext.h"
#include "DelmarSensor.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "FingerprintCore.h"
#include "HalUtils.h"
#include "Device.h"
#include "gf_base_types.h"
#include "SensorConfigProvider.h"
#include "DelmarHalUtils.h"
#ifdef SUPPORT_GET_COORDINATE_DURING_CAPTURE
#include "DelmarTouchEventUtils.h"
#endif  // SUPPORT_GET_COORDINATE_DURING_CAPTURE
#include "CaEntry.h"

#define GF_OTP_INFO_FILENAME "gf_secure_obj_1.so"

namespace goodix {
    DelmarSensor::DelmarSensor(HalContext *context, SensorConfigProvider *provider) :
            Sensor(context),
            mSensorIds(0),
            mRetrySensorIds(0),
            mSensorUIType(0),
            mHasPressAreaInfo(0),
            mCoordinateInfo({0}),
            mPressAreaInfo({0}),
            mSensorType(DELMAR_SENSOR_TYPE_UNKOWN),
            mSensorNum(0),
            mNotifyRstFlag(1),
            mAuthRetryCount(0),
            mCaliDataState(0) {
        if (provider == nullptr) {
            mpProvider = new SensorConfigProvider();
        } else {
            mpProvider = provider;
        }

        mMorphotype = 0;
        mCfMark = 0;
        mCfMaskType = 0;
    }

    DelmarSensor::~DelmarSensor() {
        delete mpProvider;
        mpProvider = nullptr;
    }

    gf_error_t DelmarSensor::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            uint8_t hal_otp[MAX_SENSOR_NUM][DELMAR_OTP_INFO_BUFFER_LEN] = {{ 0 }};
            gf_delmar_sensor_init_t cmd = {{ 0 }};
            gf_cmd_header_t *header = (gf_cmd_header_t *)&cmd;
            header->target = GF_TARGET_SENSOR;
            header->cmd_id = GF_CMD_SENSOR_INIT;
            // MAX_SENSOR_NUM may be larger than available sensor count
            for (int32_t i =0; i < MAX_SENSOR_NUM; i++) {
                char filename[GF_MAX_FILE_NAME_LEN] = { 0 };
                snprintf(filename, sizeof(filename), "%s_%d", GF_OTP_INFO_FILENAME, i);
                err = HalUtils::loadDataFromSdcard(filename, hal_otp[i], DELMAR_OTP_INFO_BUFFER_LEN);
                if (err == GF_SUCCESS) {
                    memcpy(cmd.io_sdcard_otp[i], hal_otp[i], DELMAR_OTP_INFO_BUFFER_LEN);
                    cmd.io_sdcard_otp_len[i] =  DELMAR_OTP_INFO_BUFFER_LEN;
                }
            }
            char buf[PROPERTY_VALUE_MAX] = {0};
            int32_t ret = 0;
            ret = property_get("vendor.boot.verifiedbootstate", buf, NULL);
            LOG_D(LOG_TAG, "verifiedbootstate: %s ret %d", buf, ret);
            if (0 != strcmp(buf, "orange")) {
                cmd.file_unlock_en = 0;
            } else {
                cmd.file_unlock_en = 1;
            }
            cmd.file_unlock_en = 1;

            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            mSensorType = cmd.o_sensor_type;
            mSensorNum = cmd.o_sensor_num;
            mMorphotype = cmd.o_morphotype;
            mCfMark = cmd.o_cf_mark;
            mCfMaskType = cmd.o_cf_mask_type;
            mCaliDataState = cmd.o_cali_state;
            LOG_D(LOG_TAG, "[%s] mSensorType:%d, mSensorNum:%u, morphotype:%u, cfMark:%d, cfMaskType:%u, mCaliDataState=0x%08x", \
                __func__, mSensorType, mSensorNum, mMorphotype, mCfMark, mCfMaskType, mCaliDataState);
            gf_delmar_config_t *config = new gf_delmar_config_t();
            memcpy(config, &cmd.o_delmar_config, sizeof(gf_delmar_config_t));
            mContext->mConfig = (gf_config_t *)config;
            int32_t i = 0;
            for (i = 0; i < (int32_t) mSensorNum; i++) {
                if(i >= MAX_SENSOR_NUM) {
                    break;
                }
                char filename[GF_MAX_FILE_NAME_LEN] = { 0 };
                snprintf(filename, sizeof(filename), "%s_%d", GF_OTP_INFO_FILENAME, i);
                if (cmd.io_sdcard_otp_len[i] !=0 && memcmp(cmd.io_sdcard_otp[i], hal_otp[i], cmd.io_sdcard_otp_len[i]) != 0) {
                    LOG_I(LOG_TAG, "[%s] update new otp data to HAL storage", __func__);
                    HalUtils::saveDataToSdcard(filename, cmd.io_sdcard_otp[i], DELMAR_OTP_INFO_BUFFER_LEN);
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarSensor::captureImage(int32_t op, uint32_t retry) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (op == GF_OPERATION_AUTHENTICATE) {
                if (retry == mAuthRetryCount && retry > 0) {
                    LOG_D(LOG_TAG, "[%s] no need to send capture cmd again for auth retry, ignore it.", __func__);
                    LOG_D(LOG_TAG, "[%s] It has been done by moving up to onAfterAuthAlgo().", __func__);
                    break;
                }
                mAuthRetryCount = retry;
            }
            if (op == GF_OPERATION_ENROLL) {
                retry = 1;
            }
            if (mContext->mCenter->hasUpEvt() || (mContext->mConfig->support_ui_ready
                                                  && !isSensorUIReady())) {
                err = GF_ERROR_TOO_FAST;
                break;
            }
            gf_delmar_capture_image_t cmd = {{ {0} }};
            cmd.common.header.target = GF_TARGET_SENSOR;
            cmd.common.header.cmd_id = GF_CMD_SENSOR_CAPTURE_IMG;
            cmd.common.i_operation = op;
            cmd.common.i_retry_count = retry;
            cmd.valid_sensor_ids = mSensorIds;
#ifdef SUPPORT_GET_COORDINATE_DURING_CAPTURE
            if (mSensorType == DELMAR_SENSOR_TYPE_SINGLE_T) {
                DelmarTouchEventUtils::start();
            }
#endif  // SUPPORT_GET_COORDINATE_DURING_CAPTURE
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);

            if (mContext->mCenter->hasUpEvt() || (mContext->mConfig->support_ui_ready
                                                  && !isSensorUIReady())) {
                err = GF_ERROR_TOO_FAST;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarSensor::readImage(uint32_t retry, uint64_t sensorIds) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_read_image_t cmd = {{ 0 }};

        FUNC_ENTER();
        UNUSED_VAR(retry);

        do {
            if (mContext->mCenter->hasUpEvt()) {
                err = GF_ERROR_TOO_FAST;
                break;
            }
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_DELMAR_SENSOR_READ_IMAGE;
            cmd.i_sensor_ids = sensorIds;
            LOG_D(LOG_TAG, "[%s] read image, sensor ids: %08x%08x", __func__,
                    SPLIT_UINT64_TO_UINT32_ARGS(sensorIds));
            cmd.i_ui_type = mSensorUIType;
            LOG_D(LOG_TAG, "[%s] calculateMask, sensor ui type: %d", __func__, mSensorUIType);
            cmd.i_has_press_area_info = mHasPressAreaInfo;
            LOG_D(LOG_TAG, "[%s] calculateMask, mHasPressAreaInfo: %d", __func__, mHasPressAreaInfo);
            cmd.coordinate_info = mCoordinateInfo;
            cmd.press_area_info = mPressAreaInfo;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err == GF_SUCCESS) {
                LOG_I(LOG_TAG, "[%s] readImage and calculate mask success.", __func__);
            } else {
                LOG_I(LOG_TAG, "[%s] readImage and calculate mask failed.", __func__);
            }
            GF_ERROR_BREAK(err);

            if (mContext->mCenter->hasUpEvt()) {
                err = GF_ERROR_TOO_FAST;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }


    gf_error_t DelmarSensor::calculateMask(uint64_t sensorIds) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_calculate_mask_t cmd = {{ 0 }};

        FUNC_ENTER();

        do {
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_DELMAR_SENSOR_CALCULATE_MASK;
            cmd.i_sensor_ids = sensorIds;
            LOG_D(LOG_TAG, "[%s] calculateMask, sensor ids: %08x%08x", __func__,
                    SPLIT_UINT64_TO_UINT32_ARGS(sensorIds));
            cmd.i_ui_type = mSensorUIType;
            LOG_D(LOG_TAG, "[%s] calculateMask, sensor ui type: %d", __func__, mSensorUIType);
            cmd.i_has_press_area_info = mHasPressAreaInfo;
            LOG_D(LOG_TAG, "[%s] calculateMask, mHasPressAreaInfo: %d", __func__, mHasPressAreaInfo);
            cmd.coordinate_info = mCoordinateInfo;
            cmd.press_area_info = mPressAreaInfo;
            err = invokeCommand(&cmd, sizeof(cmd));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void DelmarSensor::updateSensorPriority() {
        int64_t sensorIds = 0;
        if (mSensorUIType == DELMAR_SENSOR_UI_TYPE_CIRCLE) {
            sensorIds = DelmarHalUtils::calculateCircleSensorPriority(
                    &mCoordinateInfo, mSensorNum, mMorphotype);
        } else {
            sensorIds = DelmarHalUtils::calculateEllipseSensorPriority(
                    &mCoordinateInfo, &mPressAreaInfo, mSensorNum,
                    (int32_t) mContext->mSensorInfo.col, (int32_t) mContext->mSensorInfo.row,
                    (gf_delmar_config_t *) mContext->mConfig, mMorphotype);
        }
        setSensorIds(sensorIds, DelmarHalUtils::isInCenterMode(sensorIds, mMorphotype));
    }

    void DelmarSensor::switchRetrySensorIds() {
        if (mRetrySensorIds > 0) {
            mSensorIds = mRetrySensorIds;
            mRetrySensorIds = 0;
        }
    }

    void DelmarSensor::setSensorIds(uint64_t sensorIds, uint32_t sensorsCenterMode) {
        LOG_D(LOG_TAG, "[%s] set sensor ids: 0x%08x%08x", __func__,
                SPLIT_UINT64_TO_UINT32_ARGS(sensorIds));
        LOG_D(LOG_TAG, "[%s] sensorsCenterMode: %d", __func__, sensorsCenterMode);
        if (sensorsCenterMode) {
            mSensorIds = sensorIds & 0xFFFF;
            mRetrySensorIds = (sensorIds >> 16) & 0xFFFF;
        } else {
            mSensorIds = sensorIds;
            mRetrySensorIds = 0;
        }
        LOG_D(LOG_TAG, "[%s] mSensorIds: 0x%08x%08x", __func__,
                SPLIT_UINT64_TO_UINT32_ARGS(mSensorIds));
        LOG_D(LOG_TAG, "[%s] mRetrySensorIds: 0x%08x%08x", __func__,
                SPLIT_UINT64_TO_UINT32_ARGS(mRetrySensorIds));
    }

    void DelmarSensor::setCoordinateInfo(gf_delmar_coordinate_info_t *coordinate_info) {
        VOID_FUNC_ENTER();
        memcpy(&mCoordinateInfo, coordinate_info, sizeof(gf_delmar_coordinate_info_t));
        VOID_FUNC_EXIT();
    }

    void DelmarSensor::setSensorAreaInfo(gf_delmar_press_area_info_t *press_area_info) {
        VOID_FUNC_ENTER();
        mHasPressAreaInfo = 1;
        memcpy(&mPressAreaInfo, press_area_info, sizeof(gf_delmar_press_area_info_t));
        VOID_FUNC_EXIT();
    }

    gf_error_t DelmarSensor::wakeupSensor() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            Mutex::Autolock _l(mContext->mWakeUpLock);
            // no need to reset for sensor without mcu
            if (mSensorType == DELMAR_SENSOR_TYPE_GM182) {
                err = mContext->mDevice->reset();
            }
            if (err == GF_SUCCESS) {
                err = doWakeup(mContext);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarSensor::sleepSensor() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
#ifdef SUPPORT_GET_COORDINATE_DURING_CAPTURE
            if (mSensorType == DELMAR_SENSOR_TYPE_SINGLE_T) {
                DelmarTouchEventUtils::stop();
            }
#endif   // SUPPORT_GET_COORDINATE_DURING_CAPTURE
            Sensor::sleepSensor();
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_event_type_t DelmarSensor::getIrqEventType() {
        gf_event_type_t event = EVENT_UNKNOWN;
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            // only response irq in wakeup state
            if (mSensorSleep == SLEEP) {
                LOG_D(LOG_TAG, "[%s] ignore irq when sleep", __func__);
                break;
            }
            gf_get_irq_type_cmd_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_GET_IRQ_TYPE;
            err = invokeCommand(&cmd, sizeof(gf_get_irq_type_cmd_t));

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] retrieve irq type fail", __func__);
                break;
            } else {
                event = (gf_event_type_t)cmd.o_irq_type;
                LOG_D(LOG_TAG, "[%s] retrieved event: %d from irq", __func__, event);
            }
        } while (0);

        FUNC_EXIT(err);
        return event;
    }

    gf_error_t DelmarSensor::onMessage(const MsgBus::Message &msg) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        switch (msg.msg) {
            case MsgBus::MSG_HARDWARE_RESET: {
                // notify ta hw reset apply for sync status
                LOG_D(LOG_TAG, "[%s] mSensorType=%d, mNotifyRstFlag=%d", __func__,
                                     mSensorType, mNotifyRstFlag);
                if (mSensorType == DELMAR_SENSOR_TYPE_SINGLE_T && mNotifyRstFlag) {
                    gf_cmd_header_t cmd = { 0 };
                    cmd.target = GF_TARGET_SENSOR;
                    cmd.cmd_id = GF_CMD_DELMAR_SENSOR_NOTIFY_RESET;
                    // here call CaEntry::sendCommand, but not HalBase::invokeCommand,
                    // it's to avoid endless loop when ta is dead
                    err = mContext->invokeCommand(&cmd, sizeof(cmd));
                }
                break;
            }
            case MsgBus::MSG_EVENT_QUEUED: {
                clearSensorUIReady();
                break;
            }
            default: {
                LOG_D(LOG_TAG, "msg : %d", msg.msg);
                err = Sensor::onMessage(msg);
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
