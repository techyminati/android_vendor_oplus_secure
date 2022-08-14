/************************************************************************************
 ** File: - vendor\fingerprint\hwbinder\jiiov_optical\anc_fingerprint.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for Jiiov(android O)
 **
 ** Version: 1.0
 ** Date created: 15:09:11,23/8/2021
 ** Author: Fengxiong.Wu@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Fengxiong.Wu         2021/08/23       modify hidl interface for Android S
 ************************************************************************************/

#include <hardware/hw_auth_token.h>

#include <hardware/hardware.h>
#include "fingerprint.h"
#include "anc_fingerprint.h"


#include <android/log.h>

#include <inttypes.h>

#include <unistd.h>

#include "fingerprint_type.h"

#include "anc_hal.h"
#include "Perf.h"
//#include "vendor/oplus/hardware/biometrics/fingerprintservice/1.0/IFingerprintHalService.h"
#ifdef FP_DCS_ENABLE
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"
#endif /* FP_DCS_ENABLE */
#ifdef FP_CONFIG_SETTINGS_ENABLE
#include "FingerprintSettings.h"
#endif




using android::Hypnus;
using android::Dcs;

namespace vendor {
namespace oplus {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_1 {
namespace implementation {

#define ANC_UNUSED(x) (void)(x)

using RequestStatus =
        android::hardware::biometrics::fingerprint::V2_1::RequestStatus;

typedef enum {
    OPLUS_EXTENSION_COMMAND_AUTHENTICATE_AS_TYPE = 1,
    OPLUS_EXTENSION_COMMAND_CLEAN_UP = 2,
    OPLUS_EXTENSION_COMMAND_PAUSE_ENROLL = 3,
    OPLUS_EXTENSION_COMMAND_CONTINUE_ENROLL = 4,
    OPLUS_EXTENSION_COMMAND_SET_TOUCH_EVENT_LISTENER = 5,
    OPLUS_EXTENSION_COMMAND_DYNAMICALLY_CONFIG_LOG = 6,
    OPLUS_EXTENSION_COMMAND_PAUSE_IDENTIFY = 7,
    OPLUS_EXTENSION_COMMAND_CONTINUE_IDENTIFY = 8,
    OPLUS_EXTENSION_COMMAND_GET_ALIKEY_STATUS = 9,
    OPLUS_EXTENSION_COMMAND_GET_ENROLLMENT_TOTAL_TIMES = 10,
    OPLUS_EXTENSION_COMMAND_SET_SCREEN_STATE = 11,
    OPLUS_EXTENSION_COMMAND_GET_ENGINEERING_INFO = 12,
    OPLUS_EXTENSION_COMMAND_TOUCH_DOWN = 13,
    OPLUS_EXTENSION_COMMAND_TOUCH_UP = 14,
    OPLUS_EXTENSION_COMMAND_SEND_FINGERPRINT_CMD = 15,
    OPLUS_EXTENSION_COMMAND_CB_MONITOR_EVENT_TRIGGERED = 16,
    OPLUS_EXTENSION_COMMAND_CB_IMAGE_INFO_ACQUIRED = 17,
    OPLUS_EXTENSION_COMMAND_CB_SYNC_TEMPLATES = 18,
    OPLUS_EXTENSION_COMMAND_CB_ENGINEERING_INFO_UPDATED = 19,
}OPLUS_EXTENSION_COMMAND_TYPE;

typedef enum {
    ANC_EXTERNAL_BIND_CORE = 1,
    ANC_EXTERNAL_SCALE_CPU_FREQUENCY = 2,
    ANC_SEND_DCS_EVENT_INFO = 3,
    ANC_SETUXTHREAD = 4,
}ANC_EXTERNAL_TYPE;

#define ACTION_TYPE 12
#define ACTION_TIMEOUT_500  500
#define ACTION_TIMEOUT_1000 1000
#define ACTION_TIMEOUT_2000 2000
#define ACTION_TIMEOUT_3000 3000


AncFingerprint *AncFingerprint::s_instance_ = nullptr;


AncFingerprint::AncFingerprint() {
    s_instance_ = this;
    p_device_ = nullptr;
    sp_fp_client_callback_ = nullptr;
}

AncFingerprint::~AncFingerprint() {
    if (nullptr != p_device_) {
        if (0 != DeinitFingerprintDevice(p_device_)) {
            ALOGE("fail to deinit anc device");
        }
        p_device_ = nullptr;
    }
}

IBiometricsFingerprint* AncFingerprint::getInstance() {
    if (nullptr == s_instance_) {
        s_instance_ = new AncFingerprint();
    }

    return s_instance_;
}

bool AncFingerprint::init() {
    if (0 == InitFingerprintDevice(&p_device_)) {
        if (p_device_ == nullptr) {
            ALOGE("fingerprint devices is nullptr");
            return false;
        }
    } else {
        ALOGE("fail to init fingerprint devices");
        p_device_ = nullptr;
        return false;
    }

    SetEnrollCallBackFunction(p_device_, onEnrollResult);
    SetAcquireCallBackFunction(p_device_, onAcquired);
    SetAuthenticateCallBackFunction(p_device_, onAuthenticated);
    SetErrorCallBackFunction(p_device_, onError);
    SetRemoveCallBackFunction(p_device_, onRemoved);
    SetEnumerateCallBackFunction(p_device_, onEnumerate);
    SetExcuteCommandCallBackFunction(p_device_, onExcuteCommand);
    SetExternalWorkerFunction(p_device_, doExternalWork);

    return true;
}

Return<uint64_t> AncFingerprint::setNotify(
        const sp<IBiometricsFingerprintClientCallback>& clientCallback) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    sp_fp_client_callback_ = clientCallback;

    // This is here because HAL 2.1 doesn't have a way to propagate a
    // unique token for its driver. Subsequent versions should send a unique
    // token for each call to setNotify(). This is fine as long as there's only
    // one fingerprint device on the platform.
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> AncFingerprint::setHalCallback(
        const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx) {
    std::lock_guard<std::mutex> lock(mClientCallbackMutex);
    sp_fp_client_callback_Ex = clientCallbackEx;
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> AncFingerprint::preEnroll()  {
    return AncPreEnroll(p_device_);
}

Return<RequestStatus> AncFingerprint::enroll(const hidl_array<uint8_t, 69>& hat,
        uint32_t gid, uint32_t timeoutSec) {
    int ret = AncEnroll(p_device_, hat.data(), gid, timeoutSec);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::postEnroll() {
    int ret = AncPostEnroll(p_device_);

    if (0 != ret) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<uint64_t> AncFingerprint::getAuthenticatorId() {
    return AncGetAuthenticatorId(p_device_);
}

Return<RequestStatus> AncFingerprint::cancel() {
    AncCancel(p_device_);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::enumerate()  {
    AncEnumerate(p_device_);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::remove(uint32_t gid, uint32_t fid) {
    AncRemove(p_device_, gid, fid);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::setActiveGroup(uint32_t gid,
        const hidl_string& storePath) {
    if ((storePath.size() >= PATH_MAX) || (storePath.size() <= 0)) {
        ALOGE("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        return RequestStatus::SYS_EINVAL;
    }

    int ret = AncSetActiveGroup(p_device_, gid, storePath.c_str());

    if (0 != ret) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::authenticate(uint64_t operationId,
        uint32_t gid) {
    ALOGE("authenticateAsType");

    int ret = AncAuthenticate(p_device_, operationId, gid);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::authenticateAsType(uint64_t operationId, uint32_t gid,
                           int32_t fingerprintAuthType) {
    ANC_UNUSED(operationId);
    ANC_UNUSED(gid);
    ANC_UNUSED(fingerprintAuthType);
    //int ret = this->excuteCommand(OPPO_EXTENSION_COMMAND_AUTHENTICATE_AS_TYPE,
                                  //inputput_vector, outputput_vector);
    return RequestStatus::SYS_OK;
}

Return<bool> AncFingerprint::isUdfps(uint32_t sensorId) {
    ALOGD("isUdfps sensor_id = %d", sensorId);
    if (sensorId >= E_SENSOR_ID_MAX) {
        ALOGE("unknown sensor");
        return false;
    }
    return (fp_config_info[sensorId].fp_type) == 4 ? true:false;
}

Return<void> AncFingerprint::onFingerDown(uint32_t x, uint32_t y, float minor, float major) {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;
    ANC_UNUSED(x);
    ANC_UNUSED(y);
    ANC_UNUSED(minor);
    ANC_UNUSED(major);

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_TOUCH_DOWN, inputput_vector, outputput_vector);
    ANC_UNUSED(ret);

    return Void();
}

Return<void> AncFingerprint::onFingerUp() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;
    ALOGD("onFingerUp");

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_TOUCH_UP, inputput_vector, outputput_vector);

    ANC_UNUSED(ret);
    return Void();
}
/*
Return<RequestStatus> AncFingerprint::cleanUp() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_CLEAN_UP,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}
*/
Return<RequestStatus> AncFingerprint::pauseEnroll() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_PAUSE_ENROLL,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::continueEnroll() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_CONTINUE_ENROLL,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::setTouchEventListener() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_SET_TOUCH_EVENT_LISTENER,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::dynamicallyConfigLog(uint32_t on) {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    inputput_vector.resize(sizeof(on));
    memcpy(inputput_vector.data(), &on, sizeof(on));

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_DYNAMICALLY_CONFIG_LOG,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}
/*
Return<RequestStatus> AncFingerprint::pauseIdentify() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_PAUSE_IDENTIFY,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::continueIdentify() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_CONTINUE_IDENTIFY,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::getAlikeyStatus() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_GET_ALIKEY_STATUS,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}
*/
Return<int32_t> AncFingerprint::getEnrollmentTotalTimes() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;
#if 0
    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_GET_ENROLLMENT_TOTAL_TIMES,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return 0;
    }
    return 0;
#else
    return 20;
#endif
}

Return<RequestStatus> AncFingerprint::setScreenState(int32_t screen_state) {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;
    int32_t temp_screen_state = static_cast<int32_t>(screen_state);

    inputput_vector.resize(sizeof(temp_screen_state));
    memcpy(inputput_vector.data(), &temp_screen_state, sizeof(temp_screen_state));

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_SET_SCREEN_STATE,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

int AncFingerprint::getBrightnessValue() {
    int32_t length = 0;
    int index = 0;
    ALOGD("[AncFingerprint] getBrightnessValue start");

    memset(mBrightValue, 0, sizeof(mBrightValue));
    char *brightness_paths[] = {
        "/sys/class/leds/lcd-backlight/brightness",
        "/sys/class/backlight/panel0-backlight/brightness",
    };
    for (index = 0; index < sizeof(brightness_paths)/sizeof(brightness_paths[0]); index ++) {
        if (access(brightness_paths[index], 0) == 0) {
            ALOGI("[AncFingerprint] Brightness path index %d, path:%s", index, brightness_paths[index]);
            break;
        }
    }
    if (index == sizeof(brightness_paths)/sizeof(brightness_paths[0])) {
        ALOGI("[AncFingerprint] no brightness path available");
        return FINGERPRINT_RIGHTNESS_ERROR;
    }
    int fd = open(brightness_paths[index], O_RDONLY);
    if (fd < 0) {
        ALOGI("[AncFingerprint] setBrightness err:%d, errno =%d", fd, errno);
        return  FINGERPRINT_RIGHTNESS_ERROR;
    }
    length = read(fd, mBrightValue, sizeof(mBrightValue));
    if (length > 0) {
        ALOGI("[AncFingerprint] get mBrightValue = %s  length = %d ", mBrightValue, length);
    }
    else {
        ALOGI("[AncFingerprint] read brightness value fail");
        close(fd);
        return  FINGERPRINT_RIGHTNESS_ERROR;
    }
    close(fd);
    return FINGERPRINT_RIGHTNESS_SUCCESS;
}

int AncFingerprint::setBrightnessValue() {
    int32_t length = 0;
    int index = 0;
    ALOGD("[AncFingerprint]setBrightnessValue start");
    char *brightness_paths[] = {
        "/sys/class/leds/lcd-backlight/brightness",
        "/sys/class/backlight/panel0-backlight/brightness",
    };
    for (index = 0; index < sizeof(brightness_paths)/sizeof(brightness_paths[0]); index ++) {
        if (access(brightness_paths[index], 0) == 0) {
            ALOGI("[AncFingerprint] Brightness path index %d, path: %s", index, brightness_paths[index]);
            break;
        }
    }
    if (index == sizeof(brightness_paths)/sizeof(brightness_paths[0])) {
        ALOGE("[AncFingerprint] no brightness path available");
        return FINGERPRINT_RIGHTNESS_ERROR;
    }

    int fd = open(brightness_paths[index], O_WRONLY);
    if (fd < 0) {
        ALOGI("[AncFingerprint] setBrightness err:%d, errno = %d", fd, errno);
        return  FINGERPRINT_RIGHTNESS_ERROR;
    }
    length = write(fd, mBrightValue, sizeof(mBrightValue));
    ALOGI("[AncFingerprint] set mBrightValue = %s  length = %d ", mBrightValue, length);
    if (length < 0) {
        ALOGE("[AncFingerprint] write brightness value fail");
        close(fd);
        return  FINGERPRINT_RIGHTNESS_ERROR;
    }
    close(fd);
    return FINGERPRINT_RIGHTNESS_SUCCESS;
}

Return<int32_t> AncFingerprint::getEngineeringInfo(uint32_t type) {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    inputput_vector.resize(sizeof(type));
    memcpy(inputput_vector.data(), &type, sizeof(type));

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_GET_ENGINEERING_INFO,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return -1;
    }

    return 0;
}

Return<RequestStatus> AncFingerprint::touchDown() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_TOUCH_DOWN,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }

    return RequestStatus::SYS_OK;
}

Return<RequestStatus> AncFingerprint::touchUp() {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;

    int ret = this->excuteCommand(OPLUS_EXTENSION_COMMAND_TOUCH_UP,
                                  inputput_vector, outputput_vector);
    if (0 != ret) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<int32_t> AncFingerprint::sendFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& in_buf) {
    std::vector<uint8_t> inputput_vector;
    std::vector<uint8_t> outputput_vector;
    int ret = FINGERPRINT_RIGHTNESS_ERROR;

    inputput_vector.resize(in_buf.size());
    memcpy(inputput_vector.data(), in_buf.data(), in_buf.size());

    ALOGD("optical sendFingerprintCmd %d", cmdId);

    switch (cmdId) {
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIMULATOR_TP: //CMD_FINGERPRINT_TP
    {
        if (0 == in_buf[0]) {
            ALOGD("simulator_tp down");
            touchDown();
        }
        else {
            ALOGD("simulator_tp up");
            touchUp();
        }
        break;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_START_CALI:
    {
        ret = getBrightnessValue();
        if (FINGERPRINT_RIGHTNESS_ERROR == ret) {
            ALOGE("[AncFingerprint] getBrightnessValue fail");
            return -14;    //RequestStatus::SYS_EFAULT;
        }
        return 0;    //RequestStatus::SYS_OK;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_END_CALI:
    {
        ret = setBrightnessValue();
        if (FINGERPRINT_RIGHTNESS_ERROR == ret) {
            ALOGE("[AncFingerprint] setBrightnessValue fail");
            return -14;    //RequestStatus::SYS_EFAULT;
        }
        return 0;    //RequestStatus::SYS_OK;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE:
    {
        uint64_t operationId = 0;
        uint32_t gid = 0;
        int32_t authtype;

        int8_t *value = (int8_t*)in_buf.data();
        memcpy(&authtype, value, sizeof(uint32_t));
        ALOGE("FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE operationId = %lu, gid = %d, authtype = %d", operationId, gid, authtype);

        authenticateAsType(operationId, gid, authtype);
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_PAUSE_ENROLL:
    {
        pauseEnroll();
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_CONTINUE_ENROLL:
    {
        continueEnroll();
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SET_TOUCHEVENT_LISTENER:
    {
        setTouchEventListener();
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_DYNAMICALLY_LOG:
    {
        uint32_t on = 0;
        int8_t *value = (int8_t*)in_buf.data();
        memcpy(&on, value, sizeof(uint32_t));
        ALOGE("FINGERPRINT_CMD_ID_DYNAMICALLY_LOG on = %d", on);
        dynamicallyConfigLog(on);
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_GET_ENROLL_TIMES:
    {
        int32_t enroll_times = 0;
        enroll_times = getEnrollmentTotalTimes();
        return enroll_times;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SET_SCREEN_STATE:
    {
        int32_t screen_state;
        int8_t *value = (int8_t*)in_buf.data();
        memcpy(&screen_state, value, sizeof(int32_t));
        ALOGE("FINGERPRINT_CMD_ID_SET_SCREEN_STATE screen_state = %d", screen_state);
        setScreenState(screen_state);
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO:
    {
        uint32_t type = 0;
        int8_t *value = (int8_t*)in_buf.data();
        memcpy(&type, value, sizeof(uint32_t));
        ALOGE("FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO type = %d", type);
        return getEngineeringInfo(type);
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_GET_SENSOR_ID:
    {
        ALOGE("FINGERPRINT_CMD_ID_GET_SENSOR_ID = %d", fp_config_info_init.sensor_id);
        return fp_config_info_init.sensor_id;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIDE_PRESS_ENABLE:
    {
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIDE_SCREEN_STATE:
    {
        return 0;
    }
    case fingerprint_cmd_Id_t::FINGERPRINT_CMD_ID_SIDE_POWER_KEY_PRESSED:
    {
        return 0;
    }
    default:
        break;
    }

    ret = excuteCommand(cmdId, inputput_vector, outputput_vector);
    if (0 != ret) {
        return -22; //return RequestStatus::SYS_EINVAL;
    }

    return 0; //return RequestStatus::SYS_OK;
}


void AncFingerprint::onEnrollResult(void *p_device, uint32_t finger_id,
        uint32_t group_id, uint32_t remaining) {
    ALOGD("onEnrollResult, finger id:%d, group id:%d, remaining:%d", finger_id, group_id, remaining);
    ANC_UNUSED(p_device);

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (!s_instance_->sp_fp_client_callback_->onEnrollResult(reinterpret_cast<uint64_t>(s_instance_),
                finger_id, group_id, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnrollResult callback");
    }
}

#define HAL_VENDOR_BASE 1000 // draft, will fixed later
void AncFingerprint::onAcquired(void *p_device, int32_t vendor_code) {
    //int32_t amended_vendor_code = 0;
    FingerprintAcquiredInfo acquired_info = static_cast<FingerprintAcquiredInfo>(vendor_code);
    ANC_UNUSED(p_device);

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    ALOGD("onAcquired, acquired info:%d, vendor code:%d", acquired_info, vendor_code);

    /*if (vendor_code >= HAL_VENDOR_BASE) {
        amended_vendor_code = vendor_code - HAL_VENDOR_BASE;
        acquired_info = FingerprintAcquiredInfo::ACQUIRED_VENDOR;
    }
    ALOGD("onAcquired, amended acquired info:%d, amended vendor code:%d", acquired_info, amended_vendor_code);*/

    if (!s_instance_->sp_fp_client_callback_->onAcquired(reinterpret_cast<uint64_t>(s_instance_),
                acquired_info, vendor_code).isOk()) {
        ALOGE("failed to do onAcquired");
    }
}

void AncFingerprint::onAuthenticated(void *p_device, uint32_t finger_id,
        uint32_t group_id, const uint8_t* token, uint32_t token_length) {
    const std::vector<uint8_t> output_token(token, token + token_length);
    ANC_UNUSED(p_device);

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    ALOGD("OnAuthenticated, finger id:%d, group id:%d", finger_id, group_id);
    if (!s_instance_->sp_fp_client_callback_->onAuthenticated(reinterpret_cast<uint64_t>(s_instance_),
                finger_id, group_id, hidl_vec<uint8_t>(output_token)).isOk()) {
        ALOGE("failed to do onAuthenticated");
    }
}

void AncFingerprint::onError(void *p_device, int vendor_code) {
    int32_t amended_vendor_code = 0;
    FingerprintError error = static_cast<FingerprintError>(vendor_code);
    ANC_UNUSED(p_device);

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    ALOGD("OnError, error:%d, vendor code:%d", error, vendor_code);
    if (vendor_code >= HAL_VENDOR_BASE) {
        amended_vendor_code = vendor_code - HAL_VENDOR_BASE;
        error = FingerprintError::ERROR_VENDOR;
    }
    ALOGD("OnError, amended error:%d, amended vendor code:%d", error, amended_vendor_code);
    if (!s_instance_->sp_fp_client_callback_->onError(reinterpret_cast<uint64_t>(s_instance_),
                error, amended_vendor_code).isOk()) {
        ALOGE("failed to do onError");
    }
}

void AncFingerprint::onRemoved(void *p_device, uint32_t finger_id,
        uint32_t group_id, uint32_t remaining) {
    ALOGD("OnRemoved, finger id:%d, group id:%d, remaining:%d", finger_id, group_id, remaining);
    ANC_UNUSED(p_device);

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (!s_instance_->sp_fp_client_callback_->onRemoved(reinterpret_cast<uint64_t>(s_instance_),
                finger_id, group_id, remaining).isOk()) {
        ALOGE("failed to do onRemoved");
    }
}

void AncFingerprint::onEnumerate(void *p_device, uint32_t finger_id,
        uint32_t group_id, uint32_t remaining) {
    ALOGD("OnEnumerate, finger id:%d, group id:%d, remaining:%d", finger_id, group_id, remaining);
    ANC_UNUSED(p_device);

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (!s_instance_->sp_fp_client_callback_->onEnumerate(reinterpret_cast<uint64_t>(s_instance_),
                finger_id, group_id, remaining).isOk()) {
        ALOGE("failed to do onEnumerate");
    }
}

void AncFingerprint::onTouchDown(void *p_device) {
    ALOGD("onTouchDown");
    ANC_UNUSED(p_device);
    const uint64_t devId = reinterpret_cast<uint64_t>(s_instance_);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    if (s_instance_->sp_fp_client_callback_Ex == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (!s_instance_->sp_fp_client_callback_Ex->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_DOWN,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
        ALOGE("failed to do onTouchDown");
    }
}

void AncFingerprint::onTouchUp(void *p_device) {
    ALOGD("onTouchUp");
    ANC_UNUSED(p_device);
    const uint64_t devId = reinterpret_cast<uint64_t>(s_instance_);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    if (s_instance_->sp_fp_client_callback_Ex == nullptr) {
        ALOGE("fingerprint callbackEx is nullptr");
        return;
    }

    if (!s_instance_->sp_fp_client_callback_Ex->onFingerprintCmd(FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_UP,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
        ALOGE("failed to do onTouchUp");
    }
}

void AncFingerprint::onMonitorEventTriggered(const uint8_t *p_out, uint32_t out_length) {
    uint8_t *p_output = const_cast<uint8_t*>(p_out);
    uint32_t type = 0;

    ALOGD("onMonitorEventTriggered");

    if (s_instance_->sp_fp_client_callback_Ex == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (sizeof(type) > out_length) {
        ALOGE("need size:%lu, output size:%ud", sizeof(type), out_length);
        return;
    }

    memcpy(&type, p_output, sizeof(type));
    p_output += sizeof(type);
    hidl_string data = reinterpret_cast<char *>(p_output);

    //if (!s_instance_->sp_fp_client_callback_Ex->onMonitorEventTriggered(type, data).isOk()) {
    //    ALOGE("failed to do onMonitorEventTriggered");
    //}
}


void AncFingerprint::onImageInfoAcquired(const uint8_t *p_out, uint32_t out_length) {
    uint8_t *p_output = const_cast<uint8_t*>(p_out);
    uint32_t type = 0;
    uint32_t quality = 0;
    uint32_t match_score = 0;
    uint32_t output_length = sizeof(type) + sizeof(quality) + sizeof(match_score);

    ALOGD("onImageInfoAcquired");

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (output_length > out_length) {
        ALOGE("need size:%ud, output size:%ud", output_length, out_length);
        return;
    }

    memcpy(&type, p_output, sizeof(type));
    p_output += sizeof(type);
    memcpy(&quality, p_output, sizeof(quality));
    p_output += sizeof(quality);
    memcpy(&match_score, p_output, sizeof(match_score));

    //if (!s_instance_->sp_fp_client_callback_->onImageInfoAcquired(type, quality, match_score).isOk()) {
    //    ALOGE("failed to do onImageInfoAcquired");
    //}
}

void AncFingerprint::onSyncTemplates(void *p_device, const uint8_t *p_out, uint32_t out_length) {
    uint8_t *p_output = const_cast<uint8_t*>(p_out);
    uint32_t group_id = 0;
    uint32_t output_length = sizeof(group_id);
    uint32_t output_finger_ids_length = out_length - sizeof(group_id);
    ALOGD("onSyncTemplates, out_length = %d, ids_length = %d", out_length, output_finger_ids_length);

    ALOGD("onSyncTemplates");
    ANC_UNUSED(p_device);

    if (s_instance_->sp_fp_client_callback_ == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (output_length > out_length) {
        ALOGE("need size:%ud, output size:%ud", output_length, out_length);
        return;
    }

    memcpy(&group_id, p_output, sizeof(group_id));
    p_output += sizeof(group_id);

    int32_t rem = output_finger_ids_length/sizeof(uint32_t);
    int32_t *fids = (int32_t*)malloc(rem * sizeof(int32_t));
    if (fids == NULL) {
        ALOGE("malloc fail");
        return;
    }
    memcpy(fids, p_output, output_finger_ids_length);

    ALOGD("onEnumerate()");
    if (rem == 0) {
        ALOGD("fingerprint template is null");
        if (!s_instance_->sp_fp_client_callback_->onEnumerate(reinterpret_cast<uint64_t>(s_instance_),
                    0, group_id, 0).isOk()) {
            ALOGE("failed to do onEnumerate");
        }
    }

    for (int32_t index = 0; index < rem; index++) {
        ALOGE("fids[%d] = %d, gids[%d] = %d", index, fids[index], index, group_id);
        if (!s_instance_->sp_fp_client_callback_->onEnumerate(reinterpret_cast<uint64_t>(s_instance_),
                    fids[index], group_id, rem - index - 1).isOk()) {
            ALOGE("failed to do onEnumerate");
        } else {
            ALOGD("BSP sucess to do onEnumerate, count = %d --", index+1);
        }
    }
    if (fids) {
        free(fids);
        fids = NULL;
    }
}

hidl_string AncFingerprint::getHidlstring(uint32_t param) {
        char data[64];
        snprintf(data, 63, "%u", param);
        ALOGD("getString16(param=%u)", param);
        return hidl_string(data, strlen(data));
}


void AncFingerprint::onEngineeringInfoUpdated(const uint8_t *p_out, uint32_t out_length) {
    uint8_t *p_output = const_cast<uint8_t*>(p_out);
    uint32_t lenth = 0;
    hidl_vec<uint32_t> keys;
    uint32_t keys_length = 0;
    std::vector<hidl_string> values;
    hidl_vec<uint32_t> temp_values;
    uint32_t values_length = 0;
    uint32_t output_length = sizeof(lenth) + sizeof(keys_length) + sizeof(values_length)
                            + keys_length + values_length;

    ALOGD("onEngineeringInfoUpdated");

    if (s_instance_->sp_fp_client_callback_Ex == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (output_length > out_length) {
        ALOGE("need size:%ud, output size:%ud", output_length, out_length);
        return;
    }

    memcpy(&lenth, p_output, sizeof(lenth));
    p_output += sizeof(lenth);
    memcpy(&keys_length, p_output, sizeof(keys_length));
    p_output += sizeof(keys_length);
    memcpy(&values_length, p_output, sizeof(values_length));
    p_output += sizeof(values_length);
    keys.resize(keys_length);
    memcpy(keys.data(), p_output, keys_length*sizeof(uint32_t));
    p_output += keys_length*sizeof(uint32_t);
    temp_values.resize(values_length);
    memcpy(temp_values.data(), p_output, values_length*sizeof(uint32_t));

    for (int i=0; i < values_length; i++) {
        values.push_back(getHidlstring(temp_values[i]));
        ALOGD("keys [%d] = %d, temp_values [%d] %d", i, keys[i], i, temp_values[i]);
        ALOGD("values [%d] %s", i, values[i].c_str());
        ALOGD("values [%d] %s", i, getHidlstring(temp_values[i]).c_str());
    }
    for (int i=0; i < values.size(); i++) {
        ALOGD("values [%d] %s", i, values[i].c_str());
    }

    if (!s_instance_->sp_fp_client_callback_Ex->onEngineeringInfoUpdated(lenth, keys, (hidl_vec<hidl_string>)values).isOk()) {
        ALOGE("failed to do onEngineeringInfoUpdated");
    }
}

void AncFingerprint::onFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& result, uint32_t resultLen) {
    ALOGD("onFingerprintCmd");

    for (int i=0; i < resultLen; i++) {
        ALOGD("reslut[%d] %c", i, result[i]);
    }
    if (s_instance_->sp_fp_client_callback_Ex == nullptr) {
        ALOGE("fingerprint callback is nullptr");
        return;
    }

    if (!s_instance_->sp_fp_client_callback_Ex->onFingerprintCmd(cmdId, result, resultLen).isOk()) {
        ALOGE("failed to do onFingerprintCmd");
    }
}


int AncFingerprint::excuteCommand(int32_t command_id, const std::vector<uint8_t>& inputData,
                                    std::vector<uint8_t>& outputData) {
    uint8_t *p_output_buffer = nullptr;
    uint32_t output_buffer_length = 0;
    ALOGD("anc AncFingerprintExtension, excuteCommand: %d", command_id);

    int ret = AncExcuteCommand(p_device_, command_id,
                            static_cast<const uint8_t*>(inputData.data()), (uint32_t)inputData.size(),
                            (uint8_t **)(&p_output_buffer), (uint32_t *)&output_buffer_length);
    if (0 == ret) {
        outputData.resize(output_buffer_length);
        memcpy(outputData.data(), p_output_buffer, output_buffer_length);
    }

    return ret;
}

void AncFingerprint::onExcuteCommand(void *p_device, int32_t command_id, int32_t argument, const uint8_t *p_out, uint32_t out_length) {
    std::vector<int8_t> output_vector;
    ANC_UNUSED(p_device);
    ANC_UNUSED(argument);

    ALOGD("OnExcuteCommand, command_id:%d, out:%p, out length:%d", command_id, p_out, out_length);
    for (uint32_t i = 0; i < out_length; i++) {
        output_vector.push_back((int8_t)*(p_out + i));
    }

    switch (command_id) {
        case OPLUS_EXTENSION_COMMAND_TOUCH_DOWN:
            onTouchDown(p_device);
            break;
        case OPLUS_EXTENSION_COMMAND_TOUCH_UP:
            onTouchUp(p_device);
            break;
        case OPLUS_EXTENSION_COMMAND_CB_MONITOR_EVENT_TRIGGERED:
            onMonitorEventTriggered(p_out, out_length);
            break;
        case OPLUS_EXTENSION_COMMAND_CB_IMAGE_INFO_ACQUIRED:
            onImageInfoAcquired(p_out, out_length);
            break;
        case OPLUS_EXTENSION_COMMAND_CB_SYNC_TEMPLATES:
            onSyncTemplates(p_device, p_out, out_length);
            break;
        case OPLUS_EXTENSION_COMMAND_CB_ENGINEERING_INFO_UPDATED:
            onEngineeringInfoUpdated(p_out, out_length);
            break;
        default :
            onFingerprintCmd(command_id, hidl_vec<int8_t>(output_vector), out_length);
            break;
    }
}

void AncFingerprint::doExternalWork(void *p_device, int32_t type, const uint8_t *p_buffer, uint32_t buffer_length) {
    int32_t scaling_time = 0;
    ANC_UNUSED(p_device);
    int32_t pid = 0;
    int32_t tid = 0;
    int32_t enable = 0;
#ifdef FP_DCS_ENABLE
    oplus_fingerprint_dcs_info_t report_dcs_info;
#endif /* FP_DCS_ENABLE */
    ALOGD("DoExternalWork, type:%d, buffer:%p, buffer length:%d", type, p_buffer, buffer_length);

    switch (type) {
        case ANC_EXTERNAL_BIND_CORE:
            tid = gettid();
            Hypnus::getInstance()->bind_big_core_bytid(tid);
            ALOGE("bind_big_core_bytid tid : %d", tid);
            break;
        case ANC_EXTERNAL_SCALE_CPU_FREQUENCY:
            if ((p_buffer != nullptr) && (sizeof(scaling_time) == buffer_length)) {
                memcpy(&scaling_time, p_buffer, sizeof(scaling_time));
                Hypnus::getInstance()->setAction(ACTION_TYPE, scaling_time);
                ALOGE("scaling time : %d", scaling_time);
            }
            break;
        case ANC_SEND_DCS_EVENT_INFO:
#ifdef FP_DCS_ENABLE
            memset(&report_dcs_info, 0, sizeof(oplus_fingerprint_dcs_info_t));
            memcpy(&report_dcs_info, p_buffer, buffer_length);
            Dcs::getInstance()->reportDcsEventInfo(report_dcs_info);
#endif /* FP_DCS_ENABLE */
            break;
        case ANC_SETUXTHREAD:
            memcpy(&pid, p_buffer, sizeof(int32_t));
            p_buffer += sizeof(int32_t);
            memcpy(&tid, p_buffer, sizeof(int32_t));
            p_buffer += sizeof(int32_t);
            memcpy(&enable, p_buffer, sizeof(int32_t));
            p_buffer += sizeof(int32_t);
            Hypnus::getInstance()->setUxthread(pid, tid, enable);
            break;
        default :
            ALOGE("invalid type : %d", type);
            break;
    }
}



}  // namespace implementation
}  // namespace V2_1
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace oplus
}  // namespace vendor

