/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _DELMARFINGERPRINTCORE_H_
#define _DELMARFINGERPRINTCORE_H_

#include "FingerprintCore.h"
#include "gf_delmar_types.h"

namespace goodix {
    class DelmarSensor;
    class DelmarAlgo;

    class DelmarFingerprintCore : public FingerprintCore {
    public:
        explicit DelmarFingerprintCore(HalContext *context);
        virtual ~DelmarFingerprintCore();
    protected:
        virtual gf_error_t prepareEnrollRequest();
        virtual gf_error_t prepareAuthRequest();
        virtual gf_error_t onEnrollStart(EnrollContext *context);
        virtual gf_error_t onEnrollStop(EnrollContext *context);
        virtual gf_error_t onAfterEnrollAlgo(EnrollContext *context);
        virtual gf_error_t onAuthStart(AuthenticateContext *context);
        virtual gf_error_t onAuthStop(AuthenticateContext *context);
        virtual gf_error_t onAfterEnrollCapture(EnrollContext *context);
        virtual gf_error_t onBeforeAuthCapture(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthCapture(AuthenticateContext *context);
        virtual bool needRetryIfCaptureFailed(AuthenticateContext * context);
        virtual gf_error_t onBeforeAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthAlgo(AuthenticateContext *context);
        virtual void onAuthError(AuthenticateContext *context);
        virtual void onError(gf_error_t err);
        virtual bool needRetry(AuthenticateContext *context);
        virtual gf_error_t notifyAuthSuccess(AuthenticateContext *context);
        virtual void notifyAuthNotMatched();
        virtual gf_error_t notifyDismatchInfo(gf_error_t info);
        virtual gf_error_t dumpKPI(const char *func_name);
        virtual gf_error_t checkEnrollAuthReady();
        virtual gf_error_t onAfterAuthAlgo4MultiFinger(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthSuccess(AuthenticateContext *context);
        uint8_t mAuthSuccessNotified;
    private:
        gf_error_t clearSensorIds();
        bool needRetryByDismatchReason(gf_algo_auth_image_t *auth);
        DelmarSensor *getSensor(void);
        DelmarAlgo *getAlgo(void);
        uint64_t calcStudySensorIds(uint64_t originalSensorIds);
        gf_error_t switchFingerCapture4MultiFinger(AuthenticateContext *context);
        gf_error_t auth4MultiFinger(AuthenticateContext *context, uint8_t isFirstFinger,
               gf_algo_auth_image_t *studyCmd, uint64_t *studySensorIds, uint64_t *dumpSensorIds);
        gf_error_t study4MultiFinger(AuthenticateContext *context, gf_algo_auth_image_t *studyCmd,
               uint64_t studySensorIds);
        gf_error_t doImageReadAndEnroll(EnrollContext *context, uint64_t sensorIds, bool *abort);
        gf_error_t doSimplyEnroll(EnrollContext *context);
        gf_error_t splitPostAuthFlow(AuthenticateContext *context);
        bool needRetryByResult(HalContext *context, gf_error_t result, gf_delmar_algo_auth_image_t* cmd);
        uint8_t mAuthFailNotified;
        uint32_t mValidEnrollPressCount;
        bool mEnrollTemplatesFull;
        uint32_t mCaptureFailRetryCount;
        gf_error_t mRty0Err;
    };
}

#endif /* _DELMARFINGERPRINTCORE_H_ */
