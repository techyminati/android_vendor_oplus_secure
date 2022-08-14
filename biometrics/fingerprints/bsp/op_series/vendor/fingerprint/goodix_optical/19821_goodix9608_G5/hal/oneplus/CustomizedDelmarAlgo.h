/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
* Version: V1.0
* Description:
* History:
*/

#ifndef _CUSTOMIZEDDELMARALGO_H_
#define _CUSTOMIZEDDELMARALGO_H_


#include "DelmarAlgo.h"
#include "gf_delmar_types.h"

namespace goodix {
    class CustomizedDelmarAlgo : public DelmarAlgo {
    public:
        explicit CustomizedDelmarAlgo(HalContext *context);
        virtual ~CustomizedDelmarAlgo();
        virtual gf_error_t init();
        virtual gf_error_t enrollImage(gf_algo_enroll_image_t *enrll);
        gf_error_t customizedInitFakeParams();
    };
}  // namespace goodix
#endif  /* _CUSTOMIZEDDELMARALGO_H_ */

