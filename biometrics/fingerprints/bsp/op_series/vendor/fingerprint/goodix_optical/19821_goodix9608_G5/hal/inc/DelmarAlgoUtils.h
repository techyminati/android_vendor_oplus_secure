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

    class DelmarAlgoUtils {
    public:
        static int32_t detectTemperature();
#ifdef SUPPORT_DSP_HAL
        static gf_error_t fastAuthImageDsp(HalContext *context, gf_algo_auth_image_t *auth);
        static gf_error_t authImageDsp(HalContext *context, gf_algo_auth_image_t *auth,
            uint8_t isFirstAuth, uint64_t authSensorIds);
#endif  // SUPPORT_DSP_HAL
    };
}  // namespace goodix

#endif  // _DELMARALGOUTILS_H_