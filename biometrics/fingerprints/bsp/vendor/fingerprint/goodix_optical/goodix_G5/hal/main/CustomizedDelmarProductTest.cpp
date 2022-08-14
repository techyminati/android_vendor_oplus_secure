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
        : DelmarProductTest(context) {
        mContext = context;
        CustomizedSensor *sensor = (CustomizedSensor*) context->mSensor;
        mProductScreenId = sensor->getProductScreenId();
        LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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
            case SCREEN_TYPE_CC161_SAMSUNG_ID:
            case SCREEN_TYPE_DD306_SAMSUNG_ID: {
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

    gf_error_t CustomizedDelmarProductTest::savePerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold) {
        gf_error_t err = GF_SUCCESS;
        char file_path[GF_DUMP_FILE_PATH_MAX_LEN] = {0};
        Vector<uint8_t> buffer;
        uint8_t sensorIdx = 0;
        FUNC_ENTER();

        do {
            if (NULL == timeStamp || NULL == cal_cmd || sensorNum < 1 || sensorNum > 4) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            sensorIdx = cal_cmd->cmd_base.i_sensor_index;
            LOG_D(LOG_TAG, "[%s] sensorIdx=%d, operationStep=%d", __func__, sensorIdx, operationStep);
            gf_calculate_cmd_result_t * result = &(cal_cmd->o_result);

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
                // BBlackPixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BBlackPixelNum", "%d",
                    result->nBBlackPixelNum, "<=%d", threshold->maxBWhitePixelNumLow, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // BWhitePixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "BWhitePixelNum", "%d",
                    result->nBWhitePixelNum, "<=%d", threshold->maxBWhitePixelNumLow, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));
                // HotPixelNum
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "HotPixelNum", "%d",
                    result->nHotPixelNum, "<=%d", threshold->maxHotConnected, false, 1);
                buffer.appendArray((uint8_t *) line, strlen(line));

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
                PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(line, sizeof(line), "MaxTNoise", "%.4f",
                    result->nMaxTNoise, "<%f", threshold->maxLightStability, false, 1024.0);
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
}  // namespace goodix
