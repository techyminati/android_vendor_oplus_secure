/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][ShenzhenAlgo]"

#include <stdlib.h>
#include <string.h>
#include <cutils/fs.h>

#include "HalContext.h"
#include "SZAlgo.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "FingerprintCore.h"
#include "fake_params.h"

namespace goodix
{

    SZAlgo::SZAlgo(HalContext *context) : Algo(context)
    {
        spmt_pass_or_not = 1;
    }

    SZAlgo::~SZAlgo()
    {
    }

    gf_error_t SZAlgo::init()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {

            LOG_E(LOG_TAG, "[%s]", "v03.02.02.53");
            gf_sz_algo_init_t cmd;
            memset(&cmd, 0, sizeof(gf_sz_algo_init_t));
            gf_cmd_header_t *header = (gf_cmd_header_t *)&cmd;
            header->target = GF_TARGET_ALGO;
            header->cmd_id = GF_CMD_ALGO_INIT;
            /*spmt pass is 1*/
            cmd.spmt_pass = 1;
            err = invokeCommand(&cmd, sizeof(cmd));
            spmt_pass_or_not = cmd.spmt_pass;
            LOG_I(LOG_TAG, "[%s] dmsg cal verison %s", __func__, cmd.factory_algo_version);
            GF_ERROR_BREAK(err);
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    void SZAlgo::getSpmtPassOrNot(uint32_t * spmt_pass)
    {
        VOID_FUNC_ENTER();
        *spmt_pass = spmt_pass_or_not;
        VOID_FUNC_EXIT();
    }
}  // namespace goodix
