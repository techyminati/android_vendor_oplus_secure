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
#define VERSION_FP_MINOR      "4"
#define VERSION_FP_REVISION   "7"

// build version num
#define VERSION_FP VERSION_FP_MAJOR "." VERSION_FP_MINOR "." VERSION_FP_REVISION

// build date
#ifndef BUILD_DATE
#define BUILD_DATE "XXXXXXXXXXXX"
#endif

// build sec type define
#ifdef SEC_TYPE_VALUE
#define VERSION_SEC_TYPE SEC_TYPE_VALUE
#else
#define VERSION_SEC_TYPE
#endif

// build cust info
#ifdef BUILD_CUST_INFO
#define VERSION_CUST_INFO "[" BUILD_CUST_INFO "]"
#else
#define VERSION_CUST_INFO
#endif

// build dump/debug ... status
#ifdef EXT_INFO_VALUE
#define VERSION_EXT_INFO EXT_INFO_VALUE
#else
#define VERSION_EXT_INFO
#endif

// build hal version
#ifdef PLATFORM_VERSION
#define FP_HAL_VERSION (VERSION_FP "(" BUILD_DATE "-" PLATFORM_VERSION "-" VERSION_SEC_TYPE VERSION_CUST_INFO ")" VERSION_EXT_INFO)
#else
#define FP_HAL_VERSION (VERSION_FP "(" BUILD_DATE "-" VERSION_SEC_TYPE VERSION_CUST_INFO ")" VERSION_EXT_INFO)
#endif

#endif /* __SILEAD_VERSION_H__ */
