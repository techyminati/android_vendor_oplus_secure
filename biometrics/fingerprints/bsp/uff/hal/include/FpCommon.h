/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/FpCommon.h
 **
 ** Description:
 **      common define for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef __FP_COMMON_H__
#define __FP_COMMON_H__

#include <stdint.h>
#include <stddef.h>
#include "HalLog.h"

#ifdef __cplusplus
extern "C"
{
#endif  // #ifdef __cplusplus


#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define FP_ERROR_BREAK(err)  { if (FP_SUCCESS != (err)) { break; } }

//TBD:for debug
#define CHECK_RESULT_SUCCESS_AND_BREAK(result) \
    do { \
        if (result != FP_SUCCESS) { \
            LOG_E(LOG_TAG, "%s:%d: result:%d", __func__, __LINE__, result); \
            goto fp_out; \
        } \
    } while (0)

#define CHECK_RESULT_SUCCESS(result) \
    do { \
        if (result != FP_SUCCESS) { \
            LOG_E(LOG_TAG, "%s:%d: result:%d", __func__, __LINE__, result); \
            goto fp_out; \
        } \
    } while (0)

#define CHECK_RESULT_NULLPTR(result) \
    do { \
        if (result == nullptr) { \
            LOG_E(LOG_TAG, "%s:%d: result is null", __func__, __LINE__); \
            goto fp_out; \
        } \
    } while (0)

#define CHECK_NEED_RETRY(result) \
    do { \
        if (result == FP_HAL_AUTH_NEED_RETRY) { \
            LOG_E(LOG_TAG, "%s:%d: result need retry", __func__, __LINE__); \
            continue; \
    } \
    } while (0)


#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus

#endif  //__FP_COMMON_H__
