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
#include "DelmarHalDump.h"
#include "CustomizedDelmarSensor.h"
#include "CustomizedDevice.h"
#include "DelmarHalUtils.h"

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
        TABLE(OPERATION_STEP_CALCULATE_GAIN_COLLECT),
        TABLE(CMD_TEST_SZ_FT_STOP_EXPO_AUTO_CALIBRATION),
        TABLE(CMD_TEST_SZ_FT_RESET),
        TABLE(PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN),
        TABLE(PRODUCT_TEST_CMD_SPI),
        TABLE(CMD_TEST_SZ_FT_INIT),
        TABLE(CMD_TEST_SZ_FT_EXIT),
        TABLE(CMD_TEST_SZ_FT_CALIBRATE),
        TABLE(PRODUCT_TEST_CMD_OTP_FLASH),
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
        init();
    }

    CustomizedDelmarProductTest::~CustomizedDelmarProductTest() {
    }
    void CustomizedDelmarProductTest::init() {
        mTestConfig.badPointNum = 300;
        mTestConfig.clusterNum = 20;
        mTestConfig.pixelOfLargestBadCluster = 100;
        mTestConfig.lightHBadLineNum = 0;
        mTestConfig.lightVBadLineNum = 0;
        mTestConfig.maxHotConnectedNum = 25;
        mTestConfig.darkTNoise = 3;
        mTestConfig.lightTNoise = 8;
        mTestConfig.darkSNoise = 4;
        mTestConfig.lightSNoise = 300;
        mTestConfig.unorSignalLPF = 50;
        mTestConfig.signalLPF = 70;
        mTestConfig.dataNoiseFlatLPF = 5;
        mTestConfig.bpnInClusters = 120;
        mTestConfig.tSNR = 12;
        mTestConfig.ssnrLPF = 10;
        mTestConfig.sharpnessLPF = 0.35;
        mTestConfig.minLightHighMean = 1400;
        mTestConfig.maxLightHighMean = 3000;
        mTestConfig.minDiffFleshHM = 200;
        mTestConfig.minDiffFleshML = 100;
        mTestConfig.minDiffBlackHM = 100;
        mTestConfig.minDiffBlackML = 50;
        mTestConfig.maxDiffOffset = 1000;
        mTestConfig.maxTNoise = 10;
        mTestConfig.lowCorrPitchLPF = 100;
        mTestConfig.maxValidArea = 500;
        mTestConfig.minChartDirection = -15;
        mTestConfig.maxChartDirection = 15;
        mTestConfig.aaDarkDiff = 22;
        mTestConfig.minAngle = -2.5;
        mTestConfig.maxAngle = 2.5;
        mTestConfig.standardAngle = (90.0 + 6.5);
        mTestConfig.chipCenterOffsetX = 12.0;
        mTestConfig.chipCenterOffsetY = 12.0;
        mTestConfig.standardCenterX = 61;
        mTestConfig.standardCenterY = 101;
        mTestConfig.blackPixelNum = 150;
        mTestConfig.whitePixelNum = 150;
        mTestConfig.lightLeakRatio = 0.75;
        mTestConfig.maxITODepth = 0;
        mTestConfig.dpBadPointNum = 999999;
        mTestConfig.dpMaxBpnInRow = 999999;
        mTestConfig.dpMeanDiff = 25;
        mTestConfig.dPSNoiseDark = 13;
        mTestConfig.dPSNoiseLight = 19;
        mTestConfig.maxHAFBadPointNum = 45;
        mTestConfig.maxHAFBadBlockNum = 7;
        mTestConfig.maxHAFBadRatioNum = 0;
        mTestConfig.realChartDirection = 0;
        mTestConfig.maxDarkLightLeak = 30;
        mTestConfig.maxGhostNum = 100;
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
                    LOG_E(LOG_TAG, "[%s] sensor_index =%d, ChartDirection=%d(%d<= x <=%d)", __func__, sensor_index, cal_cmd->o_result.nChartDirection[sensor_index], mTestConfig.minChartDirection, mTestConfig.maxChartDirection);
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

    gf_error_t CustomizedDelmarProductTest::saveProductTestingResult(gf_error_t err) {
        FUNC_ENTER();
        gf_delmar_save_pass_flag_cmd_t cmd = {{0}};
        uint32_t size = sizeof(gf_delmar_save_pass_flag_cmd_t);
        CustomizedDelmarSensor *sensor = static_cast<CustomizedDelmarSensor *>(mContext->mSensor);

        LOG_D(LOG_TAG, "[%s] saveProductTestingResult  0x%x", __func__, err);
        do {
            cmd.cmd_header.cmd_id = GF_CMD_TEST_SAVE_SPMT_PASS_FLAG;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            cmd.cmd_header.result = err;
            if (GF_SUCCESS != invokeCommand(&cmd, size)) {
                LOG_E(LOG_TAG, "[%s] saveProductTestingResult failed", __func__);
            }
            if (err == GF_SUCCESS) {
                sensor->setSpmtPassOrNot(1);
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

    const int8_t *test1DecodeUint32(uint32_t *value, const int8_t *buf)
    {
        // little endian
        const uint8_t* tmp = (const uint8_t*) buf;
        *value = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;
        buf = buf + sizeof(uint32_t);
        return buf;
    }

    bool reliabilityOrNot = false;
    gf_error_t CustomizedDelmarProductTest::executeCommand(int32_t cmdId, const int8_t *in,
                                                 uint32_t inLen, int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_SUCCESS;
        gf_error_t ret = GF_SUCCESS;
        CustomizedDelmarSensor *sensor = (CustomizedDelmarSensor*) mContext->mSensor;
        FUNC_ENTER();
        const int8_t *in_buf_temp = in;
        uint32_t temp = 0;
        uint32_t operationStep = 0;
        uint32_t token = 0;
        LOG_D(LOG_TAG, "[%s] cmdId=%d (%s)", __func__, cmdId, toString(cmdId));
        int8_t operation[48];
        int8_t *current = operation;

        if (NULL != in_buf_temp) {
            do {
                in_buf_temp = test1DecodeUint32(&token, in_buf_temp);
                switch (token) {
                    case PRODUCT_TEST_TOKEN_COLLECT_PHASE: {
                        in_buf_temp = test1DecodeUint32(&operationStep, in_buf_temp);
                        break;
                    }
                    default: {
                        in_buf_temp = test1DecodeUint32(&temp, in_buf_temp);
                        break;
                    }
                }
            } while (in_buf_temp < in + inLen);
        }
        if ((operationStep >= OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT-5)
            && (operationStep <= OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN -5)) {
            operationStep += 5;
        }

        if (isNeedCtlSensor(cmdId)) {
            err = sensor->wakeupSensor();
            if (err != GF_SUCCESS) {
                notifyResetStatus(err, cmdId);
                FUNC_EXIT(err);
                return err;
            }
        }

        switch (cmdId) {
			case PRODUCT_TEST_CMD_GET_VERSION:
            case PRODUCT_TEST_CMD_SPI:
                LOG_D(LOG_TAG,"PRODUCT_TEST_CMD_SPI for normal calibration test");
                reliabilityOrNot = false;
                err = testSpi();
                break;

            case CMD_TEST_SZ_FACTORY_TEST_GET_RELIABILTY_INFO: {
                LOG_D(LOG_TAG,"PRODUCT_TEST_CMD_SPI for reliability calibration test");
                reliabilityOrNot = true;
                err = testSpi();
                break;
            }
            case PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN: {
                err = testResetInterruptPin();
                if (err != GF_SUCCESS) {
                    LOG_E(LOG_TAG, "[%s] PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN failed", __func__);
                }
            }
            case PRODUCT_TEST_CMD_OTP_FLASH: {
                err = testOTPFLash();
                if (err != GF_SUCCESS) {
                    LOG_E(LOG_TAG, "[%s] testOTPFLash failed", __func__);
                }
                break;
            }
            case PRODUCT_TEST_CMD_PERFORMANCE_TESTING:{
                switch (operationStep) {
                    case OPERATION_STEP_CALCULATE_GAIN_COLLECT: {
                        if (reliabilityOrNot) {
                        LOG_D(LOG_TAG, "RELIABILTY TEST");
                        fp_eng_tests_result eng_result;
#ifdef SUPPORT_DUMP
                        char cur_time_str[GF_DUMP_FILE_PATH_MAX_LEN];
                        DelmarHalUtils::genTimestamp(cur_time_str, sizeof(cur_time_str));
                        DelmarHalDump *bkdump = static_cast<DelmarHalDump *>(mContext->mHalDump);
                        bkdump->setDumpStamp(cur_time_str);
#endif
                        eng_result.cmdId = PRODUCT_TEST_CMD_PERFORMANCE_TESTING;
                        //snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_COLLECT TEST PASS");
                        LOG_E(LOG_TAG, "[%s] the string is %s", __func__, eng_result.info);
                        gf_delmar_sensor_info_t sensor_info;
                        LOG_D(LOG_TAG, "[%s]sensor_info size: %d", __func__, (int32_t)sizeof(gf_delmar_sensor_info_t));
                        memset((void *) &sensor_info, 0, sizeof(gf_delmar_sensor_info_t));
                        getSensorInfo((gf_delmar_sensor_info_t *)&sensor_info, 1);
                        LOG_D(LOG_TAG, "[%s]sensor_info: 0x%x, 0x%x, 0x%x", __func__, *(uint32_t*)(&sensor_info.o_sensor_id[0]), *(uint32_t*)(&sensor_info.o_sensor_id[4]),
                                            *(uint32_t*)(&sensor_info.o_sensor_id[8]));
                        char uid[2*DELMAR_SENSOR_ID_BUFFER_LEN+1] = {0};
                        int j = 0;
                        for (int i = 0; i < DELMAR_SENSOR_ID_BUFFER_LEN; i++) {
                            snprintf(&uid[j], sizeof(uid) - j, "%02x", sensor_info.o_sensor_id[i]);
                            j = j + 2;
                        }
                        uid[2*DELMAR_SENSOR_ID_BUFFER_LEN] = '\0';
                        snprintf(eng_result.info, sizeof(eng_result.info), "CHIPID:0x%s,\nPASS", uid);
                        eng_result.result = 1;
                        notifyEngResult(&eng_result);
                        break;
                        }
                        sensor->setSpmtPassOrNot(0);
                        saveProductTestingResult(GF_ERROR_TEST_SPMT);
                        LOG_D(LOG_TAG, "[%s] delete spmt_pass file and set pass_flag disable.", __func__);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                        }

                    case OPERATION_STEP_CALCULATE_GAIN: {
                        if (reliabilityOrNot) {
                            LOG_D(LOG_TAG, "RELIABILTY TEST");
                            fp_eng_tests_result eng_result;
                            eng_result.cmdId = PRODUCT_TEST_CMD_PERFORMANCE_TESTING;
                            snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN TEST PASS");
                            eng_result.result = 1;
                            notifyEngResult(&eng_result);
                            break;
                        }
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        if (GF_SUCCESS != err) {
                           LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN faild");
                        }
                        else {
                           LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN SUCCESS");
                        }
                        break;
                    }

                    case OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT: {
                         if (reliabilityOrNot) {
                            LOG_D(LOG_TAG, "RELIABILTY TEST");
                            fp_eng_tests_result eng_result;
                            eng_result.cmdId = PRODUCT_TEST_CMD_PERFORMANCE_TESTING;
                            snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN TEST PASS");
                            eng_result.result = 1;
                            notifyEngResult(&eng_result);
                            break;
                        }
                         memset(operation, 0, sizeof(operation));
                         current = operation;
                         current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT);
                         current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                         err = performanceTest(operation, 4*sizeof(int32_t));
                         if (GF_SUCCESS != err) {
                             LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT faild");
                             break;
                         }
                         else {
                             LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT SUCCESS");
                         }
                         break;
                    }

                    case OPERATION_STEP_CALCULATE_GAIN_TWO: {
                        if (reliabilityOrNot) {
                            LOG_D(LOG_TAG, "RELIABILTY TEST");
                            fp_eng_tests_result eng_result;
                            eng_result.cmdId = PRODUCT_TEST_CMD_PERFORMANCE_TESTING;
                            snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN TEST PASS");
                            eng_result.result = 1;
                            notifyEngResult(&eng_result);
                            break;
                        }
                        memset(operation, 0, sizeof(operation));
                        current = operation;
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_TWO);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO faild");
                            break;
                        }
                        else {
                            LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO SUCCESS");
                        }
                        break;
                    }

                    case OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT: {
                        if (reliabilityOrNot) {
                            LOG_D(LOG_TAG, "RELIABILTY TEST");
                            fp_eng_tests_result eng_result;
                            eng_result.cmdId = PRODUCT_TEST_CMD_PERFORMANCE_TESTING;
                            snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN TEST PASS");
                            eng_result.result = 1;
                            notifyEngResult(&eng_result);
                            break;
                        }
                        memset(operation, 0, sizeof(operation));
                        current = operation;
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT faild");
                            break;
                        }
                        else {
                            LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT SUCCESS");
                        }
                        break;
                    }

                    case OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN: {
                        if (reliabilityOrNot) {
                            LOG_D(LOG_TAG, "RELIABILTY TEST");
                            fp_eng_tests_result eng_result;
                            eng_result.cmdId = PRODUCT_TEST_CMD_PERFORMANCE_TESTING;
                            snprintf(eng_result.info, sizeof(eng_result.info), "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN TEST PASS");
                            eng_result.result = 1;
                            notifyEngResult(&eng_result);
                            break;
                        }
                        memset(operation, 0, sizeof(operation));
                        current = operation;
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN faild");
                            break;
                        }
                        else {
                            LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN SUCCESS");
                        }
                        break;
                    }

                    case OPERATION_STEP_BASEDATA_MAX_COLLECT: {
                        LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_H_FLESH");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MAX_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_BASEDATA_MID_COLLECT: {
                        LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_M_FLESH");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MID_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_BASEDATA_MIN_COLLECT: {
                        LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_L_FLESH");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MIN_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_BASEDATA_DARK_COLLECT: {
                        LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_DARK_BASE");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_DARK_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT: {
                        LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_L_DARK");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_BASEDATA_MID_DARK_COLLECT: {
                        LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_M_DARK");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MID_DARK_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT: {
                        LOG_D(LOG_TAG, "CMD_TEST_SZ_FT_CAPTURE_H_DARK");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_CIRCLEDATA_COLLECT: {
                        usleep(200);
                        LOG_D(LOG_TAG, "OPERATION_STEP_CIRCLEDATA_COLLECT sleep 2");
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CIRCLEDATA_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_CHARTDATA_COLLECT: {
                        LOG_D(LOG_TAG, "OPERATION_STEP_CHARTDATA_COLLECT");
                        usleep(200);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CHARTDATA_COLLECT);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_GET_KB_CALIBRATION: {
                        LOG_D(LOG_TAG, "OPERATION_STEP_GET_KB_CALIBRATION");
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_GET_KB_CALIBRATION);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS: {
                        LOG_D(LOG_TAG, "OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS");
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_COLLECT_PHASE, OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS);
                        current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_FEATURE_TYPE, 2);
                        err = performanceTest(operation, 4*sizeof(int32_t));
                        break;
                    }

                    case OPERATION_STEP_FINISHED: {
                        notifyOnce_flag = true;
                        cancelAgingTimer();
                        err = sensor->sleepSensor();
                        fp_set_tpirq_enable(0);
                        mContext->mCenter->deregisterHandler(this);
                        break;
                    }
                }
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

            case PRODUCT_TEST_CMD_IMAGE_QUALITY: {
                LOG_D(LOG_TAG, "[%s] test image kpi", __func__);
                err = sensor->wakeupSensor();
                if (err != GF_SUCCESS) {
                    notifyResetStatus(err, cmdId);
                    FUNC_EXIT(err);
                    return err;
                }
                //mWorkState = STATE_KPI;
                fp_set_tpirq_enable(1);
                mContext->mCenter->registerHandler(this);
                if (err != GF_SUCCESS) {
                    LOG_E(LOG_TAG, "[%s] testImageKPI failed", __func__);
                }
                usleep(700*1000);   //delay for handling onEvent
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
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] execute cmd failed, cmd_id = %d", __func__, cmdId);
        }
        FUNC_EXIT(err);
        return err;
    }


    bool CustomizedDelmarProductTest::isNeedCtlSensor(uint32_t cmdId) {
        bool ret = false;

        switch (cmdId) {
            case PRODUCT_TEST_CMD_SPI:
            case CMD_TEST_SZ_FACTORY_TEST_GET_RELIABILTY_INFO:
            case PRODUCT_TEST_CMD_OTP_FLASH:
            case CMD_TEST_SZ_FT_RESET:
            case PRODUCT_TEST_CMD_PERFORMANCE_TESTING:
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

    void CustomizedDelmarProductTest:: notifySpiTestCmd(const int8_t *result, int32_t resultLen) {
        gf_error_t err = GF_SUCCESS;
        int8_t *test_result = NULL;
        uint32_t len = 0;
        char value[128];
        gf_delmar_test_spi_cmd_t *cmd = NULL;
        uint32_t size = sizeof(gf_delmar_test_spi_cmd_t);
        uint32_t sensorNum = ((DelmarSensor*) mContext->mSensor)->getAvailableSensorNum();
        FUNC_ENTER();
        fp_eng_tests_result eng_result;

        UNUSED_VAR(result);
        UNUSED_VAR(resultLen);
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
        }
        while (0);

        if (NULL != cmd) {
            LOG_D(LOG_TAG,
                  "[%s] mcu_chip_id=0x%04X, sensor_chip_id=0x%04X, flash_id=0x%04X, product_id=0x%04X",
                  __func__, cmd->o_mcu_chip_id, cmd->o_sensor_chip_id[0], cmd->o_flash_id,
                  cmd->o_random_number[0]);
        } else {
            return;
        }
        notifyTestCmd(0, PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN, test_result, len);

        gf_delmar_sensor_info_t sensor_info;
        memset((void *) &sensor_info, 0, sizeof(gf_delmar_sensor_info_t));
        getSensorInfo((gf_delmar_sensor_info_t *)&sensor_info, 1);

        if (err == GF_SUCCESS && (0x5 == sensor_info.o_optical_type) && (0x04 <= sensor_info.o_otp_version)) {
            eng_result.result = 1;
            snprintf(eng_result.info, sizeof(eng_result.info), "%s: SMT1_ID:%d,SMT2_ID:%d,FLASH ID:%d,SENSOR ID:%d,MCU ID:%d,PMIC ID:%d,SENSOR VERSION:%d,G5 LIGHT:%s",
                "PASS", sensor_info.o_vendor_id[0], sensor_info.o_vendor_id[1], cmd->o_flash_id, cmd->o_sensor_chip_id[0], cmd->o_mcu_chip_id, cmd->o_mcu_chip_id, cmd->o_random_number[0],
                ((0x5 == sensor_info.o_optical_type) && (0x04 <= sensor_info.o_otp_version))?"3.0":"2.0");
        } else {
            eng_result.result = -1;
            snprintf(eng_result.info, sizeof(eng_result.info), "%s: SMT1_ID:%d,SMT2_ID:%d,FLASH ID:%d,SENSOR ID:%d,MCU ID:%d,PMIC ID:%d,SENSOR VERSION:%d,G5 LIGHT:%s",
                "FAIL", sensor_info.o_vendor_id[0], sensor_info.o_vendor_id[1], cmd->o_flash_id, cmd->o_sensor_chip_id[0], cmd->o_mcu_chip_id, cmd->o_mcu_chip_id, cmd->o_random_number[0],
                ((0x5 == sensor_info.o_optical_type) && (0x04 <= sensor_info.o_otp_version)) ? "3.0" : "2.0");
        }
        if (err == GF_SUCCESS && (property_get("vendor.boot.hw_version", value, NULL) != 0) && (strcmp(value, "11") == 0)) {
            eng_result.result = 1;
            strncpy(eng_result.info, "PASS", sizeof("PASS"));
            ALOGD("this is T0 device");
        }
        if (reliabilityOrNot) {
            eng_result.cmdId = CMD_RELIABLITY_SENSOR_NAME;
        } else {
            eng_result.cmdId = PRODUCT_TEST_CMD_SPI;
        }
        notifyEngResult(&eng_result);

        if (cmd != NULL) {
            delete cmd;
        }

        if (test_result != NULL) {
            delete []test_result;
        }

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
        eng_result.cmdId = PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN;
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
        eng_result.cmdId = PRODUCT_TEST_CMD_OTP_FLASH;
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
            case CMD_RELIABLITY_SENSOR_NAME : // CMD_RELIABILITY_FT_SPI_TEST
            {
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
            case OPERATION_STEP_CALCULATE_GAIN_COLLECT: {
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

    gf_error_t CustomizedDelmarProductTest::testSpi() {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_sensor_info_t sensor_info = {{0}};
        FUNC_ENTER();
        char value[128];
        do {
            memset(&sensor_info, 0, sizeof(gf_delmar_sensor_info_t));
            getSensorInfo(&sensor_info, 1);
            LOG_D(LOG_TAG, "this is light  type version %x %x", sensor_info.o_optical_type, sensor_info.o_otp_version);
            /*set spmt threhold of light type version 3.0*/
            if ((0x5 == sensor_info.o_optical_type) && (0x04 <= sensor_info.o_otp_version)) {
                mTestConfig.badPointNum = 300;
                mTestConfig.clusterNum = 20;
                mTestConfig.pixelOfLargestBadCluster = 100;
                mTestConfig.lightHBadLineNum = 0;
                mTestConfig.lightVBadLineNum = 0;
                mTestConfig.maxHotConnectedNum = 25;
                mTestConfig.darkTNoise = 3;
                mTestConfig.lightTNoise = 8;
                mTestConfig.darkSNoise = 4;
                mTestConfig.lightSNoise = 300;
                if((property_get("ro.boot.project_name", value, NULL) != 0) && (strcmp(value, "19855") == 0)){
                    LOG_D(LOG_TAG,"this is 19867");
                    mTestConfig.unorSignalLPF = 34.5;
                    mTestConfig.tSNR = 14.5;
                } else if((property_get("ro.boot.project_name", value, NULL) != 0) && (strcmp(value, "19811") == 0)){
                    LOG_D(LOG_TAG,"this is 19811");
                    mTestConfig.unorSignalLPF = 34.5;
                    mTestConfig.tSNR = 14.5;
                } else {
                    LOG_D(LOG_TAG,"this is not 19867&19811");
                    mTestConfig.unorSignalLPF = 26;
                    mTestConfig.tSNR = 11;
                }
                mTestConfig.signalLPF = 1;
                mTestConfig.dataNoiseFlatLPF = 5;
                mTestConfig.bpnInClusters = 130;
                mTestConfig.ssnrLPF = 1;
                mTestConfig.sharpnessLPF = 0.35;
                mTestConfig.minLightHighMean = 1400;
                mTestConfig.maxLightHighMean = 3000;
                mTestConfig.minDiffFleshHM = 200;
                mTestConfig.minDiffFleshML = 100;
                mTestConfig.minDiffBlackHM = 100;
                mTestConfig.minDiffBlackML = 50;
                mTestConfig.maxDiffOffset = 1000;
                mTestConfig.maxTNoise = 10;
                mTestConfig.lowCorrPitchLPF = 100;
                mTestConfig.maxValidArea = 30;
                mTestConfig.minChartDirection = -15;
                mTestConfig.maxChartDirection = 15;
                mTestConfig.aaDarkDiff = 22;
                mTestConfig.minAngle = -2.5;
                mTestConfig.maxAngle = 2.5;
                mTestConfig.standardAngle = (90.0 + 6.5);
                mTestConfig.chipCenterOffsetX = 12.0;
                mTestConfig.chipCenterOffsetY = 12.0;
                mTestConfig.standardCenterX = 60;
                mTestConfig.standardCenterY = 90;
                mTestConfig.blackPixelNum = 150;
                mTestConfig.whitePixelNum = 150;
                mTestConfig.lightLeakRatio = 0.75;
                mTestConfig.maxITODepth = 0;
                mTestConfig.dpBadPointNum = 50;
                mTestConfig.dpMaxBpnInRow = 8;
                mTestConfig.dpMeanDiff = 10;
                mTestConfig.dPSNoiseDark = 13;
                mTestConfig.dPSNoiseLight = 200000;
                mTestConfig.maxHAFBadPointNum = 45;
                mTestConfig.maxHAFBadBlockNum = 7;
                mTestConfig.maxHAFBadRatioNum = 200000;
                mTestConfig.realChartDirection = 0;
                mTestConfig.maxDarkLightLeak = 30;
                mTestConfig.maxGhostNum = 100;
            }
        } while (0);

        err = DelmarProductTest::testSpi();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t CustomizedDelmarProductTest::onEvent(gf_event_type_t e) {
        int8_t *test_result = NULL;
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        fp_eng_tests_result eng_result;
        uint32_t len = 0;

        LOG_D(LOG_TAG, "[%s] sz factory test get event: %d", __func__, e);
        do {
            if (EVENT_FINGER_DOWN== e) {
                LOG_D(LOG_TAG, "[%s] finger down", __func__);
                gf_delmar_image_quality_test_t cmd = {{0}};
                char value[128] = {0};
                CustomizedDevice *local_devce = (CustomizedDevice*)mContext->mDevice;
                cmd.header.cmd_id = GF_CMD_TEST_GET_IMAGE_QUALITY;
                cmd.header.target = GF_TARGET_PRODUCT_TEST;
                cmd.o_is_fake_finger = 0;
                cmd.o_image_quality = 0;
                cmd.i_sensor_index = 0;
                cmd.i_temperature = DelmarAlgoUtils::detectTemperature();
                 if ((property_get("ro.boot.project_name", value, NULL) != 0) && (strcmp(value, "19811") == 0)) {
                    cmd.i_coordinate_info.sensor_x[0] = 720;
                    cmd.i_coordinate_info.sensor_y[0] = 2550;
                } else {
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
                err = invokeCommand(&cmd, sizeof(gf_delmar_image_quality_test_t));
                if (GF_SUCCESS != err) {
                    eng_result.result = -1;
                    snprintf(eng_result.info, sizeof(eng_result.info), "FAIL:%s", gf_strerror(err));
                    LOG_E(LOG_TAG, "[%s] image quality test return failed", __func__);
                } else {
#ifdef SUPPORT_DUMP
                DelmarHalDump *bkdump = static_cast<DelmarHalDump *>(mContext->mHalDump);
                bkdump->dumpImageTestData(err, (uint8_t*)&cmd, sizeof(gf_delmar_image_quality_test_t));
#endif  //  dump data
                //20180518 add for eng test
                if (cmd.o_is_fake_finger == 0 && cmd.o_image_quality > 15 && cmd.o_valid_area > 65) {
                   eng_result.result = 1;
                   snprintf(eng_result.info, sizeof(eng_result.info), "FAKE:PASS \n score:%d,area:%d",cmd.o_image_quality,cmd.o_valid_area);
                } else {
                   if ((cmd.o_is_fake_finger != 0) || cmd.o_valid_area <= 65) {
                        cmd.o_image_quality = 0;
                   }
                   eng_result.result = -1;
                   snprintf(eng_result.info, sizeof(eng_result.info), "FAKE:%s \n score:%d,area:%d",(cmd.o_is_fake_finger == 0) ? "PASS" : "FAIL", cmd.o_image_quality,cmd.o_valid_area);
                   LOG_E(LOG_TAG, "[%s] image quality test not pass", __func__);
                }
                }
                len += HAL_TEST_SIZEOF_INT32;   //errno
                len += HAL_TEST_SIZEOF_INT32;   //quality score
                test_result = new int8_t[len] {0};
                if (NULL == test_result) {
                    len = 0;
                    LOG_E(LOG_TAG, "[%s] test_rest out of memory", __func__);
                    err = GF_ERROR_OUT_OF_MEMORY;
                    break;
                }
                memset(test_result, 0, len);
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, PRODUCT_TEST_TOKEN_IMAGE_QUALITY, (int32_t)cmd.o_image_quality);
                eng_result.cmdId = PRODUCT_TEST_CMD_IMAGE_QUALITY;
                notifyTestCmd(0, PRODUCT_TEST_CMD_IMAGE_QUALITY, test_result, len);
                //notifyEngResult(&eng_result);
                LOG_D(LOG_TAG, "[%s] is fake finger %d, image_quality %d, valid_area %d", __func__, cmd.o_is_fake_finger, cmd.o_image_quality, cmd.o_valid_area);
            } else {
            err = DelmarProductTest::onEvent(e);
            }
           } while (0);
        FUNC_EXIT(err);
        if (test_result != NULL) {
            delete []test_result;
        }
        return err;
    }
}  // namespace goodix
