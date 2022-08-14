/******************************************************************************
 * @file   GPComFunc.h
 * @brief  Contains GP communication functions header file.
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
 * Daniel Ye   2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __GPCOMFUNC_H__
#define __GPCOMFUNC_H__

#include <dlfcn.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "public/tee_type.h"
#include "public/tee_error.h"
#include "public/tee_client_api.h"

#define UUID_LEN  16

typedef struct gp_handle {
    void *_data;

    TEEC_Result (*TEEC_InitializeContext)(const char *name, TEEC_Context *context);
    TEEC_Result (*TEEC_FinalizeContext)(TEEC_Context *context);
    TEEC_Result (*TEEC_RegisterSharedMemory)(TEEC_Context *context, TEEC_SharedMemory *sharedMem);
    TEEC_Result (*TEEC_AllocateSharedMemory)(TEEC_Context *context, TEEC_SharedMemory *sharedMem);
    TEEC_Result (*TEEC_ReleaseSharedMemory)(TEEC_SharedMemory *sharedMem);
    TEEC_Result (*TEEC_OpenSession)(TEEC_Context *context, TEEC_Session *session, const TEEC_UUID *destination, uint32_t connectionMethod,
                                    const void *connectionData, TEEC_Operation *operation, uint32_t *returnOrigin);
    TEEC_Result (*TEEC_CloseSession)(TEEC_Session *session);
    TEEC_Result (*TEEC_InvokeCommand)(TEEC_Session *session, uint32_t commandID, TEEC_Operation *operation, uint32_t *returnOrigin);
    TEEC_Result (*TEEC_RequestCancellation)(TEEC_Operation *operation);

    int32_t (*TEECCom_load_trustlet)(struct gp_handle *gp_handle, TEEC_Context **clnt_context, TEEC_Session **clnt_handle, const char __unused *path, const TEEC_UUID *tauuid);
    void (*TEECCom_close_trustlet)(struct gp_handle *gp_handle, TEEC_Context *clnt_context, TEEC_Session *clnt_handle);
} gp_handle_t;

int32_t gp_open_handle(gp_handle_t **handle);
int32_t gp_free_handle(gp_handle_t **handle);
char* gp_error_strings(int32_t err);

#endif /* __GPCOMFUNC_H__ */
