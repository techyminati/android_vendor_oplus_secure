/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _PERSIST_DATA_H_
#define _PERSIST_DATA_H_

#include "gf_error.h"

namespace goodix {

    class PersistData {
    public:
        PersistData();
        ~PersistData();
        gf_error_t getPersistData(uint8_t **data, uint32_t *datalen);
        gf_error_t savePersistData(uint8_t *data, uint32_t datalen);
    };
}  // namespace goodix

#endif  // #ifndef _PERSIST_DATA_H_
