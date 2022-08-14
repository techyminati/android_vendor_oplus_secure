/************************************************************************************
 ** File: - CustomizedDelmarProductTest.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix(android Q)
 **
 ** Version: 1.0
 ** Date created: 15:09:11,15/08/2019
 ** Author: Bangxiong.Wu@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Bangxiong.Wu       2019/10/12       create file
 **  Bangxiong.Wu       2019/10/12       add lcdtype CC161_SAMSUNG for SM8250
 ************************************************************************************/

#define LOG_TAG "[GF_HAL][CustomizedDelmarProductTest]"

#include "CustomizedDelmarProductTest.h"
#include "HalLog.h"
#include "HalUtils.h"
#include "HalContext.h"
#include "CustomizedHalConfig.h"
#include "CustomizedSensor.h"
#include "gf_customized_types.h"
#include "TestUtils.hpp"

#define ABS(x) ((x) > 0 ? (x) : -(x))
#define GF_DUMP_FILE_PATH_MAX_LEN  256
#define GF_NEGATIVE_BREAK(ret) { if ((ret) < 0) {break;} }
#define PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(buf, len, title, format1, array, format2, threshold, onlyOneItem, denominator) \
    do { \
        char *ptr = (buf); \
        int returnVal = 0; \
        uint32_t restLen = (uint32_t) (len); \
        returnVal = snprintf(ptr, restLen, "%s,", (title)); \
        GF_NEGATIVE_BREAK(returnVal); \
        ptr += returnVal; \
        restLen -= returnVal; \
        for (uint32_t x = 0; x < sensorNum; x++) { \
            returnVal = snprintf(ptr, restLen, format1 "(" format2 "),", \
                ((onlyOneItem) ? (array)[0] : (array)[x]) / (denominator), threshold); \
            GF_NEGATIVE_BREAK(returnVal); \
            ptr += returnVal; \
            restLen -= returnVal; \
        } \
        GF_NEGATIVE_BREAK(returnVal); \
        returnVal = snprintf(ptr, restLen, "%s", "\n"); \
    } while (0)

namespace goodix {
    CustomizedDelmarProductTest::CustomizedDelmarProductTest(HalContext *context)
        : DelmarProductTest(context),
        mTestResetInterruptResult(GF_SUCCESS) {
        mContext = context;
        CustomizedSensor *sensor = (CustomizedSensor*) context->mSensor;
        mProductScreenId = sensor->getProductScreenId();
        gf_delmar_sensor_type_t sensorType = ((DelmarSensor*) context->mSensor)->getSenosrType();
        uint8_t opticalType = ((DelmarSensor*) context->mSensor)->getOpticalType();
        uint32_t sensorBgVersion = ((DelmarSensor*) context->mSensor)->getSensorBgVersion();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d, sensorType=%d, opticalType=%d, sensorBgVersion=%d", __func__,
                                                         mProductScreenId, sensorType, opticalType, sensorBgVersion);
        if (sensorType == DELMAR_SENSOR_TYPE_SINGLE_T_SE2 || sensorType == DELMAR_SENSOR_TYPE_SINGLE_T_SE5) {
            setThresholdNewFlow(opticalType, sensorBgVersion);
        } else {
            setThresholdOldFlow();
        }
        DelmarProductTest::onInitFinished();
    }

    void CustomizedDelmarProductTest::setThresholdNewFlow(uint8_t opticalType, uint32_t bgVersion) {
        VOID_FUNC_ENTER();
        if (opticalType == DELMAR_OPTICAL_TYPE_3_0) {
            // G6 op3.0
            mNewTestConfig.maxBadPixelNum = 200;
            mNewTestConfig.maxBadBlockNum = 14;
            mNewTestConfig.maxBadBlockLargest = 67;
            mNewTestConfig.maxBadPixelAllBlocks = 87;
            mNewTestConfig.maxBadLineNum = 0;
            mNewTestConfig.maxHotPixelLowNum = 100;
            mNewTestConfig.maxHotLineNum = 0;
            mNewTestConfig.maxHotBlockLargest = 17;
            mNewTestConfig.maxDPBadPixelNum = 50;
            mNewTestConfig.maxDPBadPixelInRow = 8;
            mNewTestConfig.maxDPDiffMean = 10;
            mNewTestConfig.maxDPSNoiseDarkN = 5.1;
            mNewTestConfig.maxDPAADiffDark = 22;
            mNewTestConfig.maxHAFBadPixelNum = 68;
            mNewTestConfig.maxHAFBadBlockNum = 11;
            mNewTestConfig.maxTNoiseDarkN = 1.2;
            mNewTestConfig.maxTNoiseLightN = 10;
            mNewTestConfig.maxSNoiseDarkN = 1.6;
            mNewTestConfig.maxSNoiseLightN = 120;
            mNewTestConfig.minLightHighMean = 1000;
            mNewTestConfig.maxLightHighMean = 3000;
            mNewTestConfig.maxLightLeakRatio = 0.75;
            mNewTestConfig.minSignalL = 26;
            mNewTestConfig.maxSNoiseFlatL = 5;
            mNewTestConfig.minTSNR = 11;
            mNewTestConfig.minSharpnessL = 0.0;
            mNewTestConfig.maxAssemblyAngleOffset = 2.5;
            mNewTestConfig.maxAssemblyXOffset = 12.0;
            mNewTestConfig.maxAssemblyYOffset = 12.0;
            mNewTestConfig.minDiffFleshHM = 200;
            mNewTestConfig.minDiffFleshML = 100;
            mNewTestConfig.minDiffBlackHM = 100;
            mNewTestConfig.minDiffBlackML = 50;
            mNewTestConfig.maxDiffOffset = 1500;
            mNewTestConfig.maxLightStability = 10;
            mNewTestConfig.maxLightLeakDark = 30;
            mNewTestConfig.maxLowCorrPitch = 100;
            mNewTestConfig.minValidAreaRatio = 0.8;
            mNewTestConfig.maxChartDirection = 15;
            mNewTestConfig.standardAngle = 96.5;
            mNewTestConfig.standardCenterX = 60;
            mNewTestConfig.standardCenterY = 60;
        } else if (opticalType == DELMAR_OPTICAL_TYPE_7_0) {
            //  G6 op7.0 green bottom
            mNewTestConfig.maxBadPixelNum = 150;
            mNewTestConfig.maxBadBlockNum = 10;
            mNewTestConfig.maxBadBlockLargest = 67;
            mNewTestConfig.maxBadPixelAllBlocks = 87;
            mNewTestConfig.maxBadLineNum = 0;
            mNewTestConfig.maxHotPixelLowNum = 100;
            mNewTestConfig.maxHotLineNum = 0;
            mNewTestConfig.maxHotBlockLargest = 17;
            mNewTestConfig.maxDPBadPixelNum = 50;
            mNewTestConfig.maxDPBadPixelInRow = 8;
            mNewTestConfig.maxDPDiffMean = 10;
            mNewTestConfig.maxDPSNoiseDarkN = 5.1;
            mNewTestConfig.maxDPAADiffDark = 22;
            mNewTestConfig.maxHAFBadPixelNum = 90;
            mNewTestConfig.maxHAFBadBlockNum = 15;

            mNewTestConfig.maxAntiFakeDx = 30;
            mNewTestConfig.minAntiFakeDx = 6;
            mNewTestConfig.maxAntiFakeDy = 30;
            mNewTestConfig.minAntiFakeDy = 6;

            mNewTestConfig.maxTNoiseDarkN = 1.5;
            mNewTestConfig.maxTNoiseLightN = 20;
            mNewTestConfig.maxSNoiseDarkN = 1.6;
            mNewTestConfig.maxSNoiseLightN = 120;
            mNewTestConfig.minLightHighMean = 1000;
            mNewTestConfig.maxLightHighMean = 3000;
            mNewTestConfig.maxLightLeakRatio = 0.75;
            mNewTestConfig.minPolarDegree = 1.35;
            if (bgVersion == 0) {
                mNewTestConfig.minSignalLLow = 19.94;
                mNewTestConfig.minSignalLHigh = 33.50;
            } else {
                mNewTestConfig.minSignalLLow = 31.56;
                mNewTestConfig.minSignalLHigh = 53.04;
            }

            mNewTestConfig.maxSNoiseFlatL = 5;
            if (bgVersion == 0) {
                mNewTestConfig.minTSNRLow = 12.76;
                mNewTestConfig.minTSNRHigh = 19.59;
            } else {
                mNewTestConfig.minTSNRLow = 15.54;
                mNewTestConfig.minTSNRHigh = 23.85;
            }
            mNewTestConfig.minSharpnessL = 0.0;
            mNewTestConfig.maxAssemblyAngleOffset = 7.5;
            mNewTestConfig.maxAssemblyXOffset = 11.0;
            mNewTestConfig.maxAssemblyYOffset = 11.0;
            mNewTestConfig.minDiffFleshHM = 200;
            mNewTestConfig.minDiffFleshML = 100;
            mNewTestConfig.minDiffBlackHM = 100;
            mNewTestConfig.minDiffBlackML = 50;
            mNewTestConfig.maxDiffOffset = 1500;
            mNewTestConfig.maxLightStability = 20;
            mNewTestConfig.maxLightLeakDark = 30;
            mNewTestConfig.maxLowCorrPitch = 100;
            mNewTestConfig.minValidAreaRatio = 0.8;
            mNewTestConfig.maxChartDirection = 15;
            mNewTestConfig.maxChartGhostShadow = 20;
            mNewTestConfig.maxChartDirty = 0;
            mNewTestConfig.standardAngle = 90.0;
            mNewTestConfig.standardCenterX = 60;
            mNewTestConfig.standardCenterY = 60;
        }

        if (mProductScreenId == SCREEN_TYPE_AE009_SAMSUNG_ID ||
            mProductScreenId == SCREEN_TYPE_AE009_BOE_ID ||
            mProductScreenId == SCREEN_TYPE_AD119_SAMSUNG_ID) {
            mNewTestConfig.maxChartGhostShadow = 10000;   // set bigest for not check
            mNewTestConfig.maxChartDirty = 10000;   // set bigest for not check
        }
        VOID_FUNC_EXIT();
    }

    void CustomizedDelmarProductTest::setThresholdOldFlow() {
        VOID_FUNC_ENTER();
        // G5
                mTestConfig.maxBadPointNum = 300;
                mTestConfig.maxClusterNum = 20;
                mTestConfig.maxPixelOfLargestBadCluster = 100;
                mTestConfig.maxBpnInClusters = 130;
                mTestConfig.maxLightHBadLineNum = 0;
                mTestConfig.maxLightVBadLineNum = 0;
                mTestConfig.maxBBlackPixelNum = 150;
                mTestConfig.maxBWhitePixelNumLow = 150;
                mTestConfig.maxHotConnected = 25;
                mTestConfig.maxDPBadPointNum = 50;
                mTestConfig.maxDPBpnInRow = 8;
                mTestConfig.maxDPDiffMean = 10;
                mTestConfig.maxDPSNoiseDark = 13;
                mTestConfig.maxDPAADarkDiff = 22;
                mTestConfig.maxHAFBadPointNum = 45;
                mTestConfig.maxHAFBadBlockNum = 7;
                mTestConfig.maxTNoiseDark = 3;
                mTestConfig.maxTNoiseLight = 8;
                mTestConfig.maxSNoiseDark = 4;
                mTestConfig.maxSNoiseLight = 300;
                mTestConfig.minLightHighMean = 1400;
                mTestConfig.maxLightHighMean = 3000;
                mTestConfig.maxLightLeakRatio = 0.75;
                mTestConfig.minSignal = 20;
                mTestConfig.maxFlatSNoise = 5;
                mTestConfig.minTSNR = 8.5;
                mTestConfig.minSharpness = 0.35;
                mTestConfig.maxAssemblyAngle = 2.5;
                mTestConfig.maxCenterXOffset = 12.0;
                mTestConfig.maxCenterYOffset = 12.0;
                mTestConfig.minDiffFleshHM = 200;
                mTestConfig.minDiffFleshML = 100;
                mTestConfig.minDiffBlackHM = 100;
                mTestConfig.minDiffBlackML = 50;
                mTestConfig.maxDiffOffset = 1500;
                mTestConfig.maxLightStability = 10;
                mTestConfig.maxDarkLightLeak = 30;
                mTestConfig.maxLowCorrPitch = 100;
                mTestConfig.maxInValidArea = 30;
                mTestConfig.maxChartDirection = 15;
                mTestConfig.standardAngle = 96.5;
                mTestConfig.standardCenterX = 60;
                mTestConfig.standardCenterY = 90;
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID: {
                break;
            }
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                mTestConfig.minSignal = 26;
                mTestConfig.minTSNR = 11;
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d has not set threshold, use default threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        VOID_FUNC_EXIT();
    }

    CustomizedDelmarProductTest::~CustomizedDelmarProductTest() {
    }

    gf_error_t CustomizedDelmarProductTest::checkGapValueOfEachBrightness(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensor_num) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                err = DelmarProductTest::checkGapValueOfEachBrightness(cal_cmd, sensor_num);
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d have not checked threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::checkStabilityOfSwitchingBrightness(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        #ifndef BYPASS_BRIGHTNESSCHECK
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                err = DelmarProductTest::checkStabilityOfSwitchingBrightness(cal_cmd, sensorNum);
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d have not checked threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        #endif
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::checkChartDirection(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        int32_t tempDirection = 0;
        int32_t directionOffset = 0;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                    tempDirection = ABS(cal_cmd->o_result.nChartDirection[sensor_index]);
                    directionOffset = tempDirection;
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
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d have not checked threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::checkPerformanceThreshold(gf_calculate_cmd_result_t *cal_result) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                err = DelmarProductTest::checkPerformanceThreshold(cal_result);
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d have not checked threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::checkChipOffsetAngel(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                err = DelmarProductTest::checkChipOffsetAngel(cal_cmd, sensorNum);
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d have not checked threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::checkChipOffsetCoordinate(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                err = DelmarProductTest::checkChipOffsetCoordinate(cal_cmd, sensorNum);
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d have not checked threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::checkBadPoint(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        switch (mProductScreenId) {
            case SCREEN_TYPE_BD187_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_SAMSUNG_ID:
            case SCREEN_TYPE_AD097_BOE_ID:
            case SCREEN_TYPE_CC161_SAMSUNG_ID: {
                err = DelmarProductTest::checkBadPoint(cal_cmd, sensorNum);
                break;
            }
            default: {
                LOG_D(LOG_TAG, "[%s] mProductScreenId=%d have not checked threshold.",
                               __func__, mProductScreenId);
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::saveOldPerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold) {
        gf_error_t err = GF_SUCCESS;
        char file_path[GF_DUMP_FILE_PATH_MAX_LEN] = {0};
        Vector<uint8_t> buffer;
        FUNC_ENTER();

        do {
            gf_calculate_cmd_result_t * result = &(cal_cmd->o_result);
            if (NULL == result || NULL == threshold) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            buffer.clear();

            if (OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS == operationStep) {
                char line[1024] = { 0 };
                // SensorID
                strncat(line, "SensorID,", strlen("SensorID,"));
                for (int id = 0; id < (int32_t) sensorNum; id++) {
                    char sensor_id[DELMAR_SENSOR_OTP_INFO_LEN * 2 + 1] = { 0 };
                    for (int i = 0; i < DELMAR_SENSOR_OTP_INFO_LEN; i++) {
                        char subId[3] = { 0 };
                        snprintf(subId, sizeof(subId), "%02X", result->nSensorID[id][i]);
                        strncat(sensor_id, subId, sizeof(subId));
                    }
                    strncat(line, sensor_id, strlen(sensor_id));
                    strncat(line, ",", strlen(","));
                }
                strncat(line, "\n", strlen("\n"));
                buffer.appendArray((uint8_t *) line, strlen(line));

                // ScreenId
                snprintf(line, sizeof(line), "ScreenId,%d\n", mProductScreenId);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // BadPointNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BadPointNum", "%d",
                    result->nBadPointNum, "<=%d", threshold->maxBadPointNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // ClusterNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "ClusterNum", "%d",
                    result->nClusterNum, "<=%d", threshold->maxClusterNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // PixelOfLargestBadCluster
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "PixelOfLargestBadCluster", "%d",
                    result->nPixelOfLargestBadCluster, "<=%d", threshold->maxPixelOfLargestBadCluster, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // BpnInClusters
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BpnInClusters", "%d",
                    result->nBpnInClusters, "<=%d", threshold->maxBpnInClusters, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightHBadLineNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightHBadLineNum", "%d",
                    result->nLightHBadLineNum, "<=%d", threshold->maxLightHBadLineNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightVBadLineNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightVBadLineNum", "%d",
                    result->nLightVBadLineNum, "<=%d", threshold->maxLightVBadLineNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                /*
                // BBlackPixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BBlackPixelNum", "%d",
                    result->nBBlackPixelNum, "<=%d", threshold->maxBWhitePixelNumLow, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                */
                // BWhitePixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BWhitePixelNum", "%d",
                    result->nBWhitePixelNum, "<=%d", threshold->maxBWhitePixelNumLow, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // HotPixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HotPixelNum", "%d",
                    result->nHotPixelNum, "<=%d", threshold->maxHotConnected, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                /*
                // HotLineNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HotLineNum", "%d",
                    result->nHotLineNum, "<=%d", threshold->maxHotLineNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                */

                // DPBadPointNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPBadPointNum", "%d",
                    result->nDpBadPointNum, "<=%d", threshold->maxDPBadPointNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DPBpnInRow
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPBpnInRow", "%d",
                    result->nDpMaxBpnInRow, "<=%d", threshold->maxDPBpnInRow, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DpMeanDiff
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DpMeanDiff", "%d",
                    result->nDpMeanDiff, "<=%d", threshold->maxDPDiffMean, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DPSNoiseDark
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPSNoiseDark", "%.3f",
                    result->nDPSNoiseDark, "<=%d", threshold->maxDPSNoiseDark, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DPAADarkDiff
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPAADarkDiff", "%d",
                    result->nDPAADarkDiff, "<=%d", threshold->maxDPAADarkDiff, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // HAFBadPointNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HAFBadPointNum", "%d",
                    result->nHAFBadPointNum, "<=%d", threshold->maxHAFBadPointNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // HAFBadBlockNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HAFBadBlockNum", "%d",
                    result->nHAFBadBlockNum, "<=%d", threshold->maxHAFBadBlockNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // DarkTNoise
                memset(line, 0, sizeof(line));
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DarkTNoise", "%.4f",
                    result->nDarkTNoise, "<=%d", threshold->maxTNoiseDark, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightTNoise
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightTNoise", "%.4f",
                    result->nLightTNoise, "<=%d", threshold->maxTNoiseLight, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DarkSNoise
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DarkSNoise", "%.4f",
                    result->nDarkSNoise, "<=%d", threshold->maxSNoiseDark, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightSNoise
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightSNoise", "%.4f",
                    result->nLightSNoise, "<=%d", threshold->maxSNoiseLight, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // MinLightHighMean
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightHighMean", "%d",
                    result->nLightHighMean, ">=%d", threshold->minLightHighMean, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MaxLightHighMean
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightHighMean", "%d",
                    result->nLightHighMean, "<=%d", threshold->maxLightHighMean, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // LightLeakRatio
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightLeakRatio", "%.3f",
                    result->nLightLeakRatio, "<=%f", threshold->maxLightLeakRatio, false, 256.0);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // UnorSignal_LPF
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "UnorSignal_LPF", "%.4f",
                    result->nUnorSignal_LPF, ">=%d", threshold->minSignal, false, 256.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DataNoiseFlatLPF
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DataNoiseFlatLPF", "%.3f",
                    result->nDataNoiseFlatLPF, "<=%d", threshold->maxFlatSNoise, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // TSNR
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "TSNR", "%.4f",
                    result->nTSNR, ">=%f", threshold->minTSNR, false, 256.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // Sharpness_LPF
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "Sharpness_LPF", "%.4f",
                    (result->nShapeness_LPF), ">=%f", threshold->minSharpness, false, (double) (1<<20));
                buffer.appendArray((uint8_t *) line, strlen(line));

                // MaxAngletoChip
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "AngletoChip", "%.3f",
                    result->nAngelOffset, "<=%.2f", threshold->maxAssemblyAngle, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // CenterXtoChip
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "CenterXtoChip", "%.3f",
                    result->nCenterXOffset, "<=%.2f", threshold->maxCenterXOffset, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // CenterYtoChip
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "CenterYtoChip", "%.3f",
                    result->nCenterYOffset, "<=%.2f", threshold->maxCenterYOffset, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // MinDiffFleshHM
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffFleshHM", "%d",
                    result->nMinDiffFleshHM, ">=%d", threshold->minDiffFleshHM, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MinDiffFleshML
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffFleshML", "%d",
                    result->nMinDiffFleshML, ">=%d", threshold->minDiffFleshML, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MinDiffBlackHM
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffBlackHM", "%d",
                    result->nMinDiffBlackHM, ">=%d", threshold->minDiffBlackHM, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MinDiffBlackML
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffBlackML", "%d",
                    result->nMinDiffBlackML, ">=%d", threshold->minDiffBlackML, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MaxDiffOffset
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MaxDiffOffset", "%d",
                    result->nMaxDiffOffset, "<=%d", threshold->maxDiffOffset, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MaxTNoise
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightStability", "%.4f",
                    result->nMaxTNoise, "<%f", threshold->maxLightStability, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // maxDarkLightLeak/MtDarkOp
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DarkLightLeak", "%.4f",
                    result->nMtDarkOp, "<=%d", threshold->maxDarkLightLeak, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LowCorrPitch
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LowCorrPitch", "%d",
                    result->nLowCorrPitch_LPF, "<=%d", threshold->maxLowCorrPitch, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // InValidArea
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "InValidArea", "%d",
                    result->nInValidArea, "<=%d", threshold->maxInValidArea, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MaxChartDirection
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "ChartDirection", "%d",
                    result->nDirectionOffset, "<=%d", threshold->maxChartDirection, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                if (buffer.size() > 0) {
                    uint8_t * buffer_tmp = (uint8_t *)buffer.array();
                    uint32_t buffer_len = (uint32_t)buffer.size();
                    char file_name[GF_DUMP_FILE_PATH_MAX_LEN] = {0};
                    snprintf(file_name, GF_DUMP_FILE_PATH_MAX_LEN, "%s_err%05d_result_threshold.csv", timeStamp, cal_cmd->err_code);
                    snprintf(file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s/test", getDumpRootDir());

                    err = HalUtils::writeFile(file_path, file_name, buffer_tmp, buffer_len);
                }
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::saveNewPerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold_old) {
        gf_error_t err = GF_SUCCESS;
        char file_path[GF_DUMP_FILE_PATH_MAX_LEN] = {0};
        uint8_t opticalType = ((DelmarSensor*) mContext->mSensor)->getOpticalType();
        Vector<uint8_t> buffer;
        UNUSED_VAR(threshold_old);
        FUNC_ENTER();

        do {
            gf_new_product_report_result_t* result = &(cal_cmd->o_new_product_report_result);
            gf_delmar_new_product_test_threshold_t *threshold = &(cal_cmd->o_new_product_threshold);

            if (NULL == result || NULL == threshold) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            buffer.clear();

            if (OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS == operationStep) {
                char line[1024] = { 0 };
                // SensorID
                strncat(line, "SensorID,", strlen("SensorID,"));
                for (int id = 0; id < (int32_t) sensorNum; id++) {
                    char sensor_id[DELMAR_SENSOR_OTP_INFO_LEN * 2 + 1] = { 0 };
                    for (int i = 0; i < DELMAR_SENSOR_OTP_INFO_LEN; i++) {
                        char subId[3] = { 0 };
                        snprintf(subId, sizeof(subId), "%02X", result->nSensorID[id][i]);
                        strncat(sensor_id, subId, sizeof(subId));
                    }
                    strncat(line, sensor_id, strlen(sensor_id));
                    strncat(line, ",", strlen(","));
                }
                strncat(line, "\n", strlen("\n"));
                buffer.appendArray((uint8_t *) line, strlen(line));

                // ScreenId
                snprintf(line, sizeof(line), "ScreenId,%d\n", mProductScreenId);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // Bad-Pixel/Cluster
                // BadPixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BadPointNum", "%d",
                        result->nBadPixelNum, "<=%d", threshold->maxBadPixelNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // BadBlockNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "ClusterNum", "%d",
                        result->nBadBlockNum, "<=%d", threshold->maxBadBlockNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // BadBlockLargest
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "PixelOfLargestBadCluster", "%d",
                        result->nBadBlockLargest, "<=%d", threshold->maxBadBlockLargest, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // BadPixelAllBlocks
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BpnInClusters", "%d",
                        result->nBadPixelAllBlocks, "<=%d", threshold->maxBadPixelAllBlocks, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // BadLineNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightHVBadLineNum", "%d",
                        result->nBadLineNum, "<=%d", threshold->maxBadLineNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // HotPixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BWhitePixelNum", "%d",
                        result->nHotPixelNum, "<=%d", threshold->maxHotPixelLowNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // maxHotBlockLargest
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HotPixelNum", "%d",
                        result->nHotBlockLargest, "<=%d", threshold->maxHotBlockLargest, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // HotLineNum
                /*
                   PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HotLineNum", "%d",
                   result->nHotLineNum, "<=%d", threshold->maxHotLineNum, false, 1);
                   buffer.appendArray((uint8_t *) line, strlen(line));
                 */

                // DP
                // DPBadPointNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPBadPointNum", "%d",
                        result->nDPBadPixelNum, "<=%d", threshold->maxDPBadPixelNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DPBpnInRow
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPBpnInRow", "%d",
                        result->nDPBadPixelInRow, "<=%d", threshold->maxDPBadPixelInRow, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DPSNoiseDark
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPSNoiseDark", "%.3f",
                        result->nDPSNoiseDarkN, "<=%f", threshold->maxDPSNoiseDarkN, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DPAADarkDiff
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DPAADarkDiff", "%d",
                        result->nDPAADiffDark, "<=%d", threshold->maxDPAADiffDark, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // HAF
                // HAFBadPointNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HAFBadPointNum", "%d",
                        result->nHAFBadPixelNum, "<=%d", threshold->maxHAFBadPixelNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // HAFBadBlockNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HAFBadBlockNum", "%d",
                        result->nHAFBadBlockNum, "<=%d", threshold->maxHAFBadBlockNum, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                if (opticalType == DELMAR_OPTICAL_TYPE_7_0) {
                    // AF
                    for (uint32_t i = 0; i < sensorNum && i < MAX_SENSOR_NUM; i++) {
                        result->nAntiFakeDx[i] = ABS(result->nAntiFakeDx[i]);
                        result->nAntiFakeDy[i] = ABS(result->nAntiFakeDy[i]);
                    }
                    // AntiFakeDx
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "AntiFakeDx", "%.4f",
                        result->nAntiFakeDx, "<=%d", threshold->maxAntiFakeDx, false, 1024.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "AntiFakeDx", "%.4f",
                        result->nAntiFakeDx, ">=%d", threshold->minAntiFakeDx, false, 1024.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                    // AntiFakeDy
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "AntiFakeDy", "%.4f",
                        result->nAntiFakeDy, "<=%d", threshold->maxAntiFakeDy, false, 1024.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "AntiFakeDy", "%.4f",
                        result->nAntiFakeDy, ">=%d", threshold->minAntiFakeDy, false, 1024.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                }

                // TNoise/SNoise
                // DarkTNoise
                memset(line, 0, sizeof(line));
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DarkTNoise", "%.4f",
                        result->nTNoiseDarkN, "<=%f", threshold->maxTNoiseDarkN, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightTNoise
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightTNoise", "%.4f",
                        result->nTNoiseLightN, "<=%f", threshold->maxTNoiseLightN, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DarkSNoise
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DarkSNoise", "%.4f",
                        result->nSNoiseDarkN, "<=%f", threshold->maxSNoiseDarkN, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightSNoise
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightSNoise", "%.4f",
                        result->nSNoiseLightN, "<=%d", threshold->maxSNoiseLightN, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // LightHighMean
                // MinLightHighMean
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightHighMean", "%d",
                        result->nLightHighMean, ">=%d", threshold->minLightHighMean, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MaxLightHighMean
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightHighMean", "%d",
                        result->nLightHighMean, "<=%d", threshold->maxLightHighMean, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // LightLeakRatio
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightLeakRatio", "%.3f",
                        result->nLightLeakRatio, "<=%f", threshold->maxLightLeakRatio, false, 256.0);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // PolarDegree
                if (opticalType == DELMAR_OPTICAL_TYPE_7_0) {
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "PolarDegree", "%.3f",
                        result->nPolarDegree, ">=%f", threshold->minPolarDegree, false, 1024.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                }
                // Singal/FlatSNoise/TSNR/Sharpness
                // UnorSignal_LPF
                if (opticalType != DELMAR_OPTICAL_TYPE_7_0) {
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "UnorSignal_LPF", "%.4f",
                            result->nSignalL, ">=%d", threshold->minSignalL, false, 256.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                } else {
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "SignalLLow", "%.4f",
                            result->nSignalLLow, ">=%f", threshold->minSignalLLow, false, 256.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "SignalHigh", "%.4f",
                            result->nSignalLHigh, ">=%f", threshold->minSignalLHigh, false, 256.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                }
                // DataNoiseFlatLPF
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "DataNoiseFlatLPF", "%.3f",
                        result->nSNoiseFlatL, "<=%d", threshold->maxSNoiseFlatL, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // TSNR
                if (opticalType != DELMAR_OPTICAL_TYPE_7_0) {
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "TSNR", "%.4f",
                            result->nTSNR, ">=%f", threshold->minTSNR, false, 256.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                } else {
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "TSNRLow", "%.4f",
                            result->nTSNRLow, ">=%f", threshold->minTSNRLow, false, 256.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                    PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "TSNRHigh", "%.4f",
                            result->nTSNRHigh, ">=%f", threshold->minTSNRHigh, false, 256.0);
                    buffer.appendArray((uint8_t *) line, strlen(line));
                }

                // AssemblyAngle/CenterXOffset/CenterYOffset
                // AssemblyAngleOffset
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "AngletoChip", "%.3f",
                        result->nAssemblyAngelOffset, "<=%.2f", threshold->maxAssemblyAngleOffset, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // AssemblyXOffset
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "CenterXtoChip", "%.3f",
                        result->nAssemblyXOffset, "<=%.2f", threshold->maxAssemblyXOffset, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // AssemblyYOffset
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "CenterYtoChip", "%.3f",
                        result->nAssemblyYOffset, "<=%.2f", threshold->maxAssemblyYOffset, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

                // Diff Flesh and Diff Black
                // DiffFleshHM
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffFleshHM", "%d",
                        result->nDiffFleshHM, ">=%d", threshold->minDiffFleshHM, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DiffFleshML
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffFleshML", "%d",
                        result->nDiffFleshML, ">=%d", threshold->minDiffFleshML, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DiffBlackHM
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffBlackHM", "%d",
                        result->nDiffBlackHM, ">=%d", threshold->minDiffBlackHM, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // DiffBlackML
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MinDiffBlackML", "%d",
                        result->nDiffBlackML, ">=%d", threshold->minDiffBlackML, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // MaxDiffOffset
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MaxDiffOffset", "%d",
                        result->nDiffOffset, "<=%d", threshold->maxDiffOffset, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightStability
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MaxTNoise", "%.4f",
                        result->nLightStability, "<%f", threshold->maxLightStability, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LightLeakDark
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LightLeakDark", "%d",
                        result->nLightLeakDark, "<=%d", threshold->maxLightLeakDark, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // LowCorrPitch
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "LowCorrPitch", "%d",
                        result->nLowCorrPitch, "<=%d", threshold->maxLowCorrPitch, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // ValidAreaRatio
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "ValidAreaRatio", "%.4f",
                        result->nValidAreaRatio, ">=%f", threshold->minValidAreaRatio, false, 1024.0);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // ChartDirection
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "ChartDirection", "%d",
                        result->nChartDirection, "<=%d", threshold->maxChartDirection, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                /*
                // ChartGhostShadow
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "ChartGhostShadow", "%d",
                        result->nChartGhostShadow, "<=%d", threshold->maxChartGhostShadow, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // ChartDirty
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "ChartDirty", "%d",
                        result->nChartDirty, "<=%d", threshold->maxChartDirty, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                */

                if (buffer.size() > 0) {
                    uint8_t * buffer_tmp = (uint8_t *)buffer.array();
                    uint32_t buffer_len = (uint32_t)buffer.size();
                    char file_name[GF_DUMP_FILE_PATH_MAX_LEN] = {0};
                    snprintf(file_name, GF_DUMP_FILE_PATH_MAX_LEN, "%s_err%05d_result_threshold.csv", timeStamp, cal_cmd->err_code);
                    snprintf(file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s/test", getDumpRootDir());

                    err = HalUtils::writeFile(file_path, file_name, buffer_tmp, buffer_len);
                }
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }


    gf_error_t CustomizedDelmarProductTest::savePerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold) {
        gf_error_t err = GF_SUCCESS;
        Vector<uint8_t> buffer;
        uint8_t sensorIdx = 0;
        FUNC_ENTER();

        do {
            if (NULL == timeStamp || NULL == cal_cmd || sensorNum < 1 || sensorNum > 4) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            sensorIdx = cal_cmd->cmd_base.i_sensor_index;
            gf_delmar_sensor_type_t sensorType = ((DelmarSensor*) mContext->mSensor)->getSenosrType();
            LOG_D(LOG_TAG, "[%s] sensorIdx=%d, operationStep=%d, sensorType = %d", __func__, sensorIdx, operationStep, sensorType);

            if (sensorType == DELMAR_SENSOR_TYPE_SINGLE_T_SE2 || sensorType == DELMAR_SENSOR_TYPE_SINGLE_T_SE5) {
                err = saveNewPerformanceTestResult(timeStamp, cal_cmd, operationStep, sensorNum, threshold);
            } else {
                err = saveOldPerformanceTestResult(timeStamp, cal_cmd, operationStep, sensorNum, threshold);
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::testOTPFLash() {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        CustomizedSensor *cSensor = (CustomizedSensor*) sensor;
        gf_customized_config_t customizedConfig = { 0 };
        uint8_t opticalType = sensor->getOpticalType();
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

            if (err == GF_SUCCESS) {
                cSensor->getCustomizedConfig(&customizedConfig);
                LOG_D(LOG_TAG, "[%s] opticalType=%d, configOpticalType=%d", __func__,
                             opticalType, customizedConfig.customized_optical_type);
                if (opticalType != customizedConfig.customized_optical_type) {
                    LOG_E(LOG_TAG, "[%s] opticalType miss match!", __func__);
                    err = GF_ERROR_TEST_OTP;
                }
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

            notifyTestCmd(0, PRODUCT_TEST_CMD_OTP_FLASH, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }
        FUNC_EXIT(err);
        return err;
    }
    bool CustomizedDelmarProductTest::isNeedLock(int32_t cmdId) {
        if (cmdId == PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN) {
            return false;
        }
        return DelmarProductTest::isNeedLock(cmdId);
    }

    void CustomizedDelmarProductTest::testResetInterruptPinFinished(gf_error_t result) {
        VOID_FUNC_ENTER();
        mTestResetInterruptResult = result;
        DelmarProductTest::testResetInterruptPinFinished(result);
        VOID_FUNC_EXIT();
    }

    gf_error_t CustomizedDelmarProductTest::testResetInterruptPin() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            err = DelmarProductTest::testResetInterruptPin();
            GF_ERROR_BREAK(err);
            while (mTestingRstIntPin) {
                LOG_D(LOG_TAG, "[%s] wait asyn message.", __func__);
                usleep(100);
            }
            err = mTestResetInterruptResult;
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
