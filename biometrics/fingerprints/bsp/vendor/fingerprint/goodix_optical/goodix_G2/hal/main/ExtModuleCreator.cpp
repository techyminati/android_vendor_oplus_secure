/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "SZExtCustomized.h"
#include "SZCustomizedProductTest.h"
#include "HalContext.h"
#include "ExtFido.h"

namespace goodix
{
    ProductTest* createProductTest(HalContext* context)
    {
        GF_ASSERT_NOT_NULL(context);
        ProductTest* test = nullptr;
        gf_chip_series_t series = context->mSensorInfo.chip_series;
        test = new SZCustomizedProductTest(context);
        context->mProductTest = test;
        return test;
    }

    ExtCustomized* createExtCustomized(HalContext* context)
    {
        GF_ASSERT_NOT_NULL(context);
        ExtCustomized* ext = nullptr;
        gf_chip_series_t series = context->mSensorInfo.chip_series;
        switch (series)
        {
            case GF_SHENZHEN:
            {
                ext = new SZExtCustomized(context);
                break;
            }
            default:
            {
                break;
            }
        }
        return ext;
    }

    ExtFido* createExtFido(HalContext* context)
    {
        return new ExtFido(context);
    }
}   // namespace goodix

