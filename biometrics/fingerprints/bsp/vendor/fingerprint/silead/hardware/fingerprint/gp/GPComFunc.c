/******************************************************************************
 * @file   GPComFunc.c
 * @brief  Contains GP communication functions.
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

#define FILE_TAG "GP_WRAPPER"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "gp/GPComFunc.h"

#define GP_LIBRARY "libMcClient.so"

typedef struct {
    void *libHandle;
} _priv_data_t;

static void gp_close(gp_handle_t *gp_handle, TEEC_Context *clnt_context, TEEC_Session *clnt_handle);
static int32_t gp_load_trustlet(gp_handle_t *gp_handle, TEEC_Context **clnt_context, TEEC_Session **clnt_handle, const char __unused *path, const TEEC_UUID *tauuid);

#define DECLARE_LIB_FUNC(x, sx) \
    do { \
        handle->x = dlsym(data->libHandle, sx); \
        if (handle->x == NULL) { \
            LOG_MSG_DEBUG("Loading %s failed (%d:%s)", sx, errno, strerror(errno)); \
            goto exit; \
        } \
    } while(0)

int32_t gp_open_handle(gp_handle_t **ret_handle)
{
    gp_handle_t *handle = NULL;
    _priv_data_t *data = NULL;

    LOG_MSG_VERBOSE("Using Target Lib : %s", GP_LIBRARY);
    data = (_priv_data_t*)malloc(sizeof(_priv_data_t));
    if(data == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    data->libHandle = dlopen(GP_LIBRARY, RTLD_NOW);
    if(data->libHandle == NULL) {
        LOG_MSG_DEBUG("load GPCom API library failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }
    LOG_MSG_VERBOSE("GPCom API library successed (%p)", data->libHandle);

    handle = (gp_handle_t*)malloc(sizeof(gp_handle_t));
    if(handle == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->_data = data;

    // Setup internal functions
    handle->TEECCom_load_trustlet = gp_load_trustlet;
    handle->TEECCom_close_trustlet = gp_close;

    // Setup TEECom Functions
    DECLARE_LIB_FUNC(TEEC_InitializeContext, "TEEC_InitializeContext");
    DECLARE_LIB_FUNC(TEEC_FinalizeContext, "TEEC_FinalizeContext");
    DECLARE_LIB_FUNC(TEEC_RegisterSharedMemory, "TEEC_RegisterSharedMemory");
    DECLARE_LIB_FUNC(TEEC_AllocateSharedMemory, "TEEC_AllocateSharedMemory");
    DECLARE_LIB_FUNC(TEEC_ReleaseSharedMemory, "TEEC_ReleaseSharedMemory");
    DECLARE_LIB_FUNC(TEEC_OpenSession, "TEEC_OpenSession");
    DECLARE_LIB_FUNC(TEEC_CloseSession, "TEEC_CloseSession");
    DECLARE_LIB_FUNC(TEEC_InvokeCommand, "TEEC_InvokeCommand");
    DECLARE_LIB_FUNC(TEEC_RequestCancellation, "TEEC_RequestCancellation");

    if (ret_handle) {
        *ret_handle = handle;
        return 0;
    }

exit:
    if(handle != NULL) {
        free(handle);
        handle = NULL;
    }
    if(data != NULL) {
        if(data->libHandle != NULL) {
            dlclose(data->libHandle);
            data->libHandle = NULL;
        }
        free(data);
        data = NULL;
    }
    return -1;
}

int32_t gp_free_handle(gp_handle_t **handle_ptr)
{
    _priv_data_t *data = NULL;
    gp_handle_t *handle = NULL;

    if ((handle_ptr != NULL) && (*handle_ptr != NULL)) {
        handle = *handle_ptr;
        data = (_priv_data_t*)handle->_data;

        if(data != NULL) {
            if(data->libHandle != NULL) {
                dlclose(data->libHandle);
                data->libHandle = NULL;
            }
            free(data);
            data = NULL;
        }
        if(handle != NULL) {
            free(handle);
            handle = NULL;
        }
        *handle_ptr = NULL;
    }

    return 0;
}

char* gp_error_strings(int32_t err)
{
    switch (err) {
    case TEEC_ERROR_GENERIC:
        return "GPCom: Generic error.";
    case TEEC_ERROR_ACCESS_DENIED:
        return "GPCom: The underlying security system denies the access to the object.";

    case TEEC_ERROR_CANCEL:
        return "GPCom: The pending operation is cancelled.";

    case TEEC_ERROR_ACCESS_CONFLICT:
        return "GPCom: The underlying system detects a conflict.";
    case TEEC_ERROR_EXCESS_DATA:
        return "GPCom: Too much data for the operation or some data remain unprocessed by the operation.";
    case TEEC_ERROR_BAD_FORMAT:
        return "GPCom: Error of data format.";
    case TEEC_ERROR_BAD_PARAMETERS:
        return "GPCom: The specified parameters are invalid.";
    case TEEC_ERROR_BAD_STATE:
        return "GPCom: Illegal state for the operation.";
    case TEEC_ERROR_ITEM_NOT_FOUND:
        return "GPCom: The item is not found.";
    case TEEC_ERROR_NOT_IMPLEMENTED:
        return "GPCom: The specified operation is not implemented.";
    default:
        return "GPCom: Unknown error";
    }
}

/**
*  Function _gp_initialize:
*  Description:
*           Initialize: create a device context.
*  Output : TEEC_Context **context     = points to the device context
*
**/
static TEEC_Result _gp_initialize(gp_handle_t *gp_handle, TEEC_Context **context)
{
    TEEC_Result nError = TEEC_SUCCESS;
    TEEC_Context *pContext = NULL;

    if (gp_handle == NULL || context == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    *context = NULL;

    pContext = (TEEC_Context *)malloc(sizeof(TEEC_Context));
    if (pContext == NULL) {
        return TEEC_ERROR_OUT_OF_MEMORY;
    }

    memset(pContext, 0, sizeof(TEEC_Context));
    /* Create Device context  */
    nError = gp_handle->TEEC_InitializeContext(NULL, pContext);
    if (nError != TEEC_SUCCESS) {
        LOG_MSG_DEBUG("TEEC_InitializeContext failed (%08x), %s", nError, gp_error_strings(nError));
        if (nError == TEEC_ERROR_COMMUNICATION) {
            LOG_MSG_DEBUG("The client could not communicate with the service");
        }
        free(pContext);
    } else {
        *context = pContext;
    }

    return nError;
}

/**
*  Function _gp_finalize:
*  Description:
*           Finalize: delete the device context.
*  Input :  TEEC_Context *context     = the device context
*
**/
static TEEC_Result _gp_finalize(gp_handle_t *gp_handle, TEEC_Context *context)
{
    if (gp_handle == NULL || context == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    gp_handle->TEEC_FinalizeContext(context);
    free(context);
    return TEEC_SUCCESS;
}

/**
*  Function _gp_close_session:
*  Description:
*           Close the client session.
*  Input :  TEEC_Session *session - session handler
*
**/
static TEEC_Result _gp_close_session(gp_handle_t *gp_handle, TEEC_Session *session)
{
    if (gp_handle == NULL || session == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    gp_handle->TEEC_CloseSession(session);
    free(session);
    return TEEC_SUCCESS;
}

/**
*  Function _gp_open_session:
*  Description:
*           Open a client session with a specified service.
*  Input :  TEEC_Context  *context,    = the device context
*  Output:  TEEC_Session  **session   = points to the session handle
*
**/
static TEEC_Result _gp_open_session(gp_handle_t *gp_handle, TEEC_Context *context, TEEC_Session **session, const TEEC_UUID *tauuid)
{
    TEEC_Operation sOperation;
    TEEC_Result nError = TEEC_SUCCESS;
    TEEC_UUID uuid;

    if (gp_handle == NULL || context == NULL || session == NULL || tauuid == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    *session = (TEEC_Session *)malloc(sizeof(TEEC_Session));
    if (*session == NULL) {
        return TEEC_ERROR_OUT_OF_MEMORY;
    }

    memset(*session, 0, sizeof(TEEC_Session));
    memset(&sOperation, 0, sizeof(TEEC_Operation));
    memcpy(&uuid, tauuid, sizeof(TEEC_UUID));
    nError = gp_handle->TEEC_OpenSession(context,
                                         *session,         /* OUT session */
                                         &uuid,            /* destination UUID */
                                         TEEC_LOGIN_PUBLIC,/* connectionMethod */
                                         NULL,             /* connectionData */
                                         &sOperation,      /* IN OUT operation */
                                         NULL              /* OUT returnOrigin, optional */
                                        );
    if (nError != TEEC_SUCCESS) {
        LOG_MSG_DEBUG("load %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x failed (%08x), %s", uuid.timeLow, uuid.timeMid, uuid.timeHiAndVersion,
                      uuid.clockSeqAndNode[0], uuid.clockSeqAndNode[1], uuid.clockSeqAndNode[2], uuid.clockSeqAndNode[3],
                      uuid.clockSeqAndNode[4], uuid.clockSeqAndNode[5], uuid.clockSeqAndNode[6], uuid.clockSeqAndNode[7], nError, gp_error_strings(nError));
        free(*session);
        return nError;
    }

    LOG_MSG_DEBUG("load %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x success", uuid.timeLow, uuid.timeMid, uuid.timeHiAndVersion,
                  uuid.clockSeqAndNode[0], uuid.clockSeqAndNode[1], uuid.clockSeqAndNode[2], uuid.clockSeqAndNode[3],
                  uuid.clockSeqAndNode[4], uuid.clockSeqAndNode[5], uuid.clockSeqAndNode[6], uuid.clockSeqAndNode[7]);
    return TEEC_SUCCESS;
}

static int32_t gp_load_trustlet(gp_handle_t *gp_handle, TEEC_Context **clnt_context, TEEC_Session **clnt_handle, const char __unused *path, const TEEC_UUID *tauuid)
{
    TEEC_Result nError = TEEC_SUCCESS;
    TEEC_Context *context = NULL;
    TEEC_Session *session = NULL;

    if (gp_handle == NULL || clnt_context == NULL || clnt_handle == NULL || tauuid == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -1;
    }

    nError = _gp_initialize(gp_handle, &context);
    if (nError != TEEC_SUCCESS) {
        LOG_MSG_ERROR("_gp_initialize failed (%08x), %s", nError, gp_error_strings(nError));
        return -1;
    }

    /* Open a session */
    nError = _gp_open_session(gp_handle, context, &session, tauuid);
    if (nError != TEEC_SUCCESS) {
        LOG_MSG_ERROR("_gp_open_session failed (%08x), %s", nError, gp_error_strings(nError));
        _gp_finalize(gp_handle, context);
        return -1;
    }

    *clnt_context = context;
    *clnt_handle = session;

    return 0;
}

static void gp_close(gp_handle_t *gp_handle, TEEC_Context *clnt_context, TEEC_Session *clnt_handle)
{
    TEEC_Result nError = TEEC_SUCCESS;

    /* Close the session */
    if (clnt_context != NULL) {
        nError = _gp_close_session(gp_handle, clnt_handle);
        if (nError != TEEC_SUCCESS) {
            LOG_MSG_DEBUG("closeSession failed (%08x), %s", nError, gp_error_strings(nError));
        }
    }

    /* Finalize */
    if (clnt_handle != NULL) {
        nError = _gp_finalize(gp_handle, clnt_context);
        if (nError != TEEC_SUCCESS) {
            LOG_MSG_DEBUG("finalize failed (%08x), %s", nError, gp_error_strings(nError));
        }
    }
}