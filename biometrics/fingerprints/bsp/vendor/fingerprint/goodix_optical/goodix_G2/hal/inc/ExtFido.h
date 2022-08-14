/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _EXTFIDO_H_
#define _EXTFIDO_H_

#include "fingerprint_oplus.h"

namespace goodix
{
    class HalContext;

    class ExtFido
    {
    public:
        explicit ExtFido(HalContext* context);
        int32_t authenticateFido(uint32_t groupId, uint8_t *aaid, uint32_t aaidLen,
                uint8_t *finalChallenge, uint32_t challengeLen);
        int32_t stopAuthenticateFido();
        int32_t isIdValid(uint32_t groupId, uint32_t fingerId);
        int32_t getIdList(uint32_t groupId, uint32_t *list, int32_t *count);
        int32_t invokeFidoCommand(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf,
                uint32_t *out_buf_len);
        void setNotify(fingerprint_notify_t notify);

    private:
        HalContext* mpContext;
    };

    // The interface is implemented by project
    ExtFido* createExtFido(HalContext* context);
}  // namespace goodix

#endif /* _EXTFIDO_H_ */
