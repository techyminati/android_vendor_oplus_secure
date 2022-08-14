/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _CUSTOMIZEDDELMARALGO_H_
#define _CUSTOMIZEDDELMARALGO_H_

#include "HalContext.h"
#include "DelmarAlgo.h"

namespace goodix {
    class CustomizedDelmarAlgo : public DelmarAlgo {
    public:
        explicit CustomizedDelmarAlgo(HalContext *context);
        virtual ~CustomizedDelmarAlgo();
        virtual bool isCheckFingerUp();
    protected:
        virtual const char* getDisableStudyProperty();
    };
}  // namespace goodix

#endif /* _CUSTOMIZEDDELMARALGO_H_ */
