#define LOG_TAG "[ANC_HAL][OplusExtension]"

#include "anc_extension_command.h"

#include <stdlib.h>

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
#include "oplus_extension_command.h"
#include "anc_extension_factory_mode.h"

#ifdef VIRTUAL_SENSOR
#include "anc_tac_virtual_sensor.h"
#endif

static uint8_t g_output[] = "test command!"; // draft, will fixed later

static ANC_RETURN_TYPE ExtCommandTest(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    p_command->command_respond.argument = 3;
    p_command->command_respond.p_buffer = g_output;
    p_command->command_respond.buffer_length = (sizeof(g_output)/sizeof(g_output[0]));

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

void DoExtCommandEnrollImage(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncSensorCommandParam sensor_param;
    int index = 0;
    int type = 0;
    int read_image=0;
    uint32_t remaining = 0;
    uint32_t finger_id = 0xFFFFFFFF;
    ANC_BOOL is_finished = ANC_FALSE;

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;
    ANC_LOGD("enroll image test, buffer length:%d", p_command->command.buffer_length);
    if (p_command->command.buffer_length > 0) {
        type = (int)*p_command->command.p_buffer;
        ANC_LOGD("enroll image test, type:%d", type);
    } else {
        ANC_LOGE("param is error!");
        goto DO_FAIL;
    }

    if (p_command->command.buffer_length > 1) {
        read_image = (int)p_command->command.p_buffer[1];
        ANC_LOGD("enroll image test, read image:%d", read_image);
    }

    if (ANC_OK != (ret_val = AlgoInitEnroll())) {
        ANC_LOGE("fail to init enroll");
        goto DO_FAIL;
    }

    if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
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
        if (ANC_OK != (ret_val = SensorCaptureImage())) {
            ANC_LOGE("fail to capture image");
            break;
        }

        if (read_image == 1) {
            uint8_t* p_tac_image_buffer = NULL;
            uint32_t tac_image_length = 0;
#ifdef ANC_GET_IMAGE_FROM_TA
            if (ANC_OK != (ret_val = ExtensionGetImage(&p_tac_image_buffer, &tac_image_length, 0))) {
                ANC_LOGE("fail to capture image");
                break;
            }
#endif
            p_command->command_respond.p_buffer=p_tac_image_buffer;
            p_command->command_respond.buffer_length=tac_image_length;
            p_command->command_respond.argument=type;
            p_manager->p_producer->OnExcuteCommand(p_device,
                            p_command->command_id, p_command->command_respond.argument,
                            p_command->command_respond.p_buffer,
                            p_command->command_respond.buffer_length);
        }


        if (ANC_OK != (ret_val = AlgoEnroll(&remaining, &finger_id))) {
            if (ANC_ALGO_ENROLL_FINISH == ret_val) {
                is_finished = ANC_TRUE;
                ANC_LOGE("enroll finished");
            } else {
                ANC_LOGE("fail to enroll");
            }
            break;
        }

        index++;
    } while (ANC_OK == ret_val);
    ANC_LOGD("enroll count : %d", index);

    if (ANC_OK != (ret_val = AlgoDeinitEnroll(&finger_id,is_finished))) {
        ANC_LOGE("fail to deinit enroll");
        goto DO_FAIL;
    }


DO_FAIL :
    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
    return ;
}

void DoExtCommandVerifyImage(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncSensorCommandParam sensor_param;
    int index = 0;
    int type = 0;
    int read_image=0;

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    if (p_command->command.buffer_length > 0) {
        type = (int)*p_command->command.p_buffer;
        ANC_LOGD("verify image test, type:%d", type);
    } else {
        ANC_LOGE("param is error!");
        goto DO_FAIL;
    }

    if (p_command->command.buffer_length > 1) {
        read_image = (int)p_command->command.p_buffer[1];
        ANC_LOGD("enroll image test, read image:%d", read_image);
    }

    if (ANC_OK != (ret_val = AlgoInitVerify())) {
        ANC_LOGE("fail to init verify");
        goto DO_FAIL;
    }

    if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
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
        if (ANC_OK != (ret_val = SensorCaptureImage())) {
            ANC_LOGE("fail to capture image");
            break;
        }

        if (read_image == 1) {
            uint8_t* p_tac_image_buffer = NULL;
            uint32_t tac_image_length = 0;
#ifdef ANC_GET_IMAGE_FROM_TA
            if (ANC_OK != (ret_val = ExtensionGetImage(&p_tac_image_buffer, &tac_image_length, 0))) {
                ANC_LOGE("fail to capture image");
                break;
            }
#endif
            p_command->command_respond.p_buffer=p_tac_image_buffer;
            p_command->command_respond.buffer_length=tac_image_length;
            p_command->command_respond.argument=type;
            p_manager->p_producer->OnExcuteCommand(p_device,
                            p_command->command_id, p_command->command_respond.argument,
                            p_command->command_respond.p_buffer,
                            p_command->command_respond.buffer_length);
        }
        uint32_t finger_id;
        uint32_t retry_count = 0;
        uint32_t need_study = 0;
        uint32_t algo_status = 0;
        if (ANC_OK != (ret_val = AlgoVerify(&finger_id, &need_study, &algo_status, retry_count))) {
            ANC_LOGE("fail to verify");
            break;
        }
        if (need_study) {
            AlgoFeatureStudy(finger_id);
        }


        index++;
    } while (ANC_OK == ret_val);
    ANC_LOGD("verify count : %d", index);

    if (ANC_OK != (ret_val = AlgoDeinitVerify())) {
        ANC_LOGE("fail to deinit verify");
        goto DO_FAIL;
    }

DO_FAIL :
    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
    return ;
}

#ifdef ANC_GET_IMAGE_FROM_HAL
static ANC_RETURN_TYPE ExtCommandReadBmp(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    AncReadExtBmpBuffer(p_command);

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}
#endif

#ifdef ANC_GET_IMAGE_FROM_TA
static ANC_BOOL g_is_last_sleep = ANC_TRUE;
static ANC_RETURN_TYPE ExtensionGetImageFromTA(AncFingerprintManager *p_manager, uint8_t *p_input_buffer, uint32_t input_buffer_length, uint8_t **pp_buffer, uint32_t *p_buffer_length, int need_save) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint8_t retry_cnt = 0;
    ANC_SENSOR_INNER_DATA_MODE capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT;


    if (NULL == p_input_buffer) {
        return ANC_FAIL;
    }

    if (input_buffer_length > 0) {
        retry_cnt = p_input_buffer[0];
        ANC_LOGD("Capture para retry count is :%d", retry_cnt);
        switch (retry_cnt) {
            case 0:
                capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT;
                break;
            case 1:
                capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT_EXPOSURE;
                break;
            case 2:
                capture_mode = ANC_SENSOR_TRANSMIT_EXPOSURE;
                break;
            case 3:
                capture_mode = ANC_SENSOR_TRANSMIT;
                break;
            default:
                capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT;
                break;
        }
        ANC_LOGD("Capture para mode is :%#x", capture_mode);
    }

    p_manager->p_consumer->GotoIdle(p_manager);

    if (capture_mode & ANC_SENSOR_FIRST_EXPOSURE) {
        /* try to sleep then wakeup if senosr is not sleep */
        if (!g_is_last_sleep) {
            if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP))) {
                goto DO_FAIL;
            }
        }
        if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
            goto DO_FAIL;
        }
    }

    if (input_buffer_length > 3) {
        AncSensorCommandParam sensor_param;
        sensor_param.type = ANC_SENSOR_CMD_SET_FRAME_FUSION_NUM;
        sensor_param.data.fusion_param.retry0 = p_input_buffer[1];
        sensor_param.data.fusion_param.retry1 = p_input_buffer[2];
        sensor_param.data.fusion_param.retry2 = p_input_buffer[3];
        ANC_LOGD("Capture para fusion num is : retry0 = %d, retry1 = %d, retry2 = %d", sensor_param.data.fusion_param.retry0,
                sensor_param.data.fusion_param.retry1, sensor_param.data.fusion_param.retry2);

        if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
            ANC_LOGE("fail to set frame fusion number");
            goto DO_FAIL;
        }
    }

    if (ANC_OK == (ret_val = SensorCaptureImageWithMode(capture_mode, NULL))) {
        if (ANC_OK != (ret_val = ExtensionGetImage(pp_buffer, p_buffer_length, need_save))) {
            ANC_LOGE("fail to get image from ta");
        }
    } else {
        ANC_LOGE("fail to capture image");
    }

DO_FAIL :
    /* sensor should not go to sleep if retry second exposure is ongoing */
    if (!(capture_mode & ANC_SENSOR_SECOND_EXPOSURE)) {
        p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
        g_is_last_sleep = ANC_TRUE;
    } else {
        g_is_last_sleep = ANC_FALSE;
    }
    return ret_val;
}
#endif

#ifdef ANC_GET_SENSOR_IMAGE
static ANC_RETURN_TYPE ExtCommandGetSensorImage(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;
    uint8_t* p_tac_image_buffer = NULL;
    uint32_t tac_image_length = 0;
    uint8_t response[1];
#ifdef ANC_GET_IMAGE_FROM_TA
    if (ANC_OK != (ret_val = ExtensionGetImageFromTA(p_manager, p_input_buffer, input_buffer_length, &p_tac_image_buffer, &tac_image_length, ANC_TRUE))) {
        ANC_LOGE("fail to capture image");
        response[0] = (uint8_t)ret_val;
        p_tac_image_buffer = response;
        tac_image_length = 1;
    }
#ifdef ANC_PADDING_INFO_IN_IMAGE_BUFFER
    else {
        AncExtensionImage *p_extension_image = (AncExtensionImage *)(p_tac_image_buffer - offsetof(AncExtensionImage, p_image_data));
        ANC_LOGD("exposure reg value = %d", p_extension_image->exposure_reg_val);
        *(p_tac_image_buffer + tac_image_length) = (uint8_t)((p_extension_image->exposure_reg_val >> 8) & 0xFF);
        *(p_tac_image_buffer + tac_image_length + 1) = (uint8_t)(p_extension_image->exposure_reg_val & 0xFF);
        switch (SensorGetAbnormalExpoType()) {
            case ANC_OK:
                *(p_tac_image_buffer + tac_image_length + 2) = 0;
                break;
            case ANC_CAPTURE_LOW_AUTO_EXP:
                *(p_tac_image_buffer + tac_image_length + 2) = 1;
                break;
            case ANC_CAPTURE_LOW_ENV_LIGHT:
                *(p_tac_image_buffer + tac_image_length + 2) = 2;
                break;
            case ANC_CAPTURE_HIGH_ENV_LIGHT:
                *(p_tac_image_buffer + tac_image_length + 2) = 3;
                break;
            default:
                *(p_tac_image_buffer + tac_image_length + 2) = 0;
                break;
        }
        ANC_LOGD("abnormal expo type = %d", *(p_tac_image_buffer + tac_image_length + 2));

        uint32_t env_light_value = p_extension_image->env_light_value;
        ANC_LOGD("env light value = %d", env_light_value);
        *(p_tac_image_buffer + tac_image_length + 3) = (env_light_value >> 24) & 0xFF;
        *(p_tac_image_buffer + tac_image_length + 4) = (env_light_value >> 16) & 0xFF;
        *(p_tac_image_buffer + tac_image_length + 5) = (env_light_value >> 8) & 0xFF;
        *(p_tac_image_buffer + tac_image_length + 6) = env_light_value & 0xFF;

        uint32_t image_ready_time = p_extension_image->image_ready_time;
        ANC_LOGD("image ready time = %d", p_extension_image->image_ready_time);
        *(p_tac_image_buffer + tac_image_length + 7) = (image_ready_time >> 8) & 0xFF;
        *(p_tac_image_buffer + tac_image_length + 8) = image_ready_time & 0xFF;

        ANC_LOGD("exposure time = %d", p_extension_image->exposure_time);
        *(p_tac_image_buffer + tac_image_length + 9) = (uint8_t)((p_extension_image->exposure_time >> 24) & 0xFF);
        *(p_tac_image_buffer + tac_image_length + 10) = (uint8_t)((p_extension_image->exposure_time >> 16) & 0xFF);
        *(p_tac_image_buffer + tac_image_length + 11) = (uint8_t)((p_extension_image->exposure_time >> 8) & 0xFF);
        *(p_tac_image_buffer + tac_image_length + 12) = (uint8_t)(p_extension_image->exposure_time & 0xFF);

        tac_image_length += 13;
    }
#endif
#endif
    p_command->command_respond.p_buffer = p_tac_image_buffer;
    p_command->command_respond.buffer_length = tac_image_length;
    p_command->command_respond.argument = 0;
    p_manager->p_producer->OnExcuteCommand(p_device,
                            p_command->command_id, p_command->command_respond.argument,
                            p_command->command_respond.p_buffer,
                            p_command->command_respond.buffer_length);

    return ret_val;
}
#endif

static ANC_RETURN_TYPE ExtCommandGetAuthToken(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncHwAuthToken temp_auth_token;
    uint32_t token_size = sizeof(temp_auth_token);
    uint64_t challenge;

    p_command->command_respond.p_buffer = NULL;
    p_command->command_respond.buffer_length = 0;

    ANC_LOGD("input buffer : %s", p_manager->p_producer->command.command.p_buffer);
    challenge = strtoull((const char*)p_manager->p_producer->command.command.p_buffer, 0, 10);
    ANC_LOGD("challenge : %llu", challenge);
    AncMemset(&temp_auth_token, 0, token_size);
    temp_auth_token.challenge = challenge;
    if (ANC_OK == (ret_val = ExtensionGetAuthToken((uint8_t *)&temp_auth_token, token_size))) {
        p_command->command_respond.p_buffer = (uint8_t *)&temp_auth_token;
        p_command->command_respond.buffer_length = token_size;
    }

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandSetHBM(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;
    if ((NULL != p_input_buffer) && (input_buffer_length == 1)) {
        ANC_LOGD("hbm status:%d", p_input_buffer[0]);
        p_manager->p_hbm_event_manager->SetHbm(p_manager, p_input_buffer[0]);
    }

    return ret_val;
}

#ifdef VIRTUAL_SENSOR
static ANC_RETURN_TYPE ExtCommandSetInjectImagesPath(AncFingerprintManager *p_manager) {
    return VcSetCurrentImagePath(p_manager->p_producer->command.command.p_buffer, p_manager->p_producer->command.command.buffer_length);
}
#endif

static ANC_RETURN_TYPE ConvertCharBufferToSensorParam(uint8_t *p_buffer,
                                        uint32_t buffer_length,
                                        AncSensorCommandParam *p_param) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if ((NULL == p_buffer) || (buffer_length < 1) || (NULL == p_param)) {
        ANC_LOGE("sensor param, input buffer:%p, input buffer length:%d, output:%p",
                     p_buffer, buffer_length, p_param);
        return ANC_FAIL;
    }

    AncMemset(p_param, 0, sizeof(AncSensorCommandParam));
    p_param->type = (ANC_SENSOR_CMD_TYPE)(p_buffer[0]);
    ANC_LOGD("sensor param, type:%d", p_param->type);

    switch (p_param->type) {
        case ANC_SENSOR_CMD_SET_EXPOSURE_TIME :
            if (buffer_length < 3) {
                p_param->data.exposure_time = ((int8_t)p_buffer[1]) * 1000;  //ms to us
                ANC_LOGD("set exposure time: %d us,", p_param->data.exposure_time);
            } else {
                p_param->type = ANC_SENSOR_CMD_SET_RETRY_EXPOSURE_TIME;
                p_param->data.retry_exposure_time.type = p_buffer[1];
                p_param->data.retry_exposure_time.retry0 = ((int8_t)p_buffer[2]) * 1000;  //ms to us
                p_param->data.retry_exposure_time.retry1 = ((int8_t)p_buffer[3]) * 1000;  //ms to us
                p_param->data.retry_exposure_time.retry2 = ((int8_t)p_buffer[4]) * 1000;  //ms to us
                ANC_LOGD("set retry exposure time, type:%d, retry0:%d us, retry1:%d us, retry2:%d us,", p_param->data.retry_exposure_time.type,
                    p_param->data.retry_exposure_time.retry0, p_param->data.retry_exposure_time.retry1, p_param->data.retry_exposure_time.retry2);
            }
            break;
        case ANC_SENSOR_CMD_SET_RETRY_EXPOSURE_TIME :
            p_param->data.retry_exposure_time.type = p_buffer[1];
            p_param->data.retry_exposure_time.retry0 = ((int8_t)p_buffer[2]) * 1000;  //ms to us
            p_param->data.retry_exposure_time.retry1 = ((int8_t)p_buffer[3]) * 1000;  //ms to us
            p_param->data.retry_exposure_time.retry2 = ((int8_t)p_buffer[4]) * 1000;  //ms to us
            ANC_LOGD("set retry exposure time, type:%d, retry0:%d us, retry0:%d us, retry0:%d us,", p_param->data.retry_exposure_time.type,
                    p_param->data.retry_exposure_time.retry0, p_param->data.retry_exposure_time.retry1, p_param->data.retry_exposure_time.retry2);
            break;
        case ANC_SENSOR_CMD_SET_IMAGE_SIZE :
            p_param->data.image_param.width = (uint32_t)((p_buffer[1] << 8) + p_buffer[2]);
            p_param->data.image_param.height = (uint32_t)((p_buffer[3] << 8) + p_buffer[4]);
            p_param->data.image_param.image_size = p_param->data.image_param.width * p_param->data.image_param.height * 2;
            ANC_LOGD("set image size: width = %d, height = %d", p_param->data.image_param.width, p_param->data.image_param.height);
            break;
        case ANC_SENSOR_CMD_VC_SETTING :
            p_param->data.vc_image_type = p_buffer[1];
            break;
        case ANC_SENSOR_CMD_WRITE_REGISTER :
            if (buffer_length == 3) {
                p_param->data.reg_param.addr = (uint16_t)p_buffer[1];
                p_param->data.reg_param.data = p_buffer[2];
            } else if (buffer_length == 4) {
                p_param->data.reg_param.addr = (uint16_t)(((uint16_t)p_buffer[1] << 8) | (uint16_t)p_buffer[2]);
                p_param->data.reg_param.data = p_buffer[3];
            } else {
                ANC_LOGE("write register, wrong buffer size = %d", buffer_length);
                break;
            }
            ANC_LOGD("write register: addr = %#x, data = %#x", p_param->data.reg_param.addr, p_param->data.reg_param.data);
            break;
        case ANC_SENSOR_CMD_SET_FRAME_FUSION_NUM :
            p_param->data.fusion_param.retry0 = p_buffer[1];
            p_param->data.fusion_param.retry1 = p_buffer[2];
            p_param->data.fusion_param.retry2 = p_buffer[3];
            ANC_LOGD("set frame fusion num: retry0 = %d, retry1 = %d, retry2 = %d",
                p_param->data.fusion_param.retry0, p_param->data.fusion_param.retry1, p_param->data.fusion_param.retry2);
            break;
        case ANC_SENSOR_CMD_SET_EXPO_CONFIG_THRESHOLD :
            p_param->data.expo_config_threshold.low_auto_expo_threshold = (int32_t)((p_buffer[1] << 24) | (p_buffer[2] << 16) | (p_buffer[3] << 8) | p_buffer[4]);
            p_param->data.expo_config_threshold.low_env_light_threshold = (uint32_t)((p_buffer[5] << 24) | (p_buffer[6] << 16) | (p_buffer[7] << 8) | p_buffer[8]);
            p_param->data.expo_config_threshold.high_env_light_threshold = (uint32_t)((p_buffer[9] << 24) | (p_buffer[10] << 16) | (p_buffer[11] << 8) | p_buffer[12]);
            ANC_LOGD("set expo config threshold: low auto expo time = %d us, low env light = %d, high env light = %d",
                p_param->data.expo_config_threshold.low_auto_expo_threshold, p_param->data.expo_config_threshold.low_env_light_threshold,
                p_param->data.expo_config_threshold.high_env_light_threshold);
            break;
        case ANC_SENSOR_CMD_READ_REGISTER :
            p_param->data.reg_param.addr = ((p_buffer[1] << 8) & 0xff00) | (p_buffer[2] & 0x00ff);
            break;
        case ANC_SENSOR_CMD_GET_EXPOSURE_TIME :
        case ANC_SENSOR_CMD_GET_EXPOSURE_REG_VAL :
        case ANC_SENSOR_CMD_EFUSE_READ_MT_CUSTOM_MODULE_INFO :
        case ANC_SENSOR_CMD_EFUSE_READ_CHIP_PROTO_INFO :
            break;
        default :
            ANC_LOGE("don't support this sensor param, type:%d", p_param->type);
            ret_val = ANC_FAIL;
            break;
    }

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandSetSensorParam(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncSensorCommandParam sensor_param;
    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;

    if ((NULL != p_input_buffer) && (input_buffer_length > 1)) {
        AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));

        ret_val = ConvertCharBufferToSensorParam(p_input_buffer, input_buffer_length, &sensor_param);
        if(ANC_OK == ret_val) {
#ifdef ANC_GET_IMAGE_FROM_TA
        /* ATTENTION: for JV0302 don't need to wakeup sensor here since no actual sensor spi transfer */
        // if (ANC_OK == (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
            ret_val = SensorSetParam(&sensor_param);
        // }
        // p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
#endif
            if(ANC_OK == ret_val) {
                ANC_LOGD("sensor set param ok");
            } else {
                ANC_LOGE("failed to sensor set param");
            }
        }

    } else {
        ANC_LOGE("sensor param is error!, input buffer:%p, input buffer length:%d",
                     p_input_buffer, input_buffer_length);
        ret_val = ANC_FAIL;
    }

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ConvertSensorParamToCharBuffer(AncSensorCommandParam *p_param,
                              uint8_t *p_buffer, uint32_t *p_buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if ((NULL == p_param) || (NULL == p_buffer) || (NULL == p_buffer_length)) {
        ANC_LOGE("sensor param, input:%p,  output buffer:%p, output buffer length:%p",
                     p_param, p_buffer, p_buffer_length);
        return ANC_FAIL;
    }

    switch (p_param->type) {
        case ANC_SENSOR_CMD_GET_EXPOSURE_TIME :
            p_buffer[0] = (uint8_t)ANC_SENSOR_CMD_GET_EXPOSURE_TIME;
            p_buffer[1] = (uint8_t)((p_param->data.exposure_time >> 8) & 0x00FF);
            p_buffer[2] = (uint8_t)(p_param->data.exposure_time & 0x00FF);
            *p_buffer_length = 3;
            break;
        case ANC_SENSOR_CMD_GET_EXPOSURE_REG_VAL :
            p_buffer[0] = (uint8_t)ANC_SENSOR_CMD_GET_EXPOSURE_REG_VAL;
            p_buffer[1] = (uint8_t)((p_param->data.exposure_time >> 8) & 0x00FF);
            p_buffer[2] = (uint8_t)(p_param->data.exposure_time & 0x00FF);
            *p_buffer_length = 3;
            break;
        case ANC_SENSOR_CMD_READ_REGISTER :
            p_buffer[0] = (uint8_t)ANC_SENSOR_CMD_READ_REGISTER;
            p_buffer[1] = (uint8_t)(p_param->data.reg_param.addr);
            p_buffer[2] = (uint8_t)(p_param->data.reg_param.data);
            *p_buffer_length = 3;
            break;
        case ANC_SENSOR_CMD_EFUSE_READ_MT_CUSTOM_MODULE_INFO :
            p_buffer[0] = (uint8_t)ANC_SENSOR_CMD_EFUSE_READ_MT_CUSTOM_MODULE_INFO;
            AncMemcpy(&p_buffer[1], &(p_param->data.custom_module_info), sizeof(p_param->data.custom_module_info));
            *p_buffer_length = sizeof(p_param->data.custom_module_info) + 1;
            break;
        case ANC_SENSOR_CMD_EFUSE_READ_CHIP_PROTO_INFO :
            p_buffer[0] = (uint8_t)ANC_SENSOR_CMD_EFUSE_READ_CHIP_PROTO_INFO;
            AncMemcpy(&p_buffer[1], &(p_param->data.chip_proto_info), sizeof(p_param->data.chip_proto_info));
            *p_buffer_length = sizeof(p_param->data.chip_proto_info) + 1;
            break;

        default :
            ANC_LOGE("don't support this sensor param, type:%d", p_param->type);
            break;
    }

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandGetSensorParam(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncSensorCommandParam sensor_param;
    AncSensorCommandParam output_buffer;
    uint32_t output_buffer_length = 0;
    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;

    ret_val = ConvertCharBufferToSensorParam(p_input_buffer, input_buffer_length, &sensor_param);
    if (ANC_OK == ret_val) {
#ifdef ANC_GET_IMAGE_FROM_TA
        if (ANC_OK == (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
            ret_val = SensorGetParam(&sensor_param);
        }
        p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
#endif
        if (ANC_OK == ret_val) {
            AncMemset(&output_buffer, 0, sizeof(AncSensorCommandParam));
            ret_val = ConvertSensorParamToCharBuffer(&sensor_param,
                              (uint8_t *)&output_buffer, &output_buffer_length);
            if (ANC_OK == ret_val) {
                p_command->command_respond.p_buffer = (uint8_t *)&output_buffer;
                p_command->command_respond.buffer_length = output_buffer_length;
            } else {
                ANC_LOGE("fail to convert sensor param to char buffer");
            }
        } else {
            ANC_LOGE("fail to get sensor param");
            ret_val = ANC_FAIL;
        }
    }

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE ExtCommandSetSensorStatus(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    uint8_t *p_input_buffer = p_command->command.p_buffer;
    uint32_t input_buffer_length = p_command->command.buffer_length;

    if ((NULL == p_input_buffer) && (input_buffer_length < 1)) {
        ANC_LOGE("sensor status param is error!, input buffer:%p, input buffer length:%d",
                     p_input_buffer, input_buffer_length);
        return ANC_FAIL;
    } else {
        // add some codes
        if (NULL != p_input_buffer)
            ANC_LOGE("input buffer[0]:%d", p_input_buffer[0]);
    }

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, p_command->command_respond.argument,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE InnerExtensionCommand(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    int32_t command_id = p_command->command_id;

    switch (command_id) {
        case EXTENSION_COMMAND_TEST :
            ret_val = ExtCommandTest(p_manager);
            break;
        case EXTENSION_COMMAND_TEST_ENROLL :
            p_manager->p_producer->PushTaskToConsumer(p_manager, DoExtCommandEnrollImage, (void *)p_manager,
                                                    (uint8_t *)"DoExtCommandEnrollImage");
            break;
        case EXTENSION_COMMAND_TEST_VERIFY :
            p_manager->p_producer->PushTaskToConsumer(p_manager, DoExtCommandVerifyImage, (void *)p_manager,
                                                    (uint8_t *)"DoExtCommandVerifyImage");
            break;
#ifdef ANC_GET_IMAGE_FROM_HAL
        case EXTENSION_COMMAND_TEST_READ_BMP:
            ret_val = ExtCommandReadBmp(p_manager);
            break;
#endif
        case EXTENSION_COMMAND_GET_AUTH_TOKEN:
            ret_val = ExtCommandGetAuthToken(p_manager);
            break;
        case EXTENSION_COMMAND_APP_SET_HBM:
            ret_val = ExtCommandSetHBM(p_manager);
            break;
#ifdef ANC_GET_SENSOR_IMAGE
        case EXTENSION_COMMAND_GET_SENSOR_IMAGE:
            ret_val = ExtCommandGetSensorImage(p_manager);
            break;
#endif
        case EXTENSION_COMMAND_SET_SENSOR_PARAM:
            ret_val = ExtCommandSetSensorParam(p_manager);
            break;
        case EXTENSION_COMMAND_GET_SENSOR_PARAM:
            ret_val = ExtCommandGetSensorParam(p_manager);
            break;
        case EXTENSION_COMMAND_SET_SENSOR_STATUS:
            ret_val = ExtCommandSetSensorStatus(p_manager);
            break;
#ifdef VIRTUAL_SENSOR
        case EXTENSION_COMMAND_SET_INJECT_IMAGES_PATH:
            ret_val = ExtCommandSetInjectImagesPath(p_manager);
            break;
#endif
        default :
            ret_val = ANC_FAIL_INVALID_COMMAND;
            ANC_LOGW("extension, invalid inner command id:%d", command_id);
            break;
    }

    return ret_val;
}

ANC_RETURN_TYPE ExtCommandHeartRateFingerDown(AncFingerprintManager *p_manager) {
    p_manager->p_tp_event_manager->HeartRateFingerDown(p_manager);
    return p_manager->p_producer->StartHeartBeatRateDetect(p_manager);
}

ANC_RETURN_TYPE ExtCommandHeartRateFingerUp(AncFingerprintManager *p_manager) {
    return p_manager->p_tp_event_manager->HeartRateFingerUp(p_manager);
}

ANC_RETURN_TYPE ExtensionCommandWork(AncFingerprintManager *p_manager,
          uint8_t **p_output_buffer, uint32_t *p_output_buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    int32_t command_id = p_command->command_id;
    ANC_LOGD("ExtensionCommandWork oplus command_id = %d",command_id);

    if ((ANC_FAIL_INVALID_COMMAND == (ret_val = OplusExtensionCommand(p_manager))) &&
        (ANC_FAIL_INVALID_COMMAND == (ret_val = InnerExtensionCommand(p_manager))) &&
        (ANC_FAIL_INVALID_COMMAND == (ret_val = FactoryModeExtensionCommand(p_manager)))) {
        ANC_LOGE("extension, invalid command id:%d", command_id);
    }

    *p_output_buffer = p_manager->p_producer->command.command_respond.p_buffer;
    *p_output_buffer_length = p_manager->p_producer->command.command_respond.buffer_length;

    return ret_val;
}
