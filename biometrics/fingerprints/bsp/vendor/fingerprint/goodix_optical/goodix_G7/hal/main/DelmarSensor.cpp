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
#include "to_string.h"

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
            mSensorBgVersion(0),
            mCChipType(0),
            mValidEnrollPressCount(0),
            mQrCodeDataLen(0),
            mIsInHBMode(false) {
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
        mLowBrightnessMode = 0;
        for (uint32_t i = 0; i < MAX_SENSOR_NUM; i++) {
            memset(mSensorId[i], 0, DELMAR_SENSOR_ID_MAX_BUFFER_LEN);
        }
        memset(mQrCodeData, 0, MAX_QR_CODE_INFO_LEN);
    }

    DelmarSensor::~DelmarSensor() {
        delete mpProvider;
        mpProvider = nullptr;
    }

    gf_error_t sensorGetQRCode(uint8_t *otp_qr_info, uint8_t* qr_code) {
        gf_error_t err = GF_SUCCESS;
        char str[19];
        uint32_t m_code = 0;
        uint16_t date = 0;
        uint64_t status = 0;
        // Q9180695A3WX001BY4
        uint8_t *data = NULL;
        uint8_t full_table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
        uint8_t day[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X'};
        uint8_t status_table[] = {'O', 'X', 'G', 'P'};
        FUNC_ENTER();

        do {
            if (NULL == otp_qr_info || NULL == qr_code) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            memset(str, 0, sizeof(str));
            data = otp_qr_info;
            m_code = data[1] << 16 | data[2] << 8 | data[3];
            date = data[4] << 8 | data[5];
            status = data[6];
            status = status << 32;
            status = status | (uint32_t)(data[7] << 24) | data[8] << 16 | data[9] << 8 | data[10];

            snprintf(str, sizeof(str), "%c%07d%c%c%c%c%c%c%c%c%c%c", data[0], m_code, full_table[date >> 10 & 0x3f],
                        full_table[date >> 6 & 0x0f], day[date & 0x3f], status_table[status >> 36 & 0x0f],
                        full_table[status >> 30 & 0x3f], full_table[status >> 24 & 0x3f], full_table[status >> 18 & 0x3f],
                        full_table[status >> 12 & 0x3f], full_table[status >> 6 & 0x3f], full_table[status & 0x3f]);
            memcpy(qr_code, str, sizeof(str));
            // LOG_D(LOG_TAG, "[%s] module QR code: %s", __func__, qr_code);
        } while (0);

        FUNC_EXIT(err);
        return err;
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
            cmd->i_device_status = getDeviceStatus();
            LOG_D(LOG_TAG, "[%s] device run in state: [%d]!", __func__, cmd->i_device_status);
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
            mCChipType = cmd->o_c_chip_type;
            mQrCodeDataLen = cmd->o_qr_code_data_len;
            sensorGetQRCode(cmd->o_qr_code_data, (uint8_t *)mQrCodeData);
            for (uint32_t i = 0; i < mSensorNum; i++) {
                memcpy(mSensorId[i], cmd->o_sensor_id[i], mSensorIdLen);
                for (uint32_t j = 0; j < mSensorIdLen; j++) {
                    LOG_D(LOG_TAG, "[%s] mSensorId[%d][%02x]:%02x", __func__, i, j, mSensorId[i][j]);
                }
            }
            LOG_D(LOG_TAG, "[%s] mQRCode=%s, len=%d", __func__, mQrCodeData,strlen((const char *)mQrCodeData));
            property_set(PROPERTY_FINGERPRINT_QRCODE_VALUE, (const char *)mQrCodeData);
            if (strlen((const char *)mQrCodeData)!=0) {
                property_set(PROPERTY_FINGERPRINT_QRCODE, "1");
            }

            LOG_D(LOG_TAG, "[%s] mSensorType:%d, mSensorNum:%u, morphotype:%u, "
                            "cfMark:%d, cfMaskType:%u, mCaliDataState=0x%08x, "
                            "mOpticalType=%d, mQrCodeDataLen=%d", \
                            __func__, mSensorType, mSensorNum, mMorphotype,
                            mCfMark, mCfMaskType, mCaliDataState, mOpticalType,
                            mQrCodeDataLen);
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
    uint8_t DelmarSensor::getDeviceStatus(void) {
        uint8_t status = LOCKED_DEVICE;
        if (isDeviceUnlocked()) {
            status = UNLOCKED_DEVICE;
        }
        return status;
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
            cmd.lb_mode = mLowBrightnessMode;
            cmd.valid_enroll_press_count = (op == GF_OPERATION_ENROLL) ? mValidEnrollPressCount : (0);
            cmd.light_sensor_val = getLightSensorVal();
            LOG_D(LOG_TAG, "[%s] lb_mode: %d", __func__, cmd.lb_mode);
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
            if (mContext->mCenter->hasUpEvt() || (mContext->mConfig->support_ui_ready
                                                  && !isSensorUIReady())) {
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
            if (mContext->mCenter->hasUpEvt() || (mContext->mConfig->support_ui_ready
                                                  && !isSensorUIReady())) {
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

    void DelmarSensor::getCoordinateInfo(gf_delmar_coordinate_info_t *coordinate_info) {
        VOID_FUNC_ENTER();
        if (coordinate_info != NULL) {
            memcpy(coordinate_info, &mCoordinateInfo, sizeof(gf_delmar_coordinate_info_t));
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
            if (mIsInHBMode) {
                LOG_D(LOG_TAG, "[%s] ignore irq when in HB mode", __func__);
                break;
            }
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
                LOG_D(LOG_TAG, "[%s]MSG_EVENT_QUEUED mSensorType=%d, mNotifyRstFlag=%d", __func__,
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
                LOG_D(LOG_TAG, "deal MSG_EVENT_QUEUED");
                clearSensorUIReady();
                break;
            }
            default: {
                LOG_D(LOG_TAG, "msg : %d %s", msg.msg, msg_to_str(msg.msg));
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

    int32_t DelmarSensor::getLightSensorVal(void) {
        return 0;
    }
}  // namespace goodix
