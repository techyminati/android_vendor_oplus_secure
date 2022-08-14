/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][HalContextExt]"

#include "HalContextExt.h"
#include "HalLog.h"

namespace goodix {
    HalContextExt::HalContextExt() :
        HalContext() {
        VOID_FUNC_ENTER();
        VOID_FUNC_EXIT();
    }

    HalContextExt::~HalContextExt() {
    }

    gf_error_t HalContextExt::init() {
        FUNC_ENTER();
        gf_error_t err = HalContext::init();

        if (err == GF_SUCCESS) {
            // TODO
            // init some ext modules
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalContextExt::deinit() {
        FUNC_ENTER();
        gf_error_t err = HalContext::deinit();
        // TODO
        // deinit some ext modules
        FUNC_EXIT(err);
        return GF_SUCCESS;
    }
}  // namespace goodix

