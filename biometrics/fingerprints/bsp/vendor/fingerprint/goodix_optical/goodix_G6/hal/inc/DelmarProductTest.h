/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _DELMARPRODUCTTEST_H_
#define _DELMARPRODUCTTEST_H_

#include "ProductTest.h"
#include "EventCenter.h"
#include "Mutex.h"
#include "gf_delmar_product_test_types.h"
#include "DelmarFingerprintCore.h"
#include "SensorConfigProvider.h"
#include "DelmarProductTestDefine.h"

namespace goodix {
    class Timer;

    class DelmarProductTest : public ProductTest, public IEventHandler {
    public:
        explicit DelmarProductTest(HalContext* context);
        virtual ~DelmarProductTest();
        void testCaptureImage();
        virtual void onInitFinished();

    protected:
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);
        virtual gf_error_t getConfig(int8_t** cfgBuf, uint32_t* bufLen);
        virtual gf_error_t performanceTest(const int8_t *in, uint32_t inLen);
        virtual gf_error_t testSpi();
        virtual gf_error_t getQRCode();
        virtual gf_error_t testResetInterruptPin();
        virtual gf_error_t testOTPFLash();
        virtual gf_error_t testMorphotype();
        virtual gf_error_t onEvent(gf_event_type_t e);
        virtual void notifyPerformanceTest(gf_error_t err, uint32_t phase, void* cmd);
        virtual void notifyLocationCircleImage(gf_error_t err, void* cmd);
        virtual void notifyLocationCircleTesting(gf_error_t err, void* cmd);
        virtual void notifyResetInterruptPin(gf_error_t err);
        virtual void notifyResetStatus(gf_error_t err, int32_t cmdId);
        bool isPerformanceTestCollectPhase(uint32_t phase);
        virtual void testResetInterruptPinFinished(gf_error_t result);
        virtual bool isNeedLock(int32_t cmdId);
        bool isNeedCtlSensor(uint32_t cmdId);
        virtual gf_error_t checkChartDirection(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        virtual gf_error_t checkGapValueOfEachBrightness(
                gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensor_num);
        virtual gf_error_t checkStabilityOfSwitchingBrightness(
                gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        virtual gf_error_t checkPerformanceThreshold(gf_calculate_cmd_result_t *cal_result);
        virtual gf_error_t getSensorInfo(gf_delmar_sensor_info_t* sensor_info, uint32_t sensor_info_cnt);
        virtual gf_error_t checkChipOffsetAngel(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        virtual gf_error_t checkChipOffsetCoordinate(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        virtual gf_error_t checkBadPoint(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        virtual gf_error_t checkPgaGainThreshold(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        virtual gf_error_t saveProductTestingResult(gf_error_t err);
        virtual gf_error_t testSetGainTargetValue(const int8_t *in, uint32_t inLen);
        virtual gf_error_t testGetGainTargetValue();
        virtual gf_error_t savePerformanceTestResult(char *timeStamp, gf_delmar_calculate_cmd_t *cal_cmd, uint32_t operationStep, uint8_t sensorNum, gf_delmar_product_test_config_t *threshold);
        virtual gf_error_t testAgeStart(const int8_t *in, uint32_t inLen);
        virtual gf_error_t testAgeStop();
        virtual gf_error_t getVersion();
        virtual gf_error_t setSensorConfig(const int8_t *in, uint32_t inLen);
        virtual gf_error_t notifySensorConfig(gf_error_t err_code);
        virtual void ageTestNotify(gf_error_t err, uint8_t isFinal);
        virtual gf_error_t testImageQuality(const int8_t *in, uint32_t inLen);
        virtual uint32_t ageTestGetMaxContinueFailCount();
        virtual uint32_t getLocationCircleCenterOffset();
        virtual gf_error_t testCaptureSingleImage();
        virtual gf_error_t setSamplingConfig(const int8_t *in, uint32_t inLen);
        virtual gf_error_t getSamplingConfig();
        virtual void notifySamplingConfig(gf_error_t err, gf_delmar_sampling_cfg_cmd_t* cmd);
        virtual gf_error_t getCaliState();

    private:
        static void resetInterruptPinTimerThread(union sigval v);

    protected:
        bool mTestingRstIntPin;
        Mutex mRstIntPinMutex;
        Timer* mpRstIntPinTimer;
        DelmarFingerprintCore* mpFingerprintCore;
        gf_delmar_product_test_config_t mTestConfig;
        gf_delmar_new_product_test_threshold_t mNewTestConfig;
        bool mAgeTestStopFlag;
        uint32_t mAgeTestInterval;
        uint64_t mAgeTestCount;
        GoodixSensorConfig mSensorConfig;
        uint64_t mStepStartTime;
        uint32_t mStepKpiTime[OPERATION_STEP_FINISHED - OPERATION_STEP_COLLECT_NONE + 1];

    private:
        uint32_t mAgeTestFailInTotal = 0;  // record fail times in all during the whole age test
    };

}  // namespace goodix


#endif /* _DELMARPRODUCTTEST_H_ */
