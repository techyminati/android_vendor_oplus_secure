/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "ExtFido.h"
#include "HalLog.h"
#include "HalContext.h"
#include "FingerprintCore.h"

namespace goodix
{
    ExtFido::ExtFido(HalContext* context) :
            mpContext(context)
    {
    }

    int32_t ExtFido::authenticateFido(uint32_t groupId, uint8_t *aaid, uint32_t aaidLen,
            uint8_t *finalChallenge, uint32_t challengeLen)
    {
        return mpContext->mFingerprintCore->authenticateFido(groupId, aaid, aaidLen, finalChallenge,
                challengeLen);
    }

    int32_t ExtFido::stopAuthenticateFido()
    {
        return mpContext->mFingerprintCore->cancelFido();
    }

    int32_t ExtFido::isIdValid(uint32_t groupId, uint32_t fingerId)
    {
        return mpContext->mFingerprintCore->isIdValid(groupId, fingerId);
    }

    int32_t ExtFido::getIdList(uint32_t gid, uint32_t *list, int32_t *count)
    {
        return mpContext->mFingerprintCore->getIdList(gid, list, count);
    }

    int32_t ExtFido::invokeFidoCommand(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf,
            uint32_t *out_buf_len)
    {
        // TODO implement
        UNUSED_VAR(in_buf);
        UNUSED_VAR(in_buf_len);
        UNUSED_VAR(out_buf);
        UNUSED_VAR(out_buf_len);
        return 0;
    }

    void ExtFido::setNotify(fingerprint_notify_t notify)
    {
        mpContext->mFingerprintCore->setFidoNotify(notify);
    }
}  // namespace goodix
