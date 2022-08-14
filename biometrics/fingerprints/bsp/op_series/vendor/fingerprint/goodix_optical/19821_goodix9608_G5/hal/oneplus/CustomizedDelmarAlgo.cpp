/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][CustomizedDelmarAlgo]"

#include "CustomizedDelmarAlgo.h"
#include "HalLog.h"
#include "gf_error.h"
#include "HalUtils.h"
#include "HalContext.h"
#include "DelmarAlgoUtils.h"
#include "DelmarAlgo.h"
#include "gf_customized_algo_types.h"



namespace goodix {

    CustomizedDelmarAlgo::CustomizedDelmarAlgo(HalContext *context)
        : DelmarAlgo(context) {
    }

    CustomizedDelmarAlgo::~CustomizedDelmarAlgo() {
    }

    gf_error_t CustomizedDelmarAlgo::customizedInitFakeParams() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            gf_delmar_enroll_time_flag_t cmd = {{0}};
            cmd.header.cmd_id = GF_CMD_ALGO_INIT_PARAMS;
            cmd.header.target = GF_TARGET_ALGO;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarAlgo::init() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            err = DelmarAlgo::init();
            GF_ERROR_BREAK(err);
            customizedInitFakeParams();
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarAlgo::enrollImage(gf_algo_enroll_image_t *enroll) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            GF_NULL_BREAK(enroll, err);
            gf_delmar_algo_enroll_image_t* delmarEnrollCmd =  (gf_delmar_algo_enroll_image_t*)enroll;
            enroll->header.target = GF_TARGET_ALGO;
            enroll->header.cmd_id = GF_CMD_ALGO_ENROLL;
            delmarEnrollCmd->i_temp = DelmarAlgoUtils::detectTemperature();
            err = invokeCommand(enroll, sizeof(gf_delmar_algo_enroll_image_t));
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix

