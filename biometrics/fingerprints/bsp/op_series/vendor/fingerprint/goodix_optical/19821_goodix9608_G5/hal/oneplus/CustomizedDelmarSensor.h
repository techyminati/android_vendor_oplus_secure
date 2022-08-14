/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _CUSTOMIZEDDELMARSENSOR_H_
#define _CUSTOMIZEDDELMARSENSOR_H_

#include "DelmarSensor.h"
#include "HalLog.h"

namespace goodix {

    class CustomizedDelmarSensor : public DelmarSensor {
    public:
        explicit CustomizedDelmarSensor(HalContext *context, SensorConfigProvider *provider = nullptr);  // NOLINT(575)
        virtual ~CustomizedDelmarSensor();
        gf_error_t customizedExposureTime();
        gf_error_t customizedGetSpmtPassFlag();
        virtual gf_error_t init();
        gf_error_t readImage(uint32_t retry, uint64_t sensorIds);

        void getSpmtPassOrNot(uint32_t *spmt_pass) {
            VOID_FUNC_ENTER();
            *spmt_pass = spmt_pass_or_not;
            VOID_FUNC_EXIT();
        }

        void setSpmtPassOrNot(uint32_t spmt_pass) {
            VOID_FUNC_ENTER();
            spmt_pass_or_not = spmt_pass;
            VOID_FUNC_EXIT();
        }

        gf_error_t wakeupSensor();

    private:
        uint32_t spmt_pass_or_not;
    };
}  // namespace goodix

#endif /* _CUSTOMIZEDDELMARSENSOR_H_ */
