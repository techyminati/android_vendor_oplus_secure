/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <stdlib.h>
#include <string.h>

#include <android/log.h>
#include "gf_error.h"
#include "gf_ca_entry.h"
#include "gf_type_define.h"

#include "fingerprint_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>


#define LOG_TAG "[gf_ca_entry]"

#define GF_CMD_TEEC_PARAM_TYPES TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, \
        TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE)

static TEEC_Context *g_context = NULL;
static TEEC_Session *g_session = NULL;
#define gf_ta_UUID_5658 { 0x05060000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}
#define gf_ta_UUID_3626 { 0x05060000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}
#define gf_ta_UUID_3636 { 0x05060000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 }}
#define gf_ta_UUID_3956 { 0x05060000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05 }}

#define GF_IOC_WAKELOCK_TIMEOUT_ENABLE        _IO('g', 18)
#define GF_IOC_WAKELOCK_TIMEOUT_DISABLE        _IO('g', 19)

static const TEEC_UUID UUID_5658 = gf_ta_UUID_5658;
static const TEEC_UUID UUID_3626 = gf_ta_UUID_3626;
static const TEEC_UUID UUID_3636 = gf_ta_UUID_3636;
static const TEEC_UUID UUID_3956 = gf_ta_UUID_3956;

static int g_ca_fd = -1; /* device handle */

void gf_ca_set_handle(int fd) {
    g_ca_fd = fd;
}

gf_error_t goodix_sysfs_node_wakelock_enable(uint8_t enable) {
    gf_error_t err = GF_SUCCESS;
    LOG_D("[%s] enter enable =%d ", __func__,enable);
    do {
        if (g_ca_fd < 0) {
            LOG_E( "[%s], no devic ", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }
        if (enable > 0 ){
            if (ioctl(g_ca_fd, GF_IOC_WAKELOCK_TIMEOUT_ENABLE) != 0) {
                LOG_E("[%s] GF_IOC_WAKELOCK_TIMEOUT_ENABLE ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }else if(enable == 0){
            if (ioctl(g_ca_fd, GF_IOC_WAKELOCK_TIMEOUT_DISABLE) != 0) {
                LOG_E("[%s] GF_IOC_WAKELOCK_TIMEOUT_ENABLE ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }else{
            LOG_E("[%s] enale is wrong", __func__);
            break;
        }
    } while (0);
    return err;
}

gf_error_t gf_ca_open_session() {
    gf_error_t err = GF_SUCCESS;
    TEEC_UUID UUID = gf_ta_UUID_5658; //default
    LOG_D("[%s] enter", __func__);

    UUID = gf_get_uuid();

    do {
        g_context = (TEEC_Context *) malloc(sizeof(TEEC_Context));
        if (NULL == g_context) {
            LOG_E("[%s], malloc g_context failed", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(g_context, 0, sizeof(TEEC_Context));

        TEEC_Result result = TEEC_InitializeContext(NULL, g_context);
        if (TEEC_SUCCESS != result) {
            LOG_E("[%s], TEEC_InitializeContext failed result = 0x%x", __func__, result);
            err = GF_ERROR_OPEN_TA_FAILED;
            break;
        }

        TEEC_Operation operation;

        g_session = (TEEC_Session *) malloc(sizeof(TEEC_Session));
        if (NULL == g_session) {
            LOG_E("[%s], malloc g_session failed", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(g_session, 0, sizeof(TEEC_Session));
        memset(&operation, 0, sizeof(TEEC_Operation));

        result = TEEC_OpenSession(g_context, g_session, &UUID, TEEC_LOGIN_PUBLIC, NULL, &operation,
                NULL);
        if (TEEC_SUCCESS != result) {
            LOG_E("[%s], TEEC_OpenSession failed result = 0x%x", __func__, result);
            err = GF_ERROR_OPEN_TA_FAILED;
            break;
        }
    } while (0);

    if (err != GF_SUCCESS) {
        gf_ca_close_session();
    }

    LOG_D("[%s] exit", __func__);
    return err;
}

void gf_ca_close_session(void) {
    LOG_D("[%s] enter", __func__);

    if (NULL != g_session) {
        TEEC_CloseSession(g_session);
        free(g_session);
        g_session = NULL;
    }

    if (NULL != g_context) {
        TEEC_FinalizeContext(g_context);
        free(g_context);
        g_context = NULL;
    }
}

gf_error_t gf_ca_invoke_command(uint32_t operation_id, uint32_t cmd_id, void *cmd, int len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t ret = TEEC_SUCCESS;
    TEEC_Operation operation;
    memset(&operation, 0, sizeof(TEEC_Operation));
    operation.paramTypes = GF_CMD_TEEC_PARAM_TYPES;
    operation.params[0].tmpref.buffer = cmd;
    operation.params[0].tmpref.size = len;
    operation.params[1].value.a = cmd_id;
    //add for wakelock
    goodix_sysfs_node_wakelock_enable(1);

    ret = TEEC_InvokeCommand(g_session, operation_id, &operation, NULL);
    if (GF_SUCCESS == ret) {
        err = operation.params[1].value.b;
    } else {
        err = GF_ERROR_TA_DEAD;
        gf_ca_close_session();
    }
    goodix_sysfs_node_wakelock_enable(0);

    return err;
}

TEEC_UUID gf_get_uuid() {
    TEEC_UUID uuid = gf_ta_UUID_5658; //default

    LOG_D("[%s] enter", __func__);

    if( 0 == strcmp(fp_config_info_init.fp_id_string, "G_3626")) {
        memcpy(&uuid, &UUID_3626, sizeof(UUID_3626));
        LOG_D("[%s] sensor is G_3626", __func__);
    } else if(0 == strcmp(fp_config_info_init.fp_id_string, "G_3636")) {
        memcpy(&uuid, &UUID_3636, sizeof(UUID_3636));
        LOG_D("[%s] sensor is G_3636", __func__);
    } else if(0 == strcmp(fp_config_info_init.fp_id_string, "G_3956")) {
        memcpy(&uuid, &UUID_3956, sizeof(UUID_3956));
        LOG_D("[%s] sensor is G_3956", __func__);

    } else {
        //todo
    }
out:
    return uuid;
}