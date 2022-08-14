/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: Basic posix function used by dump
 * History: None
 * Version: 1.0
 */
#ifndef _DUMP_DECODER_POSIX_H_
#define _DUMP_DECODER_POSIX_H_

/**
* support OS:
*    Android Linux Windows
*/

#include <sys/types.h>
#include <stdint.h>

#ifdef __android__
#include <android/log.h>
#else  // __android__
#include <stdio.h>
#endif  // __android__

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#define LOG_TAG "[gf_dump_decoder]"

#ifdef __android__
#define LOG_D(...) (__android_log_print(ANDROID_LOG_DEBUG, __VA_ARGS__))
#else  // __android__
#define LOG_D(TAG, ...) \
    do  \
    { \
        printf(__VA_ARGS__); \
        printf("\n"); \
    }  \
    while (0)
#endif  // __android__

/**
* no support for Windows now
*/
int32_t mkdirs(const char* path, mode_t mode);

/**
* for:create multilevel directory which base on the root path
* support platform: Windows Linux Android
*
* path: relative path, data/http or ./data/
* mode: for Linux and Android, 0755
* root_path: absolute or relative path
*
*/
int32_t mkdirs_relative(const char* path, mode_t mode, const char* root_path);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _DUMP_DECODER_POSIX_H_
