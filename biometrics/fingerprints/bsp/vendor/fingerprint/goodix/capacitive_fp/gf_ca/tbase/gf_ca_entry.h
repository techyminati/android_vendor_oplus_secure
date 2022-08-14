/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_CA_ENTRY_H__
#define __GF_CA_ENTRY_H__

#include <gf_error.h>
#include <gf_type_define.h>
#include <tee_client_api.h>

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_ca_open_session();
void gf_ca_close_session(void);
gf_error_t gf_ca_invoke_command(uint32_t operation_id, uint32_t cmd_id, void *cmd, int len);

void gf_ca_set_handle(int fd);
TEEC_UUID gf_get_uuid();

#ifndef GF_LOG_LEVEL
#define GF_LOG_LEVEL 1
#endif

#define GF_LOG_VERBOSE_LEVEL   4
#define GF_LOG_DEBUG_LEVEL   3
#define GF_LOG_INFO_LEVEL    2
#define GF_LOG_ERROR_LEVEL   1

#define LOG_D(...) \
do { \
    if (GF_LOG_LEVEL >= GF_LOG_DEBUG_LEVEL) { \
        __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__);\
    } \
} while (0);

#define LOG_E(...) \
do { \
    if (GF_LOG_LEVEL >= GF_LOG_ERROR_LEVEL) { \
        __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__);\
    } \
} while (0);
#ifdef __cplusplus
}
#endif

#endif  // __GF_CA_ENTRY_H__
