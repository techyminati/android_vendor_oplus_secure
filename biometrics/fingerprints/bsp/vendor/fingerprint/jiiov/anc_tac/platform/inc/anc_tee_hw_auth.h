#ifndef __ANC_TEE_HW_AUTH_QSEE_H__
#define __ANC_TEE_HW_AUTH_QSEE_H__

#include "anc_type.h"
#include "anc_error.h"


ANC_RETURN_TYPE GetHmacKeyFromKeymasterTa(uint8_t *p_hmac_key,  uint32_t *p_hmac_key_size);



#endif


