/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#include "CoreCreator.h"
#include "HalContext.h"
#include "CustomizedSensor.h"
#include "CustomizedFingerprintCore.h"
#include "DelmarAlgo.h"
#ifdef SUPPORT_USB_CDC
#include "CdcDevice.h"
#endif  // SUPPORT_USB_CDC
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
            case GF_DELMAR: {
                sensor = new CustomizedSensor(context);
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

        Algo *algo = new DelmarAlgo(context);
        return algo;
    }

    Device *createDevice(HalContext *context) {
#ifdef SUPPORT_USB_CDC
        return new CdcDevice(context);
#else  // SUPPORT_USB_CDC
        return new CustomizedDevice(context);
#endif  // SUPPORT_USB_CDC
    }
}  // namespace goodix
