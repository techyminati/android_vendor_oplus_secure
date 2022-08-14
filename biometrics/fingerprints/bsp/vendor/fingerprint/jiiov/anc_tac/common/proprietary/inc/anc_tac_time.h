#ifndef __ANC_TAC_TIME_H__
#define __ANC_TAC_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "anc_log.h"
#include "anc_error.h"

long long AncGetCurrentTimeMs();
long long AncGetElapsedRealTimeMs();
long long AncGetDiffTime(long long start);
long long AncGetCurrentCATimeMs();

#ifdef ANC_DEBUG
#define ANC_LOGDD(fmt...) ANC_LOGD(fmt)
#else
#define ANC_LOGDD(fmt...)
#endif

#define ANC_TIME_MEASURE_START(name)                    \
    long long AncTimeStart##name = AncGetElapsedRealTimeMs();


#define ANC_TIME_MEASURE_END(name, flag)                \
    {                                                   \
        ANC_LOGDD("%s spent %lld ms",                   \
            flag, AncGetDiffTime(AncTimeStart##name));  \
    }

#define ANC_GET_TIME_MEASURE_END(name, flag, p_time)        \
    {                                                       \
        long long time = AncGetDiffTime(AncTimeStart##name);\
        ANC_LOGDD("%s spent %lld ms",                       \
            flag, time);                                    \
        *p_time = time;                                     \
    }

ANC_RETURN_TYPE AncStrfTime(char *p_dst, size_t dst_len);

#ifdef __cplusplus
}
#endif

#endif
