/******************************************************************************
 * @file   logmsg.h
 * @brief  Contains log message header file.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered 
 * into with Silead Inc. This software may be subject to export or import 
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * David Wang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __LOGMSG_H__
#define __LOGMSG_H__

#ifdef SIL_DEBUG_ALL_LOG
    #ifdef SIL_DEBUG_LOG_DYNAMIC // debug dynamic
        #define LOG_DBG_DYNAMIC 1
        #ifdef HOST_OS_LINUX_TOOL
            #undef LOG_DBG_VERBOSE
        #endif
        #ifndef LOG_DBG_VERBOSE
            #define LOG_DBG_VERBOSE 1
        #endif
    #else // debug all
        #define LOG_DBG_DYNAMIC 0
        #undef LOG_DBG_VERBOSE
        #define LOG_DBG_VERBOSE 1
    #endif
    #define LOG_DBG_DEBUG 1
#else // debug normal
    #define LOG_DBG_DYNAMIC 0
    #undef LOG_DBG_VERBOSE
    #define LOG_DBG_VERBOSE 0
    #define LOG_DBG_DEBUG 0
#endif

#ifndef LOG_DBG
#define LOG_DBG 1
#endif

#undef LOG_TAG
#define LOG_TAG "fingerprint"
//#define FILE_TAG ""  // should be defined in each file

//==================================================================================================
#ifndef LOG_DBG_DYNAMIC
#define LOG_DBG_DYNAMIC 0
#endif

#ifndef LOG_DBG
#define LOG_DBG 0
#endif

#ifndef LOG_DBG_VERBOSE
#define LOG_DBG_VERBOSE 0
#endif

#if LOG_DBG_DYNAMIC
#undef LOG_DBG
#define LOG_DBG 1
#endif

#if LOG_DBG
#undef LOG_NDEBUG
#define LOG_NDEBUG 0
#endif

#ifndef FILE_TAG
#define FILE_TAG "fp"
#endif

#include <sys/types.h>
#include <stdint.h>

#ifndef __predict_false
#define __predict_false(exp) __builtin_expect((exp) != 0, 0)
#endif

#ifndef LOG_DUMP_IF
#define LOG_DUMP_IF(cond, ...) ( (__predict_false(cond)) ? ((void)silfp_log_dump(__VA_ARGS__)) : (void)0 )
#endif

#if LOG_DBG
    #if LOG_DBG_DYNAMIC
        #if LOG_DBG_VERBOSE
            #define LOG_MSG_VERBOSE(fmt, ...) LOG_DUMP_IF(silfp_log_is_loggable(0), "[%s]V[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
        #else
            #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
        #endif

        #define LOG_MSG_DEBUG(fmt, ...)   LOG_DUMP_IF(silfp_log_is_loggable(1), "[%s]D[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
        #define LOG_MSG_INFO(fmt, ...)    LOG_DUMP_IF(silfp_log_is_loggable(2), "[%s]I[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

        #define LOG_FUNC_ENTRY()          LOG_DUMP_IF(silfp_log_is_loggable(1), "[%s]~~~~~~~~~~~ +%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
        #define LOG_FUNC_EXIT()           LOG_DUMP_IF(silfp_log_is_loggable(1), "[%s]~~~~~~~~~~~ -%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
    #else
        #if LOG_DBG_VERBOSE
            #define LOG_MSG_VERBOSE(fmt, ...) silfp_log_dump("[%s]V[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
        #else
            #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
        #endif
        #if LOG_DBG_DEBUG
            #define LOG_MSG_DEBUG(fmt, ...)   silfp_log_dump("[%s]D[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
            #define LOG_FUNC_ENTRY()          silfp_log_dump("[%s]~~~~~~~~~~~ +%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
            #define LOG_FUNC_EXIT()           silfp_log_dump("[%s]~~~~~~~~~~~ -%s ~~~~~~~~~~~", FILE_TAG, __FUNCTION__ )
        #else
            #define LOG_MSG_DEBUG(fmt, ...)   ((void)0)
            #define LOG_FUNC_ENTRY()          ((void)0)
            #define LOG_FUNC_EXIT()           ((void)0)
        #endif
        #define LOG_MSG_INFO(fmt, ...)    silfp_log_dump("[%s]I[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #endif
#else
    #define LOG_MSG_VERBOSE(fmt, ...) ((void)0)
    #define LOG_MSG_DEBUG(fmt, ...)   ((void)0)
    #define LOG_MSG_INFO(fmt, ...)    ((void)0)

    #define LOG_FUNC_ENTRY()          ((void)0)
    #define LOG_FUNC_EXIT()           ((void)0)
#endif

#define LOG_MSG_WARNING(fmt, ...) silfp_log_dump("[%s]W[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_MSG_ERROR(fmt, ...)   silfp_log_dump("[%s]E[%s:%d] " fmt, FILE_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

void silfp_log_set_level(int32_t level);
int32_t silfp_log_is_loggable(int32_t level);

int32_t silfp_log_dump_init(void);
void silfp_log_dump_deinit(void);
void silfp_log_dump_set_path(const void *path, uint32_t len);

void silfp_log_dump(const char *fmt, ...);
void silfp_log_dump_cfg(uint32_t chipid, uint32_t subid, uint32_t vid);

//==================================================================================================

#endif // __LOGMSG_H__
