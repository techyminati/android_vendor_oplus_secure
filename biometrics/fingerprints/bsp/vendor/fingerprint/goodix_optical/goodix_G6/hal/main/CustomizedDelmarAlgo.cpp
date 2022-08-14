/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][CustomizedDelmarAlgo]"

#include "CustomizedDelmarAlgo.h"

namespace goodix {

    CustomizedDelmarAlgo::CustomizedDelmarAlgo(HalContext *context) :
        DelmarAlgo(context) {
    }

    CustomizedDelmarAlgo::~CustomizedDelmarAlgo() {
    }

    bool CustomizedDelmarAlgo::isCheckFingerUp() {
        return true;
    }
}  // namespace goodix
