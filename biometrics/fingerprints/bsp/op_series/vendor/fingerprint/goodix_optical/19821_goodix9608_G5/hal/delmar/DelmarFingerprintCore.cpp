/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][DelmarFingerprintCore]"

#include <stdlib.h>
#include <endian.h>
#include "HalContext.h"
#include "DelmarFingerprintCore.h"
#include "HalLog.h"
#include "EventCenter.h"
#include "FingerprintCore.h"
#include "HalUtils.h"
#include "Sensor.h"
#include "Algo.h"
#include "gf_delmar_types.h"
#include "DelmarSensor.h"
#include "DelmarAlgo.h"
#include "DelmarHalUtils.h"

namespace goodix {

    DelmarFingerprintCore::DelmarFingerprintCore(HalContext *context)
        : FingerprintCore(context),
        mAuthSuccessNotified(0) {
    }

    DelmarFingerprintCore::~DelmarFingerprintCore() {
    }

    // clear sensor ids info used by dump
    gf_error_t DelmarFingerprintCore::clearSensorIds() {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_sensor_ids_t cmd = { {0} };
        FUNC_ENTER();
        do {
            DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
            cmd.study_sensor_ids = 0;
            cmd.valid_sensor_ids = 0;
            err = algo->updateSensorIds(&cmd);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    DelmarSensor *DelmarFingerprintCore::getSensor(void) {
        return (DelmarSensor *) mContext->mSensor;
    }

    DelmarAlgo *DelmarFingerprintCore::getAlgo(void) {
        return (DelmarAlgo *) mContext->mAlgo;
    }

    gf_error_t DelmarFingerprintCore::checkEnrollAuthReady() {
        uint32_t caliState = getSensor()->getCaliState();
        if ((caliState & (1 << DELMAR_CALI_STATE_KB_READY_BIT)) &&
            (caliState & (1 << DELMAR_CALI_STATE_PGA_GAIN_READY_BIT))) {
            return GF_SUCCESS;
        } else {
            LOG_E(LOG_TAG, "[%s] cali data not ready caliState=0x%08x",
                            __func__, caliState);
            return GF_ERROR_CALIBRATION_NOT_READY;
        }
    }

    gf_error_t DelmarFingerprintCore::prepareEnrollRequest() {
        return checkEnrollAuthReady();
    }

    gf_error_t DelmarFingerprintCore::prepareAuthRequest() {
        return checkEnrollAuthReady();
    }

    gf_error_t DelmarFingerprintCore::onEnrollStart(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            err = clearSensorIds();
            GF_ERROR_BREAK(err);
            err = mContext->mSensor->wakeupSensor();
            GF_ERROR_BREAK(err);
            err = FingerprintCore::onEnrollStart(context);
            GF_ERROR_BREAK(err);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onEnrollStop(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        err = FingerprintCore::onEnrollStop(context);
        mContext->mSensor->sleepSensor();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterEnrollAlgo(EnrollContext *context) {
        gf_error_t err = context->result;
        gf_delmar_config_t *delmarConfig = (gf_delmar_config_t *) mContext->mConfig;
        uint8_t enrollRecaptureCount = 0;
        FUNC_ENTER();

        do {
            GF_ERROR_BREAK(err);
            if (context->enroll_cmd->o_samples_remaining == 0) {
                LOG_D(LOG_TAG, "[%s] enroll finish, no need to recapture!", __func__);
                break;
            }
            if (delmarConfig->enroll_recapture_count <= 0) {
                LOG_D(LOG_TAG, "[%s] do nothing if enroll recapture count <=0", __func__);
                break;
            }
            notifyEnrollProgress(context);
            sendMessage(MsgBus::MSG_ENROLL_CONTINUE_CAPTURE, &(context->result), sizeof(gf_error_t));
            err = clearSensorIds();
            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] fail to clear sensor ids!", __func__);
                break;
            }
            err = mContext->mSensor->captureImage(GF_OPERATION_ENROLL, enrollRecaptureCount + 1);
            context->result = err;
            onAfterEnrollCapture(context);

            if (context->result != GF_SUCCESS) {
                onEnrollError(context);
                break;
            }

            enrollRecaptureCount++;
            LOG_D(LOG_TAG, "[%s] enrollRecaptureCount = %d", __func__, enrollRecaptureCount);
            onBeforeEnrollAlgo(context);
            mContext->mAlgo->enrollImage(context->enroll_cmd);
        } while (context->enroll_cmd->o_samples_remaining > 0 && enrollRecaptureCount < delmarConfig->enroll_recapture_count);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAuthStart(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            mAuthSuccessNotified = 0;
            err = mContext->mSensor->wakeupSensor();
            GF_ERROR_BREAK(err);
            err = FingerprintCore::onAuthStart(context);
            GF_ERROR_BREAK(err);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAuthStop(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_auth_end_t cmd = { { 0 } };
        FUNC_ENTER();

        do {
            err = FingerprintCore::onAuthStop(context);
            GF_ERROR_BREAK(err);
            mContext->mSensor->sleepSensor();
            if (context->result != GF_ERROR_TOO_FAST
                    && context->result != GF_ERROR_UI_READY_TIMEOUT) {
                cmd.header.target = GF_TARGET_ALGO;
                cmd.header.cmd_id = GF_DELMAR_CMD_ALGO_AUTHENTICATE_END;
                cmd.finger_id = context->auth_cmd->o_finger_id;
                err = invokeCommand(&cmd, sizeof(gf_delmar_auth_end_t));
            }
            if (context->result != GF_SUCCESS) {
                LOG_D(LOG_TAG, "[%s] auth fail total time %d ms, result=%d", __func__,
                    (int32_t)((HalUtils::getCurrentTimeMicrosecond() - context->fingerDownTime)/1000),
                    context->result);
            }
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterEnrollCapture(EnrollContext *context) {
        UNUSED_VAR(context);
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = getSensor();
        uint64_t sensorIds = sensor->getSensorIds();
        uint64_t sensorId = 0;
        uint32_t sensorNum = sensor->getAvailableSensorNum();
        FUNC_ENTER();

        do {
            if (context->result != GF_SUCCESS) {
                err = context->result;
                break;
            }
            for (uint32_t i = 0; i < sensorNum; i++) {
                sensorId = sensorIds & 0xFF;
                if (sensorId > 0) {
                    err = sensor->readImage(0, sensorId);
                    GF_ERROR_BREAK(err);
                } else {
                    break;
                }
                sensorIds = sensorIds >> 8;
            }
        } while (0);

        context->result = err;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onBeforeAuthCapture(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        do {
            err = clearSensorIds();
            if (err != GF_SUCCESS) {
                context->result = err;
                break;
            }
            err = FingerprintCore::onBeforeAuthCapture(context);
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterAuthCapture(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = getSensor();
        FUNC_ENTER();
        do {
            // read image of the first sensor
            uint64_t sensorId = 0;
            sensorId = sensor->getSensorIds() & 0xFF;
            err = sensor->readImage(context->retry, sensorId);
            context->result = err;
            GF_ERROR_BREAK(err);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onBeforeAuthAlgo(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
        uint64_t sensorIds = sensor->getSensorIds();
        FUNC_ENTER();

        algo->setFirstAuth(1);
        algo->setAuthSensorIds(sensorIds & 0xFF);

        UNUSED_VAR(context);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterAuthAlgo(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
        gf_delmar_sensor_ids_t cmd = { {0} };
        FUNC_ENTER();

        do {
            uint64_t originalSensorIds = sensor->getSensorIds();
            cmd.study_sensor_ids = originalSensorIds;
            cmd.valid_sensor_ids = originalSensorIds;

            algo->setFirstAuth(0);
            if (context->result == GF_SUCCESS) {
                if (mContext->mConfig->support_performance_dump) {
                    mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - context->fingerDownTime;
                }
                notifyAuthSuccess(context);
                mAuthSuccessNotified = 1;
            }
            if (GF_DELMAR_FAKE_ERROR(context->result)) {
                cmd.study_sensor_ids = 0;
                cmd.valid_sensor_ids &= 0xFF;
                LOG_E(LOG_TAG, "[%s] Fake is detected %d.", __func__, context->result);
                break;
            }
            // remove the first sensor id
            uint64_t sensorIds = originalSensorIds >> 8;
            if (sensorIds > 0) {
                err = sensor->readImage(context->retry, sensorIds);
                // read second sensor image err
                if (err != GF_SUCCESS) {
                    if (context->result != GF_SUCCESS) {
                        LOG_E(LOG_TAG, "[%s] read second iamge err=%d", __func__, err);
                        // context->result = err; // only notify first auth err
                        // first sensor auth fail, use for dump etc.
                        cmd.study_sensor_ids = 0;
                        cmd.valid_sensor_ids &= 0xFF;
                    } else {
                        // first sensor auth success, use for study.
                        cmd.study_sensor_ids &= 0xFF;
                        cmd.valid_sensor_ids &= 0xFF;
                    }
                    break;
                } else if (context->result != GF_SUCCESS) {
                    if (context->auth_cmd->i_retry_count == 0) {
                        LOG_D(LOG_TAG, "[%s] shift to an earlier time to capture image for retry", __func__);
                        sensor->switchRetrySensorIds();
                        err = sensor->captureImage(GF_OPERATION_AUTHENTICATE, context->auth_cmd->i_retry_count + 1);
                        GF_ERROR_BREAK(err);
                    }
                    algo->setAuthSensorIds(sensorIds);
                    err = algo->authImage(context->auth_cmd);
                    // the error of first sensor has high priority if error happended, but not if success
                    if (err == GF_SUCCESS) {
                        context->result = err;
                    }
                    cmd.study_sensor_ids = calcStudySensorIds(originalSensorIds);
                }
            }
        } while (0);

        LOG_D(LOG_TAG, "[%s] valid_sensor_ids: %08x%08x", __func__,
                    SPLIT_UINT64_TO_UINT32_ARGS(cmd.valid_sensor_ids));
        LOG_D(LOG_TAG, "[%s] study_sensor_ids: %08x%08x", __func__,
                    SPLIT_UINT64_TO_UINT32_ARGS(cmd.study_sensor_ids));

        err = algo->updateSensorIds(&cmd);
        FUNC_EXIT(err);
        return err;
    }

    uint64_t DelmarFingerprintCore::calcStudySensorIds(uint64_t originalSensorIds) {
        if (originalSensorIds == 0) {
            return 0;
        }
        uint64_t studySensorIds = originalSensorIds;
        uint8_t authSuccessSensorId = getAlgo()->getAuthSuccessSensorId();
        if (authSuccessSensorId == 0) {
            return 0;
        }

        while (studySensorIds > 0) {
            if ((studySensorIds & 0xFF) == studySensorIds) {
                return studySensorIds;
            }
            studySensorIds >>= 8;
        }
        return 0;
    }

    void DelmarFingerprintCore::onAuthError(AuthenticateContext *context) {
        gf_algo_auth_image_t *auth = context->auth_cmd;
        if (auth != nullptr && (context->result == GF_ERROR_ACQUIRED_PARTIAL
                || context->result == GF_ERROR_ACQUIRED_IMAGER_DIRTY)) {
            notifyDismatchInfo(context->result);
        } else if (context->result == GF_ERROR_ACQUIRED_IMAGER_DRY) {
            notifyAuthNotMatched();
        } else if (context->result == GF_ERROR_NOT_LIVE_FINGER) {
            // TODO: dump kpi performance too if fake finger
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_NOT_LIVE_FINGER);
        } else {
            FingerprintCore::onAuthError(context);
        }
    }

    void DelmarFingerprintCore::onError(gf_error_t err) {
        if (err == GF_ERROR_UI_READY_TIMEOUT) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
        } else if (err == GF_ERROR_ANOMALY_FINGER) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
        } else {
            FingerprintCore::onError(err);
        }
    }

    bool DelmarFingerprintCore::needRetry(AuthenticateContext *context) {
        bool needRetry = false;
        int32_t maxRetry = mContext->mConfig->max_authenticate_failed_attempts;
        VOID_FUNC_ENTER();

        if (context->retry < maxRetry && !checkFingerLeave()) {
            needRetry = context->result == GF_ERROR_NOT_MATCH
                || context->result == GF_ERROR_ACQUIRED_IMAGER_DRY
                || context->result == GF_ERROR_ACQUIRED_PARTIAL
                || context->result == GF_ERROR_ACQUIRED_IMAGER_DIRTY;
        }

        if (needRetry && mContext->mConfig->support_performance_dump) {
            dumpKPI(__func__);
        }

        VOID_FUNC_EXIT();
        return needRetry;
    }

    gf_error_t DelmarFingerprintCore::notifyAuthSuccess(AuthenticateContext *context) {
        if (mAuthSuccessNotified == 1) {
            return GF_SUCCESS;
        }
        return FingerprintCore::notifyAuthSuccess(context);
    }

    gf_error_t DelmarFingerprintCore::notifyDismatchInfo(gf_error_t info) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] dismatch due to error= %d", __func__, info);

        if (info == GF_ERROR_ACQUIRED_PARTIAL) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_PARTIAL);
        } else if (info == GF_ERROR_ACQUIRED_IMAGER_DIRTY) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
        }

        if (mContext->mConfig->support_performance_dump) {
            dumpKPI(__func__);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::dumpKPI(const char *func_name) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (nullptr == func_name) {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] func_name is nullptr", __func__);
                break;
            }

            // get kpi
            gf_delmar_algo_kpi_t kpi = { { 0 } };
            kpi.header.target = GF_TARGET_ALGO;
            kpi.header.cmd_id = GF_CMD_ALGO_KPI;
            gf_error_t err = invokeCommand(&kpi, sizeof(gf_delmar_algo_kpi_t));

            if (GF_SUCCESS != err) {
                break;
            }

            err = DelmarHalUtils::printKpiPerformance(getWorkState(),
                    mTotalKpiTime, &kpi.o_performance, func_name);
            if (err != GF_SUCCESS) {
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
