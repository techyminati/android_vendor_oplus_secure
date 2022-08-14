#define LOG_TAG "[GF_HAL][CustomizedDelmarProductTest]"

#include <string.h>
#include <stdbool.h>
#include <cmath>
#include "CustomizedDelmarProductTest.h"
#include "gf_customized_sensor_types.h"
#include "fp_eng_test.h"
#include "DelmarProductTestDefine.h"
#include "HalLog.h"
#include "HalContext.h"
#include "DelmarSensor.h"
#include "TestUtils.hpp"
#include "Timer.h"
#include "HalContextExt.h"
#include "CustomizedDelmarCommon.h"
#include "cutils/properties.h"
#include "DelmarAlgoUtils.h"
#include "CustomizedDelmarHalDump.h"
#include "CustomizedDelmarSensor.h"
#include "CustomizedDevice.h"
#include "DelmarHalUtils.h"
#include "gf_customized_algo_types.h"

#include "FingerprintCore.h"
extern "C" {
#include "Fpsys.h"
}

#define HBM_PATH "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/drm/card0/card0-DSI-1/hbm"
#define BRIGHTNESS_PATH "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/backlight/panel0-backlight/brightness"
#define OPEN_HBM_VALUE "5"
#define CLOSE_HBM_VALUE "0"
#define HIGH_BRIGHTNESS_VALUE "1023"
#define LOW_BRIGHTNESS_VALUE "600"
#define CHECK_UNLOCK_WHEN_CAL 1

static bool notifyOnce_flag = true;  // notifyOnce_flag
//  static uint8_t m_pgagain = 0;  // m_pgagain

namespace goodix {

    sz_product_test_cmd_table_t szProductTestCmdTable[] = {
        TABLE(CMD_TEST_SZ_FINGER_DOWN),
        TABLE(CMD_TEST_SZ_FINGER_UP),
        TABLE(CMD_TEST_SZ_ENROLL),
        TABLE(CMD_TEST_SZ_FIND_SENSOR),
        TABLE(CMD_TEST_SZ_FUSION_PREVIEW),
        TABLE(CMD_TEST_SZ_UNTRUSTED_ENROLL),
        TABLE(CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE),
        TABLE(CMD_TEST_SZ_DELETE_UNTRUSTED_ENROLLED_FINGER),
        TABLE(CMD_TEST_SZ_RAWDATA_PREVIEW),
        TABLE(CMD_TEST_SZ_LDC_CALIBRATE),
        TABLE(CMD_TEST_SZ_ENROLL_TEMPLATE_COUNT),
        TABLE(CMD_TEST_SZ_UPDATE_CAPTURE_PARM),
        TABLE(CMD_TEST_SZ_CANCEL),
        TABLE(CMD_TEST_SZ_GET_CONFIG),
        TABLE(CMD_TEST_SZ_GET_VERSION),
        TABLE(CMD_TEST_SZ_K_B_CALIBRATION),
        TABLE(CMD_TEST_SZ_SET_GROUP_ID),
        TABLE(CMD_TEST_SZ_UPDATE_CFG),
        TABLE(CMD_TEST_SZ_UPDATE_FW),
        TABLE(CMD_TEST_SZ_UNTRUSTED_ENUMERATE),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_DARK_BASE),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_H_DARK),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_L_DARK),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_H_FLESH),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_L_FLESH),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHART),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHECKBOX),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_LOCATION_IMAGE),
        TABLE(CMD_TEST_SZ_FT_FACTORY_PERFORMANCE),
        TABLE(CMD_TEST_SZ_FT_EXPO_AUTO_CALIBRATION),
        TABLE(CMD_TEST_SZ_FT_STOP_EXPO_AUTO_CALIBRATION),
        TABLE(CMD_TEST_SZ_FT_RESET),
        TABLE(CMD_TEST_SZ_FT_SPI_RST_INT),
        TABLE(CMD_TEST_SZ_FT_SPI),
        TABLE(CMD_TEST_SZ_FT_INIT),
        TABLE(CMD_TEST_SZ_FT_EXIT),
        TABLE(CMD_TEST_SZ_FT_CALIBRATE),
        TABLE(CMD_TEST_SZ_FT_MT_CHECK),
        TABLE(CMD_TEST_SZ_FT_KPI),
        TABLE(CMD_TEST_SZ_FT_FACTORY_CAPTURE_IMAGE_MANUAL),
        TABLE(CMD_TEST_SZ_DUMP_TEMPLATE),
        TABLE(CMD_TEST_SZ_SET_HBM_MODE),
        TABLE(CMD_TEST_SZ_CLOSE_HBM_MODE),
        TABLE(CMD_TEST_SZ_SET_HIGH_BRIGHTNESS),
        TABLE(CMD_TEST_SZ_SET_LOW_BRIGHTNESS),
        TABLE(CMD_TEST_SZ_SET_DUMP_ENABLE_FLAG),
        TABLE(CMD_TEST_SZ_FACTORY_TEST_GET_MT_INFO),
        TABLE(CMD_TEST_SZ_SET_MAX_BRIGHTNESS),
        TABLE(CMD_TEST_SZ_LOCAL_AREA_SAMPLE),
        TABLE(CMD_TEST_SZ_ENABLE_POWER),
        TABLE(CMD_TEST_SZ_DISABLE_POWER),
        TABLE(CMD_TEST_SZ_UI_READY),
        TABLE(CMD_TEST_SZ_UI_DISAPPER),
        TABLE(CMD_TEST_SZ_ENABLE_PAY_ENVIRONMENT_LEVEL),
        TABLE(CMD_TEST_SZ_DISABLE_PAY_ENVIRONMENT_LEVEL),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_M_FLESH),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_M_DARK),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CIRCLEDATA),
        TABLE(CMD_TEST_SZ_FACTORY_TEST_GET_RELIABILTY_INFO),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_FLESH_CIRCLEDATA),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHECKBOX_CAPTURE),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHART_CAPTURE),
        TABLE(CMD_TEST_SZ_FT_SAMPLE_CALIBRATE_CHART),
        TABLE(CMD_TEST_SZ_FT_SAMPLE_CALIBRATE),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_CALI_INIT),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_CALI_EXIT),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_CALI_BASE),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_CALI_AUTO_CALIBRATION),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_STOP_CALI_AUTO_CALIBRATION),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_CALI_PERFORMANCE),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_CALI_SCREEN_CHART),
        TABLE(CMD_TEST_SZ_FT_SIMPLE_CANCLE),
        TABLE(CMD_TEST_PRODUCT_CMD_MAX),
    };
    CustomizedDelmarProductTest::CustomizedDelmarProductTest(HalContext *context)
        : DelmarProductTest(context) {
        uint8_t opticalType = ((DelmarSensor*) context->mSensor)->getOpticalType();
        cusSensorBgVersion = ((DelmarSensor*) context->mSensor)->getSensorBgVersion();
        LOG_D(LOG_TAG, "[%s] opticalType=%d, bgversion=%d", __func__, opticalType, cusSensorBgVersion);
        init(opticalType, cusSensorBgVersion);
    }

    CustomizedDelmarProductTest::~CustomizedDelmarProductTest() {
    }
    void CustomizedDelmarProductTest::init(uint8_t opticalType, uint32_t bgVersion) {
        if (opticalType == DELMAR_OPTICAL_TYPE_3_0) {
            // G6 op3.0
            mNewTestConfig.maxBadPixelNum = 200;
            mNewTestConfig.maxBadBlockNum = 14;
            mNewTestConfig.maxBadBlockLargest = 67;
            mNewTestConfig.maxBadPixelAllBlocks = 87;
            mNewTestConfig.maxBadLineNum = 0;
            mNewTestConfig.maxHotPixelLowNum = 100;
            mNewTestConfig.maxHotLineNum = 1000;  // not check
            mNewTestConfig.maxHotBlockLargest = 17;
            mNewTestConfig.maxDPBadPixelNum = 50;
            mNewTestConfig.maxDPBadPixelInRow = 8;
            mNewTestConfig.maxDPDiffMean = 10;  // not check
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
            mNewTestConfig.minSharpnessL = 0.0;  // not check
            mNewTestConfig.maxAssemblyAngleOffset = 2.5;
            mNewTestConfig.maxAssemblyXOffset = 10.0;
            mNewTestConfig.maxAssemblyYOffset = 10.0;
            mNewTestConfig.minDiffFleshHM = 200;
            mNewTestConfig.minDiffFleshML = 100;
            mNewTestConfig.minDiffBlackHM = 100;
            mNewTestConfig.minDiffBlackML = 50;
            mNewTestConfig.minDiffBlackLD = 50;
            mNewTestConfig.maxDiffOffset = 1500;
            mNewTestConfig.maxLightStability = 10;
            mNewTestConfig.maxLightLeakDark = 30;
            mNewTestConfig.maxLowCorrPitch = 100;  // not check
            mNewTestConfig.minValidAreaRatio = 0.8;
            mNewTestConfig.maxChartDirection = 15;
            mNewTestConfig.maxChartGhostShadow = 20;  // not check
            mNewTestConfig.maxChartDirty = 1000;  // not check this for 3.0, set it big
            mNewTestConfig.standardAngle = 90.0;
            mNewTestConfig.standardCenterX = 60;
            mNewTestConfig.standardCenterY = 65;
        } else if (opticalType == DELMAR_OPTICAL_TYPE_7_0) {
            //  G6 op7.0
            mNewTestConfig.maxBadPixelNum = 150;
            mNewTestConfig.maxBadBlockNum = 10;
            mNewTestConfig.maxBadBlockLargest = 67;
            mNewTestConfig.maxBadPixelAllBlocks = 87;
            mNewTestConfig.maxBadLineNum = 0;
            mNewTestConfig.maxHotPixelLowNum = 100;
            mNewTestConfig.maxHotLineNum = 0;  // not check
            mNewTestConfig.maxHotBlockLargest = 17;
            mNewTestConfig.maxDPBadPixelNum = 50;
            mNewTestConfig.maxDPBadPixelInRow = 8;
            mNewTestConfig.maxDPDiffMean = 10;  // not check
            mNewTestConfig.maxDPSNoiseDarkN = 5.1;
            mNewTestConfig.maxDPAADiffDark = 22;
            mNewTestConfig.maxHAFBadPixelNum = 90;  // not check
            mNewTestConfig.maxHAFBadBlockNum = 15;  // not check
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
                mNewTestConfig.minTSNRLow = 12.0;
                mNewTestConfig.minTSNRHigh = 20.58;
            }
            mNewTestConfig.minSharpnessL = 0.0;  // not check
            mNewTestConfig.maxAssemblyAngleOffset = 2.5;
            mNewTestConfig.maxAssemblyXOffset = 12.0;
            mNewTestConfig.maxAssemblyYOffset = 12.0;
            mNewTestConfig.minDiffFleshHM = 200;
            mNewTestConfig.minDiffFleshML = 100;
            mNewTestConfig.minDiffBlackHM = 100;
            mNewTestConfig.minDiffBlackML = 50;
            mNewTestConfig.minDiffBlackLD = 50;
            mNewTestConfig.maxDiffOffset = 1500;
            mNewTestConfig.maxLightStability = 20;
            mNewTestConfig.maxLightLeakDark = 30;
            mNewTestConfig.maxLowCorrPitch = 100;  // not check
            mNewTestConfig.minValidAreaRatio = 0.8;
            mNewTestConfig.maxChartDirection = 15;
            mNewTestConfig.maxChartGhostShadow = 20;  // not check
            mNewTestConfig.maxChartDirty = 0;  // not check
            mNewTestConfig.standardAngle = 90.0;
            mNewTestConfig.standardCenterX = 60;
            mNewTestConfig.standardCenterY = 60;
        }
    }
    gf_error_t CustomizedDelmarProductTest::checkChartDirection(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum) {
        gf_error_t err = GF_SUCCESS;
        int32_t sensor_index = 0;
        FUNC_ENTER();
        do {
            /*direction must be in 0~10 and 170 ~ 180*/
            for (sensor_index = 0; sensor_index < sensorNum; sensor_index++) {
                cal_cmd->o_result.nDirectionOffset[sensor_index] = abs(abs(cal_cmd->o_result.nChartDirection[sensor_index]) -0);
                if (cal_cmd->o_result.nDirectionOffset[sensor_index] > mTestConfig.maxChartDirection) {
                    cal_cmd->o_result.error_type[sensor_index] = GF_ERROR_CHART_DIRECTION_INCORRECT;
                    err = GF_ERROR_CHART_DIRECTION_INCORRECT;
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, maxChartDirection=%d(x <=%d)", __func__, sensor_index, cal_cmd->o_result.nChartDirection[sensor_index], mTestConfig.maxChartDirection);
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

    const char *CustomizedDelmarProductTest::toString(int32_t cmdID) {
        uint32_t idx = 0;
        uint32_t len = sizeof(szProductTestCmdTable) / sizeof(sz_product_test_cmd_table_t);

        for (idx = 0; idx < len; idx++) {
            if (cmdID == szProductTestCmdTable[idx].cmdID) {
                return szProductTestCmdTable[idx].cmdIDString;
            }
        }
        return "NULL";
    }

    void CustomizedDelmarProductTest::updaetReliabilityThreshold(uint32_t bgVersion) {
        mNewTestConfig.maxBadPixelNum = 165;
        mNewTestConfig.maxBadBlockNum = 11;
        mNewTestConfig.maxBadBlockLargest = 74;
        mNewTestConfig.maxBadPixelAllBlocks = 96;
        mNewTestConfig.maxBadLineNum = 0;
        mNewTestConfig.maxHotPixelLowNum = 110;
        mNewTestConfig.maxHotBlockLargest = 19;
        mNewTestConfig.maxDPBadPixelNum = 55;
        mNewTestConfig.maxDPSNoiseDarkN = 5.7;
        mNewTestConfig.maxDPAADiffDark = 25;
        mNewTestConfig.maxAntiFakeDx = 33;
        mNewTestConfig.minAntiFakeDx = 5;
        mNewTestConfig.maxAntiFakeDy = 33;
        mNewTestConfig.minAntiFakeDy = 5;
        mNewTestConfig.maxTNoiseDarkN = 1.7;
        mNewTestConfig.maxTNoiseLightN = 22;
        mNewTestConfig.maxSNoiseDarkN = 1.8;
        mNewTestConfig.maxSNoiseLightN = 132;
        mNewTestConfig.maxLightLeakRatio = 0.83;
        mNewTestConfig.minPolarDegree = 1.28;
        if (bgVersion == 0) {
            mNewTestConfig.minSignalLLow = 17.94;
            mNewTestConfig.minSignalLHigh = 30.15;
        } else {
            mNewTestConfig.minSignalLLow = 28.40;
            mNewTestConfig.minSignalLHigh = 47.74;
        }
        mNewTestConfig.maxSNoiseFlatL = 10;
        if (bgVersion == 0) {
            mNewTestConfig.minTSNRLow = 11.48;
            mNewTestConfig.minTSNRHigh = 17.63;
        } else {
            mNewTestConfig.minTSNRLow = 10.80;
            mNewTestConfig.minTSNRHigh = 18.52;
        }
    }

    gf_error_t CustomizedDelmarProductTest::checkBeforeCal(int32_t cmd) {
        CustomizedDelmarSensor * cDelmarSensor = (CustomizedDelmarSensor *)mContext->mSensor;
        fp_eng_tests_result eng_result ={0, 0, {0}};
        char value1[128] = {0};
        char value2[128] = {0};
        gf_error_t ret = GF_SUCCESS;
        if (cDelmarSensor->isSensorVaild == -2) {
            snprintf(eng_result.info, sizeof(eng_result.info), "FAIL:Fingerprint pin id can't match sensorType!");
            ret = GF_ERROR_SENSOR_NOT_AVAILABLE;
        } else if (cDelmarSensor->isSensorVaild == -1) {
            snprintf(eng_result.info, sizeof(eng_result.info), "FAIL:Sensor_type is invaild!");
            ret = GF_ERROR_SENSOR_NOT_AVAILABLE;
        } else if (CHECK_UNLOCK_WHEN_CAL) {
            property_get("ro.boot.verifiedbootstate", value1, "green");
            property_get("gf.debug.dump_data", value2, "0");
            if ((value1[0] == 'o' ||  value1[0] == 'O') && value2[0] == '0'){
                snprintf(eng_result.info, sizeof(eng_result.info), "FAIL:The device is unlock!");
                ret = GF_ERROR_SENSOR_NOT_AVAILABLE;
            }
        }

        if (ret != GF_SUCCESS) {
           eng_result.cmdId = cmd;
           notifyEngResult(&eng_result);
        }
        return ret;
    }

    bool reliabilityOrNot = false;
    gf_error_t CustomizedDelmarProductTest::executeCommand(int32_t cmdId, const int8_t *in,
                                                 uint32_t inLen, int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_SUCCESS;
        gf_error_t ret = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        uint8_t opticalType = sensor->getOpticalType();
        FUNC_ENTER();

        LOG_D(LOG_TAG, "[%s] cmdId=%d (%s)", __func__, cmdId, toString(cmdId));
        int8_t operation[48];
        int8_t *current = operation;

        if (isNeedCtlSensor(cmdId)) {
            err = sensor->wakeupSensor();
            if (err != GF_SUCCESS) {
                notifyResetStatus(err, cmdId);
                FUNC_EXIT(err);
                return err;
            }
        }

        switch (cmdId) {
            case CMD_TEST_SZ_FT_SPI: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_SPI for normal calibration test");
                reliabilityOrNot = false;
                err = testSpi();
                break;
            }
            case CMD_TEST_SZ_FACTORY_TEST_GET_RELIABILTY_INFO: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_SPI for reliability calibration test");
                reliabilityOrNot = true;
                updaetReliabilityThreshold(cusSensorBgVersion);
                err = testSpi();
                break;
            }
            case CMD_TEST_SZ_FT_RESET:
            case CMD_TEST_SZ_FT_SPI_RST_INT: {
                err = testResetInterruptPin();
                break;
            }
            case CMD_TEST_SZ_FT_MT_CHECK: {
                err = testOTPFLash();
                break;
            }
            case CMD_TEST_SZ_FT_EXPO_AUTO_CALIBRATION: {
                if (reliabilityOrNot) {
                    LOG_D(LOG_TAG, "RELIABILTY TEST");
                    fp_eng_tests_result eng_result;
                    eng_result.cmdId = CMD_FT_EXPO_AUTO_CALIBRATION_TEST;
                    snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_COLLECT TEST ");
                    snprintf(eng_result.info, sizeof(eng_result.info), "%sPASS", eng_result.info);
                    eng_result.result = 1;
                    notifyEngResult(&eng_result);
                    break;
                }
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_COLLECT);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }

            case CMD_TEST_SZ_FT_INIT: {
                if (reliabilityOrNot) {
                    LOG_D(LOG_TAG, "RELIABILTY TEST");
                    fp_eng_tests_result eng_result;
                    eng_result.cmdId = CMD_FT_INIT_TEST;
                    snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN TEST ");
                    snprintf(eng_result.info, sizeof(eng_result.info), "%sPASS", eng_result.info);
                    eng_result.result = 1;
                    notifyEngResult(&eng_result);
                    break;
                }
                LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                err = performanceTest(operation, 4*sizeof(int32_t));
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN faild");
                    break;
                }
                /*caculate gain*/
                LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT");
                memset(operation, 0, sizeof(operation));
                current = operation;
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                err = performanceTest(operation, 4*sizeof(int32_t));
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT faild");
                    break;
                }

                /*caculate gain two*/
                memset(operation, 0, sizeof(operation));
                current = operation;
                LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_TWO);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                err = performanceTest(operation, 4*sizeof(int32_t));
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO faild");
                    break;
                }

                /*caculate gain two collect*/
                memset(operation, 0, sizeof(operation));
                current = operation;
                LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                err = performanceTest(operation, 4*sizeof(int32_t));
                if (GF_SUCCESS != err) {
                    LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT faild");
                    break;
                }
                memset(operation, 0, sizeof(operation));
                current = operation;
                LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_H_FLESH: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_H_FLESH");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MAX_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_M_FLESH: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_M_FLESH");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MID_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }

            case CMD_TEST_SZ_FT_CAPTURE_L_FLESH: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_L_FLESH");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MIN_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }

            case CMD_TEST_SZ_FT_CAPTURE_H_DARK: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_H_DARK");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_M_DARK: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_M_DARK");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MID_DARK_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_L_DARK: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_L_DARK");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_DARK_BASE: {
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_DARK_BASE");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_DARK_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_CIRCLEDATA: {
                usleep(200);
                LOG_D(LOG_TAG, "OPERATION_STEP_CIRCLEDATA_COLLECT sleep 2");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CIRCLEDATA_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_FLESH_CIRCLEDATA: {
                usleep(200);
                if (opticalType == DELMAR_OPTICAL_TYPE_3_0) {
                    err = GF_SUCCESS;
                    LOG_D(LOG_TAG, "OP3 DO NOT HAVE CAPTURE_FLESH_CIRCLEDATA");
                    gf_delmar_performance_test_cmd_base_t* col_cmd = new gf_delmar_performance_test_cmd_base_t();
                    memset(col_cmd, 0, sizeof(gf_delmar_performance_test_cmd_base_t));
                    col_cmd->i_collect_phase = OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT;
                    notifyPerformanceTest(err, OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT, &col_cmd);
                    break;
                }
                LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_FLESH_CIRCLEDATA");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_CHECKBOX: {
                LOG_D(LOG_TAG, "OPERATION_STEP_CHARTDATA_COLLECT");
                usleep(200);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CHARTDATA_COLLECT);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FT_CAPTURE_CHART: {
                LOG_D(LOG_TAG, "OPERATION_STEP_GET_KB_CALIBRATION");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_GET_KB_CALIBRATION);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 4*sizeof(int32_t));
                break;
            }
            case CMD_TEST_SZ_FACTORY_TEST_GET_MT_INFO: {
                LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS");
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS);
                if (reliabilityOrNot) {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_DISABLE_SAVE_FLAG, 1);
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 4);
                } else {
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_DISABLE_SAVE_FLAG, 0);
                    current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                }
                err = performanceTest(operation, 6*sizeof(int32_t));
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

            case CMD_TEST_SZ_FT_KPI: {
                LOG_D(LOG_TAG, "[%s] test image kpi", __func__);
                err = sensor->wakeupSensor();
                if (err != GF_SUCCESS) {
                    notifyResetStatus(err, cmdId);
                    FUNC_EXIT(err);
                    return err;
                }
                fp_set_tpirq_enable(1);
                mContext->mCenter->registerHandler(this);
                break;
            }

            case CMD_TEST_SZ_FIND_SENSOR: {
                // err = findSensor();
                startAgingTimer(1);
                break;
            }
            case CMD_TEST_SZ_FT_EXIT: {
                notifyOnce_flag = true;
                cancelAgingTimer();
                err = sensor->sleepSensor();
                fp_set_tpirq_enable(0);
                mContext->mCenter->deregisterHandler(this);
                break;
            }

            default: {
                err = DelmarProductTest::executeCommand(cmdId, in, inLen, out, outLen);
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

    bool CustomizedDelmarProductTest::isNeedCtlSensor(uint32_t cmdId) {
        bool ret = false;

        switch (cmdId) {
            case CMD_TEST_SZ_FT_SPI:
            case CMD_TEST_SZ_FACTORY_TEST_GET_RELIABILTY_INFO:
            case CMD_TEST_SZ_FT_MT_CHECK:
            case CMD_TEST_SZ_FT_RESET:
            case CMD_TEST_SZ_FT_EXPO_AUTO_CALIBRATION:
            case CMD_TEST_SZ_FT_INIT:
            case CMD_TEST_SZ_FT_CAPTURE_H_FLESH:
            case CMD_TEST_SZ_FT_CAPTURE_L_FLESH:
            case CMD_TEST_SZ_FT_CAPTURE_H_DARK:
            case CMD_TEST_SZ_FT_CAPTURE_L_DARK:
            case CMD_TEST_SZ_FT_CAPTURE_M_FLESH:
            case CMD_TEST_SZ_FT_CAPTURE_M_DARK:
            case CMD_TEST_SZ_FT_CAPTURE_DARK_BASE:
            case CMD_TEST_SZ_FT_CAPTURE_CHECKBOX:
            case CMD_TEST_SZ_FT_CAPTURE_CHART:
            case CMD_TEST_SZ_FT_CAPTURE_CIRCLEDATA:
			case CMD_TEST_SZ_FT_CAPTURE_FLESH_CIRCLEDATA:
            case CMD_TEST_SZ_FACTORY_TEST_GET_MT_INFO: {
                ret = true;
                break;
            }

            default: {
                break;
            }
        }

        return ret;
    }

    void CustomizedDelmarProductTest::notifyPerformanceTest(gf_error_t err, uint32_t phase,
                                                  void *cmd) {
        int8_t *test_result = NULL;
        gf_delmar_performance_test_cmd_base_t *parent_result = NULL;
        gf_delmar_calculate_cmd_t *cal_cmd = NULL;
        gf_new_product_report_result_t *cal_result = NULL;
        uint32_t len = 0;
        char tmp_result[4096] = {0};
        fp_eng_tests_result eng_result = {0, 0, ""};
        bool go_on = false;
        DelmarProductTest::notifyPerformanceTest(err, phase, cmd);
        bool isCollectPhase = true;
        uint32_t sensorNum = ((DelmarSensor*) mContext->mSensor)->getAvailableSensorNum();
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        uint8_t opticalType = sensor->getOpticalType();
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
            // PgaGain
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint8_t));
            // ExposureTime
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int16_t));
            // FrameNumber
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // ItoPatternCode
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint16_t));
            // HotPixelNum
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // ValidArea
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // LowCorrPitch_LPF
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // EdgePixelLevel1
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // EdgePixelLevel2
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // FlatSNoise
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // FlatSNoise_LPF
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // FlatSNoise_LPF_MT
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // ChartDirection
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // Signal
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // SSNR
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // Noise
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // Shapeness
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // Signal_LPF
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // SSNR_LPF
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // Noise_LPF
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // Shapeness_LPF
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(int32_t));
            // DarkTNoise
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // DarkSNoise
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // LightTNoise
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // LightSNoise
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // LightHighMean
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // CenterXtoChip
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // CenterYtoChip
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
            // AngletoChip
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint32_t));
        } else if (parent_result->i_collect_phase == OPERATION_STEP_CALCULATE_GAIN) {
            // pga gain
            len += HAL_TEST_SIZEOF_ARRAY(sensorNum * sizeof(uint8_t));
        }

        test_result = new int8_t[len] { 0 };

        if (test_result != NULL) {
            memset(test_result, 0, len);
            int8_t *current = test_result;

            if (parent_result->i_collect_phase == OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS) {
                cal_cmd = (gf_delmar_calculate_cmd_t *)cmd;
                cal_result = &cal_cmd->o_new_product_report_result;

                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, parent_result->i_collect_phase);
                /*current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_PGA_GAIN,
                                (int8_t *)cal_result->nPgaGain, sensorNum * sizeof(uint8_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_EXPOSURE_TIME,
                                (int8_t *)cal_result->nExposureTime, sensorNum * sizeof(int16_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_FRAME_NUMBER,
                                (int8_t *)cal_result->nFrameNumber, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_ITO_PATTERN_CODE,
                                (int8_t *)cal_result->nItoPatternCode, sensorNum * sizeof(uint16_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_HOT_PIXEL_NUM,
                                (int8_t *)cal_result->nHotPixelNum, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_VALID_AREA,
                                (int8_t *)cal_result->nInValidArea, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LOW_CORR_PITCH_LPF,
                                (int8_t *)cal_result->nLowCorrPitch_LPF, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_FLAT_SNOISE,
                                (int8_t *)cal_result->nDataNoiseFlat, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_FLAT_SNOISE_LPF,
                                (int8_t *)cal_result->nDataNoiseFlatLPF, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_FLAT_SNOISE_LPF_MT,
                                (int8_t *)cal_result->nDataNoiseFlatLPF_MT, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_CHART_DIRECTION,
                                (int8_t *)cal_result->nChartDirection, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SIGNAL,
                                (int8_t *)cal_result->nSignal, sensorNum * sizeof(int32_t));

                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SSNR,
                                (int8_t *)cal_result->nSSNR, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_NOISE,
                                (int8_t *)cal_result->nNoise, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SHAPENESS,
                                (int8_t *)cal_result->nShapeness, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SIGNAL_LPF,
                                (int8_t *)cal_result->nSignal_LPF, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SSNR_LPF,
                                (int8_t *)cal_result->nSSNR_LPF, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_NOISE_LPF,
                                (int8_t *)cal_result->nNoise_LPF, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_SHAPENESS_LPF,
                                (int8_t *)cal_result->nShapeness_LPF, sensorNum * sizeof(int32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DARK_TNOISE,
                                (int8_t *)cal_result->nDarkTNoise, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_DARK_SNOISE,
                                (int8_t *)cal_result->nDarkSNoise, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_TNOISE,
                                (int8_t *)cal_result->nLightTNoise, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_SNOISE,
                                (int8_t *)cal_result->nLightSNoise, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_RESULT_LIGHT_HIGH_MEAN,
                                (int8_t *)cal_result->nLightHighMean, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_CENTER_X_TO_CHIP,
                                (int8_t *)cal_result->nCenterXtoChip, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_CENTER_Y_TO_CHIP,
                                (int8_t *)cal_result->nCenterYtoChip, sensorNum * sizeof(uint32_t));
                current = TestUtils::testEncodeArray(current, PRODUCT_TEST_TOKEN_ANGLE_TO_CHIP,
                                (int8_t *)cal_result->nAngletoChip, sensorNum * sizeof(uint32_t));*/
            } else {
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
/*
        if (NULL != cal_result) {
            angel = cal_result->nAngletoChip[0] * 180.0 /12868.0;
            if (angel >= 0) {
                angel = mTestConfig.standardAngle - angel;
            } else {
                angel = -mTestConfig.standardAngle - angel;
            }
        }
*/
        switch (phase) {
            case OPERATION_STEP_CALCULATE_GAIN: {
                if (err == GF_SUCCESS) {
                    go_on = true;
                } else {
                    eng_result.cmdId = CMD_FT_INIT_TEST;
                    snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN TEST ");
                    go_on = false;
                }
                break;
            }
            case OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT: {
                if (err == GF_SUCCESS) {
                    go_on = true;
                } else {
                    eng_result.cmdId = CMD_FT_INIT_TEST;
                    snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT TEST ");
                    go_on = false;
                }
                break;
            }
            case OPERATION_STEP_CALCULATE_GAIN_TWO: {
                if (err == GF_SUCCESS) {
                    go_on = true;
                } else {
                    eng_result.cmdId = CMD_FT_INIT_TEST;
                    snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO TEST ");
                    go_on = false;
                }
                break;
            }
            case OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT: {
                if (err == GF_SUCCESS) {
                    go_on = true;
                } else {
                    eng_result.cmdId = CMD_FT_INIT_TEST;
                    snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT TEST ");
                    go_on = false;
                }
                break;
            }
            case OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN: {
                go_on = false;
                eng_result.cmdId = CMD_FT_INIT_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN TEST ");
                break;
            }
            case OPERATION_STEP_CALCULATE_GAIN_COLLECT: {
                go_on = false;
                gf_delmar_sensor_info_t sensor_info;
                LOG_D(LOG_TAG, "[%s]sensor_info size: %d", __func__, (int32_t)sizeof(gf_delmar_sensor_info_t));
                memset((void *) &sensor_info, 0, sizeof(gf_delmar_sensor_info_t));
                getSensorInfo((gf_delmar_sensor_info_t *)&sensor_info, 1);
                LOG_D(LOG_TAG, "[%s]sensor_info: 0x%x, 0x%x, 0x%x", __func__, *(uint32_t*)(&sensor_info.o_sensor_id[0]), *(uint32_t*)(&sensor_info.o_sensor_id[4]),
                                    *(uint32_t*)(&sensor_info.o_sensor_id[8]));
                char uid[2*DELMAR_SENSOR_ID_MAX_BUFFER_LEN+1] = {0};
                int j = 0;
                for (int i = 0; i < DELMAR_SENSOR_ID_MAX_BUFFER_LEN; i++) {
                    snprintf(&uid[j], sizeof(uid) - j, "%02x", sensor_info.o_sensor_id[i]);
                    j = j + 2;
                }
                uid[2*DELMAR_SENSOR_ID_MAX_BUFFER_LEN] = '\0';

                snprintf(eng_result.info, sizeof(eng_result.info), "CHIPID:0x%s,\n", uid);
                eng_result.cmdId = CMD_FT_EXPO_AUTO_CALIBRATION_TEST;
                break;
            }
            case OPERATION_STEP_BASEDATA_MAX_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_H_FLESH_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_BASEDATA_MAX_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_BASEDATA_MID_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_M_FLESH_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_BASEDATA_MID_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_BASEDATA_MIN_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_L_FLESH_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_BASEDATA_MIN_COLLECT TEST ");
            }
            break;
            case OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_H_DARK_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_BASEDATA_MID_DARK_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_M_DARK_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_BASEDATA_MID_DARK_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_L_DARK_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_BASEDATA_DARK_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_DARK_BASE_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_BASEDATA_DARK_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_CIRCLEDATA_COLLECT: {
                /*case OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT:*/
                eng_result.cmdId = CMD_FT_CAPTURE_CIRCLEDATA_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CIRCLEDATA_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT: {
                /*case OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT:*/
                eng_result.cmdId = CMD_FT_CAPTURE_FLESH_CIRCLEDATA_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT TEST ");
                break;
            }			
            case OPERATION_STEP_CHARTDATA_COLLECT: {
                eng_result.cmdId = CMD_FT_CAPTURE_CHECKBOX_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CHARTDATA_COLLECT TEST ");
                break;
            }
            case OPERATION_STEP_GET_KB_CALIBRATION: {
                eng_result.cmdId = CMD_FT_CAPTURE_CHART_TEST;
                snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_GET_KB_CALIBRATION TEST ");
                break;
            }
            case OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS: {
                if (NULL == cal_result) {
                    LOG_D(LOG_TAG, "[%s]cal_result is null", __func__);
                    break;
                }
                if (opticalType == DELMAR_OPTICAL_TYPE_7_0) {
                    snprintf(tmp_result, sizeof(tmp_result), "OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS TEST\n"
                        "MINDIFFFLESHHM:%d,MINDIFFFLESHHM_MIN:%d,MINDIFFFLESHHM_RESULT:%s,(>=%d)\n"
                        "MINDIFFFLESHML:%d,MINDIFFFLESHML_MIN:%d,MINDIFFFLESHML_RESULT:%s,(>=%d)\n"
                        "MINDIFFBLACKHM:%d,MINDIFFBLACKHM_MIN:%d,MINDIFFBLACKHM_RESULT:%s,(>=%d)\n"
                        "MINDIFFBLACKML:%d,MINDIFFBLACKML_MIN:%d,MINDIFFBLACKML_RESULT:%s,(>=%d)\n"
                        "MAXLIGHTSTABILITY:%.2f,MAXLIGHTSTABILITY_MAX:%.2f,MAXLIGHTSTABILITY_RESULT:%s,(<%.2f)\n"
                        // bad
                        "BADPIXELNUM:%d,BADPIXELNUM_MAX:%d,BADPIXELNUM_RESULT:%s,(<=%d)\n"
                        "BADBLOCKNUM:%d,BADBLOCKNUM_MAX:%d,BADBLOCKNUM_RESULT:%s,(<=%d)\n"
                        "BADPIXELALLBLOCKS:%d,BADPIXELALLBLOCKS_MAX:%d,BADPIXELALLBLOCKS_RESULT:%s,(<=%d)\n"
                        "BADLINENUM:%d,BADLINENUM_MAX:%d,BADLINENUM_RESULT:%s,(<=%d)\n"
                        "BADBLOCKLARGEST:%d,BADBLOCKLARGEST_MAX:%d,BADBLOCKLARGEST_RESULT:%s,(<=%d)\n"
                        "HOTPIXELNUM:%d,HOTPIXELNUM_MAX:%d,HOTPIXELNUM_RESULT:%s,(<=%d)\n"
                        "HOTBLOCKLARGEST:%d,HOTBLOCKLARGEST_MAX:%d,HOTBLOCKLARGEST_RESULT:%s,(<=%d)\n"
                        "HOTLINENUM:%d,HOTLINENUM_MAX:%d,HOTLINENUM_RESULT:%s,(<=%d)\n"
                        // "LOWCORRPITCH_LPF:%d,LOWCORRPITCH_LPF_MAX:%d,LOWCORRPITCH_LPF_RESULT:%s,(<=%d)\n"
                        "DPAADIFFDARK:%d,DPAADIFFDARK_MAX:%d,DPAADIFFDARK_RESULT:%s,(<=%d)\n"
                        "DPBADPIXELNUM:%d,DPBADPIXELNUM_MAX:%d,DPBADPIXELNUM_RESULT:%s,(<=%d)\n"
                        "DPBADPIXELINROW:%d,DPBADPIXELINROW_MAX:%d,DPBADPIXELINROW_RESULT:%s,(<=%d)\n"
                        "DPSNOISEDARKN:%.2f,DPSNOISEDARKN_MAX:%.2f,DPSNOISEDARKN_RESULT:%s,(<=%.2f)\n"
                        // HAF
                        // "HAFBADPIXELNUM:%d,HAFBADPIXELNUM_MAX:%d,HAFBADPIXELNUM_RESULT:%s,(<=%d)\n"
                        // "HAFBADBLOCKNUM:%d,HAFBADBLOCKNUM_MAX:%d,HAFBADBLOCKNUM_RESULT:%s,(<=%d)\n"
                        // fake
                        "ANTIFAKEDX:%.2f,ANTIFAKEDX_MIN:%d,ANTIFAKEDX_MAX:%d,ANTIFAKEDX_RESULT:%s,(%d<=x<=%d)\n"
                        "ANTIFAKEDY:%.2f,ANTIFAKEDY_MIN:%d,ANTIFAKEDY_MAX:%d,ANTIFAKEDY_RESULT:%s,(%d<=x<=%d)\n"
                        // noise
                        "LIGHTHIGHMEAN:%d,LIGHTHIGHMEAN_MIN:%d,LIGHTHIGHMEAN_MAX:%d,LIGHTHIGHMEAN_RESULT:%s,(%d<=x<=%d)\n"
                        "TNOISEDARKN:%.2f,TNOISEDARKN_MAX:%.2f,TNOISEDARKN_RESULT:%s,(<=%.2f)\n"
                        "TNOISELIGHTN:%.2f,TNOISELIGHTN_MAX:%.2f,TNOISELIGHTN_RESULT:%s,(<=%.2f)\n"
                        "SNOISEDARKN:%.2f,SNOISEDARKN_MAX:%.2f,SNOISEDARKN_RESULT:%s,(<=%.2f)\n"
                        "TNOISELIGHTN:%.2f,TNOISELIGHTN_MAX:%d,TNOISELIGHTN_RESULT:%s,(<=%d)\n"
                        "SNOISEFLATL:%.2f,SNOISEFLATL_MAX:%d,SNOISEFLATL_RESULT:%s,(<=%d)\n"
                        "SIGNALLLOW:%.2f,SIGNALLLOW_MIN:%.2f,SIGNALLLOW_RESULT:%s,(>=%.2f)\n"
                        "SIGNALLHIGH:%.2f,SIGNALLHIGH_MIN:%.2f,SIGNALLHIGH_RESULT:%s,(>=%.2f)\n"
                        "TSNRLOW:%.3f,TSNRLOW_MIN:%.3f,TSNRLOW_RESULT:%s,(>=%.3f)\n"
                        "TSNRHIGH:%.3f,TSNRHIGH_MIN:%.3f,TSNRHIGH_RESULT:%s,(>=%.3f)\n"
                        "DIFFBLACKLD:%d,DIFFBLACKLD_MIN:%d,TSNRHIGH_RESULT:%s,(>=%d)\n"
                        // "SHAPENESS_LPF:%f,SHAPENESS_LPF_MIN:%f,SHAPENESS_LPF_RESULT:%s,(>=%f)\n"
                        "ASSEMBLYXOFFSET:%.2f,ASSEMBLYXOFFSET_MAX:%.2f,ASSEMBLYXOFFSET_RESULT:%s,(<=%.2f)\n"
                        "ASSEMBLYYOFFSET:%.2f,ASSEMBLYYOFFSET_MAX:%.2f,ASSEMBLYYOFFSET_RESULT:%s,(<=%.2f)\n"
                        "ASSEMBLYANGELOFFSET:%.2f,ASSEMBLYANGELOFFSET_MAX:%.2f,ASSEMBLYANGELOFFSET_RESULT:%s,(x<=%.2f)\n"
                        "POLARDEGREE:%.2f, POLARDEGREE_MIN:%.2f, POLARDEGREE_RESULT:%s,(>=%.2f)\n"
                        // light
                        "LIGHTLEAKDARK:%d,LIGHTLEAKDARK_MAX:%d,LIGHTLEAKDARK_RESULT:%s,(<=%d)\n"
                        "LIGHTLEAKRATIO:%.2f,LIGHTLEAKRATIO_MAX:%.2f,LIGHTLEAKRATIO_RESULT:%s,(<=%.2f)\n"
                        "DIFFOFFSET:%d,DIFFOFFSET_MAX:%d,DIFFOFFSET_RESULT:%s,(<=%d)\n"
                        "VALIDAREARATIO:%.2f,VALIDAREARATIO_MIN:%.2f,VALIDAREARATIO_RESULT:%s,(>=%.2f)\n"
                        "CHARTDIRECTION:%d,CHARTDIRECTION_MIN:%d,CHARTDIRECTION_MAX:%d,CHARTDIRECTION_RESULT:%s,()\n",

                        cal_result->nDiffFleshHM[0], mNewTestConfig.minDiffFleshHM, cal_result->nDiffFleshHM[0] >= mNewTestConfig.minDiffFleshHM ? "PASS":"FAILED", mNewTestConfig.minDiffFleshHM,
                        cal_result->nDiffFleshML[0], mNewTestConfig.minDiffFleshML, cal_result->nDiffFleshML[0] >= mNewTestConfig.minDiffFleshML ? "PASS":"FAILED", mNewTestConfig.minDiffFleshML,
                        cal_result->nDiffBlackHM[0], mNewTestConfig.minDiffBlackHM, cal_result->nDiffBlackHM[0] >= mNewTestConfig.minDiffBlackHM ? "PASS":"FAILED", mNewTestConfig.minDiffBlackHM,
                        cal_result->nDiffBlackML[0], mNewTestConfig.minDiffBlackML, cal_result->nDiffBlackML[0] >= mNewTestConfig.minDiffBlackML ? "PASS":"FAILED", mNewTestConfig.minDiffBlackML,
                        cal_result->nLightStability[0]/1024.0, mNewTestConfig.maxLightStability, cal_result->nLightStability[0]/1024.0 < mNewTestConfig.maxLightStability?"PASS" : "FAILED", mNewTestConfig.maxLightStability,
                        // bad
                        cal_result->nBadPixelNum[0], mNewTestConfig.maxBadPixelNum, cal_result->nBadPixelNum[0] <= mNewTestConfig.maxBadPixelNum ? "PASS":"FAILED", mNewTestConfig.maxBadPixelNum,
                        cal_result->nBadBlockNum[0], mNewTestConfig.maxBadBlockNum, cal_result->nBadBlockNum[0] <= mNewTestConfig.maxBadBlockNum ? "PASS" : "FAILED", mNewTestConfig.maxBadBlockNum,
                        cal_result->nBadPixelAllBlocks[0], mNewTestConfig.maxBadPixelAllBlocks, cal_result->nBadPixelAllBlocks[0] <= mNewTestConfig.maxBadPixelAllBlocks ? "PASS" : "FAILED", mNewTestConfig.maxBadPixelAllBlocks,
                        cal_result->nBadLineNum[0], mNewTestConfig.maxBadLineNum, cal_result->nBadLineNum[0] <= mNewTestConfig.maxBadLineNum ? "PASS":"FAILED", mNewTestConfig.maxBadLineNum,
                        cal_result->nBadBlockLargest[0], mNewTestConfig.maxBadBlockLargest, cal_result->nBadBlockLargest[0] <= mNewTestConfig.maxBadBlockLargest ? "PASS":"FAILED", mNewTestConfig.maxBadBlockLargest,
                        cal_result->nHotPixelNum[0], mNewTestConfig.maxHotPixelLowNum, cal_result->nHotPixelNum[0] <= mNewTestConfig.maxHotPixelLowNum ? "PASS":"FAILED", mNewTestConfig.maxHotPixelLowNum,
                        cal_result->nHotBlockLargest[0], mNewTestConfig.maxHotBlockLargest, cal_result->nHotBlockLargest[0] <= mNewTestConfig.maxHotBlockLargest ? "PASS":"FAILED", mNewTestConfig.maxHotBlockLargest,
                        cal_result->nHotLineNum[0], mNewTestConfig.maxHotLineNum, cal_result->nHotLineNum[0] <= mNewTestConfig.maxHotLineNum ? "PASS":"FAILED", mNewTestConfig.maxHotLineNum,
                        // cal_result->nLowCorrPitch[0], mNewTestConfig.maxLowCorrPitch, cal_result->nLowCorrPitch[0] <= mNewTestConfig.maxLowCorrPitch ? "PASS":"FAILED", mNewTestConfig.maxLowCorrPitch,
                        cal_result->nDPAADiffDark[0], mNewTestConfig.maxDPAADiffDark, cal_result->nDPAADiffDark[0]<= mNewTestConfig.maxDPAADiffDark ? "PASS" : "FAILED", mNewTestConfig.maxDPAADiffDark,
                        cal_result->nDPBadPixelNum[0], mNewTestConfig.maxDPBadPixelNum, cal_result->nDPBadPixelNum[0]<= mNewTestConfig.maxDPBadPixelNum ? "PASS" : "FAILED", mNewTestConfig.maxDPBadPixelNum,
                        cal_result->nDPBadPixelInRow[0], mNewTestConfig.maxDPBadPixelInRow, cal_result->nDPBadPixelInRow[0]<= mNewTestConfig.maxDPBadPixelInRow ? "PASS" : "FAILED", mNewTestConfig.maxDPBadPixelInRow,
                        cal_result->nDPSNoiseDarkN[0] / 1024.0, mNewTestConfig.maxDPSNoiseDarkN, ((cal_result->nDPSNoiseDarkN[0] / 1024.0)<= mNewTestConfig.maxDPSNoiseDarkN) ? "PASS" : "FAILED", mNewTestConfig.maxDPSNoiseDarkN,
                        // HAF
                        // cal_result->nHAFBadPixelNum[0], mNewTestConfig.maxHAFBadPixelNum, cal_result->nHAFBadPixelNum[0]<= mNewTestConfig.maxHAFBadPixelNum ? "PASS" : "FAILED", mNewTestConfig.maxHAFBadPixelNum,
                        // cal_result->nHAFBadBlockNum[0], mNewTestConfig.maxHAFBadBlockNum, cal_result->nHAFBadBlockNum[0]<= mNewTestConfig.maxHAFBadBlockNum ? "PASS" : "FAILED", mNewTestConfig.maxHAFBadBlockNum,
                        // fake
                        cal_result->nAntiFakeDx[0] / 1024.0,mNewTestConfig.minAntiFakeDx, mNewTestConfig.maxAntiFakeDx,((cal_result->nAntiFakeDx[0] / 1024.0 >= mNewTestConfig.minAntiFakeDx) && (cal_result->nAntiFakeDx[0] / 1024.0 <= mNewTestConfig.maxAntiFakeDx)) ? "PASS" : "FAILED", mNewTestConfig.minAntiFakeDx, mNewTestConfig.maxAntiFakeDx,
                        cal_result->nAntiFakeDy[0] / 1024.0,mNewTestConfig.minAntiFakeDy, mNewTestConfig.maxAntiFakeDy,((cal_result->nAntiFakeDy[0] / 1024.0 >= mNewTestConfig.minAntiFakeDy) && (cal_result->nAntiFakeDy[0] / 1024.0 <= mNewTestConfig.maxAntiFakeDy)) ? "PASS" : "FAILED", mNewTestConfig.minAntiFakeDy, mNewTestConfig.maxAntiFakeDy,
                        // noise
                        cal_result->nLightHighMean[0], mNewTestConfig.minLightHighMean, mNewTestConfig.maxLightHighMean, ((cal_result->nLightHighMean[0] >= mNewTestConfig.minLightHighMean) && (cal_result->nLightHighMean[0] <= mNewTestConfig.maxLightHighMean)) ? "PASS" : "FAILED", mNewTestConfig.minLightHighMean, mNewTestConfig.maxLightHighMean,
                        cal_result->nTNoiseDarkN[0] / 1024.0, mNewTestConfig.maxTNoiseDarkN, ((cal_result->nTNoiseDarkN[0] / 1024.0) <= mNewTestConfig.maxTNoiseDarkN) ? "PASS":"FAILED", mNewTestConfig.maxTNoiseDarkN,
                        cal_result->nTNoiseLightN[0] / 1024.0, mNewTestConfig.maxTNoiseLightN, cal_result->nTNoiseLightN[0] / 1024.0 <= mNewTestConfig.maxTNoiseLightN ? "PASS":"FAILED", mNewTestConfig.maxTNoiseLightN,
                        cal_result->nSNoiseDarkN[0] / 1024.0, mNewTestConfig.maxSNoiseDarkN, cal_result->nSNoiseDarkN[0 ]/ 1024.0 <= mNewTestConfig.maxSNoiseDarkN ? "PASS":"FAILED", mNewTestConfig.maxSNoiseDarkN,
                        cal_result->nSNoiseLightN[0] / 1024.0, mNewTestConfig.maxSNoiseLightN, cal_result->nSNoiseLightN[0] / 1024.0 <= mNewTestConfig.maxSNoiseLightN ? "PASS":"FAILED", mNewTestConfig.maxSNoiseLightN,
                        (cal_result->nSNoiseFlatL[0] / 1024.0), mNewTestConfig.maxSNoiseFlatL, (cal_result->nSNoiseFlatL[0] / 1024.0) <= mNewTestConfig.maxSNoiseFlatL ? "PASS":"FAILED", mNewTestConfig.maxSNoiseFlatL,
                        cal_result->nSignalLLow[0] / 256.0, mNewTestConfig.minSignalLLow, cal_result->nSignalLLow[0] / 256.0 >= mNewTestConfig.minSignalLLow?"PASS":"FAILED", mNewTestConfig.minSignalLLow,
                        cal_result->nSignalLHigh[0] / 256.0, mNewTestConfig.minSignalLHigh, cal_result->nSignalLHigh[0] / 256.0 >= mNewTestConfig.minSignalLHigh?"PASS":"FAILED", mNewTestConfig.minSignalLHigh,
                        /*cal_result->nSignal_LPF[0] / 256.0, mTestConfig.signalLPF, cal_result->nSignal_LPF[0] / 256.0 >= mTestConfig.signalLPF ? "PASS":"FAILED", mTestConfig.signalLPF,*/
                        /*cal_result->nSSNR_LPF[0] / 256.0, mTestConfig.ssnrLPF, cal_result->nSSNR_LPF[0] / 256.0 >= mTestConfig.ssnrLPF ? "PASS":"FAILED", mTestConfig.ssnrLPF,*/
                        cal_result->nTSNRLow[0] / 256.0, mNewTestConfig.minTSNRLow, cal_result->nTSNRLow[0]/256.0 >= mNewTestConfig.minTSNRLow?"PASS" : "FAILED", mNewTestConfig.minTSNRLow,
                        cal_result->nTSNRHigh[0] / 256.0, mNewTestConfig.minTSNRHigh, cal_result->nTSNRHigh[0]/256.0 >= mNewTestConfig.minTSNRHigh?"PASS" : "FAILED", mNewTestConfig.minTSNRHigh,
                        cal_result->nDiffBlackLD[0], mNewTestConfig.minDiffBlackLD, cal_result->nDiffBlackLD[0] >= mNewTestConfig.minDiffBlackLD?"PASS" : "FAILED", mNewTestConfig.minDiffBlackLD,
                        // (double)cal_result->nShapeness_LPF[0] / (1 << 20), mNewTestConfig.minSharpness, (double)cal_result->nShapeness_LPF[0] / (1 << 20)>= mNewTestConfig.minSharpness ? "PASS" : "FAILED", mNewTestConfig.minSharpness,
                        cal_result->nAssemblyXOffset[0], mNewTestConfig.maxAssemblyXOffset, cal_result->nAssemblyXOffset[0] <= mNewTestConfig.maxAssemblyXOffset ? "PASS":"FAILED", mNewTestConfig.maxAssemblyXOffset,
                        cal_result->nAssemblyYOffset[0], mNewTestConfig.maxAssemblyYOffset, cal_result->nAssemblyYOffset[0] <= mNewTestConfig.maxAssemblyYOffset ? "PASS":"FAILED", mNewTestConfig.maxAssemblyYOffset,
                        cal_result->nAssemblyAngelOffset[0], mNewTestConfig.maxAssemblyAngleOffset, (cal_result->nAssemblyAngelOffset[0] <= mNewTestConfig.maxAssemblyAngleOffset) ? "PASS" : "FAILED", mNewTestConfig.maxAssemblyAngleOffset,
                        cal_result->nPolarDegree[0] / 1024.0, mNewTestConfig.minPolarDegree, cal_result->nPolarDegree[0] / 1024.0 >=  mNewTestConfig.minPolarDegree ? "PASS":"FAILED", mNewTestConfig.minPolarDegree,
                        // light
                        cal_result->nLightLeakDark[0], mNewTestConfig.maxLightLeakDark, cal_result->nLightLeakDark[0] <= mNewTestConfig.maxLightLeakDark ? "PASS" : "FAILED", mNewTestConfig.maxLightLeakDark,
                        cal_result->nLightLeakRatio[0] / 256.0, mNewTestConfig.maxLightLeakRatio, (cal_result->nLightLeakRatio[0] / 256.0) <= mNewTestConfig.maxLightLeakRatio ? "PASS" : "FAILED", mNewTestConfig.maxLightLeakRatio,
                        cal_result->nDiffOffset[0], mNewTestConfig.maxDiffOffset, cal_result->nDiffOffset[0] <= mNewTestConfig.maxDiffOffset ? "PASS" : "FAILED", mNewTestConfig.maxDiffOffset,
                        cal_result->nValidAreaRatio[0] / 1024.0, mNewTestConfig.minValidAreaRatio, (cal_result->nValidAreaRatio[0] / 1024.0) >= mNewTestConfig.minValidAreaRatio ? "PASS":"FAILED", mNewTestConfig.minValidAreaRatio,
                        cal_result->nChartDirection[0], mNewTestConfig.maxChartDirection, mNewTestConfig.maxChartDirection, (abs(abs(cal_result->nChartDirection[0])-0) <= mNewTestConfig.maxChartDirection) ? "PASS" : "FAILED");
                } else {
                    snprintf(tmp_result, sizeof(tmp_result), "OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS TEST\n"
                        "MINDIFFFLESHHM:%d,MINDIFFFLESHHM_MIN:%d,MINDIFFFLESHHM_RESULT:%s,(>=%d)\n"
                        "MINDIFFFLESHML:%d,MINDIFFFLESHML_MIN:%d,MINDIFFFLESHML_RESULT:%s,(>=%d)\n"
                        "MINDIFFBLACKHM:%d,MINDIFFBLACKHM_MIN:%d,MINDIFFBLACKHM_RESULT:%s,(>=%d)\n"
                        "MINDIFFBLACKML:%d,MINDIFFBLACKML_MIN:%d,MINDIFFBLACKML_RESULT:%s,(>=%d)\n"
                        "MAXLIGHTSTABILITY:%.2f,MAXLIGHTSTABILITY_MAX:%.2f,MAXLIGHTSTABILITY_RESULT:%s,(<%.2f)\n"
                        // bad
                        "BADPIXELNUM:%d,BADPIXELNUM_MAX:%d,BADPIXELNUM_RESULT:%s,(<=%d)\n"
                        "BADBLOCKNUM:%d,BADBLOCKNUM_MAX:%d,BADBLOCKNUM_RESULT:%s,(<=%d)\n"
                        "BADPIXELALLBLOCKS:%d,BADPIXELALLBLOCKS_MAX:%d,BADPIXELALLBLOCKS_RESULT:%s,(<=%d)\n"
                        "BADLINENUM:%d,BADLINENUM_MAX:%d,BADLINENUM_RESULT:%s,(<=%d)\n"
                        "BADBLOCKLARGEST:%d,BADBLOCKLARGEST_MAX:%d,BADBLOCKLARGEST_RESULT:%s,(<=%d)\n"
                        "HOTPIXELNUM:%d,HOTPIXELNUM_MAX:%d,HOTPIXELNUM_RESULT:%s,(<=%d)\n"
                        "HOTBLOCKLARGEST:%d,HOTBLOCKLARGEST_MAX:%d,HOTBLOCKLARGEST_RESULT:%s,(<=%d)\n"
                        // "LOWCORRPITCH_LPF:%d,LOWCORRPITCH_LPF_MAX:%d,LOWCORRPITCH_LPF_RESULT:%s,(<=%d)\n"
                        "DPAADIFFDARK:%d,DPAADIFFDARK_MAX:%d,DPAADIFFDARK_RESULT:%s,(<=%d)\n"
                        "DPBADPIXELNUM:%d,DPBADPIXELNUM_MAX:%d,DPBADPIXELNUM_RESULT:%s,(<=%d)\n"
                        "DPBADPIXELINROW:%d,DPBADPIXELINROW_MAX:%d,DPBADPIXELINROW_RESULT:%s,(<=%d)\n"
                        "DPSNOISEDARKN:%.2f,DPSNOISEDARKN_MAX:%.2f,DPSNOISEDARKN_RESULT:%s,(<=%.2f)\n"
                        // HAF
                        "HAFBADPIXELNUM:%d,HAFBADPIXELNUM_MAX:%d,HAFBADPIXELNUM_RESULT:%s,(<=%d)\n"
                        "HAFBADBLOCKNUM:%d,HAFBADBLOCKNUM_MAX:%d,HAFBADBLOCKNUM_RESULT:%s,(<=%d)\n"
                        // noise
                        "LIGHTHIGHMEAN:%d,LIGHTHIGHMEAN_MIN:%d,LIGHTHIGHMEAN_MAX:%d,LIGHTHIGHMEAN_RESULT:%s,(%d<=x<=%d)\n"
                        "TNOISEDARKN:%.2f,TNOISEDARKN_MAX:%.2f,TNOISEDARKN_RESULT:%s,(<=%.2f)\n"
                        "TNOISELIGHTN:%.2f,TNOISELIGHTN_MAX:%.2f,TNOISELIGHTN_RESULT:%s,(<=%.2f)\n"
                        "SNOISEDARKN:%.2f,SNOISEDARKN_MAX:%.2f,SNOISEDARKN_RESULT:%s,(<=%.2f)\n"
                        "TNOISELIGHTN:%.2f,TNOISELIGHTN_MAX:%d,TNOISELIGHTN_RESULT:%s,(<=%d)\n"
                        "SNOISEFLATL:%.2f,SNOISEFLATL_MAX:%d,SNOISEFLATL_RESULT:%s,(<=%d)\n"
                        "SIGNALL:%.2f,SIGNALL_MIN:%.2f,SIGNALL_RESULT:%s,(>=%.2f)\n"
                        "TSNR:%.3f,TSNR_MIN:%.3f,TSNR_RESULT:%s,(>=%.3f)\n"
                        // "SHAPENESS_LPF:%f,SHAPENESS_LPF_MIN:%f,SHAPENESS_LPF_RESULT:%s,(>=%f)\n"
                        "ASSEMBLYXOFFSET:%.2f,ASSEMBLYXOFFSET_MAX:%.2f,ASSEMBLYXOFFSET_RESULT:%s,(<=%.2f)\n"
                        "ASSEMBLYYOFFSET:%.2f,ASSEMBLYYOFFSET_MAX:%.2f,ASSEMBLYYOFFSET_RESULT:%s,(<=%.2f)\n"
                        "ASSEMBLYANGELOFFSET:%.2f,ASSEMBLYANGELOFFSET_MAX:%.2f,ASSEMBLYANGELOFFSET_RESULT:%s,(x<=%.2f)\n"
                        // light
                        "LIGHTLEAKDARK:%d,LIGHTLEAKDARK_MAX:%d,LIGHTLEAKDARK_RESULT:%s,(<=%d)\n"
                        "LIGHTLEAKRATIO:%.2f,LIGHTLEAKRATIO_MAX:%.2f,LIGHTLEAKRATIO_RESULT:%s,(<=%.2f)\n"
                        "DIFFOFFSET:%d,DIFFOFFSET_MAX:%d,DIFFOFFSET_RESULT:%s,(<=%d)\n"
                        "VALIDAREARATIO:%.2f,VALIDAREARATIO_MIN:%.2f,VALIDAREARATIO_RESULT:%s,(>=%.2f)\n"
                        "CHARTDIRECTION:%d,CHARTDIRECTION_MIN:%d,CHARTDIRECTION_MAX:%d,CHARTDIRECTION_RESULT:%s,()\n",

                        cal_result->nDiffFleshHM[0], mNewTestConfig.minDiffFleshHM, cal_result->nDiffFleshHM[0] >= mNewTestConfig.minDiffFleshHM ? "PASS":"FAILED", mNewTestConfig.minDiffFleshHM,
                        cal_result->nDiffFleshML[0], mNewTestConfig.minDiffFleshML, cal_result->nDiffFleshML[0] >= mNewTestConfig.minDiffFleshML ? "PASS":"FAILED", mNewTestConfig.minDiffFleshML,
                        cal_result->nDiffBlackHM[0], mNewTestConfig.minDiffBlackHM, cal_result->nDiffBlackHM[0] >= mNewTestConfig.minDiffBlackHM ? "PASS":"FAILED", mNewTestConfig.minDiffBlackHM,
                        cal_result->nDiffBlackML[0], mNewTestConfig.minDiffBlackML, cal_result->nDiffBlackML[0] >= mNewTestConfig.minDiffBlackML ? "PASS":"FAILED", mNewTestConfig.minDiffBlackML,
                        cal_result->nLightStability[0]/1024.0, mNewTestConfig.maxLightStability, cal_result->nLightStability[0]/1024.0 < mNewTestConfig.maxLightStability?"PASS" : "FAILED", mNewTestConfig.maxLightStability,
                        // bad
                        cal_result->nBadPixelNum[0], mNewTestConfig.maxBadPixelNum, cal_result->nBadPixelNum[0] <= mNewTestConfig.maxBadPixelNum ? "PASS":"FAILED", mNewTestConfig.maxBadPixelNum,
                        cal_result->nBadBlockNum[0], mNewTestConfig.maxBadBlockNum, cal_result->nBadBlockNum[0] <= mNewTestConfig.maxBadBlockNum ? "PASS" : "FAILED", mNewTestConfig.maxBadBlockNum,
                        cal_result->nBadPixelAllBlocks[0], mNewTestConfig.maxBadPixelAllBlocks, cal_result->nBadPixelAllBlocks[0] <= mNewTestConfig.maxBadPixelAllBlocks ? "PASS" : "FAILED", mNewTestConfig.maxBadPixelAllBlocks,
                        cal_result->nBadLineNum[0], mNewTestConfig.maxBadLineNum, cal_result->nBadLineNum[0] <= mNewTestConfig.maxBadLineNum ? "PASS":"FAILED", mNewTestConfig.maxBadLineNum,
                        cal_result->nBadBlockLargest[0], mNewTestConfig.maxBadBlockLargest, cal_result->nBadBlockLargest[0] <= mNewTestConfig.maxBadBlockLargest ? "PASS":"FAILED", mNewTestConfig.maxBadBlockLargest,
                        cal_result->nHotPixelNum[0], mNewTestConfig.maxHotPixelLowNum, cal_result->nHotPixelNum[0] <= mNewTestConfig.maxHotPixelLowNum ? "PASS":"FAILED", mNewTestConfig.maxHotPixelLowNum,
                        cal_result->nHotBlockLargest[0], mNewTestConfig.maxHotBlockLargest, cal_result->nHotBlockLargest[0] <= mNewTestConfig.maxHotBlockLargest ? "PASS":"FAILED", mNewTestConfig.maxHotBlockLargest,
                        // cal_result->nLowCorrPitch[0], mNewTestConfig.maxLowCorrPitch, cal_result->nLowCorrPitch[0] <= mNewTestConfig.maxLowCorrPitch ? "PASS":"FAILED", mNewTestConfig.maxLowCorrPitch,
                        cal_result->nDPAADiffDark[0], mNewTestConfig.maxDPAADiffDark, cal_result->nDPAADiffDark[0]<= mNewTestConfig.maxDPAADiffDark ? "PASS" : "FAILED", mNewTestConfig.maxDPAADiffDark,
                        cal_result->nDPBadPixelNum[0], mNewTestConfig.maxDPBadPixelNum, cal_result->nDPBadPixelNum[0]<= mNewTestConfig.maxDPBadPixelNum ? "PASS" : "FAILED", mNewTestConfig.maxDPBadPixelNum,
                        cal_result->nDPBadPixelInRow[0], mNewTestConfig.maxDPBadPixelInRow, cal_result->nDPBadPixelInRow[0]<= mNewTestConfig.maxDPBadPixelInRow ? "PASS" : "FAILED", mNewTestConfig.maxDPBadPixelInRow,
                        cal_result->nDPSNoiseDarkN[0] / 1024.0, mNewTestConfig.maxDPSNoiseDarkN, ((cal_result->nDPSNoiseDarkN[0] / 1024.0)<= mNewTestConfig.maxDPSNoiseDarkN) ? "PASS" : "FAILED", mNewTestConfig.maxDPSNoiseDarkN,
                        // HAF
                        cal_result->nHAFBadPixelNum[0], mNewTestConfig.maxHAFBadPixelNum, cal_result->nHAFBadPixelNum[0]<= mNewTestConfig.maxHAFBadPixelNum ? "PASS" : "FAILED", mNewTestConfig.maxHAFBadPixelNum,
                        cal_result->nHAFBadBlockNum[0], mNewTestConfig.maxHAFBadBlockNum, cal_result->nHAFBadBlockNum[0]<= mNewTestConfig.maxHAFBadBlockNum ? "PASS" : "FAILED", mNewTestConfig.maxHAFBadBlockNum,
                        // noise
                        cal_result->nLightHighMean[0], mNewTestConfig.minLightHighMean, mNewTestConfig.maxLightHighMean, ((cal_result->nLightHighMean[0] >= mNewTestConfig.minLightHighMean) && (cal_result->nLightHighMean[0] <= mNewTestConfig.maxLightHighMean)) ? "PASS" : "FAILED", mNewTestConfig.minLightHighMean, mNewTestConfig.maxLightHighMean,
                        cal_result->nTNoiseDarkN[0] / 1024.0, mNewTestConfig.maxTNoiseDarkN, ((cal_result->nTNoiseDarkN[0] / 1024.0) <= mNewTestConfig.maxTNoiseDarkN) ? "PASS":"FAILED", mNewTestConfig.maxTNoiseDarkN,
                        cal_result->nTNoiseLightN[0] / 1024.0, mNewTestConfig.maxTNoiseLightN, cal_result->nTNoiseLightN[0] / 1024.0 <= mNewTestConfig.maxTNoiseLightN ? "PASS":"FAILED", mNewTestConfig.maxTNoiseLightN,
                        cal_result->nSNoiseDarkN[0] / 1024.0, mNewTestConfig.maxSNoiseDarkN, cal_result->nSNoiseDarkN[0 ]/ 1024.0 <= mNewTestConfig.maxSNoiseDarkN ? "PASS":"FAILED", mNewTestConfig.maxSNoiseDarkN,
                        cal_result->nSNoiseLightN[0] / 1024.0, mNewTestConfig.maxSNoiseLightN, cal_result->nSNoiseLightN[0] / 1024.0 <= mNewTestConfig.maxSNoiseLightN ? "PASS":"FAILED", mNewTestConfig.maxSNoiseLightN,
                        (cal_result->nSNoiseFlatL[0] / 1024.0), mNewTestConfig.maxSNoiseFlatL, (cal_result->nSNoiseFlatL[0] / 1024.0) <= mNewTestConfig.maxSNoiseFlatL ? "PASS":"FAILED", mNewTestConfig.maxSNoiseFlatL,
                        cal_result->nSignalL[0] / 256.0, mNewTestConfig.minSignalL, cal_result->nSignalL[0] / 256.0 >= mNewTestConfig.minSignalL?"PASS":"FAILED", mNewTestConfig.minSignalL,
                        cal_result->nTSNR[0] / 256.0, mNewTestConfig.minTSNR, cal_result->nTSNR[0]/256.0 >= mNewTestConfig.minTSNR?"PASS" : "FAILED", mNewTestConfig.minTSNR,
                        // (double)cal_result->nShapeness_LPF[0] / (1 << 20), mNewTestConfig.minSharpness, (double)cal_result->nShapeness_LPF[0] / (1 << 20)>= mNewTestConfig.minSharpness ? "PASS" : "FAILED", mNewTestConfig.minSharpness,
                        cal_result->nAssemblyXOffset[0], mNewTestConfig.maxAssemblyXOffset, cal_result->nAssemblyXOffset[0] <= mNewTestConfig.maxAssemblyXOffset ? "PASS":"FAILED", mNewTestConfig.maxAssemblyXOffset,
                        cal_result->nAssemblyYOffset[0], mNewTestConfig.maxAssemblyYOffset, cal_result->nAssemblyYOffset[0] <= mNewTestConfig.maxAssemblyYOffset ? "PASS":"FAILED", mNewTestConfig.maxAssemblyYOffset,
                        cal_result->nAssemblyAngelOffset[0], mNewTestConfig.maxAssemblyAngleOffset, (cal_result->nAssemblyAngelOffset[0] <= mNewTestConfig.maxAssemblyAngleOffset) ? "PASS" : "FAILED", mNewTestConfig.maxAssemblyAngleOffset,
                        // light
                        cal_result->nLightLeakDark[0], mNewTestConfig.maxLightLeakDark, cal_result->nLightLeakDark[0] <= mNewTestConfig.maxLightLeakDark ? "PASS" : "FAILED", mNewTestConfig.maxLightLeakDark,
                        cal_result->nLightLeakRatio[0] / 256.0, mNewTestConfig.maxLightLeakRatio, (cal_result->nLightLeakRatio[0] / 256.0) <= mNewTestConfig.maxLightLeakRatio ? "PASS" : "FAILED", mNewTestConfig.maxLightLeakRatio,
                        cal_result->nDiffOffset[0], mNewTestConfig.maxDiffOffset, cal_result->nDiffOffset[0] <= mNewTestConfig.maxDiffOffset ? "PASS" : "FAILED", mNewTestConfig.maxDiffOffset,
                        cal_result->nValidAreaRatio[0] / 1024.0, mNewTestConfig.minValidAreaRatio, (cal_result->nValidAreaRatio[0] / 1024.0) >= mNewTestConfig.minValidAreaRatio ? "PASS":"FAILED", mNewTestConfig.minValidAreaRatio,
                        cal_result->nChartDirection[0], mNewTestConfig.maxChartDirection, mNewTestConfig.maxChartDirection, (abs(abs(cal_result->nChartDirection[0])-0) <= mNewTestConfig.maxChartDirection) ? "PASS" : "FAILED");
                }
                LOG_D(LOG_TAG, "[%s]xy_info: x= %.2f,y = %.2f,an= %.2f", __func__, cal_result->nAssemblyXOffset[0], cal_result->nAssemblyYOffset[0], cal_result->nAssemblyAngelOffset[0]);
                LOG_D(LOG_TAG, "[%s]xy_info: standardCenterX= %f,standardCenterX= %f", __func__, mNewTestConfig.standardCenterX, mNewTestConfig.standardCenterY);
                memcpy(eng_result.info, tmp_result, sizeof(tmp_result));
                eng_result.cmdId = CMD_SENSOR_NAME;
                break;
            }

            default: {
                break;
            }
        }
        if (err == GF_SUCCESS) {
            snprintf(eng_result.info, sizeof(eng_result.info), "%sPASS", eng_result.info);
            eng_result.result = 1;
        } else {
            snprintf(eng_result.info, sizeof(eng_result.info), "%sFAIL:%s", eng_result.info, gf_strerror(err));
            eng_result.result = -1;
        }
        if (!go_on) {
            notifyEngResult(&eng_result);
        }

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    void CustomizedDelmarProductTest:: notifySpiTestCmd(const int8_t *result, int32_t resultLen) {
        gf_error_t err = GF_SUCCESS;
        uint32_t sensorNum = ((DelmarSensor*) mContext->mSensor)->getAvailableSensorNum();
        fp_eng_tests_result eng_result;
        gf_delmar_sensor_info_t sensor_info;

        UNUSED_VAR(resultLen);
        FUNC_ENTER();

        memset((void *) &sensor_info, 0, sizeof(gf_delmar_sensor_info_t));
        getSensorInfo((gf_delmar_sensor_info_t *)&sensor_info, 1);

        /*analysis result*/
        const int8_t* in_buf = result;
        uint32_t token = 0;
        uint32_t err_code = 0;
        uint32_t mcu_chip_id = 0;
        uint32_t sensor_chip_id = 0;
        uint32_t random_number = 0;
        uint32_t flash_id = 0;
        in_buf = TestUtils::testDecodeUint32(&token, in_buf);

        switch (token) {
            case PRODUCT_TEST_TOKEN_ERROR_CODE: {
                in_buf = TestUtils::testDecodeUint32(&err_code, in_buf);
                break;
            }
            case PRODUCT_TEST_TOKEN_MCU_CHIP_ID: {
                in_buf = TestUtils::testDecodeUint32(&mcu_chip_id, in_buf);
                break;
            }
            case PRODUCT_TEST_TOKEN_SENSOR_CHIP_ID: {
                if (sensorNum == 1) {
                    in_buf = TestUtils::testDecodeUint32(&sensor_chip_id, in_buf);
                }
                break;
            }
            case PRODUCT_TEST_TOKEN_RANDOM_NUM: {
                if (sensorNum == 1) {
                    in_buf = TestUtils::testDecodeUint32(&random_number, in_buf);
                }
                break;
            }
            case PRODUCT_TEST_TOKEN_FLASH_CHIP_ID: {
                in_buf = TestUtils::testDecodeUint32(&flash_id, in_buf);
                break;
            }

            default: {
                break;
            }
        }
        if (err == GF_SUCCESS) {
            CustomizedDelmarSensor * cDelmarSensor = (CustomizedDelmarSensor *)mContext->mSensor;

            eng_result.result = 1;
            snprintf(eng_result.info, sizeof(eng_result.info), "%s: SMT1_ID:%d,SMT2_ID:%d,FLASH ID:%d,SENSOR ID:%d,MCU ID:%d,SENSOR VERSION:%d,SENSOR INFO:G6_%d",
                "PASS", sensor_info.o_vendor_id[0], sensor_info.o_vendor_id[1], flash_id, sensor_chip_id, mcu_chip_id, random_number, cDelmarSensor->mSensorType);
        }

        if (reliabilityOrNot) {
            eng_result.cmdId = CMD_RELIABLITY_SENSOR_NAME;
        } else {
            eng_result.cmdId = CMD_FT_SPI_TEST;
        }
        notifyEngResult(&eng_result);
        VOID_FUNC_EXIT();
    }

    void CustomizedDelmarProductTest::notifyTestCmd(int64_t devId, int32_t cmdId,
                                const int8_t *result, int32_t resultLen) {
        VOID_FUNC_ENTER();

        ProductTest::notifyTestCmd(devId, cmdId, result, resultLen);
        switch (cmdId) {
            case PRODUCT_TEST_CMD_SPI: {
                notifySpiTestCmd(result, resultLen);
                break;
            }
            default: {
                break;
            }
        }

        VOID_FUNC_EXIT();
    }

    void CustomizedDelmarProductTest::notifyResetInterruptPin(gf_error_t err) {
        VOID_FUNC_ENTER();

        fp_eng_tests_result eng_result;

        if (err == GF_SUCCESS) {
            eng_result.result = 1;
            snprintf(eng_result.info, sizeof(eng_result.info), "PASS");
        } else {
            eng_result.result = -1;
            snprintf(eng_result.info, sizeof(eng_result.info), "FAIL:%s", gf_strerror(err));
        }
        eng_result.cmdId = CMD_FT_SPI_RST_INT_TEST;
        notifyEngResult(&eng_result);
        DelmarProductTest::notifyResetInterruptPin(err);
        VOID_FUNC_EXIT();
    }

    gf_error_t CustomizedDelmarProductTest::testOTPFLash() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        fp_eng_tests_result eng_result;

        err = DelmarProductTest::testOTPFLash();
        if (err == GF_SUCCESS) {
            LOG_D(LOG_TAG, "OTP PASS PASS")
            eng_result.result = 1;
            snprintf(eng_result.info, sizeof(eng_result.info), "OTP TEST PASS");
        } else {
            LOG_D(LOG_TAG, "OTP FAIL FAIL")
            eng_result.result = -1;
            snprintf(eng_result.info, sizeof(eng_result.info), "OTP TEST FAIL:%s", gf_strerror(err));
        }
        eng_result.cmdId = CMD_FT_MT_CHECK_TEST;
        notifyEngResult(&eng_result);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::findSensor(void) {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_read_image_t cmd = {{ 0 }};
        uint32_t size = sizeof(gf_delmar_read_image_t);
        fp_eng_tests_result eng_result;
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        FUNC_ENTER();
        err = sensor->wakeupSensor();
        do {
            memset(&cmd, 0, size);
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_DELMAR_SENSOR_READ_IMAGE;
            cmd.i_sensor_ids = 1;
            err = invokeCommand(&cmd, size);

            if (GF_SUCCESS == err) {
                //  notifyFindSensorPreviewBmp(err, &cmd);
            } else {
                LOG_E(LOG_TAG, "[%s] gf_hal_find_sensor faild", __func__);
            }

            if (err == GF_SUCCESS) {
                eng_result.result = 1;
                snprintf(eng_result.info, sizeof(eng_result.info), "PASS");
            } else {
                eng_result.result = -1;
                snprintf(eng_result.info, sizeof(eng_result.info), "%s", gf_strerror(err));
            }
            eng_result.cmdId = CMD_CAPTURE_AGING_TEST;
            if (notifyOnce_flag == true) {
                notifyEngResult(&eng_result);
                notifyOnce_flag = false;
            }
        } while (0);
        FUNC_EXIT(err);
        return err;
    }


    void CustomizedDelmarProductTest::agingTimerThread(union sigval v) {
        do {
            if (NULL == v.sival_ptr) {
                LOG_E(LOG_TAG, "[%s] invalid timer signal data", __func__);
                break;
            }

            CustomizedDelmarProductTest* delmarProductTest = static_cast<CustomizedDelmarProductTest*>(v.sival_ptr);
            LOG_E(LOG_TAG, "[%s] invoke_findSensor", __func__);
            delmarProductTest-> findSensor();
        } while (0);
        // TODO implement
        VOID_FUNC_EXIT();
    }

    void CustomizedDelmarProductTest::startAgingTimer(uint32_t intervalSec) {
        VOID_FUNC_ENTER();

        do {
            gf_error_t err = GF_SUCCESS;

            if (NULL == pmAgingTimer) {
                pmAgingTimer = Timer::createTimer((timer_thread_t) agingTimerThread, this);
                if (NULL == pmAgingTimer) {
                    LOG_E(LOG_TAG, "[%s] create aging timer failed", __func__);
                    break;
                }
            }

            err = pmAgingTimer->set(intervalSec, 0, intervalSec, 0);
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] start aging timer failed", __func__);
            }
        } while (0);

        VOID_FUNC_EXIT();
    }

    void CustomizedDelmarProductTest::cancelAgingTimer() {
        VOID_FUNC_ENTER();
        if (pmAgingTimer != NULL) {
            delete pmAgingTimer;
            pmAgingTimer = NULL;
        }
        VOID_FUNC_EXIT();
    }

    void CustomizedDelmarProductTest::notifyEngResult(fp_eng_tests_result *eng_result) {
        VOID_FUNC_ENTER();
        if (eng_notify != NULL) {
            eng_notify(eng_result);
        }
        VOID_FUNC_EXIT();
    }

    gf_error_t CustomizedDelmarProductTest::setEngNotify(fingerprint_eng_notify_t notify) {
        gf_error_t err = GF_SUCCESS;
        int tmp_value = 0x12;
        int *tmp = NULL;
        VOID_FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] CustomizedDelmarProductTest 0x%llx, %p", __func__, (unsigned long long)this, this);
        LOG_E(LOG_TAG, "[%s] tmp 0x%llx, %p", __func__, (unsigned long long)tmp, tmp);
        tmp = &tmp_value;
        LOG_E(LOG_TAG, "[%s] tmp 0x%llx, %p", __func__, (unsigned long long)tmp, tmp);
        eng_notify = notify;
        VOID_FUNC_EXIT();
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::startFactoryTestCmd(uint32_t cmd_id) {
        gf_error_t err = GF_SUCCESS;
        int8_t* out = NULL;
        uint32_t outLen = 0;

        LOG_E(LOG_TAG, "[%s] this %p", __func__, this);
        FUNC_ENTER();

        err = checkBeforeCal(cmd_id);
        if (err != GF_SUCCESS) {
           return err;
        }

        switch (cmd_id) {
            case CMD_FT_INIT_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_INIT, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_DARK_BASE_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_DARK_BASE, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_H_DARK_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_H_DARK, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_L_DARK_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_L_DARK, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_H_FLESH_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_H_FLESH, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_L_FLESH_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_L_FLESH, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_CHECKBOX_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_CHECKBOX, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_CHART_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_CHART, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_EXPO_AUTO_CALIBRATION_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_EXPO_AUTO_CALIBRATION, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_SPI_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_SPI, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_RELIABLITY_SENSOR_NAME : {
                err = onCommand(CMD_TEST_SZ_FACTORY_TEST_GET_RELIABILTY_INFO, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_MT_CHECK_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_MT_CHECK, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_SPI_RST_INT_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_SPI_RST_INT, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_QUALITY_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_KPI, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_CAPTURE_AGING_TEST: {
                err = onCommand(CMD_TEST_SZ_FIND_SENSOR, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_SENSOR_NAME: {
                err = onCommand(CMD_TEST_SZ_FACTORY_TEST_GET_MT_INFO, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_M_FLESH_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_M_FLESH, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_M_DARK_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_M_DARK, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_CIRCLEDATA_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_CIRCLEDATA, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_CAPTURE_FLESH_CIRCLEDATA_TEST: {
                err = onCommand(CMD_TEST_SZ_FT_CAPTURE_FLESH_CIRCLEDATA, NULL, 0, &out, &outLen);
                break;
            }

            default: {
                LOG_E(LOG_TAG, "[%s] cmd %d", __func__, cmd_id);
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::stopFactoryTestCmd(uint32_t cmd_id) {
        gf_error_t err = GF_SUCCESS;
        int8_t* out = NULL;
        uint32_t outLen = 0;

        FUNC_ENTER();

        switch (cmd_id) {
            case CMD_CAPTURE_AGING_TEST:
            case CMD_FT_EXIT_TEST: {
                err = ProductTest::onCommand(CMD_TEST_SZ_FT_EXIT, NULL, 0, &out, &outLen);
                break;
            }
            case CMD_FT_EXPO_AUTO_CALIBRATION_TEST: {
                err = ProductTest::onCommand(CMD_TEST_SZ_FT_STOP_EXPO_AUTO_CALIBRATION, NULL, 0, &out, &outLen);
                break;
            }
            default: {
                LOG_D("%s: default test", __func__);
                break;
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::onEvent(gf_event_type_t e) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        fp_eng_tests_result eng_result;
        LOG_D(LOG_TAG, "[%s] sz factory test get event: %d", __func__, e);

        do {
            if (EVENT_FINGER_DOWN== e) {
                LOG_D(LOG_TAG, "[%s] finger down", __func__);
                gf_customized_image_quality_test_t cmd = {{0}};
                char value[128] = {0};
                CustomizedDevice *local_devce = (CustomizedDevice*)mContext->mDevice;
                cmd.header.cmd_id = GF_CMD_TEST_GET_IMAGE_QUALITY;
                cmd.header.target = GF_TARGET_PRODUCT_TEST;
                cmd.o_is_fake_finger = 0;
                cmd.o_image_quality = 0;
                cmd.i_sensor_index = 0;
                cmd.i_temperature = mContext->mSensor->detectTemperature();
                if ((property_get("ro.boot.project_name", value, NULL) != 0) && (strcmp(value, "19811") == 0)) {
                    cmd.i_coordinate_info.sensor_x[0] = 720;
                    cmd.i_coordinate_info.sensor_y[0] = 2550;
                    LOG_D(LOG_TAG, "19811 sensor center position:720,2550");
                } else {
                    LOG_D(LOG_TAG, "19821 and the same type device sensor center position:540,1910");
                    cmd.i_coordinate_info.sensor_x[0] = 540;
                    cmd.i_coordinate_info.sensor_y[0] = 1910;
                }
                cmd.i_coordinate_info.sensor_rotation[0] = 84;
                if (local_devce->tp_info.x != 0 && local_devce->tp_info.y != 0) {
                    LOG_D(LOG_TAG, "[%s] calculateMask, tp_info_x=%d, tp_info_y=%d", __func__, local_devce->tp_info.x, local_devce->tp_info.y);
                    cmd.i_coordinate_info.press_x = local_devce->tp_info.x;
                    cmd.i_coordinate_info.press_y = local_devce->tp_info.y;
                } else {
                    if ((property_get("ro.boot.project_name", value, NULL) != 0) && (strcmp(value, "19811") == 0)) {
                        cmd.i_coordinate_info.press_x = 720;
                        cmd.i_coordinate_info.press_y = 2550;
                    } else {
                        cmd.i_coordinate_info.press_x = 540;
                        cmd.i_coordinate_info.press_y = 1910;
                    }
                }
                LOG_D(LOG_TAG, "[%s] calculateMask, press_x = %d, press_y = %d", __func__, cmd.i_coordinate_info.press_x, cmd.i_coordinate_info.press_y);

                cmd.i_coordinate_info.radius = 127;
                err = invokeCommand(&cmd, sizeof(gf_customized_image_quality_test_t));
                if (GF_SUCCESS != err) {
                    eng_result.result = -1;
                    snprintf(eng_result.info, sizeof(eng_result.info), "FAIL:%s", gf_strerror(err));
                    LOG_E(LOG_TAG, "[%s] image quality test return failed", __func__);
                } else {
#ifdef SUPPORT_DUMP
                    CustomizedDelmarHalDump *bkdump = static_cast<CustomizedDelmarHalDump *>(mContext->mHalDump);
                    bkdump->dumpImageTestData(err, (uint8_t*)&cmd, sizeof(gf_customized_image_quality_test_t));
#endif  //  dump data
                    if (cmd.o_is_fake_finger == 0 && cmd.o_image_quality > 15 && cmd.o_valid_area > 65) {
                        eng_result.result = 1;
                        snprintf(eng_result.info, sizeof(eng_result.info), "FAKE:PASS \n score:%d,area:%d", cmd.o_image_quality, cmd.o_valid_area);
                    } else {
                        eng_result.result = -1;
                        snprintf(eng_result.info, sizeof(eng_result.info), "FAKE:%s \n score:%d,area:%d", (cmd.o_is_fake_finger == 0) ? "PASS" : "FAIL", cmd.o_image_quality, cmd.o_valid_area);
                        LOG_E(LOG_TAG, "[%s] image quality test not pass", __func__);
                    }
                }
                eng_result.cmdId = CMD_QUALITY_TEST;
                notifyEngResult(&eng_result);
                LOG_D(LOG_TAG, "[%s] is fake finger %d, image_quality %d, valid_area %d", __func__, cmd.o_is_fake_finger, cmd.o_image_quality, cmd.o_valid_area);
            } else {
                err = DelmarProductTest::onEvent(e);
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
