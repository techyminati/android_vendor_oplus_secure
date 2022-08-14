/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _DELMARSENSOR_H_
#define _DELMARSENSOR_H_

#include "Sensor.h"
#include "gf_delmar_types.h"
#define MAX_MUTI_AUTH_FINGERS 2

namespace goodix {
    class SensorConfigProvider;

    class DelmarSensor : public Sensor {
    public:
        explicit DelmarSensor(HalContext *context, SensorConfigProvider *provider = nullptr);  // NOLINT(575)
        virtual ~DelmarSensor();
        virtual gf_error_t init();
        virtual gf_error_t wakeupSensor();
        virtual gf_error_t sleepSensor();
        virtual gf_error_t captureImage(int32_t op, uint32_t retry);
        virtual gf_error_t readImage(uint32_t retry, uint64_t sensorIds);
        virtual gf_event_type_t getIrqEventType();
        virtual gf_error_t onMessage(const MsgBus::Message &msg);

        void switchRetrySensorIds();
        void setSensorIds(uint64_t sensorIds, uint32_t sensorsCenterMode = 0);  // NOLINT(575)
        void setCoordinateInfo(gf_delmar_coordinate_info_t *coordinate_info);
        void setSensorAreaInfo(gf_delmar_press_area_info_t *press_area_info);
        // calculate sensor ids by coordinate info & sensor area info
        void updateSensorPriority();
        void switchFingerCoordinate(uint8_t fingerIndex);
        inline uint64_t getSensorIds(void) {
            return mSensorIds;
        }

        inline void setSensorUIType(uint8_t type) {
            mSensorUIType = type;
        }
        inline void setHasPressAreaInfo(uint8_t hasPressAreaInfo) {
            mHasPressAreaInfo = hasPressAreaInfo;
        }

        inline uint32_t getAvailableSensorNum() {
            return mSensorNum;
        }

        inline gf_delmar_sensor_type_t getSenosrType() {
            return mSensorType;
        }

        inline uint8_t getMorphotype() {
            return mMorphotype;
        }

        inline uint8_t getCfMark() {
            return mCfMark;
        }

        inline uint8_t getCfMaskType() {
            return mCfMaskType;
        }

        inline uint8_t getOpticalType() {
            return mOpticalType;
        }

        inline SensorConfigProvider* getSensorConfigProvider() {
            return mpProvider;
        }

        inline void setNotifyRstFlag(uint8_t flag) {
            mNotifyRstFlag = flag;
        }

        inline void setCaliState(uint32_t state) {
            mCaliDataState = state;
        }

        inline uint32_t getCaliState() {
            return mCaliDataState;
        }

        inline void setBrightnessLevel(uint8_t level) {
            mBrightnessLevel = level;
        }

        inline uint32_t getMultiFingerAuth() {
            return isMultiFingerAuth;
        }

        inline uint32_t getSensorIdLen() {
            return mSensorIdLen;
        }

        inline uint32_t getSensorBgVersion() {
            return mSensorBgVersion;
        }

    protected:
        virtual gf_error_t createInitCmd(gf_delmar_sensor_init_t** cmd, int32_t* size);
        virtual gf_error_t handleInitResult(gf_delmar_sensor_init_t *cmd);
        virtual bool isDeviceUnlocked(void);

    private:
        uint64_t mSensorIds;
        uint64_t mRetrySensorIds;
        uint8_t mSensorUIType;
        uint8_t mHasPressAreaInfo;
        gf_delmar_coordinate_info_t mCoordinateInfo;
        gf_delmar_press_area_info_t mPressAreaInfo;
        gf_delmar_sensor_type_t mSensorType;
        uint32_t mSensorNum;
        uint8_t mNotifyRstFlag;
        uint8_t mMorphotype;
        uint8_t mCfMark;
        uint8_t mCfMaskType;
        uint8_t mOpticalType;
        uint32_t mCaliDataState;
        uint32_t isMultiFingerAuth;
        uint32_t mClearCacheImage;
        int32_t mPressX[MAX_MUTI_AUTH_FINGERS];
        int32_t mPressY[MAX_MUTI_AUTH_FINGERS];
        SensorConfigProvider *mpProvider;
        uint8_t mBrightnessLevel;
        uint32_t mSensorIdLen;
        uint32_t mSensorBgVersion;
    };
}  // namespace goodix

#endif /* _DELMARSENSOR_H_ */
