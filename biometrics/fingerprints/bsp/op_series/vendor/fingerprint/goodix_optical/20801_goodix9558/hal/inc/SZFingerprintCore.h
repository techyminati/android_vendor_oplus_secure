/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _SZFINGERPRINTCORE_H_
#define _SZFINGERPRINTCORE_H_

#include "FingerprintCore.h"

namespace goodix {
    class SZFingerprintCore: public FingerprintCore {
    public:
        explicit SZFingerprintCore(HalContext *context);
        virtual ~SZFingerprintCore();
        virtual gf_error_t setEnvironmentLevel(uint32_t save_level);
    protected:
        virtual void forceStudy(void);
        virtual gf_error_t init();
        virtual gf_error_t prepareEnrollRequest();
        virtual gf_error_t onEnrollRequested(const void *hat, uint32_t gid,
                                             uint32_t timeoutSec);
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
        virtual gf_error_t notifyRemove(uint32_t gid, uint32_t fid,
                                        uint32_t remainingTemplates);
        virtual gf_error_t notifyEnumerate(gf_enumerate_t *result);
        virtual void doCancel();
        virtual gf_error_t dumpKPI(const char *func_name);
    };
}

#endif /* _SZFINGERPRINTCORE_H_ */

