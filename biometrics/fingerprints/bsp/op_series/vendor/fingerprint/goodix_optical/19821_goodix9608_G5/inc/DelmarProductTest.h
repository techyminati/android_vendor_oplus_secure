/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
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
#include "DelmarProductTestDefine.h"

namespace goodix {
    class Timer;

    class DelmarProductTest : public ProductTest, public IEventHandler {
    public:
        explicit DelmarProductTest(HalContext* context);
        virtual ~DelmarProductTest();

    protected:
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);
        virtual gf_error_t getConfig(int8_t** cfgBuf, uint32_t* bufLen);
        virtual gf_error_t performanceTest(const int8_t *in, uint32_t inLen);
        virtual gf_error_t testSpi();
        virtual gf_error_t testResetInterruptPin();
        virtual gf_error_t testOTPFLash();
        virtual gf_error_t onEvent(gf_event_type_t e);
        virtual void notifyPerformanceTest(gf_error_t err, uint32_t phase, void* cmd);
        virtual void notifyLocationCircleImage(gf_error_t err, void* cmd);
        virtual void notifyLocationCircleTesting(gf_error_t err, void* cmd);
        virtual void notifyResetInterruptPin(gf_error_t err);
        virtual void notifyResetStatus(gf_error_t err, int32_t cmdId);
        bool isPerformanceTestCollectPhase(uint32_t phase);
        void testResetInterruptPinFinished(gf_error_t result);
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
        virtual gf_error_t checkPgaGainThreshold(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        virtual gf_error_t saveProductTestingResult(gf_error_t err);
        virtual gf_error_t testSetGainTargetValue(const int8_t *in, uint32_t inLen);
        virtual gf_error_t testGetGainTargetValue();

    private:
        static void resetInterruptPinTimerThread(union sigval v);

    protected:
        bool mTestingRstIntPin;
        Mutex mRstIntPinMutex;
        Timer* mpRstIntPinTimer;
        DelmarFingerprintCore* mpFingerprintCore;
        gf_delmar_product_test_config_t mTestConfig;
    };

}  // namespace goodix


#endif /* _DELMARPRODUCTTEST_H_ */
