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

namespace goodix {
    class SensorConfigProvider;

    class DelmarSensor : public Sensor {
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
        uint8_t mAuthRetryCount;
        uint32_t mCaliDataState;
        SensorConfigProvider *mpProvider;

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
        gf_error_t calculateMask(uint64_t sensorIds);
        // calculate sensor ids by coordinate info & sensor area info
        void updateSensorPriority();

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
    };
}  // namespace goodix

#endif /* _DELMARSENSOR_H_ */
