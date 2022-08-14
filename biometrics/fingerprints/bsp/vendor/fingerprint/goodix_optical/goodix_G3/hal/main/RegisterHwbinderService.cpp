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
#include "ProductTest.h"

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
            ProductTest* ext = createProductTest(context);
            if (NULL == ext)
            {
                LOG_D(LOG_TAG, "create ProductTest failed");
                break;
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
