#ifndef _CUSTOMIZEDDELMARPRODUCTTEST_H_
#define _CUSTOMIZEDDELMARPRODUCTTEST_H_

#include "DelmarProductTest.h"
#include "fp_eng_test.h"
#define TABLE(cmdID) {cmdID, #cmdID}

typedef struct SZ_PRODUCT_TEST_CMD_TABLE {
    int32_t cmdID;
    const char *cmdIDString;
} sz_product_test_cmd_table_t;

namespace goodix {

    class CustomizedDelmarProductTest : public DelmarProductTest {
    public:
        explicit CustomizedDelmarProductTest(HalContext* context);
        virtual ~CustomizedDelmarProductTest();
        gf_error_t setEngNotify(fingerprint_eng_notify_t notify);
        gf_error_t startFactoryTestCmd(uint32_t cmd_id);
        gf_error_t stopFactoryTestCmd(uint32_t cmd_id);
        void init(uint8_t opticalType, uint32_t bgVersion);

        fingerprint_eng_notify_t eng_notify = nullptr;

    protected:
        virtual gf_error_t findSensor(void);
        // add for aging test
        virtual void startAgingTimer(uint32_t intervalSec);
        virtual void cancelAgingTimer();
        gf_error_t checkChartDirection(gf_delmar_calculate_cmd_t* cal_cmd, int32_t sensorNum);
        gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);
        bool isNeedCtlSensor(uint32_t cmdId);
        void notifyPerformanceTest(gf_error_t err, uint32_t phase, void *cmd);
        void notifyResetInterruptPin(gf_error_t err);
        gf_error_t testOTPFLash();
        void notifyEngResult(fp_eng_tests_result *eng_result);
        void notifyTestCmd(int64_t devId, int32_t cmdId, const int8_t *result, int32_t resultLen);
        const char *toString(int32_t cmdID);
        void updaetReliabilityThreshold(uint32_t bgVersion);
        gf_error_t onEvent(gf_event_type_t e);
        gf_error_t checkBeforeCal(int32_t cmd);

    private:
        // add for aging test
        Timer* pmAgingTimer = nullptr;
        uint32_t cusSensorBgVersion;
        static void agingTimerThread(union sigval v);
        void notifySpiTestCmd(const int8_t *result, int32_t resultLen);
    };

}  // namespace goodix

#endif  // customizeddelmarproducttest.h
