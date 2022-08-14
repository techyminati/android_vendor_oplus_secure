/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _DELMARALGO_H_
#define _DELMARALGO_H_

#include "Algo.h"
#include "gf_delmar_types.h"

namespace goodix {
    class DelmarAlgo : public Algo {
    public:
        explicit DelmarAlgo(HalContext *context);
        virtual ~DelmarAlgo();
        virtual gf_error_t init();
        virtual gf_error_t enrollImage(gf_algo_enroll_image_t *enrll);
        virtual gf_error_t authImage(gf_algo_auth_image_t *auth);
        virtual gf_algo_enroll_image_t* createEnrollCmd();
        virtual void destroyEnrollCmd(gf_algo_enroll_image_t* cmd);
        virtual gf_algo_auth_image_t* createAuthCmd();
        virtual void destroyAuthCmd(gf_algo_auth_image_t* cmd);
        gf_error_t updateSensorIds(gf_delmar_sensor_ids_t *info);
        inline void setFirstSensor(bool firstAuth) {
            mIsFirstSensor = firstAuth;
        }
        inline void setAuthSensorIds(uint64_t sensorIds) {
            mAuthSensorIds = sensorIds;
        }
        inline uint8_t getAuthSuccessSensorId(void) {
            return mAuthSuccessSensorId;
        }
        inline void setEnrollSensorIds(uint64_t sensorIds) {
            mEnrollSensorIds = sensorIds;
        }

        inline uint64_t getStudySensorIds() {
            return mStudySensorIds;
        }
    private:
        uint64_t mAuthSensorIds;
        uint64_t mEnrollSensorIds;
        uint64_t mStudySensorIds;
        // the flag to mark the first time enroll/auth for multi-sensor rawdata captured once
        bool mIsFirstSensor;
        uint8_t mAuthSuccessSensorId;
    };
}  // namespace goodix
#endif  /* _DELMARALGO_H_ */
