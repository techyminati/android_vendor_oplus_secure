/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef EXTSERVICE_HWBINDER_GOODIXFINGERPRINTDAEMON_H_
#define EXTSERVICE_HWBINDER_GOODIXFINGERPRINTDAEMON_H_

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <hidl/HidlSupport.h>

#include "ProductTest.h"

namespace vendor
{
namespace goodix
{
namespace hardware
{
namespace biometrics
{
namespace fingerprint
{
namespace V2_1
{
namespace implementation
{

    using ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon;
    using ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemonCallback;
    using ::android::hardware::Return;
    using ::android::hardware::Void;
    using ::android::hardware::hidl_vec;
    using ::android::hardware::hidl_string;
    using ::android::hardware::hidl_death_recipient;
    using ::android::sp;

    using ::goodix::ProductTest;

    class GoodixFingerprintDaemon : public IGoodixFingerprintDaemon, public hidl_death_recipient
    {
    public:
        static GoodixFingerprintDaemon* getInstance(ProductTest* ext);
        virtual ~GoodixFingerprintDaemon();
        // override
        virtual Return<void> setNotify(const ::android::sp<IGoodixFingerprintDaemonCallback>& callback);
        virtual Return<void> sendCommand(int32_t cmdId, const hidl_vec<int8_t>& in_buf, sendCommand_cb _cb);
        virtual void serviceDied(uint64_t cookie, const ::android::wp<::android::hidl::base::V1_0::IBase>& who);
    private:
        GoodixFingerprintDaemon(ProductTest* ext);
        static void extTestCallback(int64_t devId, int32_t msgId, int32_t cmdId, const int8_t* data, uint32_t len);
        static GoodixFingerprintDaemon* sInstance;
        sp<IGoodixFingerprintDaemonCallback> mCallback;
        ProductTest* mProductTest;
    };

}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // goodix
}  // namespace goodix
}  // namespace vendor


#endif /* EXTSERVICE_HWBINDER_GOODIXFINGERPRINTDAEMON_H_ */
