#define LOG_TAG "[ANC_TAC][AuxiliaryCommand]"

#include "anc_auxiliary_command.h"

#include "anc_command.h"
#include "anc_ca.h"
#include "anc_log.h"
#include "anc_memory_wrapper.h"
#include "anc_ta_version.h"


static ANC_RETURN_TYPE AuxiliaryGetTaVersion(uint32_t *p_version) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSendCommand anc_send_command;
    AncSendCommandRespond anc_send_respond;

    if (NULL == p_version) {
        ANC_LOGE("anc version is NULL");
        return ANC_FAIL;
    }

    AncMemset(&anc_send_respond, 0, sizeof(anc_send_respond));
    AncMemset(&anc_send_command, 0, sizeof(anc_send_command));
    anc_send_command.size = sizeof(AncSendCommand);
    anc_send_command.id = ANC_CMD_GET_VERSION;
    ret_val = AncCaTransmit(&anc_send_command, &anc_send_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send get version command, ret value:%d", ret_val);
        *p_version = 0;
    } else {
        ret_val = (ANC_RETURN_TYPE)anc_send_respond.status;
        *p_version = anc_send_respond.respond.data;
    }

    return ret_val;
}

ANC_RETURN_TYPE AuxiliaryCheckTaVersion() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint32_t ta_version_saved_in_hal = 0;
    uint32_t ta_version = 0;

    if (ANC_OK != (ret_val = AncGetTaVersion(&ta_version_saved_in_hal))) {
        ANC_LOGE("fail to get ta version in hal");
        goto DO_FAIL;
    }

    if (ANC_OK != (ret_val = AuxiliaryGetTaVersion(&ta_version))) {
        ANC_LOGE("fail to get ta version from ta");
        goto DO_FAIL;
    }

    ret_val = (ta_version_saved_in_hal == ta_version) ? ANC_OK : ANC_FAIL;
    ANC_LOGD("ta version in hal:%#x, ta version:%#x", ta_version_saved_in_hal, ta_version);

DO_FAIL :
    return ret_val;
}
