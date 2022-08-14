
#include <stdio.h>
#ifdef ANDROID
#include <android/log.h>
#else
#include <stdarg.h>
#endif
#include <string.h>
#include "plat_log.h"

#define MAX_BUFLEN 1024

#ifdef EGIS_DBG
LOG_LEVEL g_log_level = LOG_VERBOSE;
#else
LOG_LEVEL g_log_level = LOG_INFO;
#endif

void set_debug_level(LOG_LEVEL level)
{
	output_log(LOG_ERROR, "RBS", "", "", 0, "set_debug_level %d -> %d", g_log_level, level);
	g_log_level = level;
}

#ifdef ANDROID
void output_log(LOG_LEVEL level, const char *tag, const char *file_name,
		const char *func, int line, const char *format, ...)
{
	char buffer[MAX_BUFLEN];

	if (format == NULL) return;
	if (g_log_level > level) return;
#ifdef DISABLE_ALGOAPI_LOG
	if (strcmp(tag, "ETS-ALGOAPI") == 0) {
		return;
	}
#endif

	va_list vl;
	va_start(vl, format);
	vsnprintf(buffer, MAX_BUFLEN, format, vl);
	va_end(vl);

	switch (level) {
		case LOG_ERROR:
			__android_log_print(ANDROID_LOG_ERROR, tag,
					    "[%s] [%s:%d] %s \n", file_name,
					    func, line, buffer);
			break;
		case LOG_INFO:
			__android_log_print(ANDROID_LOG_INFO, tag,
					    "[%s] [%s:%d] %s \n", file_name,
					    func, line, buffer);
			break;
		case LOG_DEBUG:
			__android_log_print(ANDROID_LOG_DEBUG, tag,
					    "[%s] [%s:%d] %s \n", file_name,
					    func, line, buffer);
			break;
		case LOG_VERBOSE:
			__android_log_print(ANDROID_LOG_VERBOSE, tag,
					    "[%s] [%s:%d] %s \n", file_name,
					    func, line, buffer);
			break;
		default:
			break;
	}
}
#else
void output_log(LOG_LEVEL level, const char *tag, const char *file_name,
		const char *func, int line, const char *format, ...)
{
	char buffer[MAX_BUFLEN];

	if (format == NULL) return;
	if (g_log_level > level) return;

	va_list vl;
	va_start(vl, format);
	vsnprintf(buffer, MAX_BUFLEN, format, vl);
	va_end(vl);

	switch (level) {
		case LOG_ERROR:
			printf("ERROR !! [%s] [%s:%d] %s \n", file_name, func, line, buffer);
			break;
		default:
			printf("[%s] [%s:%d] %s \n", file_name, func, line, buffer);
			break;
	}
}

#endif