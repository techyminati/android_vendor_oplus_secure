#ifndef HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BPHWGOODIXFINGERPRINTDAEMONCALLBACK_H
#define HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BPHWGOODIXFINGERPRINTDAEMONCALLBACK_H

#include <hidl/HidlTransportSupport.h>

#include <vendor/goodix/hardware/biometrics/fingerprint/2.1/IHwGoodixFingerprintDaemonCallback.h>

namespace vendor {
namespace goodix {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {

struct BpHwGoodixFingerprintDaemonCallback : public ::android::hardware::BpInterface<IGoodixFingerprintDaemonCallback>, public ::android::hardware::details::HidlInstrumentor {
    explicit BpHwGoodixFingerprintDaemonCallback(const ::android::sp<::android::hardware::IBinder> &_hidl_impl);

    typedef IGoodixFingerprintDaemonCallback Pure;

    typedef android::hardware::details::bphw_tag _hidl_tag;

    virtual bool isRemote() const override { return true; }

    // Methods from ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemonCallback follow.
    static ::android::hardware::Return<void>  _hidl_onDaemonMessage(::android::hardware::IInterface* _hidl_this, ::android::hardware::details::HidlInstrumentor *_hidl_this_instrumentor, int64_t devId, int32_t msgId, int32_t cmdId, const ::android::hardware::hidl_vec<int8_t>& msg_data);

    // Methods from ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemonCallback follow.
    ::android::hardware::Return<void> onDaemonMessage(int64_t devId, int32_t msgId, int32_t cmdId, const ::android::hardware::hidl_vec<int8_t>& msg_data) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;
    ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;
    ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> setHALInstrumentation() override;
    ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;
    ::android::hardware::Return<void> ping() override;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;
    ::android::hardware::Return<void> notifySyspropsChanged() override;
    ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;

private:
    std::mutex _hidl_mMutex;
    std::vector<::android::sp<::android::hardware::hidl_binder_death_recipient>> _hidl_mDeathRecipients;
};

}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace goodix
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_BPHWGOODIXFINGERPRINTDAEMONCALLBACK_H
