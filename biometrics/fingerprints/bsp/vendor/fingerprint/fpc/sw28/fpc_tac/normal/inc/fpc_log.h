/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_LOG_H
#define FPC_LOG_H

#ifdef NO_ANDROID

#include <stdio.h>

#define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGI(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

#else

#ifndef NDK_ROOT //USE CUTILS

#include <log/log.h>
#ifdef FPC_DEBUG_LOGGING
#define LOGD(...) ALOGD(__VA_ARGS__)
#else
#define LOGD(...) {}
#endif

#define LOGI(...) ALOGI(__VA_ARGS__)

#define LOGE(...) ALOGE(__VA_ARGS__)

#else //USE ANDROID LOG

#include <android/log.h>
#ifdef FPC_DEBUG_LOGGING
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#else
#define LOGD(...) {}
#endif

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#endif // NDK_ROOT
#endif // NO_ANDROID
#endif // FPC_LOG_H

