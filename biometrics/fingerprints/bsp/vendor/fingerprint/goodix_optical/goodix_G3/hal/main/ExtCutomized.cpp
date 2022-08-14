/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][HalExtension]"

#include "ExtCustomized.h"

namespace goodix
{

    ExtCustomized::ExtCustomized(HalContext* context)
        : HalBase(context),
          mNotify(nullptr)
    {
    }

    ExtCustomized::~ExtCustomized()
    {
    }

    void ExtCustomized::setNotify(fingerprint_notify_t notify)
    {
        mNotify = notify;
    }

    gf_error_t ExtCustomized::executeCommand(uint32_t cmdId, uint8_t *in, uint32_t inLen, uint8_t **out, uint32_t *outLen)
    {
        UNUSED_VAR(cmdId);
        UNUSED_VAR(in);
        UNUSED_VAR(inLen);
        UNUSED_VAR(out);
        UNUSED_VAR(outLen);
        return GF_SUCCESS;
    }

    gf_error_t ExtCustomized::onEvent(gf_event_type_t e)
    {
        UNUSED_VAR(e);
        return GF_SUCCESS;
    }

}  // namespace goodix
