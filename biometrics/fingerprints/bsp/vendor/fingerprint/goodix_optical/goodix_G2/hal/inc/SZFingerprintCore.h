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
#include "gf_sz_types.h"
#include "gf_algo_types.h"


namespace goodix
{
    class SZFingerprintCore : public FingerprintCore
    {
    public:
        explicit SZFingerprintCore(HalContext *context);
        virtual ~SZFingerprintCore();
    protected:
        virtual gf_error_t init();
        virtual gf_error_t onAuthStart(AuthenticateContext *context);
        virtual gf_error_t onAuthStop(AuthenticateContext *context);
        virtual void forceStudy(void);
        virtual gf_error_t onEnrollStart(EnrollContext *context);
        virtual gf_error_t onEnrollStop(EnrollContext *context);
        virtual gf_error_t onAfterAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthRetry(AuthenticateContext *context);
        virtual gf_error_t setEnvironmentLevel(uint32_t save_level);
        virtual gf_error_t prepareAuthRequest();
        virtual gf_error_t prepareEnrollRequest();
    };
}

#endif /* _SZFINGERPRINTCORE_H_ */

