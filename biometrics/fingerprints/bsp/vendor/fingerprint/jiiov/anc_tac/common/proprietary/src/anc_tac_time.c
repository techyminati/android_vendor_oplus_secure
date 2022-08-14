#include "anc_tac_time.h"

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include "anc_error.h"
#include "anc_lib.h"

#define ONE_HOUR_MS 60*60*1000
long long AncGetCurrentTimeMs()
{
    struct timeval tv;
    long long ms;
    gettimeofday(&tv, NULL);
    ms = ((long long)tv.tv_sec * 1000) + ((long long)tv.tv_usec / 1000);
    return ms;
}

long long AncGetElapsedRealTimeMs()
{
    struct timespec ts;
    long long ms;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    ms = ((long long)ts.tv_sec * 1000) + ((long long)ts.tv_nsec / 1000000);
    return ms;
}

long long AncGetDiffTime(long long start)
{
    long long current_time = AncGetElapsedRealTimeMs();
    return (current_time - start);
}

long long AncGetCurrentCATimeMs() {
    struct timeval tv;
    struct tm *p_tm;
    long long time_ms;

    gettimeofday(&tv,NULL);
    p_tm = localtime(&(tv.tv_sec));

    time_ms = ((long long)p_tm->tm_hour * ONE_HOUR_MS) + (p_tm->tm_min * 60 * 1000L) + (p_tm->tm_sec * 1000L) + (tv.tv_usec / 1000);

    return time_ms;
}

ANC_RETURN_TYPE AncStrfTime(char *p_dst, size_t dst_len) {
    struct timeval tv;
    struct tm *p_tm;
    char ms[40];
    time_t timet = 0;

    if ( dst_len < 50 || p_dst == NULL) {
        return ANC_FAIL;
    }
    gettimeofday(&tv,NULL);
    timet=tv.tv_sec;
    p_tm = localtime(&timet);
    strftime(p_dst, dst_len, "%m%d%H%M%S" ,p_tm);
    AncItoa((int)tv.tv_usec/1000, ms, 10);
    char ms_buf[5] = "";
    sprintf(ms_buf, "%03d",(int)tv.tv_usec/1000);
    strcat(p_dst, ms_buf);

    return ANC_OK;
}
