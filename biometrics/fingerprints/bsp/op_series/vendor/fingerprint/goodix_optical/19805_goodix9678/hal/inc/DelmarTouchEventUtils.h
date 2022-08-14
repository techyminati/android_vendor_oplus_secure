/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _DELMARTOUCHEVENTUTILS_H_
#define _DELMARTOUCHEVENTUTILS_H_
#include "gf_delmar_types.h"

using ::android::Vector;

namespace goodix {
    class DelmarTouchEventUtils {
    public:
        static void start();
        static void stop();
        static gf_error_t getTouchCoordinate(gf_delmar_coordinate_info_t *coordinate_info);
    };
}  // namespace goodix

#endif  // _DELMARTOUCHEVENTUTILS_H_
