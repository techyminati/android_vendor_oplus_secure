/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#include "CoreCreator.h"
#include "HalContext.h"
#include "SZSensor.h"
#include "SZAlgo.h"
#include "CustomizedFingerprintCore.h"
#include "CustomizedDevice.h"

namespace goodix {
    FingerprintCore *createFingerprintCore(HalContext *context) {
        return new CustomizedFingerprintCore(context);
    }

    Sensor *createSensor(HalContext *context) {
        GF_ASSERT_NOT_NULL(context);
        Sensor *sensor = nullptr;
        gf_chip_series_t series = context->mSensorInfo.chip_series;

        switch (series) {
            case GF_SHENZHEN: {
                sensor = new SZSensor(context);
                break;
            }

            case GF_UNKNOWN_SERIES: {
                break;
            }

            default: {
                break;
            }
        }

        return sensor;
    }


    Algo *createAlgo(HalContext *context) {
        if (nullptr == context) {
            return nullptr;
        }

        Algo *algo = new SZAlgo(context);
        return algo;
    }

    Device *createDevice(HalContext *context) {
        return new CustomizedDevice(context);
    }
}  // namespace goodix
