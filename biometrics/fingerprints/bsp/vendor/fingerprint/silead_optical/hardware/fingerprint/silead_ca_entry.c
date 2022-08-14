/******************************************************************************
 * @file   silead_ca_entry.c
 * @brief  Contains CA entry functions.
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
 * <author>      <date>     <version>     <desc>
 * David Wang    2018/4/2    0.1.0      Init version
 * Bangxiong.Wu  2019/04/09  1.0.0      fp thread exit when send cmd to ta exceed limit
 *
 *****************************************************************************/

#define FILE_TAG "silead_ca_entry"
#include "log/logmsg.h"

#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "silead_error.h"
#include "silead_ca.h"
#include "silead_ca_impl.h"
#include "tz_cmd.h"

#ifdef SECURITY_TYPE_QSEE
#include "qsee/silead_qsee_impl.h"
#endif
#ifdef SECURITY_TYPE_TEE
#include "tee/silead_tee_impl.h"
#endif
#ifdef SECURITY_TYPE_GP
#include "gp/silead_gp_impl.h"
#endif
#ifdef SECURITY_TYPE_TRUSTY
#include "trusty/silead_trusty_impl.h"
#endif
#ifdef SECURITY_TYPE_OPTEE
#include "optee/silead_optee_impl.h"
#endif
#ifdef SECURITY_TYPE_NOSEC
#include "nosec/silead_nosec_impl.h"
#endif

#define DUMP_PUINT_VALUE(p) ((p != NULL) ? (*p) : 0)

#define ERROR_TO_IDLE_STEPS 5

extern void silfp_worker_set_to_break_mode(void);
extern int32_t silfp_impl_wakelock(uint8_t lock);

ca_impl_handle_t m_ca_handler;
uint32_t m_ta_send_error_times = 0;

static pthread_mutex_t m_tz_lock;
#define mutex_lock()    pthread_mutex_lock(&m_tz_lock); silfp_impl_wakelock(1)
#define mutex_unlock()  silfp_impl_wakelock(0); pthread_mutex_unlock(&m_tz_lock)
#define mutex_init()    pthread_mutex_init(&m_tz_lock, NULL)
#define mutex_destroy() pthread_mutex_destroy(&m_tz_lock);

static void _ca_check_send_result(int32_t ret)
{
    if (ret == -SL_ERROR_TA_OPEN_FAILED || ret == -SL_ERROR_TA_SEND_FAILED) {
        m_ta_send_error_times++;
    } else {
        m_ta_send_error_times = 0;
    }

    if (m_ta_send_error_times >= ERROR_TO_IDLE_STEPS) {
        //silfp_worker_set_to_break_mode();
        LOG_MSG_ERROR("send cmd to ta error times exceed limit, EXIT!!!");
        exit(0);
    }
}

int32_t silfp_ca_send_modified_command(uint32_t cmd, void *buffer, uint32_t len, uint32_t flag, uint32_t v1, uint32_t v2, uint32_t *data1, uint32_t *data2)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_ca_handler.ca_send_modified_command != NULL) {
        LOG_MSG_DEBUG("> 0x%x (%s) [%u %u 0x%x:0x%x]", cmd, silfp_cmd_to_str(cmd), len, flag, v1, v2);
        mutex_lock();
        ret = m_ca_handler.ca_send_modified_command(cmd, buffer, len, flag, v1, v2, data1, data2);
        mutex_unlock();
        LOG_MSG_DEBUG("< 0x%x %d (%s) [0x%x:0x%x]", cmd, ret, silfp_err_to_string(ret), DUMP_PUINT_VALUE(data1), DUMP_PUINT_VALUE(data2));
    }

    _ca_check_send_result(ret);

    return ret;
}

int32_t silfp_ca_send_normal_command(uint32_t cmd, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4, uint32_t *data1, uint32_t *data2, uint32_t *data3)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_ca_handler.ca_send_normal_command != NULL) {
        LOG_MSG_DEBUG("> 0x%x (%s) [0x%x:0x%x:0x%x:0x%x]", cmd, silfp_cmd_to_str(cmd), v1, v2, v3, v4);
        mutex_lock();
        ret = m_ca_handler.ca_send_normal_command(cmd, v1, v2, v3, v4, data1, data2, data3);
        mutex_unlock();
        LOG_MSG_DEBUG("< 0x%x %d(%s) [0x%x:0x%x:0x%x]", cmd, ret, silfp_err_to_string(ret), DUMP_PUINT_VALUE(data1), DUMP_PUINT_VALUE(data2), DUMP_PUINT_VALUE(data3));
    }

    _ca_check_send_result(ret);

    return ret;
}

int32_t silfp_ca_open(const void *ta_name)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    memset(&m_ca_handler, 0, sizeof(m_ca_handler));

    mutex_init();

#ifdef SECURITY_TYPE_QSEE
    if (ret < 0) {
        ret = silfp_ca_qsee_register(&m_ca_handler, ta_name);
    }
#endif
#ifdef SECURITY_TYPE_GP
    if (ret < 0) {
        ret = silfp_ca_gp_register(&m_ca_handler, ta_name);
    }
#endif
#ifdef SECURITY_TYPE_TEE
    if (ret < 0) {
        ret = silfp_ca_tee_register(&m_ca_handler, ta_name);
    }
#endif
#ifdef SECURITY_TYPE_TRUSTY
    if (ret < 0) {
        ret = silfp_ca_trusty_register(&m_ca_handler, ta_name);
    }
#endif
#ifdef SECURITY_TYPE_OPTEE
    if (ret < 0) {
        ret = silfp_ca_optee_register(&m_ca_handler, ta_name);
    }
#endif
#ifdef SECURITY_TYPE_NOSEC
    if (ret < 0) {
        ret = silfp_ca_nosec_register(&m_ca_handler, ta_name);
    }
#endif

    return ret;
}

int32_t silfp_ca_close(void)
{
    int32_t ret = 0;

    if (m_ca_handler.ca_close == NULL) {
        return -SL_ERROR_TA_OPEN_FAILED;
    }

    ret = m_ca_handler.ca_close();
    mutex_destroy();

    return ret;
}

int32_t silfp_ca_keymaster_get(void **buffer)
{
    if (m_ca_handler.ca_keymaster_get == NULL) {
        return 0;
    }

    return m_ca_handler.ca_keymaster_get(buffer);
}
