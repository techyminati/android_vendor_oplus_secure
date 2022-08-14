/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _SZSENSOR_H_
#define _SZSENSOR_H_

#include "Sensor.h"
#include "gf_sz_types.h"
#include "MsgBus.h"

namespace goodix {
    class SZSensor : public Sensor{
    public:
        explicit SZSensor(HalContext *context);
        virtual ~SZSensor();
        virtual gf_error_t init();
        virtual gf_error_t sleepSensor();
        virtual gf_error_t wakeupSensor();
        virtual gf_error_t setStudyDisable(int32_t disable);
        virtual gf_error_t setExpoTimetLevel();
        virtual void setMulExpoLevel(uint32_t level, uint32_t force = 0);  // NOLINT(575)
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
    private:
        uint32_t mExpoTimetLevel;
    };
}

#endif /* _SZSENSOR_H_ */

