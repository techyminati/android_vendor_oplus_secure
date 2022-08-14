#define LOG_TAG "[ANC_COMMON][Version]"

#include "anc_ta_version.h"

#include "anc_log.h"

#define ANC_TA_VERSION_MAJOR 0x1
#ifndef FP_JIIOV_TEMPLATE_UPDATE_ENABLE
#define ANC_TA_VERSION_MINOR 0x2
#else
#define ANC_TA_VERSION_MINOR 0x3
#endif

ANC_RETURN_TYPE AncGetTaVersion(uint32_t *p_version) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (NULL == p_version) {
        ANC_LOGE("anc version is NULL");
        return ANC_FAIL;
    }


    *p_version = (ANC_TA_VERSION_MAJOR << 8) | ANC_TA_VERSION_MINOR;


    return ret_val;
}
