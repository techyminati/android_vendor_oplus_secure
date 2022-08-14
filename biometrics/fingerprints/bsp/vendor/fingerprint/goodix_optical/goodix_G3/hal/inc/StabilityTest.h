#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "gf_error.h"
#include "HalBase.h"
#include "EventCenter.h"
#include "gf_algo_types.h"
#include "gf_event.h"
#include "fingerprint_oplus.h"

#define MAX_FILE_PATH_LEN 256


namespace goodix
{
    class StabilityTest{
    public:
    typedef struct
    {
        int coordinate_x;
        int coordinate_y;
        int press_area;
        int temperature;
        uint8_t reserve[20];
    } IMG_INFO_T;

    typedef struct
    {
        uint8_t dsp_config; //控制
        uint8_t reserve[20]; //预留控制位
    } AUTH_CONFIG_T; //必须要包含本功能

    typedef struct
    {
        char init_config_path; //控制
        char init_cali_path; //预留控制位
    } INIT_CONFIG_T; //必须要包含本功能

    typedef struct
    {
        uint8_t  auth_result;
        uint8_t  fail_reason;
        uint8_t  quality_score;
        uint8_t  match_score;
        uint8_t  signal_value;
        uint8_t  retry_times;
        uint8_t  algo_version;
        uint8_t  chip_ic;
        uint8_t  dsp_availalbe;
    }STABILITYTEST_NOTIFYINFO_T;

    typedef struct
    {
        int img_count;
        char imagePath[30][128];
    } STABILITYTEST_INBUF_T;


    explicit StabilityTest();
    virtual  ~StabilityTest();
    void init();
    void deinit();

    fingerprint_notify_t mStabilityTestNotify;
    gf_error_t onStabilityTestcmd(int32_t cmd_id, int8_t* in_buf, uint32_t size);
    gf_error_t notifyStabilityTestInfo(STABILITYTEST_NOTIFYINFO_T* info);
    gf_error_t getStabilityTestInfo();

    gf_error_t (*pTestDeinit)(void);
    gf_error_t (*pTestInit)(char *config_path, char *cali_path);
    gf_error_t (*pTestEnroll)(int img_count, char *images[], void *img_info, uint32_t img_info_size) ;
    gf_error_t (*pTestAuth)(int img_count, char *images[],AUTH_CONFIG_T *config, uint32_t auth_config_size, void *img_info, uint32_t img_info_size);
    gf_error_t (*pTestGetFingerList)(uint32_t *list, uint32_t *finger_count);
    gf_error_t (*pTestDeleteFinger)(uint32_t finger_id) ;


    //gf_error_t (*pTestSetNotifyCallback)(notifyAcquiredTestInfo_t hook) = NULL;

    gf_error_t StabilityTestInit(char *config_path, char *cali_path);
    gf_error_t StabilityTestDeinit(void);
    gf_error_t StabilityTestEnroll(int img_count, char *images[], void *img_info, uint32_t img_info_size);
    gf_error_t StabilityTestAuth(int img_count, char *images[], AUTH_CONFIG_T *config, void *img_info, uint32_t img_info_size);
    gf_error_t StabilityTestGetFingerList(uint32_t *list, uint32_t *finger_count);
    gf_error_t StabilityTestDeleteFinger(uint32_t finger_id);
    };
} //namespcae
