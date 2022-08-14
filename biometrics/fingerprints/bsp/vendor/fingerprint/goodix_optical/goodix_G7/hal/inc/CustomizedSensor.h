/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _CUSTOMIZEDSENSOR_H_
#define _CUSTOMIZEDSENSOR_H_

#include "DelmarSensor.h"
#include "gf_delmar_types.h"
#include "gf_customized_types.h"

namespace goodix {
    class CustomizedSensor : public DelmarSensor {
    public:
        explicit CustomizedSensor(HalContext *context);
        virtual ~CustomizedSensor();
        inline int32_t getProductScreenId() {
            return mProductScreenId;
        }
        virtual int32_t detectTemperature();

        inline void getCustomizedConfig(gf_customized_config_t *config) {
            memcpy(config, &mCustomizedConfig, sizeof(gf_customized_config_t));
        }
    protected:
        virtual gf_error_t createInitCmd(gf_delmar_sensor_init_t** cmd, int32_t* size);
        virtual gf_error_t handleInitResult(gf_delmar_sensor_init_t *cmd);
        virtual bool isDeviceUnlocked(void);

    private:
        int32_t mProductScreenId;
        gf_customized_config_t mCustomizedConfig;
    };
}  // namespace goodix



#endif /* _CUSTOMIZEDSENSOR_H_ */
