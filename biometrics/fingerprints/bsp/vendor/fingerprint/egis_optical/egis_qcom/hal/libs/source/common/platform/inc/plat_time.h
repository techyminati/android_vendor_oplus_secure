#ifndef __PLAT_TIME_H__
#define __PLAT_TIME_H__

#ifdef __cplusplus
extern "C"
{
#endif

unsigned long long plat_get_time();
unsigned long plat_get_diff_time(unsigned long long begin);
void plat_wait_time(unsigned long msecs);

void plat_sleep_time(unsigned long timeInMs);

#ifdef __cplusplus
	}
#endif
#ifdef EGIS_SPEED_DBG
#define TIME_MEASURE_START(name) \
	unsigned long long timeMeasureStart##name = plat_get_time();
#define TIME_MEASURE_STOP(name, x)                                     \
	{                                                              \
		egislog_d(x " speed_test spent %d ms",                            \
			  plat_get_diff_time(timeMeasureStart##name)); \
	}
#define TIME_MEASURE_STOP_AND_RESTART(name, x)                                     \
	{                                                              \
		egislog_d(x " speed_test spent %d ms",                            \
			  plat_get_diff_time(timeMeasureStart##name)); \
			  timeMeasureStart##name = plat_get_time(); \
	}
#else
#define TIME_MEASURE_START(name)
#define TIME_MEASURE_STOP(name, x)
#define TIME_MEASURE_STOP_AND_RESTART(name, x) 
#endif

#endif
