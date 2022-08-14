/************************************************************************************
 ** File: - goodix\fingerprint\fingerprint_hwbinder\OpticalFingerprint.cpp
 ** 
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix(android O)
 **
 ** Version: 1.0
 ** Date created: 20:44, 2020/06/09
 ** Author: 
 ** TAG: BSP.Biometrics.Fingerprint
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  
 ************************************************************************************/
#define LOG_TAG "[GF_HAL][StabilityTest]"

//#include "StabilityTest.h"
#include "gf_error.h"
#include <vector>
#include "HalLog.h"
#include "HalUtils.h"
#include "HalContext.h"
#include "HalDsp.h"
#include "SZProductTestDefine.h"
#include <dlfcn.h>



namespace goodix {

    void* pdill = NULL;
    StabilityTest::StabilityTest() {
        FUNC_ENTER();
        init();
    }

    StabilityTest::~StabilityTest() {
        FUNC_ENTER();
        deinit();
    }

    void StabilityTest::init() {
        pdill = dlopen("/vendor/lib64/libgf_test.so", RTLD_NOW);
        if (pdill == NULL) {
            LOG_E(LOG_TAG, "[%s] dlopen error! ", __func__);
        } else {
            LOG_I(LOG_TAG, "[%s] dlopen ok ! ", __func__);
        }
    }

    void StabilityTest::deinit() {
        dlclose(pdill);
        LOG_I(LOG_TAG, "[%s] dlclose! ", __func__);
    }


    gf_error_t StabilityTest::onStabilityTestcmd(int32_t cmd_id, int8_t* in_buf, uint32_t size) {
        FUNC_ENTER();
        int count;
        char *config_path = "/data/local/tmp/config_data";
        char *cali_path = "/data/local/tmp/cali_data";
        gf_error_t err = GF_SUCCESS;
        char patharg[30][128];
        int img_count = 0;
        char* pathin[30];

        if (NULL != in_buf) {
        STABILITYTEST_INBUF_T *StabilityTest_inbuf = (STABILITYTEST_INBUF_T*)in_buf;
        img_count = StabilityTest_inbuf->img_count;
        LOG_E(LOG_TAG, "[%s] img_count = %d ", __func__, img_count);
        for (count=0;count <= img_count;count++) {
            LOG_E(LOG_TAG, "[%s] done first %s \n", __func__, StabilityTest_inbuf->imagePath[count]);
            strcpy(patharg[count], StabilityTest_inbuf->imagePath[count]);
            LOG_E(LOG_TAG, "[%s] patharg   %s \n", __func__, patharg[count]);
            pathin[count] = patharg[count];
        }
        LOG_E(LOG_TAG, "[%s] char done ", __func__);
        }

        pTestDeinit = (gf_error_t(*)(void))dlsym(pdill, "testDeinit");
        pTestInit = (gf_error_t(*)(char *config_path, char *cali_path))dlsym(pdill, "testInit");
        pTestEnroll = (gf_error_t(*)(int img_count, char *images[], void *img_info, uint32_t img_info_size))dlsym(pdill, "testEnroll");
        pTestAuth = (gf_error_t(*)(int img_count, char *images[], StabilityTest::AUTH_CONFIG_T *config,
                                   uint32_t auth_config_size, void *img_info, uint32_t img_info_size))dlsym(pdill, "testAuth");
        pTestGetFingerList = (gf_error_t(*)(uint32_t *list, uint32_t *finger_count))dlsym(pdill, "testGetFingerList");
        pTestDeleteFinger = (gf_error_t(*)(uint32_t finer_id))dlsym(pdill, "testDeleteFinger");
        //pTestSetNotifyCallback = (gf_error_t (*)(notifyAcquiredTestInfo_t hook))dlsym(pdill, "testSetNotifyCallback");

        /*switch (cmd_id) {
                case STABILITYTEST_CMD_ENROLL:
                err = (*pTestEnroll)(img_count, pathin, NULL, 0);
                //err = StabilityTestEnroll(img_count, pathin, NULL, 0);
                break;
                case STABILITYTEST_CMD_AUTH:
                err = (*pTestAuth)(img_count, pathin, NULL, 0, NULL, 0);
                //err = StabilityTestAuth(img_count, pathin, NULL, NULL, 0);
                break;
                case STABILITYTEST_CMD_REMOVE:
                err = (*pTestDeleteFinger)(0);
                //err = StabilityTestDeleteFinger(12345);
                break;
                case STABILITYTEST_CMD_PREENROLL:
                err = (*pTestInit)(config_path, cali_path);
                //err = StabilityTestInit(config_path, cali_path);
                break;
                case STABILITYTEST_CMD_CANCEL:
                err = (*pTestDeinit)();
                //err = StabilityTestDeinit();
                break;
            }
            err = getStabilityTestInfo();
            FUNC_EXIT(err);
            return err;
        }*/


        switch (cmd_id) {
            case STABILITYTEST_CMD_ENROLL:
            //err = (*pTestEnroll)(img_count, pathin, NULL, 0);
            err = StabilityTestEnroll(img_count, pathin, NULL, 0);
            break;
            case STABILITYTEST_CMD_AUTH:
            //err = (*pTestAuth)(img_count, pathin, NULL, 0, NULL, 0);
            err = StabilityTestAuth(img_count, pathin, NULL, NULL, 0);
            break;
            case STABILITYTEST_CMD_REMOVE:
            //err = (*pTestDeleteFinger)(0);
            err = StabilityTestDeleteFinger(12345);
            break;
            case STABILITYTEST_CMD_PREENROLL:
            //err = (*pTestInit)(config_path, cali_path);
            err = StabilityTestInit(config_path, cali_path);
            break;
            case STABILITYTEST_CMD_CANCEL:
            //err = (*pTestDeinit)();
            err = StabilityTestDeinit();
            break;
        }
        err = getStabilityTestInfo();
        FUNC_EXIT(err);
        return err;
    }


    gf_error_t StabilityTest::getStabilityTestInfo() {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;
        STABILITYTEST_NOTIFYINFO_T info = {{0}};
        info.auth_result = 0;
        info.fail_reason = 121;
        info.quality_score = 53;
        info.match_score = 30;
        info.signal_value = 59;
        info.retry_times = 2;
        info.algo_version = 168;
        info.chip_ic = 112;
        info.dsp_availalbe = 1;
        notifyStabilityTestInfo(&info);
        FUNC_EXIT(err);
        return err;
    }




    gf_error_t StabilityTest::notifyStabilityTestInfo(STABILITYTEST_NOTIFYINFO_T* info) {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;
        if (mStabilityTestNotify!= nullptr) {
            fingerprint_msg_t message;
            memset(&message, 0, sizeof(fingerprint_msg_t));
            message.type = FINGERPRINT_ACQUIRED_STABILITYTEST_DATA;
            message.data.acquired_test_info.auth_result = 0;
            LOG_D(LOG_TAG, "[%s]  StabilityTest NotifyData = ", __func__);
            mStabilityTestNotify(&message);
        }
        return err;
    }

    gf_error_t StabilityTest::StabilityTestInit(char *config_path, char *cali_path) {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;
        LOG_D(LOG_TAG, "[%s]  StabilityTest NotifyData = %s ", __func__, config_path);
        LOG_D(LOG_TAG, "[%s]  StabilityTest NotifyData = %s ", __func__, cali_path);
        return err;
    }

    gf_error_t StabilityTest::StabilityTestDeinit(void) {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;
        return err;
    }

    gf_error_t StabilityTest::StabilityTestEnroll(int img_count, char *images[], void *img_info, uint32_t img_info_size) {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;
        int count;
        LOG_E(LOG_TAG, "Enroll img_count %d", img_count);
        for (count = 0;count < img_count;count++) {
        LOG_E(LOG_TAG, "Enroll path %d = %p",  count, &images[count]);
        LOG_E(LOG_TAG, "Enroll path %d = %s",  count, images[count]);
        }
        UNUSED_VAR(img_info);
        UNUSED_VAR(img_info_size);
        return err;
    }
    gf_error_t StabilityTest::StabilityTestAuth(int img_count, char *images[], AUTH_CONFIG_T *config, void *img_info, uint32_t img_info_size) {
        gf_error_t err = GF_SUCCESS;
        int count;
        LOG_E(LOG_TAG, "Auth img_count %d ", img_count);
        for (count = 0;count < img_count;count++) {
        LOG_E(LOG_TAG, "Auth images[%d] path = %s", count, images[count]);
        }
        UNUSED_VAR(config);
        UNUSED_VAR(img_info);
        UNUSED_VAR(img_info_size);
        return err;
    }
    gf_error_t StabilityTest::StabilityTestGetFingerList(uint32_t *list, uint32_t *finger_count) {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(list);
        UNUSED_VAR(finger_count);
        return err;
    }
    gf_error_t StabilityTest::StabilityTestDeleteFinger(uint32_t finger_id) {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;
        LOG_E(LOG_TAG, "remove finger %d", finger_id);
        return err;
    }
}
