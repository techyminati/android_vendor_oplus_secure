/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "RegisterService.h"
#ifndef SUPPORT_STABLE_AIDL
#include "RegisterHwbinderService.h"
#else   // SUPPORT_STABLE_AIDL
#include "RegisterBinderService.h"
#endif   // SUPPORT_STABLE_AIDL


namespace goodix {

    gf_error_t registerService(HalContext *context) {
#ifndef SUPPORT_STABLE_AIDL
        return registerHwbinderService(context);
#else  // SUPPORT_STABLE_AIDL
        return registerBinderService(context);
#endif  // SUPPORT_STABLE_AIDL
    }

}

