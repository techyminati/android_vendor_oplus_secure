/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/HALLog.cpp
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _HALLOG_H_
#define _HALLOG_H_

#include <android/log.h>

#ifndef FP_LOG_LEVEL
#define FP_LOG_LEVEL 4
#endif  // FP_LOG_LEVEL

#define FP_LOG_VERBOSE_LEVEL 4
#define FP_LOG_DEBUG_LEVEL 3
#define FP_LOG_INFO_LEVEL 2
#define FP_LOG_ERROR_LEVEL 1

#define LOG_V(...)                                                 \
    do {                                                           \
        if (FP_LOG_LEVEL >= FP_LOG_VERBOSE_LEVEL) {                \
            __android_log_print(ANDROID_LOG_VERBOSE, __VA_ARGS__); \
        }                                                          \
    } while (0);

#define LOG_D(...)                                               \
    do {                                                         \
        if (FP_LOG_LEVEL >= FP_LOG_DEBUG_LEVEL) {                \
            __android_log_print(ANDROID_LOG_DEBUG, __VA_ARGS__); \
        }                                                        \
    } while (0);

#define LOG_I(...)                                              \
    do {                                                        \
        if (FP_LOG_LEVEL >= FP_LOG_INFO_LEVEL) {                \
            __android_log_print(ANDROID_LOG_INFO, __VA_ARGS__); \
        }                                                       \
    } while (0);

#define LOG_E(...)                                               \
    do {                                                         \
        if (FP_LOG_LEVEL >= FP_LOG_ERROR_LEVEL) {                \
            __android_log_print(ANDROID_LOG_ERROR, __VA_ARGS__); \
        }                                                        \
    } while (0);

#ifdef SUPPORT_ATRACE
#include <cutils/trace.h>
#define ATRCE_BEGIN() atrace_begin(ATRACE_TAG_HAL, __func__)
#define ATRCE_END() atrace_end(ATRACE_TAG_HAL)
#else
#define ATRCE_BEGIN()
#define ATRCE_END()
#endif

/* FUNC_ENTER */
#define FUNC_ENTER()                            \
    do {                                        \
        LOG_V(LOG_TAG, "[%s] enter", __func__); \
        ATRCE_BEGIN(); \
    } while (0);

/* FUNC_EXIT */
#define FUNC_EXIT(err)                                                                      \
    do {                                                                                    \
        if (FP_SUCCESS == (err)) {                                                          \
            LOG_V(LOG_TAG, "[%s] exit", __func__);                                          \
        } else {                                                                            \
            LOG_E(LOG_TAG, "[%s] exit. errno=%d", __func__, err); \
        }                                                                                   \
        ATRCE_END();\
    } while (0)

/* VOID_FUNC_ENTER */
#define VOID_FUNC_ENTER()                       \
    do {                                        \
        LOG_V(LOG_TAG, "[%s] enter", __func__); \
        ATRCE_BEGIN(); \
    } while (0);

/* VOID_FUNC_EXIT */
#define VOID_FUNC_EXIT()                        \
    do {                                        \
        LOG_V(LOG_TAG, "[%s] exit", __func__); \
        ATRCE_END();\
    } while (0);

#endif  // _HALLOG_H_
