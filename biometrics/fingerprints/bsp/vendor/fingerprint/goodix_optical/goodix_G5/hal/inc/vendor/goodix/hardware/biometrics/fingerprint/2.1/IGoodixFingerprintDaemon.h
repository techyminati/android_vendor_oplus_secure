#ifndef HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_IGOODIXFINGERPRINTDAEMON_H
#define HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_IGOODIXFINGERPRINTDAEMON_H

#include <android/hidl/base/1.0/IBase.h>
#include <vendor/goodix/hardware/biometrics/fingerprint/2.1/IGoodixFingerprintDaemonCallback.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace goodix {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {

struct IGoodixFingerprintDaemon : public ::android::hidl::base::V1_0::IBase {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    // @entry
    virtual ::android::hardware::Return<void> setNotify(const ::android::sp<::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemonCallback>& callback) = 0;

    using sendCommand_cb = std::function<void(int32_t resultCode, const ::android::hardware::hidl_vec<int8_t>& out_buf)>;
    virtual ::android::hardware::Return<void> sendCommand(int32_t cmdId, const ::android::hardware::hidl_vec<int8_t>& in_buf, sendCommand_cb _hidl_cb) = 0;

    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;

    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> setHALInstrumentation() override;

    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;

    virtual ::android::hardware::Return<void> ping() override;

    using getDebugInfo_cb = std::function<void(const ::android::hidl::base::V1_0::DebugInfo& info)>;
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> notifySyspropsChanged() override;

    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;
    // cast static functions
    static ::android::hardware::Return<::android::sp<::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon>> castFrom(const ::android::sp<::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IGoodixFingerprintDaemon> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IGoodixFingerprintDaemon> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IGoodixFingerprintDaemon> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IGoodixFingerprintDaemon> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IGoodixFingerprintDaemon> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IGoodixFingerprintDaemon> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IGoodixFingerprintDaemon> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IGoodixFingerprintDaemon> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

static inline std::string toString(const ::android::sp<::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace goodix
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_GOODIX_HARDWARE_BIOMETRICS_FINGERPRINT_V2_1_IGOODIXFINGERPRINTDAEMON_H
