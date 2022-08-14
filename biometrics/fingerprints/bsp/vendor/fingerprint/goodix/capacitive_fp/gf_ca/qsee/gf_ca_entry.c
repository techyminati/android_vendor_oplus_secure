/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_ca entry file
 * History:
 * Version: 1.0
 */

#include <errno.h>
#include <android/log.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/msm_ion.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gf_user_type_define.h"
#include "gf_keymaster.h"
#include "QSEEComAPI.h"
#include "gf_error.h"
#include "gf_ca_entry.h"
#include "gf_type_define.h"
#include "gf_ca_dump_log.h"
#include "gf_sec_ion.h"
#include "gf_ca_common.h"
#include "gf_whitebox.h"

#ifdef WHITEBOX_TEE_REE
#include <cutils/properties.h>
#endif  // WHITEBOX_TEE_REE

#ifdef WHITEBOX_DYNAMIC_CONTROL
#define PROPERTY_WHITEBOX_ENABLED  "gf.debug.whitebox.enabled"
#define READ_WHITEBOX_ENABLE_STATUS() (g_whitebox_enabled \
    = property_get_int32(PROPERTY_WHITEBOX_ENABLED, 0))
// flag to mark whitebox dynamic enable status, default value is disabled.
static int32_t g_whitebox_enabled = 0;
#else  // WHITEBOX_DYNAMIC_CONTROL undefined
#define READ_WHITEBOX_ENABLE_STATUS()
// when WHITEBOX_DYNAMIC_CONTROL macro is not defined, whitebox is always
// enabled when WHITEBOX_TEE_REE is defined.
static const int32_t g_whitebox_enabled = 1;
#endif  // end of WHITEBOX_DYNAMIC_CONTROL

#define RETRYMAX 15

// The data never be encrypted
typedef struct
{
    uint32_t cmd_buffer_addr;  // initialized by qsee TEE environment
    int16_t dynamic_whitebox;
    int16_t whitebox_enabled;
} qsc_cmd_prefix_t;

typedef struct extend_msg
{
    uint32_t send_cmd_token_id;
    uint32_t operation_id;
    uint64_t tm_msec;
    gf_log_level_t logdump_level;
} extend_msg_t;

/* Do not modify*/
typedef struct qsc_send_cmd
{
    extend_msg_t msg;
    uint32_t cmd_id;
    uint32_t len;
} qsc_send_cmd_t;

typedef struct qsc_send_cmd_rsp
{
    uint32_t data;
    int32_t status;
} qsc_send_cmd_rsp_t;

static pthread_mutex_t g_ca_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;  // ca_cmd_mutex
struct QSEECom_handle *g_ta_handle = NULL;  // ta_handle
static uint32_t g_send_cmd_token = 0;  // send cmd token
#define TA_PATH_NUM   5
char *ta_path[TA_PATH_NUM] =  // ta_path
{
    "/odm/vendor/firmware",
    "/oplus_version/vendor/firmware",
    "/vendor/firmware_mnt/image",
    "/vendor/firmware",
    "/data/vendor/euclid/version/vendor/firmware"
};
char ta_name[] = "goodixfp";  // ta_name
uint32_t ta_buf_size = 4096;  // ta_buf_size

#define LOG_TAG "[gf_ca_entry]"
#define LOG_D(...) (__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOG_E(...) (__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

/**
 * Function: gf_ta_send_modified_cmd_req
 * Description: send modified cmd req
 * Input: operation_id, cmd_id, data, data_size, err
 * Output: none
 * Return:
 * 0       if succeed to send modified cmd req
 * others  if fail to send modified cmd req
 */
static int32_t gf_ta_send_modified_cmd_req(uint32_t operation_id,
                                           uint32_t cmd_id,
                                           unsigned char *data, int32_t data_size, gf_error_t *err)
{
    int32_t retval = 0;
    qsc_send_cmd_t *send_cmd = NULL;
    qsc_cmd_prefix_t *cmd_prefix = NULL;
    qsc_send_cmd_rsp_t *resp = NULL;
    int32_t send_len = 0;
    int32_t resp_len = 0;
    struct timeval time;
    struct tm current_tm = {0};
    struct QSEECom_ion_fd_info ion_fd_info;
    qsc_ion_info_t ion_handle;
    uint32_t extend_size = 0;
    uint32_t aligned_data_size = 0;
    uint8_t dynamic_whitebox = (cmd_id != GF_USER_CMD_INIT_CA_AUTH
        && cmd_id != GF_USER_CMD_CONFIRM_CA_AUTH) ? WB_TRUE : WB_FALSE;
    int32_t i = 0;
    uint32_t len = 0;
    uint8_t real_digest[HMAC_DIGEST_SIZE];
    uint8_t *expected_digest = NULL;
    uint8_t *buffer = NULL;
    uint8_t *cmd_ptr = NULL;

    LOG_D("[%s] enter", __func__);

    do
    {
        g_send_cmd_token++;
        LOG_D("[%s] begin token %d, cmd_id=%d", __func__, g_send_cmd_token, cmd_id);
        struct QSEECom_handle *tac_handle = g_ta_handle;
        ion_handle.ion_fd = 0;
#if TARGET_ION_ABI_VERSION < 2
        ion_handle.ion_alloc_handle.handle = 0;
#endif  // #if TARGET_ION_ABI_VERSION < 2
        extend_size = gf_ca_get_logbuf_length();
        aligned_data_size = QSEECOM_ALIGN(AES_BLOCK_ALIGN(data_size) + HMAC_DIGEST_SIZE);
        retval = gf_sec_ion_malloc(&ion_handle, (aligned_data_size + extend_size));

        if (0 != retval)
        {
            LOG_E("[%s] gf_sec_ion_malloc failed with retval = %d\n", __func__, retval);
            retval = -1;
            break;
        }

        memset(ion_handle.ion_sbuffer, 0, aligned_data_size + extend_size);

        memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
        ion_fd_info.data[0].fd = ion_handle.ifd_data_fd;
        ion_fd_info.data[0].cmd_buf_offset = offsetof(qsc_cmd_prefix_t, cmd_buffer_addr);
        // memory structure of 'tac_handle->ion_sbuffer':
        // ('###' represent aligned space)
        // |<---            send_len              --->|<-resp_len->|
        // |              |              |<-32->|     |            |
        // |--------------+--------------+------+-----+------------|
        // |  cmd_prefix  | send_cmd ### | hmac | ### |    resp    |
        // |--------------+--------------+------+-----+------------|
        // 'send_cmd' may be encrypted and hmac calculated, but 'resp'
        // is always raw data.
        cmd_prefix = (qsc_cmd_prefix_t *) (tac_handle->ion_sbuffer);
        send_cmd = (qsc_send_cmd_t *) (tac_handle->ion_sbuffer + sizeof(qsc_cmd_prefix_t));
        send_len = QSEECOM_ALIGN(sizeof(qsc_cmd_prefix_t)
                + AES_BLOCK_ALIGN(sizeof(qsc_send_cmd_t)) + HMAC_DIGEST_SIZE);
        resp_len = QSEECOM_ALIGN(sizeof(qsc_send_cmd_rsp_t));
        resp = (qsc_send_cmd_rsp_t *) (tac_handle->ion_sbuffer + send_len);
        send_cmd->msg.send_cmd_token_id = g_send_cmd_token;
        send_cmd->msg.operation_id = operation_id;
        send_cmd->msg.logdump_level = g_logdump_level;
        send_cmd->cmd_id = cmd_id;
        send_cmd->len = data_size;
        resp->status = 0;
        gettimeofday(&time, NULL);
        localtime_r(&time.tv_sec, &current_tm);
        send_cmd->msg.tm_msec = 1000 * ((60 * 60) * current_tm.tm_hour + 60 *
                                        current_tm.tm_min +
                                        current_tm.tm_sec) + time.tv_usec / 1000;

#ifdef WHITEBOX_TEE_REE
        cmd_prefix->whitebox_enabled = g_whitebox_enabled;
        if (g_whitebox_enabled > 0)
        {
            len = AES_BLOCK_ALIGN(data_size);
            buffer = (uint8_t *) malloc(len);
            if (buffer == NULL)
            {
                retval = -1;
                break;
            }

            LOG_D("[%s] whitebox is enabled.", __func__);
            WHITEBOX_ENCRYPT(data, buffer, data_size, dynamic_whitebox);
            memcpy(ion_handle.ion_sbuffer, buffer, len);
            whitebox_exec(WB_OP_HMAC_CRC, buffer, ion_handle.ion_sbuffer + len, len, 0);

            cmd_ptr = (uint8_t *) send_cmd;
            len = AES_BLOCK_ALIGN(sizeof(qsc_send_cmd_t));
            WHITEBOX_ENCRYPT(cmd_ptr, cmd_ptr, sizeof(qsc_send_cmd_t), dynamic_whitebox);
            expected_digest = cmd_ptr + len;
            whitebox_exec(WB_OP_HMAC_CRC, cmd_ptr, expected_digest, len, 0);
            // save dynamic whitebox flag
            cmd_prefix->dynamic_whitebox = (dynamic_whitebox == WB_TRUE ? 1 : 0);
        }
        else
        {
            LOG_D("[%s] whitebox is disabled.", __func__);
            memcpy(ion_handle.ion_sbuffer, data, data_size);
        }
#else  // WHITEBOX_TEE_REE
        LOG_D("[%s] whitebox is disabled.", __func__);
        memcpy(ion_handle.ion_sbuffer, data, data_size);
#endif  // WHITEBOX_TEE_REE

        retval = QSEECom_send_modified_cmd(tac_handle, tac_handle->ion_sbuffer, send_len,
                                           resp, resp_len, &ion_fd_info);

        if (0 != retval)
        {
            LOG_E("[%s] QSEECom_send_modified_cmd failed with retval = %d\n", __func__, retval);
            gf_sec_ion_free(&ion_handle);
            retval = -1;
            break;
        }
        retval = resp->status;

#ifdef WHITEBOX_TEE_REE
        if (g_whitebox_enabled > 0)
        {
            expected_digest = ion_handle.ion_sbuffer + AES_BLOCK_ALIGN(data_size);
            memcpy(buffer, ion_handle.ion_sbuffer, AES_BLOCK_ALIGN(data_size));
            whitebox_exec(WB_OP_HMAC_CRC, buffer, real_digest, AES_BLOCK_ALIGN(data_size), 0);
            retval = memcmp(expected_digest, real_digest, HMAC_DIGEST_SIZE);
            if (0 != retval)
            {
                LOG_E("[%s] Check command return data digest failed.", __func__);
                retval = -1;
                break;
            }

            if (cmd_id == GF_USER_CMD_CONFIRM_CA_AUTH)
            {
                if (data_size != AES_BLOCK_ALIGN(data_size))
                {
                    LOG_E("[%s] The size of command data for ca_auth_confirm"
                            " command must be aligned with AES block. ", __func__);
                    retval = -1;
                    break;
                }
                memcpy(data, (uint8_t *) ion_handle.ion_sbuffer, AES_BLOCK_ALIGN(data_size));
            }
            else
            {
                WHITEBOX_DECRYPT((uint8_t *) ion_handle.ion_sbuffer, data, data_size, dynamic_whitebox);
            }
        }  // if (g_whitebox_enabled > 0)
        else
        {
            memcpy(data, (uint8_t *) ion_handle.ion_sbuffer, data_size);
        }
#else  // WHITEBOX_TEE_REE
        memcpy(data, (uint8_t *) ion_handle.ion_sbuffer, data_size);
#endif  // WHITEBOX_TEE_REE
        gf_ca_logdump((gf_ta_log_info_t *)(ion_handle.ion_sbuffer +
                                                aligned_data_size));
        *err = resp->data;
        gf_sec_ion_free(&ion_handle);
    } while (0);  // do...

#ifdef WHITEBOX_TEE_REE
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
#endif  // WHITEBOX_TEE_REE

    LOG_D("[%s] exit", __func__);
    return retval;
}

/**
 * Function: gf_ca_set_handle
 * Description: set handle
 * Input: fd
 * Output: none
 * Return: none
 */
void gf_ca_set_handle(int32_t fd)
{
    UNUSED_VAR(fd);
}

/**
 * Function: send_hmac_key
 * Description: send hmac key
 * Input: none
 * Output: none
 * Return: none
 */
static void send_hmac_key()
{
    gf_error_t err = GF_SUCCESS;
    int32_t size = sizeof(gf_hmac_key_t);
    gf_hmac_key_t cmd;
    LOG_D("[%s] enter", __func__);
    memset(&cmd, 0, size);

    do
    {
        get_key_from_keymaster(cmd.hmac_key, QSEE_HMAC_KEY_MAX_LEN);
        err = gf_ca_invoke_command(GF_USER_OPERATION_ID, GF_USER_CMD_SET_HMAC_KEY, &cmd,
                             size);
        if (err != GF_SUCCESS)
        {
            LOG_E("[%s] ca invoke command failed, errno=%d", __func__, err);
        }
    }
    while (0);

    LOG_D("[%s] exit", __func__);
}

/**
 * Function: gf_ca_open_session
 * Description: open session
 * Input: none
 * Output: none
 * Return:
 * GF_SUCCESS               if succeed to open session
 * GF_ERROR_OPEN_TA_FAILED  if fail to open session
 */
gf_error_t gf_ca_open_session(void)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t ret = 0;
    uint32_t env_secure = WB_TRUE;
    int32_t i;
    int32_t retry = RETRYMAX;
    LOG_D("[%s] zoulian enter", __func__);

#ifdef WHITEBOX_TEE_REE
    READ_WHITEBOX_ENABLE_STATUS();
    if (g_whitebox_enabled > 0)
    {
        whitebox_exec(WB_OP_CHECK_ENV, NULL, &env_secure, sizeof(uint32_t), 0);
        if (env_secure == WB_FALSE)
        {
            LOG_E("[%s] Running environment of current process is invalid, quit.", __func__);
            return GF_ERROR_GENERIC;
        }
    }
#endif  // WHITEBOX_TEE_REE

    for (i = 0; i < TA_PATH_NUM; i++)
    {
        while (retry > 0)
        {
            retry--;
            LOG_E("[%s] Loading %s retry %d.", __func__, ta_name, RETRYMAX-retry);
            ret = QSEECom_start_app(&g_ta_handle, ta_path[i], ta_name, ta_buf_size);

            if (ret == 0)
            {
                break;
            }
            else{
                LOG_E("[%s] Loading %s failed: ret=%d, errno=%d.", __func__, ta_name, ret,
                    errno);
                if (errno == 2)
                {
                    break;
                }
            }
            sleep(1);
        }
        if (ret)
        {
            continue;
        }
        LOG_D("[%s] Loading %s Succeed.", __func__, ta_name);
        gf_sec_ion_init();

#ifdef WHITEBOX_TEE_REE
        if (g_whitebox_enabled > 0)
        {
            err = gf_ca_tee_auth();
            if (err != GF_SUCCESS)
            {
                return err;
            }
        }
#endif  // WHITEBOX_TEE_REE


        send_hmac_key();
        return GF_SUCCESS;
    }  // for...

    LOG_D("[%s] Loading %s failed: ret=%d, errno=%d.", __func__, ta_name, ret,
          errno);
    return GF_ERROR_OPEN_TA_FAILED;
}

/**
 * Function: gf_ca_close_session
 * Description: close session
 * Input: none
 * Output: none
 * Return: none
 */
void gf_ca_close_session(void)
{
    LOG_D("[%s] enter", __func__);

    do
    {
        gf_sec_ion_destroy();

        if (g_ta_handle != NULL)
        {
            uint32_t ret = 0;
            ret = QSEECom_shutdown_app(&g_ta_handle);

            if (ret)
            {
                LOG_E("[%s] Unload %s failed: ret=%d, errno=%d", __func__, ta_name, ret, errno);
            }
            else
            {
                g_ta_handle = NULL;
                LOG_D("[%s] Unload %s succeed.", __func__, ta_name);
            }

            break;
        }
        else
        {
            LOG_E("[%s] g_ta_handle is NULL.", __func__);
            break;
        }
    } while (0);

    return;
    LOG_D("[%s] exit", __func__);
}

/**
 * Function: gf_ca_invoke_command
 * Description: send command from gf_ca to gf_ta
 * Input: operation_id, cmd_id, cmd, len
 * Output: none
 * Return:
 * GF_SUCCESS               if succeed to invoke command
 * GF_ERROR_OPEN_TA_FAILED  if fail to invoke command
 */
gf_error_t gf_ca_invoke_command(uint32_t operation_id, uint32_t cmd_id,
                                void *cmd, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    LOG_D("[%s] enter", __func__);

    do
    {
        int32_t ret = gf_ta_send_modified_cmd_req(operation_id, cmd_id, cmd, len, &err);

        if (GF_SUCCESS == ret)
        {
            LOG_D("[%s] gx_ta_send_modified_cmd_req success", __func__);

            if (GF_SUCCESS == err)
            {
                LOG_D("[%s] ta execution succeed", __func__);
            }
            else
            {
                LOG_E("[%s] ta execution failed, errno=%d", __func__, err);
            }
        }
        else
        {
            LOG_E("[%s] gx_ta_send_modified_cmd_req fail, ret=%d", __func__, ret);
            err = GF_ERROR_TA_DEAD;
        }
    }
    while (0);

    LOG_D("[%s] exit", __func__);
    return err;
}
