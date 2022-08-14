//
//    Copyright 2017 Egis Technology Inc.
// 
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Egis in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//

#include <stdint.h>
#include <vector>

#include "rbs_hidl.h"
#include "rbs_fingerprint.h"
#include <sched.h>
#define LOG_TAG "FingerprintHal"
#include "plat_log.h"
#include "vendor_custom.h"
#include "fingerprint_type.h"
#ifdef FP_DCS_ENABLE
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"
#endif
#define    FINGERPRINT_CMD_ID_NONE 0
#define    FINGERPRINT_PRODUCT_TEST_CMD_GET_OTP_QRCODE  31
#define    FINGERPRINT_CMD_ID_BASE  1000
#define    FINGERPRINT_CMD_ID_CAMERA  1001
#define    FINGERPRINT_CMD_ID_SIMULATOR_TP  1002
#define    FINGERPRINT_CMD_ID_START_CALI  1003
#define    FINGERPRINT_CMD_ID_END_CALI  1004
#define    FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE  1005
#define    FINGERPRINT_CMD_ID_PAUSE_ENROLL  1006
#define    FINGERPRINT_CMD_ID_CONTINUE_ENROLL  1007
#define    FINGERPRINT_CMD_ID_SET_TOUCHEVENT_LISTENER  1008
#define    FINGERPRINT_CMD_ID_DYNAMICALLY_LOG  1009
#define    FINGERPRINT_CMD_ID_GET_ENROLL_TIMES  1010
#define    FINGERPRINT_CMD_ID_SET_SCREEN_STATE  1011
#define    FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO  1012
#define    FINGERPRINT_CMD_ID_GET_SENSOR_ID  1013
#define    FINGERPRINT_CMD_ID_SIDE_PRESS_ENABLE  1014
#define    FINGERPRINT_CMD_ID_SIDE_SCREEN_STATE  1015
#define    FINGERPRINT_CMD_ID_SIDE_POWER_KEY_PRESSED  1016
using android::Hypnus;
typedef struct {
    char *key;
    char value[64];
} key_value_t;

ets_hidl::ets_hidl() : mDevice(nullptr), mClientCallbackEx(nullptr)
{
    ets_callback.on_acquired = on_acquired;
    ets_callback.on_authenticated = on_authenticated;
    ets_callback.on_enroll_result = on_enroll_result;
    ets_callback.on_enumerate = on_enumerate;
    ets_callback.on_error = on_error;
    ets_callback.on_removed = on_removed;
    ets_callback.on_touch_down = on_touch_down;
    ets_callback.on_touch_up = on_touch_up;
    //ets_callback.on_monitor_event_triggered = on_monitor_event_triggered;
    //ets_callback.on_image_info_acquired = on_image_info_acquired;
    //ets_callback.on_sync_templates = on_sync_templates;
    ets_callback.on_engineering_info_updated = on_engineering_info_updated;
    ets_callback.on_fingerprint_cmd = on_fingerprint_cmd;
    ets_callback.on_send_dcsmsg = on_send_dcsmsg;
    ets_callback.set_action = set_action;
}

ets_hidl::~ets_hidl()
{
    close();
}

int ets_hidl::close()
{
    if(mDevice != nullptr) {
        fingerprint_close(mDevice);
        mDevice = nullptr;
        egislog_e("ets_hidl close");
    }
    return 0;
}

void ets_hidl::egis_bind_big_core()
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(6, &mask);
    CPU_SET(7, &mask);
    sched_setaffinity(getpid(), sizeof(cpu_set_t), (cpu_set_t *)&mask);
    egislog_e("egis bind big core");
    return;
}

int ets_hidl::open()
{
    int ret = fingerprint_open(&mDevice);
    if(!ret) {
        fingerprint_set_callback(&mDevice, &ets_callback, this);
    }
    egis_bind_big_core();
    return ret;
}

Return<uint64_t> ets_hidl::setNotify(const sp<IBiometricsFingerprintClientCallback>& clientCallback)
{
    mClientCallback = clientCallback;
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> ets_hidl::preEnroll()
{
    return fingerprint_pre_enroll(mDevice);
}

Return<RequestStatus> ets_hidl::enroll(const hidl_array<uint8_t, 69>& hat,
                                       uint32_t gid, uint32_t timeout_sec)
{
    int ret = fingerprint_enroll(mDevice, static_cast<const uint8_t*>(hat.data()),
                gid, timeout_sec);
    if (ret != 0) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::postEnroll()
{
    if (fingerprint_post_enroll(mDevice)) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::cancel()
{
    fingerprint_cancel();
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::enumerate()
{
    fingerprint_enumerate();
    return RequestStatus::SYS_OK;
}

Return<uint64_t> ets_hidl::getAuthenticatorId()
{
    return fingerprint_get_auth_id(mDevice);
}

Return<RequestStatus> ets_hidl::remove(uint32_t gid, uint32_t fid)
{

    if (fingerprint_remove(gid, fid)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::setActiveGroup(uint32_t gid, const hidl_string& storePath)
{
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        egislog_e("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        return RequestStatus::SYS_EINVAL;
    }
    if (fingerprint_set_active_group(gid, storePath.c_str())) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::authenticate(uint64_t operation_id, uint32_t gid)
{
    if (fingerprint_authenticate(mDevice, operation_id, gid)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

//For OPLUS
Return<RequestStatus> ets_hidl::authenticateAsType(uint64_t operation_id, uint32_t gid, int32_t authtype)
{
    egislog_d("%s(operation_id=%d, gid=%d, authtype=%d)", __func__, operation_id, gid, authtype);
	return RequestStatus::SYS_OK;
}

Return<int32_t> ets_hidl::getEnrollmentTotalTimes()
{
    egislog_d("%s enter", __func__);
    return VENDOR_ET7XX_ENROLL_COUNTS;
}

Return<RequestStatus> ets_hidl::pauseEnroll()
{
    egislog_d("%s enter", __func__);
    fingerprint_pause_enroll();
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::continueEnroll()
{
    egislog_d("%s enter", __func__);
    fingerprint_continue_enroll();
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::pauseIdentify()
{
    egislog_d("%s enter", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::continueIdentify()
{
    egislog_d("%s enter", __func__);
    return RequestStatus::SYS_OK;
}


Return<RequestStatus> ets_hidl::setTouchEventListener()
{
    egislog_d("%s enter", __func__);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::setScreenState(int32_t screen_state)
{
    egislog_d("%s setScreenState enter!", __func__);
    int32_t state = static_cast<int32_t>(screen_state);
    fingerprint_set_screen_state(state);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> ets_hidl::dynamicallyConfigLog(uint32_t on)
{
    egislog_d("%s enter", __func__);
    return RequestStatus::SYS_OK;
}

Return<int32_t> ets_hidl::getEngineeringInfo(uint32_t type)
{
    int32_t ret = -1;
    egislog_d("%s enter %d type", __func__, type);
    ret = fingerprint_get_engineering_info(type);
    egislog_d("%s ret %d", __func__, ret);
    return ret;
}

#define CMD_HOST_TOUCH_SET_TOUCHING 101
#define CMD_HOST_TOUCH_RESET_TOUCHING 102
#define CMD_HOST_TOUCH_SET_UI_READY 200
#define PID_HOST_TOUCH 7

Return<int32_t> ets_hidl::sendFingerprintCmd(int32_t cmdId, const hidl_vec<int8_t>& in_buf)
{
    ALOGD("[ANC_HIDL]sendFingerprintCmd, cmd id %d, in_buf= %s ", cmdId, (int8_t*)in_buf.data());
    int ret = -1/*FINGERPRINT_RIGHTNESS_ERROR*/;
    switch (cmdId) {
        case FINGERPRINT_CMD_ID_CAMERA: //CMD_FINGERPRINT_CAMERA
            ALOGD("keymode_enable(enable = %d)", in_buf[0]);
            //ret = fpc_keymode_enable(hal, static_cast<int32_t>(in_buf[0]));
            break;
        case FINGERPRINT_PRODUCT_TEST_CMD_GET_OTP_QRCODE:
            ALOGD("sendfingercmd = %d)", cmdId);
            //ret = fpc_hal_notify_qrcode(hal, cmdId);
            break;
        case FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE:
        {
            uint64_t operationId = 0;
            uint32_t gid = 0;
            int32_t authtype;

            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&authtype, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_AUTHENTICATE_TYPE operationId = %d, gid = %d, authtype = %d", operationId, gid, authtype);

            authenticateAsType(operationId, gid, authtype);
            return 0;
        }
        case FINGERPRINT_CMD_ID_PAUSE_ENROLL:
        {
            pauseEnroll();
            return 0;
        }
        case FINGERPRINT_CMD_ID_CONTINUE_ENROLL:
        {
            continueEnroll();
            return 0;
        }
        case FINGERPRINT_CMD_ID_SET_TOUCHEVENT_LISTENER:
        {
            setTouchEventListener();
            return 0;
        }
        case FINGERPRINT_CMD_ID_DYNAMICALLY_LOG:
        {
            uint32_t on = 0;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&on, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_DYNAMICALLY_LOG on = %d", on);
            dynamicallyConfigLog(on);
            return 0;
        }
        case FINGERPRINT_CMD_ID_GET_ENROLL_TIMES:
        {
            int32_t enroll_times = 0;
            enroll_times = getEnrollmentTotalTimes();
            return enroll_times;
        }
        case FINGERPRINT_CMD_ID_SET_SCREEN_STATE:
        {
            int32_t screen_state;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&screen_state, value, sizeof(int32_t));
            ALOGE("FINGERPRINT_CMD_ID_SET_SCREEN_STATE screen_state = %d", screen_state);
            setScreenState(screen_state);
            return 0;
        }
        case FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO:
        {
            uint32_t type = 0;
            int8_t *value = (int8_t*)in_buf.data();
            memcpy(&type, value, sizeof(uint32_t));
            ALOGE("FINGERPRINT_CMD_ID_GET_ENGINEERING_INFO type = %d", type);
            return getEngineeringInfo(type);
        }
        case FINGERPRINT_CMD_ID_GET_SENSOR_ID:
        {
            ALOGE("FINGERPRINT_CMD_ID_GET_SENSOR_ID = %d", fp_config_info_init.sensor_id);
            return fp_config_info_init.sensor_id;
        }
        case FINGERPRINT_CMD_ID_SIDE_PRESS_ENABLE:
        {
            return 0;
        }
        case FINGERPRINT_CMD_ID_SIDE_SCREEN_STATE:
        {
            return 0;
        }
        case FINGERPRINT_CMD_ID_SIDE_POWER_KEY_PRESSED:
        {
            return 0;
        }
        default:
            break;
    }
    egislog_d("%s enter cmdId = %d", __func__, cmdId);
    int cmdRet = fingerprint_sendFingerprintCmd(cmdId, static_cast<const int8_t*>(in_buf.data()), in_buf.size());
    egislog_d("%s cmdRet : %d", __func__, cmdRet);
    return cmdRet;
}

void ets_hidl::on_enroll_result(void* context, uint32_t fid, uint32_t gid, uint32_t remaining)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    egislog_d("onEnrollResult(fid=%d, gid=%d, remaing=%d)", fid, gid, remaining);
    if(!me->mClientCallback->onEnrollResult(reinterpret_cast<uint64_t>(context), fid, gid, remaining).isOk()) {
        egislog_e("failed to invoke fingerprint onEnrollResult callback");
    }
}

void ets_hidl::on_acquired(void* context, int info)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    int32_t vendor_code = 0;
    FingerprintAcquiredInfo acquired_info = static_cast<FingerprintAcquiredInfo>(info);
    int accquired_vendor = static_cast<int>(FingerprintAcquiredInfo::ACQUIRED_VENDOR);

	/*
    if (info >= accquired_vendor) {
        vendor_code = info - accquired_vendor;
        acquired_info = FingerprintAcquiredInfo::ACQUIRED_VENDOR;
    }
	*/
    egislog_d("onAcquired(info=%d, vendor=%d)", acquired_info, vendor_code);
    if(!me->mClientCallback->onAcquired(reinterpret_cast<uint64_t>(context), acquired_info, vendor_code).isOk()){
        egislog_e("failed to invoke fingerprint onAcquired callback");
    }
}

void ets_hidl::on_authenticated(void* context, uint32_t fid, uint32_t gid,
                                const uint8_t* token, uint32_t size_token)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    const std::vector<uint8_t> hidl_token(token, token + size_token);
    egislog_d("onAuthenticated(fid=%d, gid=%d)", fid, gid);
    if(!me->mClientCallback->onAuthenticated(reinterpret_cast<uint64_t>(context), fid, gid,
                                  hidl_vec<uint8_t>(hidl_token)).isOk()){
        egislog_e("failed to invoke fingerprint onAuthenticated callback(fid=%d, gid=%d)", fid, gid);
    }
}

void ets_hidl::on_error(void* context, int code)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    int32_t vendor_code = 0;
    FingerprintError error = static_cast<FingerprintError>(code);
    int error_vendor = static_cast<int>(FingerprintError::ERROR_VENDOR);

    if (code >= error_vendor) {
        vendor_code = code - error_vendor;
        error = FingerprintError::ERROR_VENDOR;
    }
    egislog_e("onError(error=%d, vendor=%d)", code, vendor_code);
    if(!me->mClientCallback->onError(reinterpret_cast<uint64_t>(context), error, vendor_code).isOk()){
        egislog_e("failed to invoke fingerprint onError callback");
    }
}

void ets_hidl::on_removed(void* context, uint32_t fid, uint32_t gid,
                   uint32_t remaining)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    egislog_d("onRemoved(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if(!me->mClientCallback->onRemoved(reinterpret_cast<uint64_t>(context), fid, gid, remaining).isOk()){
        egislog_e("failed to invoke fingerprint onRemoved callback");
    }
}

void ets_hidl::on_enumerate(void* context, uint32_t fid,
                                       uint32_t gid, uint32_t remaining)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    egislog_d("onEnumerate(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if(!me->mClientCallback->onEnumerate(reinterpret_cast<uint64_t>(context), fid, gid, remaining).isOk()){
        egislog_e("failed to invoke fingerprint onEnumerate callback");
    }
}

void ets_hidl::on_touch_down(void* context)
{
    egislog_d("onTouchDown()");
    ets_hidl* me = static_cast<ets_hidl*>(context);
    const uint64_t devId = reinterpret_cast<uint64_t>(context);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    if (me->mClientCallbackEx == nullptr) {
        egislog_e("fingerprint callback is nullptr");
        return;
    }

    if (!me->mClientCallbackEx->onFingerprintCmd(1201/*FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_DOWN*/,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
        egislog_e("failed to do onTouchDown");
    }
}

void ets_hidl::on_touch_up(void* context)
{
    egislog_d("onTouchUp()");
    ets_hidl* me = static_cast<ets_hidl*>(context);
    const uint64_t devId = reinterpret_cast<uint64_t>(context);
    const std::vector<int8_t> deviceId(&devId, &devId + sizeof(uint64_t));
    if (me->mClientCallbackEx == nullptr) {
        egislog_e("fingerprint callback is nullptr");
        return;
    }

    if (!me->mClientCallbackEx->onFingerprintCmd(1202/*FINGERPRINT_CALLBACK_CMD_ID_ON_TOUCH_UP*/,
                    hidl_vec<int8_t>(deviceId), sizeof(uint64_t)).isOk()) {
        egislog_e("failed to do onTouchUp");
    }
}

#if 0
void ets_hidl::on_monitor_event_triggered(void* context, uint32_t type, char* data)
{
    std::string data_string = data;
    ets_hidl* me = static_cast<ets_hidl*>(context);
    egislog_d("onMonitorEventTriggered(type=%d, data=%s)", type, data);
    if(!me->mClientCallback->onMonitorEventTriggered(type, data_string).isOk()){
       egislog_e("failed to invoke fingerprint onMonitorEventTriggered callback");
    }
}

void ets_hidl::on_image_info_acquired(void* context, uint32_t type, uint32_t quality, uint32_t match_score)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    egislog_d("onImageInfoAcquired(type=%d, quality=%d, match_score=%d)", type, quality, match_score);
    if (!me->mClientCallback->onAcquired(reinterpret_cast<uint64_t>(context),
                quality, match_score).isOk()) {
         egislog_e("failed to invoke fingerprint onImageInfoAcquired callback");
    }

}

void ets_hidl::on_sync_templates(void* context, uint32_t* fingerIds, int fingerIds_size, uint32_t groupId)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    std::vector<uint32_t> fingerIds_vec;
    for(int i = 0; i < fingerIds_size; i++) {
        fingerIds_vec.push_back(*(fingerIds + i));
        egislog_d("onSyncTemplates(fingerIds = %d)", *(fingerIds + i));
    }
    egislog_d("onSyncTemplates(groupId=%d)", groupId);
    /*if(!me->mClientCallback->onSyncTemplates(reinterpret_cast<uint64_t>(context), hidl_vec<uint32_t>(fingerIds_vec), groupId).isOk()){
        egislog_e("failed to invoke fingerprint onSyncTemplates callback");
    }*/
}
#endif

hidl_string ets_hidl::getHidlstring(uint32_t param)
{
    char data[64];
    snprintf(data, 63, "%u", param);
    egislog_d("getString16 param=%u", param);
    return hidl_string(data, strlen(data));
}

void ets_hidl::on_engineering_info_updated(void* context, uint32_t lenth, uint32_t* keys, int keys_size, uint32_t* values, int values_size)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
	uint32_t* key = (uint32_t*)malloc(keys_size * sizeof(uint32_t));
	hidl_string *value = NULL;

	value = new hidl_string[values_size];
	/*
    std::vector<uint32_t> keys_vec;
    std::vector<hidl_string> values_vec;
	*/
    egislog_d("onEngineeringInfoUpdated(lenth=%d, keys_size=%d, values_size=%d)", lenth, keys_size, values_size);
    for(int i = 0; i < keys_size; i++) {
        //keys_vec.push_back(*(keys + i));
		*(key+i) = *(keys+i);
        egislog_d("onEngineeringInfoUpdated(keys = %d)", *(keys + i));
    }
    for(int i = 0; i < values_size; i++) {
        //values_vec.push_back(getHidlstring(*(values + i)));
		*(value+i) = getHidlstring(*(values+i));
        egislog_d("onEngineeringInfoUpdated(values = %d)", *(values + i));
    }

    const std::vector<uint32_t> keys_vec(key, key + lenth);
    const std::vector<hidl_string> values_vec(value, value + lenth);
    if(!me->mClientCallbackEx->onEngineeringInfoUpdated(lenth, hidl_vec<uint32_t>(keys_vec), hidl_vec<hidl_string>(values_vec)).isOk()){
        egislog_e("failed to invoke fingerprint onEngineeringInfoUpdated callback");
    }
	free(key);
	delete[] value;
}

void ets_hidl::on_fingerprint_cmd(void* context, int32_t cmdId, int8_t* result, uint32_t resultLen)
{
    ets_hidl* me = static_cast<ets_hidl*>(context);
    std::vector<int8_t> result_vec;
    egislog_d("onFingerprintCmd(cmdId=%d,  resultLen=%d)", cmdId, resultLen);
    for(int i = 0; i < resultLen; i++) {
        result_vec.push_back(*(result + i));
        egislog_d("onFingerprintCmd(result = %d)", *(result + i));
    }
    if(!me->mClientCallbackEx->onFingerprintCmd(cmdId, hidl_vec<int8_t>(result_vec), resultLen).isOk()){
        egislog_e("failed to invoke fingerprint onFingerprintCmd callback");
    }
}

void ets_hidl::set_action(void* context, uint32_t type, uint32_t time_out) {
    egislog_d(" set_action start");
#ifdef FP_HYPNUSD_ENABLE
    Hypnus::getInstance()->setAction(type, time_out);
#endif
    egislog_d("set_action end");
}

void ets_hidl::on_send_dcsmsg(void* context, int32_t cmdId, int8_t* dcsmsg, uint32_t dcsmsgLen)
{
#ifdef FP_DCS_ENABLE
    egislog_d("enter into FINGERPRINT_AUTHENTICATED CommonDcsmsg");
    sp<::vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService> service = vendor::oplus::hardware::commondcs::V1_0::ICommonDcsHalService::getService("commondcsservice");
    if (service != NULL) {
        egislog_d(" send fingerprint auth dcsmsg");
        hidl_vec<vendor::oplus::hardware::commondcs::V1_0::StringPair> fp_dcsmsg;
        int i;
        int total = dcsmsgLen/sizeof(key_value_t);
        vendor::oplus::hardware::commondcs::V1_0::StringPair dataArray[total];
        key_value_t *msg = (key_value_t *)dcsmsg;
        for(int i=0;i<total;i++) {
            dataArray[i].key = msg[i].key;
            dataArray[i].value = msg[i].value;
            egislog_d("getString[%d] param=%s",i, (dataArray[i].value).c_str());
        }
        fp_dcsmsg.setToExternal(dataArray, total);
        service->notifyMsgToCommonDcs(fp_dcsmsg, "20120", "fingerprint_auth_dcsmsg");
    } else {
        egislog_e("service NULL");
    }
#endif
}

Return<bool> ets_hidl::isUdfps(uint32_t sensorId)
{
	 ALOGD("isUdfps sensor_id = %d", sensorId);
        if(sensorId >= E_SENSOR_ID_MAX) {
            ALOGE("unknown sensor");
            return false;
        }
        return (fp_config_info[sensorId].fp_type) == 4? true:false;
}

Return<void> ets_hidl::onFingerDown(uint32_t x, uint32_t y, float minor, float major)
{
	 ALOGD("onFingerDown");
        //mDevice->touchDown(mDevice);
        return Void();
}

Return<void> ets_hidl::onFingerUp()
{
	 ALOGD("onFingerUp");
        //mDevice->touchDown(mDevice);
        return Void();
}

Return<uint64_t> ets_hidl::setHalCallback(const sp<IBiometricsFingerprintClientCallbackEx>& clientCallbackEx)
{
	std::lock_guard<std::mutex> lock(callback_mutex);
    mClientCallbackEx = clientCallbackEx;
    return reinterpret_cast<uint64_t>(this);
	//return reinterpret_cast<uint64_t>(&mDevice);
}