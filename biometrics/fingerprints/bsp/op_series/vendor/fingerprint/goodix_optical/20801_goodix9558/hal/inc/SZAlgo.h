/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _SZALGO_H_
#define _SZALGO_H_

#include "Algo.h"
#include "gf_sz_types.h"

namespace goodix {
    class SZAlgo : public Algo {
    public:
        explicit SZAlgo(HalContext *context);
        virtual ~SZAlgo();
        virtual gf_error_t init();
        virtual void getSpmtPassOrNot(uint32_t *spmt_pass);
        virtual void setSpmtPassOrNot(uint32_t spmt_pass);
        virtual gf_error_t authImage(gf_algo_auth_image_t *auth);
        virtual void getFeatureTime(uint32_t* first_time, uint32_t* second_time);

        gf_algo_enroll_image_t* createEnrollCmd();
        gf_algo_auth_image_t* createAuthCmd();

        uint32_t mExpoTime;
        uint32_t mValidTime;
    protected:
        uint32_t spmt_pass_or_not;
        uint32_t first_get_feature_time;
        uint32_t second_get_feature_time;
    };
}

#endif /* _SZALGO_H_ */

