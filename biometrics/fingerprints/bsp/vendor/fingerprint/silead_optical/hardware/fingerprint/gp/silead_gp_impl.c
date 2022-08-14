/******************************************************************************
 * @file   silead_gp_impl.c
 * @brief  Contains GP communication implements.
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

#define FILE_TAG "GP_IMP"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "GPComFunc.h"
#include "silead_error.h"
#include "silead_gp_impl.h"

#define FP_TZAPP_PATH "/system/vendor/app/mcRegistry"
#ifndef FP_GP_TZAPP_NAME
#define FP_GP_TZAPP_NAME (TEEC_UUID){0x511ead0a, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
#endif

static gp_handle_t *m_tz_handle = NULL;
static TEEC_Context *m_fp_context = NULL;
static TEEC_Session *m_fp_handle = NULL;

#define TZ_OK TEEC_SUCCESS

static int32_t _ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t flag, uint32_t v1, uint32_t v2,
        uint32_t *data1, uint32_t *data2)
{
    int32_t ret = 0;
    int32_t always_get = 0;
    TEEC_Operation sOperation;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    if (buffer == NULL || len <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    if (flag != 0) {
        sOperation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_INOUT, TEEC_NONE);
    } else {
        sOperation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_OUTPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);
    }

    sOperation.params[0].value.a = v1;
    sOperation.params[0].value.b = v2;
    sOperation.params[2].tmpref.buffer = buffer;
    sOperation.params[2].tmpref.size = len;

    if (flag != 0 && flag != 2 && flag != 4) {
        memset((void *)buffer, 0, len);
    }

    ret = m_tz_handle->TEEC_InvokeCommand(m_fp_handle, cmd, &sOperation, NULL);
    if (TZ_OK != ret) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        ret = sOperation.params[0].value.a;
        if (flag == 3 || flag == 4) {
            always_get = 1;
        }
    }

    if (ret >= 0 || always_get) {
        if (data1 != NULL) {
            *data1 = sOperation.params[0].value.b;
        }
        if (data2 != NULL) {
            *data2 = sOperation.params[1].value.a;
        }
    }

    return ret;
}

static int32_t _ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4,
                                       uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    int32_t ret = 0;
    TEEC_Operation sOperation;

    if (m_tz_handle == NULL || m_fp_handle == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    memset(&sOperation, 0, sizeof(TEEC_Operation));
    sOperation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_INOUT, TEEC_NONE, TEEC_NONE);

    sOperation.params[0].value.a = v1;
    sOperation.params[0].value.b = v2;
    sOperation.params[1].value.a = v3;
    sOperation.params[1].value.b = v4;

    ret = m_tz_handle->TEEC_InvokeCommand(m_fp_handle, cmd, &sOperation, NULL);
    if (TZ_OK != ret) {
        LOG_MSG_ERROR("send cmd(0x%02X) error (%d:%d:%s)", cmd, ret, errno, strerror(errno));
        ret = -SL_ERROR_TA_SEND_FAILED;
    } else {
        ret = sOperation.params[0].value.a;
    }

    if (data1 != NULL) {
        *data1 = sOperation.params[0].value.b;
    }
    if (data2 != NULL) {
        *data2 = sOperation.params[1].value.a;
    }
    if (data3 != NULL) {
        *data3 = sOperation.params[1].value.b;
    }

    return ret;
}

static int32_t _ca_str_to_uuid(const void *str, TEEC_UUID *uuid)
{
    int32_t ret = 0;
    char *buf = NULL;
    int32_t len = 0;

    char *value = NULL;
    char *saveptr = NULL;
    uint64_t leastsig = 0;

    if (str == NULL || uuid == NULL) {
        return -1;
    }

    len = strlen(str);
    if (len != 36) {
        LOG_MSG_ERROR("uuid invalid");
        return -1;
    }

    do {
        buf = malloc(len);
        if (buf == NULL) {
            ret = -1;
            break;
        }
        memcpy(buf, str, len);

        value = strtok_r(buf, "-", &saveptr);
        if (saveptr == NULL || (saveptr != buf + 9)) { // round 1: 8 + 1
            ret = -1;
            break;
        }
        uuid->timeLow = (uint32_t)strtoul(value, NULL, 16);

        value = strtok_r(NULL, "-", &saveptr);
        if (saveptr == NULL || (saveptr != buf + 14)) { // round 2: 8 + 1 + 4 + 1
            ret = -1;
            break;
        }
        uuid->timeMid = (uint16_t)strtoul(value, NULL, 16);

        value = strtok_r(NULL, "-", &saveptr);
        if (saveptr == NULL || (saveptr != buf + 19)) { // round 3: 8 + 1 + 4 + 1 + 4 + 1
            ret = -1;
            break;
        }
        uuid->timeHiAndVersion = (uint16_t)strtoul(value, NULL, 16);

        value = strtok_r(NULL, "-", &saveptr);
        if (saveptr == NULL || (saveptr != buf + 24)) { // round 4: 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1
            ret = -1;
            break;
        }
        leastsig = (uint16_t)strtoul(value, NULL, 16);
        uuid->clockSeqAndNode[0] = (uint8_t)(leastsig >> 8);
        uuid->clockSeqAndNode[1] = (uint8_t)leastsig;

        value = strtok_r(NULL, "-", &saveptr);
        if (saveptr != NULL) { // round 5: 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12
            ret = -1;
            break;
        }
        leastsig = (uint64_t)strtoull(value, NULL, 16);
        uuid->clockSeqAndNode[2] = (uint8_t)(leastsig >> 40);
        uuid->clockSeqAndNode[3] = (uint8_t)(leastsig >> 32);
        uuid->clockSeqAndNode[4] = (uint8_t)(leastsig >> 24);
        uuid->clockSeqAndNode[5] = (uint8_t)(leastsig >> 16);
        uuid->clockSeqAndNode[6] = (uint8_t)(leastsig >> 8);
        uuid->clockSeqAndNode[7] = (uint8_t)leastsig;
    } while (0);

    if (buf != NULL) {
        free(buf);
    }

    return ret;
}

static int32_t _ca_open(const void *ta_name)
{
    int32_t ret = 0;
    TEEC_UUID uuid;

    if (gp_open_handle(&m_tz_handle) < 0) {
        LOG_MSG_ERROR("gp_open_handle fail");
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    ret = _ca_str_to_uuid(ta_name, &uuid);
    if (ret >= 0) {
        ret = m_tz_handle->TEECCom_load_trustlet(m_tz_handle, &m_fp_context, &m_fp_handle, FP_TZAPP_PATH, &uuid);
    }

    if (ret < 0) {
        ret = m_tz_handle->TEECCom_load_trustlet(m_tz_handle, &m_fp_context, &m_fp_handle, FP_TZAPP_PATH, &FP_GP_TZAPP_NAME);
    }

    if (ret < 0) {
        ret = -SL_ERROR_TA_OPEN_FAILED;
    }

    return ret;
}

static int32_t _ca_close(void)
{
    LOG_MSG_DEBUG("gp close");

    if (m_tz_handle != NULL) {
        m_tz_handle->TEECCom_close_trustlet(m_tz_handle, m_fp_context, m_fp_handle);
        m_fp_context = NULL;
        m_fp_handle = NULL;
        gp_free_handle(&m_tz_handle);
        m_tz_handle = NULL;
    }

    return 0;
}

int32_t silfp_ca_gp_register(ca_impl_handle_t *handle, const void *ta_name)
{
    int32_t ret = 0;

    if (handle == NULL) {
        LOG_MSG_VERBOSE("handle buffer is invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _ca_open(ta_name);
    if (ret < 0) {
        _ca_close();
    }

    memset(handle, 0, sizeof(ca_impl_handle_t));
    handle->ca_send_modified_command = _ca_send_modified_command;
    handle->ca_send_normal_command = _ca_send_normal_command;
    handle->ca_close = _ca_close;

    return ret;
}
