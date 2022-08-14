/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _ALGO_H_
#define _ALGO_H_

#include "gf_error.h"
#include "HalBase.h"
#include "gf_algo_types.h"

namespace goodix {
    class Algo : public HalBase {
    public:
        explicit Algo(HalContext *context);
        virtual ~Algo();
        virtual gf_error_t init();
        virtual gf_error_t enrollImage(gf_algo_enroll_image_t *enrll);
        virtual gf_error_t authImage(gf_algo_auth_image_t *auth);
        virtual gf_algo_enroll_image_t* createEnrollCmd();
        virtual void destroyEnrollCmd(gf_algo_enroll_image_t* cmd);
        virtual gf_algo_auth_image_t* createAuthCmd();
        virtual void destroyAuthCmd(gf_algo_auth_image_t* cmd);
    };
}  // namespace goodix
#endif  /* _ALGO_H_ */
