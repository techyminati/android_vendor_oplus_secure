#ifndef __PLAT_LOG_H_
#define __PLAT_LOG_H_

#include <string.h>

#if defined(TZ_MODE) && defined(__TRUSTONIC__)
#define malloc malloc_NOT_SUPPORTED
#define sprintf sprintf_NOT_SUPPORTED
#define vsnprintf vsnprintf_NOT_SUPPORTED
#define snprintf snprintf_NOT_SUPPORTED
#endif

typedef enum {
	LOG_VERBOSE = 2,
	LOG_DEBUG = 3,
	LOG_INFO = 4,
	LOG_WARN = 5,
	LOG_ERROR = 6,
	LOG_ASSERT = 7,
} LOG_LEVEL;

#ifdef _MSC_VER
#ifndef __func__
#define __func__ __FUNCTION__
#endif
#endif

#define FILE_NAME \
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define egislog(level, format, ...)                                       \
	do {                                                              \
		output_log(level, LOG_TAG, FILE_NAME, __func__, __LINE__, \
			   format, ##__VA_ARGS__);                        \
	} while (0)
#define ex_log(level, format, ...)                                      \
	do {                                                            \
		output_log(level, "RBS", FILE_NAME, __func__, __LINE__, \
			   format, ##__VA_ARGS__);                      \
	} while (0)

#if !defined(LOGD)
#define LOGD(format, ...)                                                   \
	do {                                                                \
		output_log(LOG_DEBUG, "RBS", FILE_NAME, __func__, __LINE__, \
			   format, ##__VA_ARGS__);                          \
	} while (0)
#endif

#if !defined(LOGE)
#define LOGE(format, ...)                                                   \
	do {                                                                \
		output_log(LOG_ERROR, "RBS", FILE_NAME, __func__, __LINE__, \
			   format, ##__VA_ARGS__);                          \
	} while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif
void output_log(LOG_LEVEL level, const char *tag, const char *file_name,
		const char *func, int line, const char *format, ...);

void set_debug_level(LOG_LEVEL level);
#ifdef __cplusplus
}
#endif

#ifdef _WINDOWS
#define egislog_e(format, ...)                             \
	do {                                               \
		egislog(LOG_ERROR, format, ##__VA_ARGS__); \
	} while (0)
#define egislog_d(format, ...)                             \
	do {                                               \
		egislog(LOG_DEBUG, format, ##__VA_ARGS__); \
	} while (0)
#define egislog_i(format, ...)                            \
	do {                                              \
		egislog(LOG_INFO, format, ##__VA_ARGS__); \
	} while (0)
#define egislog_v(format, ...)                               \
	do {                                                 \
		egislog(LOG_VERBOSE, format, ##__VA_ARGS__); \
	} while (0)
#else
#define egislog_e(format, args...)                  \
	do {                                        \
		egislog(LOG_ERROR, format, ##args); \
	} while (0)
#define egislog_d(format, args...)                  \
	do {                                        \
		egislog(LOG_DEBUG, format, ##args); \
	} while (0)
#define egislog_i(format, args...)                 \
	do {                                       \
		egislog(LOG_INFO, format, ##args); \
	} while (0)
#define egislog_v(format, args...)                    \
	do {                                          \
		egislog(LOG_VERBOSE, format, ##args); \
	} while (0)
#endif

#endif