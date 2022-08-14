/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef _CUSTOMIZEDDELMARPRODUCTTEST_H_
#define _CUSTOMIZEDDELMARPRODUCTTEST_H_

#include "DelmarProductTest.h"

namespace goodix {

    class CustomizedDelmarProductTest : public DelmarProductTest {
    public:
        explicit CustomizedDelmarProductTest(HalContext* context);
        virtual ~CustomizedDelmarProductTest();

    protected:
        virtual gf_error_t testOTPFLash();
        virtual gf_error_t testResetInterruptPin();
        virtual bool isNeedLock(int32_t cmdId);
        void testResetInterruptPinFinished(gf_error_t result);
        gf_error_t checkChartDirection(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        gf_error_t checkGapValueOfEachBrightness(
                        gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensor_num);
        gf_error_t checkStabilityOfSwitchingBrightness(
                        gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        gf_error_t checkPerformanceThreshold(gf_calculate_cmd_result_t *cal_result);
        gf_error_t checkChipOffsetAngel(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        gf_error_t checkChipOffsetCoordinate(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        gf_error_t savePerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold);
        gf_error_t saveOldPerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold);
        gf_error_t saveNewPerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold);
        gf_error_t checkBadPoint(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);

    private:
        void setThresholdNewFlow(uint8_t opticalType, uint32_t bgVersion);
        void setThresholdOldFlow();
        int32_t mProductScreenId;
        gf_error_t mTestResetInterruptResult;
    };

}  // namespace goodix

#endif  // _CUSTOMIZEDDELMARPRODUCTTEST_H_
