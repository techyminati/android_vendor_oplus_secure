#define LOG_TAG "[ANC_TAC][MMI]"

#include "factory_test_ca.h"


#include "anc_ca.h"
#include "anc_extension.h"
#include "anc_log.h"
#include "anc_memory_wrapper.h"
#include "anc_lib.h"
#include "anc_type.h"



ANC_RETURN_TYPE ExtensionFactoryTest(AncFactoryTestCommand *p_factory_test_command,
                                    AncFactoryTestCommandRespond **p_factory_test_command_respond) {

    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionCommand anc_extension_command;
    AncExtensionCommandRespond anc_extension_respond;

    uint32_t share_buffer_size = 0;
    uint8_t *p_share_buffer = NULL;
    uint8_t *p_input_buffer = NULL;

    ANC_CA_LOCK_SHARED_BUFFER();

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        goto DO_FAIL;
    }

    if ((NULL != p_factory_test_command)
       && (share_buffer_size >= (sizeof(AncFactoryTestCommand) + p_factory_test_command->buffer_length))) {
        AncMemcpy(p_share_buffer, p_factory_test_command, sizeof(AncFactoryTestCommand));
        p_input_buffer = p_share_buffer + offsetof(AncFactoryTestCommand, p_buffer);
        AncMemcpy(p_input_buffer, p_factory_test_command->p_buffer, p_factory_test_command->buffer_length);
    } else {
        ANC_LOGE("factory test command is error:%p", p_factory_test_command);
        if (NULL != p_factory_test_command) {
            ANC_LOGE("input buffer length:%d, need buffer length:%d",
                        (sizeof(AncFactoryTestCommand) + p_factory_test_command->buffer_length),
                        share_buffer_size);
        }
        ret_val = ANC_FAIL;
        goto DO_FAIL;
    }

    AncMemset(&anc_extension_command, 0, sizeof(anc_extension_command));
    AncMemset(&anc_extension_respond, 0, sizeof(anc_extension_respond));
    anc_extension_command.command = ANC_CMD_EXTENSION_FACTORY_TEST;
    ret_val = ExtensionTransmitNoLockSharedBuffer(&anc_extension_command, &anc_extension_respond);
    if(ANC_OK != ret_val) {
        ANC_LOGE("fail to transmit extension command, return value:%d", ret_val);
    }
    if (NULL != p_factory_test_command_respond) {
        *p_factory_test_command_respond = (AncFactoryTestCommandRespond *)p_share_buffer;
    } else {
        ANC_LOGD("factory test command respond is NULL");
    }

DO_FAIL:
    ANC_CA_UNLOCK_SHARED_BUFFER();

    return ret_val;
}
