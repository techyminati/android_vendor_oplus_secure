/******************************************************************************
 * @file   silead_version.h
 * @brief  Contains CA version definitions.
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
 * Jack Zhang  2018/4/2    0.1.0      Init version
 * Tiven Xie   2018/5/10   0.1.1      Update CA version to 4.15.1
 * Tiven Xie   2018/5/17   0.1.2      Update CA version to 4.15.2
 *
 *****************************************************************************/

#ifndef __SILEAD_VERSION_H__
#define __SILEAD_VERSION_H__

#define VERSION_FP_MAJOR      "5"
#define VERSION_FP_MINOR      "0"
#define VERSION_FP_REVISION   "18"

// build version num
#define VERSION_FP VERSION_FP_MAJOR "." VERSION_FP_MINOR "." VERSION_FP_REVISION

// build date
#ifndef BUILD_DATE
#define BUILD_DATE "XXXXXXXXXXXX"
#endif

// build sec type define
// n:non security t:trustonic q:qsee g:global platform s:trusty
#define SECURITY_TYPE_NONE_VALUE "n"
#define SECURITY_TYPE_TEE_VALUE "t"
#define SECURITY_TYPE_QSEE_VALUE "q"
#define SECURITY_TYPE_GP_VALUE "g"
#define SECURITY_TYPE_TRUSTY_VALUE "s"
#define SECURITY_TYPE_OPTEE_VALUE "o"

#ifndef SECURITY_TYPE_NOSEC
#undef SECURITY_TYPE_NONE_VALUE
#define SECURITY_TYPE_NONE_VALUE
#endif
#ifndef SECURITY_TYPE_TEE
#undef SECURITY_TYPE_TEE_VALUE
#define SECURITY_TYPE_TEE_VALUE
#endif
#ifndef SECURITY_TYPE_QSEE
#undef SECURITY_TYPE_QSEE_VALUE
#define SECURITY_TYPE_QSEE_VALUE
#endif
#ifndef SECURITY_TYPE_GP
#undef SECURITY_TYPE_GP_VALUE
#define SECURITY_TYPE_GP_VALUE
#endif
#ifndef SECURITY_TYPE_TRUSTY
#undef SECURITY_TYPE_TRUSTY_VALUE
#define SECURITY_TYPE_TRUSTY_VALUE
#endif
#ifndef SECURITY_TYPE_OPTEE
#undef SECURITY_TYPE_OPTEE_VALUE
#define SECURITY_TYPE_OPTEE_VALUE
#endif

#define VERSION_SEC_TYPE SECURITY_TYPE_NONE_VALUE SECURITY_TYPE_TEE_VALUE SECURITY_TYPE_QSEE_VALUE SECURITY_TYPE_GP_VALUE SECURITY_TYPE_TRUSTY_VALUE SECURITY_TYPE_OPTEE_VALUE

// build cust info
#ifdef BUILD_CUST_INFO
#define VERSION_CUST_INFO "[" BUILD_CUST_INFO "]"
#else
#define VERSION_CUST_INFO
#endif

// build dump/debug ... status
#define INFO_DUMP_IMG "i"
#define INFO_DUMP_IMG_DYNAMIC "d"
#define INFO_DEBUG_ALL_LOG "l"
#define INFO_DEBUG_ALL_LOG_DYNAMIC "d"
#define INFO_EXT_SKT_SERVER "s"
#define INFO_EXT_CAPTURE "c"

#ifndef SIL_DUMP_IMAGE_DYNAMIC
#undef INFO_DUMP_IMG_DYNAMIC
#define INFO_DUMP_IMG_DYNAMIC
#endif
#ifndef SIL_DUMP_IMAGE
#undef INFO_DUMP_IMG
#define INFO_DUMP_IMG
#undef INFO_DUMP_IMG_DYNAMIC
#define INFO_DUMP_IMG_DYNAMIC
#endif

#ifndef SIL_DEBUG_LOG_DYNAMIC
#undef INFO_DEBUG_ALL_LOG_DYNAMIC
#define INFO_DEBUG_ALL_LOG_DYNAMIC
#endif
#ifndef SIL_DEBUG_ALL_LOG
#undef INFO_DEBUG_ALL_LOG
#define INFO_DEBUG_ALL_LOG
#undef INFO_DEBUG_ALL_LOG_DYNAMIC
#define INFO_DEBUG_ALL_LOG_DYNAMIC
#endif

#ifndef SIL_FP_EXT_CAPTURE_ENABLE
#undef INFO_EXT_CAPTURE
#define INFO_EXT_CAPTURE
#endif
#ifndef SIL_FP_EXT_SKT_SERVER_ENABLE
#undef INFO_EXT_SKT_SERVER
#define INFO_EXT_SKT_SERVER
#undef INFO_EXT_CAPTURE
#define INFO_EXT_CAPTURE
#endif

#define VERSION_EXT_INFO INFO_DUMP_IMG INFO_DUMP_IMG_DYNAMIC INFO_DEBUG_ALL_LOG INFO_DEBUG_ALL_LOG_DYNAMIC INFO_EXT_SKT_SERVER INFO_EXT_CAPTURE

// build hal version
#ifdef PLATFORM_VERSION
#define FP_HAL_VERSION (VERSION_FP "(" BUILD_DATE "-" PLATFORM_VERSION "-" VERSION_SEC_TYPE VERSION_CUST_INFO ")" VERSION_EXT_INFO)
#else
#define FP_HAL_VERSION (VERSION_FP "(" BUILD_DATE "-" VERSION_SEC_TYPE VERSION_CUST_INFO ")" VERSION_EXT_INFO)
#endif

#endif /* __SILEAD_VERSION_H__ */
