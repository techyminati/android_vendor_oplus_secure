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

namespace goodix
{
    class SZAlgo : public Algo
    {
    public:
        explicit SZAlgo(HalContext *context);
        virtual ~SZAlgo();
        virtual gf_error_t init();
        virtual void getSpmtPassOrNot(uint32_t * spmt_pass);
    protected:
        uint32_t spmt_pass_or_not;
    };
}

#endif /* _SZALGO_H_ */

