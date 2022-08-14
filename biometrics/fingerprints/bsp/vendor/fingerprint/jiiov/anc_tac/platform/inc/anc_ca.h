#ifndef __ANC_CA_H__
#define __ANC_CA_H__

#include "anc_command.h"
#include "anc_error.h"


ANC_RETURN_TYPE InitAncCa();
ANC_RETURN_TYPE DeinitAncCa();
ANC_RETURN_TYPE AncCaTransmit(AncSendCommand *p_send_cmd, AncSendCommandRespond *p_send_rsp_cmd);

#define ANC_CA_LOCK_SHARED_BUFFER() AncCaLockSharedBuffer((uint8_t *)__func__)
#define ANC_CA_UNLOCK_SHARED_BUFFER() AncCaUnlockSharedBuffer((uint8_t *)__func__)
ANC_RETURN_TYPE AncCaLockSharedBuffer(uint8_t *p_function_name);
ANC_RETURN_TYPE AncCaUnlockSharedBuffer(uint8_t *p_function_name);
ANC_RETURN_TYPE AncGetIonSharedBuffer(uint8_t **p_share_buffer, uint32_t *p_share_buffer_size);
ANC_RETURN_TYPE AncCaTransmitModified(AncSendCommand *p_send_cmd, AncSendCommandRespond *p_send_rsp_cmd);
ANC_RETURN_TYPE AncTestSharedBuffer();

#endif
