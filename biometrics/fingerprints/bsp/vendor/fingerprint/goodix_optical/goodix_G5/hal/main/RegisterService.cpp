/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "RegisterService.h"
#if defined(__ANDROID_O) || defined(__ANDROID_P) || defined(__ANDROID_Q)
#include "RegisterHwbinderService.h"
#else   // __ANDROID_O
#include "RegisterBinderService.h"
#endif   // __ANDROID_O


namespace goodix {

    gf_error_t registerService(HalContext *context) {
#if defined(__ANDROID_O) || defined(__ANDROID_P) || defined(__ANDROID_Q)
        return registerHwbinderService(context);
#else  // __ANDROID_O
        return registerBinderService(context);
#endif  // __ANDROID_O
    }

}

