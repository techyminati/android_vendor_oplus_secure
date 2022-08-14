#define LOG_TAG "[ANC_TAC][Sensor]"

#include "anc_tac_sensor.h"

#include <string.h>

#include "anc_error.h"
#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "sensor_command.h"
#include "anc_memory_wrapper.h"
#include "anc_tac_time.h"
#include "anc_utils.h"
#include "anc_log_string.h"

#ifdef VIRTUAL_SENSOR
#include "anc_tac_virtual_sensor.h"
#endif

static ANC_BOOL g_use_fixed_expo_for_retry = ANC_FALSE;
static ANC_RETURN_TYPE g_abnormal_expo_type = ANC_OK;

#ifdef ANC_QUICKLY_PICK_UP_FINGER
    extern long long g_capture_last_expo_start_time;
    extern long long g_capture_retry0_total_expo_time;
    static long long g_capture_second_expo_start_time;
#endif


static ANC_RETURN_TYPE SensorTransmitCommon(AncSensorCommand *p_sensor, AncSensorCommandRespond *p_sensor_respond,
                                        ANC_BOOL is_used_share_buffer) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSendCommand anc_send_command;
    AncSendCommandRespond anc_send_respond;

    AncMemset(&anc_send_respond, 0, sizeof(anc_send_respond));
    AncMemset(&anc_send_command, 0, sizeof(anc_send_command));
    anc_send_command.size = sizeof(AncSendCommand);
    anc_send_command.id = ANC_CMD_SENSOR;
    AncMemcpy(&(anc_send_command.data.sensor), p_sensor, sizeof(AncSensorCommand));

    char sensor_param_string[50] = "";
    if ((ANC_CMD_SENSOR_SET_POWER_MODE == p_sensor->command) || (ANC_CMD_SENSOR_SET_PARAM == p_sensor->command)
        || (ANC_CMD_SENSOR_GET_PARAM == p_sensor->command)) {
        int32_t param = p_sensor->data;
        if (ANC_CMD_SENSOR_SET_POWER_MODE != p_sensor->command) {
            param = (int32_t)p_sensor->param.type;
        }
        AncSnprintf(sensor_param_string, sizeof(sensor_param_string), ", param = %d (%s)", param,
            AncConvertSensorParamToString(p_sensor->command, param));
    }
    ANC_LOGD("TRANSMIT >>>> command id = %d (%s)%s", p_sensor->command,
        AncConvertCommandIdToString(ANC_CMD_SENSOR, p_sensor->command), sensor_param_string);
    long long time_start = AncGetElapsedRealTimeMs();

    if (is_used_share_buffer) {
        ret_val = AncCaTransmitModified(&anc_send_command, &anc_send_respond);
    } else {
        ret_val = AncCaTransmit(&anc_send_command, &anc_send_respond);
    }
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send sensor command, ret value:%d, command id:%d",
                  ret_val, anc_send_command.data.sensor.command);
    } else {
        ret_val = (ANC_RETURN_TYPE)anc_send_respond.status;
        AncMemcpy(p_sensor_respond, &(anc_send_respond.respond.sensor), sizeof(AncSensorCommandRespond));
    }

    long long time_end = AncGetElapsedRealTimeMs();
    ANC_LOGD("TRANSMIT <<<< command id = %d (%s)%s, spent time = %lld ms, ret_val = %d (%s)", p_sensor->command,
        AncConvertCommandIdToString(ANC_CMD_SENSOR, p_sensor->command), sensor_param_string,
        (time_end - time_start), ret_val, AncConvertReturnTypeToString(ret_val));

    return ret_val;
}

#ifdef VIRTUAL_SENSOR
static ANC_RETURN_TYPE SensorTransmitNoLockSharedBuffer(AncSensorCommand *p_sensor, AncSensorCommandRespond *p_sensor_respond) {
    return SensorTransmitCommon(p_sensor, p_sensor_respond, ANC_TRUE);
}
#endif

static ANC_RETURN_TYPE SensorTransmit(AncSensorCommand *p_sensor, AncSensorCommandRespond *p_sensor_respond) {
    return SensorTransmitCommon(p_sensor, p_sensor_respond, ANC_FALSE);
}


ANC_RETURN_TYPE InitSensor() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommand anc_sensor_command;
    AncSensorCommandRespond anc_sensor_respond;

    AncMemset(&anc_sensor_command, 0, sizeof(anc_sensor_command));
    AncMemset(&anc_sensor_respond, 0, sizeof(anc_sensor_respond));
    anc_sensor_command.command = ANC_CMD_SENSOR_INIT;
    ret_val = SensorTransmit(&anc_sensor_command, &anc_sensor_respond);

    return ret_val;
}

ANC_RETURN_TYPE DeinitSensor() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommand anc_sensor_command;
    AncSensorCommandRespond anc_sensor_respond;

    AncMemset(&anc_sensor_command, 0, sizeof(anc_sensor_command));
    AncMemset(&anc_sensor_respond, 0, sizeof(anc_sensor_respond));
    anc_sensor_command.command = ANC_CMD_SENSOR_DEINIT;
    ret_val = SensorTransmit(&anc_sensor_command, &anc_sensor_respond);

    return ret_val;
}

ANC_RETURN_TYPE SensorSetParam(AncSensorCommandParam *p_param) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommand anc_sensor_command;
    AncSensorCommandRespond anc_sensor_respond;

    if (NULL == p_param) {
        ANC_LOGE("fail to set sensor param, param is null!");
        return ANC_FAIL;
    }

    AncMemset(&anc_sensor_command, 0, sizeof(anc_sensor_command));
    AncMemset(&anc_sensor_respond, 0, sizeof(anc_sensor_respond));
    anc_sensor_command.command = ANC_CMD_SENSOR_SET_PARAM;
    AncMemcpy(&(anc_sensor_command.param), p_param, sizeof(AncSensorCommandParam));
    ret_val = SensorTransmit(&anc_sensor_command, &anc_sensor_respond);

    return ret_val;
}

ANC_RETURN_TYPE SensorGetParam(AncSensorCommandParam *p_param) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommand anc_sensor_command;
    AncSensorCommandRespond anc_sensor_respond;

    if (NULL == p_param) {
        ANC_LOGE("fail to get sensor param, param is null!");
        return ANC_FAIL;
    }

    AncMemset(&anc_sensor_command, 0, sizeof(anc_sensor_command));
    AncMemset(&anc_sensor_respond, 0, sizeof(anc_sensor_respond));
    anc_sensor_command.command = ANC_CMD_SENSOR_GET_PARAM;
    AncMemcpy(&(anc_sensor_command.param), p_param, sizeof(AncSensorCommandParam));
    ret_val = SensorTransmit(&anc_sensor_command, &anc_sensor_respond);
    if (ANC_OK == ret_val) {
        if (p_param->type == anc_sensor_respond.param.type) {
            AncMemcpy(p_param, &(anc_sensor_respond.param), sizeof(AncSensorCommandParam));
        }
    } else {
        ANC_LOGE("fail to get sensor param, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorGetChipId(uint32_t *sensor_chip_id) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommand anc_sensor_command;
    AncSensorCommandRespond anc_sensor_respond;

    if (NULL == sensor_chip_id) {
        ANC_LOGE("fail to get chip id, param is null!");
        return ANC_FAIL;
    }

    AncMemset(&anc_sensor_command, 0, sizeof(anc_sensor_command));
    AncMemset(&anc_sensor_respond, 0, sizeof(anc_sensor_respond));
    anc_sensor_command.command = ANC_CMD_SENSOR_GET_CHIP_ID;
    ret_val = SensorTransmit(&anc_sensor_command, &anc_sensor_respond);
    if (ANC_OK == ret_val) {
        *sensor_chip_id = anc_sensor_respond.data;
    } else {
        ANC_LOGE("fail to get chip id, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorSetPowerMode(ANC_SENSOR_POWER_MODE power_mode) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommand anc_sensor_command;
    AncSensorCommandRespond anc_sensor_respond;

    AncMemset(&anc_sensor_command, 0, sizeof(anc_sensor_command));
    AncMemset(&anc_sensor_respond, 0, sizeof(anc_sensor_respond));
    anc_sensor_command.command = ANC_CMD_SENSOR_SET_POWER_MODE;
    anc_sensor_command.data = (int32_t)power_mode;
    ret_val = SensorTransmit(&anc_sensor_command, &anc_sensor_respond);

    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to set sensor power mode %d, ret value:%d", power_mode, ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorSelfTest(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;
    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));

    sensor_param.type = ANC_SENSOR_CMD_SELF_TEST;
    if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
        ANC_LOGE("fail to do sensor self test, ret value: %d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorAgingTest(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (ANC_OK != (ret_val = SensorSetExposureTime(10000))) {
        return ret_val;
    }

    if (ANC_OK != (ret_val = SensorCaptureImage())) {
        ANC_LOGE("fail to capture image, ret value: %d", ret_val);
    }

    ret_val = SensorSetExposureTime(AUTO_EXPOSURE_TIME);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to set exposure time, ret value: %d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorGetModuleId(char *p_module_id, uint32_t buffer_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;
    size_t module_id_size = sizeof(ANC_SENSOR_EFUSE_CUSTOM_MODULE_INFO);

    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));
    if (buffer_size < (module_id_size + 1)) {
        ANC_LOGE("buffer is too small, buffer size: %d, module id size: %d", buffer_size, module_id_size);
        return ANC_FAIL_BUFFER_TOO_SMALL;
    }

    sensor_param.type = ANC_SENSOR_CMD_EFUSE_READ_MT_CUSTOM_MODULE_INFO;
    if (ANC_OK != (ret_val = SensorGetParam(&sensor_param))) {
        ANC_LOGE("fail to get module id, ret value: %d", ret_val);
    } else {
        AncMemcpy(p_module_id, &sensor_param.data.custom_module_info, module_id_size);
        p_module_id[module_id_size] = '\0';

        ANC_LOGD("Get Module Id: %s", p_module_id);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorRestoreDefaultImageSize(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;
    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));

    sensor_param.type = ANC_SENSOR_CMD_RESTORE_DEFAULT_IMAGE_SIZE;

    if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
        ANC_LOGE("fail to restore default image size, ret value: %d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorSetExposureTime(int32_t exposure_time_us) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;
    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));

    sensor_param.type = ANC_SENSOR_CMD_SET_EXPOSURE_TIME;
    sensor_param.data.exposure_time = exposure_time_us;

    if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
        ANC_LOGE("fail to set exposure time, ret value: %d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorSetRetryExposureTime(uint8_t type, int32_t retry0_exposure_time_us,
        int32_t retry1_exposure_time_us, int32_t retry2_exposure_time_us) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;
    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));

    sensor_param.type = ANC_SENSOR_CMD_SET_RETRY_EXPOSURE_TIME;
    sensor_param.data.retry_exposure_time.type = type;
    sensor_param.data.retry_exposure_time.retry0 = retry0_exposure_time_us;
    sensor_param.data.retry_exposure_time.retry1 = retry1_exposure_time_us;
    sensor_param.data.retry_exposure_time.retry2 = retry2_exposure_time_us;

    if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
        ANC_LOGE("fail to set retry exposure time, ret value: %d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorGetExposureTime(int32_t *exposure_time_us) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;
    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));

    sensor_param.type = ANC_SENSOR_CMD_GET_EXPOSURE_TIME;
    if (ANC_OK != (ret_val = SensorGetParam(&sensor_param))) {
        ANC_LOGE("fail to get exposure time, ret value: %d", ret_val);
    } else {
        ANC_LOGD("Get Exposure Time = %d us", sensor_param.data.exposure_time);
        *exposure_time_us = sensor_param.data.exposure_time;
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorGetTotalExposureTime(int32_t *exposure_time_ms) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;
    AncMemset(&sensor_param, 0, sizeof(AncSensorCommandParam));

    sensor_param.type = ANC_SENSOR_CMD_GET_TOTAL_EXPOSURE_TIME;
    if (ANC_OK != (ret_val = SensorGetParam(&sensor_param))) {
        ANC_LOGE("fail to get total exposure time, ret value: %d", ret_val);
    } else {
        *exposure_time_ms = (sensor_param.data.exposure_time / 1000 + 1);
        ANC_LOGD("Get Total Exposure Time = %d ms", *exposure_time_ms);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorSetFrameFusionNum(uint8_t retry0, uint8_t retry1, uint8_t retry2) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommandParam sensor_param;

    sensor_param.type = ANC_SENSOR_CMD_SET_FRAME_FUSION_NUM;
    sensor_param.data.fusion_param.retry0 = retry0;
    sensor_param.data.fusion_param.retry1 = retry1;
    sensor_param.data.fusion_param.retry2 = retry2;

    if (ANC_OK != (ret_val = SensorSetParam(&sensor_param))) {
        ANC_LOGE("fail to set frame fusion num, ret value: %d", ret_val);
    }

    return ret_val;
}

static ANC_RETURN_TYPE SensorTacCaptureImage(ANC_SENSOR_INNER_DATA_MODE mode, int32_t *p_exposure_time_us) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorCommand anc_sensor_command;
    AncSensorCommandRespond anc_sensor_respond;

    AncMemset(&anc_sensor_command, 0, sizeof(anc_sensor_command));
    AncMemset(&anc_sensor_respond, 0, sizeof(anc_sensor_respond));
    anc_sensor_command.command = ANC_CMD_SENSOR_CAPTURE_IMAGE;
    anc_sensor_command.data = (int32_t)mode;

#ifndef VIRTUAL_SENSOR
    ret_val = SensorTransmit(&anc_sensor_command, &anc_sensor_respond);
#else
    ANC_CA_LOCK_SHARED_BUFFER();

    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;
    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        return ret_val;
    }
    ret_val = AncGetVirtualSensorImage(p_share_buffer, share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get virtual sensor image");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        return ret_val;
    }

    ret_val = SensorTransmitNoLockSharedBuffer(&anc_sensor_command, &anc_sensor_respond);

    ANC_CA_UNLOCK_SHARED_BUFFER();
#endif
    if (NULL != p_exposure_time_us) {
        *p_exposure_time_us = anc_sensor_respond.param.data.exposure_time;
        ANC_LOGD("Exposure Time = %d us", *p_exposure_time_us);
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorCaptureImage(void) {
    return SensorTacCaptureImage(ANC_SENSOR_EXPOSURE_TRANSMIT, NULL);
}

ANC_RETURN_TYPE SensorCaptureImageWithMode(ANC_SENSOR_INNER_DATA_MODE mode, int32_t *p_exposure_time_us) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    /* reset fixed expo flag to false when retry0 */
    if (mode & ANC_SENSOR_FIRST_EXPOSURE) {
        g_use_fixed_expo_for_retry = ANC_FALSE;
    }

    if (!g_use_fixed_expo_for_retry) {
        // if (ANC_OK == (ret_val = SensorSetExposureTime(AUTO_EXPOSURE_TIME))) {
#ifdef ANC_QUICKLY_PICK_UP_FINGER
            if (mode & ANC_SENSOR_FIRST_EXPOSURE) {
                g_capture_last_expo_start_time = AncGetElapsedRealTimeMs();
            } else {
                g_capture_last_expo_start_time = g_capture_second_expo_start_time;
            }
#endif
            ret_val = SensorTacCaptureImage(mode, p_exposure_time_us);
            if (ANC_OK == ret_val) {
                g_abnormal_expo_type = ret_val;
            } else if ((ANC_CAPTURE_LOW_ENV_LIGHT == ret_val) || (ANC_CAPTURE_HIGH_ENV_LIGHT == ret_val)
                    || (ANC_CAPTURE_LOW_AUTO_EXP == ret_val)) {
                ANC_LOGE("Auto Expo error value is %d, set fixed expo for retry", ret_val);
                g_use_fixed_expo_for_retry = ANC_TRUE;
                g_abnormal_expo_type = ret_val;
            }
        // }
    }

    if (g_use_fixed_expo_for_retry) {
        ANC_LOGD("Use fixed expo for retry");
        mode &= ~ANC_SENSOR_SKIP_SCAN_CONFIG;
#ifdef ANC_QUICKLY_PICK_UP_FINGER
        if (mode & ANC_SENSOR_FIRST_EXPOSURE) {
            g_capture_last_expo_start_time = AncGetElapsedRealTimeMs();
        } else {
            g_capture_last_expo_start_time = g_capture_second_expo_start_time;
        }
#endif
        if (ANC_OK != (ret_val = SensorTacCaptureImage((mode | ANC_SENSOR_FIXED_EXPO_FOR_RETRY), NULL))) {
            ANC_LOGE("Fixed Expo capture error: return value = %d", ret_val);
        }
    }

#ifdef ANC_QUICKLY_PICK_UP_FINGER
    if (mode & ANC_SENSOR_SECOND_EXPOSURE) {
        g_capture_second_expo_start_time = AncGetElapsedRealTimeMs();
        if (mode & ANC_SENSOR_FIRST_EXPOSURE) {
            g_capture_retry0_total_expo_time = g_capture_second_expo_start_time - g_capture_last_expo_start_time;
        }
    }
#endif

    return ret_val;
}

ANC_RETURN_TYPE SensorGetAbnormalExpoType(void) {
    return g_abnormal_expo_type;
}

ANC_BOOL SensorIsAbnormalExpo(void) {
    return g_use_fixed_expo_for_retry;
}
