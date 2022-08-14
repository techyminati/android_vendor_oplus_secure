/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "SZExtCustomized.h"
#include "ExtCustomized.h"

namespace goodix
{

    SZExtCustomized::SZExtCustomized(HalContext *context)
        : ExtCustomized(context)
    {
        // NOLINT(432)
    }

    SZExtCustomized::~SZExtCustomized()
    {
    }

    gf_error_t SZExtCustomized::executeCommand(uint32_t cmdId, uint8_t *in,
                                               uint32_t inLen, uint8_t **out, uint32_t *outLen)
    {
        UNUSED_VAR(cmdId);
        UNUSED_VAR(in);
        UNUSED_VAR(inLen);
        UNUSED_VAR(out);
        UNUSED_VAR(outLen);
        return GF_SUCCESS;
    }

}  // namespace goodix
