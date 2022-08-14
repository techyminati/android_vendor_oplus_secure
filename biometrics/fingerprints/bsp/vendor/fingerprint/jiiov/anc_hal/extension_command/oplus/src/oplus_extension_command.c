#define LOG_TAG "[ANC_HAL][OplusExtension]"

#include "anc_extension_command.h"

#include <stdlib.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "anc_log.h"
#include "anc_ca.h"
#include "anc_algorithm.h"
#include "anc_tac_sensor.h"
#include "anc_extension.h"
#include "anc_token.h"
#include "anc_file.h"
#include "anc_lib.h"
#include "anc_ca_image.h"
#include "anc_hal_extension_command.h"
#include "anc_memory_wrapper.h"
#include "anc_extension_factory_mode.h"


#define IMAGE_PASS_SCORE (20)
#define OPLUS_ENGINEERING_INFO_LENGTH 3

static bool g_cancel_aging_test = ANC_FALSE;
extern bool g_cancel_heart_rate_detect;
static bool g_report_up_event = ANC_FALSE;

typedef enum {
    FINGERPRINT_GET_IMAGE_SNR = 0,
    FINGERPRINT_GET_IMAGE_QUALITYS = 1,
    FINGERPRINT_GET_BAD_PIXELS = 2,
    FINGERPRINT_SELF_TEST = 3,
}ENGINEERING_INFO_ACQUIRE_ACTION_T;

typedef enum {
    SUCCESSED = 0,
    IMAGE_QUALITY = 1,
    SNR_SUCCESSED = 2,
    IMAGE_SNR = 3,
    BAD_PIXEL_NUM = 4,
    LOCAL_BAD_PIXEL_NUM = 5,
    M_ALL_TILT_ANGLE = 6,
    M_BLOCK_TILT_ANGLE_MAX = 7,
    LOCAL_BIG_PIXEL_NUM = 8,
    QUALITY_PASS = 9
}ENGINEERING_PARAMETER_GROUP_T;

typedef struct __attribute__((__packed__)) {
    uint32_t lenth;
    uint32_t keys_length;
    uint32_t values_length;
    uint32_t key[OPLUS_ENGINEERING_INFO_LENGTH];
    uint32_t value[OPLUS_ENGINEERING_INFO_LENGTH];
}OplusEngineeringInfo;

static ANC_RETURN_TYPE ExtCommandAuthenticateAsType(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;
    uint64_t operationId = 0;
    uint32_t gid = 0;
    int32_t fingerprint_auth_type = 0;
    uint32_t input_param_length = sizeof(operationId) + sizeof(gid) + sizeof(fingerprint_auth_type);

    if (input_param_length != input_buffer_length) {
        ret_val = ANC_FAIL;
        goto DO_FAIL;
    }

    AncMemcpy(&operationId, p_input_buffer, sizeof(operationId));
    p_input_buffer += sizeof(operationId);
    AncMemcpy(&gid, p_input_buffer, sizeof(gid));
    p_input_buffer += sizeof(gid);
    AncMemcpy(&fingerprint_auth_type, p_input_buffer, sizeof(fingerprint_auth_type));
    ANC_LOGD("ExtCommandAuthenticateAsType :%d", fingerprint_auth_type);

DO_FAIL :
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandCleanUp(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    // todo

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandPauseEnroll(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    // AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    // AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    ret_val = p_manager->p_tp_event_manager->SetFpEnable(p_manager,false);
    if(ANC_OK != ret_val) {
        ANC_LOGE("enroll PauseEnroll setFpEnable failed");
    }
    // p_manager->p_producer->OnExcuteCommand(p_device,
    //                     p_command->command_id, (int32_t)ret_val,
    //                     p_command->command_respond.p_buffer,
    //                     p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandContinueEnroll(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    // AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    // AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    // // todo

    // p_manager->p_producer->OnExcuteCommand(p_device,
    //                     p_command->command_id, (int32_t)ret_val,
    //                     p_command->command_respond.p_buffer,
    //                     p_command->command_respond.buffer_length);

    ret_val = p_manager->p_tp_event_manager->SetFpEnable(p_manager,true);
    if(ANC_OK != ret_val) {
        ANC_LOGE("enroll ContinueEnroll setFpEnable failed");
    }
    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandSetTouchEventListener(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    // todo

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandDynamicallyConfigLog(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;
    uint32_t on = 0;

    if (sizeof(on) != input_buffer_length) {
        ret_val = ANC_FAIL;
        goto DO_FAIL;
    }

    AncMemcpy(&on, p_input_buffer, sizeof(on));
    if (on) {
        AppSetSaveFileConfig(1);
    } else {
        AppSetSaveFileConfig(0);
    }

DO_FAIL :
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandPauseIdentify(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    // todo

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandContinueIdentify(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    // todo

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandGetAlikeyStatus(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    // todo

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static int32_t g_enroll_total_times = 0;
static ANC_RETURN_TYPE ExtCommandGetEnrollmentTotalTimes(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    ret_val = AlgoGetEnrollTotalTimes(&g_enroll_total_times);
    if (ANC_OK != ret_val) {
        g_enroll_total_times = 0;
    }

    p_command->command_respond.p_buffer = (uint8_t *)&g_enroll_total_times;
    p_command->command_respond.buffer_length = sizeof(g_enroll_total_times);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandSelfTest(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
        ANC_LOGE("fail to wakeup sensor");
    } else {
        ret_val = SensorSelfTest();
    }

    if (ANC_OK != ret_val) {
        if (ANC_OK != p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_HW_RESET_INIT)) {
            ANC_LOGE("fail to hw reset init");
            goto DO_FAIL;
        }
        if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
            ANC_LOGE("fail to wakeup sensor");
        } else {
            ret_val = SensorSelfTest();
        }
    }

#ifdef ANC_REPORT_SELF_TEST
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    OplusEngineeringInfo oplus_engineering_info;
    AncMemset(&oplus_engineering_info, 0, sizeof(OplusEngineeringInfo));
    oplus_engineering_info.lenth = 1;
    oplus_engineering_info.keys_length = 1;
    oplus_engineering_info.values_length = 1;

    oplus_engineering_info.key[0] = SUCCESSED;
    oplus_engineering_info.value[0] = (ret_val == ANC_OK) ? 1 : 0;

    p_command->command_respond.buffer_length = sizeof(oplus_engineering_info);
    p_command->command_respond.p_buffer = (uint8_t*)&oplus_engineering_info;
    p_command->command_id = EXTENSION_COMMAND_CB_ENGINEERING_INFO_UPDATED;

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);
#endif

DO_FAIL :
    if (ANC_OK != p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP)) {
        ANC_LOGE("fail to sleep");
    }

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandSetScreenState(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;
    int32_t screen_state = 0;

    if (sizeof(screen_state) != input_buffer_length) {
        ret_val = ANC_FAIL;
        goto DO_FAIL;
    }

    AncMemcpy(&screen_state, p_input_buffer, sizeof(screen_state));

DO_FAIL :
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

extern bool g_cancel;
static void DoImageQuality(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_RETURN_TYPE set_hbm_ret = ANC_OK;

    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;

    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    OplusEngineeringInfo oplus_engineering_info;
    oplus_engineering_info.lenth = OPLUS_ENGINEERING_INFO_LENGTH;
    oplus_engineering_info.keys_length = OPLUS_ENGINEERING_INFO_LENGTH;
    oplus_engineering_info.values_length = OPLUS_ENGINEERING_INFO_LENGTH;

    uint32_t image_quality_score = 0;
    if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP_RESET))) {
        goto DO_FAIL;
    }

    //capture image and get image quality
    while(ANC_TRUE) {
        if(g_cancel == ANC_TRUE) {
            goto DO_FAIL;
        }
        ret_val = p_manager->p_tp_event_manager->WaitTouchDown(p_manager, WAIT_TOUCH_DOWN_TIME_OUT);
        if(ANC_OK != ret_val) {
            continue;
        }
        if (g_cancel == ANC_TRUE) {
            goto DO_FAIL;
        }

        long long last_finger_up_time = p_manager->p_tp_event_manager->finger_up_time;
        ExtCommandCbOnTouchDown(p_manager);

        ret_val = p_manager->p_hbm_event_manager->SetHbm(p_manager, true);
        usleep(ANC_CAPTURE_DELAY); // delay 17ms

        if ((p_manager->p_tp_event_manager->IsTpTouchUp()) || (last_finger_up_time != p_manager->p_tp_event_manager->finger_up_time)) {
            p_manager->p_hbm_event_manager->SetHbm(p_manager, false);
            continue;
        }

        ret_val = SensorCaptureImage();
        set_hbm_ret = p_manager->p_hbm_event_manager->SetHbm(p_manager, false);
        if ((ANC_CAPTURE_LOW_AUTO_EXP == ret_val) || (ANC_CAPTURE_LOW_ENV_LIGHT == ret_val)
            || (ANC_CAPTURE_HIGH_ENV_LIGHT == ret_val)) {
            image_quality_score = 1;
            ANC_LOGE("Factory Image Quality Test abnormal expo");
        } else if(ANC_OK != ret_val) {
            image_quality_score = 0;
            ANC_LOGE("fail to get image quality Score, ret value:%d", ret_val);
        } else {
            if ((p_manager->p_tp_event_manager->IsTpTouchUp()) || (last_finger_up_time != p_manager->p_tp_event_manager->finger_up_time)) {
                continue;
            }

            ret_val = AlgoGetImageQualityScore(&image_quality_score);
            if (ANC_OK != ret_val) {
                ANC_LOGE("get image quality score failed, ret value:%d", ret_val);
            }
        }

        ANC_LOGE("ExtCommandGetEngineeringInfo image_quality_score %d", image_quality_score);

        oplus_engineering_info.key[0] = SUCCESSED;
        oplus_engineering_info.value[0] = (ret_val == ANC_OK)?1:0;
        oplus_engineering_info.key[1] = IMAGE_QUALITY;
        oplus_engineering_info.value[1] = image_quality_score;

        oplus_engineering_info.key[2] = QUALITY_PASS;
        oplus_engineering_info.value[2] = (image_quality_score >= IMAGE_PASS_SCORE)?1:0;

        p_command->command_respond.buffer_length = sizeof(oplus_engineering_info);
        p_command->command_respond.p_buffer = (uint8_t*)&oplus_engineering_info;
        p_command->command_id = EXTENSION_COMMAND_CB_ENGINEERING_INFO_UPDATED;

        p_manager->p_producer->OnExcuteCommand(p_device,
                            p_command->command_id, (int32_t)ret_val,
                            p_command->command_respond.p_buffer,
                            p_command->command_respond.buffer_length);
    }
DO_FAIL:
    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
    ANC_LOGD("ExtCommandGetEngineeringInfo image_quality_score cancel");

}

static ANC_RETURN_TYPE ExtCommandGetEngineeringInfo(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;
    uint32_t type = 0;

    if (sizeof(type) != input_buffer_length) {
        ret_val = ANC_FAIL;
        //goto DO_FAIL;
    }

    AncMemcpy(&type, p_input_buffer, sizeof(type));

    ANC_LOGE("ExtCommandGetEngineeringInfo type %d", type);

    switch (type) {
        case FINGERPRINT_GET_IMAGE_SNR:
            ANC_LOGD("getImageSnr CALL_API");
            break;
        case FINGERPRINT_GET_IMAGE_QUALITYS:
            ANC_LOGD("get image_quality CALL_API");
            p_manager->p_producer->PushTaskToConsumer(p_manager, DoImageQuality, (void *)p_manager,
                                                    (uint8_t *)"DoImageQuality");
            break;
        case FINGERPRINT_GET_BAD_PIXELS:
            ANC_LOGD("get bad pixels CALL_API");
            break;
        case FINGERPRINT_SELF_TEST:
            ANC_LOGD("self test CALL_API");
            ret_val = ExtCommandSelfTest(p_manager);
            break;
        default:
            ANC_LOGE("invalid msg type %d", type);
            return ANC_FAIL;

    }

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandTouchDown(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    p_manager->p_tp_event_manager->TouchDown(p_manager);

    ANC_LOGD("Oplus get touch down event from Factory Test");
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

ANC_RETURN_TYPE OplusExtCommandCbOnTouchDown(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    pthread_mutex_lock(&p_manager->p_producer->reportup_mutex);
    g_report_up_event = ANC_FALSE;
    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    p_manager->p_producer->OnExcuteCommand(p_device,
                           EXTENSION_COMMAND_TOUCH_DOWN, 0, NULL, 0);
    pthread_mutex_unlock(&p_manager->p_producer->reportup_mutex);
    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandTouchUp(AncFingerprintManager *p_manager) {

    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    p_manager->p_tp_event_manager->TouchUp(p_manager);

    ANC_LOGD("Oplus get touch up event from Factory Test");

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

ANC_RETURN_TYPE OplusExtCommandCbOnTouchUp(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    pthread_mutex_lock(&p_manager->p_producer->reportup_mutex);
    if (ANC_FALSE == g_report_up_event) {
        p_command->command_respond.p_buffer = NULL;
        p_command->command_respond.buffer_length = 0;

        p_manager->p_producer->OnExcuteCommand(p_device,
                           EXTENSION_COMMAND_TOUCH_UP, 0, NULL, 0);
        g_report_up_event = ANC_TRUE;
    }
    pthread_mutex_unlock(&p_manager->p_producer->reportup_mutex);
    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandHBMReady(AncFingerprintManager *p_manager) {

    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    p_manager->p_hbm_event_manager->HBMReady(p_manager);

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandGetModuleId(AncFingerprintManager *p_manager,
                                                char *p_module_id, uint32_t module_id_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
        ANC_LOGE("fail to wakeup sensor");
        goto DO_FAIL;
    }

    if (ANC_OK != (ret_val = SensorGetModuleId(p_module_id, module_id_size))) {
        goto DO_FAIL;
    }

DO_FAIL :
    if (ANC_OK != p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP)) {
        ANC_LOGE("fail to sleep");
    }

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandStopAgingTest(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    g_cancel_aging_test = ANC_TRUE;
    ANC_LOGD("stop aging test");
    p_manager->p_consumer->GotoIdle(p_manager);
    g_cancel_aging_test = ANC_FALSE;

    return ret_val;
}

#define AGINGTEST_RESULT_FLAG 100
#define AGING_RESULT_PASS 0
#define AGING_RESULT_FAIL 1

typedef struct __attribute__ ((__packed__)) {
    /* data */
    uint32_t flag;
    uint32_t result;
    uint32_t result_len;
}OplusEngineeringAgingTestInfo;


void DoExtCommandAgingTest(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    OplusEngineeringAgingTestInfo aging_test_info = {
        .flag = AGINGTEST_RESULT_FLAG,
        .result_len = sizeof(aging_test_info.result) + sizeof(aging_test_info.flag),
    };
    aging_test_info.result = AGING_RESULT_FAIL;

    if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
        ANC_LOGE("fail to wakeup sensor");
    } else {
        if (ANC_OK != (ret_val = SensorSetExposureTime(10000))) {
            ANC_LOGE("fail to set exposure time : 10 ms");
        }
    }

    if (ANC_OK != ret_val) {
        if (ANC_OK != p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_HW_RESET_INIT)) {
            ANC_LOGE("fail to hw reset init");
            goto DO_FAIL;
        }
        if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
            ANC_LOGE("fail to wakeup sensor");
            goto DO_FAIL;
        }
        if (ANC_OK != (ret_val = SensorSetExposureTime(10000))) {
            ANC_LOGE("fail to set exposure time : 10 ms");
            goto DO_FAIL;
        }
    }

    while (ANC_TRUE) {
        ANC_LOGD("aging test cancel is %d", g_cancel_aging_test);
        if(ANC_TRUE == g_cancel_aging_test) {
            ANC_LOGE("cancel aging test");
            aging_test_info.result = AGING_RESULT_PASS;
            break;
        }

        aging_test_info.result = AGING_RESULT_FAIL;
        ret_val = p_manager->p_hbm_event_manager->SetHbm(p_manager, true);
        if(ANC_OK != ret_val) {
            ANC_LOGE("aging test set hbm on failed");
            break;
        }

        if (ANC_OK != (ret_val = SensorCaptureImage())) {
            ANC_LOGE("fail to capture image, ret value: %d", ret_val);
            if (ANC_OK != p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_HW_RESET_INIT)) {
                ANC_LOGE("fail to hw reset init");
                break;
            }
            if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
                ANC_LOGE("fail to wakeup sensor");
                break;
            }
            if (ANC_OK != (ret_val = SensorSetExposureTime(10000))) {
                ANC_LOGE("fail to set exposure time : 10 ms");
                break;
            }
            if (ANC_OK != (ret_val = SensorCaptureImage())) {
                ANC_LOGE("fail to capture image, ret value: %d", ret_val);
                break;
            }
        }

        ret_val = p_manager->p_hbm_event_manager->SetHbm(p_manager, false);
        if(ANC_OK != ret_val) {
            ANC_LOGE("aging test set hbm off failed");
            break;
        }
        aging_test_info.result = AGING_RESULT_PASS;
        p_command->command_respond.p_buffer = (uint8_t *)&aging_test_info;
        p_command->command_respond.buffer_length = sizeof(aging_test_info);
        p_manager->p_producer->OnExcuteCommand(p_device,
                    EXTENSION_COMMAND_EM_AGING_TEST_CALLBACK, (int32_t)ret_val,
                    p_command->command_respond.p_buffer,
                    p_command->command_respond.buffer_length);

        sleep(1); // waiting for 1 second
    }

    if (ANC_OK != (ret_val = SensorSetExposureTime(AUTO_EXPOSURE_TIME))) {
        ANC_LOGE("fail to set exposure time : AUTO_EXPO");
        goto DO_FAIL;
    }

DO_FAIL :
    if (AGING_RESULT_FAIL == aging_test_info.result) {
        p_command->command_respond.p_buffer = (uint8_t *)&aging_test_info;
        p_command->command_respond.buffer_length = sizeof(aging_test_info);
        p_manager->p_producer->OnExcuteCommand(p_device,
                    EXTENSION_COMMAND_EM_AGING_TEST_CALLBACK, (int32_t)ret_val,
                    p_command->command_respond.p_buffer,
                    p_command->command_respond.buffer_length);
    }

    if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP))) {
        ANC_LOGE("fail to sleep");
    }
    return ;
}

typedef struct __attribute__ ((__packed__)) {
    uint32_t token1;
    uint32_t errcode;
    uint32_t token2;
    uint32_t data_length;
    char module_id[32];
}OplusEngineeringModuleIdInfo;

char g_bright_value[50];
char *g_brightness_paths[] = {
    "/sys/class/leds/lcd-backlight/brightness",
    "/sys/class/backlight/panel0-backlight/brightness",
};
static ANC_RETURN_TYPE OplusSearchBrightnessPath(unsigned long *p_index) {
    unsigned long index = 0;

    for (index = 0; index < sizeof(g_brightness_paths)/sizeof(g_brightness_paths[0]); index ++) {
        if (access(g_brightness_paths[index], 0) == 0) {
            ANC_LOGD("oplus Brightness path index %d, path:%s", index, g_brightness_paths[index]);
            break;
        }
    }

    *p_index = index;

    if (index == sizeof(g_brightness_paths)/sizeof(g_brightness_paths[0])) {
        ANC_LOGD("oplus no brightness path available");
        return ANC_FAIL;
    }

    return ANC_OK;
}

static ANC_RETURN_TYPE OplusExtGetBrightnessValue() {
    ssize_t length = 0;
    unsigned long index = 0;
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("oplus getBrightnessValue start");

    AncMemset(g_bright_value, 0, sizeof(g_bright_value));

    ret_val = OplusSearchBrightnessPath(&index);
    if(ANC_FAIL == ret_val) {
        ANC_LOGD("oplus no brightness path available");
        return ANC_FAIL;
    }

    int fd = open(g_brightness_paths[index], O_RDONLY);
    if (fd < 0) {
        ANC_LOGD("oplus setBrightness err:%d, errno =%d", fd, errno);
        return ANC_FAIL;
    }
    length = read(fd, g_bright_value, sizeof(g_bright_value));
    if (length > 0) {
        ANC_LOGD("oplus get g_bright_value = %s  length = %d ", g_bright_value, length);
    }
    else {
        ANC_LOGD("oplus read brightness value fail");
        close(fd);
        return ANC_FAIL;
    }
    close(fd);
    return ANC_OK;
}

static ANC_RETURN_TYPE OplusExtSetBrightnessValue() {
    ssize_t length = 0;
    unsigned long  index = 0;
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("oplus setBrightnessValue start");

    ret_val = OplusSearchBrightnessPath(&index);
    if(ANC_FAIL == ret_val) {
        ANC_LOGD("oplus no brightness path available");
        return ANC_FAIL;
    }

    int fd = open(g_brightness_paths[index], O_WRONLY);
    if (fd < 0) {
        ANC_LOGD("oplus setBrightness err:%d, errno = %d", fd, errno);
        return  ANC_FAIL;
    }
    length = write(fd, g_bright_value, sizeof(g_bright_value));
    ANC_LOGD("oplus set g_bright_value = %s  length = %d ", g_bright_value, length);
    if (length < 0) {
        ANC_LOGE("oplus write brightness value fail");
        close(fd);
        return  ANC_FAIL;
    }
    close(fd);
    return ANC_OK;
}

static ANC_RETURN_TYPE ExtCommandSendFingerprintCmd(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    int32_t command_id = p_command->command_id;

    OplusEngineeringModuleIdInfo module_id_info;

    switch (command_id) {
        case EXTENSION_COMMAND_EM_AGING_TEST:
            p_manager->p_producer->PushTaskToConsumer(p_manager, DoExtCommandAgingTest, (void *)p_manager,
                                                    (uint8_t *)"DoExtCommandAgingTest");
            break;
        case EXTENSION_COMMAND_EM_STOP_AGING_TEST:
            ret_val = ExtCommandStopAgingTest(p_manager);
            break;
        case EXTENSION_COMMAND_GET_QR_CODE :
            AncMemset(&module_id_info, 0, sizeof(module_id_info));
            module_id_info.token1 = 1;
            module_id_info.errcode = 0;
            module_id_info.token2 = module_id_info.token1;
            ret_val = ExtCommandGetModuleId(p_manager, module_id_info.module_id, sizeof(module_id_info.module_id));
            module_id_info.data_length = (uint32_t)strlen(module_id_info.module_id);
            p_command->command_respond.p_buffer = (uint8_t *)&module_id_info;
            p_command->command_respond.buffer_length = sizeof(module_id_info);
            p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);
            break;
        case EXTENSION_COMMAND_START_CALI:
            ret_val = OplusExtGetBrightnessValue();
            break;
        case EXTENSION_COMMAND_END_CALI:
            ret_val = OplusExtSetBrightnessValue();
            break;
        default :
            ret_val = ANC_FAIL_INVALID_COMMAND;
            ANC_LOGE("extension, invalid oplus fingerprint command id:%d", command_id);
            break;
    }

    return ret_val;
}

ANC_RETURN_TYPE OplusExtensionCommand(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    int32_t command_id = p_command->command_id;

    ANC_LOGD("OplusExtensionCommand command id:%d", command_id);

    switch (command_id) {
        case EXTENSION_COMMAND_AUTHENTICATE_AS_TYPE:
            ret_val = ExtCommandAuthenticateAsType(p_manager);
            break;
        case EXTENSION_COMMAND_CLEAN_UP:
            ret_val = ExtCommandCleanUp(p_manager);
            break;
        case EXTENSION_COMMAND_PAUSE_ENROLL:
            ret_val = ExtCommandPauseEnroll(p_manager);
            break;
        case EXTENSION_COMMAND_CONTINUE_ENROLL:
            ret_val = ExtCommandContinueEnroll(p_manager);
            break;
        case EXTENSION_COMMAND_SET_TOUCH_EVENT_LISTENER:
            ret_val = ExtCommandSetTouchEventListener(p_manager);
            break;
        case EXTENSION_COMMAND_DYNAMICALLY_CONFIG_LOG:
            ret_val = ExtCommandDynamicallyConfigLog(p_manager);
            break;
        case EXTENSION_COMMAND_PAUSE_IDENTIFY:
            ret_val = ExtCommandPauseIdentify(p_manager);
            break;
        case EXTENSION_COMMAND_CONTINUE_IDENTIFY:
            ret_val = ExtCommandContinueIdentify(p_manager);
            break;
        case EXTENSION_COMMAND_GET_ALIKEY_STATUS:
            ret_val = ExtCommandGetAlikeyStatus(p_manager);
            break;
        case EXTENSION_COMMAND_GET_ENROLLMENT_TOTAL_TIMES:
            ret_val = ExtCommandGetEnrollmentTotalTimes(p_manager);
            break;
        case EXTENSION_COMMAND_SET_SCREEN_STATE:
            ret_val = ExtCommandSetScreenState(p_manager);
            break;
        case EXTENSION_COMMAND_GET_ENGINEERING_INFO:
            ret_val = ExtCommandGetEngineeringInfo(p_manager);
            break;
        case EXTENSION_COMMAND_TOUCH_DOWN:
            ret_val = ExtCommandTouchDown(p_manager);
            break;
        case EXTENSION_COMMAND_TOUCH_UP:
            ret_val = ExtCommandTouchUp(p_manager);
            break;
        case EXTENSION_COMMAND_HBM_READY:
            ret_val = ExtCommandHBMReady(p_manager);
            break;
        case EXTENSION_COMMAND_SEND_FINGERPRINT_CMD:
        case EXTENSION_COMMAND_EM_AGING_TEST:
        case EXTENSION_COMMAND_EM_STOP_AGING_TEST:
        case EXTENSION_COMMAND_GET_QR_CODE:
        case EXTENSION_COMMAND_START_CALI:
        case EXTENSION_COMMAND_END_CALI:
            ret_val = ExtCommandSendFingerprintCmd(p_manager);
            break;
        case EXTENSION_COMMAND_PRE_HEART_RATE_DETECT:
            g_cancel_heart_rate_detect = ANC_FALSE;
            break;
        case EXTENSION_COMMAND_POST_HEART_RATE_DETECT:
            g_cancel_heart_rate_detect = ANC_TRUE;
            break;
        case EXTENSION_COMMAND_HEART_RATE_FINGER_DOWN:
            if (ANC_FALSE == g_cancel_heart_rate_detect) {
                ret_val = ExtCommandHeartRateFingerDown(p_manager);
            }
            break;
        case EXTENSION_COMMAND_HEART_RATE_FINGER_UP:
            if (ANC_FALSE == g_cancel_heart_rate_detect) {
                ret_val = ExtCommandHeartRateFingerUp(p_manager);
            }
            break;
        default :
            ret_val = ANC_FAIL_INVALID_COMMAND;
            ANC_LOGW("extension, invalid oplus command id:%d", command_id);
            break;
    }

    return ret_val;
}

ANC_RETURN_TYPE OplusExtensionSetCaliProperty(ANC_BOOL isCali) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    if (property_set("vendor.fingerprint.cali", isCali ? "1" : "0") != 0) {
        ret_val = ANC_FAIL;
    }
    return ret_val;
}
