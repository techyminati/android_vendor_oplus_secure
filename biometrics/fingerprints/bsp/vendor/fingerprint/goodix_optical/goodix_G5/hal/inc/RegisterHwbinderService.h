/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _REGISTERHWBINDERSERVICE_H_
#define _REGISTERHWBINDERSERVICE_H_

#include "gf_error.h"

namespace goodix
{
    class HalContext;

    gf_error_t registerHwbinderService(HalContext* context);
}


#endif /* _REGISTERHWBINDERSERVICE_H_ */
