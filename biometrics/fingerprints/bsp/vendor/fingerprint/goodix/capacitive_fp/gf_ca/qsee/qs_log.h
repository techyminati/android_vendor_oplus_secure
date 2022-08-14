/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: qs_log header file
 * History:
 * Version: 1.0
 */

#ifndef _QS_LOG_H_
#define _QS_LOG_H_

#include <android/log.h>

#define LOG_D(...) (__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOG_E(...) (__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#endif  // _QS_LOG_H_
