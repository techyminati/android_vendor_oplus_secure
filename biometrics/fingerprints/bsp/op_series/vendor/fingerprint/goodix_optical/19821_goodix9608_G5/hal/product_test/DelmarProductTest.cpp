/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][DelmarProductTest]"

#include <inttypes.h>
#include "DelmarProductTest.h"
#include "HalContext.h"
#include "gf_sensor_types.h"
#include "gf_delmar_config.h"
#include "DelmarSensor.h"
#include "DelmarAlgo.h"
#include "TestUtils.hpp"
#include "HalUtils.h"
#include "gf_delmar_types.h"
#include "Timer.h"
#include "Device.h"
#include "Mutex.h"
#include "DelmarHalDump.h"
#include "SensorConfigProvider.h"
#include "DelmarHalUtils.h"

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define PACKAGE_ARRAY_AND_THRESHOLD(inArray, minTh, maxTh, sensorNum, denominator, outArray) \
    do { \
        if (NULL == outArray) {break;} \
        memset(outArray, 0, (sensorNum + 2) * sizeof(float)); \
        if (NULL == inArray) {break;} \
        for (uint32_t x = 0; x < sensorNum; x++) { \
            (outArray)[x] = (float)((inArray)[x] / (denominator)); \
        } \
        outArray[sensorNum] = (float) (minTh / 1.0); \
        outArray[sensorNum + 1] = (float) (maxTh / 1.0); \
    } while (0)

namespace goodix {
    typedef struct GF_PERFORMANCE_TEST_PARAMS {
        uint32_t phase;
        uint32_t sensorX;
        uint32_t sensorY;
        uint32_t sensorWidth;
        uint32_t sensorHeight;
        uint32_t sensorOffset;
        uint32_t screenWidth;
        uint32_t screenHeight;
        uint32_t sensor_center_x[MAX_SENSOR_NUM];
        uint32_t sensor_center_y[MAX_SENSOR_NUM];
        float sensor_ito_rotation;
        uint32_t sensorIndex;
    } gf_performance_test_params_t;  /*params*/

    DelmarProductTest::DelmarProductTest(HalContext *context)
        : ProductTest(context),
          mTestingRstIntPin(false),
          mpRstIntPinTimer(NULL),
          mTestConfig({0}) {  // NOLINT(660)
        mpFingerprintCore = new DelmarFingerprintCore(context);

        mTestConfig.badPointNum = 10000;
        mTestConfig.clusterNum = 10000;
        mTestConfig.pixelOfLargestBadCluster = 10000;
        mTestConfig.bpnInClusters = 10000;
        mTestConfig.lightHBadLineNum = 40;
        mTestConfig.lightVBadLineNum = 40;
        mTestConfig.maxHotConnectedNum = 200;
        mTestConfig.lowCorrPitchLPF = 1000;
        mTestConfig.maxValidArea = 1000;
        mTestConfig.minChartDirection = 0;
        mTestConfig.maxChartDirection = 15;
        mTestConfig.aaDarkDiff = 10;
        mTestConfig.minAngle = -7.5;
        mTestConfig.maxAngle = 7.5;
        mTestConfig.minLightHighMean = 0;
        mTestConfig.maxLightHighMean = 10000;
        mTestConfig.minDiffFleshHM = 450;
        mTestConfig.minDiffFleshML = 200;
        mTestConfig.minDiffBlackHM = 200;
        mTestConfig.minDiffBlackML = 100;
        mTestConfig.maxDiffOffset = 1000;
        mTestConfig.darkTNoise = 8;
        mTestConfig.lightTNoise = 30;
        mTestConfig.darkSNoise = 24;
        mTestConfig.lightSNoise = 300;
        mTestConfig.dataNoiseFlatLPF = 6;
        mTestConfig.unorSignalLPF = 1;
        mTestConfig.signalLPF = 1;
        mTestConfig.ssnrLPF = 1;
        mTestConfig.sharpnessLPF = 0.0;
        mTestConfig.tSNR = 1;
        mTestConfig.maxTNoise = 10;
        mTestConfig.standardAngle = 90.0;
        mTestConfig.chipCenterOffsetX = 5.0;
        mTestConfig.chipCenterOffsetY = 5.0;
        mTestConfig.standardCenterX = 60;
        mTestConfig.standardCenterY = 90;

        mTestConfig.blackPixelNum = 200;
        mTestConfig.whitePixelNum = 200;
        mTestConfig.lightLeakRatio = 50.0;
        mTestConfig.maxITODepth = 0;
        mTestConfig.dpBadPointNum = 100;
        mTestConfig.dpMaxBpnInRow = 100;
        mTestConfig.dpMeanDiff = 100;
        mTestConfig.dPSNoiseDark = 100;
        mTestConfig.dPSNoiseLight = 100;
        mTestConfig.maxHAFBadPointNum = 200;
        mTestConfig.maxHAFBadBlockNum = 200;
        mTestConfig.maxHAFBadRatioNum = 200;
        mTestConfig.realChartDirection = 0;
        mTestConfig.maxDarkLightLeak = 30;
        mTestConfig.maxGhostNum = 100;
    }

    DelmarProductTest::~DelmarProductTest() {
        delete mpFingerprintCore;
        mpFingerprintCore = nullptr;
    }

    bool DelmarProductTest::isNeedCtlSensor(uint32_t cmdId) {
        bool ret = false;

        switch (cmdId) {
            case PRODUCT_TEST_CMD_SPI:
            case PRODUCT_TEST_CMD_OTP_FLASH:
            case PRODUCT_TEST_CMD_PERFORMANCE_TESTING: {
                ret = true;
                break;
            }

            default: {
                break;
            }
        }

        return ret;
    }

    static float getAngleConversion(float angle) {
        float tmpAngle = 0;
        if (angle < 0) {
            tmpAngle = angle + 360;
        } else {
            tmpAngle = angle;
        }
        if (0 <= tmpAngle && tmpAngle <= 45) {
            tmpAngle = -tmpAngle;
        } else if (45 < tmpAngle && tmpAngle <= 135) {
            tmpAngle = -(tmpAngle - 90);
        } else if (135 < tmpAngle && tmpAngle <= 225) {
            tmpAngle = -(tmpAngle - 180);
        } else if (225 < tmpAngle && tmpAngle <= 315) {
            tmpAngle = -(tmpAngle - 270);
        } else if (315 < tmpAngle && tmpAngle < 360) {
            tmpAngle = -(tmpAngle - 360);
        }
        LOG_D(LOG_TAG, "[%s] tmpAngle=%f", __func__, tmpAngle);
        return tmpAngle;
    }

    gf_error_t DelmarProductTest::executeCommand(int32_t cmdId, const int8_t *in,
                                                 uint32_t inLen, int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_SUCCESS;
        gf_error_t ret = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] cmdId=%d", __func__, cmdId);

        if (isNeedCtlSensor(cmdId)) {
            err = sensor->wakeupSensor();
            if (err != GF_SUCCESS) {
                notifyResetStatus(err, cmdId);
                FUNC_EXIT(err);
                return err;
            }
        }
        switch (cmdId) {
            case PRODUCT_TEST_CMD_SPI: {
                err = testSpi();
                break;
            }

            case PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN: {
                err = testResetInterruptPin();
                break;
            }

            case PRODUCT_TEST_CMD_OTP_FLASH: {
                err = testOTPFLash();
                break;
            }

            case PRODUCT_TEST_CMD_PERFORMANCE_TESTING: {
                err = performanceTest(in, inLen);
                break;
            }

            case PRODUCT_TEST_CMD_CANCEL: {
                mpFingerprintCore->cancel();
                break;
            }

            case PRODUCT_TEST_CMD_SET_GAIN_TARGET: {
                err = testSetGainTargetValue(in, inLen);
                break;
            }

            case PRODUCT_TEST_CMD_GET_GAIN_TARGET: {
                err = testGetGainTargetValue();
                break;
            }

            default: {
                err = ProductTest::executeCommand(cmdId, in, inLen, out, outLen);
                break;
            }
        }
        if (isNeedCtlSensor(cmdId)) {
            ret = sensor->sleepSensor();
            if (err == GF_SUCCESS && ret != GF_SUCCESS) {
                err = ret;
                LOG_E(LOG_TAG, "[%s] sleep sensor failed!", __func__);
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::getConfig(int8_t **cfgBuf, uint32_t *bufLen) {
        gf_error_t err = GF_SUCCESS;
        int8_t *test_result = NULL;
        uint32_t len = 0;
        FUNC_ENTER();
        UNUSED_VAR(cfgBuf);
        UNUSED_VAR(bufLen);
        do {
            // gf_chip_type_t chip_type;
            len += HAL_TEST_SIZEOF_INT32;
            // gf_chip_series_t chip_series;
            len += HAL_TEST_SIZEOF_INT32;
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t max_fingers;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t max_fingers_per_user;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t forbidden_untrusted_enroll;
            len += HAL_TEST_SIZEOF_INT32;
            // uint8_t forbidden_enroll_duplicate_fingers;
            len += HAL_TEST_SIZEOF_INT32;
            // uint8_t support_performance_dump;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t enrolling_min_templates;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t valid_image_quality_threshold;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t valid_image_area_threshold;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t duplicate_finger_overlay_score;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t increase_rate_between_stitch_info;
            len += HAL_TEST_SIZEOF_INT32;
            // gf_authenticate_order_t authenticate_order;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t template_update_save_threshold;
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (test_result != NULL) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_CHIP_TYPE,
                                                     mContext->mSensorInfo.chip_type);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_CHIP_SERIES,
                                                     mContext->mSensorInfo.chip_series);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MAX_FINGERS,
                                                     mContext->mConfig->max_fingers);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MAX_FINGERS_PER_USER,
                                                     mContext->mConfig->max_fingers_per_user);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_FORBIDDEN_UNTRUSTED_ENROLL,
                                                     (int32_t) mContext->mConfig->forbidden_untrusted_enroll);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS,
                                                     (int32_t) mContext->mConfig->forbidden_enroll_duplicate_fingers);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_SUPPORT_PERFORMANCE_DUMP,
                                                     (int32_t) mContext->mConfig->support_performance_dump);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_ENROLLING_MIN_TEMPLATES,
                                                     mContext->mConfig->enrolling_min_templates);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD,
                                                     mContext->mConfig->valid_image_quality_threshold);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD,
                                                     mContext->mConfig->valid_image_area_threshold);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE,
                                                     mContext->mConfig->duplicate_finger_overlay_score);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO,
                                                     mContext->mConfig->increase_rate_between_stitch_info);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_AUTHENTICATE_ORDER,
                                                     mContext->mConfig->authenticate_order);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_TEMPLATE_UPDATE_SAVE_THRESHOLD,
                                                     mContext->mConfig->template_update_save_threshold);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            notifyTestCmd(0, CMD_TEST_GET_CONFIG, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        FUNC_EXIT(err);
        return err;
    }

#ifdef SUPPORT_CALCULATE_BAD_POINT_BEFORE_KB_CALI
    gf_error_t DelmarProductTest::checkBadPoint(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        FUNC_ENTER();
        do {
            // check BadPoint & Cluster
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_cmd->o_result.nBadPointNum[sensor_index] <= mTestConfig.badPointNum
                    && cal_cmd->o_result.nClusterNum[sensor_index] <= mTestConfig.clusterNum
                    && cal_cmd->o_result.nPixelOfLargestBadCluster[sensor_index] <= mTestConfig.pixelOfLargestBadCluster
                    && cal_cmd->o_result.nBpnInClusters[sensor_index] <= mTestConfig.bpnInClusters
                    && cal_cmd->o_result.nLightHBadLineNum[sensor_index] <= mTestConfig.lightHBadLineNum
                    && cal_cmd->o_result.nLightVBadLineNum[sensor_index] <= mTestConfig.lightVBadLineNum) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_BAD_POINT_CLUSTER;
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_BAD_POINT_CLUSTER;
                    LOG_E(LOG_TAG, "[%s] BadPointNum=%d(<=%d)", __func__, cal_cmd->o_result.nBadPointNum[sensor_index], mTestConfig.badPointNum);
                    LOG_E(LOG_TAG, "[%s] ClusterNum=%d(<=%d)", __func__, cal_cmd->o_result.nClusterNum[sensor_index], mTestConfig.clusterNum);
                    LOG_E(LOG_TAG, "[%s] PixelOfLargestBadCluster=%d(<=%d)", __func__, cal_cmd->o_result.nPixelOfLargestBadCluster[sensor_index], mTestConfig.pixelOfLargestBadCluster);
                    LOG_E(LOG_TAG, "[%s] BpnInClusters=%d(<=%d)", __func__, cal_cmd->o_result.nBpnInClusters[sensor_index], mTestConfig.bpnInClusters);
                    LOG_E(LOG_TAG, "[%s] LightHBadLineNum=%d(<=%d)", __func__, cal_cmd->o_result.nLightHBadLineNum[sensor_index], mTestConfig.lightHBadLineNum);
                    LOG_E(LOG_TAG, "[%s] LightVBadLineNum=%d(<=%d)", __func__, cal_cmd->o_result.nLightVBadLineNum[sensor_index], mTestConfig.lightVBadLineNum);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check BadPoint & Cluster result fail=%d", __func__, err);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
#endif  // SUPPORT_CALCULATE_BAD_POINT_BEFORE_KB_CALI

    gf_error_t DelmarProductTest::checkGapValueOfEachBrightness(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensor_num) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        FUNC_ENTER();
        do {
            for (sensor_index = 0; sensor_index < sensor_num; sensor_index++) {
                if (cal_cmd->o_result.nMinDiffFleshHM[sensor_index] < mTestConfig.minDiffFleshHM || cal_cmd->o_result.nMinDiffFleshML[sensor_index] < mTestConfig.minDiffFleshML
                    || cal_cmd->o_result.nMinDiffBlackHM[sensor_index] < mTestConfig.minDiffBlackHM || cal_cmd->o_result.nMinDiffBlackML[sensor_index] < mTestConfig.minDiffBlackML
                    || cal_cmd->o_result.nMaxDiffOffset[sensor_index] > mTestConfig.maxDiffOffset) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_SWITCH_BRIGHTNESS;
                    err = GF_ERROR_SWITCH_BRIGHTNESS;
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, nMinDiffFleshHM =%d(>=%d)", __func__, sensor_index, cal_cmd->o_result.nMinDiffFleshHM[sensor_index], mTestConfig.minDiffFleshHM);
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, nMinDiffFleshML =%d(>=%d)", __func__, sensor_index, cal_cmd->o_result.nMinDiffFleshML[sensor_index], mTestConfig.minDiffFleshML);
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, nMinDiffBlackHM =%d(>=%d)", __func__, sensor_index, cal_cmd->o_result.nMinDiffBlackHM[sensor_index], mTestConfig.minDiffBlackHM);
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, nMinDiffBlackML =%d(>=%d)", __func__, sensor_index, cal_cmd->o_result.nMinDiffBlackML[sensor_index], mTestConfig.minDiffBlackML);
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, nMaxDiffOffset =%d(>=%d)", __func__, sensor_index, cal_cmd->o_result.nMaxDiffOffset[sensor_index], mTestConfig.maxDiffOffset);
                } else {
                    cal_cmd->o_result.error_type[sensor_index] = GF_SUCCESS;
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check gap value of each brightness fail. error =%d", __func__, err);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::checkStabilityOfSwitchingBrightness(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        FUNC_ENTER();
        do {
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if ((cal_cmd->o_result.nMaxTNoise[sensor_index] / 1024.0) >= mTestConfig.maxTNoise) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_BRIGHTNESS_INSTABILITY;
                    err = GF_ERROR_BRIGHTNESS_INSTABILITY;
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, nMaxTNoise =%.3f(<%f)", __func__, sensor_index, (cal_cmd->o_result.nMaxTNoise[sensor_index] / 1024.0), mTestConfig.maxTNoise);
                } else {
                    cal_cmd->o_result.error_type[sensor_index] = GF_SUCCESS;
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check stability of switching brightness fail. error =%d", __func__, err);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::checkChartDirection(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        int32_t tempDirection = 0;
        int32_t directionOffset = 0;
        FUNC_ENTER();
        do {
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                tempDirection = ABS(cal_cmd->o_result.nChartDirection[sensor_index]);
                if (tempDirection <= 45) {
                    directionOffset = tempDirection;
                } else {
                    directionOffset = ABS(tempDirection - 90);
                }
                cal_cmd->o_result.nDirectionOffset[sensor_index] = directionOffset;
                LOG_D(LOG_TAG, "[%s] sensor_index =%d, directionOffset=%d", __func__, sensor_index, cal_cmd->o_result.nDirectionOffset[sensor_index]);
                if (directionOffset > mTestConfig.maxChartDirection) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_CHART_DIRECTION;
                    err = GF_ERROR_CHART_DIRECTION;
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, directionOffset=%d(<=%d)", __func__, sensor_index, directionOffset, mTestConfig.maxChartDirection);
                } else {
                    cal_cmd->o_result.error_type[sensor_index] = GF_SUCCESS;
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check chart direction fail. error =%d", __func__, err);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::checkChipOffsetAngel(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        float angel = 0.0;
        FUNC_ENTER();
        do {
            // check AngletoChip
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                angel = cal_cmd->o_result.nAngletoChip[sensor_index] * 180.0 /12868.0;
                if (angel >= 0) {
                    angel = mTestConfig.standardAngle - angel;
                } else {
                    angel = -mTestConfig.standardAngle - angel;
                }
                cal_cmd->o_result.nAngelOffset[sensor_index] = ABS(angel);
                if (ABS(angel) <= mTestConfig.maxAngle) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_LOC_CIRCLE_TEST;
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_LOC_CIRCLE_TEST;
                    LOG_E(LOG_TAG, "[%s] OffsetAngel=%.2f(<=%.2f)", __func__, angel, mTestConfig.maxAngle);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check AngletoChip result fail=%d", __func__, err);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::checkChipOffsetCoordinate(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        float offsetX = 0.0;
        float offsetY = 0.0;
        FUNC_ENTER();
        do {
            // check AngletoChip
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                offsetX = cal_cmd->o_result.nCenterXtoChip[sensor_index] / 256 - mTestConfig.standardCenterX;
                offsetY = cal_cmd->o_result.nCenterYtoChip[sensor_index] / 256 - mTestConfig.standardCenterY;
                if (offsetX < 0) {
                    offsetX = -offsetX;
                }
                if (offsetY < 0) {
                    offsetY = -offsetY;
                }
                cal_cmd->o_result.nCenterXOffset[sensor_index] = offsetX;
                cal_cmd->o_result.nCenterYOffset[sensor_index] = offsetY;
                if (offsetX <= mTestConfig.chipCenterOffsetX && offsetY <= mTestConfig.chipCenterOffsetY) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_LOC_CIRCLE_TEST;
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_LOC_CIRCLE_TEST;
                    LOG_E(LOG_TAG, "[%s] CenterOffsetX=%.2f(<=%.2f), CenterOffsetY=%.2f(<=%.2f)", __func__, offsetX, mTestConfig.chipCenterOffsetX, offsetY, mTestConfig.chipCenterOffsetY);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check Chip Offset Coordinate result fail=%d", __func__, err);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::checkPgaGainThreshold(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        FUNC_ENTER();
        do {
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_cmd->o_result.nLightHighMean[sensor_index] >= mTestConfig.minLightHighMean
                    && cal_cmd->o_result.nLightHighMean[sensor_index] <= mTestConfig.maxLightHighMean) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_CALCULATE_PGA_GAIN;
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_CALCULATE_PGA_GAIN;
                    LOG_E(LOG_TAG, "[%s] LightHighMean=%d(%d<= x <=%d)", __func__, cal_cmd->o_result.nLightHighMean[sensor_index], mTestConfig.minLightHighMean, mTestConfig.maxLightHighMean);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check Pga Gain Threshold result fail=%d", __func__, err);
                break;
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }
    gf_error_t DelmarProductTest::checkPerformanceThreshold(gf_calculate_cmd_result_t *cal_result) {
        gf_error_t err = GF_SUCCESS;
        uint32_t sensor_index = 0;
        uint32_t sensorNum = ((DelmarSensor*) mContext->mSensor)->getAvailableSensorNum();

        FUNC_ENTER();
        do {
            // check BadPoint & Cluster
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_result->nBadPointNum[sensor_index] <= mTestConfig.badPointNum
                    && cal_result->nClusterNum[sensor_index] <= mTestConfig.clusterNum
                    && cal_result->nPixelOfLargestBadCluster[sensor_index] <= mTestConfig.pixelOfLargestBadCluster
                    && cal_result->nBpnInClusters[sensor_index] <= mTestConfig.bpnInClusters
                    && cal_result->nLightHBadLineNum[sensor_index] <= mTestConfig.lightHBadLineNum
                    && cal_result->nLightVBadLineNum[sensor_index] <= mTestConfig.lightVBadLineNum
                    && cal_result->nHotPixelNum[sensor_index] <= mTestConfig.maxHotConnectedNum
                    && cal_result->nBWhitePixelNum[sensor_index] <= mTestConfig.whitePixelNum) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_BAD_POINT_CLUSTER;
                    cal_result->error_type[sensor_index] = GF_ERROR_BAD_POINT_CLUSTER;
                    LOG_E(LOG_TAG, "[%s] BadPointNum=%d(<=%d)", __func__, cal_result->nBadPointNum[sensor_index], mTestConfig.badPointNum);
                    LOG_E(LOG_TAG, "[%s] ClusterNum=%d(<=%d)", __func__, cal_result->nClusterNum[sensor_index], mTestConfig.clusterNum);
                    LOG_E(LOG_TAG, "[%s] PixelOfLargestBadCluster=%d(<=%d)", __func__, cal_result->nPixelOfLargestBadCluster[sensor_index], mTestConfig.pixelOfLargestBadCluster);
                    LOG_E(LOG_TAG, "[%s] BpnInClusters=%d(<=%d)", __func__, cal_result->nBpnInClusters[sensor_index], mTestConfig.bpnInClusters);
                    LOG_E(LOG_TAG, "[%s] LightHBadLineNum=%d(<=%d)", __func__, cal_result->nLightHBadLineNum[sensor_index], mTestConfig.lightHBadLineNum);
                    LOG_E(LOG_TAG, "[%s] LightVBadLineNum=%d(<=%d)", __func__, cal_result->nLightVBadLineNum[sensor_index], mTestConfig.lightVBadLineNum);
                    LOG_E(LOG_TAG, "[%s] HotPixelNum=%d(<=%d)", __func__, cal_result->nHotPixelNum[sensor_index], mTestConfig.maxHotConnectedNum);
                    LOG_E(LOG_TAG, "[%s] BWhitePixelNum=%d(<=%d)", __func__, cal_result->nBWhitePixelNum[sensor_index], mTestConfig.whitePixelNum);
                    LOG_E(LOG_TAG, "[%s] BBlackPixelNum=%d(<=%d)", __func__, cal_result->nBBlackPixelNum[sensor_index], mTestConfig.blackPixelNum);
                    LOG_E(LOG_TAG, "[%s] nGhostNum=%d(<=%d)", __func__, cal_result->nGhostNum[sensor_index], mTestConfig.maxGhostNum);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check BadPoint & Cluster result fail=%d", __func__, err);
                break;
            }

            // check InvalidArea
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_result->nValidArea[sensor_index] <= mTestConfig.maxValidArea
                    && cal_result->nLowCorrPitch_LPF[sensor_index] <= mTestConfig.lowCorrPitchLPF
                    && cal_result->nMtDarkOp[sensor_index] <= mTestConfig.maxDarkLightLeak) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_PRESS_INCOMPLETE;
                    cal_result->error_type[sensor_index] = GF_ERROR_PRESS_INCOMPLETE;
                    LOG_E(LOG_TAG, "[%s] LowCorrPitch_LPF=%d(<=%d)", __func__, cal_result->nLowCorrPitch_LPF[sensor_index], mTestConfig.lowCorrPitchLPF);
                    LOG_E(LOG_TAG, "[%s] ValidArea=%d(<=%d)", __func__, cal_result->nValidArea[sensor_index], mTestConfig.maxValidArea);
                    LOG_E(LOG_TAG, "[%s] MtDarkOp=%d(<=%d)", __func__, cal_result->nMtDarkOp[sensor_index], mTestConfig.maxDarkLightLeak);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check ValidArea result fail=%d", __func__, err);
                break;
            }

            // check AADarkDiff
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_result->nAADarkDiff[sensor_index] <= mTestConfig.aaDarkDiff) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_BLACK_RUBBER_LEAKAGE;
                    cal_result->error_type[sensor_index] = GF_ERROR_BLACK_RUBBER_LEAKAGE;
                    LOG_E(LOG_TAG, "[%s] AADarkDiff=%d(<=%d)", __func__, cal_result->nAADarkDiff[sensor_index], mTestConfig.aaDarkDiff);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check AADarkDiff result fail=%d", __func__, err);
                break;
            }

            // check LightHighMean
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_result->nLightHighMean[sensor_index] >= mTestConfig.minLightHighMean
                    && cal_result->nLightHighMean[sensor_index] <= mTestConfig.maxLightHighMean) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_HIGH_LIGHT;
                    cal_result->error_type[sensor_index] = GF_ERROR_HIGH_LIGHT;
                    LOG_E(LOG_TAG, "[%s] LightHighMean=%d(%d<= x <=%d)", __func__, cal_result->nLightHighMean[sensor_index], mTestConfig.minLightHighMean, mTestConfig.maxLightHighMean);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check LightHighMean result fail=%d", __func__, err);
                break;
            }

            // check TNoise
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if ((cal_result->nDarkTNoise[sensor_index] / 1024.0) <= mTestConfig.darkTNoise
                    && (cal_result->nLightTNoise[sensor_index] / 1024.0) <= mTestConfig.lightTNoise) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_TNOISE;
                    cal_result->error_type[sensor_index] = GF_ERROR_TNOISE;
                    LOG_E(LOG_TAG, "[%s] DarkTNoise=%.3f(<=%d)", __func__, (cal_result->nDarkTNoise[sensor_index] / 1024.0), mTestConfig.darkTNoise);
                    LOG_E(LOG_TAG, "[%s] LightTNoise=%.3f(<=%d)", __func__, (cal_result->nLightTNoise[sensor_index] / 1024.0), mTestConfig.lightTNoise);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check TNoise result fail=%d", __func__, err);
                break;
            }

            // check SNoise
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if ((cal_result->nDarkSNoise[sensor_index] / 1024.0) <= mTestConfig.darkSNoise
                    && (cal_result->nLightSNoise[sensor_index] / 1024.0) <= mTestConfig.lightSNoise) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_SNOISE;
                    cal_result->error_type[sensor_index] = GF_ERROR_SNOISE;
                    LOG_E(LOG_TAG, "[%s] DarkSNoise=%.3f(<=%d)", __func__, (cal_result->nDarkSNoise[sensor_index] / 1024.0), mTestConfig.darkSNoise);
                    LOG_E(LOG_TAG, "[%s] LightSNoise=%.3f(<=%d)", __func__, (cal_result->nLightSNoise[sensor_index] / 1024.0), mTestConfig.lightSNoise);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check SNoise result fail=%d", __func__, err);
                break;
            }

            // check FlatSnoise
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if ((cal_result->nDataNoiseFlatLPF[sensor_index] / 1024.0) <= mTestConfig.dataNoiseFlatLPF) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_FLATSNOISE;
                    cal_result->error_type[sensor_index] = GF_ERROR_FLATSNOISE;
                    LOG_E(LOG_TAG, "[%s] DataNoiseFlatLPF=%.3f(<=%d)", __func__, (cal_result->nDataNoiseFlatLPF[sensor_index] / 1024.0), mTestConfig.dataNoiseFlatLPF);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check FlatSnoise result fail=%d", __func__, err);
                break;
            }

            // check Signal
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if ((cal_result->nUnorSignal_LPF[sensor_index] / 256.0) >= mTestConfig.unorSignalLPF) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_SIGNAL;
                    cal_result->error_type[sensor_index] = GF_ERROR_SIGNAL;
                    LOG_E(LOG_TAG, "[%s] UnorSignal_LPF=%.3f(>=%f)", __func__, (cal_result->nUnorSignal_LPF[sensor_index] / 256.0), mTestConfig.unorSignalLPF);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check Signal result fail=%d", __func__, err);
                break;
            }

            // check SSNR
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (1) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_SSNR;
                    cal_result->error_type[sensor_index] = GF_ERROR_SSNR;
                    LOG_E(LOG_TAG, "[%s] SSNR_LPF=%.3f(>=%d)", __func__, (cal_result->nSSNR_LPF[sensor_index] /256.0), mTestConfig.ssnrLPF);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check SSNR result fail=%d", __func__, err);
                break;
            }

            // check TSNR
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if ((cal_result->nTSNR[sensor_index] / 256.0) >= mTestConfig.tSNR) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_TSNR;
                    cal_result->error_type[sensor_index] = GF_ERROR_TSNR;
                    LOG_E(LOG_TAG, "[%s] TSNR=%.3f(>=%f)", __func__, (cal_result->nTSNR[sensor_index] /256.0), mTestConfig.tSNR);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check TSNR result fail=%d", __func__, err);
                break;
            }

            // check Shapeness
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (((double)cal_result->nShapeness_LPF[sensor_index] / (1 << 20)) >= mTestConfig.sharpnessLPF) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_SHARPNESS;
                    cal_result->error_type[sensor_index] = GF_ERROR_SHARPNESS;
                    LOG_E(LOG_TAG, "[%s] Shapeness_LPF=%.3f(>=%.3f)", __func__, ((double)cal_result->nShapeness_LPF[sensor_index] / (1 << 20)), mTestConfig.sharpnessLPF);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check Shapeness result fail=%d", __func__, err);
                break;
            }

            // check CF bad point
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_result->nHAFBadBlockNum[sensor_index] <= mTestConfig.maxHAFBadBlockNum
                    && cal_result->nHAFBadPointNum[sensor_index] <= mTestConfig.maxHAFBadPointNum) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_HAF_BAD_POINT_CLUSTER;
                    cal_result->error_type[sensor_index] = GF_ERROR_HAF_BAD_POINT_CLUSTER;
                    LOG_E(LOG_TAG, "[%s] HAFBadBlockNum=%d(<=%d)", __func__, cal_result->nHAFBadBlockNum[sensor_index], mTestConfig.maxHAFBadBlockNum);
                    LOG_E(LOG_TAG, "[%s] HAFBadPointNum=%d(<=%d)", __func__, cal_result->nHAFBadPointNum[sensor_index], mTestConfig.maxHAFBadPointNum);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check HAF bad point result fail=%d", __func__, err);
                break;
            }

            // check dark pixel param
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if (cal_result->nDpMeanDiff[sensor_index] <= mTestConfig.dpMeanDiff
                    && (cal_result->nDPSNoiseDark[sensor_index] / 1024.0) <= mTestConfig.dPSNoiseDark
                    && cal_result->nDpBadPointNum[sensor_index] <= mTestConfig.dpBadPointNum
                    && cal_result->nDpMaxBpnInRow[sensor_index] <= mTestConfig.dpMaxBpnInRow) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_DARK_PIXEL_PARAM;
                    cal_result->error_type[sensor_index] = GF_ERROR_DARK_PIXEL_PARAM;
                    LOG_E(LOG_TAG, "[%s] DpMeanDiff=%d(<=%d)", __func__, cal_result->nDpMeanDiff[sensor_index], mTestConfig.dpMeanDiff);
                    LOG_E(LOG_TAG, "[%s] DPSNoiseDark=%.3f(<=%d)", __func__, (cal_result->nDPSNoiseDark[sensor_index] / 1024.0), mTestConfig.dPSNoiseDark);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check dark pixel param result fail=%d", __func__, err);
                break;
            }

            // check screen param
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                if ((cal_result->nLightLeakRatio[sensor_index] / 256.0) <= mTestConfig.lightLeakRatio) {
                    cal_result->error_type[sensor_index] = GF_SUCCESS;
                } else {
                    err = GF_ERROR_SCREEN_PARAM;
                    cal_result->error_type[sensor_index] = GF_ERROR_SCREEN_PARAM;
                    LOG_E(LOG_TAG, "[%s] nLightLeakRatio=%.3f(<=%f)", __func__, (cal_result->nLightLeakRatio[sensor_index] / 256.0), mTestConfig.lightLeakRatio);
                }
            }
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] check screen param result fail=%d", __func__, err);
                break;
            }
        } while (0);
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] check performance threshold fail. error =%d", __func__, err);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::saveProductTestingResult(gf_error_t err) {
        FUNC_ENTER();
        FUNC_EXIT(err);
        return err;
    }

    bool DelmarProductTest::isPerformanceTestCollectPhase(uint32_t phase) {
        return (phase == OPERATION_STEP_BASEDATA_DARK_COLLECT)
               || (phase == OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT)
               || (phase == OPERATION_STEP_BASEDATA_MID_DARK_COLLECT)
               || (phase == OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT)
               || (phase == OPERATION_STEP_BASEDATA_MIN_COLLECT)
               || (phase == OPERATION_STEP_BASEDATA_MID_COLLECT)
               || (phase == OPERATION_STEP_BASEDATA_MAX_COLLECT)
               || (phase == OPERATION_STEP_CHARTDATA_COLLECT)
               || (phase == OPERATION_STEP_CIRCLEDATA_COLLECT)
               || (phase == OPERATION_STEP_CALCULATE_GAIN_COLLECT)
               || (phase == OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT)
               || (phase == OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT)
               || (phase == OPERATION_STEP_CALCULATE_MID_BRIGHTNESS_GAIN_COLLECT)
               || (phase == OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_GAIN_COLLECT)
               || (phase == OPERATION_STEP_MID_BRIGHTNESS_FRESH_COLLECT)
               || (phase == OPERATION_STEP_LOW_BRIGHTNESS_FRESH_COLLECT);
    }

    gf_error_t DelmarProductTest::performanceTest(const int8_t *in, uint32_t inLen) {
        gf_error_t err = GF_SUCCESS;
        uint32_t token = 0;
        gf_performance_test_params_t params = { 0 };
        void *cmd = NULL;
        uint64_t t0 = 0;
        uint64_t t1 = 0;
        uint64_t t2 = 0;
        uint64_t t3 = 0;
        int32_t size = 0;
        int32_t i = 0;
        uint32_t status_bar_height = 0;
        uint32_t is_full_screen = 0;
        uint32_t is_whole_circle_pic = 0;
        uint32_t externalCmdId = 0;
        uint32_t feature_type = 0;
        gf_delmar_collect_cmd_t *col_cmd = NULL;
        gf_delmar_calculate_cmd_t *cal_cmd = NULL;
        GoodixSensorConfig sensorConfig = {{0}};
#ifdef SUPPORT_DUMP
        uint8_t *circle_bmp_data = NULL;
        uint32_t size_calculate = sizeof(gf_delmar_calculate_cmd_t);
        uint32_t size_threshold = sizeof(gf_delmar_product_test_config_t);
        uint8_t* temp_buff = NULL;
#endif  // SUPPORT_DUMP
        char cur_time_str[TIME_STAMP_LEN];
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        uint32_t caliState = 0;
        int32_t sensorNum = sensor->getAvailableSensorNum();
        FUNC_ENTER();

        do {
            params.phase = 0;
            params.sensorX = 491;
            params.sensorY = 1786;
            params.sensorWidth = 84;
            params.sensorHeight = 86;
            params.sensorOffset = 10;
            params.screenWidth = 1080;
            params.screenHeight = 2208;
            params.sensorIndex = 0;
            t2 = HalUtils::getCurrentTimeMicrosecond();
            const int8_t *in_buf = in;

            if (NULL != in_buf) {
                do {
                    in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                    switch (token) {
                        case PRODUCT_TEST_TOKEN_COLLECT_PHASE: {
                            in_buf = TestUtils::testDecodeUint32(&params.phase, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_X: {
                            in_buf = TestUtils::testDecodeUint32(&params.sensorX, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_Y: {
                            in_buf = TestUtils::testDecodeUint32(&params.sensorY, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_WIDTH: {
                            in_buf = TestUtils::testDecodeUint32(&params.sensorWidth, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_HEIGHT: {
                            in_buf = TestUtils::testDecodeUint32(&params.sensorHeight, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_OFFSET: {
                            in_buf = TestUtils::testDecodeUint32(&params.sensorOffset, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_SCREEN_WIDTH: {
                            in_buf = TestUtils::testDecodeUint32(&params.screenWidth, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_SCREEN_HEIGHT: {
                            in_buf = TestUtils::testDecodeUint32(&params.screenHeight, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_PERFORMANCE_TEST_CMD_ID: {
                            in_buf = TestUtils::testDecodeUint32(&externalCmdId, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_SENSOR_INDEX: {
                            in_buf = TestUtils::testDecodeUint32(&params.sensorIndex, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_STATUS_BAR_HEIGHT: {
                            in_buf = TestUtils::testDecodeUint32(&status_bar_height, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_IS_FULL_SCREEN: {
                            in_buf = TestUtils::testDecodeUint32(&is_full_screen, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_IS_WHOLE_CIRCLE_PIC: {
                            in_buf = TestUtils::testDecodeUint32(&is_whole_circle_pic, in_buf);
                            break;
                        }

                        case PRODUCT_TEST_TOKEN_FEATURE_TYPE: {
                            in_buf = TestUtils::testDecodeUint32(&feature_type, in_buf);
                            break;
                        }

                        default: {
                            LOG_D(LOG_TAG, "[%s] invilid token", __func__);
                            break;
                        }
                    }
                } while (in_buf < in + inLen);
            }


            sensor->getSensorConfigProvider()->getConfig(&sensorConfig, sensorNum);
            for (i = 0; i < sensorNum; i++) {
                params.sensor_center_x[i] = sensorConfig.sensorX[i];
                params.sensor_center_y[i] = sensorConfig.sensorY[i];
                params.sensor_ito_rotation = sensorConfig.sensorRotation[i];
                LOG_D(LOG_TAG, "[%s] sensor_center_x[%d]=%d", __func__, i, params.sensor_center_x[i]);
                LOG_D(LOG_TAG, "[%s] sensor_center_y[%d]=%d", __func__, i, params.sensor_center_y[i]);
            }
            params.sensor_ito_rotation = getAngleConversion(params.sensor_ito_rotation);
            LOG_D(LOG_TAG, "[%s] sensor_ito_rotation=%f", __func__, params.sensor_ito_rotation);
            LOG_D(LOG_TAG, "[%s] phase=%d", __func__, params.phase);
            LOG_D(LOG_TAG, "[%s] sensorX=%d", __func__, params.sensorX);
            LOG_D(LOG_TAG, "[%s] sensorY=%d", __func__, params.sensorY);
            LOG_D(LOG_TAG, "[%s] sensorWidth=%d", __func__, params.sensorWidth);
            LOG_D(LOG_TAG, "[%s] sensorHeight=%d", __func__, params.sensorHeight);
            LOG_D(LOG_TAG, "[%s] sensorOffset=%d", __func__, params.sensorOffset);
            LOG_D(LOG_TAG, "[%s] screenWidth=%d", __func__, params.screenWidth);
            LOG_D(LOG_TAG, "[%s] screenHeight=%d", __func__, params.screenHeight);
            LOG_D(LOG_TAG, "[%s] externalCmdId=%d", __func__, externalCmdId);
            LOG_D(LOG_TAG, "[%s] sensorIndex=%d", __func__, params.sensorIndex);
            LOG_D(LOG_TAG, "[%s] is_whole_circle_pic=%d", __func__, is_whole_circle_pic);
            LOG_D(LOG_TAG, "[%s] feature_type=%d", __func__, feature_type);

            if (isPerformanceTestCollectPhase(params.phase)) {
                col_cmd = new gf_delmar_collect_cmd_t();
                cmd = col_cmd;
                size = sizeof(gf_delmar_collect_cmd_t);
                memset(col_cmd, 0, size);
                col_cmd->cmd_base.header.cmd_id = GF_CMD_TEST_PERFORMANCE_TESTING;
                col_cmd->cmd_base.header.target = GF_TARGET_PRODUCT_TEST;
                col_cmd->cmd_base.i_collect_phase = params.phase;
                col_cmd->cmd_base.i_sensor_index = params.sensorIndex;
            } else {
                cal_cmd = new gf_delmar_calculate_cmd_t();
                cmd = cal_cmd;
                size = sizeof(gf_delmar_calculate_cmd_t);
                memset(cal_cmd, 0, size);
                cal_cmd->cmd_base.header.cmd_id = GF_CMD_TEST_PERFORMANCE_TESTING;
                cal_cmd->cmd_base.header.target = GF_TARGET_PRODUCT_TEST;
                cal_cmd->cmd_base.i_collect_phase = params.phase;
                cal_cmd->cmd_base.i_sensor_index = params.sensorIndex;
                cal_cmd->i_loc_params.nSensorX = params.sensorX;
                cal_cmd->i_loc_params.nSensorY = params.sensorY;
                cal_cmd->i_loc_params.nSensorWidth = params.sensorWidth;
                cal_cmd->i_loc_params.nSensorHeight = params.sensorHeight;
                cal_cmd->i_loc_params.nSensorOffset = params.sensorOffset;
                cal_cmd->i_loc_params.nScreenWidth = params.screenWidth;
                cal_cmd->i_loc_params.nScreenHeight = params.screenHeight;
                cal_cmd->i_loc_params.nIs_Whole_Pic = is_whole_circle_pic;
                cal_cmd->feature_type = feature_type;
                for (i = 0; i < sensorNum; i++) {
                    cal_cmd->i_loc_params.nSensor_Center_X[i] = params.sensor_center_x[i];
                    cal_cmd->i_loc_params.nSensor_Center_Y[i] = params.sensor_center_y[i];
                    cal_cmd->i_loc_params.nSensorITORotation = params.sensor_ito_rotation;
                }
            }

            t0 = HalUtils::getCurrentTimeMicrosecond();
            err = invokeCommand(cmd, size);
            t1 = HalUtils::getCurrentTimeMicrosecond();
            LOG_I(LOG_TAG, "[%s] gf_hal_test_invoke_command cost time. phase =%d, time =%"
                  PRId64 "", __func__, params.phase, (int64_t)(t1 - t0) / 1000);
            memset(cur_time_str, 0, sizeof(cur_time_str));
            DelmarHalUtils::genTimestamp(cur_time_str, sizeof(cur_time_str));

            if (params.phase == OPERATION_STEP_GET_LOCATION_CIRCLE) {
                notifyLocationCircleImage(err, cmd);
            } else {
                if (params.phase == OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN
                    && cal_cmd != NULL) {
                    if (err == GF_SUCCESS) {
                        err = checkPgaGainThreshold(cal_cmd, sensorNum);
                    }
                }
                if (params.phase == OPERATION_STEP_GET_KB_CALIBRATION
                    && cal_cmd != NULL) {
#ifdef SUPPORT_CALCULATE_BAD_POINT_BEFORE_KB_CALI
                    gf_error_t save_cali_err = GF_SUCCESS;
                    if (feature_type != KB_CALIBRATION_TYPE && feature_type != GAIN_AND_CALIBRATION_TYPE) {
                        save_cali_err = err;
                        err = checkBadPoint(cal_cmd, sensorNum);
                        if (err == GF_SUCCESS) {
                            err = checkGapValueOfEachBrightness(cal_cmd, sensorNum);
                        }
                        if (err == GF_SUCCESS) {
                            err = checkStabilityOfSwitchingBrightness(cal_cmd, sensorNum);
                        }
                        if (err == GF_SUCCESS) {
                            err = save_cali_err;
                        }
                    }
#endif  // SUPPORT_CALCULATE_BAD_POINT_BEFORE_KB_CALI
                }
                if (params.phase == OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS
                    && cal_cmd != NULL) {
                    if (err ==GF_SUCCESS) {
                        err = checkGapValueOfEachBrightness(cal_cmd, sensorNum);
                        if (err == GF_SUCCESS) {
                            err = checkStabilityOfSwitchingBrightness(cal_cmd, sensorNum);
                        }
                        if (err == GF_SUCCESS) {
                            err = checkChartDirection(cal_cmd, sensorNum);
                        }
                        if (err == GF_SUCCESS) {
                            err = checkChipOffsetAngel(cal_cmd, sensorNum);
                        }
                        if (err == GF_SUCCESS) {
                            err = checkChipOffsetCoordinate(cal_cmd, sensorNum);
                        }
                        if (err == GF_SUCCESS) {
                            // this interface will be used when the threshold be updated.
                            err = checkPerformanceThreshold(&cal_cmd->o_result);
                        }

                        // save product testing result
                        saveProductTestingResult(err);
                    }
                    cal_cmd->err_code = err;
                    LOG_D(LOG_TAG, "[%s] err=%d", __func__, err);
                    for (i = 0; i < sensorNum; i++) {
                        cal_cmd->o_result.error_type[i] = err;
                    }
                }
                notifyPerformanceTest(err, params.phase, cmd);
            }

            if (isPerformanceTestCollectPhase(params.phase)) {
#ifdef SUPPORT_DUMP
                DelmarHalDump *bkdump = static_cast<DelmarHalDump *>(mContext->mHalDump);
                bkdump->dumpRawDataForPerformanceTest(col_cmd->o_raw_data,
                                                      cur_time_str,
                                                      col_cmd->cmd_base.o_frame_num,
                                                      mContext->mSensorInfo.col,
                                                      mContext->mSensorInfo.row,
                                                      params.phase,
                                                      DUMP_RAW_DATA_FOR_PERFORMANCE,
                                                      params.sensorIndex,
                                                      feature_type);
                if (OPERATION_STEP_BASEDATA_MAX_COLLECT == params.phase) {
                    bkdump->dumpRawDataForPerformanceTest(col_cmd->o_raw_data_flesh,
                                                      cur_time_str,
                                                      col_cmd->cmd_base.o_frame_num,
                                                      mContext->mSensorInfo.col,
                                                      mContext->mSensorInfo.row,
                                                      OPERATION_STEP_MAXFLESH_COLLECT,
                                                      DUMP_RAW_DATA_FOR_PERFORMANCE,
                                                      params.sensorIndex,
                                                      feature_type);
                }
#ifdef SUPPORT_DUMP_ORIGIN_DATA
                bkdump->dumpPermanceTestOriginData(DUMP_ORIGIN_DATA_FOR_PERFORMANCE,
                                                   col_cmd->origin_data,
                                                   params.phase,
                                                   cur_time_str,
                                                   col_cmd->frame_num,
                                                   2,
                                                   col_cmd->origin_col,
                                                   col_cmd->origin_row);
                if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == err && col_cmd->crc_origin_len > 0) {
                    bkdump->doDumpCrcOriginData(col_cmd->crc_origin_data);
                }
#endif  // SUPPORT_DUMP_ORIGIN_DATA
#endif  // SUPPORT_DUMP

                if (params.phase == OPERATION_STEP_BASEDATA_DARK_COLLECT) {
                    // TODO dump sensor id
                }

                if (params.phase == OPERATION_STEP_CIRCLEDATA_COLLECT) {
#ifdef SUPPORT_DUMP
                    for (i = 0; i < sensorNum; i++) {
                        circle_bmp_data = new uint8_t[LOCATION_CIRCLE_PIC_BUFFER_LEN] { 0 };
                        TestUtils::quantizeRawdataToBmp(col_cmd->o_raw_data + i * col_cmd->cmd_base.o_frame_num * mContext->mSensorInfo.col * mContext->mSensorInfo.row,
                                                        circle_bmp_data, mContext->mSensorInfo.col, mContext->mSensorInfo.row);
                        bkdump->dumpLocationCircleBmp(cur_time_str, circle_bmp_data, mContext->mSensorInfo.col,
                                                        mContext->mSensorInfo.row, params.phase, i);
                    }
#endif  // SUPPORT_DUMP
                }
            } else if (params.phase == OPERATION_STEP_CALCULATE_RESULT
                     || params.phase == OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS
                     || params.phase == OPERATION_STEP_GET_KB_CALIBRATION) {
                if (err == GF_SUCCESS && params.phase == OPERATION_STEP_GET_KB_CALIBRATION) {
                    caliState = sensor->getCaliState();
                    caliState |= (1 << DELMAR_CALI_STATE_KB_READY_BIT);
                    sensor->setCaliState(caliState);
                }
                // dump performance test parameter
#ifdef SUPPORT_DUMP
                temp_buff = (uint8_t *) malloc(size_calculate + size_threshold);
                if (NULL == temp_buff) {
                    LOG_E(LOG_TAG, "[%s] out of memory, temp_buff", __func__);
                    err = GF_ERROR_OUT_OF_MEMORY;
                    break;
                }
                memset(temp_buff, 0, size_calculate + size_threshold);
                memcpy(temp_buff, cal_cmd, size_calculate);
                memcpy(temp_buff + size_calculate, &mTestConfig, size_threshold);
                DelmarHalDump *bkdump = static_cast<DelmarHalDump *>(mContext->mHalDump);
                bkdump->dumpPerformanceTestParameters(cur_time_str, temp_buff, params.phase);
#endif  // SUPPORT_DUMP
            } else if (params.phase == OPERATION_STEP_CALCULATE_GAIN_TWO&& cal_cmd != NULL) {
                if (err == GF_SUCCESS) {
                    caliState = sensor->getCaliState();
                    caliState |= (1 << DELMAR_CALI_STATE_PGA_GAIN_READY_BIT);
                    sensor->setCaliState(caliState);
                }
#ifdef SUPPORT_DUMP
                DelmarHalDump *bkdump = static_cast<DelmarHalDump *>(mContext->mHalDump);
                bkdump->dumpPgaGainRecalculateResult(cur_time_str, cal_cmd);
#endif  // SUPPORT_DUMP
                for (i = 0; i < sensorNum; i++) {
                    LOG_D(LOG_TAG, "[%s] sendsorIndex_%d Recalculate pga_gain=%d", __func__,
                        i, cal_cmd->o_pga_gain[i]);
                }
            } else if (params.phase == OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_GAIN && cal_cmd != NULL) {
                if (err == GF_SUCCESS) {
                    caliState = sensor->getCaliState();
                    LOG_D(LOG_TAG, "[%s] before, caliState=0x%08x", __func__, caliState);
                    caliState |= (1 << DELMAR_CALI_STATE_LB_PGA_GAIN_READY_BIT);
                    LOG_D(LOG_TAG, "[%s] after, caliState=0x%08x", __func__, caliState);
                    sensor->setCaliState(caliState);
                }
#ifdef SUPPORT_DUMP
                DelmarHalDump *bkdump = static_cast<DelmarHalDump *>(mContext->mHalDump);
                bkdump->dumpLbPgaGainRecalculateResult(cur_time_str, cal_cmd);
#endif  // SUPPORT_DUMP
                for (i = 0; i < sensorNum; i++) {
                    LOG_D(LOG_TAG, "[%s] sendsorIndex_%d Recalculate pga_gain=%d", __func__,
                        i, cal_cmd->o_pga_gain[i]);
                }
            }

            t3 = HalUtils::getCurrentTimeMicrosecond();
            LOG_I(LOG_TAG, "[%s] per operation step cost time. phase =%d, time =%" PRId64
                  "", __func__, params.phase, (int64_t)(t3 - t2) / 1000);
        } while (0);

        if (NULL != col_cmd) {
            delete col_cmd;
        }

        if (NULL != cal_cmd) {
            delete cal_cmd;
        }
#ifdef SUPPORT_DUMP
        if (NULL != temp_buff) {
            free(temp_buff);
            temp_buff = NULL;
        }
#endif  // SUPPORT_DUMP
        FUNC_EXIT(err);
        return err;
    }

    void DelmarProductTest::notifyPerformanceTest(gf_error_t err, uint32_t phase,
                                                  void *cmd) {
        int8_t *test_result = NULL;
        gf_delmar_performance_test_cmd_base_t *parent_result = NULL;
        gf_delmar_calculate_cmd_t *cal_cmd = NULL;
        gf_calculate_cmd_result_t *cal_result = NULL;
        uint32_t len = 0;
        bool isCollectPhase = true;
        uint32_t sensorNum = ((DelmarSensor*) mContext->mSensor)->getAvailableSensorNum();
        uint32_t resultAndThSize = (sensorNum + 2)* sizeof(float);
        VOID_FUNC_ENTER();
        UNUSED_VAR(phase);
        UNUSED_VAR(isCollectPhase);
        parent_result = (gf_delmar_performance_test_cmd_base_t *) cmd;
        isCollectPhase = isPerformanceTestCollectPhase(parent_result->i_collect_phase);
        // error code
        len += HAL_TEST_SIZEOF_INT32;
        // collect_phase
        len += HAL_TEST_SIZEOF_INT32;

        if (parent_result->i_collect_phase == OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS) {
            // 39 items
            len += HAL_TEST_SIZEOF_ARRAY(resultAndThSize) * 39;
        } else if (parent_result->i_collect_phase == OPERATION_STEP_CALCULATE_GAIN) {
            // pga gain
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint8_t));
        } else if (parent_result->i_collect_phase == OPERATION_STEP_GET_KB_CALIBRATION) {
            // calculate the key index before KB calibration
            len += HAL_TEST_SIZEOF_ARRAY(resultAndThSize) * 12;
        }

        test_result = new int8_t[len] { 0 };

        if (test_result != NULL) {
            memset(test_result, 0, len);
            int8_t *current = test_result;
            if (parent_result->i_collect_phase == OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS) {
                float* resultAndTh = new float[resultAndThSize / sizeof(float)] { 0 };
                cal_cmd = (gf_delmar_calculate_cmd_t *)cmd;
                cal_result = &cal_cmd->o_result;

                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, parent_result->i_collect_phase);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nBadPointNum, 0, mTestConfig.badPointNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_BAD_POINT_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nClusterNum, 0, mTestConfig.clusterNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_CLUSTER_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nPixelOfLargestBadCluster, 0, mTestConfig.pixelOfLargestBadCluster,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LARGEST_BAD_CLUSTER,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nBpnInClusters, 0, mTestConfig.bpnInClusters,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_BPN_IN_CLUSTER,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightHBadLineNum, 0, mTestConfig.lightHBadLineNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_HBAD_LINE_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightVBadLineNum, 0, mTestConfig.lightVBadLineNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_VBAD_LINE_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nHotPixelNum, 0, mTestConfig.maxHotConnectedNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_HOT_PIXEL_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLowCorrPitch_LPF, 0, mTestConfig.lowCorrPitchLPF,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LOW_CORR_PITCH_LPF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nValidArea, 0, mTestConfig.maxValidArea,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_VALID_AREA,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nAADarkDiff, 0, mTestConfig.aaDarkDiff,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_AA_DARK_DIFF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nAngelOffset, 0, mTestConfig.maxAngle,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_ANGLE_TO_CHIP,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightHighMean, mTestConfig.minLightHighMean,
                                 mTestConfig.maxLightHighMean, sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_HIGH_MEAN,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDarkTNoise, 0, mTestConfig.darkTNoise,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DARK_TNOISE,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDarkSNoise, 0, mTestConfig.darkSNoise,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DARK_SNOISE,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightTNoise, 0, mTestConfig.lightTNoise,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_TNOISE,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightSNoise, 0, mTestConfig.lightSNoise,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_SNOISE,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDataNoiseFlatLPF, 0, mTestConfig.dataNoiseFlatLPF,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_FLAT_SNOISE_LPF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nUnorSignal_LPF, mTestConfig.unorSignalLPF, 0,
                                            sensorNum, 256.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_UNOR_SIGNAL_LPF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nSignal_LPF, mTestConfig.signalLPF, 0,
                                            sensorNum, 256.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SIGNAL_LPF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nSSNR_LPF, mTestConfig.ssnrLPF, 0,
                                            sensorNum, 256.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SSNR_LPF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nShapeness_LPF, mTestConfig.sharpnessLPF, 0,
                                            sensorNum, (double) (1<<20), resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SHARPNESS_LPF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nTSNR, mTestConfig.tSNR, 0,
                                            sensorNum, 256.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_TSNR,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMaxTNoise, 0, mTestConfig.maxTNoise,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MAX_T_NOISE,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nHAFBadPointNum, 0, mTestConfig.maxHAFBadPointNum,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_HAF_BAD_POINT_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nHAFBadBlockNum, 0, mTestConfig.maxHAFBadBlockNum,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_HAF_BAD_BLOCK_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nBWhitePixelNum, 0, mTestConfig.whitePixelNum,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_BWHITE_PIXEL_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nBBlackPixelNum, 0, mTestConfig.blackPixelNum,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_BBLACK_PIXEL_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightLeakRatio, 0, mTestConfig.lightLeakRatio,
                                            sensorNum, 256.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_LEAK_RATIO,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDpBadPointNum, 0, mTestConfig.dpBadPointNum,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DP_BAD_POINT_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDpMaxBpnInRow, 0, mTestConfig.dpMaxBpnInRow,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DP_MAX_BPN_IN_ROW,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDpMeanDiff, 0, mTestConfig.dpMeanDiff,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DP_MEAN_DIFF,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDPSNoiseDark, 0, mTestConfig.dPSNoiseDark,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DP_SNOISE_DARK,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDPSNoiseLight, 0, mTestConfig.dPSNoiseLight,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DP_SNOISE_LIGHT,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nDirectionOffset, 0, mTestConfig.maxChartDirection,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_CHART_DIRECTION,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMinDiffFleshHM, mTestConfig.minDiffFleshHM, 0,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_FLESH_HM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMinDiffFleshML, mTestConfig.minDiffFleshML, 0,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_FLESH_ML,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMinDiffBlackHM, mTestConfig.minDiffBlackHM, 0,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_BLACK_HM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMinDiffBlackML, mTestConfig.minDiffBlackML, 0,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_BLACK_ML,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMaxDiffOffset, 0, mTestConfig.maxDiffOffset,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MAX_DIFF_OFFSET,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                if (resultAndTh != NULL) {
                    delete []resultAndTh;
                }
            } else if (parent_result->i_collect_phase == OPERATION_STEP_GET_KB_CALIBRATION) {
                float* resultAndTh = new float[resultAndThSize / sizeof(float)] { 0 };
                cal_cmd = (gf_delmar_calculate_cmd_t *)cmd;
                cal_result = &cal_cmd->o_result;
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, parent_result->i_collect_phase);
                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nBadPointNum, 0, mTestConfig.badPointNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_BAD_POINT_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nClusterNum, 0, mTestConfig.clusterNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_CLUSTER_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nPixelOfLargestBadCluster, 0, mTestConfig.pixelOfLargestBadCluster,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LARGEST_BAD_CLUSTER,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nBpnInClusters, 0, mTestConfig.bpnInClusters,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_BPN_IN_CLUSTER,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightHBadLineNum, 0, mTestConfig.lightHBadLineNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_HBAD_LINE_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nLightVBadLineNum, 0, mTestConfig.lightVBadLineNum,
                                                 sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_VBAD_LINE_NUM,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMaxDiffOffset, 0, mTestConfig.maxDiffOffset,
                                            sensorNum, 1.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MAX_DIFF_OFFSET,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                PACKAGE_ARRAY_AND_THRESHOLD(cal_result->nMaxTNoise, 0, mTestConfig.maxTNoise,
                                            sensorNum, 1024.0, resultAndTh);
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_MAX_T_NOISE,
                                                     (int8_t *)resultAndTh, resultAndThSize);

                if (resultAndTh != NULL) {
                    delete []resultAndTh;
                }
            } else {
            if ((parent_result->i_collect_phase >= OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT) && 
               (parent_result->i_collect_phase <= OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN)) {
               parent_result->i_collect_phase -= 5;
           }
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, parent_result->i_collect_phase);
                if (parent_result->i_collect_phase == OPERATION_STEP_CALCULATE_GAIN) {
                    cal_cmd = (gf_delmar_calculate_cmd_t *)cmd;
                    LOG_D(LOG_TAG, "[%s] pga gain=%d", __func__, cal_cmd->o_pga_gain[0]);
                    current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_PGA_GAIN,
                                            (int8_t *)cal_cmd->o_pga_gain, sensorNum * sizeof(uint8_t));
                }
            }

            TestUtils::testMemoryCheck(__func__, test_result, current, len);
        } else {
            len = 0;
        }

        notifyTestCmd(0, PRODUCT_TEST_CMD_PERFORMANCE_TESTING, test_result, len);

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    void DelmarProductTest::notifyLocationCircleImage(gf_error_t err, void *cmd) {
        int8_t *test_result = NULL;
        uint32_t len = 0;
        uint32_t array_len = LOCATION_CIRCLE_PIC_BUFFER_LEN;
        // gf_test_location_circle_image_t* location_circle = NULL;
        gf_cmd_header_t *header = (gf_cmd_header_t *)cmd;
        gf_delmar_calculate_cmd_t *cal_result = NULL;
        VOID_FUNC_ENTER();

        if (GF_CMD_TEST_LOCATION_CIRCLE_TESTING == header->cmd_id) {
            // location_circle = (gf_test_location_circle_image_t*) result;
        } else if (GF_CMD_TEST_PERFORMANCE_TESTING == header->cmd_id) {
            cal_result = (gf_delmar_calculate_cmd_t *) cmd;
        }

        // error code
        len += HAL_TEST_SIZEOF_INT32;
        // collect_phase
        len += HAL_TEST_SIZEOF_INT32;
        // location circle image
        len += HAL_TEST_SIZEOF_ARRAY(array_len);
        LOG_D(LOG_TAG, "[%s] cmd_id=%d", __func__, header->cmd_id);
        LOG_D(LOG_TAG, "[%s] len=%d, array_len=%d", __func__, len, array_len);
        test_result = new int8_t[len] { 0 };

        if (test_result != NULL) {
            int8_t *current = test_result;
            current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE, err);

            if (GF_CMD_TEST_LOCATION_CIRCLE_TESTING == header->cmd_id) {
                // current = TestUtils::testEncodeInt32(current, TEST_PARAM_TOKEN_LOCATION_CIRCLE_TEST_COLLECT_PHASE, location_circle->nOperationStep);
                // current = TestUtils::testEncodeArray(current, TEST_TOKEN_LOCATION_CIRCLE_IMAGE_DATA, location_circle->location_circle_data, array_len);
            } else if (GF_CMD_TEST_PERFORMANCE_TESTING == header->cmd_id) {
                current = TestUtils::testEncodeInt32(current,
                                                     PRODUCT_TEST_TOKEN_COLLECT_PHASE,
                                                     cal_result->cmd_base.i_collect_phase);
                /*
                current = TestUtils::testEncodeArray(current,
                                                     TEST_TOKEN_PERFORMANCE_TEST_LOCATION_CIRCLE_IMAGE_DATA,
                                                     (int8_t *)cal_result->o_result.location_circle_data, array_len);
                */
            }

            TestUtils::testMemoryCheck(__func__, test_result, current, len);
        } else {
            len = 0;
        }

        notifyTestCmd(0, PRODUCT_TEST_CMD_PERFORMANCE_TESTING, test_result, len);

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    void DelmarProductTest::notifyLocationCircleTesting(gf_error_t err, void *cmd) {
        UNUSED_VAR(err);
        UNUSED_VAR(cmd);
    }

    gf_error_t DelmarProductTest::testSpi() {
        gf_error_t err = GF_SUCCESS;
        int8_t *test_result = NULL;
        uint32_t len = 0;
        gf_delmar_test_spi_cmd_t *cmd = NULL;
        uint32_t size = sizeof(gf_delmar_test_spi_cmd_t);
        uint32_t sensorNum = ((DelmarSensor*) mContext->mSensor)->getAvailableSensorNum();
        FUNC_ENTER();

        do {
            cmd = new gf_delmar_test_spi_cmd_t();

            if (NULL == cmd) {
                LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(cmd, 0, size);
            cmd->header.target = GF_TARGET_PRODUCT_TEST;
            cmd->header.cmd_id = GF_CMD_TEST_SPI;
            err = invokeCommand(cmd, sizeof(gf_delmar_test_spi_cmd_t));
            if (err != GF_SUCCESS) {
                err = GF_ERROR_SPI_TEST;
            }
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // chip type
            len += HAL_TEST_SIZEOF_INT32;
            // chip series
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t mcu_chip_id;
            len += HAL_TEST_SIZEOF_INT32;
            // uint16_t * sensorNum sensor_chip_id;
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int16_t));
            // uint16_t flash_id;
            len += HAL_TEST_SIZEOF_INT32;
            // uint16_t * sensorNum random_num;
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int16_t));
            test_result = new int8_t[len] { 0 };

            if (NULL == test_result) {
                len = 0;
                LOG_E(LOG_TAG, "[%s] test_result out of memory", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(test_result, 0, len);
            int8_t *current = test_result;
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_CHIP_TYPE,
                                                 mContext->mSensorInfo.chip_type);
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_CHIP_SERIES,
                                                 mContext->mSensorInfo.chip_series);
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_MCU_CHIP_ID,
                                                 (int32_t) cmd->o_mcu_chip_id);
            current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_SENSOR_CHIP_ID,
                                                 (int8_t *) cmd->o_sensor_chip_id, sensorNum * sizeof(int16_t));
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FLASH_CHIP_ID,
                                                 (int32_t)cmd->o_flash_id);
            current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RANDOM_NUM,
                                                 (int8_t *) cmd->o_random_number, sensorNum * sizeof(int16_t));
            TestUtils::testMemoryCheck(__func__, test_result, current, len);
        } while (0);

        if (NULL != cmd) {
            LOG_D(LOG_TAG,
                  "[%s] mcu_chip_id=0x%04X, sensor_chip_id=0x%04X, flash_id=0x%04X, product_id=0x%04X",
                  __func__, cmd->o_mcu_chip_id, cmd->o_sensor_chip_id[0], cmd->o_flash_id,
                  cmd->o_random_number[0]);
        }
        notifyTestCmd(0, PRODUCT_TEST_CMD_SPI, test_result, len);
        /*gf_delmar_sensor_info_t sensor_info[4];
        memset(sensor_info, 0, 4);
        getSensorInfo(&sensor_info[0], 4*sizeof(gf_delmar_sensor_info_t));
         LOG_D(LOG_TAG,"[%s]sensor_info 0: 0x%x, 0x%x, 0x%x", __func__, *(uint32_t*)(&sensor_info[0].o_sensor_id[0]), *(uint32_t*)(&sensor_info[0].o_sensor_id[4]),
            *(uint32_t*)(&sensor_info[0].o_sensor_id[8]));
         LOG_D(LOG_TAG,"[%s]sensor_info 1: 0x%x, 0x%x, 0x%x", __func__, *(uint32_t*)(&sensor_info[1].o_sensor_id[0]), *(uint32_t*)(&sensor_info[1].o_sensor_id[4]),
            *(uint32_t*)(&sensor_info[1].o_sensor_id[8]));
         LOG_D(LOG_TAG,"[%s]sensor_info 2: 0x%x, 0x%x, 0x%x", __func__, *(uint32_t*)(&sensor_info[2].o_sensor_id[0]), *(uint32_t*)(&sensor_info[2].o_sensor_id[4]),
            *(uint32_t*)(&sensor_info[2].o_sensor_id[8]));
         LOG_D(LOG_TAG,"[%s]sensor_info 3: 0x%x, 0x%x, 0x%x", __func__, *(uint32_t*)(&sensor_info[3].o_sensor_id[0]), *(uint32_t*)(&sensor_info[3].o_sensor_id[4]),
            *(uint32_t*)(&sensor_info[3].o_sensor_id[8]));*/
        if (cmd != NULL) {
            delete cmd;
        }

        if (test_result != NULL) {
            delete []test_result;
        }

        FUNC_EXIT(err);
        return err;
    }

    void DelmarProductTest::resetInterruptPinTimerThread(union sigval v) {
        VOID_FUNC_ENTER();
        DelmarProductTest *pt = (DelmarProductTest *) v.sival_ptr;

        if (pt != NULL) {
            pt->testResetInterruptPinFinished(GF_ERROR_RST_INT_TEST);
        } else {
            LOG_E(LOG_TAG, "[%s] sival_ptr is null", __func__);
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t DelmarProductTest::onEvent(gf_event_type_t e) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (EVENT_IRQ_RESET == e) {
                LOG_D(LOG_TAG, "[%s] receive irq", __func__);
                testResetInterruptPinFinished(GF_SUCCESS);
            } else {
                LOG_E(LOG_TAG, "[%s] ignore event<%d>", __func__, e);
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::testResetInterruptPin() {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_rst_int_pin_cmd_t *cmd = NULL;
        uint32_t size = sizeof(gf_delmar_rst_int_pin_cmd_t);
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        FUNC_ENTER();

        do {
            err = sensor->wakeupSensor();
            if (err != GF_SUCCESS) {
                break;
            }

            cmd = new gf_delmar_rst_int_pin_cmd_t();

            if (NULL == cmd) {
                LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(cmd, 0, size);
            cmd->header.target = GF_TARGET_PRODUCT_TEST;
            cmd->header.cmd_id = GF_CMD_TEST_RESET_INTERRUPT_PIN;
            cmd->i_step = MCU_RESET_STEP;
            err = invokeCommand(cmd, size);

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] test reset and interrupt pin between mcu and sensor fail",
                      __func__);
                break;
            }

            mContext->mCenter->registerHandler(this);
            mTestingRstIntPin = true;
            // hard reset
            {  // NOLINT(660)
                sensor->setNotifyRstFlag(0);
                AutoMutex _l(mContext->mSensorLock);
                mContext->mDevice->reset();
            }

            if (mpRstIntPinTimer != NULL) {
                delete mpRstIntPinTimer;
                mpRstIntPinTimer = NULL;
            }

            mpRstIntPinTimer = Timer::createTimer((timer_thread_t)
                                                  resetInterruptPinTimerThread, this);

            if (NULL == mpRstIntPinTimer) {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] create timer fail", __func__);
                break;
            }

            (void)mpRstIntPinTimer->set(5, 0, 5, 0);
            // interact with mcu
            memset(cmd, 0, size);
            cmd->header.target = GF_TARGET_PRODUCT_TEST;
            cmd->header.cmd_id = GF_CMD_TEST_RESET_INTERRUPT_PIN;
            cmd->i_step = MCU_WAIT_READY_STEP;
            err = invokeCommand(cmd, sizeof(gf_delmar_rst_int_pin_cmd_t));

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] hard reset fail", __func__);
                LOG_E(LOG_TAG, "[%s] RST/INT test fail", __func__);
            }
            LOG_D(LOG_TAG, "[%s] check mcu ready", __func__);
        } while (0);

        if (err != GF_SUCCESS) {
            err = GF_ERROR_RST_INT_TEST;
            mTestingRstIntPin = true;
            testResetInterruptPinFinished(err);
        } else {
            // success will be notified if irq_reset received and timer is not timed out
        }

        if (cmd != NULL) {
            delete cmd;
        }

        FUNC_EXIT(err);
        return err;
    }

    void DelmarProductTest::notifyResetInterruptPin(gf_error_t err) {
        VOID_FUNC_ENTER();
        int8_t *test_result = NULL;
        uint32_t len = 0;
        // chip type
        len += HAL_TEST_SIZEOF_INT32;
        // chip series
        len += HAL_TEST_SIZEOF_INT32;
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        test_result = new int8_t[len];

        if (test_result != NULL) {
            memset(test_result, 0, len);
            int8_t *current = test_result;
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_CHIP_TYPE,
                                                 mContext->mSensorInfo.chip_type);
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_CHIP_SERIES,
                                                 mContext->mSensorInfo.chip_series);
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
            TestUtils::testMemoryCheck(__func__, test_result, current, len);
        } else {
            len = 0;
        }

        notifyTestCmd(0, PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN, test_result, len);

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    void DelmarProductTest::testResetInterruptPinFinished(gf_error_t result) {
        VOID_FUNC_ENTER();
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        AutoMutex _l(mRstIntPinMutex);
        LOG_D(LOG_TAG, "[%s] result=%d", __func__, result);
        sensor->setNotifyRstFlag(1);
        sensor->sleepSensor();
        if (mTestingRstIntPin) {
            notifyResetInterruptPin(result);
            mTestingRstIntPin = false;

            if (mpRstIntPinTimer != NULL) {
                delete mpRstIntPinTimer;
                mpRstIntPinTimer = NULL;
            }

            mContext->mCenter->deregisterHandler(this);
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t DelmarProductTest::testOTPFLash() {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_test_otp_flash_cmd_t cmd = { 0 };
        int8_t *test_result = NULL;
        uint32_t len = 0;
        FUNC_ENTER();

        do {
            cmd.target = GF_TARGET_PRODUCT_TEST;
            cmd.cmd_id = GF_CMD_TEST_OTP_FLASH;
            err = invokeCommand(&cmd, sizeof(gf_delmar_test_otp_flash_cmd_t));
            if (err != GF_SUCCESS) {
                err = GF_ERROR_OTP_FLASH_TEST;
            }
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // chip type
            len += HAL_TEST_SIZEOF_INT32;
            // chip series
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (NULL == test_result) {
                len = 0;
                LOG_E(LOG_TAG, "[%s] test_result out of memory", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(test_result, 0, len);
            int8_t *current = test_result;
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_CHIP_TYPE,
                                                 mContext->mSensorInfo.chip_type);
            current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_CHIP_SERIES,
                                                 mContext->mSensorInfo.chip_series);
            TestUtils::testMemoryCheck(__func__, test_result, current, len);
        } while (0);

        notifyTestCmd(0, PRODUCT_TEST_CMD_OTP_FLASH, test_result, len);

        if (test_result != NULL) {
            delete []test_result;
        }

        FUNC_EXIT(err);
        return err;
    }

    bool DelmarProductTest::isNeedLock(int32_t cmdId) {
        UNUSED_VAR(cmdId);
        return true;
    }

    gf_error_t DelmarProductTest::getSensorInfo(gf_delmar_sensor_info_t* sensor_info, uint32_t sensor_info_cnt) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_get_sensor_info_cmd_t get_cmd = {{0}};
        DelmarSensor *sensor = (DelmarSensor *) mContext->mSensor;
        uint32_t sensorNum = sensor->getAvailableSensorNum();

        FUNC_ENTER();
        do {
            if (NULL == sensor_info) {
                LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            if (sensor_info_cnt > sensorNum) {
                LOG_E(LOG_TAG, "[%s] too many sensor num", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            get_cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            get_cmd.cmd_header.cmd_id = GF_CMD_TEST_GET_SENSOR_INFO;
            err = invokeCommand(&get_cmd, sizeof(gf_delmar_get_sensor_info_cmd_t));
            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] get chip info failed", __func__);
                break;
            }
            uint32_t sensorIndex = 0;
            for (sensorIndex = 0; sensorIndex < sensorNum; sensorIndex++) {
                if(sensorIndex >= MAX_SENSOR_NUM) {
                    break;
                }
                memcpy(&sensor_info[sensorIndex], &get_cmd.chip_info[sensorIndex], sizeof(gf_delmar_sensor_info_t));
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::testSetGainTargetValue(const int8_t *in, uint32_t inLen) {
        gf_error_t err = GF_SUCCESS;
        uint32_t token = 0;
        int8_t *test_result = NULL;
        int8_t *current = NULL;
        uint32_t len = 0;
        gf_delmar_gain_target_cmd_t cmd = {{0}};
        FUNC_ENTER();
        do {
            const int8_t *in_buf = in;
            if (NULL != in_buf) {
                do {
                    in_buf = TestUtils::testDecodeUint32(&token, in_buf);
                    switch (token) {
                        case PRODUCT_TEST_TOKEN_GAIN_TARGET: {
                            in_buf = TestUtils::testDecodeUint32((uint32_t*)&cmd.io_value, in_buf);
                            break;
                        }
                        default: {
                            LOG_D(LOG_TAG, "[%s] invilid token", __func__);
                            break;
                        }
                    }
                } while (in_buf < in + inLen);
            } else {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            cmd.cmd_header.cmd_id = GF_CMD_TEST_GAIN_TARGET;
            cmd.flag = 1;
            LOG_D(LOG_TAG, "[%s] gain target:%d", __func__, cmd.io_value);
            err = invokeCommand(&cmd, sizeof(gf_delmar_gain_target_cmd_t));
            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] set gain target value failed", __func__);
                break;
            }
        } while (0);

        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        test_result = new int8_t[len] { 0 };

        current = test_result;
        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
        TestUtils::testMemoryCheck(__func__, test_result, current, len);
        notifyTestCmd(0, PRODUCT_TEST_CMD_SET_GAIN_TARGET, test_result, len);
        delete []test_result;

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarProductTest::testGetGainTargetValue() {
        gf_error_t err = GF_SUCCESS;
        int8_t *test_result = NULL;
        int8_t *current = NULL;
        uint32_t len = 0;
        gf_delmar_gain_target_cmd_t cmd = {{0}};
        FUNC_ENTER();
        do {
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            cmd.cmd_header.cmd_id = GF_CMD_TEST_GAIN_TARGET;
            cmd.flag = 0;
            err = invokeCommand(&cmd, sizeof(gf_delmar_gain_target_cmd_t));
            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] set gain target value failed", __func__);
                break;
            }
        } while (0);

        LOG_D(LOG_TAG, "[%s] gain target:%d", __func__, cmd.io_value);
        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // gain target value
        len += HAL_TEST_SIZEOF_INT32;
        test_result = new int8_t[len] { 0 };

        current = test_result;
        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_GAIN_TARGET, cmd.io_value);
        TestUtils::testMemoryCheck(__func__, test_result, current, len);
        notifyTestCmd(0, PRODUCT_TEST_CMD_GET_GAIN_TARGET, test_result, len);
        delete []test_result;

        FUNC_EXIT(err);
        return err;
    }

    void DelmarProductTest::notifyResetStatus(gf_error_t err, int32_t cmdId) {
        int8_t *test_result = NULL;
        uint32_t len = 0;
        int8_t *current = NULL;
        VOID_FUNC_ENTER();
        // error code
        len += HAL_TEST_SIZEOF_INT32;
        test_result = new int8_t[len] { 0 };
        current = test_result;
        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
        TestUtils::testMemoryCheck(__func__, test_result, current, len);
        LOG_D(LOG_TAG, "[%s] notifyResetStatus:err =%d", __func__, err);
        notifyTestCmd(0, cmdId, test_result, len);
        delete []test_result;
        VOID_FUNC_EXIT();
    }
}  // namespace goodix
