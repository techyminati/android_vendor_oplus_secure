/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CORECREATOR_H_
#define _CORECREATOR_H_

#include "FingerprintCore.h"
#include "Sensor.h"
#include "Algo.h"
#include "Device.h"

namespace goodix {
    FingerprintCore *createFingerprintCore(HalContext *context);

    Sensor *createSensor(HalContext *context);

    Algo *createAlgo(HalContext *context);

    Device *createDevice(HalContext *context);
}  // namespace goodix

#endif /* _CORECREATOR_H_ */
