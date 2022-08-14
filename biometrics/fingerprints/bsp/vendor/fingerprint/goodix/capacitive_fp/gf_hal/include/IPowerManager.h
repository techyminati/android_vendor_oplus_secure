/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: PowerManager header file
 * History:
 * Version: 1.0
 */

#ifndef _IPOWERMANAGER_H_
#define _IPOWERMANAGER_H_

#include <utils/Errors.h>
#include <binder/IInterface.h>
#define POWER_MANAGER  "power"
namespace android
{
    class IPowerManager : public IInterface
    {
    public:
        enum
        {
#ifdef __PLATFORM_MTK
#ifdef __ANDROID_N
            IS_INTERACTIVE = IBinder::FIRST_CALL_TRANSACTION + 13,
#else  // __ANDROID_N
            IS_INTERACTIVE = IBinder::FIRST_CALL_TRANSACTION + 14,
#endif  // __ANDROID_N
#else  // __PLATFORM_MTK
            IS_INTERACTIVE = IBinder::FIRST_CALL_TRANSACTION + 11,
#endif  // __PLATFORM_MTK
        };
        DECLARE_META_INTERFACE(PowerManager);
        virtual bool isInteractive() = 0;
    };
};  // namespace android

#endif  // _IPOWERMANAGER_H_
