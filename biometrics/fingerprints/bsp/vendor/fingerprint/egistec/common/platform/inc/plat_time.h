#ifndef __PLAT_TIME_H__
#define __PLAT_TIME_H__

unsigned long long plat_get_time();
unsigned long plat_get_diff_time(unsigned long long begin);
void plat_wait_time(unsigned long msecs);

void plat_sleep_time(unsigned long timeInMs);

#define TIME_MEASURE_START(name) \
	unsigned long long timeMeasureStart##name = plat_get_time();
#define TIME_MEASURE_STOP(name, x)                                     \
	{                                                              \
		egislog_d(" speed_test spent %d ms",                            \
			  plat_get_diff_time(timeMeasureStart##name)); \
	}

#endif
