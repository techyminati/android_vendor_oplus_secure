/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][CustomizedDelmarAlgo]"

#include <cutils/properties.h>
#include "CustomizedDelmarAlgo.h"
#include "HalLog.h"
#include "gf_error.h"
#include "HalUtils.h"
#include "HalContext.h"
#include "DelmarAlgoUtils.h"
#include "DelmarAlgo.h"
#include "gf_customized_algo_types.h"
#include "DelmarSensor.h"


namespace goodix {

    CustomizedDelmarAlgo::CustomizedDelmarAlgo(HalContext *context)
        : DelmarAlgo(context) {
    }

    CustomizedDelmarAlgo::~CustomizedDelmarAlgo() {
    }
    void CustomizedDelmarAlgo::customizedSetCaliProperty() {
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        uint32_t caliState = sensor->getCaliState();
        if ((caliState & (1 << DELMAR_CALI_STATE_KB_READY_BIT)) &&
            (caliState & (1 << DELMAR_CALI_STATE_PGA_GAIN_READY_BIT))) {
            property_set("vendor.fingerprint.cali", "1");
        } else {
            LOG_E(LOG_TAG, "[%s] cali data not ready caliState=0x%08x", __func__, caliState);
            property_set("vendor.fingerprint.cali", "0");
        }
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
            customizedSetCaliProperty();
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    const char* CustomizedDelmarAlgo::getDisableStudyProperty() {
        return NULL;
    }
}  // namespace goodix

