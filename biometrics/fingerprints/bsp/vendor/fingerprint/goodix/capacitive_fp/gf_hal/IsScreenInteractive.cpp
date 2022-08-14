/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <binder/IServiceManager.h>
#include "IPowerManager.h"
#include "IsScreenInteractive.h"

using namespace android;

/**
 * Function: gfIsScreenInteractive
 * Description: Get the status of screen.
 * Input: None
 * Output: None
 * Return: int32_t
 */
int32_t gfIsScreenInteractive()
{
    sp < IServiceManager > sm = defaultServiceManager();
    sp < IBinder > binder = sm->getService(String16(POWER_MANAGER));
    sp < IPowerManager > powermanager = interface_cast < IPowerManager > (binder);

    if (powermanager == NULL)
    {
        ALOG(LOG_VERBOSE, LOG_TAG, "can't get IPowerManager\n");
        return -1;
    }

    bool state = powermanager->isInteractive();
    ALOG(LOG_VERBOSE, LOG_TAG, "PowerManager get screen_state=%d\n", state);
    return state;
}
