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
#include "anc_utils.h"
#include "anc_ca_image.h"
#include "anc_hal_extension_command.h"
#include "anc_memory_wrapper.h"
#include "cJSON.h"
#include "s2j.h"
#include "extension_command.h"
#include "anc_ft_result_handle.h"
#include "factory_test_ca.h"
#include "anc_tac_time.h"

#define CHECK_EXT_RET(_expr)               \
    if (ANC_OK != (ret_val = _expr)) {     \
        ANC_LOGE("failed %s", #_expr);     \
        break;                             \
    }

#define ADD_STRING_TO_ARRAY(_json_obj, _string_array, _array_len)                               \
    {                                                                                           \
        cJSON *p_array = cJSON_CreateArray();                                                   \
        if (p_array != NULL) {                                                                  \
            for (uint32_t i = 0; i < _array_len; i++) {                                         \
                cJSON_AddItemToArray(p_array, cJSON_CreateString(_string_array[i]));            \
            }                                                                                   \
            cJSON_AddItemToObject(_json_obj, "images", p_array);                                \
        } else {                                                                                \
            ANC_LOGE("failed create json array");                                               \
        }                                                                                       \
    }

typedef struct {
    char imei[40];
    char root_path[IMAGE_FILE_NAME_LEN];
    char seq_no[20];
}AncInitInputInfo;

static int32_t m_fixed_expo_time = 10000;
static int32_t m_dark_fixed_expo_time = 10000;

static AncFTConfig g_config;

static ANC_RETURN_TYPE JsonToInitInputInfo(const char* p_json, uint32_t len, AncInitInputInfo *p_info) {
    char *p_json_str = AncMalloc(len + 1);
    AncMemcpy(p_json_str, p_json, len);
    p_json_str[len] = '\0';
    ANC_LOGD("p_json_str:%s", p_json_str);

    cJSON *json_obj = cJSON_Parse(p_json_str);
    cJSON *json_imei =  cJSON_GetObjectItem(json_obj, "imei");
    cJSON *json_root_path = cJSON_GetObjectItem(json_obj, "root_path");
    cJSON *json_seq_no = cJSON_GetObjectItem(json_obj, "seq_no");

    char *p_imei_str = json_imei->valuestring;
    char *p_root_path = json_root_path->valuestring;
    char *p_seq_no = json_seq_no->valuestring;
    ANC_LOGD("value:%s, root_path:%s, seq_no:%s", p_imei_str, p_root_path, p_seq_no);

    AncMemset(p_info, 0, sizeof(AncInitInputInfo));
    AncMemcpy(p_info->imei, p_imei_str, strlen(p_imei_str));
    AncMemcpy(p_info->root_path, p_root_path, strlen(p_root_path));
    AncMemcpy(p_info->seq_no, p_seq_no, strlen(p_seq_no));

    cJSON_Delete(json_obj);
    AncFree(p_json_str);
    return ANC_OK;
}

static ANC_RETURN_TYPE InitOutInfoToJson(const ANC_SENSOR_EFUSE_CHIP_PROTO_INFO *p_chip_proto_info, const AncInitInputInfo *p_input_info, char *p_output_buffer, uint32_t buffer_len, char *p_path, uint32_t path_len) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    uint32_t cnt = (uint32_t)AncSnprintf(p_path, path_len, "%s/%s_%s_%s", p_input_info->root_path, p_input_info->imei, p_chip_proto_info->chip_info, p_input_info->seq_no);
    if (cnt >= path_len) {
        ANC_LOGE("Path buffer length is not enough, expect:%d, actual:%d", path_len, cnt);
        ret_val = ANC_FAIL;
    } else {
        cJSON *json_out_path = NULL;
        json_out_path = cJSON_CreateObject();
        cJSON_AddStringToObject(json_out_path, "path", p_path);
        cJSON_AddStringToObject(json_out_path, "module_id", p_chip_proto_info->chip_info);
        cJSON_PrintBuffer(json_out_path, p_output_buffer, (int)buffer_len);
        cJSON_Delete(json_out_path);
    }

    return ret_val;
}

static ANC_RETURN_TYPE FormatJsonOutput(cJSON *obj, uint32_t ret_val, char* p_output_buffer, uint32_t output_buffer_len,
     AncHalExtensionCommand *p_command_respond) {
    uint32_t *p_ret_val = (uint32_t*)p_output_buffer;
    *p_ret_val = ret_val;

    uint32_t ret_val_len = sizeof(uint32_t);
    char *p_json_buffer = p_output_buffer + ret_val_len;
    uint32_t json_buffer_len = output_buffer_len - ret_val_len;

    cJSON_PrintBuffer(obj, p_json_buffer, (int)json_buffer_len);
    p_command_respond->command_respond.p_buffer = (uint8_t*)p_output_buffer;
    p_command_respond->command_respond.buffer_length = (uint32_t)strlen(p_json_buffer) + ret_val_len;

    ANC_LOGD("FormatJsonOutput ret:%d, json_len:%d, json:%s", ret_val, (uint32_t)strlen(p_json_buffer), p_json_buffer);
    return ANC_OK;
}

static ANC_RETURN_TYPE DefectToJson(cJSON *p_root, const AncFTDefectResult *p_defect, int defect_count) {
    cJSON *p_array = cJSON_CreateArray();
    if (NULL == p_array)
    {
        ANC_LOGE("%s : fail to create cJSON array", __func__);
        return ANC_FAIL_MEMORY;
    }
    for (int i = 0; i < defect_count; i++) {
        cJSON *obj = cJSON_CreateObject();
        if (NULL == obj)
        {
            ANC_LOGE("%s : fail to create cJSON obj", __func__);
            return ANC_FAIL_MEMORY;
        }

        cJSON_AddNumberToObject(obj, "defect_total", p_defect[i].total);
        cJSON_AddNumberToObject(obj, "defect_lines", p_defect[i].lines);
        cJSON_AddNumberToObject(obj, "defect_clusters", p_defect[i].clusters);
        cJSON_AddNumberToObject(obj, "defect_max_cluster_size", p_defect[i].max_cluster_size);

        cJSON_AddItemToArray(p_array, obj);
    }

    cJSON_AddItemToObject(p_root, "defects", p_array);
    return ANC_OK;
}

static ANC_RETURN_TYPE SnrToJson(cJSON *p_root, const AncFTSnrResult *p_snr, int snr_count) {
    cJSON *p_array = cJSON_CreateArray();
    if (NULL == p_array)
    {
        ANC_LOGE("%s : fail to create cJSON array", __func__);
        return ANC_FAIL_MEMORY;
    }

    for (int i = 0; i < snr_count; i++) {
        cJSON *obj = cJSON_CreateObject();
        if (NULL == obj)
        {
            ANC_LOGE("%s : fail to create cJSON obj", __func__);
            return ANC_FAIL_MEMORY;
        }

        S2J_JSON_SET_ARRAY_ELEMENT(obj, &p_snr[i], int, snr, 9);
        cJSON_AddNumberToObject(obj, "snr_highest", p_snr[i].highest);
        cJSON_AddNumberToObject(obj, "snr_lowest", p_snr[i].lowest);

        S2J_JSON_SET_ARRAY_ELEMENT(obj, &p_snr[i], int, signal, 9);
        cJSON_AddNumberToObject(obj, "snr_highest_signal", p_snr[i].highest_signal);
        cJSON_AddNumberToObject(obj, "snr_lowest_signal", p_snr[i].lowest_signal);

        S2J_JSON_SET_ARRAY_ELEMENT(obj, &p_snr[i], int, noise, 9);
        cJSON_AddNumberToObject(obj, "snr_highest_noise", p_snr[i].highest_noise);
        cJSON_AddNumberToObject(obj, "snr_lowest_noise", p_snr[i].lowest_noise);

        cJSON_AddItemToArray(p_array, obj);
    }

    cJSON_AddItemToObject(p_root, "snrs", p_array);
    return ANC_OK;
}

ANC_RETURN_TYPE ExtCommandFTInit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    if ((p_command->command.p_buffer == NULL) || (p_command->command.buffer_length <= 0)) {
        return ANC_FAIL;
    }

    AncInitInputInfo info;
    char output_buffer[400] = {0};
    char path[200] = {0};
    uint32_t ret_val_len = 4;
    uint32_t json_output_buffer_len = (uint32_t)(sizeof(output_buffer) / sizeof(output_buffer[0])) - ret_val_len;

    do {
        CHECK_EXT_RET(p_manager->p_hbm_event_manager->SetHbm(p_manager, 1));
        CHECK_EXT_RET(JsonToInitInputInfo((char *)p_command->command.p_buffer, p_command->command.buffer_length, &info));
        CHECK_EXT_RET(p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP));

        AncSensorCommandParam param;
        param.type = ANC_SENSOR_CMD_EFUSE_READ_CHIP_PROTO_INFO;
        CHECK_EXT_RET(SensorGetParam(&param));
        CHECK_EXT_RET(ExtensionFTInit());

        CHECK_EXT_RET(InitOutInfoToJson(&param.data.chip_proto_info, &info, &output_buffer[ret_val_len],
                json_output_buffer_len, path, sizeof(path) / sizeof(path[0])));
        CHECK_EXT_RET(ANCSetImageSavePath(path));

        ExtensionFTGetConfig(&g_config);
    } while (0);

    uint32_t* p_ret_val = (uint32_t*)output_buffer;
    *p_ret_val = ret_val;
    uint32_t out_len = ret_val_len + (uint32_t)strlen(&output_buffer[ret_val_len]);
    ANC_LOGD("path json output:%s, len:%d, ret:%d", &output_buffer[ret_val_len], out_len, ret_val);
    p_command->command_respond.p_buffer = (uint8_t*)output_buffer;
    p_command->command_respond.buffer_length = out_len;

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);
    return ret_val;
}

#define MAKE_RETURE_VAL(p_buffer, val)        \
    do {                                                    \
        uint32_t* p_ret_val = (uint32_t*)p_buffer;          \
        *p_ret_val = val;                                   \
    } while(0)

ANC_RETURN_TYPE ExtCommandFTModuleTest(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    do {
        /// 设置固定曝光10ms
        CHECK_EXT_RET(SensorSetExposureTime(m_fixed_expo_time));

        ret_val = ExtensionFTModuleTest();
    } while (0);

    ANC_LOGD("ExtCommandFTModuleTest ret:%d", ret_val);

    uint8_t out_buffer[8] = {0};
    uint32_t* p_ret_val = (uint32_t*)out_buffer;
    *p_ret_val = ret_val;

    p_command->command_respond.p_buffer = out_buffer;
    p_command->command_respond.buffer_length = sizeof(uint32_t);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTModuleTestV2(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    do {
        ret_val = ExtensionFTModuleTestV2();
    } while (0);

    ANC_LOGD("ExtCommandFTModuleTest ret:%d", ret_val);

    uint8_t out_buffer[8] = {0};
    uint32_t* p_ret_val = (uint32_t*)out_buffer;
    *p_ret_val = ret_val;

    p_command->command_respond.p_buffer = out_buffer;
    p_command->command_respond.buffer_length = sizeof(uint32_t);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE SaveImageToFile(uint8_t *p_buffer, uint32_t p_buffer_length, int32_t cmd, uint32_t exp_type,
                int32_t exp_time, char *p_file_name, uint32_t file_name_len) {
#ifdef ANC_SAVE_ALGO_FILE
    if ((p_buffer == NULL) || (p_buffer_length == 0)) {
        ANC_LOGE("transmit image after got buffer error");
        return ANC_FAIL;
    }
    AncFTImageInfo *p_src_image = (AncFTImageInfo*)p_buffer;
    uint8_t *p_image_data = p_buffer + offsetof(AncFTImageInfo, p_image_data);

    AncFPImageData dest_image;
    dest_image.cmd = cmd;
    dest_image.exp_type = exp_type;
    dest_image.exp_time = exp_time;
    dest_image.width = p_src_image->width;
    dest_image.height = p_src_image->height;
    dest_image.image_size = p_src_image->image_size;
    dest_image.p_buffer = p_image_data;

    ANC_RETURN_TYPE ret = ANCSaveFPImage(&dest_image, p_file_name, file_name_len);
    if (ret != ANC_OK) {
        ANC_LOGE("failed save image, ret:%d", ret);
        return ret;
    }
#else
    ANC_UNUSED(p_buffer);
    ANC_UNUSED(p_buffer_length);
    ANC_UNUSED(cmd);
    ANC_UNUSED(exp_type);
    ANC_UNUSED(exp_time);
    ANC_UNUSED(p_file_name);
    ANC_UNUSED(file_name_len);
#endif
    return ANC_OK;
}

static ANC_RETURN_TYPE TransmitImage(int32_t cmd, uint32_t image_index, uint32_t exp_type, int32_t exp_time, char *p_file_name, uint32_t file_name_len) {
    uint8_t *p_buffer = NULL;
    uint32_t p_buffer_length = 0;
    ANC_RETURN_TYPE ret = ANC_OK;
    ret = ExtensionFTTransmitImage(image_index, &p_buffer, &p_buffer_length);
    if (ret != ANC_OK) {
        ANC_LOGE("failet transmit image, ret:%d", ret);
        return ret;
    }
    return SaveImageToFile(p_buffer, p_buffer_length, cmd, exp_type, exp_time, p_file_name, file_name_len);
}

static ANC_RETURN_TYPE TransmitImageFromCache(int32_t cmd, uint32_t image_index, char *p_file_name, uint32_t file_name_len) {
    uint8_t *p_buffer = NULL;
    uint32_t p_buffer_length = 0;
    int32_t expo_time = 0;
    ANC_RETURN_TYPE ret_val = ANC_OK;
    do {
         CHECK_EXT_RET(ExtensionFTTransmitImageFromCache(image_index, &expo_time, &p_buffer, &p_buffer_length));
         CHECK_EXT_RET(SaveImageToFile(p_buffer, p_buffer_length, cmd, 1, expo_time, p_file_name, file_name_len));

    } while (0);
    return ret_val;
}

static ANC_RETURN_TYPE SetBaseSum(uint32_t base_sum) {
    AncSensorCommandParam sensor_param;
    ANC_RETURN_TYPE ret_val = ANC_OK;

    do {
        sensor_param.type = ANC_SENSOR_CMD_WRITE_REGISTER;
        sensor_param.data.reg_param.addr = 0x28C6;
        sensor_param.data.reg_param.data = ((base_sum >> 8) & 0xFF);
        CHECK_EXT_RET(SensorSetParam(&sensor_param));

        sensor_param.type = ANC_SENSOR_CMD_WRITE_REGISTER;
        sensor_param.data.reg_param.addr = 0x28C7;
        sensor_param.data.reg_param.data = (base_sum & 0xFF);
        CHECK_EXT_RET(SensorSetParam(&sensor_param));
    } while (0);

    return ret_val;
}

typedef char (*AncFuncPtr)[IMAGE_FILE_NAME_LEN];
static ANC_RETURN_TYPE captureMulImage(int32_t cmd, const uint32_t *p_base_sum_arr, int32_t *p_exp_times, AncFuncPtr ptr, uint32_t len) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    // AncSensorCommandParam sensor_param;

    do {
        CHECK_EXT_RET(ExtensionFTReset());

        for (uint32_t i = 0; i < len; i++) {
            ///1. 设置自动曝光
            CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));

            ///2. base_sum
            SetBaseSum(p_base_sum_arr[i]);

            ///3. 抓图,获取曝光时间
            CHECK_EXT_RET(SensorCaptureImage());
            CHECK_EXT_RET(SensorGetExposureTime(&p_exp_times[i]));

            ///4. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
            CHECK_EXT_RET(TransmitImage(cmd, i, 0, p_exp_times[i], ptr[i], IMAGE_FILE_NAME_LEN));
        }
        if (ret_val != ANC_OK) {
            ANC_LOGE("an error occurred during image fetching in captureMulImage");
            break;
        }

    } while (0);

    return ret_val;
}

static ANC_RETURN_TYPE captureMulImageAutoTime(int32_t cmd, int32_t *p_exp_times, AncFuncPtr ptr, uint32_t len) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    do {
        CHECK_EXT_RET(ExtensionFTReset());

        for (uint32_t i = 0; i < len; i++) {
            CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));

            ///2. 抓图,获取曝光时间
            CHECK_EXT_RET(SensorCaptureImage());
            CHECK_EXT_RET(SensorGetExposureTime(&p_exp_times[i]));

            ///3. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
            CHECK_EXT_RET(TransmitImage(cmd, i, 0, p_exp_times[i], ptr[i], IMAGE_FILE_NAME_LEN));
        }
        if (ret_val != ANC_OK) {
            ANC_LOGE("an error occurred during image fetching in captureMulImage");
            break;
        }

    } while (0);

    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteAutoExpCalibration(AncFingerprintManager *p_manager) {
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    // AncSensorCommandParam sensor_param;
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint32_t target_base_sum = 0;
    int32_t target_exp_time = 0;
    char image_file_names[7][IMAGE_FILE_NAME_LEN] = {{0}, {0}, {0}, {0}, {0}};
    // uint32_t default_base_sum = 25000;

    do {
        // CHECK_EXT_RET(ExtensionFTReset());

        int32_t exp_times[7] = {0};
        uint32_t base_sum_arr[7] = {0, 5000, 10000, 15000, 20000, 25000, 30000};
        uint32_t len = sizeof(base_sum_arr) / sizeof(base_sum_arr[0]);
        CHECK_EXT_RET(captureMulImage(p_command->command_id, base_sum_arr,
                exp_times, image_file_names, len));

        /// 5. 调用ta算法实现计算base_sum的值
        CHECK_EXT_RET(ExtensionFTCalibrationBaseSum(base_sum_arr, len, &target_base_sum));

        /// 拿到对应base sum图的曝光时间
        for (uint32_t i = 0; i < len; i++) {
            if (target_base_sum == base_sum_arr[i]) {
                target_exp_time = exp_times[i];
                break;
            }
        }
         SetBaseSum(target_base_sum);
    } while (0);

    // white auto exp calibration JSON object
    s2j_create_json_obj(json_white_auto_exp_calibration);
    cJSON_AddNumberToObject(json_white_auto_exp_calibration, "base_sum", target_base_sum);
    cJSON_AddNumberToObject(json_white_auto_exp_calibration, "white_exposure_time", target_exp_time);
    ADD_STRING_TO_ARRAY(json_white_auto_exp_calibration, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    const int output_buffer_len = 1024;
    char *p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_white_auto_exp_calibration, ret_val, p_output_buffer, output_buffer_len, p_command);
    s2j_delete_json_obj(json_white_auto_exp_calibration);

    ANC_LOGD("ExtCommandFTWhiteAutoExpTest, ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    AncFree(p_output_buffer);
    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteExpCalibration(AncFingerprintManager *p_manager) {
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    ANC_RETURN_TYPE ret_val = ANC_OK;

    char image_file_names[2][IMAGE_FILE_NAME_LEN] = {{0}};
    AncFTExpoResult auto_expo_result;
    AncFTExpoResult fixed_expo_result;

    AncMemset(&auto_expo_result, 0 ,sizeof(auto_expo_result));
    AncMemset(&fixed_expo_result, 0 ,sizeof(fixed_expo_result));

    do {
        CHECK_EXT_RET(ExtensionFTReset());

        CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));
        /// 1. 自动曝光校准
        CHECK_EXT_RET(ExtensionFTExpoCalibration(0, &auto_expo_result));
        /// 2. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 0, 0, auto_expo_result.expo_time, image_file_names[0], IMAGE_FILE_NAME_LEN));

        /// 3. 固动曝光校准
        CHECK_EXT_RET(ExtensionFTReset());
        CHECK_EXT_RET(ExtensionFTExpoCalibration(1, &fixed_expo_result));

        m_fixed_expo_time = fixed_expo_result.expo_time;
        m_dark_fixed_expo_time = (int32_t)(m_fixed_expo_time/4.0);
        ANC_LOGD("fixed_expo_time=%d m_black_fixed_expo_time=%d", m_fixed_expo_time, m_dark_fixed_expo_time);

        /// 4. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 0, 1, fixed_expo_result.expo_time, image_file_names[1], IMAGE_FILE_NAME_LEN));


    } while (0);

    s2j_create_json_obj(json_white_expo);
    int32_t max_val = (auto_expo_result.expo_max_val) * 10000 + fixed_expo_result.expo_max_val;
    cJSON_AddNumberToObject(json_white_expo, "white_exposure_time", fixed_expo_result.expo_time);
    cJSON_AddNumberToObject(json_white_expo, "base_sum", max_val);
    ADD_STRING_TO_ARRAY(json_white_expo, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));
    char output_buffer[1024] = {0};

    FormatJsonOutput(json_white_expo, ret_val, output_buffer, sizeof(output_buffer), p_command);
    s2j_delete_json_obj(json_white_expo);

    ANC_LOGE("ExtCommandFTWhiteExpCalibration, ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhitePreventATest(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncFTWhitePreventInfo result;
    char image_file_names[2][IMAGE_FILE_NAME_LEN] = {{0}};
    int32_t exp_time = 0;

    AncMemset(&result, 0, sizeof(result));
    do {
        CHECK_EXT_RET(ExtensionFTReset());

        /// 1. 设置固定曝光10ms
        CHECK_EXT_RET(SensorSetExposureTime(m_fixed_expo_time));

        /// 2. 采图
        CHECK_EXT_RET(SensorCaptureImage());

        ///3. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 0, 1, m_fixed_expo_time, image_file_names[0], IMAGE_FILE_NAME_LEN));

        /// 增加对肉色头曝光时间的卡控
        CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));
        CHECK_EXT_RET(SensorCaptureImage());
        CHECK_EXT_RET(SensorGetExposureTime(&exp_time));
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 1, 0, exp_time, image_file_names[1], IMAGE_FILE_NAME_LEN));

        /// 3. 调用TA算法实现，并传回white_mean
        CHECK_EXT_RET(ExtensionWhitePrevent(2, &result));

    } while (0);

    // white prevent a JSON object
    s2j_create_json_obj(json_white_prevent_a);
    cJSON_AddNumberToObject(json_white_prevent_a, "white_mean", result.mean_info.mean);
    cJSON_AddNumberToObject(json_white_prevent_a, "white_exposure_time", result.exp_time);
    ADD_STRING_TO_ARRAY(json_white_prevent_a, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    char output_buffer[300] = {0};
    FormatJsonOutput(json_white_prevent_a, ret_val, output_buffer, sizeof(output_buffer), p_command);
    s2j_delete_json_obj(json_white_prevent_a);

    ANC_LOGD("ExtCommandFTWhitePreventATest ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteAutoExpATest(AncFingerprintManager *p_manager) {
    /// 肉色头自动曝光测试已经通过base sum的测试项拿到自动曝光时间了，此接口暂时不用调用
    ANC_UNUSED(p_manager);
    return ANC_OK;
}

ANC_RETURN_TYPE ExtCommandFTWhiteLensAndDefectTCNormal(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    char image_file_names[6][IMAGE_FILE_NAME_LEN] = {{0}, {0}, {0}, {0}, {0}, {0}};
    AncFTLensTestResult lens_test_result;
    AncFTDefectResult light_defect_result, dark_defect_result;
    int32_t exp_time = 0;


    AncMemset(&lens_test_result, 0, sizeof(lens_test_result));
    AncMemset(&light_defect_result, 0, sizeof(light_defect_result));
    AncMemset(&dark_defect_result, 0, sizeof(dark_defect_result));
    do {
        CHECK_EXT_RET(ExtensionFTReset());

        /// 1. 设置自动曝光
        CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));

        /// 2. 连续采图5张，都缓存到TA的test模块中
        const int count = 5;
        for (uint32_t i = 0; i < count; i++) {
            CHECK_EXT_RET(SensorCaptureImage());
            CHECK_EXT_RET(SensorGetExposureTime(&exp_time));

            ///3. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
            CHECK_EXT_RET(TransmitImage(p_command->command_id, i, 0, exp_time, image_file_names[i], IMAGE_FILE_NAME_LEN));
        }

        if (ANC_OK != ret_val) {
            break;
        }

        /// 用固定曝光取一张暗图,用于defect暗点的检测
        CHECK_EXT_RET(SensorSetExposureTime(m_dark_fixed_expo_time));
        CHECK_EXT_RET(SensorCaptureImage());
        CHECK_EXT_RET(TransmitImage(p_command->command_id, count, 1, m_dark_fixed_expo_time, image_file_names[count], IMAGE_FILE_NAME_LEN));


        /// 3. 做lens测试
        CHECK_EXT_RET(ExtensionFTLensTest(0, &lens_test_result));

        /// 4. 坏点坏块测试
        CHECK_EXT_RET(ExtensionFTDefectTest(0, &light_defect_result));
        CHECK_EXT_RET(ExtensionFTDefectTest(5, &dark_defect_result));

        /// 5. base图采集
        CHECK_EXT_RET(ExtensionFTBaseImageCalibration(count));
    } while (0);

    // white lens and dead JSON object
    s2j_create_json_obj(json_white_lens_and_defect);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "oc_x", lens_test_result.oc_result.x);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "oc_y", lens_test_result.oc_result.y);

    cJSON_AddNumberToObject(json_white_lens_and_defect, "hf_ratio", lens_test_result.hf_result.ratio);

    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_top_left", lens_test_result.sh_result.top_left);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_top_right", lens_test_result.sh_result.top_right);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_bottom_left", lens_test_result.sh_result.bottom_left);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_bottom_right", lens_test_result.sh_result.bottom_right);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_highest", lens_test_result.sh_result.highest);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_lowest", lens_test_result.sh_result.lowest);

    cJSON_AddNumberToObject(json_white_lens_and_defect, "fov_size", lens_test_result.fov_result.size);
    AncFTDefectResult defects[2] = {light_defect_result, dark_defect_result};
    DefectToJson(json_white_lens_and_defect,defects, sizeof(defects) / sizeof(defects[0]));

    ADD_STRING_TO_ARRAY(json_white_lens_and_defect, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    const int output_buffer_len = 2048;
    char *p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_white_lens_and_defect,
        ret_val, p_output_buffer, output_buffer_len,  p_command);
    s2j_delete_json_obj(json_white_lens_and_defect);

    ANC_LOGD("ExtCommandFTWhiteLensAndDeadTC ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);
    AncFree(p_output_buffer);
    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteLensAndDefectTCExtension(AncFingerprintManager *p_manager) {
   ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    char image_file_names[6][IMAGE_FILE_NAME_LEN] = {{0}, {0}, {0}, {0}, {0}, {0}};
    AncFTLensTestResult lens_test_result;
    AncFTDefectResult light_defect_result, dark_defect_result;
    int32_t exp_time = 0;


    AncMemset(&lens_test_result, 0, sizeof(lens_test_result));
    AncMemset(&light_defect_result, 0, sizeof(light_defect_result));
    AncMemset(&dark_defect_result, 0, sizeof(dark_defect_result));
    do {
        CHECK_EXT_RET(ExtensionFTReset());

        /// 1. 设置自动曝光
        CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));

        /// 2. 连续采图5张，都缓存到TA的test模块中
        const int count = 5;
        for (uint32_t i = 0; i < count; i++) {
            CHECK_EXT_RET(SensorCaptureImage());
            CHECK_EXT_RET(SensorGetExposureTime(&exp_time));

            ///3. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
            CHECK_EXT_RET(TransmitImage(p_command->command_id, i, 0, exp_time, image_file_names[i], IMAGE_FILE_NAME_LEN));
        }

        if (ANC_OK != ret_val) {
            break;
        }

        /// 用固定曝光取一张暗图,用于defect暗点的检测
        CHECK_EXT_RET(SensorSetExposureTime(m_dark_fixed_expo_time));
        CHECK_EXT_RET(SensorCaptureImage());
        CHECK_EXT_RET(TransmitImage(p_command->command_id, count, 1, m_dark_fixed_expo_time, image_file_names[count], IMAGE_FILE_NAME_LEN));


        /// 3. 做lens测试
        CHECK_EXT_RET(ExtensionFTLensTest(0, &lens_test_result));

        /// 4. 坏点坏块测试
        CHECK_EXT_RET(ExtensionFTDefectTest(0, &light_defect_result));
        CHECK_EXT_RET(ExtensionFTDefectTest(5, &dark_defect_result));

        /// 5. base图采集
        CHECK_EXT_RET(ExtensionFTBaseImageCalibration(count));

        if (g_config.white_entity.is_need_adjust && g_config.white_entity.capture_image_count > 0){
            CHECK_EXT_RET(ExtensionFTCaptureImageToCache((uint32_t)exp_time, g_config.white_entity.capture_image_count));
        } else {
            ANC_LOGE("unexpected call");
            ret_val = ANC_FAIL;
            break;
        }
    } while (0);

    // white lens and dead JSON object
    s2j_create_json_obj(json_white_lens_and_defect);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "oc_x", lens_test_result.oc_result.x);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "oc_y", lens_test_result.oc_result.y);

    cJSON_AddNumberToObject(json_white_lens_and_defect, "hf_ratio", lens_test_result.hf_result.ratio);

    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_top_left", lens_test_result.sh_result.top_left);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_top_right", lens_test_result.sh_result.top_right);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_bottom_left", lens_test_result.sh_result.bottom_left);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_bottom_right", lens_test_result.sh_result.bottom_right);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_highest", lens_test_result.sh_result.highest);
    cJSON_AddNumberToObject(json_white_lens_and_defect, "sh_lowest", lens_test_result.sh_result.lowest);

    cJSON_AddNumberToObject(json_white_lens_and_defect, "fov_size", lens_test_result.fov_result.size);
    AncFTDefectResult defects[2] = {light_defect_result, dark_defect_result};
    DefectToJson(json_white_lens_and_defect,defects, sizeof(defects) / sizeof(defects[0]));

    ADD_STRING_TO_ARRAY(json_white_lens_and_defect, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    const int output_buffer_len = 2048;
    char *p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_white_lens_and_defect,
        ret_val, p_output_buffer, output_buffer_len,  p_command);
    s2j_delete_json_obj(json_white_lens_and_defect);

    ANC_LOGD("ExtCommandFTWhiteLensAndDefectTCExtension ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);
    AncFree(p_output_buffer);
    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteLensAndDefectTC(AncFingerprintManager *p_manager) {
    if (g_config.white_entity.is_need_adjust) {
        return ExtCommandFTWhiteLensAndDefectTCExtension(p_manager);
    }
    return ExtCommandFTWhiteLensAndDefectTCNormal(p_manager);
}

ANC_RETURN_TYPE ExtCommandFTWhiteInstallOffsetTest(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncFTMarkPositioningResult icon_offset_result;
    char image_file_names[1][IMAGE_FILE_NAME_LEN] = {{0}};
    int32_t exp_time = 0;

    AncMemset(&icon_offset_result, 0, sizeof(icon_offset_result));
    do {
        CHECK_EXT_RET(ExtensionFTReset());

        /// 1. 设置自动曝光 todo: 固定曝光
        CHECK_EXT_RET(SensorSetExposureTime(12000));

         /// 2. 采图
        CHECK_EXT_RET(SensorCaptureImage());
        CHECK_EXT_RET(SensorGetExposureTime(&exp_time));

        ///3. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 0, 1, exp_time, image_file_names[0], IMAGE_FILE_NAME_LEN));

        /// 4. 调用TA算法
        CHECK_EXT_RET(ExtensionFTMarkPositioning(&icon_offset_result));

    } while (0);

    // white install offset JSON object
    s2j_create_json_obj(json_white_install_offset);
    cJSON_AddNumberToObject(json_white_install_offset, "x", icon_offset_result.x);
    cJSON_AddNumberToObject(json_white_install_offset, "y", icon_offset_result.y);
    ADD_STRING_TO_ARRAY(json_white_install_offset, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));
    char output_buffer[150] = {0};
    FormatJsonOutput(json_white_install_offset, ret_val, output_buffer,
        sizeof(output_buffer), p_command);
    s2j_delete_json_obj(json_white_install_offset);

    ANC_LOGD("ExtCommandFTWhiteInstallOffsetTest ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTBlackAllTC(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncFTBlackAlgoResult black_result;
    char image_file_names[6][IMAGE_FILE_NAME_LEN] = {{0}};
    int32_t exp_time = 0;

    AncMemset(&black_result, 0, sizeof(black_result));
    do {
        CHECK_EXT_RET(ExtensionFTReset());
        CHECK_EXT_RET(SensorSetExposureTime(m_fixed_expo_time));
        CHECK_EXT_RET(SensorCaptureImage());
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 0, 1, m_fixed_expo_time, image_file_names[0], IMAGE_FILE_NAME_LEN));

        CHECK_EXT_RET(SensorSetExposureTime(-1));
        /// TODO: 此处本应该采集一张图，但是为了配合jiachen测试特意添加的
        const uint32_t count = 5;
        for (uint32_t i = 0; i < count; i++) {
             /// 2. 采图
            CHECK_EXT_RET(SensorCaptureImage());
            CHECK_EXT_RET(SensorGetExposureTime(&exp_time));

            /// 3. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
            CHECK_EXT_RET(TransmitImage(p_command->command_id, i + 1, 0, exp_time, image_file_names[i + 1], IMAGE_FILE_NAME_LEN));
        }
        if (ANC_OK != ret_val) {
            break;
        }

        CHECK_EXT_RET(ExtensionFTBlackTest(&black_result));
    } while (0);

    // black all JSON object
    s2j_create_json_obj(json_black_all);
    cJSON_AddNumberToObject(json_black_all, "black_mean", black_result.mean_info.mean);
    cJSON_AddNumberToObject(json_black_all, "black_exposure_time", black_result.exp_time);
    cJSON_AddNumberToObject(json_black_all, "screen_signal", black_result.screen_signal);
    cJSON_AddNumberToObject(json_black_all, "screen_leak_ratio", black_result.screen_leak_ratio);
    ADD_STRING_TO_ARRAY(json_black_all, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    const int output_buffer_len = 1024;
    char *p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_black_all, ret_val, p_output_buffer, output_buffer_len, p_command);
    s2j_delete_json_obj(json_black_all);

    ANC_LOGD("ExtCommandFTBlackAllTC ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    AncFree(p_output_buffer);
    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteStripeAllTCNormal(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncFTSnrResult snr_result, tsnr_result;
    AncFTChartAnalyseResult chart_analyse_result;
    char image_file_names[26][IMAGE_FILE_NAME_LEN] = {{0}};
    int32_t exp_time = 0;

    AncMemset(&snr_result, 0, sizeof(snr_result));
    AncMemset(&tsnr_result, 0, sizeof(tsnr_result));
    AncMemset(&chart_analyse_result, 0, sizeof(chart_analyse_result));
    do {
        CHECK_EXT_RET(ExtensionFTReset());
        /// 设置自动曝光
        CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));
        CHECK_EXT_RET(SensorCaptureImage());
        CHECK_EXT_RET(SensorGetExposureTime(&exp_time));
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 0, 0, exp_time, image_file_names[0], IMAGE_FILE_NAME_LEN));

        CHECK_EXT_RET(SensorSetExposureTime(exp_time));
        uint32_t count = 25;
        for (uint32_t i = 0; i < count; i++) {
            /// 采图
            CHECK_EXT_RET(SensorCaptureImage());
            CHECK_EXT_RET(SensorGetExposureTime(&exp_time));

            ///3. 将图缓存到TA相应模块中，并且通过ion传回ca存储文件
            CHECK_EXT_RET(TransmitImage(p_command->command_id, i + 1, 1, exp_time, image_file_names[i + 1], IMAGE_FILE_NAME_LEN));
        }
        if (ANC_OK != ret_val) {
            break;
        }

        /// 空域噪声
        CHECK_EXT_RET(ExtensionFTSignalNoiseTest(0, &snr_result));
        /// 时域噪声
        CHECK_EXT_RET(ExtensionFTSignalNoiseTest(1, &tsnr_result));

        /// 安装偏差测试/放大倍数
        CHECK_EXT_RET(ExtensionFTFreqMFactor(count + 1, &chart_analyse_result));

    } while (0);

    ANC_LOGD("ExtCommandFTWhiteDiagonalAllTC ret:%d", ret_val);

    // white stripe JSON object
    s2j_create_json_obj(json_white_stripe);
    AncFTSnrResult snrs[2] = {snr_result, tsnr_result};
    SnrToJson(json_white_stripe, snrs, sizeof(snrs) / sizeof(snrs[0]));
    cJSON_AddNumberToObject(json_white_stripe, "ca_line_pair_count", chart_analyse_result.line_pair_count);
    cJSON_AddNumberToObject(json_white_stripe, "ca_rotation_degree", chart_analyse_result.rotation_degree);
    cJSON_AddNumberToObject(json_white_stripe, "ca_frequency_difference", chart_analyse_result.frequency_difference);
    ADD_STRING_TO_ARRAY(json_white_stripe, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    const int output_buffer_len = 4000;
    char* p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_white_stripe, ret_val, p_output_buffer, output_buffer_len, p_command);
    s2j_delete_json_obj(json_white_stripe);

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    AncFree(p_output_buffer);
    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteStripeAllTCExtension(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    AncFTSnrResult snr_result, tsnr_result;
    AncFTChartAnalyseResult chart_analyse_result;
    int32_t exp_time = 0;

    typedef char (FileNameArray)[IMAGE_FILE_NAME_LEN];
    FileNameArray *p_files_array = NULL;

    AncMemset(&snr_result, 0, sizeof(snr_result));
    AncMemset(&tsnr_result, 0, sizeof(tsnr_result));
    AncMemset(&chart_analyse_result, 0, sizeof(chart_analyse_result));

    if (g_config.snr_capture_image_total_count > 0 && g_config.stripe_entity.capture_image_count > 0) {
        p_files_array = AncMalloc((g_config.snr_capture_image_total_count + 1) * sizeof(FileNameArray));
    } else {
        ANC_LOGE("unexpected cal");
        return ANC_FAIL;
    }

    ANC_LOGD("snr_capture_image_total_cnt:%d, snr_capture_white_image_count:%d, snr_capture_stripe_image_count:%d",
                    g_config.snr_capture_image_total_count, g_config.snr_capture_white_image_count, g_config.snr_capture_stripe_image_count);

    do {
        CHECK_EXT_RET(ExtensionFTReset());
        /// 设置自动曝光
        CHECK_EXT_RET(SensorSetExposureTime(AUTO_EXPOSURE_TIME));
        CHECK_EXT_RET(SensorCaptureImage());
        CHECK_EXT_RET(SensorGetExposureTime(&exp_time));
        CHECK_EXT_RET(TransmitImage(p_command->command_id, 0, 0, exp_time, p_files_array[0], IMAGE_FILE_NAME_LEN));

        CHECK_EXT_RET(SensorSetExposureTime(exp_time));

        for (uint32_t i = 0; i < g_config.snr_capture_stripe_image_count; i++) {
            CHECK_EXT_RET(SensorCaptureImage());
            CHECK_EXT_RET(SensorGetExposureTime(&exp_time));
            CHECK_EXT_RET(TransmitImage(p_command->command_id, i + 1, 1, exp_time, p_files_array[i + 1], IMAGE_FILE_NAME_LEN));
        }

        for (uint32_t i = 0; i < g_config.snr_capture_white_image_count; i++) {
            CHECK_EXT_RET(TransmitImageFromCache(p_command->command_id, i, p_files_array[g_config.snr_capture_stripe_image_count + i + 1], IMAGE_FILE_NAME_LEN));
        }

        /// 空域噪声
        CHECK_EXT_RET(ExtensionFTSignalNoiseTest(0, &snr_result));
        /// 时域噪声
        CHECK_EXT_RET(ExtensionFTSignalNoiseTest(1, &tsnr_result));

        /// 安装偏差测试/放大倍数
        CHECK_EXT_RET(ExtensionFTFreqMFactor(g_config.snr_capture_stripe_image_count + 1, &chart_analyse_result));

    } while (0);

    ANC_LOGD("ExtCommandFTWhiteStripeAllTCExtension ret:%d", ret_val);

    // white stripe JSON object
    s2j_create_json_obj(json_white_stripe);
    AncFTSnrResult snrs[2] = {snr_result, tsnr_result};
    SnrToJson(json_white_stripe, snrs, sizeof(snrs) / sizeof(snrs[0]));
    cJSON_AddNumberToObject(json_white_stripe, "ca_line_pair_count", chart_analyse_result.line_pair_count);
    cJSON_AddNumberToObject(json_white_stripe, "ca_rotation_degree", chart_analyse_result.rotation_degree);
    cJSON_AddNumberToObject(json_white_stripe, "ca_frequency_difference", chart_analyse_result.frequency_difference);
    ADD_STRING_TO_ARRAY(json_white_stripe, p_files_array, g_config.snr_capture_image_total_count + 1);

    const int output_buffer_len = 4000;
    char* p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_white_stripe, ret_val, p_output_buffer, output_buffer_len, p_command);
    s2j_delete_json_obj(json_white_stripe);

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    AncFree(p_output_buffer);
    if (p_files_array != NULL) {
        AncFree(p_files_array);
    }
    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTWhiteStripeAllTC(AncFingerprintManager *p_manager) {
    if (g_config.stripe_entity.is_need_adjust) {
        return ExtCommandFTWhiteStripeAllTCExtension(p_manager);
    }

    return ExtCommandFTWhiteStripeAllTCNormal(p_manager);
}

ANC_RETURN_TYPE ExtCommandFTSaveCalibrationData(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    ret_val = ExtensionSaveCalibration();
    if (ret_val == ANC_OK) {
        ExtensionSetCaliProperty(ANC_TRUE);
    } else {
        ExtensionSetCaliProperty(ANC_FALSE);
    }
    uint8_t out_buffer[8] = {0};
    uint32_t* p_ret_val = (uint32_t*)out_buffer;
    *p_ret_val = ret_val;

    p_command->command_respond.p_buffer = out_buffer;
    p_command->command_respond.buffer_length = sizeof(uint32_t);

    ANC_LOGD("ExtCommandFTSaveCalibrationData ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}

ANC_RETURN_TYPE ExtCommandFTCaptureFingerImage(AncFingerprintManager *p_manager) {
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    ANC_RETURN_TYPE ret_val = ANC_OK;
    char image_file_names[7][IMAGE_FILE_NAME_LEN] = {{0}, {0}, {0}, {0}, {0}};
    int32_t exp_times[7] = {0};
    uint32_t base_sum_arr[7] = {0, 5000, 10000, 15000, 20000, 25000, 30000};
    uint32_t len = sizeof(base_sum_arr) / sizeof(base_sum_arr[0]);

    do {
        CHECK_EXT_RET(captureMulImage(p_command->command_id, base_sum_arr,
                exp_times, image_file_names, len));
    } while (0);

    s2j_create_json_obj(json_capture_image);
    ADD_STRING_TO_ARRAY(json_capture_image, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    const int output_buffer_len = 1024;
    char *p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_capture_image, ret_val, p_output_buffer, output_buffer_len, p_command);
    s2j_delete_json_obj(json_capture_image);

    ANC_LOGD("ExtCommandFTCaptureFingerImage, ret:%d", ret_val);
    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    AncFree(p_output_buffer);
    return ANC_OK;
}

ANC_RETURN_TYPE ExtCommandFTCaptureFingerImageV2(AncFingerprintManager *p_manager) {
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    ANC_RETURN_TYPE ret_val = ANC_OK;
    char image_file_names[7][IMAGE_FILE_NAME_LEN] = {{0}, {0}, {0}, {0}, {0},{0}, {0}};
    int32_t exp_times[7] = {0};

    do {
        CHECK_EXT_RET(captureMulImageAutoTime(p_command->command_id, exp_times, image_file_names, 7));
    } while (0);

    s2j_create_json_obj(json_capture_image);
    ADD_STRING_TO_ARRAY(json_capture_image, image_file_names, sizeof(image_file_names) / sizeof(image_file_names[0]));

    const int output_buffer_len = 1024;
    char *p_output_buffer = AncMalloc(output_buffer_len);
    FormatJsonOutput(json_capture_image, ret_val, p_output_buffer, output_buffer_len, p_command);
    s2j_delete_json_obj(json_capture_image);

    ANC_LOGD("ExtCommandFTCaptureFingerImage, ret:%d", ret_val);

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    AncFree(p_output_buffer);
    return ANC_OK;
}

ANC_RETURN_TYPE ExtCommandFTDeinit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);

    do {
        CHECK_EXT_RET(p_manager->p_hbm_event_manager->SetHbm(p_manager, 0));
        CHECK_EXT_RET(ExtensionFTDeInit());
    } while (0);

    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);

    uint8_t out_buffer[8] = {0};
    uint32_t* p_ret_val = (uint32_t*)out_buffer;
    *p_ret_val = ret_val;

    p_command->command_respond.p_buffer = out_buffer;
    p_command->command_respond.buffer_length = sizeof(uint32_t);

    p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);

    return ret_val;
}
