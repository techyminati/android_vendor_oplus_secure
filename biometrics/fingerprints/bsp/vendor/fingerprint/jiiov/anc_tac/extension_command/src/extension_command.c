#define LOG_TAG "[ANC_TAC][ExtensionCommand]"

#include "extension_command.h"

#include "anc_ca.h"
#include "anc_log.h"
#include "anc_command.h"
#include "anc_tac_time.h"
#include "anc_ca_image.h"
#include "anc_memory_wrapper.h"
#include "anc_lib.h"
#include "anc_log_string.h"


ANC_RETURN_TYPE ExtensionTransmitCommon(AncExtensionCommand *p_extension, AncExtensionCommandRespond *p_extension_respond,
                                        ANC_BOOL is_used_share_buffer) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSendCommand anc_send_command;
    AncSendCommandRespond anc_send_respond;

    AncMemset(&anc_send_respond, 0, sizeof(anc_send_respond));
    AncMemset(&anc_send_command, 0, sizeof(anc_send_command));
    anc_send_command.size = sizeof(AncSendCommand);
    anc_send_command.id = ANC_CMD_EXTENSION;
    AncMemcpy(&(anc_send_command.data.extension), p_extension, sizeof(AncExtensionCommand));

    ANC_LOGD("TRANSMIT >>>> command id = %d (%s)", p_extension->command, AncConvertCommandIdToString(ANC_CMD_EXTENSION, p_extension->command));
    long long time_start = AncGetElapsedRealTimeMs();

    if (is_used_share_buffer) {
        ret_val = AncCaTransmitModified(&anc_send_command, &anc_send_respond);
    } else {
        ret_val = AncCaTransmit(&anc_send_command, &anc_send_respond);
    }
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send extension command, ret value:%d", ret_val);
    } else {
        ret_val = (ANC_RETURN_TYPE)anc_send_respond.status;
        AncMemcpy(p_extension_respond, &(anc_send_respond.respond.extension), sizeof(AncExtensionCommandRespond));
    }

    long long time_end = AncGetElapsedRealTimeMs();
    ANC_LOGD("TRANSMIT <<<< command id = %d (%s), spent time = %lld ms, ret_val = %d (%s)", p_extension->command,
        AncConvertCommandIdToString(ANC_CMD_EXTENSION, p_extension->command), (time_end - time_start),
        ret_val, AncConvertReturnTypeToString(ret_val));

    return ret_val;
}

ANC_RETURN_TYPE ExtensionTransmitNoLockSharedBuffer(AncExtensionCommand *p_extension, AncExtensionCommandRespond *p_extension_respond) {
    return ExtensionTransmitCommon(p_extension, p_extension_respond, ANC_TRUE);
}

ANC_RETURN_TYPE ExtensionTransmit(AncExtensionCommand *p_extension, AncExtensionCommandRespond *p_extension_respond) {
    return ExtensionTransmitCommon(p_extension, p_extension_respond, ANC_FALSE);
}

#ifdef ANC_GET_IMAGE_FROM_TA
ANC_RETURN_TYPE ExtensionGetImage(uint8_t **pp_buffer, uint32_t *p_buffer_length, int need_save) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;
    uint32_t share_buffer_size = 0;
    uint8_t *p_share_buffer = NULL;

    if((NULL == pp_buffer) || (NULL == p_buffer_length)) {
        ANC_LOGE("get image addr error");
        return ANC_FAIL;
    }

    ANC_CA_LOCK_SHARED_BUFFER();

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        goto DO_FAIL;
    }

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_GET_IMAGE;

    ret_val = ExtensionTransmitNoLockSharedBuffer(&anc_extension_command, &anc_extension_respond);
    if(ANC_OK == ret_val) {
        ExtensionSaveImage(p_share_buffer, share_buffer_size, pp_buffer, p_buffer_length, need_save);
    } else {
        ANC_LOGE("fail to transmit extension command");
    }

DO_FAIL:
    ANC_CA_UNLOCK_SHARED_BUFFER();

    return ret_val;
}
#endif

ANC_RETURN_TYPE ExtensionSetCurrentCATime() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_SET_TIME;
    anc_extension_command.ca_time_ms = AncGetCurrentCATimeMs();

    ret_val = ExtensionTransmit(&anc_extension_command, &anc_extension_respond);

    return ret_val;
}

ANC_RETURN_TYPE ExtensionGetAuthToken(uint8_t *p_token, uint32_t token_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_GET_AUTH_TOKEN;
    AncMemcpy(anc_extension_command.array, p_token, token_size);
    anc_extension_command.array_size = token_size;
    ret_val = ExtensionTransmit(&anc_extension_command, &anc_extension_respond);
    if (ANC_OK == ret_val) {
        if (token_size == anc_extension_respond.array_size) {
            AncMemcpy(p_token, anc_extension_respond.array, token_size);
        } else {
            ANC_LOGE("token_size is error, token size:%d, received token size:%d",
                     token_size, anc_extension_respond.array_size);
        }
    } else {
        ANC_LOGE("fail to get authenticate result, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE ExtensionLoadCalibration() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_LOAD_CALIBRAION_DATA;

#ifndef LOAD_BASE_IMAGE_FROM_RAW_FILE
    anc_extension_command.load_base_image_flag = ANC_FALSE;
    ret_val = ExtensionTransmit(&anc_extension_command, &anc_extension_respond);

#else
    ANC_CA_LOCK_SHARED_BUFFER();
    anc_extension_command.load_base_image_flag = ANC_TRUE;

#define BASE_IMAGE_BASE_IMAGE ANC_DATA_ROOT "base/calibration.bin"

    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;
    size_t read_size = 0;

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        return ret_val;
    }

    ret_val = AncReadFile(BASE_IMAGE_BASE_IMAGE, p_share_buffer, (size_t)share_buffer_size, &read_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to read base image %s: return value = %d\n", BASE_IMAGE_BASE_IMAGE, ret_val);
        ANC_CA_UNLOCK_SHARED_BUFFER();
        return ret_val;
    }

    ret_val = ExtensionTransmitNoLockSharedBuffer(&anc_extension_command, &anc_extension_respond);

    ANC_CA_UNLOCK_SHARED_BUFFER();
#endif

    return ret_val;
}

#ifdef FP_JIIOV_TEMPLATE_UPDATE_ENABLE
ANC_RETURN_TYPE ExtensionUpdateFile(AncExtensionUpdateFileInfo *p_update_info) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;
    uint32_t share_buffer_size = 0;
    uint8_t *p_share_buffer = NULL;

    if (NULL == p_update_info) {
        ANC_LOGE("extension update file, invalid param");
        return ANC_FAIL;
    }

    ANC_CA_LOCK_SHARED_BUFFER();

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        return ANC_FAIL;
    }
    size_t update_info_size = sizeof(*p_update_info);
    if(share_buffer_size < update_info_size) {
        ANC_LOGE("shared buffer size is %d insufficient, store path len %zu", share_buffer_size, update_info_size);

        ANC_CA_UNLOCK_SHARED_BUFFER();
        return ANC_FAIL_MEMORY;
    }
    AncMemcpy(p_share_buffer, p_update_info, update_info_size);

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_UPDATE_FILE;

    ret_val = ExtensionTransmitNoLockSharedBuffer(&anc_extension_command, &anc_extension_respond);
    if(ANC_OK != ret_val) {
        ANC_LOGE("extension, fail to update file");
    }

    ANC_CA_UNLOCK_SHARED_BUFFER();
    return ret_val;
}
#endif
