/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _CUSTOMIZEDSENSORCONFIGPROVIDER_H_
#define _CUSTOMIZEDSENSORCONFIGPROVIDER_H_

#include <stdint.h>
#include "gf_delmar_types.h"
#include "SensorConfigProvider.h"

namespace goodix {
    class CustomizedSensorConfigProvider : public SensorConfigProvider {
    public:
        CustomizedSensorConfigProvider();  // NOLINT(575)
        virtual ~CustomizedSensorConfigProvider() {}
        virtual gf_error_t getConfig(GoodixSensorConfig *config, int32_t sensorNum);

        inline void setProductScreenId(uint32_t productScreenId) {
            mProductScreenId = productScreenId;
        }

    private:
        int32_t mProductScreenId;
    };
}  // namespace goodix

#endif  // _SENSORCONFIGPROVIDER_H_
