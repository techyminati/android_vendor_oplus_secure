/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef _GF_GLOBAL_DEFINE_H_
#define _GF_GLOBAL_DEFINE_H_

#include "gf_base_types.h"

/***************HalContext.h***************/
#define HAL_CONTEXT_DECLARE_INJ
#define HAL_CONTEXT_DEFINE_INJ
/***************HalContext.cpp***************/
#define HAL_CONTEXT_CREATE_INJ
#define HAL_CONTEXT_DELETE_INJ
#define HAL_CONTEXT_CREATE_DEV mDevice = createDevice(this);
#define HAL_CONTEXT_INJ_DATA
#define HAL_CONTEXT_INJ_MOCK_DATA
#define HAL_CONTEXT_START_TR
#define HAL_CONTEXT_STOP_TR
#define HAL_CONTEXT_EXT_TOOLS_CREATE
#define HAL_CONTEXT_DEBUG_TOOLS_CREATE
#define HAL_CONTEXT_FRRFAR_TOOLS_CREATE
#define HAL_CONTEXT_HEARTBEATRATE_CREATE
/***************gf_modules.h***************/
#define GF_MODULES_DECLARE_INJ
#define GF_MODULES_DECLARE_AT
#define GF_MODULES_DECLARE_TR
#define GF_MODULES_DECLARE_EXT_TOOLS
#define GF_MODULES_DECLARE_DEBUG_TOOLS
#define GF_MODULES_DECLARE_FRRFAR_TOOLS
#define GF_MODULES_DECLARE_HEARTBEAT_RATE
/***************gf_modules.c***************/
#define GF_MODULES_REF_INJ
#define GF_MODULES_REF_AT
#define GF_MODULES_REF_TR
#define GF_MODULES_REF_EXT_TOOLS
#define GF_MODULES_REF_DEBUG_TOOLS
#define GF_MODULES_REF_FRRFAR_TOOLS
#define GF_MODULES_REF_HEARTBEAT_RATE
/***************gf_secure_object.c***************/
#define GF_SECURE_OBJECT_LOAD_MK_SO
#define GF_SECURE_OBJECT_GET_MK_SO_LEN
/***************app_main.c/gf_ta_entry.c***************/
#define TA_ENTRY_INIT_MC
#define TA_ENTRY_DESTROY_MC
/***************cpl_memory.c***************/
#define CPL_MEMORY_MC_MALLOC
#define CPL_MEMORY_MC_REALLOC
#define CPL_MEMORY_MC_FREE
/***************gf_trace.c***************/
#define GF_TRACE_PRINT_MC_INFO
/************FingerprintCore.cpp***********/
#define GF_SWITCH_NEXT_FINGER
/*************FingerprintCore.cpp**********/

#define GF_TRACE_INT_EXT(tag, value, append) do { UNUSED_VAR(tag, value, append); } while (0);
#define GF_TRACE_INT(tag, value) GF_TRACE_INT_EXT(tag, value, 0)
#define GF_TRACE_INT64_EXT(tag, value, append) do { UNUSED_VAR(tag, value, append); } while (0);
#define GF_TRACE_INT64(tag, value) GF_TRACE_INT64_EXT(tag, value, 0)
#define GF_TRACE_STR_EXT(tag, value, append) do { UNUSED_VAR(tag, value, append); } while (0);
#define GF_TRACE_STR(tag, value) GF_TRACE_STR_EXT(tag, value, 0)
#define GF_TRACE_BINARY_EXT(tag, value, value_len, append) do { UNUSED_VAR(tag, value, value_len, append); } while (0);
#define GF_TRACE_BINARY(tag, value, value_len) GF_TRACE_BINARY_EXT(tag, value, value_len, 0)
#define GF_TRACE_MEM_INFO()













#endif  // _GF_GLOBAL_DEFINE_H_
