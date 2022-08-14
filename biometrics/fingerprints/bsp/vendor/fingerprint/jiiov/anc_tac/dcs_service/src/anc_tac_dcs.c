#define LOG_TAG "[ANC_TAC][DCS]"

#include "anc_tac_dcs.h"

#include <string.h>

#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "sensor_command.h"
#include "anc_extension.h"
#include "anc_memory_wrapper.h"

extern AncFTAlgoTimeInfo g_time_info;
extern long long g_finger_up_down_time;
extern ANC_BOOL g_is_screen_off;

static ANC_RETURN_TYPE ExtensionDCSWithOutput(AncExtensionCommand *p_req,
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

ANC_RETURN_TYPE ExtensionDCSInitDataCollect(oplus_fingerprint_init_ta_info_t *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("mean info result cannot be a null pointer");
        return ANC_FAIL;
    }
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_INIT_DATA_COLLOCT;

    return ExtensionDCSWithOutput(&anc_extension_command, p_result,
                                       sizeof(oplus_fingerprint_init_ta_info_t));
}

ANC_RETURN_TYPE ExtensionDCSAuthResultCollect(oplus_fingerprint_auth_ta_info_t *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("mean info result cannot be a null pointer");
        return ANC_FAIL;
    }
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_AUTH_RESULT_COLLOCT;

    ANC_RETURN_TYPE ret = ExtensionDCSWithOutput(&anc_extension_command, p_result,
                                       sizeof(oplus_fingerprint_auth_ta_info_t));
    for(int32_t i = 0; i < (int32_t)(sizeof(g_time_info.capture_time) / sizeof(g_time_info.capture_time[0])); i++){
        p_result->capture_time[i] = (int32_t) g_time_info.capture_time[i];
        p_result->get_feature_time[i] = (int32_t) g_time_info.extract_time[i];
#ifndef ANC_SAVE_ALGO_FILE
        p_result->auth_time[i] = (int32_t) g_time_info.verify_time[i];
#endif
        p_result->kpi_time_all[i] = (int32_t)(p_result->get_feature_time[i]+p_result->auth_time[i] + (i == 0 ? p_result->capture_time[0] : 0));
    }
    p_result->study_time = (int32_t) g_time_info.study_time;
    p_result->kpi_time_all[3] = (int32_t) g_time_info.verify_time_all;
    p_result->errtouch_flag = (int32_t) g_finger_up_down_time;
    p_result->auth_event_state1 = g_is_screen_off;
    return ret;
}

ANC_RETURN_TYPE ExtensionDCSSingleEnrollCollect(oplus_fingerprint_singleenroll_ta_info_t *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("mean info result cannot be a null pointer");
        return ANC_FAIL;
    }
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT;

    ANC_RETURN_TYPE ret = ExtensionDCSWithOutput(&anc_extension_command, p_result,
                                       sizeof(oplus_fingerprint_singleenroll_ta_info_t));

    p_result->capture_time = (int32_t) g_time_info.capture_time[0];
#ifndef ANC_SAVE_ALGO_FILE
    p_result->get_feature_time = (int32_t) g_time_info.extract_time[0];
    p_result->enroll_time = (int32_t) g_time_info.enroll_time;
#endif
    p_result->kpi_time_all = p_result->capture_time + p_result->get_feature_time + p_result->enroll_time;

    return ret;
}

ANC_RETURN_TYPE ExtensionDCSEnrollEndCollect(oplus_fingerprint_enroll_ta_info_t *p_result) {
    if (NULL == p_result) {
        ANC_LOGE("mean info result cannot be a null pointer");
        return ANC_FAIL;
    }
    AncExtensionCommand anc_extension_command;
    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    anc_extension_command.command = ANC_CMD_EXTENSION_ENROLL_END_COLLOCT;

    return ExtensionDCSWithOutput(&anc_extension_command, p_result,
                                       sizeof(oplus_fingerprint_enroll_ta_info_t));
}
