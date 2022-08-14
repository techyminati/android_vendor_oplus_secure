/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef _GF_HAL_LOG_H_
#define _GF_HAL_LOG_H_

#include<android/log.h>
#include "gf_error.h"

#ifdef __cpluscplus
extern "C"
{
#endif  // __cpluscplus

#ifndef GF_LOG_LEVEL
#define GF_LOG_LEVEL 2
#endif  // GF_LOG_LEVEL

#define GF_LOG_VERBOSE_LEVEL   4
#define GF_LOG_DEBUG_LEVEL   3
#define GF_LOG_INFO_LEVEL    2
#define GF_LOG_ERROR_LEVEL   1

#define LOG_V(...) \
    do { \
        if (GF_LOG_LEVEL >= GF_LOG_VERBOSE_LEVEL) { \
            __android_log_print(ANDROID_LOG_VERBOSE, __VA_ARGS__);\
        } \
    } while (0);

#define LOG_D(...) \
    do { \
        if ( GF_LOG_LEVEL >= GF_LOG_DEBUG_LEVEL ) { \
            __android_log_print(ANDROID_LOG_DEBUG, __VA_ARGS__); \
        } \
    } while (0);

#define LOG_I(...) \
    do { \
        if ( GF_LOG_LEVEL >= GF_LOG_INFO_LEVEL ) { \
            __android_log_print(ANDROID_LOG_INFO, __VA_ARGS__);  \
        } \
    } while (0);

#define LOG_E(...) \
    do { \
        if ( GF_LOG_LEVEL >= GF_LOG_ERROR_LEVEL ) { \
            __android_log_print(ANDROID_LOG_ERROR, __VA_ARGS__); \
        } \
    } while (0);

/* FUNC_ENTER */
#define FUNC_ENTER() LOG_V(LOG_TAG, "[%s] enter", __func__)

/* FUNC_EXIT */
#define FUNC_EXIT(err)                               \
    do {                                             \
        if (GF_SUCCESS == (err)) {                     \
            LOG_V(LOG_TAG, "[%s] exit", __func__);   \
        } else {                                     \
            LOG_E(LOG_TAG, "[%s] exit. err=%s, errno=%d", __func__, gf_strerror(err), err); \
        }                                            \
    } while (0)

/* VOID_FUNC_ENTER */
#define VOID_FUNC_ENTER() LOG_V(LOG_TAG, "[%s] enter", __func__)

/* VOID_FUNC_EXIT */
#define VOID_FUNC_EXIT()  LOG_V(LOG_TAG, "[%s] exit", __func__)

#ifdef __cpluscplus
}
#endif  // __cpluscplus

#endif  // _GF_HAL_LOG_H_
