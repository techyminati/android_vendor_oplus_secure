/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _SZEXTCUSTOMIZED_H_
#define _SZEXTCUSTOMIZED_H_

#include "ExtCustomized.h"

namespace goodix
{

    class SZExtCustomized : public ExtCustomized
    {
    public:
        explicit SZExtCustomized(HalContext *context);
        virtual ~SZExtCustomized();
        virtual gf_error_t executeCommand(uint32_t cmdId, uint8_t *in, uint32_t inLen,
                                          uint8_t **out, uint32_t *outLen);
    };

}  // namespace goodix



#endif /* _SHENZHENEXTCUSTOMIZED_H_ */
