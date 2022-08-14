/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][registerHwBinderService]"

#include "HalLog.h"
#include "RegisterHwbinderService.h"
#include "HalContext.h"
#include "GoodixFingerprintDaemon.h"

using ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::IGoodixFingerprintDaemon;
using ::vendor::goodix::hardware::biometrics::fingerprint::V2_1::implementation::GoodixFingerprintDaemon;

namespace goodix
{
    static gf_error_t registerDaemonService(HalContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do
        {
            LOG_D(LOG_TAG, "register GoodixFingerprintDaemon hwbinder service");
            android::status_t ret = android::OK;
            android::sp <GoodixFingerprintDaemon > service = GoodixFingerprintDaemon::getInstance(context);
            if (service != nullptr)
            {
                ret = service->registerAsService();
                if (ret != android::OK)
                {
                    LOG_E(LOG_TAG, "Couldn't register GoodixFingerprintDaemon binder service!");
                    err = GF_ERROR_GENERIC;
                }
            }
            else
            {
                LOG_E(LOG_TAG, "Failed to register service %s", __func__);
                err = GF_ERROR_GENERIC;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t registerHwbinderService(HalContext* context)
    {
        LOG_D(LOG_TAG, "register GoodixFingerprintDaemon hwbinder service");
        gf_error_t err = GF_SUCCESS;
        err = registerDaemonService(context);
        // TODO: register fido service?
        return err;
    }

}
