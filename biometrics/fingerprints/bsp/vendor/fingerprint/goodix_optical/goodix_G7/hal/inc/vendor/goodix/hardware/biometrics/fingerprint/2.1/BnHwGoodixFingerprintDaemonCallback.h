#ifndef HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BNHWGOODIXFINGERPRINTDAEMONCALLBACK_H
#define HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BNHWGOODIXFINGERPRINTDAEMONCALLBACK_H

#include <vendor/goodix/hardware/biometrics/fingerprint/2.1/IHwGoodixFingerprintDaemonCallback.h>

namespace vendor {
namespace goodix {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {

struct BnHwGoodixFingerprintDaemonCallback : public ::android::hidl::base::V1_0::BnHwBase {
    explicit BnHwGoodixFingerprintDaemonCallback(const ::android::sp<IGoodixFingerprintDaemonCallback> &_hidl_impl);
    explicit BnHwGoodixFingerprintDaemonCallback(const ::android::sp<IGoodixFingerprintDaemonCallback> &_hidl_impl, const std::string& HidlInstrumentor_package, const std::string& HidlInstrumentor_interface);

    virtual ~BnHwGoodixFingerprintDaemonCallback();

    ::android::status_t onTransact(
            uint32_t _hidl_code,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            uint32_t _hidl_flags = 0,
            TransactCallback _hidl_cb = nullptr) override;


    typedef IGoodixFingerprintDaemonCallback Pure;

    typedef android::hardware::details::bnhw_tag _hidl_tag;

    ::android::sp<IGoodixFingerprintDaemonCallback> getImpl() { return _hidl_mImpl; }
    // Methods from ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemonCallback follow.
    static ::android::status_t _hidl_onDaemonMessage(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);



private:
    // Methods from ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemonCallback follow.

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> ping();
    using getDebugInfo_cb = ::android::hidl::base::V1_0::IBase::getDebugInfo_cb;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb);

    ::android::sp<IGoodixFingerprintDaemonCallback> _hidl_mImpl;
};

}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace goodix
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BNHWGOODIXFINGERPRINTDAEMONCALLBACK_H
