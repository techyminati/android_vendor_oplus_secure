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
#include <cutils/properties.h>
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

#define MAX_CAPTURE_FAIL_RETRY 2

namespace goodix {

    DelmarFingerprintCore::DelmarFingerprintCore(HalContext *context)
        : FingerprintCore(context),
        mAuthSuccessNotified(0),
        mAuthFailNotified(0),
        mValidEnrollPressCount(0),
        mEnrollTemplatesFull(false),
        mCaptureFailRetryCount(0),
        mRty0Err(GF_SUCCESS) {
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
        gf_error_t err = GF_SUCCESS;
        uint32_t caliState = getSensor()->getCaliState();
        FUNC_ENTER();
        if ((caliState & (1 << DELMAR_CALI_STATE_KB_READY_BIT)) &&
            (caliState & (1 << DELMAR_CALI_STATE_PGA_GAIN_READY_BIT))) {
            err = GF_SUCCESS;
        } else {
            LOG_E(LOG_TAG, "[%s] cali data not ready caliState=0x%08x",
                            __func__, caliState);
            err = getSensor()->getCaliLoadError();
            if (err == GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] get Cali load err fail.", __func__);
                err = GF_ERROR_INVALID_STATE;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::prepareEnrollRequest() {
        mValidEnrollPressCount = 0;
        mEnrollTemplatesFull = false;
        DelmarHalUtils::checkModuleVerions();
        mRty0Err = GF_SUCCESS;
        return checkEnrollAuthReady();
    }

    gf_error_t DelmarFingerprintCore::prepareAuthRequest() {
        DelmarHalUtils::checkModuleVerions();
        mRty0Err = GF_SUCCESS;
        return checkEnrollAuthReady();
    }

    gf_error_t DelmarFingerprintCore::onEnrollStart(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        FUNC_ENTER();
        do {
            err = clearSensorIds();
            GF_ERROR_BREAK(err);
            err = mContext->mSensor->wakeupSensor();
            GF_ERROR_BREAK(err);
            err = FingerprintCore::onEnrollStart(context);
            GF_ERROR_BREAK(err);
            sensor->setValidEnrollPressCount(mValidEnrollPressCount + 1);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
    gf_error_t DelmarFingerprintCore::onAfterEnrollNotify(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_algo_enroll_image_t* delmar_cmd = (gf_delmar_algo_enroll_image_t*)context->enroll_cmd;
        FUNC_ENTER();
        do {
            LOG_D(LOG_TAG, "[%s] current io_enr_stage:%d", __func__, delmar_cmd->io_enr_stage);
            if (delmar_cmd->io_enr_stage == 0) {
                break;
            }
            mContext->mAlgo->enrollImage(context->enroll_cmd);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onEnrollStop(EnrollContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
        FUNC_ENTER();
        err = FingerprintCore::onEnrollStop(context);
        mContext->mSensor->sleepSensor();
        algo->enrollEnd(context->enroll_cmd->o_gid, context->enroll_cmd->o_finger_id, context->enroll_cmd->o_samples_remaining);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterEnrollCapture(EnrollContext *context) {
        UNUSED_VAR(context);
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = getSensor();
        DelmarAlgo *algo = getAlgo();
        uint64_t sensorIds = sensor->getSensorIds();
        FUNC_ENTER();

        do {
            if (context->result != GF_SUCCESS) {
                err = context->result;
                break;
            }
            uint64_t sensorId = sensorIds & 0xFF;
            if (sensorId > 0) {
                err = sensor->readImage(0, sensorId);
            } else {
                err = GF_ERROR_BAD_PARAMS;
            }

            if (err == GF_SUCCESS) {
                mValidEnrollPressCount++;
                algo->setEnrollSensorIds(sensorId);
                algo->setFirstSensor(true);
            }
        } while (0);

        context->result = err;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::doImageReadAndEnroll(EnrollContext *context,
            uint64_t sensorIds, bool *hasException) {
        gf_error_t err = GF_SUCCESS;
        gf_error_t mainSensorError = err;
        gf_delmar_config_t *delmarConfig = (gf_delmar_config_t *) mContext->mConfig;
        DelmarSensor *sensor = getSensor();
        DelmarAlgo *algo = getAlgo();
        gf_delmar_algo_enroll_image_t *cmd = (gf_delmar_algo_enroll_image_t*)context->enroll_cmd;
        bool successAtLeastOneTime = false;

        FUNC_ENTER();

        *hasException = false;

        while (sensorIds > 0) {
            uint64_t sensorId = sensorIds & 0xFF;
            if (sensorId == 0) {
                err = GF_ERROR_BAD_PARAMS;
                *hasException = true;
                break;
            }

            err = sensor->readImage(0, sensorId);
            if (err != GF_SUCCESS) {
                *hasException = true;
                break;
            }

            algo->setEnrollSensorIds(sensorId);
            onBeforeEnrollAlgo(context);
            err = mContext->mAlgo->enrollImage(context->enroll_cmd);
            if (err == GF_SUCCESS) {
                successAtLeastOneTime = true;
                if (cmd->o_is_templates_full > 0) {
                    mEnrollTemplatesFull = true;
                }
            } else if (mainSensorError == GF_SUCCESS) {
                mainSensorError = err;
            }

            if (context->enroll_cmd->o_samples_remaining == 0
                    && delmarConfig->support_continue_enroll_captured_image <= 0) {
                break;
            }
            algo->setFirstSensor(false);
            sensorIds >>= 8;
        }

        if (*hasException == false) {
            if (successAtLeastOneTime) {
                err = GF_SUCCESS;
            } else {
                err = mainSensorError;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::doSimplyEnroll(EnrollContext *context) {
        gf_error_t err = context->result;
        DelmarSensor *sensor = getSensor();
        DelmarAlgo *algo = getAlgo();
        gf_delmar_algo_enroll_image_t *cmd = (gf_delmar_algo_enroll_image_t*)context->enroll_cmd;
        uint64_t sensorIds = 0;
        bool hasException = false;
        FUNC_ENTER();
        do {
            sensorIds = sensor->getSensorIds();
            if (sensorIds == 0) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            sendMessage(MsgBus::MSG_ENROLL_CONTINUE_CAPTURE, &(context->result), sizeof(gf_error_t));
            err = clearSensorIds();
            if (GF_SUCCESS != err) {
                break;
            }
            cmd->i_retry_count = 1;
            err = mContext->mSensor->captureImage(GF_OPERATION_ENROLL, 1);
            if (err != GF_SUCCESS) {
                break;
            }

            algo->setFirstSensor(1);
            err = doImageReadAndEnroll(context, sensorIds, &hasException);
        } while (0);

        context->result = err;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterEnrollAlgo(EnrollContext *context) {
        gf_error_t err = context->result;
        gf_error_t mainSensorError = err;
        bool firstSensorIncluding = false;
        DelmarSensor *sensor = getSensor();
        DelmarAlgo *algo = getAlgo();
        gf_delmar_algo_enroll_image_t *cmd = (gf_delmar_algo_enroll_image_t*)context->enroll_cmd;
        uint64_t sensorIds = 0;
        gf_delmar_config_t *delmarConfig = (gf_delmar_config_t *) mContext->mConfig;
        uint8_t enrollRecaptureCount = 0;
        bool successAtLeastOneTime = false;
        bool hasException = false;

        FUNC_ENTER();
        do {
            if (err == GF_ERROR_HIGH_LIGHT) {
                return doSimplyEnroll(context);
            }

            if (err == GF_ERROR_DUPLICATE_FINGER) {
                hasException = true;
                break;
            } else if (err == GF_SUCCESS) {
                successAtLeastOneTime = true;
            }

            if (mValidEnrollPressCount <= 1) {
                mEnrollTemplatesFull = false;
            } else if (cmd->o_is_templates_full > 0) {
                mEnrollTemplatesFull = true;
            }
            if (!firstSensorIncluding) {
                // enroll sensors except first sensor
                sensorIds = sensor->getSensorIds() >> 8;
                if (sensorIds == 0 || (context->enroll_cmd->o_samples_remaining == 0
                        && (delmarConfig->support_continue_enroll_captured_image <= 0 &&
                            delmarConfig->support_enroll_by_press_threshold <= 0))) {
                    break;
                }
            } else {
                // recapture image
                LOG_D(LOG_TAG, "[%s] start to recapture and enroll, enrollRecaptureCount = %d",
                        __func__, enrollRecaptureCount);
                sensorIds = sensor->getSensorIds();
                if (sensorIds == 0) {
                    err = GF_ERROR_BAD_PARAMS;
                    break;
                }
                notifyEnrollProgress(context);
                sendMessage(MsgBus::MSG_ENROLL_CONTINUE_CAPTURE, &(context->result), sizeof(gf_error_t));
                err = clearSensorIds();
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "[%s] fail to clear sensor ids!", __func__);
                    hasException = true;
                    break;
                }
                err = mContext->mSensor->captureImage(GF_OPERATION_ENROLL, enrollRecaptureCount);
                if (err != GF_SUCCESS) {
                    hasException = true;
                    break;
                }
            }

            algo->setFirstSensor(firstSensorIncluding);
            err = doImageReadAndEnroll(context, sensorIds, &hasException);
            if (err == GF_SUCCESS) {
                successAtLeastOneTime = true;
            } else if (mainSensorError == GF_SUCCESS) {
                mainSensorError = err;
            }
            if (hasException) {
                break;
            }
            firstSensorIncluding = true;
        }
        while (context->enroll_cmd->o_samples_remaining > 0
                && ++enrollRecaptureCount <= delmarConfig->enroll_recapture_count);

        // if templates are full, finish enroll
        if (mEnrollTemplatesFull) {
            gf_delmar_finish_enroll_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_ALGO;
            cmd.header.cmd_id = GF_DELMAR_CMD_ALGO_FINISH_ENROLL;
            cmd.i_gid = context->enroll_cmd->o_gid;
            cmd.i_finger_id = context->enroll_cmd->o_finger_id;
            cmd.i_samples_remaining = context->enroll_cmd->o_samples_remaining;
            err = invokeCommand(&cmd, sizeof(cmd));
            if (err == GF_SUCCESS) {
                context->enroll_cmd->o_samples_remaining = 0;
            }
        }

        if (successAtLeastOneTime) {
            err = GF_SUCCESS;
        } else if (mainSensorError != GF_SUCCESS) {
            err = mainSensorError;
        }
        context->result = err;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAuthStart(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            mCaptureFailRetryCount = 0;
            mAuthSuccessNotified = 0;
            mAuthFailNotified = 0;
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
            if ((context->result != GF_ERROR_TOO_FAST || context->retry > 0)
                    && context->result != GF_ERROR_UI_READY_TIMEOUT) {
                cmd.header.target = GF_TARGET_ALGO;
                cmd.header.cmd_id = GF_DELMAR_CMD_ALGO_AUTHENTICATE_END;
                cmd.finger_id = context->auth_cmd->o_finger_id;
                err = invokeCommand(&cmd, sizeof(gf_delmar_auth_end_t));
            }
            if (context->result != GF_SUCCESS) {
                LOG_D(LOG_TAG, "[%s] auth fail total time %d ms", __func__,
                    (int32_t)((HalUtils::getCurrentTimeMicrosecond() - context->fingerDownTime)/1000));
            }
            LOG_D(LOG_TAG, "[%s] retry_count=%d, result=%d", __func__, context->retry, context->result);
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onBeforeAuthCapture(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_config_t *config = (gf_delmar_config_t *) mContext->mConfig;
        gf_delmar_algo_auth_image_t* cmd = (gf_delmar_algo_auth_image_t *) context->auth_cmd;

        FUNC_ENTER();
        do {
            if (config->support_independent_fast_auth > 0 && cmd->o_force_retry > 0
                    && context->retry == 1) {
                context->retry++;
            }
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
        gf_delmar_config_t *config = (gf_delmar_config_t *) mContext->mConfig;
        FUNC_ENTER();
        do {
            if (config->support_independent_fast_auth > 0 && context->retry == 1) {
                LOG_D(LOG_TAG, "[%s] ignore read image retry=%d", __func__, context->retry);
                break;
            }
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

        algo->setFirstSensor(true);
        algo->setAuthSensorIds(sensorIds & 0xFF);

        UNUSED_VAR(context);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::switchFingerCapture4MultiFinger(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;

        FUNC_ENTER();
        do {
            sensor->switchFingerCoordinate(1);
            sensor->updateSensorPriority();
            err = FingerprintCore::onBeforeAuthCapture(context);
            GF_ERROR_BREAK(err);
            err = sensor->captureImage(GF_OPERATION_AUTHENTICATE, 0);
            GF_ERROR_BREAK(err);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::auth4MultiFinger(AuthenticateContext *context, uint8_t isFirstFinger,
                   gf_algo_auth_image_t *studyCmd, uint64_t *studySensorIds, uint64_t *dumpSensorIds) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
        uint64_t originalSensorIds = 0;
        uint64_t secondSensorIds = 0;

        FUNC_ENTER();
        do {
            originalSensorIds = sensor->getSensorIds();
            if (context->result == GF_SUCCESS) {
                if (mContext->mConfig->support_performance_dump) {
                    mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - context->fingerDownTime;
                }

                notifyAuthSuccess(context);
                *dumpSensorIds = originalSensorIds & 0xFF;
                *studySensorIds  = originalSensorIds & 0xFF;
                memcpy(studyCmd, context->auth_cmd, sizeof(gf_algo_auth_image_t));
            } else {
                algo->setFirstSensor(false);
                secondSensorIds = originalSensorIds >> 8;
                if (secondSensorIds > 0) {
                    err = sensor->readImage(context->retry, secondSensorIds);
                    if (err != GF_SUCCESS) {
                        break;
                    } else {
                        // send capture for 2nd finger early
                        if (isFirstFinger) {
                            err = switchFingerCapture4MultiFinger(context);
                            GF_ERROR_BREAK(err);
                        }
                        algo->setAuthSensorIds(secondSensorIds);
                        err = algo->authImage(context->auth_cmd);
                    }

                    *dumpSensorIds = originalSensorIds;
                    context->result = err;
                    if (err == GF_SUCCESS) {
                        if (mContext->mConfig->support_performance_dump) {
                            mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - context->fingerDownTime;
                        }
                        notifyAuthSuccess(context);
                        *studySensorIds = (originalSensorIds >> 8) & 0xFF;
                        memcpy(studyCmd, context->auth_cmd, sizeof(gf_algo_auth_image_t));
                    }
                    err = GF_SUCCESS;
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::study4MultiFinger(AuthenticateContext *context,
                   gf_algo_auth_image_t *studyCmd, uint64_t studySensorIds) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_sensor_ids_t cmd = { {0} };
        DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
        FUNC_ENTER();

        do {
            cmd.study_sensor_ids = studySensorIds;
            cmd.valid_sensor_ids = 0;
            cmd.reauthenticate_for_study = 1;
            err = algo->updateSensorIds(&cmd);
            GF_ERROR_BREAK(err);
            memcpy(context->auth_cmd, studyCmd, sizeof(gf_algo_auth_image_t));
            LOG_D(LOG_TAG, "[%s] study finger_id=%d, studySensorIds=%08x%08x", __func__,
                           context->auth_cmd->o_finger_id, SPLIT_UINT64_TO_UINT32_ARGS(studySensorIds));
            onAfterAuthSuccess(context);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterAuthAlgo4MultiFinger(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
        gf_delmar_sensor_ids_t cmd = { {0} };
        gf_algo_auth_image_t firstFingerAuthCmd4study = { {0} };
        uint64_t firstStudySensorIds = 0;
        gf_algo_auth_image_t secondFingerAuthCmd4study = { {0} };
        uint64_t secondStudySensorIds = 0;

        uint64_t firstFingerSensorIds4dump = 0;
        uint64_t secondFingerSensorIds4dump = 0;
        FUNC_ENTER();

        do {
            err = auth4MultiFinger(context, 1, &firstFingerAuthCmd4study,
                                   &firstStudySensorIds, &firstFingerSensorIds4dump);
            GF_ERROR_BREAK(err);

            // switch finger and auth
            // if 2nd sensor of 1st finger not send to auth, send captrue for 2nd finger
            if ((firstFingerSensorIds4dump >> 8) == 0) {
                err = switchFingerCapture4MultiFinger(context);
                GF_ERROR_BREAK(err);
            }

            onAfterAuthCapture(context);
            GF_ERROR_BREAK(context->result);
            onBeforeAuthAlgo(context);
            context->auth_cmd->i_retry_count = context->retry;
            err = algo->authImage(context->auth_cmd);
            context->result = err;

            err = auth4MultiFinger(context, 0, &secondFingerAuthCmd4study,
                                   &secondStudySensorIds, &secondFingerSensorIds4dump);
            GF_ERROR_BREAK(err);

            if (mContext->mConfig->support_performance_dump) {
                mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - context->fingerDownTime;
                LOG_D(LOG_TAG, "[%s] mTotalKpiTime=%ums", __func__, (uint32_t)(mTotalKpiTime / 1000));
            }
            if (firstStudySensorIds == 0 && secondStudySensorIds == 0) {
                FingerprintCore::notifyAuthNotMatched();
                mAuthFailNotified = 1;
            } else {
                context->result = GF_SUCCESS;
                mAuthSuccessNotified = 1;
            }
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_DETECTED);

            // do study
            if (firstStudySensorIds != 0) {
                err = study4MultiFinger(context, &firstFingerAuthCmd4study, firstStudySensorIds);
                GF_ERROR_BREAK(err);
            }

            if (secondStudySensorIds != 0) {
                err = study4MultiFinger(context, &secondFingerAuthCmd4study, secondStudySensorIds);
                GF_ERROR_BREAK(err);
            }
        } while (0);

        cmd.study_sensor_ids = 0;
        if ((firstFingerSensorIds4dump >> 8 & 0xFF) != 0) {
            cmd.valid_sensor_ids = (secondFingerSensorIds4dump << 16) | (firstFingerSensorIds4dump & 0xFFFF);
        } else {
            cmd.valid_sensor_ids = (secondFingerSensorIds4dump << 8) | (firstFingerSensorIds4dump & 0xFF);
        }
        LOG_D(LOG_TAG, "[%s] valid_sensor_ids: %08x%08x", __func__,
                    SPLIT_UINT64_TO_UINT32_ARGS(cmd.valid_sensor_ids));
        err = algo->updateSensorIds(&cmd);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterAuthAlgo(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        DelmarAlgo *algo = (DelmarAlgo *) mContext->mAlgo;
        gf_delmar_sensor_ids_t cmd = { {0} };
        gf_delmar_config_t *config = (gf_delmar_config_t *) mContext->mConfig;
        gf_delmar_algo_auth_image_t* auth_cmd = (gf_delmar_algo_auth_image_t *) context->auth_cmd;
        FUNC_ENTER();
#ifdef SUPPORT_MULTIPLE_FINGER_AUTH
        if (sensor->getMultiFingerAuth()) {
            err = onAfterAuthAlgo4MultiFinger(context);
            FUNC_EXIT(err);
            return err;
        }
#endif  //  SUPPORT_MULTIPLE_FINGER_AUTH
        do {
            uint64_t originalSensorIds = sensor->getSensorIds();
            cmd.study_sensor_ids = originalSensorIds;
            cmd.valid_sensor_ids = originalSensorIds;

            algo->setFirstSensor(false);
            LOG_D(LOG_TAG, "[%s] force_retry =%d, retry=%d", __func__, auth_cmd->o_force_retry, context->retry);
            if ((context->result == GF_SUCCESS && auth_cmd->o_force_retry == 0)
                    || context->result == GF_ERROR_HIGH_LIGHT) {
                if (mContext->mConfig->support_performance_dump) {
                    mTotalKpiTime = HalUtils::getCurrentTimeMicrosecond() - context->fingerDownTime;
                    LOG_D(LOG_TAG, "[%s] mTotalKpiTime=%ums", __func__, (uint32_t)(mTotalKpiTime / 1000));
                }
                // GF_ERROR_HIGH_LIGHT means that force retry is finished
                if (context->result == GF_ERROR_HIGH_LIGHT) {
                    context->result = GF_SUCCESS;
                    notifyAuthSuccess(context);
                    context->result = GF_ERROR_HIGH_LIGHT;
                } else {
                    notifyAuthSuccess(context);
                }
                mAuthSuccessNotified = 1;
            } else if (GF_DELMAR_FAKE_ERROR(context->result)) {
                cmd.study_sensor_ids = 0;
                cmd.valid_sensor_ids &= 0xFF;
                LOG_E(LOG_TAG, "[%s] Fake is detected %d.", __func__, context->result);
                break;
            }

            // study retry 0 first if force retry is required
            if (auth_cmd->o_force_retry > 0) {
                cmd.study_sensor_ids = 0x01;
                err = algo->updateSensorIds(&cmd);
                GF_ERROR_BREAK(err);
                err = splitPostAuthFlow(context);
                GF_ERROR_BREAK(err);
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
            if (config->support_independent_fast_auth > 0 && context->auth_cmd->i_retry_count == 0 &&
                    config->common.max_authenticate_failed_attempts > 0 &&
                (needRetryByResult(mContext, context->result, (gf_delmar_algo_auth_image_t *)context->auth_cmd) ||
                DelmarHalUtils::independentFastAuthNeedRetry(mContext, context->result, context->auth_cmd->i_retry_count))) {
                cmd.study_sensor_ids = 0;
                cmd.valid_sensor_ids = 0;
                LOG_D(LOG_TAG, "[%s] ignore dump when only fast auth.", __func__);
            }
        } while (0);

        if (algo->isCheckFingerUp() && context->result == GF_ERROR_TOO_FAST) {
            cmd.study_sensor_ids = 0;
            cmd.valid_sensor_ids = 0;
            LOG_D(LOG_TAG, "[%s] clean sensor_ids by GF_ERROR_TOO_FAST.", __func__);
        }
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
        gf_delmar_config_t *delmarConfig = (gf_delmar_config_t *) mContext->mConfig;
        uint8_t authSupportLongExpo = delmarConfig->support_long_exposure & 0x01;

        if (context->result == GF_ERROR_ACQUIRED_PARTIAL
                || context->result == GF_ERROR_ACQUIRED_IMAGER_DIRTY) {
            notifyDismatchInfo(context->result);
        } else if (context->result == GF_ERROR_ACQUIRED_IMAGER_DRY) {
            notifyAuthNotMatched();
        } else if (context->result == GF_ERROR_NOT_LIVE_FINGER) {
            LOG_D(LOG_TAG, "[%s] retry = %d", __func__, context->retry);
            LOG_D(LOG_TAG, "[%s] mRty0Err = %s", __func__, gf_strerror(mRty0Err));
            if (context->retry > 0 && mRty0Err != GF_ERROR_NOT_LIVE_FINGER && authSupportLongExpo == 0) {
                notifyDismatchInfo(mRty0Err);
            } else {
                notifyDismatchInfo(context->result);
            }
        } else {
            FingerprintCore::onAuthError(context);
        }
    }

    void DelmarFingerprintCore::onError(gf_error_t err) {
        if (err == GF_ERROR_UI_READY_TIMEOUT) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
        } else if (err == GF_ERROR_ANOMALY_FINGER) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
        } else if (err == GF_ERROR_NEED_CANCLE_ENROLL) {
            notifyErrorInfo(GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
        } else {
            if (err == GF_ERROR_BRIGHTNESS_NOT_STABILITY) {
                err = GF_ERROR_NOT_MATCH;
            }
            FingerprintCore::onError(err);
        }
    }

    bool DelmarFingerprintCore::needRetryByResult(HalContext *context, gf_error_t result,
                                 gf_delmar_algo_auth_image_t* cmd) {
        gf_delmar_config_t *delmarConfig = (gf_delmar_config_t *) context->mConfig;
        uint8_t authSupportLongExpo = delmarConfig->support_long_exposure & 0x01;
        bool isOptical7 = ((DelmarSensor*)context->mSensor)->getOpticalType() == DELMAR_OPTICAL_TYPE_7_0;
        bool needRetry = (result == GF_ERROR_NOT_MATCH
            || result == GF_ERROR_ACQUIRED_IMAGER_DRY
            || result == GF_ERROR_ACQUIRED_PARTIAL
            || result == GF_ERROR_ACQUIRED_IMAGER_DIRTY
            || result == GF_ERROR_BRIGHTNESS_NOT_STABILITY
            || (result == GF_ERROR_NOT_LIVE_FINGER && !isOptical7 && authSupportLongExpo == 0
                && (cmd->o_is_highlight == 0 || delmarConfig->decrease_gain_retry_if_highlight == 0)));

        LOG_D(LOG_TAG, "[%s] is_highlight=%d.  authSupportLongExpo=%d", __func__, cmd->o_is_highlight, authSupportLongExpo);
        return needRetry;
    }

    bool DelmarFingerprintCore::needRetry(AuthenticateContext *context) {
        bool needRetry = false;
        int32_t maxRetry = mContext->mConfig->max_authenticate_failed_attempts;
        gf_delmar_config_t *delmarConfig = (gf_delmar_config_t *) mContext->mConfig;
        gf_delmar_algo_auth_image_t* cmd = (gf_delmar_algo_auth_image_t *) context->auth_cmd;
        mCaptureFailRetryCount = 0;
        VOID_FUNC_ENTER();
        do {
#ifdef SUPPORT_MULTIPLE_FINGER_AUTH
            DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
            if (sensor->getMultiFingerAuth()) {
                needRetry = false;
                break;
            }
#endif  // SUPPORT_MULTIPLE_FINGER_AUTH
            if (checkFingerLeave()) {
                LOG_E(LOG_TAG, "[%s] finger leave too fast.", __func__);
                needRetry = false;
                break;
            }

            if (delmarConfig->highlight_auth_retry_count > 0 && cmd->o_is_highlight == 1) {
                maxRetry = delmarConfig->highlight_auth_retry_count;
            }

            if (context->retry < maxRetry) {
                needRetry = needRetryByResult(mContext, context->result, cmd)
                    || (delmarConfig->support_independent_fast_auth > 0 &&
                       DelmarHalUtils::independentFastAuthNeedRetry(mContext, context->result, context->retry))
                    || cmd->o_force_retry > 0;
                // record retry 0 error for onAuthError
                if (context->retry == 0) {
                    mRty0Err = context->result;
                }
            }

            if (needRetry && mContext->mConfig->support_performance_dump) {
                dumpKPI(__func__);
            }
        } while (0);

        VOID_FUNC_EXIT();
        return needRetry;
    }

    gf_error_t DelmarFingerprintCore::notifyAuthSuccess(AuthenticateContext *context) {
        if (mAuthSuccessNotified == 1) {
            return GF_SUCCESS;
        }
        return FingerprintCore::notifyAuthSuccess(context);
    }

    void DelmarFingerprintCore::notifyAuthNotMatched() {
        if (mAuthFailNotified == 1) {
            LOG_D(LOG_TAG, "[%s] mAuthFailNotified=%d", __func__, mAuthFailNotified);
            return;
        }
        return FingerprintCore::notifyAuthNotMatched();
    }

    gf_error_t DelmarFingerprintCore::notifyDismatchInfo(gf_error_t info) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] dismatch due to error= %d", __func__, info);

        if (info == GF_ERROR_ACQUIRED_PARTIAL) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_PARTIAL);
        } else if (info == GF_ERROR_ACQUIRED_IMAGER_DIRTY) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
        } else if (info == GF_ERROR_NOT_LIVE_FINGER) {
            notifyAcquiredInfo(GF_FINGERPRINT_ACQUIRED_NOT_LIVE_FINGER);
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
            /*dump kpi logs is not supported on release version*/
            if (GF_LOG_LEVEL > GF_LOG_INFO_LEVEL) {
                // get kpi
                gf_delmar_algo_kpi_t kpi = { { 0 } };
                kpi.header.target = GF_TARGET_ALGO;
                kpi.header.cmd_id = GF_CMD_ALGO_KPI;
                gf_error_t err = invokeCommand(&kpi, sizeof(gf_delmar_algo_kpi_t));

                if (GF_SUCCESS != err) {
                    break;
                }

                err = DelmarHalUtils::printKpiPerformance(getWorkState(),
                        mTotalKpiTime, &kpi.o_performance, mContext, func_name);
                if (err != GF_SUCCESS) {
                    break;
                }
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::splitPostAuthFlow(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        gf_algo_auth_image_t *auth = context->auth_cmd;
        uint64_t studySensorIds = getAlgo()->getStudySensorIds();
        uint8_t authSuccessSensorId = getAlgo()->getAuthSuccessSensorId();
        uint8_t i = 0;
        FUNC_ENTER();
        do {
            gf_delmar_auth_post_auth_t cmd = { { {0} } };
            cmd.common.i_retry_count = auth->i_retry_count;
            cmd.common.i_gid = auth->o_gid;
            cmd.common.i_finger_id = auth->o_finger_id;
            cmd.common.i_study_flag = getAlgo()->getDisableStudyPropertyFlag() ? 0xFFFFFFFF : auth->o_study_flag;
            cmd.common.header.target = GF_TARGET_BIO;
            cmd.common.header.cmd_id = GF_CMD_AUTH_POST_AUTHENTICATE;

            LOG_D(LOG_TAG, "[%s] authSuccessSensorId=0x%02x, studySensorIds=0x%08x%08x", __func__,
                    authSuccessSensorId, SPLIT_UINT64_TO_UINT32_ARGS(studySensorIds));
            cmd.i_auth_success_sensor_id = authSuccessSensorId;

            while (studySensorIds > 0) {
                for (i = GF_DELMAR_POST_AUTH_STAGE_ONE; i < GF_DELMAR_POST_AUTH_STAGE_END; i++) {
                    cmd.i_stage = i;
                    cmd.i_study_sensor_id = studySensorIds & 0xFF;
                    err = invokeCommand(&cmd, sizeof(cmd));
                    GF_ERROR_BREAK(err);
                    LOG_D(LOG_TAG, "[%s] after stage=%d, save_flag=%d", __func__,
                                    cmd.i_stage, cmd.common.o_save_flag);
                }

                studySensorIds = studySensorIds >> 8;
            }

            cmd.i_stage = GF_DELMAR_POST_AUTH_STAGE_END;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);
            if (cmd.common.o_save_flag > 0) {
                // template saved in ta
                sendMessage(MsgBus::MSG_AUTHENTICATE_SAVE_TEMPLATE_END);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarFingerprintCore::onAfterAuthSuccess(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

#ifdef SUPPORT_PAY
        usleep(100 * 1000);  // sleep 100ms for pay
#endif  // SUPPORT_PAY
        do {
            if (((gf_delmar_algo_auth_image_t *) context->auth_cmd)->o_force_retry == 0) {
                err = splitPostAuthFlow(context);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    bool DelmarFingerprintCore::needRetryIfCaptureFailed(AuthenticateContext * context) {
        FUNC_ENTER();
        if (context->result == GF_ERROR_SPI_RAW_DATA_CRC_FAILED && mCaptureFailRetryCount < MAX_CAPTURE_FAIL_RETRY) {
            mCaptureFailRetryCount++;
            return true;
        }
        return false;
    }
}  // namespace goodix
