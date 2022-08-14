/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _CUSTOMIZEDFINGERPRINTCORE_H_
#define _CUSTOMIZEDFINGERPRINTCORE_H_

#include "DelmarFingerprintCore.h"
#include "CustomizedSensorConfigProvider.h"
#include "gf_customized_types.h"


namespace goodix {
    typedef enum {
        CUSTOMIZED_FINGERPRINT_ACQUIRED_FINGER_DOWN = 1001,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_FINGER_UP = 1002,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = 1003,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_DUPLICATE_AREA = 1004,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_TOO_WET = 1005,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_DRYFINGER = 1006,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_SUNLIGHT = 1007,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_LOW_BRIGHTNESS = 1008,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_FIXED_PATTERN = 1009,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = 1010,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE = 1011,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_CAPTURE_IMAGE_FAILED = 1012,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_NOT_LIVE_FINGER = 1013,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_RESIDUAL_FINGER = 1014,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_DIRTY_SCREEN = 1015,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = 1016,
        CUSTOMIZED_FINGERPRINT_ACQUIRED_SIMULATED_FINGER = 1017,
    } CustomizedFingerprintAcquiredInfo;

    typedef struct {
        int32_t pressX;
        int32_t pressY;
        int32_t touchMajor;
        int32_t touchMinor;
        int32_t touchOrientation;
        int32_t enrollicon;
        int32_t authicon;
    } SensorPressInfo;

    class CustomizedFingerprintCore : public DelmarFingerprintCore, public MsgBus::IMsgListener {
    public:
        explicit CustomizedFingerprintCore(HalContext *context);
        virtual ~CustomizedFingerprintCore();
        gf_error_t onSensorPressInfo(int32_t touchCenterX, int32_t touchCenterY,
                                     int32_t touchMajor, int32_t touchMinor, int32_t touchOrientation);
        gf_error_t notifyTouch(gf_fingerprint_msg_type_t type);
    protected:
        // override MsgBus::IMsgListener
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        virtual void onError(gf_error_t err);
        virtual gf_error_t notifyAcquiredInfo(gf_fingerprint_acquired_info_t info);
        virtual gf_error_t onEnrollStart(EnrollContext *context);
        virtual gf_error_t onAuthStart(AuthenticateContext *context);
        virtual gf_error_t onAuthStop(AuthenticateContext *context);
        virtual gf_error_t onBeforeAuthCapture(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthCapture(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t onBeforeAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t send_auth_dcsmsg(AuthenticateContext* context, bool notified);
        virtual bool needRetryIfCaptureFailed(AuthenticateContext* context);
        virtual gf_error_t dumpKPI(const char *func_name);
    private:
        gf_error_t getAuthDetail(const MsgBus::Message &msg);
        gf_error_t loadSensorConfig();
        void setSensorInfo();
        gf_error_t getAlgoVersion();
        //uint32_t getActualRetryTimes(gf_error_t result, uint32_t retry, gf_customized_auth_detail_get_t detail);
        void initcheckcalibration();
        GoodixSensorConfig mSensorConfig;
        SensorPressInfo mPressInfo;
        uint32_t enrollicon_config;
        uint32_t authicon_config;
        char mALGO_VERSION[MAX_ALGO_VERSION_LEN];
        uint32_t mCaptureFailRetryCount;
        uint32_t mScreenState;
        gf_customized_auth_detail_get_t detail = { { 0 } };

    };
}  // namespace goodix

#endif /* _CUSTOMIZEDFINGERPRINTCORE_H_ */
