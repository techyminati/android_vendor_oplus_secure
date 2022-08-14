/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _DELMARALGOUTILS_H_
#define _DELMARALGOUTILS_H_

#include "gf_base_types.h"
#include "gf_algo_types.h"

namespace goodix {
    class HalContext;
    class HalDsp;

    class DelmarAlgoUtils {
    public:
#ifdef SUPPORT_DSP_HAL
        static gf_error_t enrollImageDsp(HalContext *context, gf_algo_enroll_image_t *enroll,
            uint8_t isFirst, uint64_t sensorIds, HalDsp *dsp);
        static gf_error_t fastAuthImageDsp(HalContext *context, gf_algo_auth_image_t *auth, HalDsp *dsp);
        static gf_error_t authImageDsp(HalContext *context, gf_algo_auth_image_t *auth,
            uint8_t isFirstAuth, uint64_t authSensorIds, HalDsp *dsp, uint8_t isCheckFingerUp);
#endif  // SUPPORT_DSP_HAL
        static void printHalVersionInAlgoUtils();
    };
}  // namespace goodix

#endif  // _DELMARALGOUTILS_H_
