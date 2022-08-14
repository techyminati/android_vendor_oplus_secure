/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "CustomizedHalConfig.h"

namespace goodix {
    const char* getDumpRootDir(void) {
        return "/data/vendor/fingerprint";
    }

    const char* getDumpControlProperty(void) {
        return "gf.debug.dump_data";
    }
}  // namespace goodix
