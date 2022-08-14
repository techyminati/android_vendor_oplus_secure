/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _HEARTBEATDEFINE_H_
#define _HEARTBEATDEFINE_H_

#define HEARTBEAT_RATE_CMD_START 400000
#define HEARTBEAT_RATE_CMD_END   499999

namespace goodix {
    typedef enum {
        HEARTBEAT_RATE_CMD_MAX = HEARTBEAT_RATE_CMD_START,
    }
    HEARTBEAT_RATE_CMD_ID;

    enum {
        HEARTBEAT_RATE_TOKEN_ERROR_CODE = 400000,
        HEARTBEAT_RATE_TOKEN_MAX,
    };
};  // namespace goodix


#endif /* _HEARTBEATDEFINE_H_  */
