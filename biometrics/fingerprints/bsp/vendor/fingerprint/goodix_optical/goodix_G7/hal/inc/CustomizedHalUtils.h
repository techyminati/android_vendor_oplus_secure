/*
 * Copyright (C) 2013-2021, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CUSTOMIZEDHALUTILS_H_
#define _CUSTOMIZEDHALUTILS_H_

#include "gf_error.h"

namespace goodix {
    class CustomizedHalUtils {
    public:
        static gf_error_t customizedEncrypt(uint8_t* src, uint32_t srcLen,
                            uint8_t **encrypt, uint32_t *encryptLen,
                            char *filename, uint32_t operation);
        static void checkDumpFileLimit();
    };
}  // namespace goodix



#endif /* _CUSTOMIZEDHALUTILS_H_ */
