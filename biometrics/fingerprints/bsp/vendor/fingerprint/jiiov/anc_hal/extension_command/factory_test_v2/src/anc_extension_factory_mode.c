#define LOG_TAG "[ANC_HAL][MMI]"

#include "anc_extension_command.h"

#include "anc_log.h"
#include "anc_ca.h"
#include "anc_memory_wrapper.h"
#include "factory_test_ca.h"
#include "anc_lib.h"
#include "anc_hal_sensor_device.h"
#include "anc_algorithm.h"
#include <string.h>
#include "anc_lib.h"
#include <stdio.h>

#define FACTORY_MODE_OUTPUT_BUFFER_SIZE 3000

typedef ANC_RETURN_TYPE (*FTFunctionPointer)(AncFingerprintManager *p_manager);
typedef struct {
    uint32_t operation_code;
    FTFunctionPointer p_function;
    const char *p_function_description;
} AncFactoryTestExternalOperation;

typedef union {
    uint8_t output_buffer[FACTORY_MODE_OUTPUT_BUFFER_SIZE];
    struct AncFTOut {
        uint32_t report_ret_val;
        char out[FACTORY_MODE_OUTPUT_BUFFER_SIZE-4];
    } data;
} AncFactoryTestOutputBuffer;
static AncFactoryTestOutputBuffer g_output_buffer;
static AncFactoryTestOutputBuffer *gp_output_buffer = &g_output_buffer;

ANC_RETURN_TYPE FteoStartup(struct AncFingerprintManager *p_manager) {
#ifdef ANC_SENSOR_SPI_MTK
    if (ANC_OK != SensorDeviceOpenSpiClk()) {
        ANC_LOGE("factory test, fail to open sensor spi clk");
        return ANC_FAIL_SPI_OPEN;
    }
#endif
    ANC_LOGD("mmi startup");
    ANC_UNUSED(p_manager);
    return ANC_OK;
}

ANC_RETURN_TYPE FteoSetHbmOn(struct AncFingerprintManager *p_manager) {
    return p_manager->p_hbm_event_manager->SetHbm(p_manager, ANC_TRUE);
}

ANC_RETURN_TYPE FteoSetHbmOff(struct AncFingerprintManager *p_manager) {
    return p_manager->p_hbm_event_manager->SetHbm(p_manager, ANC_FALSE);
}

ANC_RETURN_TYPE FteoSetCalibrationSucceess(struct AncFingerprintManager *p_manager) {
    ANC_UNUSED(p_manager);
    return ExtensionSetCaliProperty(ANC_TRUE);
}

ANC_RETURN_TYPE FteoSetCalibrationFail(struct AncFingerprintManager *p_manager) {
    ANC_UNUSED(p_manager);
    return ExtensionSetCaliProperty(ANC_FALSE);
}

ANC_RETURN_TYPE FteoInit(struct AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (ANC_OK != (ret_val = FteoSetHbmOn(p_manager))) {
        ANC_LOGD("factory test init, set hbm on, return value:%d", ret_val);
        return ret_val;
    }

    return ret_val;
}

ANC_RETURN_TYPE FteoDeinit(struct AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (ANC_OK != (ret_val = FteoSetHbmOff(p_manager))) {
        ANC_LOGD("factory test deinit, set hbm off, return value:%d", ret_val);
        return ret_val;
    }

#ifdef ANC_SENSOR_SPI_MTK
    if (ANC_OK != (ret_val = SensorDeviceCloseSpiClk())) {
        ANC_LOGD("factory test deinit, sensor sleep, return value:%d", ret_val);
        return ret_val;
    }
#endif

    return ret_val;
}

AncFactoryTestExternalOperation g_factory_test_external_operation[ANC_CMD_EXTENSION_FTEO_MAX+1] = {
    {ANC_CMD_EXTENSION_FTEO_NONE, NULL, "NO OPERATION"},
    {ANC_CMD_EXTENSION_FTEO_STARTUP, FteoStartup, "STARTUP"},
    {ANC_CMD_EXTENSION_FTEO_SET_HBM_ON, FteoSetHbmOn, "SET HBM ON"},
    {ANC_CMD_EXTENSION_FTEO_SET_HBM_OFF, FteoSetHbmOff, "SET HBM OFF"},
    {ANC_CMD_EXTENSION_FTEO_SET_CALIBRATION_SUCCEESS, FteoSetCalibrationSucceess, "SET CALIBRATION SUCCEESS"},
    {ANC_CMD_EXTENSION_FTEO_SET_CALIBRATION_FAIL, FteoSetCalibrationFail, "SET CALIBRATION FAIL"},
    {ANC_CMD_EXTENSION_FTEO_INIT, FteoInit, "INIT"},
    {ANC_CMD_EXTENSION_FTEO_DEINIT, FteoDeinit, "DEINIT"},
    {ANC_CMD_EXTENSION_FTEO_MAX, NULL, "NO OPERATION"},
};

ANC_RETURN_TYPE FactoryModeExternalOperation(AncFingerprintManager *p_manager, uint32_t operation_code) {
    if ((operation_code >= ANC_CMD_EXTENSION_FTEO_MAX)
        || (NULL == g_factory_test_external_operation[operation_code].p_function)) {

        ANC_LOGE("no external factory mode operation:%d, max:%d", operation_code,
                    ANC_CMD_EXTENSION_FTEO_MAX);
        return ANC_FAIL;
    }
    ANC_LOGD("do external factory mode operation:%d, %s", operation_code,
            g_factory_test_external_operation[operation_code].p_function_description);

    return g_factory_test_external_operation[operation_code].p_function(p_manager);
}

#ifdef ANC_SAVE_ALGO_FILE
static ANC_RETURN_TYPE FactoryModeTransmitImageGetCount(uint32_t *p_count) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (NULL == p_count) {
        ANC_LOGE("factory mode transmit image get count:%p", p_count);
        return ANC_FAIL;
    }
    *p_count = 0;

    AncFactoryTestCommand command;
    AncFactoryTestCommandRespond *p_command_respond = NULL;
    AncMemset(&command, 0, sizeof(command));
    command.command = (uint32_t)FACTORY_TEST_TRANSMIT_IMAGE_GET_COUNT;
    ret_val = ExtensionFactoryTest(&command, &p_command_respond);
    if (NULL != p_command_respond) {
        if ((p_command_respond->buffer_length > 0)
          && (p_command_respond->buffer_length <= sizeof(AncFtTransmitImageInfo))) {

            uint8_t *p_output_buffer = (uint8_t *)p_command_respond;
            p_output_buffer += offsetof(AncFactoryTestCommandRespond, p_buffer);
            AncFtTransmitImageInfo *p_image_info = (AncFtTransmitImageInfo *)p_output_buffer;
            *p_count = p_image_info->remain_count;
        } else {
            ANC_LOGE("factory test transmit image get count respond is error, buffer length:%d, max length:%d",
                        p_command_respond->buffer_length, sizeof(AncFtTransmitImageInfo));
            ret_val = ANC_FAIL;
        }
    } else {
        ANC_LOGD("factory test transmit image get count respond:%p", p_command_respond);
        ret_val = ANC_FAIL;
    }

    return ret_val;
}


static ANC_RETURN_TYPE FactoryModeSaveImage(AncFtTransmitImageInfo *p_image_info) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    uint8_t *p_file_path = p_image_info->image_path;
    uint8_t *p_file_name = p_image_info->image_name;
    uint32_t image_size = p_image_info->image_size;
    uint32_t image_width = p_image_info->image_width;
    uint32_t image_height = p_image_info->image_height;
    uint32_t image_bit = p_image_info->image_bit;
    uint32_t valid_bit = p_image_info->image_valid_bit;
    uint8_t *p_image_data = (uint8_t *)p_image_info + offsetof(AncFtTransmitImageInfo, p_image_buffer);
    if ((NULL == p_file_path) || (NULL == p_file_name)
       || (NULL == p_image_data) || (0 == image_size)) {
        ANC_LOGE("factory mode save image, file path:%p, file name:%p, image data:%p, image size:%d",
                 p_file_path, p_file_name, p_image_data, image_size);
        return ANC_FAIL;
    }

    int write_data_size = AncWriteFile((char *)p_file_path, (char *)p_file_name,
                                        p_image_data, image_size);
    if ((uint32_t)write_data_size != image_size) {
        ANC_LOGE("fail to save image : write size:%d, image size:%d\n",
                   write_data_size, image_size);
        return ANC_FT_SAVE_IMAGE_FAIL;
    }

#ifdef ANC_SAVE_BMP_FILE
    char bmp_img_name[256] = {0};
    sprintf(bmp_img_name, "%s.bmp", p_file_name);
    uint32_t img_buf_len = image_width * image_height * 4;
    uint32_t bmp_size = 0;
    uint8_t *p_bmp_buf = AncMalloc(img_buf_len);
    if (p_bmp_buf == NULL) {
        ANC_LOGE("heap size not enough, save bmp file failed");
        return ANC_FT_SAVE_IMAGE_FAIL;
    }
    ImageBin img_bin = {(int32_t)image_width, (int32_t)image_height, (int32_t)image_bit, (int32_t)valid_bit, (uint8_t *)p_image_data};
    if (ANC_OK != (ret_val = AncConvertBin2BMP(&img_bin, p_bmp_buf, img_buf_len, &bmp_size))) {
        ANC_LOGE("fail to convert bmp image, ret: %d", ret_val);
        AncFree(p_bmp_buf);
        return ret_val;
    }
    write_data_size = AncWriteFile((char *)p_file_path, bmp_img_name,
                                    p_bmp_buf, bmp_size);
    AncFree(p_bmp_buf);
    if ((uint32_t)write_data_size != bmp_size) {
        ANC_LOGE("fail to save image : write size:%d, image size:%d\n",
                write_data_size, image_size);
        return ANC_FT_SAVE_IMAGE_FAIL;
    }
#else
    ANC_UNUSED(ret_val);
    ANC_UNUSED(image_width);
    ANC_UNUSED(image_height);
    ANC_UNUSED(image_bit);
    ANC_UNUSED(valid_bit);
#endif

    return ANC_OK;
}

static ANC_RETURN_TYPE FactoryModeTransmitImage() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncFactoryTestCommand command;
    AncFactoryTestCommandRespond *p_command_respond = NULL;
    AncMemset(&command, 0, sizeof(command));
    command.command = (uint32_t)FACTORY_TEST_TRANSMIT_IMAGE;
    ret_val = ExtensionFactoryTest(&command, &p_command_respond);
    if (NULL != p_command_respond) {
        if (p_command_respond->buffer_length >= sizeof(AncFtTransmitImageInfo)) {
            uint8_t *p_output_buffer = (uint8_t *)p_command_respond;
            p_output_buffer += offsetof(AncFactoryTestCommandRespond, p_buffer);
            AncFtTransmitImageInfo *p_image_info = (AncFtTransmitImageInfo *)p_output_buffer;
            ret_val = FactoryModeSaveImage(p_image_info);
        } else {
            ANC_LOGE("factory test transmit image respond is error, buffer length:%d, max length:%d",
                        p_command_respond->buffer_length, sizeof(AncFtTransmitImageInfo));
            ret_val = ANC_FAIL;
        }
    } else {
        ANC_LOGD("factory test transmit image respond:%p", p_command_respond);
        ret_val = ANC_FAIL;
    }

    return ret_val;
}

static ANC_RETURN_TYPE FactoryModeExtensionTransmitImage() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint32_t count = 0;

    if (ANC_OK == (ret_val = FactoryModeTransmitImageGetCount(&count))) {
        ANC_LOGD("factory test transmit image count:%d", count);
        while(count-- > 0) {
            if (ANC_OK != (ret_val = FactoryModeTransmitImage())) {
                ANC_LOGE("factory test transmit image fail, return value:%d", ret_val);
                break;
            }
        }
    }

    return ret_val;
}
#endif

static ANC_RETURN_TYPE FactoryModeExtensionCommandCommon(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    AncHalExtensionCommand *p_command = &(p_manager->p_producer->command);
    int32_t command_id = p_command->command_id;

    AncFactoryTestCommand command;
    AncFactoryTestCommandRespond *p_command_respond = NULL;
    AncMemset(&command, 0, sizeof(command));
    command.command = (uint32_t)command_id;
    command.buffer_length = p_command->command.buffer_length;
    command.p_buffer = (uint8_t *)p_command->command.p_buffer;
    ret_val = ExtensionFactoryTest(&command, &p_command_respond);
    if (NULL != p_command_respond) {
        if ((p_command_respond->buffer_length > 0)
          && (p_command_respond->buffer_length <= sizeof(AncFactoryTestOutputBuffer))) {
            AncMemset(gp_output_buffer, 0, sizeof(AncFactoryTestOutputBuffer));
            uint8_t *p_output_buffer = (uint8_t *)p_command_respond;
            p_output_buffer += offsetof(AncFactoryTestCommandRespond, p_buffer);
            AncMemcpy(gp_output_buffer->output_buffer, p_output_buffer, p_command_respond->buffer_length);
            p_command->command_respond.p_buffer = gp_output_buffer->output_buffer;
            p_command->command_respond.buffer_length = p_command_respond->buffer_length;
            ANC_LOGD("factory test command respond, buffer length:%d, return value:%d, \nstring:%s",
                        p_command_respond->buffer_length, gp_output_buffer->data.report_ret_val,
                        gp_output_buffer->data.out);

            if ((ANC_CMD_EXTENSION_FTEO_NONE < p_command_respond->operation_code) &&
                    (p_command_respond->operation_code < ANC_CMD_EXTENSION_FTEO_MAX)) {
                ANC_RETURN_TYPE operation_ret_val = FactoryModeExternalOperation(p_manager,
                                                        p_command_respond->operation_code);
                if (ANC_OK != operation_ret_val) {
                    ANC_LOGE("fail to do external factory mode operation:%d", operation_ret_val);
                    return operation_ret_val;
                }
            } else {
                ANC_LOGD("need not do external factory mode operation:%d", p_command_respond->operation_code);
            }

#ifdef ANC_SAVE_ALGO_FILE
            if (p_command_respond->need_transmit_image) {
                if (ANC_OK != (ret_val = FactoryModeExtensionTransmitImage())) {
                    ANC_LOGE("factory test transmit image is fail, return value:%d", ret_val);
                }
            }
#endif

            p_manager->p_producer->OnExcuteCommand(p_device,
                        p_command->command_id, (int32_t)ret_val,
                        p_command->command_respond.p_buffer,
                        p_command->command_respond.buffer_length);
        } else {
            ANC_LOGE("factory test command respond is error, buffer length:%d, max length:%d",
                        p_command_respond->buffer_length, sizeof(AncFactoryTestOutputBuffer));
            ret_val = ANC_FAIL;
        }
    } else {
        ANC_LOGD("factory test command respond:%p", p_command_respond);
    }
    ANC_LOGD("factory test command id:%d, return value:%d", command_id, ret_val);

    return ret_val;
}

ANC_RETURN_TYPE FactoryModeExtensionCommand(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("algo version:%s", g_algo_version_string);

    if(0 == strncmp((const char *)g_algo_version_string, "JV3", 3)) {
        ret_val = FteoStartup(p_manager);
        if(ANC_OK != ret_val) {
            ANC_LOGD("FteoStartup error :%d", ret_val);
            return ret_val;
        }
    }

    return FactoryModeExtensionCommandCommon(p_manager);
}
