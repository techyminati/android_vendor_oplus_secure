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
#include "gf_sensor_types.h"
#include "MsgBus.h"

namespace goodix
{
    class SZSensor : public Sensor, public MsgBus::IMsgListener
    {
    public:
        explicit SZSensor(HalContext *context);
        virtual ~SZSensor();
        virtual gf_error_t init(gf_sensor_ids_t* ids);
        virtual gf_error_t sleepSensor();
        virtual gf_error_t wakeupSensor();
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        bool checkScreenType(char *type);

    public:
        int8_t mQrCode[MAX_QR_CODE_INFO_LEN];
    };
}

#endif /* _SZSENSOR_H_ */

