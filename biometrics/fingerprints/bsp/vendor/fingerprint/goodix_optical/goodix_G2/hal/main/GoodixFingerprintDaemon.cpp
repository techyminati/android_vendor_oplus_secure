
/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][GoodixFingerprintDaemon]"

#include <hidl/HidlBinderSupport.h>
#include <vendor/goodix/hardware/biometrics/fingerprint/2.1/BpHwGoodixFingerprintDaemonCallback.h>
#include "GoodixFingerprintDaemon.h"
#include "ProductTest.h"
#include "HalLog.h"

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
    using ::android::hardware::IInterface;
    using ::goodix::ProductTest;
    using ::goodix::product_test_callback;
    using ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon;
    using ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::BpHwGoodixFingerprintDaemonCallback;
    GoodixFingerprintDaemon* GoodixFingerprintDaemon::sInstance;

    GoodixFingerprintDaemon* GoodixFingerprintDaemon::getInstance(ProductTest* ext)
    {
        if (NULL == sInstance)
        {
            sInstance = new GoodixFingerprintDaemon(ext);
        }
        return sInstance;
    }

    GoodixFingerprintDaemon::GoodixFingerprintDaemon(ProductTest* ext)
        :mCallback(NULL),
         mProductTest(ext)
    {
        if (NULL != mProductTest)
        {
            mProductTest->setNotify((product_test_callback)extTestCallback);
        }
    }

    GoodixFingerprintDaemon::~GoodixFingerprintDaemon()
    {
        if (mProductTest != NULL)
        {
            delete mProductTest;
            mProductTest = NULL;
        }
        mCallback = NULL;
    }

    void GoodixFingerprintDaemon::extTestCallback(int64_t devId, int32_t msgId, int32_t cmdId, const int8_t* data, uint32_t len)
    {
        GoodixFingerprintDaemon* service = GoodixFingerprintDaemon::getInstance(NULL);
        if (service != NULL && service->mCallback != NULL)
        {
            hidl_vec<int8_t> buf;
            buf.setToExternal(const_cast<int8_t*>(data), len);
            service->mCallback->onDaemonMessage(devId, msgId, cmdId, buf);
        }
        return;
    }

    Return<void> GoodixFingerprintDaemon::setNotify(const ::android::sp<IGoodixFingerprintDaemonCallback>& callback)
    {
        if (callback != NULL && mCallback != NULL
                && IInterface::asBinder(static_cast<BpHwGoodixFingerprintDaemonCallback*>(callback.get()))
                    != IInterface::asBinder(static_cast<BpHwGoodixFingerprintDaemonCallback*>(mCallback.get())))
        {
            mCallback->unlinkToDeath(this);
        }

        mCallback = callback;
        if (mCallback != NULL)
        {
            mCallback->linkToDeath(this, 0);
        }
        return Return<void>();
    }

    Return<void> GoodixFingerprintDaemon::sendCommand(int32_t cmdId, const hidl_vec<int8_t>& in_buf, sendCommand_cb _cb)
    {
        hidl_vec<int8_t> out_buf;
        gf_error_t err;
        int8_t* out = NULL;
        uint32_t outLen = 0;

        err = mProductTest->onCommand(cmdId, in_buf.data(), in_buf.size(), &out, &outLen);
        out_buf.setToExternal(out, outLen);
        _cb(err, out_buf);
        return Return<void>();
    }

    void GoodixFingerprintDaemon::serviceDied(uint64_t cookie, const ::android::wp<::android::hidl::base::V1_0::IBase>& who)
    {
        UNUSED_VAR(cookie);
        UNUSED_VAR(who);
        LOG_D(LOG_TAG, "[%s] callback binder died", __func__);
        mCallback = NULL;
        return;
    }

}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // goodix
}  // namespace goodix
}  // namespace vendor
