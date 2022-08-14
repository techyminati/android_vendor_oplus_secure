#ifndef __ANC_TAC_DCS_H__
#define __ANC_TAC_DCS_H__

#include <string.h>
#include "anc_data_collect.h"
#include "dcs_type.h"
#include "anc_error.h"

ANC_RETURN_TYPE ExtensionDCSInitDataCollect(oplus_fingerprint_init_ta_info_t *p_result);

ANC_RETURN_TYPE ExtensionDCSAuthResultCollect(oplus_fingerprint_auth_ta_info_t *p_result);

ANC_RETURN_TYPE ExtensionDCSSingleEnrollCollect(oplus_fingerprint_singleenroll_ta_info_t *p_result);

ANC_RETURN_TYPE ExtensionDCSEnrollEndCollect(oplus_fingerprint_enroll_ta_info_t *p_result);

#endif
