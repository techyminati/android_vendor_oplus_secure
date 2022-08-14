/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][DelmarAlgo]"

#include "DelmarAlgo.h"
#include "HalLog.h"
#include "gf_error.h"
#include "HalUtils.h"
#include "HalContext.h"
#include "DelmarSensor.h"
#include "DelmarAlgoUtils.h"

namespace goodix {

    DelmarAlgo::DelmarAlgo(HalContext *context)
        : Algo(context),
        mAuthSensorIds(0),
        mEnrollSensorIds(0),
        mStudySensorIds(0),
        mIsFirstSensor(false),
        mAuthSuccessSensorId(0) {
    }

    DelmarAlgo::~DelmarAlgo() {
    }

    gf_error_t DelmarAlgo::init() {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        uint32_t caliState = sensor->getCaliState();
        FUNC_ENTER();
        do {
            gf_delmar_algo_init_t cmd = {{ 0 }};
            gf_cmd_header_t *header = (gf_cmd_header_t *) &cmd;
            header->target = GF_TARGET_ALGO;
            header->cmd_id = GF_CMD_ALGO_INIT;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            caliState |= cmd.o_cali_state;
            LOG_D(LOG_TAG, "[%s] o_cali_state:0x%08x, caliState:0x%08x", __func__,
                            cmd.o_cali_state, caliState);
            sensor->setCaliState(caliState);
        } while (0);
        // print hal version
        LOG_I(LOG_TAG, "[%s] HAL_VERSION=%s", __func__, GF_HAL_VERSION);
        DelmarAlgoUtils::printHalVersionInAlgoUtils();

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarAlgo::enrollImage(gf_algo_enroll_image_t *enroll) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            GF_NULL_BREAK(enroll, err);
#ifdef SUPPORT_DSP_HAL
            HalDsp *dsp = mContext->getDsp();
            err = DelmarAlgoUtils::enrollImageDsp(mContext, enroll, mIsFirstSensor ? 1 : 0, mEnrollSensorIds, dsp);
#else  // SUPPORT_DSP_HAL
            gf_delmar_algo_enroll_image_t *delmarEnrollCmd = (gf_delmar_algo_enroll_image_t*) enroll;
            enroll->header.target = GF_TARGET_ALGO;
            enroll->header.cmd_id = GF_CMD_ALGO_ENROLL;
            delmarEnrollCmd->i_enroll_sensor_ids = mEnrollSensorIds;
            delmarEnrollCmd->i_is_first_sensor = mIsFirstSensor ? 1 : 0;
            err = invokeCommand(delmarEnrollCmd, sizeof(gf_delmar_algo_enroll_image_t));
#endif  // SUPPORT_DSP_HAL
            GF_ERROR_BREAK(err);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarAlgo::authImage(gf_algo_auth_image_t *auth) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        gf_delmar_algo_auth_image_t* delmarAuthCmd = (gf_delmar_algo_auth_image_t*)auth;
        do {
            mAuthSuccessSensorId = 0;
            mStudySensorIds = 0;
            GF_NULL_BREAK(auth, err);
#ifdef SUPPORT_DSP_HAL
            HalDsp *dsp = mContext->getDsp();
            err = DelmarAlgoUtils::authImageDsp(mContext, auth, mIsFirstSensor ? 1 : 0, mAuthSensorIds, dsp, 1);
#else  // SUPPORT_DSP_HAL
            auth->header.target = GF_TARGET_ALGO;
            delmarAuthCmd->i_auth_sensor_ids = mAuthSensorIds;
            delmarAuthCmd->i_is_first_auth = mIsFirstSensor ? 1 : 0;
            delmarAuthCmd->i_temp_flag = HalUtils::detectTemperature();
            auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
            err = invokeCommand(delmarAuthCmd, sizeof(gf_delmar_algo_auth_image_t));
#endif  // SUPPORT_DSP_HAL
            mAuthSuccessSensorId = delmarAuthCmd->o_auth_success_sensor_id;
            LOG_D(LOG_TAG, "[%s] mAuthSuccessSensorId: 0x%x", __func__, mAuthSuccessSensorId);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    // update sensor ids used by study and dump after enroll/authenticate
    gf_error_t DelmarAlgo::updateSensorIds(gf_delmar_sensor_ids_t *info) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            GF_NULL_BREAK(info, err);
            info->header.target = GF_TARGET_ALGO;
            info->header.cmd_id = GF_DELMAR_CMD_UPDATE_SENSOR_IDS;
            mStudySensorIds = info->study_sensor_ids;
            err = invokeCommand(info, sizeof(gf_delmar_sensor_ids_t));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_algo_enroll_image_t* DelmarAlgo::createEnrollCmd() {
        gf_delmar_algo_enroll_image_t* cmd = new gf_delmar_algo_enroll_image_t();
        memset(cmd, 0, sizeof(gf_delmar_algo_enroll_image_t));
        cmd->common.o_samples_remaining = mContext->mConfig->enrolling_min_templates;
        return (gf_algo_enroll_image_t*)cmd;
    }

    void DelmarAlgo::destroyEnrollCmd(gf_algo_enroll_image_t* cmd) {
        if (cmd != nullptr) {
            delete (gf_delmar_algo_enroll_image_t*)cmd;
        }
    }

    gf_algo_auth_image_t* DelmarAlgo::createAuthCmd() {
        gf_delmar_algo_auth_image_t* cmd = new gf_delmar_algo_auth_image_t();
        memset(cmd, 0, sizeof(gf_delmar_algo_auth_image_t));
        return (gf_algo_auth_image_t*)cmd;
    }

    void DelmarAlgo::destroyAuthCmd(gf_algo_auth_image_t* cmd) {
        if (cmd != nullptr) {
            delete (gf_delmar_algo_auth_image_t*)cmd;
        }
    }
}  // namespace goodix
