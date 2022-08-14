#define LOG_TAG "[ANC_TAC][Token]"

#include "anc_token.h"

#include <string.h>

#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "anc_tee_hw_auth.h"
#include "anc_memory_wrapper.h"
#include "anc_tac_time.h"
#include "anc_log_string.h"


static ANC_RETURN_TYPE TokenTransmit(AncAuthTokenCommand *p_token, AncAuthTokenCommandRespond *p_token_respond) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSendCommand anc_send_command;
    AncSendCommandRespond anc_send_respond;

    AncMemset(&anc_send_respond, 0, sizeof(anc_send_respond));
    AncMemset(&anc_send_command, 0, sizeof(anc_send_command));
    anc_send_command.size = sizeof(AncSendCommand);
    anc_send_command.id = ANC_CMD_AUTH_TOKEN;
    AncMemcpy(&(anc_send_command.data.token), p_token, sizeof(AncAuthTokenCommand));

    ANC_LOGD("TRANSMIT >>>> command id = %d (%s)", p_token->command, AncConvertCommandIdToString(ANC_CMD_AUTH_TOKEN, p_token->command));
    long long time_start = AncGetElapsedRealTimeMs();

    ret_val = AncCaTransmit(&anc_send_command, &anc_send_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send token command, ret value:%d, command id:%d",
                  ret_val, anc_send_command.data.token.command);
    } else {
        ret_val = (ANC_RETURN_TYPE)anc_send_respond.status;
        AncMemcpy(p_token_respond, &(anc_send_respond.respond.token), sizeof(AncAuthTokenCommandRespond));
    }

    long long time_end = AncGetElapsedRealTimeMs();
    ANC_LOGD("TRANSMIT <<<< command id = %d (%s), spent time = %lld ms, ret_val = %d (%s)", p_token->command,
        AncConvertCommandIdToString(ANC_CMD_AUTH_TOKEN, p_token->command), (time_end - time_start),
        ret_val, AncConvertReturnTypeToString(ret_val));

    return ret_val;
}

ANC_RETURN_TYPE GetHmacKey() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAuthTokenCommand anc_token_command;
    AncAuthTokenCommandRespond anc_token_respond;
    uint32_t array_size = 0;

    AncMemset(&anc_token_command, 0, sizeof(anc_token_command));
    AncMemset(&anc_token_respond, 0, sizeof(anc_token_respond));
    anc_token_command.command = ANC_CMD_TOKEN_SET_HMAC_KEY;
    ret_val = GetHmacKeyFromKeymasterTa(anc_token_command.array, &array_size);
    if (ANC_OK == ret_val) {
        anc_token_command.size = array_size;
        ret_val = TokenTransmit(&anc_token_command, &anc_token_respond);
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to set hmac key, ret value:%d", ret_val);
        }
    } else {
        ANC_LOGE("fail to get hmac key, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE GetEnrollChallenge(uint64_t *p_challenge) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAuthTokenCommand anc_token_command;
    AncAuthTokenCommandRespond anc_token_respond;

    AncMemset(&anc_token_command, 0, sizeof(anc_token_command));
    AncMemset(&anc_token_respond, 0, sizeof(anc_token_respond));
    anc_token_command.command = ANC_CMD_TOKEN_GET_ENROLL_CHALLENGE;
    ret_val = TokenTransmit(&anc_token_command, &anc_token_respond);
    if (ANC_OK == ret_val) {
        *p_challenge = anc_token_respond.challenge;
    } else {
        ANC_LOGE("fail to get enroll Challenge, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE AuthorizeEnroll(uint8_t *p_token, uint32_t token_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAuthTokenCommand anc_token_command;
    AncAuthTokenCommandRespond anc_token_respond;

    AncMemset(&anc_token_command, 0, sizeof(anc_token_command));
    AncMemset(&anc_token_respond, 0, sizeof(anc_token_respond));
    anc_token_command.command = ANC_CMD_TOKEN_AUTHORIZE_ENROLL;
    AncMemcpy(anc_token_command.array, p_token, token_size);
    anc_token_command.size = token_size;
    ret_val = TokenTransmit(&anc_token_command, &anc_token_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to authorize enroll token, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE SetAuthenticateChallenge(uint64_t challenge) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAuthTokenCommand anc_token_command;
    AncAuthTokenCommandRespond anc_token_respond;

    AncMemset(&anc_token_command, 0, sizeof(anc_token_command));
    AncMemset(&anc_token_respond, 0, sizeof(anc_token_respond));
    anc_token_command.command = ANC_CMD_TOKEN_SET_AUTHENTICATE_CHALLENGE;
    anc_token_command.challenge = challenge;
    ret_val = TokenTransmit(&anc_token_command, &anc_token_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to set enroll Challenge, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE GetAuthenticateResult(uint8_t *p_token, uint32_t token_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAuthTokenCommand anc_token_command;
    AncAuthTokenCommandRespond anc_token_respond;

    if ((NULL == p_token) || (token_size == 0)) {
        ANC_LOGE("parameters are error, token:%p, received token size:%d", p_token, token_size);
        return ANC_FAIL;
    }

    AncMemset(&anc_token_command, 0, sizeof(anc_token_command));
    AncMemset(&anc_token_respond, 0, sizeof(anc_token_respond));
    anc_token_command.command = ANC_CMD_TOKEN_GET_AUTHENTICATE_RESULT;
    ret_val = TokenTransmit(&anc_token_command, &anc_token_respond);
    if (ANC_OK == ret_val) {
        if (token_size == anc_token_respond.size) {
            AncMemcpy(p_token, anc_token_respond.array, token_size);
        } else {
            ANC_LOGE("token_size is error, token size:%d, received token size:%d",
                     token_size, anc_token_respond.size);
        }
    } else {
        ANC_LOGE("fail to get authenticate result, ret value:%d", ret_val);
    }

    return ret_val;
}
