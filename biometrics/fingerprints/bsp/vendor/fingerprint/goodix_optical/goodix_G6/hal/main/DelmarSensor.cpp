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
#include "CaEntry.h"
#define PROPERTY_FINGERPRINT_QRCODE "oplus.fingerprint.qrcode.support"
#define PROPERTY_FINGERPRINT_QRCODE_VALUE "oplus.fingerprint.qrcode.value"

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
            mCaliDataState(0),
            isMultiFingerAuth(0),
            mClearCacheImage(1),
            mSensorIdLen(0),
            mSensorBgVersion(0) {
        if (provider == nullptr) {
            mpProvider = new SensorConfigProvider();
        } else {
            mpProvider = provider;
        }
        mPressX[0] = 0;
        mPressX[1] = 0;
        mPressY[0] = 0;
        mPressY[1] = 0;

        mMorphotype = 0;
        mCfMark = 0;
        mCfMaskType = 0;
        mOpticalType = 0;
        mBrightnessLevel = 0;
        memset(mQRCode, 0, sizeof(mQRCode));
    }

    DelmarSensor::~DelmarSensor() {
        delete mpProvider;
        mpProvider = nullptr;
    }

    gf_error_t DelmarSensor::init() {
        gf_error_t err = GF_SUCCESS;
        int32_t size = 0;
        gf_delmar_sensor_init_t* cmd = NULL;
        gf_cmd_header_t *header = NULL;
        FUNC_ENTER();

        do {
            uint8_t hal_otp[MAX_SENSOR_NUM][DELMAR_OTP_INFO_BUFFER_LEN] = {{ 0 }};
            err = createInitCmd(&cmd, &size);
            if (err != GF_SUCCESS) {
                break;
            }
            header = (gf_cmd_header_t *)cmd;
            header->target = GF_TARGET_SENSOR;
            header->cmd_id = GF_CMD_SENSOR_INIT;
            // MAX_SENSOR_NUM may be larger than available sensor count
            for (int32_t i =0; i < MAX_SENSOR_NUM; i++) {
                char filename[GF_MAX_FILE_NAME_LEN] = { 0 };
                snprintf(filename, sizeof(filename), "%s_%d", GF_OTP_INFO_FILENAME, i);
                err = HalUtils::loadDataFromSdcard(filename, hal_otp[i], DELMAR_OTP_INFO_BUFFER_LEN);
                if (err == GF_SUCCESS) {
                    memcpy(cmd->io_sdcard_otp[i], hal_otp[i], DELMAR_OTP_INFO_BUFFER_LEN);
                    cmd->io_sdcard_otp_len[i] =  DELMAR_OTP_INFO_BUFFER_LEN;
                }
            }
            if (isDeviceUnlocked()) {
                cmd->i_device_unlocked = 1;
                LOG_D(LOG_TAG, "[%s] device run in unlocked state!", __func__);
            }
            err = invokeCommand(cmd, size);
            GF_ERROR_BREAK(err);
            mSensorType = cmd->o_sensor_type;
            mSensorNum = cmd->o_sensor_num;
            mMorphotype = cmd->o_morphotype;
            mCfMark = cmd->o_cf_mark;
            mCfMaskType = cmd->o_cf_mask_type;
            mOpticalType = cmd->o_optical_type;
            mCaliDataState = cmd->o_cali_state;
            mSensorIdLen = cmd->o_sensor_id_len;
            mSensorBgVersion = cmd->o_sensor_bg_version;
            LOG_D(LOG_TAG, "[%s] mSensorType:%d, mSensorNum:%u, morphotype:%u, cfMark:%d, cfMaskType:%u, mCaliDataState=0x%08x, mOpticalType=%d", \
                __func__, mSensorType, mSensorNum, mMorphotype, mCfMark, mCfMaskType, mCaliDataState, mOpticalType);
            memcpy(mQRCode, cmd->o_qr_code, sizeof(mQRCode));
            LOG_D(LOG_TAG, "[%s] mQRCode=%s,len=%d", __func__, mQRCode,strlen((const char *)mQRCode));
            property_set(PROPERTY_FINGERPRINT_QRCODE_VALUE, (const char *)mQRCode);
            if(strlen((const char *)mQRCode)!=0)
                property_set(PROPERTY_FINGERPRINT_QRCODE, "1");
            DelmarHalUtils::setSensorPrivData(cmd->o_priv_data, sizeof(cmd->o_priv_data));
            gf_delmar_config_t *config = new gf_delmar_config_t();
            memcpy(config, &cmd->o_delmar_config, sizeof(gf_delmar_config_t));
            mContext->mConfig = (gf_config_t *)config;
            for (int32_t i = 0; i < (int32_t) mSensorNum; i++) {
                char filename[GF_MAX_FILE_NAME_LEN] = { 0 };
                snprintf(filename, sizeof(filename), "%s_%d", GF_OTP_INFO_FILENAME, i);
                if (cmd->io_sdcard_otp_len[i] !=0 && memcmp(cmd->io_sdcard_otp[i], hal_otp[i], cmd->io_sdcard_otp_len[i]) != 0) {
                    LOG_I(LOG_TAG, "[%s] update new otp data to HAL storage", __func__);
                    HalUtils::saveDataToSdcard(filename, cmd->io_sdcard_otp[i], DELMAR_OTP_INFO_BUFFER_LEN);
                }
            }
            handleInitResult(cmd);
        } while (0);

        if (cmd != NULL) {
            free(cmd);
            cmd = NULL;
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarSensor::createInitCmd(gf_delmar_sensor_init_t** cmd, int32_t* size) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (cmd == NULL || size == NULL) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            *cmd = (gf_delmar_sensor_init_t*)malloc(sizeof(gf_delmar_sensor_init_t));
            if (*cmd != NULL) {
                *size = sizeof(gf_delmar_sensor_init_t);
                memset(*cmd, 0, *size);
            } else {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
    /*this interface is to be implemented by derived classes*/
    gf_error_t DelmarSensor::handleInitResult(gf_delmar_sensor_init_t* cmd) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(cmd);
        FUNC_EXIT(err);
        return err;
    }

    /*terminal state. Indicates whether the device is root*/
    bool DelmarSensor::isDeviceUnlocked(void) {
        return false;
    }

    gf_error_t DelmarSensor::captureImage(int32_t op, uint32_t retry) {
        gf_error_t err = GF_SUCCESS;
        uint8_t exposure_mode = DELMAR_EXPOSURE_DEFAULT_SHORT;
        uint8_t continue_frame_count = 0;
        gf_delmar_config_t *config = (gf_delmar_config_t *)mContext->mConfig;
        static uint32_t lastRetry = 0;
        FUNC_ENTER();

        do {
            if (op == GF_OPERATION_AUTHENTICATE) {
                // ignore duplicate image capture calling, except retry 0
                if (retry == lastRetry && retry > 0) {
                    LOG_D(LOG_TAG, "[%s] no need to send capture cmd again for auth retry, ignore it.", __func__);
                    LOG_D(LOG_TAG, "[%s] It has been done by moving up to onAfterAuthAlgo().", __func__);
                    break;
                }
                lastRetry = retry;
            }

            if (op == GF_OPERATION_AUTHENTICATE) {
                if (mSensorType == DELMAR_SENSOR_TYPE_GM182_T_SE1) {
                    exposure_mode = DELMAR_EXPOSURE_DEFAULT_LONG;
                    // 2x2 press 4 sensors
                    if (getMorphotype() == DELMAR_MORPHOTYPE_TWO_MUL_TWO && mRetrySensorIds > 0) {
                        continue_frame_count = 0;
                        // the last 2 sensors
                        if (retry == 1) {
                            mRetrySensorIds = 0;
                        }
                    } else if (isMultiFingerAuth) {
                        continue_frame_count = 0;
                    } else {  // other cases, ex: 1x4 or 2x2(press 2 sensors)
                        // only one sensor is selected & enabled in app
                        if (DelmarHalUtils::calcSensorIdCount(mSensorIds) == 1) {
                            continue_frame_count = 0;
                        } else {
                            if (retry % 2 == 1) {
                                LOG_D(LOG_TAG, "[%s] ignore send capture retry=%d", __func__, retry);
                                break;
                            }
                            continue_frame_count = 1;
                        }
                    }
                } else {
                    if (config->support_independent_fast_auth > 0 && retry == 1) {
                        LOG_D(LOG_TAG, "[%s] ignore send capture retry=%d", __func__, retry);
                        break;
                    }
                    bool isLongExposure = retry && (config->support_long_exposure & (1 << 0));
                    exposure_mode = isLongExposure ? DELMAR_EXPOSURE_DEFAULT_LONG : DELMAR_EXPOSURE_DEFAULT_SHORT;
                }
            } else if (op == GF_OPERATION_ENROLL) {
                exposure_mode = (config->support_long_exposure & (1 << 1)) ?
                                (DELMAR_EXPOSURE_DEFAULT_LONG) : (DELMAR_EXPOSURE_DEFAULT_SHORT);
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
            cmd.exposure_mode = exposure_mode;
            cmd.continue_frame_count = continue_frame_count;
            cmd.clear_cache_image = mClearCacheImage;
            cmd.brightness_level = mBrightnessLevel;
            LOG_D(LOG_TAG, "[%s] mBrightnessLevel: %d", __func__, cmd.brightness_level);
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
        gf_delmar_config_t *config = (gf_delmar_config_t *) mContext->mConfig;

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
            if (mSensorNum == 1 && config->use_full_mask_for_single_chip > 0) {
                cmd.i_use_full_mask = 1;
            } else {
                cmd.i_use_full_mask = 0;
            }
            cmd.i_ui_type = mSensorUIType;
            cmd.i_has_press_area_info = mHasPressAreaInfo;
            cmd.i_press_area_info = mPressAreaInfo;
            cmd.i_coordinate_info = mCoordinateInfo;
            LOG_D(LOG_TAG, "[%s] read image, sensor ids: %08x%08x", __func__,
                    SPLIT_UINT64_TO_UINT32_ARGS(sensorIds));
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            if (mContext->mCenter->hasUpEvt()) {
                err = GF_ERROR_TOO_FAST;
                break;
            }

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
        mClearCacheImage = 1;
        if ((getMorphotype() != DELMAR_MORPHOTYPE_SINGLE) &&
            (coordinate_info->press_x >> 16) != 0 && (coordinate_info->press_y >> 16) != 0) {
            isMultiFingerAuth = 1;
            mPressX[0] = coordinate_info->press_x >> 16;
            mPressY[0] = coordinate_info->press_y >> 16;
            mPressX[1] = coordinate_info->press_x & 0xFFFF;
            mPressY[1] = coordinate_info->press_y & 0xFFFF;
            LOG_D(LOG_TAG, "[%s] 1st finger press (x, y)=(%d, %d)", __func__,
                                    mPressX[0], mPressY[0]);
            LOG_D(LOG_TAG, "[%s] 2nd finger press (x, y)=(%d, %d)", __func__,
                                    mPressX[1], mPressY[1]);
            mCoordinateInfo.press_x = mPressX[0];
            mCoordinateInfo.press_y = mPressY[0];
        } else {
            isMultiFingerAuth = 0;
            LOG_D(LOG_TAG, "[%s] finger press (x, y)=(%d, %d)", __func__,
                                    mCoordinateInfo.press_x, mCoordinateInfo.press_y);
        }
        VOID_FUNC_EXIT();
    }

    void DelmarSensor::switchFingerCoordinate(uint8_t fingerIndex) {
        VOID_FUNC_ENTER();
        mClearCacheImage = 0;
        mCoordinateInfo.press_x = mPressX[fingerIndex];
        mCoordinateInfo.press_y = mPressY[fingerIndex];
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
            if (mSensorType == DELMAR_SENSOR_TYPE_GM182_T_SE1) {
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
                    err = invokeCommand(&cmd, sizeof(cmd));
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

    gf_error_t DelmarSensor::getCaliLoadError() {
        gf_cmd_header_t cmd = { 0 };
        cmd.target = GF_TARGET_SENSOR;
        cmd.cmd_id = GF_CMD_DELMAR_GET_CALI_LOAD_ERROR;
        return invokeCommand(&cmd, sizeof(cmd));
    }
}  // namespace goodix
