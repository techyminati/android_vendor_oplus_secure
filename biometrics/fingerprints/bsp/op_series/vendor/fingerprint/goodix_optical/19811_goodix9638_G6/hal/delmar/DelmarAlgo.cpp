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
#include "DelmarHalUtils.h"

namespace goodix {

    DelmarAlgo::DelmarAlgo(HalContext *context)
        : Algo(context),
        mAuthSensorIds(0),
        mEnrollSensorIds(0),
        mStudySensorIds(0),
        mDumpSensorIds(0),
        mIsFirstSensor(false),
        mAuthSuccessSensorId(0) {
    }

    DelmarAlgo::~DelmarAlgo() {
    }

    gf_error_t DelmarAlgo::init() {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        uint32_t caliState = sensor->getCaliState();
        gf_delmar_algo_init_t* cmd = NULL;
        int32_t size = 0;
        FUNC_ENTER();
        do {
            err = createInitCmd(&cmd, &size);
            if (err != GF_SUCCESS) {
                break;
            }
            gf_cmd_header_t *header = (gf_cmd_header_t *) cmd;
            header->target = GF_TARGET_ALGO;
            header->cmd_id = GF_CMD_ALGO_INIT;
#ifdef SUPPORT_DSP_HAL
            cmd->i_dsp_enabled = mContext->isDSPEnabled();
#endif  // SUPPORT_DSP_HAL
            err = invokeCommand(cmd, size);
            GF_ERROR_BREAK(err);
            caliState |= cmd->o_cali_state;
            LOG_D(LOG_TAG, "[%s] o_cali_state:0x%08x, caliState:0x%08x", __func__,
                            cmd->o_cali_state, caliState);
            sensor->setCaliState(caliState);
            handleInitResult(cmd);
        } while (0);
        // print hal version
        LOG_I(LOG_TAG, "[%s] HAL_VERSION=%s", __func__, GF_HAL_VERSION);
        DelmarHalUtils::registerModuleVersion("delmar", GF_HAL_VERSION);
        DelmarAlgoUtils::printHalVersionInAlgoUtils();

        if (cmd != NULL) {
            free(cmd);
            cmd = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    static inline uint64_t mergeSensorIds(uint64_t first, uint64_t second) {
        uint64_t tmp = second;
        uint32_t i = 0;
        while ((first >> i) > 0) {
            tmp <<= 8;
            i += 8;
        }
        return first | tmp;
    }

    gf_error_t DelmarAlgo::enrollImage(gf_algo_enroll_image_t *enroll) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        int64_t enrollStart = HalUtils::getCurrentTimeMicrosecond();
        do {
            GF_NULL_BREAK(enroll, err);
            gf_delmar_algo_enroll_image_t *delmarEnrollCmd = (gf_delmar_algo_enroll_image_t*) enroll;
            enroll->header.target = GF_TARGET_ALGO;
            enroll->header.cmd_id = GF_CMD_ALGO_ENROLL;
            delmarEnrollCmd->i_enroll_sensor_ids = mEnrollSensorIds;
            delmarEnrollCmd->i_is_first_sensor = mIsFirstSensor ? 1 : 0;
            mDumpSensorIds = mergeSensorIds(mDumpSensorIds, mEnrollSensorIds);
            err = invokeCommand(delmarEnrollCmd, sizeof(gf_delmar_algo_enroll_image_t));
            GF_ERROR_BREAK(err);
        } while (0);
        DelmarHalUtils::setKpiTotalTime((int32_t)((HalUtils::getCurrentTimeMicrosecond() - enrollStart) / 1000.0));
        FUNC_EXIT(err);
        return err;
    }


    bool DelmarAlgo::isCheckFingerUp() {
        return false;
    }

    gf_error_t DelmarAlgo::normalAuthImage(gf_algo_auth_image_t *auth) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_algo_auth_image_t* delmarAuthCmd = (gf_delmar_algo_auth_image_t*)auth;
        FUNC_ENTER();

        do {
            GF_NULL_BREAK(auth, err);
            auth->header.target = GF_TARGET_ALGO;
            delmarAuthCmd->i_auth_sensor_ids = mAuthSensorIds;
            delmarAuthCmd->i_is_first_sensor = mIsFirstSensor ? 1 : 0;
            delmarAuthCmd->i_temp_flag = mContext->mSensor->detectTemperature();
            auth->header.cmd_id = GF_CMD_ALGO_AUTHENTICATE;
            err = invokeCommand(delmarAuthCmd, sizeof(gf_delmar_algo_auth_image_t));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarAlgo::authImage(gf_algo_auth_image_t *auth) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_config_t *config = (gf_delmar_config_t *) mContext->mConfig;
        UNUSED_VAR(config);
        int64_t authStart = HalUtils::getCurrentTimeMicrosecond();
        FUNC_ENTER();
        gf_delmar_algo_auth_image_t* delmarAuthCmd = (gf_delmar_algo_auth_image_t*)auth;
        do {
            mAuthSuccessSensorId = 0;
            GF_NULL_BREAK(auth, err);

#ifdef SUPPORT_DSP_HAL
            HalDsp *dsp = mContext->getDsp();
            err = DelmarAlgoUtils::authImageDsp(mContext, auth, mIsFirstSensor ? 1 : 0, mAuthSensorIds, dsp, isCheckFingerUp());
#else  // SUPPORT_DSP_HAL
            err = normalAuthImage(auth);
#endif  // SUPPORT_DSP_HAL
        } while (0);
        mAuthSuccessSensorId = delmarAuthCmd->o_auth_success_sensor_id;
        LOG_D(LOG_TAG, "[%s] mAuthSuccessSensorId: 0x%x", __func__, mAuthSuccessSensorId);

        DelmarHalUtils::setKpiTotalTime((int32_t)((HalUtils::getCurrentTimeMicrosecond() - authStart) / 1000.0));
        FUNC_EXIT(err);
        return err;
    }

    // update sensor ids used by study and dump after enroll/authenticate
    gf_error_t DelmarAlgo::updateSensorIds(gf_delmar_sensor_ids_t *info) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            GF_NULL_BREAK(info, err);
            mStudySensorIds = info->study_sensor_ids;
            mDumpSensorIds = info->valid_sensor_ids;
            info->header.target = GF_TARGET_ALGO;
            info->header.cmd_id = GF_DELMAR_CMD_UPDATE_SENSOR_IDS;
            err = invokeCommand(info, sizeof(gf_delmar_sensor_ids_t));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_algo_enroll_image_t* DelmarAlgo::createEnrollCmd() {
        gf_delmar_algo_enroll_image_t* cmd = new gf_delmar_algo_enroll_image_t();
        memset(cmd, 0, sizeof(gf_delmar_algo_enroll_image_t));
        cmd->common.o_samples_remaining = mContext->mConfig->enrolling_min_templates;
        if (isSupportEnrollFakeCheck() > 0) {
            cmd->i_temp_flag = mContext->mSensor->detectTemperature();
        }
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
        // reset raw data
        gf_error_t err = GF_SUCCESS;
        gf_base_cmd_t reset_data_cmd = {0};
        reset_data_cmd.target = GF_TARGET_ALGO;
        reset_data_cmd.cmd_id = GF_DELMAR_CMD_ALGO_RESET_DATA;
        err = invokeCommand(&reset_data_cmd, sizeof(gf_base_cmd_t));

        if (cmd != nullptr) {
            delete (gf_delmar_algo_auth_image_t*)cmd;
        }
    }

    gf_error_t DelmarAlgo::createInitCmd(gf_delmar_algo_init_t** cmd, int32_t* size) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (cmd == NULL || size == NULL) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            *cmd = (gf_delmar_algo_init_t*)malloc(sizeof(gf_delmar_algo_init_t));
            if (*cmd != NULL) {
                *size = sizeof(gf_delmar_algo_init_t);
                memset(*cmd, 0, *size);
            } else {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
    /*this interface is to be implemented by derived classes*/
    gf_error_t DelmarAlgo::handleInitResult(gf_delmar_algo_init_t* cmd) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(cmd);
        FUNC_EXIT(err);
        return err;
    }

    uint8_t DelmarAlgo::isSupportEnrollFakeCheck() {
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        return (sensor->getSenosrType() == DELMAR_SENSOR_TYPE_SINGLE_T_SE5
                    && sensor->getOpticalType() == DELMAR_OPTICAL_TYPE_3_0
                    && ((gf_delmar_config_t *) mContext->mConfig)->support_enroll_fake_check > 0) ? 1 : 0;
    }
}  // namespace goodix
