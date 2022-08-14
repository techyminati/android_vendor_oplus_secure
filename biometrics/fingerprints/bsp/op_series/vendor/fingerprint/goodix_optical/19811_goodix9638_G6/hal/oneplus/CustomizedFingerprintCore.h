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

namespace goodix {
    typedef enum CUSTOMIZED_FINGERPRINT_ACQUIRE_INFO {
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

    class CustomizedFingerprintCore : public DelmarFingerprintCore {
    public:
        explicit CustomizedFingerprintCore(HalContext *context);
        virtual ~CustomizedFingerprintCore();
        gf_error_t init();

        gf_error_t notifyAuthUpEvt();
        bool customizedIsAuthDownDetected();
        int32_t updateStatus(uint32_t status);
        gf_error_t setFingerState(uint32_t state);
        gf_error_t notifyTouch(gf_fingerprint_msg_type_t type);


    protected:
        virtual gf_error_t notifyAcquiredInfo(gf_fingerprint_acquired_info_t info);
        virtual gf_error_t notifyEnrollProgress(EnrollContext *context);
        virtual gf_error_t prepareEnrollRequest();
        virtual gf_error_t prepareAuthRequest();
        gf_error_t onAuthStop(AuthenticateContext *context);
        virtual gf_error_t dumpKPI(const char *func_name);
        gf_error_t onEnrollRequested(const void *hat, uint32_t gid, uint32_t timeoutSec);
        gf_error_t onEnrollStart(EnrollContext *context);
        gf_error_t onBeforeEnrollCapture(EnrollContext *context);
        gf_error_t onAfterEnrollCapture(EnrollContext *context);
        gf_error_t onAfterEnrollAlgo(EnrollContext *context);
        gf_error_t onAuthRequested(uint64_t operationId, uint32_t gid);
        gf_error_t onAuthStart(AuthenticateContext *context);
        gf_error_t onBeforeAuthCapture(AuthenticateContext *context);
        gf_error_t notifyAuthSuccess(AuthenticateContext *context);
        gf_error_t onAfterAuthRetry(AuthenticateContext *context);
        gf_error_t onAfterAuthSuccess(AuthenticateContext *context);
        gf_error_t onAuthUpEvt();
        void onError(gf_error_t err);
        gf_error_t cancel(bool notifyCancelMsg);
        gf_error_t enrollPause();
        gf_error_t enrollResume();
        gf_error_t authenticatePause();
        gf_error_t authenticateResume();
        int32_t setFingerReady();
        void onAuthError(AuthenticateContext *context);
        gf_error_t notifyMDM(AuthenticateContext *context);
        virtual gf_error_t send_auth_dcsmsg(AuthenticateContext* context, bool notified);
        gf_delmar_algo_kpi_t kpi = {{0}};

    protected:
        void onEnrollError(EnrollContext *context);

    private:
        uint64_t time_start = 0;
        uint32_t mStatus = 0;
        Mutex mFpUpLock;
        uint32_t mTotalTime = 0;
        uint32_t mMDMTime = 0;
        gf_error_t getAlgoVersion();
        char mALGO_VERSION[MAX_ALGO_VERSION_LEN];
    };
}  // namespace goodix

#endif /* _CUSTOMIZEDFINGERPRINTCORE_H_ */
