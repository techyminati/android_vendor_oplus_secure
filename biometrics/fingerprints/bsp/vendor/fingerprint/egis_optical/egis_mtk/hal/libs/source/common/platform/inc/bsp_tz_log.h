#ifndef __BSP_TZ_LOG_H_
#define __BSP_TZ_LOG_H_

typedef enum {
    BSP_TZ_LOG_LEVEL_NONE = 0,
    BSP_TZ_LOG_LEVEL_ERROR,
    BSP_TZ_LOG_LEVEL_INFO,
    BSP_TZ_LOG_LEVEL_DEBUG
} BSP_TZ_LOG_LEVEL;
extern void bsp_tzlog_level(BSP_TZ_LOG_LEVEL level);
extern void bsp_tzlog_printf(int level, const char* format, ...);

#endif
