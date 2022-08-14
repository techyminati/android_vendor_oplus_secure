/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "IPowerManager"
#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/Parcel.h>
#include "IPowerManager.h"

namespace android
{

    const String16 POWER_DESCRIPTOR("android.os.IPowerManager");

    class BpPowerManager : public BpInterface<IPowerManager>
    {
    public:
        explicit BpPowerManager(const sp<IBinder> &impl): BpInterface<IPowerManager>
            (impl)
        {
        }
        virtual bool isInteractive()
        {
            Parcel data;
            Parcel reply;
            bool result;
            data.writeInterfaceToken(POWER_DESCRIPTOR);
            remote()->transact(IS_INTERACTIVE, data, &reply, 0);
            reply.readExceptionCode();
            result = (0 != reply.readInt32());
            return result;
        }
    };

    IMPLEMENT_META_INTERFACE(PowerManager, "android.os.IPowerManager");
};  // namespace android
