#ifndef __ANC_TOKEN_H__
#define __ANC_TOKEN_H__

#include <stdint.h>
#include "anc_error.h"

ANC_RETURN_TYPE GetHmacKey();

ANC_RETURN_TYPE GetEnrollChallenge(uint64_t *p_challenge);
ANC_RETURN_TYPE AuthorizeEnroll(uint8_t *p_token, uint32_t token_size);

ANC_RETURN_TYPE SetAuthenticateChallenge(uint64_t challenge);
ANC_RETURN_TYPE GetAuthenticateResult(uint8_t *p_token, uint32_t token_size);

#endif
