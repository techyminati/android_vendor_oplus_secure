#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

unsigned long long plat_get_time()
{
	struct timeval tv;
	unsigned long tick;
	gettimeofday(&tv, NULL);
	tick = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	return tick;
}

unsigned long plat_get_diff_time(unsigned long long begin)
{
	unsigned long long nowTime = plat_get_time();
	// TODO: consider overflow case
	return (unsigned long)(nowTime - begin);
}

void plat_wait_time(unsigned long msecs)
{
	unsigned long long times_now, times_start = plat_get_time();
	do {
		times_now = plat_get_time();
	} while (times_now - times_start < msecs);
	return;
}

void plat_sleep_time(unsigned long timeInMs) { usleep(1000 * timeInMs); }
