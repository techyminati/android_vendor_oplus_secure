#define LOG_TAG "[ANC_TAC][MMI]"

#include "factory_test_ca.h"

#include "anc_ca.h"
#include "anc_extension.h"
#include "anc_log.h"
#include "anc_memory_wrapper.h"
#include "anc_lib.h"

#define TMP_CALIBRATION_DATA_SAVE_DIR_PATH "/mnt/vendor/persist/fingerprint/jiiov"

static ANC_RETURN_TYPE ExtensionFTCommonWithInput(uint32_t cmd, uint32_t data) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = cmd;
    anc_extension_command.data = data;

    ret_val = ExtensionTransmit(&anc_extension_command, &anc_extension_respond);
    ANC_LOGD("FT Cmd %d call, ret:%d", cmd, ret_val);

    return ret_val;
}

static ANC_RETURN_TYPE ExtensionFTCommon(uint32_t cmd) {
    return ExtensionFTCommonWithInput(cmd, 0);
}

static ANC_RETURN_TYPE ExtensionFTCommonWithOutput(AncExtensionCommand *p_req,
                                                   void *p_result,
                                                   uint32_t result_size) {
    if (NULL == p_req) {
        ANC_LOGE("cannot be a null pointer");
        return ANC_FAIL;
    }

    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommandRespond anc_extension_respond;
    uint32_t share_buffer_size = 0;
    uint8_t *p_share_buffer = NULL;

    ANC_CA_LOCK_SHARED_BUFFER();

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        goto DO_FAIL;
    }

    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    ret_val = ExtensionTransmitNoLockSharedBuffer(p_req, &anc_extension_respond);
    if ((p_result != NULL) && (result_size > 0)) {
        AncMemcpy(p_result, p_share_buffer, result_size);
    }

DO_FAIL:
    ANC_CA_UNLOCK_SHARED_BUFFER();

    return ret_val;
}

static ANC_RETURN_TYPE ExtensionFTCommonWithImageCount(uint32_t cmd,
                                                       uint32_t image_count,
                                                       void *p_result,
                                                       uint32_t result_size) {
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = cmd;
    anc_extension_command.image_index = image_count;

    return ExtensionFTCommonWithOutput(&anc_extension_command, p_result,
                                       result_size);
}

ANC_RETURN_TYPE ExtensionFTInit() {
    return ExtensionFTCommon(ANC_CMD_EXTENSION_FT_INIT);
}

ANC_RETURN_TYPE ExtensionFTModuleTest() {
    return ExtensionFTCommon(ANC_CMD_EXTENSION_FT_MODULE_TEST);
}

ANC_RETURN_TYPE ExtensionFTModuleTestV2() {
    return ExtensionFTCommon(ANC_CMD_EXTENSION_FT_MODULE_TEST_V2);
}

ANC_RETURN_TYPE ExtensionFTTransmitImage(uint32_t idx, uint8_t **pp_buffer,
                                         uint32_t *p_buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    if ((NULL == pp_buffer) || (NULL == p_buffer_length)) {
        ANC_LOGE("parameter cannot be a null pointer in transmit image");
        return ANC_FAIL;
    }

    ANC_CA_LOCK_SHARED_BUFFER();

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_TRANSMIT_IMAGE;
    anc_extension_command.image_index = idx;

    ret_val = AncGetIonSharedBuffer(pp_buffer, p_buffer_length);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        goto DO_FAIL;
    }

    ret_val = ExtensionTransmitNoLockSharedBuffer(&anc_extension_command, &anc_extension_respond);
    ANC_LOGD("transmit image called. ret:%d", ret_val);

DO_FAIL:
    ANC_CA_UNLOCK_SHARED_BUFFER();

    return ret_val;
};

ANC_RETURN_TYPE ExtensionFTTransmitImageFromCache(uint32_t idx, int32_t *p_expo_time, uint8_t **pp_buffer,
                                         uint32_t *p_buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    if ((NULL == pp_buffer) || (NULL == p_buffer_length)) {
        ANC_LOGE("parameter cannot be a null pointer in transmit image");
        return ANC_FAIL;
    }

    ANC_CA_LOCK_SHARED_BUFFER();

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_TRANSMIT_IMAGE_FROM_CACHE;
    anc_extension_command.image_index = idx;

    ret_val = AncGetIonSharedBuffer(pp_buffer, p_buffer_length);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        goto DO_FAIL;
    }

    ret_val = ExtensionTransmitNoLockSharedBuffer(&anc_extension_command, &anc_extension_respond);
    *p_expo_time = (int32_t)anc_extension_respond.data;

DO_FAIL:
    ANC_CA_UNLOCK_SHARED_BUFFER();

    return ret_val;
}

ANC_RETURN_TYPE ExtensionFTCaptureImageToCache(uint32_t expo_time, uint32_t image_count) {
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_CAPTURE_IMAGE_TO_CACHE;
    anc_extension_command.image_index = image_count;
    anc_extension_command.data = expo_time;

    AncExtensionCommandRespond anc_extension_respond;
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));

    return ExtensionTransmit(&anc_extension_command, &anc_extension_respond);
}

ANC_RETURN_TYPE ExtensionFTCalibrationBaseSum(uint32_t *base_sum_arr,
                                              uint32_t len,
                                              uint32_t *base_sum) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    if (len != sizeof(anc_extension_command.base_sum_arr) /
                   sizeof(anc_extension_command.base_sum_arr[0])) {
        ANC_LOGE("base sum array is not long enough");
        return ANC_FAIL;
    }

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_CALIBRATION_BASE_SUM;
    AncMemcpy(anc_extension_command.base_sum_arr, base_sum_arr,
              sizeof(anc_extension_command.base_sum_arr));

    ret_val = ExtensionTransmit(&anc_extension_command, &anc_extension_respond);
    if (ret_val == ANC_OK) {
        *base_sum = anc_extension_respond.data;
    }
    return ret_val;
}

ANC_RETURN_TYPE ExtensionFTMarkPositioning(AncFTMarkPositioningResult *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("mark position result cannot be a null pointer");
        return ANC_FAIL;
    }
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_MARK_POSITION;
    return ExtensionFTCommonWithOutput(&anc_extension_command, p_result,
                                       sizeof(AncFTMarkPositioningResult));
}

ANC_RETURN_TYPE ExtensionWhitePrevent(uint32_t image_count, AncFTWhitePreventInfo *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("mean info result cannot be a null pointer");
        return ANC_FAIL;
    }
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_CAL_MEAN;
    anc_extension_command.image_index = image_count;

    return ExtensionFTCommonWithOutput(&anc_extension_command, p_result,
                                       sizeof(AncFTWhitePreventInfo));
}

ANC_RETURN_TYPE ExtensionFTLensTest(uint32_t image_index,
                                    AncFTLensTestResult *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("lens test result cannot be a null pointer");
        return ANC_FAIL;
    }
    return ExtensionFTCommonWithImageCount(ANC_CMD_EXTENSION_FT_LENS_TEST,
                                           image_index, p_result,
                                           sizeof(AncFTLensTestResult));
}

ANC_RETURN_TYPE ExtensionFTDefectTest(uint32_t image_index,
                                      AncFTDefectResult *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("defect result cannot be a null pointer");
        return ANC_FAIL;
    }
    return ExtensionFTCommonWithImageCount(ANC_CMD_EXTENSION_FT_DEFECT_TEST,
                                           image_index, p_result,
                                           sizeof(AncFTDefectResult));
}

ANC_RETURN_TYPE ExtensionFTBaseImageCalibration(uint32_t image_count) {
    return ExtensionFTCommonWithImageCount(
        ANC_CMD_EXTENSION_FT_CALIBRATION_BASE_IMAGE, image_count, NULL, 0);
}

ANC_RETURN_TYPE ExtensionFTBlackTest(AncFTBlackAlgoResult *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("black expt data result cannot be a null pointer");
        return ANC_FAIL;
    }

    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_BLACK_TEST;
    return ExtensionFTCommonWithOutput(&anc_extension_command, p_result,
                                       sizeof(AncFTBlackAlgoResult));
}

/// signal/noise
ANC_RETURN_TYPE ExtensionFTSignalNoiseTest(uint32_t image_count,
                                           AncFTSnrResult *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("snr result cannot be a null pointer");
        return ANC_FAIL;
    }
    return ExtensionFTCommonWithImageCount(
        ANC_CMD_EXTENSION_FT_SIGNAL_NOISE_TEST, image_count, p_result,
        sizeof(AncFTSnrResult));
}

/// freq/M-Factor
ANC_RETURN_TYPE ExtensionFTFreqMFactor(uint32_t image_count,
                                       AncFTChartAnalyseResult *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("chart analyse result cannot be a null pointer");
        return ANC_FAIL;
    }

    return ExtensionFTCommonWithImageCount(
        ANC_CMD_EXTENSION_FT_REQ_MFACTOR_TEST, image_count, p_result,
        sizeof(AncFTChartAnalyseResult));
}

ANC_RETURN_TYPE ExtensionSaveCalibration() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    uint8_t *p_buffer = NULL;
    uint32_t buffer_len = 0;

    ANC_CA_LOCK_SHARED_BUFFER();

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_SAVE_CALIBRATION_DATA;

    ret_val = AncGetIonSharedBuffer(&p_buffer, &buffer_len);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        goto DO_FAIL;
    }

    ret_val = ExtensionTransmitNoLockSharedBuffer(&anc_extension_command, &anc_extension_respond);
    ANC_LOGD("save calibration data. ret:%d", ret_val);

DO_FAIL:
    ANC_CA_UNLOCK_SHARED_BUFFER();
#if 0
    if (ret_val == ANC_OK) {
        uint32_t size = *((uint32_t*)p_buffer);
        if (size == 0) {
            return ret_val;
        }

        ANC_LOGD("calibration size:%d", size);

        uint8_t *p_cal_buffer = (uint8_t*)((uint32_t*)p_buffer + 1);
        const uint32_t base_image_size = 160 * 160 * 2;
        const char* base_image_file = "/mnt/vendor/persist/fingerprint/jiiov/calibration_base.bin";
        int32_t writed_size = AncWriteFullPathFile(base_image_file, p_cal_buffer, base_image_size);
        if ((uint32_t)writed_size != base_image_size) {
            ANC_LOGE("failed write base image file: %d vs %d", writed_size,base_image_size );
            return ret_val;
        }

        const char* cal_info_file = "/mnt/vendor/persist/fingerprint/jiiov/calibration_config.bin";
        const uint32_t bal_info_size = size - base_image_size;
        writed_size = AncWriteFullPathFile(cal_info_file, p_cal_buffer + base_image_size, bal_info_size);
        ANC_LOGD("write calibration info file: %d vs %d", writed_size, bal_info_size);

    }
#endif

    return ret_val;
}

ANC_RETURN_TYPE ExtensionFTReset() {
    return ExtensionFTCommon(ANC_CMD_EXTENSION_RESET);
}

ANC_RETURN_TYPE ExtensionFTDeInit() {
    return ExtensionFTCommon(ANC_CMD_EXTENSION_FT_DEINIT);
}

ANC_RETURN_TYPE ExtensionFTExpoCalibration(int type, AncFTExpoResult* p_result) {
    if (NULL == p_result) {
        ANC_LOGE("expo result cannot be a null pointer");
        return ANC_FAIL;
    }

    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    if (type == 0) {
        anc_extension_command.command = ANC_CMD_EXTENSION_AUTO_EXPO_CALIBRATION;
    } else {
        anc_extension_command.command = ANC_CMD_EXTENSION_FIXED_EXPO_CALIBRATION;
    }

    return ExtensionFTCommonWithOutput(&anc_extension_command, p_result, sizeof(AncFTExpoResult));
}

ANC_RETURN_TYPE ExtensionFTGetConfig(AncFTConfig *p_config) {
    if (NULL == p_config) {
         ANC_LOGE("get config result cannot be a null pointer");
        return ANC_FAIL;
    }

    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_FT_GET_CONFIG;

    ANC_RETURN_TYPE ret = ANC_OK;
    ret = ExtensionFTCommonWithOutput(&anc_extension_command, p_config, sizeof(*p_config));
    if (ret != ANC_OK) {
        ANC_LOGW("ta version is too old, reset to default");
        AncMemset(p_config, 0x00, sizeof(*p_config));
        p_config->white_entity.is_need_adjust = 0;
        p_config->white_entity.capture_image_count = 0;
        p_config->stripe_entity.is_need_adjust = 0;
        p_config->stripe_entity.capture_image_count = 0;
        p_config->snr_capture_image_total_count = 25;
        p_config->snr_capture_white_image_count = p_config->white_entity.capture_image_count;
        p_config->snr_capture_stripe_image_count = p_config->snr_capture_image_total_count;
    }

    return ret;
}
