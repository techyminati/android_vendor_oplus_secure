/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HALCONTEXTEXT_H_
#define _HALCONTEXTEXT_H_

#include "HalContext.h"

namespace goodix {
    class HalContextExt: public HalContext {
    public:
        HalContextExt();
        ~HalContextExt();
        gf_error_t init();
        gf_error_t deinit();
    };
}  // namespace goodix

#endif /* _HALCONTEXTEXT_H_ */
