/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _REGISTERSERVICE_H_
#define _REGISTERSERVICE_H_

#include "gf_error.h"

namespace goodix {

    class HalContext;

    gf_error_t registerService(HalContext *context);

}

#endif /* _REGISTERSERVICE_H_ */
