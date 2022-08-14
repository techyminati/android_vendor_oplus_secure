/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CAENTRY_H_
#define _CAENTRY_H_

#include "gf_error.h"
#include "gf_base_types.h"
#include "gf_sensor_types.h"

namespace goodix {
    class CaEntry {
    public:
        CaEntry();
        ~CaEntry();
        gf_error_t startTap();
        gf_error_t shutdownTap();
        gf_error_t sendCommand(void *cmd, uint32_t size);
        void getCarveoutFdInfo(int32_t *fd, int32_t *len);
        // Note: The method can't access CaEntry Object data, or crash wil happend.Now only ree use the method.
        void setDeviceHandle(int32_t fd);
        void *mContext;
    };
}  // namespace goodix

#endif /* _CAENTRY_H_ */
