/************************************************************************************
 ** File: - vendor\fingerprint\main\CustomizedHalConfig.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 10:58:11,16/08/2020
 ** Author: Chen.ran@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         2020/04/15       get the lock property
************************************************************************************/

#include <cutils/properties.h>
#include <string.h>
#include "HalLog.h"
#include "CustomizedHalConfig.h"
#define LOG_TAG "[GF_HAL][CustomizedHalConfig]"

namespace goodix {
    const char* getDumpRootDir(void) {
        return "/data/vendor/fingerprint";
    }

    const char* getDumpControlProperty(void) {
        return "gf.debug.dump_data";
    }

    bool checkScreenType(char *type) {
        bool ret = false;
        char value[PROPERTY_VALUE_MAX] = { 0 };
        int len = 0;
        do {
            if (NULL == type) {
                break;
            }
            len = property_get(PROPERTY_SCREEN_TYPE, value, SCREEN_TYPE_AD097_BOE);
            LOG_D(LOG_TAG, "[%s] in type=%s system type=%s.", __func__, type, value);
            if (!strncmp(type, value, strlen(type))) {
                ret = true;
            }
        } while (0);

        LOG_D(LOG_TAG, "[%s] ret=%d.", __func__, ret);
        return ret;
    }

    bool isTerminalUnlocked(void) {
        bool ret = false;
        char value[PROPERTY_VALUE_MAX] = { 0 };
        do {
            if(!property_get_bool("ro.boot.flash.locked", 1)){
                ret = true;
                LOG_D(LOG_TAG, "[%s] enter to unlock", __func__);
            } else {
                ret = false;
                LOG_D(LOG_TAG, "[%s] enter to locked", __func__);
            }
        } while (0);
        return ret;
    }

}  // namespace goodix
