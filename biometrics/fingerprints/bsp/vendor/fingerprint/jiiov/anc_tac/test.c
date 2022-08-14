#define LOG_TAG "[ANC_CA_TEST]"

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include "anc_error.h"
#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "anc_algorithm.h"
#include "anc_tac_sensor.h"
#include "anc_token.h"
#include "anc_ca_image.h"
#include "anc_memory_wrapper.h"
#include "anc_extension.h"
#include "anc_tac_time.h"
#include "anc_hal_sensor_device.h"
#include "test.h"

#ifdef VIRTUAL_SENSOR
#include "anc_tac_virtual_sensor.h"
#endif

static void SetCurrentThreadAffinity(int32_t tid) {
    long syscallres;
    int mask = 0b10000000;
    syscallres = syscall(__NR_sched_setaffinity, tid, sizeof(mask), &mask);
    ANC_LOGD("syscall tid:%d, setaffinity: mask=%d=0x%x", tid, mask, mask);
    if (syscallres) {
        ANC_LOGE("failed to setaffinity: mask=0x%x err: %s (%d)", mask, strerror(errno), errno);
    }
}

static int enroll_image(int type) {
    int32_t ret_val = ANC_OK;
    uint32_t remaining = 0;
    ANC_BOOL is_finished = ANC_FALSE;
    AncSensorCommandParam sensor_param;

    if (ANC_OK != (ret_val = AlgoInitEnroll())) {
        ANC_LOGE("fail to init enroll");
        goto DO_FAIL;
    }

    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));
    sensor_param.type = ANC_SENSOR_CMD_VC_SETTING;
    sensor_param.data.vc_image_type = type;
    if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
        ANC_LOGE("fail to set sensor param");
        goto DO_FAIL;
    }
    uint32_t finger_id = 0;
    while (1) {
        if (ANC_OK != (ret_val = SensorCaptureImage())) {
            ANC_LOGE("fail to capture image");
            break;
        }

        ret_val = AlgoEnroll(&remaining,&finger_id);
        if (ANC_ALGO_ENROLL_FINISH == ret_val) {
            ANC_LOGD("AlgoEnroll finish");
            is_finished = ANC_TRUE;
            break;
        } else if (ANC_ALGO_ENROLL_OK_CONTINUE == ret_val) {
            ANC_LOGD("AlgoEnroll image ok");
        } else {
            ANC_LOGD("AlogEnroll image err");
        }
        ANC_LOGD("AlgoEnroll ret_val:%d, remaining:%d", ret_val, remaining);

    }

    if (ANC_OK != (ret_val = AlgoDeinitEnroll(&finger_id,is_finished))) {
        ANC_LOGE("fail to deinit enroll");
        goto DO_FAIL;
    }


DO_FAIL :
    return ret_val;
}

static int verify_image(int type) {
    int32_t ret_val = ANC_OK;
    int index = 0;
    int verify_times = 4; // just verify 4 times
    AncSensorCommandParam sensor_param;
    long long verify_time_all = 0;

    if (ANC_OK != (ret_val = AlgoInitVerify())) {
        ANC_LOGE("fail to init verify");
        goto DO_FAIL;
    }

    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));
    sensor_param.type = ANC_SENSOR_CMD_VC_SETTING;
    sensor_param.data.vc_image_type = type;
    if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
        ANC_LOGE("fail to set sensor param");
        goto DO_FAIL;
    }

    do {
        ANC_TIME_MEASURE_START(verify_all);
        if (ANC_OK != (ret_val = SensorCaptureImage())) {
            ANC_LOGE("fail to capture image");
            break;
        }

#ifdef ANC_GET_IMAGE_FROM_TA
        uint8_t *buffer=NULL;
        uint32_t buffer_len=0;
        if (ANC_OK != (ret_val = ExtensionGetImage(&buffer, &buffer_len, 1))) {
            ANC_LOGE("fail to get image");
            goto DO_FAIL;
        }
#endif

        uint32_t finger_id = 0;
        uint32_t retry_count = 0;
        uint32_t need_study = 0;
        uint32_t algo_status = 0;
        if (ANC_ALGO_MATCH_OK == (ret_val = AlgoVerify(&finger_id, &need_study, &algo_status, retry_count))) {
            ANC_LOGD("verify sucess, finger id:%d", finger_id);
        } else {
            ANC_LOGD("verify failed:%d", ret_val);
        }
        ANC_GET_TIME_MEASURE_END(verify_all, "match", &verify_time_all);
        index++;
    } while (index < verify_times);
    ANC_LOGD("verify count : %d, ret_val:%d", index, ret_val);

    if (ANC_OK != (ret_val = AlgoDeinitVerify())) {
        ANC_LOGE("fail to deinit verify");
        goto DO_FAIL;
    }

DO_FAIL :
    return ret_val;
}

int CaInit(void) {
    int32_t ret_val = ANC_OK;

    if (ANC_OK != (ret_val = InitAncCa())) {
        ANC_LOGE("fail to init anc ca");
        goto INIT_CA_FAIL;
    }

    ANC_LOGD("open sensor device");
    if (ANC_OK != (ret_val = OpenSensorDevice())) {
        ANC_LOGE("fail to open sensor device");
        goto OPEN_DEVICE_FAIL;
    }

    ANC_LOGD("power on sensor device");
    if (ANC_OK != (ret_val = SensorDevicePowerOn())) {
        ANC_LOGE("fail to power on sensor device");
        goto POWER_ON_FAIL;
    }

    ANC_LOGD("reset sensor device");
    if (ANC_OK != (ret_val = SensorDeviceReset())) {
        ANC_LOGE("fail to reset sensor device");
        goto RESET_FAIL;
    }

#ifdef ANC_SENSOR_SPI_MTK
    if (ANC_OK != (ret_val = SensorDeviceOpenSpiClk())) {
        ANC_LOGE("fail to open sensor spi clk");
        goto OPEN_SPI_FAIL;
    }
#endif

    ANC_LOGD("init sensor");
    if (ANC_OK != (ret_val = InitSensor())) {
        ANC_LOGE("fail to init sensor, ret_val = %d", ret_val);
        if (ANC_FAIL_LOAD_OTP_DATA == ret_val) {
            ANC_LOGW("fail to load otp data, continue to test");
            ret_val = ANC_OK;
        } else {
            goto INIT_SENSOR_FAIL;
        }
    }

    ANC_LOGD("CA Init Success");
    printf("CA Init Success\n");

    return ret_val;

SLEEP_SENSOR_FAIL :
    ANC_LOGD("deinit sensor");
    if (ANC_OK != DeinitSensor()) {
        ANC_LOGE("fail to deinit sensor");
    }

INIT_SENSOR_FAIL :
#ifdef ANC_SENSOR_SPI_MTK
    SensorDeviceCloseSpiClk();
OPEN_SPI_FAIL :
#endif

RESET_FAIL :
    ANC_LOGD("power off sensor device");
    if (ANC_OK != SensorDevicePowerOff()) {
        ANC_LOGE("fail to power off sensor device");
    }

POWER_ON_FAIL :
    ANC_LOGD("close sensor device");
    if (ANC_OK != CloseSensorDevice()) {
        ANC_LOGE("fail to close sensor device");
    }

OPEN_DEVICE_FAIL :
    if (ANC_OK != DeinitAncCa()) {
        ANC_LOGE("fail to deinit ca");
    }

INIT_CA_FAIL :

    ANC_LOGD("CA Init Fail");
    printf("CA Init Fail\n");

    return ret_val;
}

int CaDeinit(void) {
    int32_t ret_val = ANC_OK;

    ANC_LOGD("deinit sensor");
    if (ANC_OK != (ret_val = DeinitSensor())) {
        ANC_LOGE("fail to deinit sensor");
        goto DO_FAIL;
    }

#ifdef ANC_SENSOR_SPI_MTK
    SensorDeviceCloseSpiClk();
#endif

    ANC_LOGD("power off sensor device");
    if (ANC_OK != (ret_val = SensorDevicePowerOff())) {
        ANC_LOGE("fail to power off sensor device");
        goto DO_FAIL;
    }

    ANC_LOGD("close sensor device");
    if (ANC_OK != (ret_val = CloseSensorDevice())) {
        ANC_LOGE("fail to close sensor device");
        goto DO_FAIL;
    }

    if (ANC_OK != (ret_val = DeinitAncCa())) {
        ANC_LOGE("fail to deinit ca");
        goto DO_FAIL;
    }

    ANC_LOGD("CA DeInit Success");
    printf("CA DeInit Success\n");

    return ret_val;

DO_FAIL:

    ANC_LOGD("CA DeInit Fail");
    printf("CA DeInit Fail\n");

    return ret_val;
}

int DoFullFunctionTest(void) {
    int32_t ret_val = ANC_OK;

    ANC_LOGD("Starting full function test");

    ANC_LOGE("bind_big_core_bytid");
    SetCurrentThreadAffinity(gettid());

    // 1. wakeup sensor
    ret_val = SensorSetPowerMode(ANC_SENSOR_WAKEUP_RESET);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to wakeup sensor");
        goto DO_FAIL;
    }

    // 2. init algo
    ANC_LOGD("init algo\n");
    if (ANC_OK != (ret_val = AlgorithmInit())) {
        ANC_LOGE("fail to init algo");
        goto DO_FAIL;
    }

    if (ANC_OK != (ret_val = ExtensionLoadCalibration())) {
        ANC_LOGE("failed load calibrain. should be no calibration");
        goto DO_FAIL;
    }

    // 3. template test
    const char *root_path = "/data/vendor_de/0/fpdata/";
    if (TemplateSetActiveGroup(66, root_path) != ANC_OK) {
        ANC_LOGE("fail to init set group");
        goto DO_FAIL;
    }

    if (TemplateLoadDatabase() != ANC_OK) {
        ANC_LOGE("fail to load base");
        goto DO_FAIL;
    }

    uint32_t finger_ids[5];
    uint32_t id_count = 0;
    if (GetAllFingerprintsId(finger_ids, 5, &id_count) != ANC_OK) {
        ANC_LOGE("fail to get all finger id");
        goto DO_FAIL;
    }

    ANC_LOGD("finger id count:%d", id_count);
    for (uint32_t i = 0; i < id_count; i++) {
        ANC_LOGD("finger id:%d", finger_ids[i]);
        DeleteFingerprint(finger_ids[i]);
    }


    // 3.1 enroll diku1
    ANC_LOGD("enroll diku1\n");
    if (ANC_OK != enroll_image(1)) {
        ANC_LOGE("fail to enroll diku1");
        goto DO_FAIL;
    }

    // 3.2 enroll diku2
    // ANC_LOGD("enroll diku2\n");
    // if (ANC_OK != enroll_image(2)) {
    //     ANC_LOGE("fail to enroll diku2");
    //     goto DO_FAIL;
    // }


    // 3.3 verify
    ANC_LOGD("verify\n");
    if (ANC_OK != verify_image(3)) {
        ANC_LOGE("fail to verify");
        goto DO_FAIL;
    }

    // 4. deinit algo
    ANC_LOGD("deinit algo\n");
    if (ANC_OK != (ret_val = AlgorithmDeinit())) {
        ANC_LOGE("fail to deinit algo");
        goto DO_FAIL;
    }

    // 5. sleep sensor
    ANC_LOGD("sleep sensor\n");
    ret_val = SensorSetPowerMode(ANC_SENSOR_SLEEP);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to sleep sensor");
        goto DO_FAIL;
    }

    // 6. token
    ANC_LOGD("check token\n");
    if (ANC_OK != (ret_val = GetHmacKey())) {
        ANC_LOGE("fail to check token");
        goto DO_FAIL;
    }

DO_FAIL :

    return ret_val;
}

int DoCaptureStressTest(int test_count) {
    int32_t ret_val = ANC_OK;

    ANC_LOGD("Starting capture stress test, test count = %d", test_count);

    for (int count = 0; count < test_count; count++) {
        ANC_LOGD("Count = %d", count);
        printf("Count = %d\n", count);

        ret_val = SensorSetPowerMode(ANC_SENSOR_WAKEUP_RESET);
        ANC_LOGD("SensorWakeup ret_val = %d", ret_val);
        printf("SensorWakeup ret_val = %d\n", ret_val);
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to wakeup sensor, ret_val = %d", ret_val);
            break;
        }

        ret_val = SensorCaptureImage();
        ANC_LOGD("SensorCaptureImage ret_val = %d", ret_val);
        printf("SensorCaptureImage ret_val = %d\n", ret_val);
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to capture image, ret_val = %d", ret_val);
            break;
        }

        ret_val = SensorSetPowerMode(ANC_SENSOR_SLEEP);
        ANC_LOGD("SensorSleep ret_val = %d", ret_val);
        printf("SensorSleep ret_val = %d\n", ret_val);
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to sleep sensor, ret_val = %d", ret_val);
            break;
        }
        usleep(200 * 1000);
    }

    return ret_val;
}

int DoSPIStressTest(int test_count) {
    int32_t ret_val = ANC_OK;

    ANC_LOGD("Starting SPI stress test, test count = %d", test_count);

    for (int count = 0; count < test_count; count++) {
        ANC_LOGD("Count = %d", count);
        printf("Count = %d\n", count);

        ret_val = SensorSetPowerMode(ANC_SENSOR_WAKEUP_RESET);
        ANC_LOGD("SensorWakeup ret_val = %d", ret_val);
        printf("SensorWakeup ret_val = %d\n", ret_val);
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to wakeup sensor, ret_val = %d", ret_val);
            break;
        }

        ret_val = SensorSelfTest();
        ANC_LOGD("SensorSelfTest ret_val = %d", ret_val);
        printf("SensorSelfTest ret_val = %d\n", ret_val);
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to spi test, ret_val = %d", ret_val);
            break;
        }

        ret_val = SensorSetPowerMode(ANC_SENSOR_SLEEP);
        ANC_LOGD("SensorSleep ret_val = %d", ret_val);
        printf("SensorSleep ret_val = %d\n", ret_val);
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to sleep sensor, ret_val = %d", ret_val);
            break;
        }
        usleep(200 * 1000);
    }

    return ret_val;
}

int DoTest(ANC_CA_TEST_CASE test_case, int test_count) {
    int32_t ret_val = ANC_OK;

    ANC_LOGD("Starting test case: %d\ntest total count = %d", test_case, test_count);
    printf("Starting test case: %d\ntest total count = %d\n", test_case, test_count);

    if (ANC_OK != (ret_val = CaInit())) {
        ANC_LOGE("fail to ca init, ret_val = %d", ret_val);
        return ret_val;
    }

    switch (test_case) {
        case ANC_CA_FULL_FUNCTION_TEST:
            ret_val = DoFullFunctionTest();
            break;
        case ANC_CA_CAPTURE_STRESS_TEST:
            ret_val = DoCaptureStressTest(test_count);
            break;
        case ANC_CA_SPI_STRESS_TEST:
            ret_val = DoSPIStressTest(test_count);
            break;
        default:
            ANC_LOGE("don't support the test_case %d", test_case);
            ret_val = ANC_FAIL;
            break;
    }

    if (ANC_OK != CaDeinit()) {
        ANC_LOGE("fail to ca deinit");
    }

    if (ANC_OK == ret_val) {
        ANC_LOGD("Test Result: PASS");
        printf("Test Result: PASS\n");
    } else {
        ANC_LOGD("Test Result: FAIL (ret_val = %d)", ret_val);
        printf("Test Result: FAIL (ret_val = %d)\n", ret_val);
    }

    return ret_val;
}