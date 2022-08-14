/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CUSTOMIZEDFINGERPRINTCORE_H_
#define _CUSTOMIZEDFINGERPRINTCORE_H_

#include "SZFingerprintCore.h"

namespace goodix {
    class CustomizedFingerprintCore: public SZFingerprintCore {
    public:
        explicit CustomizedFingerprintCore(HalContext *context);
        virtual ~CustomizedFingerprintCore();
        gf_error_t enrollPause();
        gf_error_t enrollResume();
        int updateStatus(uint32_t status);
        uint64_t mUpTime;

    protected:
        virtual gf_error_t init();
        virtual gf_error_t onEnrollRequested(const void *hat, uint32_t gid, uint32_t timeoutSec);
        virtual gf_error_t onEnrollUpEvt();
        virtual gf_error_t onEnrollStart(EnrollContext *context);
        virtual gf_error_t onBeforeEnrollCapture(EnrollContext *context);
        virtual gf_error_t onAfterEnrollCapture(EnrollContext *context);
        virtual gf_error_t onBeforeEnrollAlgo(EnrollContext *context);
        virtual gf_error_t onAfterEnrollAlgo(EnrollContext *context);
        virtual gf_error_t onEnrollStop(EnrollContext *context);
        virtual void onEnrollError(EnrollContext *context);
        virtual gf_error_t notifyEnrollProgress(EnrollContext *context);
        virtual gf_error_t prepareAuthRequest();
        virtual gf_error_t onAuthRequested(uint64_t operationId, uint32_t gid);
        virtual gf_error_t onAuthUpEvt();
        virtual gf_error_t onAuthStart(AuthenticateContext *context);
        virtual gf_error_t onBeforeAuthCapture(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthCapture(AuthenticateContext *context);
        virtual gf_error_t onBeforeAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthRetry(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthSuccess(AuthenticateContext *context);
        virtual gf_error_t onAuthStop(AuthenticateContext *context);
        virtual void onAuthError(AuthenticateContext *context);
        virtual gf_error_t notifyAuthSuccess(AuthenticateContext *context);
        virtual void notifyAuthNotMatched();
        virtual gf_error_t onResetEvent();
        // notify message
        virtual gf_error_t notifyAcquiredInfo(gf_fingerprint_acquired_info_t info);
        virtual gf_error_t notifyErrorInfo(gf_fingerprint_error_t err);
        virtual void doCancel();
        gf_error_t getTemp(const char *path, int32_t *temp);
        void setLight(uint32_t light);
        gf_error_t notifyMDM(AuthenticateContext *context);
        uint64_t mDownTime;
        bool mFastUp;
        uint32_t mLight = 300;
    private:
        uint32_t mTotalTime = 0;
        uint32_t mMDMTime = 0;
    };
}  // namespace goodix
#endif /* _CustomizedFingerprintCore_H_ */
