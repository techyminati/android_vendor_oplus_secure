/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "HalContext.h"
#include "HalDump.h"
#include "SZHalDump.h"

namespace goodix
{
    HalDump* createHalDump(HalContext* context)
    {
        if (nullptr == context)
        {
            return nullptr;
        }
        HalDump* dump = nullptr;
        gf_chip_series_t series = context->mSensorInfo.chip_series;
        switch (series)
        {
            case GF_SHENZHEN:
            {
                dump = new SZHalDump(context);
                break;
            }
            case GF_UNKNOWN_SERIES:
            {
                break;
            }
            default:
            {
                break;
            }
        }
        return dump;
    }
}   // namespace goodix
