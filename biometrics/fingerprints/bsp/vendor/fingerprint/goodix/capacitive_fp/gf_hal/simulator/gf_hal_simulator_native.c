/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL layer simulator native
 * History:
 * Version: 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cutils/fs.h>
#include <sys/types.h>
#include "gf_hal_simulator_native.h"
#include "gf_hal_common.h"
#include "gf_hal_dump.h"
#include "gf_hal_log.h"
#include "gf_hal_device.h"
#include "gf_hal.h"
#include "gf_queue.h"
#include "gf_hal_simulator_dump.h"


#ifdef __SUPPORT_GF_MEMMGR
#include "gf_memmgr.h"
#include "gf_hal_mem.h"
#endif  // __SUPPORT_GF_MEMMGR

#define LOG_TAG "[GF_HAL][gf_hal_simulator_native]"

#define GF_SIMULATOR_TEST_FILE "/data/gf_data/test"
#define GF_SIMULATOR_TEST_QUICK_UP_FILE "/data/gf_data/test_quick_up"
#define GF_SIMULATOR_TEST_MISTAKE_TOUCH_FILE "/data/gf_data/test_mistake_touch"
#define GF_DEV_DIR "/data/gf_data"
#define GF_DEV_NAME "goodix_fp"
#define GF_READ_DIRTY_DATA "read_dirty_data"


#define REPLY_OK 1
#define REPLY_ERROR (-1)

typedef void (*dump_notify)(const gf_fingerprint_dump_msg_t *msg);
dump_notify g_old_dump_notify = NULL;  // dump notify callback

static int32_t g_wait_retry_times = 10;  // wait retry times
static int32_t g_pipe = -1;  // pipe

uint32_t g_group_id = 0;  // simulate group_id

/*
Function: wait_dev_file_removed
Description: wait for file removed
Input: file path
Output:
Return: none
Others:
*/
static void wait_dev_file_removed(const char *path)
{
    int32_t retry = g_wait_retry_times;

    VOID_FUNC_ENTER();

    while (access(path, F_OK) == 0 && retry > 0)
    {
        usleep(10 * 1000);
        retry--;
    }

    VOID_FUNC_EXIT();
}

/*
Function: write_int
Description: write data to file in path
Input: file path, value which will be wrote
Output:
Return: error code
Others:
*/
static int32_t write_int(const char* path, int32_t value)
{
    int32_t ret = 0;
    FILE* file = NULL;
    if (access(path, F_OK) == 0)
    {
        wait_dev_file_removed(path);
    }

    file = fopen(path, "wb");
    if (file <= 0)
    {
        LOG_E(LOG_TAG, "[%s] Open file %s error = %s", __func__, path, strerror(errno));
        ret = -1;
    }
    else
    {
        fwrite(&value, sizeof(int32_t), 1, file);
        fflush(file);
    }
    fclose(file);
    return ret;
}

/*
Function: write_reply
Description: write reply data to pipe
Input: err, reply
Output:
Return: none
Others:
*/
static void write_reply(int32_t err,  gf_simulator_reply_t* reply)
{
    reply->err_code = err == GF_SUCCESS ? REPLY_OK : REPLY_ERROR;
    if (write(g_pipe, reply, sizeof(gf_simulator_reply_t)) <= 0)
    {
        LOG_E(LOG_TAG, "[%s] write pipe error: %s", __func__, strerror(errno));
    }
}

/*
Function: on_error
Description: write reply message by socket
Input: error message
Output:
Return: none
Others:
*/
static void on_error(const gf_fingerprint_msg_t *msg)
{
    gf_simulator_reply_t reply;
    VOID_FUNC_ENTER();
    UNUSED_VAR(msg);
    write_reply(GF_ERROR_GENERIC, &reply);
    VOID_FUNC_EXIT();
}

/*
Function: on_acquired
Description: write reply data according to acquire message
Input: message
Output:
Return: none
Others:
*/
static void on_acquired(const gf_fingerprint_msg_t *msg)
{
    VOID_FUNC_ENTER();
    gf_fingerprint_acquired_info_t info = msg->data.acquired.acquired_info;
    switch (info)
    {
        case GF_FINGERPRINT_ACQUIRED_PARTIAL:
        case GF_FINGERPRINT_ACQUIRED_INSUFFICIENT:
        case GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY:
        case GF_FINGERPRINT_ACQUIRED_TOO_SLOW:
        case GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA:
        {
            gf_simulator_reply_t reply;
            write_reply(msg->data.acquired.acquired_info,  &reply);
            break;
        }

        case GF_FINGERPRINT_ACQUIRED_TOO_FAST:
        case GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE:
        case GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT:
        {
            gf_simulator_reply_t reply;
            reply.acquired.acquired_info = msg->data.acquired.acquired_info;
            write_reply(GF_SUCCESS, &reply);
            break;
        }

        default:
        {
            break;
        }
    }  // end switch
    VOID_FUNC_EXIT();
}

/*
Function: on_enroll
Description: write reply data when enroll finished
Input: message
Output:
Return: none
Others:
*/
static void on_enroll(const gf_fingerprint_msg_t *msg)
{
    VOID_FUNC_ENTER();
    gf_simulator_reply_t reply;

    LOG_D(LOG_TAG, "[%s] on_enroll(fid=%u, gid=%u, remaining=%u)", __func__,
            msg->data.enroll.finger.fid, msg->data.enroll.finger.gid,
            msg->data.enroll.samples_remaining);
    memcpy(&reply.enroll,  &(msg->data.enroll), sizeof(msg->data.enroll));
    write_reply(GF_SUCCESS, &reply);
    VOID_FUNC_EXIT();
}

/*
Function: on_authenticated
Description: write reply data when authenticate finished
Input: message
Output:
Return: none
Others:
*/
static void on_authenticated(const gf_fingerprint_msg_t *msg)
{
    gf_simulator_reply_t reply;
    VOID_FUNC_ENTER();

    memcpy(&(reply.authenticated), &(msg->data.authenticated), sizeof(msg->data.authenticated));
    write_reply(GF_SUCCESS, &reply);

    VOID_FUNC_EXIT();
}

/*
Function: on_authenticated_fido
Description: write reply data when authenticate fido finished
Input: message
Output:
Return: none
Others:
*/
static void on_authenticated_fido(const gf_fingerprint_msg_t *msg)
{
    gf_simulator_reply_t reply;
    gf_error_t err = GF_SUCCESS;
    VOID_FUNC_ENTER();

    memcpy(&(reply.authenticated_fido), &(msg->data.authenticated_fido), sizeof(msg->data.authenticated_fido));
    write_reply(err, &reply);

    VOID_FUNC_EXIT();
}

/*
Function: on_enumerate
Description: write reply data when enumerate finished
Input: message
Output:
Return: none
Others:
*/
static void on_enumerate(const gf_fingerprint_msg_t *msg)
{
    VOID_FUNC_ENTER();

    gf_simulator_reply_t reply;

    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u, remains=%u",
            __func__, msg->data.enumerated.finger.gid, msg->data.enumerated.finger.fid,
            msg->data.enumerated.remaining_templates);

    memcpy(&reply.enumerated_with_callback,  &(msg->data.enumerated), sizeof(msg->data.enumerated));

    write_reply(GF_SUCCESS, &reply);

    VOID_FUNC_EXIT();
}

/*
Function: on_remove
Description: write reply data when remove finger finished
Input: message
Output:
Return: none
Others:
*/
static void on_remove(const gf_fingerprint_msg_t *msg)
{
    VOID_FUNC_ENTER();
    gf_simulator_reply_t reply;

    LOG_D(LOG_TAG, "[%s] remove finger. group_id=%u, finger_id=%u, remains=%u",
            __func__, msg->data.removed.finger.gid, msg->data.removed.finger.fid,
            msg->data.removed.remaining_templates);

    memcpy(&reply.removed,  &(msg->data.removed), sizeof(msg->data.removed));

    write_reply(GF_SUCCESS, &reply);

    VOID_FUNC_EXIT();
}

/*
Function: notify_for_simulate
Description: deal all callback notify according to the type of message
Input: message
Output:
Return: none
Others:
*/
void notify_for_simulate(const gf_fingerprint_msg_t *msg)
{
    VOID_FUNC_ENTER();

    if (NULL == msg)
    {
        LOG_E(LOG_TAG, "[%s] msg is null", __func__);
        return;
    }

    switch (msg->type)
    {
        case GF_FINGERPRINT_ERROR:
        {
            on_error(msg);
            break;
        }

        case GF_FINGERPRINT_ACQUIRED:
        {
            on_acquired(msg);
            break;
        }

        case GF_FINGERPRINT_AUTHENTICATED:
        {
            on_authenticated(msg);
            break;
        }

        case GF_FINGERPRINT_AUTHENTICATED_FIDO:
        {
            on_authenticated_fido(msg);
            break;
        }

        case GF_FINGERPRINT_TEMPLATE_ENROLLING:
        {
            on_enroll(msg);
            break;
        }

        case GF_FINGERPRINT_TEMPLATE_REMOVED:
        {
            on_remove(msg);
            break;
        }
        case GF_FINGERPRINT_ENUMERATED:
        {
            on_enumerate(msg);
            break;
        }
        default:
        {
            break;
        }
    }  // end switch
    VOID_FUNC_EXIT();
}

/*
Function: gf_simulator_set_channel
Description: init g_pipe
Input: pipe_write
Output:
Return: none
Others:
*/
void gf_simulator_set_channel(int32_t pipe_write)
{
    g_pipe = pipe_write;
}

/*
Function: gf_simulator_enroll
Description: send enroll command
Input: auth token in data
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_enroll(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = g_group_id;
    gf_simulator_reply_t reply;
    FUNC_ENTER();

    if (NULL == data)
    {
        LOG_E(LOG_TAG, "[%s] data is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        gf_hw_auth_token_t* auth_token = (gf_hw_auth_token_t*)data;
        uint32_t timeout = *((uint32_t*)((uint8_t*)data + sizeof(gf_hw_auth_token_t)));
        err = gf_hal_enroll(g_fingerprint_device, auth_token, group_id, timeout);
    }

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_authenticate
Description: send authenticate command
Input: authenticate data
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_authenticate(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = g_group_id;
    gf_simulator_reply_t reply;
    FUNC_ENTER();
    UNUSED_VAR(data);
    err = gf_hal_authenticate(g_fingerprint_device, 0, group_id);

    write_reply(err, &reply);
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_authenticate_fido
Description: send authenticate fido command
Input: authenticate fido data
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_authenticate_fido(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = g_group_id;
    gf_simulator_reply_t reply;
    FUNC_ENTER();

    uint32_t aaid_len;
    uint32_t challenge_len;
    uint8_t *tmp = (uint8_t*)data;

    if (NULL == data)
    {
        LOG_E(LOG_TAG, "[%s] data is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        memcpy(&aaid_len, tmp, sizeof(uint32_t));
        uint8_t aaid[1024];
        tmp += sizeof(uint32_t);
        memcpy(aaid, tmp, aaid_len);
        tmp += aaid_len;
        memcpy(&challenge_len, tmp, sizeof(uint32_t));
        uint8_t challenge[1024];
        tmp += sizeof(uint32_t);
        memcpy(challenge, tmp, challenge_len);
        err = gf_hal_authenticate_fido(g_fingerprint_device, group_id, aaid, aaid_len, challenge, challenge_len);
    }

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_remove_finger
Description: send remove the finger command
Input: finger id and group id in data
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_remove_finger(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = g_group_id;
    uint32_t finger_id = 0;
    gf_simulator_reply_t reply;

    FUNC_ENTER();

    if (NULL == data)
    {
        LOG_E(LOG_TAG, "[%s] data is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        finger_id = *data;
        err = gf_hal_common_remove_without_callback(group_id, finger_id);
    }

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return  err;
}

/*
Function: gf_simulator_invoke_irq
Description: simulator down, up irq
Input: irq type in data
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_invoke_irq(int32_t* data)
{
    FILE *file = NULL;
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    VOID_FUNC_ENTER();
    do
    {
        char filepath[256] = { 0 };
        int32_t irq_type = 0;

        if (NULL == data)
        {
            LOG_E(LOG_TAG, "[%s] data is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        switch (*data)
        {
            case SIMULATOR_FINGER_DOWN:
            {
                irq_type = GF_IRQ_FINGER_DOWN_MASK;
                break;
            }
            case SIMULATOR_FINGER_UP:
            {
                irq_type = GF_IRQ_FINGER_UP_MASK;
                break;
            }
            default:
            {
                err = GF_ERROR_GENERIC;
                LOG_E("[%s] mock irq type not supported.", __func__);
                break;
            }
        }
        if (err != GF_SUCCESS)
        {
            break;
        }
        snprintf(filepath, sizeof(filepath), "%s/%s", GF_DEV_DIR, GF_DEV_NAME);
        if (write_int(filepath, irq_type) >= 0)
        {
            err = gf_enqueue(GF_NETLINK_IRQ);
            if (err  == GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] Send message value=%u", __func__, irq_type);
            }
        }
    }  // end do
    while (0);

    if (file != NULL)
    {
        fclose(file);
    }

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_set_nav_oritension
Description: set navigation oritension
Input: navigation data
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_set_nav_oritension(int32_t *data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    FUNC_ENTER();

    if (NULL == data)
    {
        LOG_E(LOG_TAG, "[%s] data is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        write_int(GF_SIMULATOR_TEST_FILE, *data);
    }

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_enumerate
Description: get current finger list
Input: none
Output: none
Return: error code
Others:
*/
gf_error_t gf_simulator_enumerate(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    int32_t i = 0;
    uint32_t fid_max_size = 5;
    uint32_t results[10] = {0};
    gf_simulator_reply_t reply;
    UNUSED_VAR(data);

    err = gf_hal_enumerate(g_fingerprint_device, &results, &fid_max_size);

    reply.enumerated.size = fid_max_size;

    gf_fingerprint_finger_id_t *finger_ids = (gf_fingerprint_finger_id_t *) results;

    if (fid_max_size)
    {
        for (; i < fid_max_size; i++)
        {
            reply.enumerated.finger_ids[i] = finger_ids[i].fid;
            reply.enumerated.group_ids[i] = finger_ids[i].gid;
        }
    }

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_set_active_group
Description: set active group
Input: groupid
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_set_active_group(int32_t* groupid)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = 0;
    gf_simulator_reply_t reply;
    FUNC_ENTER();

    do
    {
        if (NULL == groupid)
        {
            LOG_E(LOG_TAG, "[%s] groupid is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        group_id = *groupid;
        err = gf_hal_set_active_group(g_fingerprint_device, group_id);
        g_group_id = group_id;
        LOG_D(LOG_TAG, "[%s] simulate set active group done, g_group_id = %u\n", __func__, g_group_id);
    }
    while (0);

    write_reply(err, &reply);
    FUNC_EXIT(err);
    return  err;
}

/*
Function: gf_simulator_get_auth_id
Description: get g_authenticate_id
Input: none
Output: g_authenticate_id
Return: error code
Others:
*/
gf_error_t gf_simulator_get_auth_id(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint64_t authenticator_id = 0;
    gf_simulator_reply_t reply;
    UNUSED_VAR(data);
    FUNC_ENTER();

    authenticator_id = gf_hal_get_auth_id(g_fingerprint_device);
    reply.authenticator_id.auth_id = authenticator_id;

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_enumerate_with_callback
Description: get current finger list with callback
Input: none
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_enumerate_with_callback(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    UNUSED_VAR(data);
    FUNC_ENTER();

    write_reply(GF_SUCCESS, &reply);
    err = gf_hal_common_enumerate_with_callback(g_fingerprint_device);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_remove_with_callback
Description: remove finger and accept callback
Input: finger id and group id in data
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_remove_with_callback(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = g_group_id;
    uint32_t finger_id = 0;
    gf_simulator_reply_t reply;
    FUNC_ENTER();

    if (NULL == data)
    {
        LOG_E(LOG_TAG, "[%s] data is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        finger_id = *data;
        write_reply(GF_SUCCESS, &reply);
        err = gf_hal_remove(g_fingerprint_device, group_id, finger_id);
    }

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_check_memory
Description: check memory leak
Input: none
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_check_memory(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    gf_memory_check_t cmd;
    FUNC_ENTER();
    UNUSED_VAR(cmd);
    UNUSED_VAR(data);
    memset(&reply, 0, sizeof(gf_simulator_reply_t));
#ifdef __SUPPORT_GF_MEMMGR
    uint32_t leak_count = gf_memmgr_dump_leak_info();
    reply.memory_info.ca_leak_count = leak_count;
    err = gf_hal_invoke_command(GF_CMD_TEST_MEMORY_CHECK, &cmd, sizeof(cmd));
    if (err == GF_SUCCESS)
    {
        reply.memory_info.ta_leak_count = cmd.memory_info.ta_leak_count;
    }
#endif  // __SUPPORT_GF_MEMMGR
    write_reply(err, &reply);
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_clear
Description: send cancel command to clear environment
Input: none
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_clear(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    UNUSED_VAR(data);
    FUNC_ENTER();

    uint32_t size = sizeof(gf_cancel_t);
    gf_cancel_t cmd;

    err = gf_hal_invoke_command(GF_CMD_CANCEL, &cmd, size);

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_finger_quick_up_or_mistake_touch
Description: simulator finger quick up and mistake touch
Input: none
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_finger_quick_up_or_mistake_touch(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t group_id = g_group_id;
    gf_simulator_reply_t reply;
    FUNC_ENTER();

    if (NULL == data)
    {
        LOG_E(LOG_TAG, "[%s] data is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
        return err;
    }

    int32_t is_set_finger_quick_up = *data;
    data++;
    int32_t retry_count_for_mistake_touch = *data;
    if (is_set_finger_quick_up)
    {
        write_int(GF_SIMULATOR_TEST_QUICK_UP_FILE, is_set_finger_quick_up);
    }
    else if (retry_count_for_mistake_touch)
    {
        write_int(GF_SIMULATOR_TEST_FILE, retry_count_for_mistake_touch);
        write_int(GF_SIMULATOR_TEST_MISTAKE_TOUCH_FILE, is_set_finger_quick_up);
    }

    err = gf_hal_authenticate(g_fingerprint_device, 0, group_id);

    write_reply(err, &reply);
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_pre_enroll
Description: send pre enroll command
Input: none
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_pre_enroll(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    UNUSED_VAR(data);
    FUNC_ENTER();

    uint64_t challenge = gf_hal_pre_enroll(g_fingerprint_device);
    reply.pre_enroll.challenge = challenge;
    if (challenge == 0)
    {
        err = GF_ERROR_GENERIC;
    }
    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_post_enroll
Description: send post enroll command
Input: none
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_post_enroll(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    UNUSED_VAR(data);
    FUNC_ENTER();

    err = gf_hal_post_enroll(g_fingerprint_device);

    write_reply(err, &reply);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_post_enroll
Description: send cancel command
Input: none
Output:
Return: error code
Others:
*/
gf_error_t gf_simulator_cancel(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    UNUSED_VAR(data);
    FUNC_ENTER();
    write_reply(err, &reply);

    err = gf_hal_cancel(g_fingerprint_device);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_simulator_set_dump_config
Description: set dump config
Input: dump config
Output:
Return: none
Others:
*/
void gf_simulator_set_dump_config(const gf_dump_config_t* dump_cfg)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;

    if (NULL == dump_cfg)
    {
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        memcpy(&g_dump_config, dump_cfg, sizeof(gf_dump_config_t));
        LOG_D(LOG_TAG, "dump config:dump_version<%s>, dump_bigdata_version<%s>"
                        "dump_enabled<%d>, encrypt_enabled<%d>, bigdata_enabled<%d>, dump_path<%d>",
                        g_dump_config.dump_version, g_dump_config.dump_bigdata_version,
                        g_dump_config.dump_enabled, g_dump_config.dump_encrypt_enabled,
                        g_dump_config.dump_big_data_enabled, g_dump_config.dump_path);
    }

    write_reply(err, &reply);
    return;
}

/*
Function: on_dump
Description: write reply dump data
Input: dump message
Output:
Return: none
Others:
*/
static void on_dump(const gf_fingerprint_dump_msg_t *msg)
{
    VOID_FUNC_ENTER();
    gf_simulator_reply_t reply;
    LOG_D(LOG_TAG, "[%s] notify dump msg", __func__);
    memcpy(&reply.dump_buf,  msg->data, sizeof(dump_data_buffer_header_t));
    write_reply(GF_SUCCESS, &reply);
    VOID_FUNC_EXIT();
}

/*
Function: setup_dump_notify
Description: set g_fingerprint_device->dump_notify and g_old_dump_notify
Input:
Output:
Return: none
Others:
*/
static void setup_dump_notify()
{
    VOID_FUNC_ENTER();
    if (on_dump == g_fingerprint_device->dump_notify)
    {
        return;
    }
    g_old_dump_notify = g_fingerprint_device->dump_notify;
    g_fingerprint_device->dump_notify = on_dump;
    VOID_FUNC_EXIT();
}

/*
Function: cancel_dump_notify
Description: recovery g_fingerprint_device->dump_notify value
Input: none
Output: none
Return: none
Others:
*/
static void cancel_dump_notify()
{
    VOID_FUNC_ENTER();
    if (on_dump == g_fingerprint_device->dump_notify)
    {
        g_fingerprint_device->dump_notify = g_old_dump_notify;
        g_old_dump_notify = NULL;
    }
    VOID_FUNC_EXIT();
}

/*
Function: gf_simulator_dump_cmd
Description: send dump command
Input: command data
Output: none
Return: none
Others:
*/
void gf_simulator_dump_cmd(int32_t* cmd_data)
{
    VOID_FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    if (NULL == cmd_data)
    {
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        int32_t cmd = *cmd_data;
        uint8_t* data = NULL;
        uint32_t param_len = 0;

        LOG_D(LOG_TAG, "[%s] cmd=%d", __func__, cmd);
        if (CMD_SET_DUMP_PATH == cmd)
        {
            cmd_data++;
            param_len = *(uint32_t*)cmd_data;
            cmd_data++;
            data = (uint8_t*)cmd_data;
            int32_t path = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;
            if (path == DUMP_PATH_DATA)
            {
                cancel_dump_notify();
            }
            else if (path == DUMP_PATH_SDCARD)
            {
                setup_dump_notify();
            }
            else
            {
                err = GF_ERROR_BAD_PARAMS;
                return;
            }
        }
        err = gf_hal_dump_cmd(NULL, cmd, data, param_len);
    }  // end else

    write_reply(err, &reply);
    VOID_FUNC_EXIT();
    return;
}

/*
Function: gf_simulator_reset_apk_enable_dump_flag
Description: reset g_apk_enabled_dump
Input: none
Output: none
Return: none
Others:
*/
void gf_simulator_reset_apk_enable_dump_flag()
{
    g_apk_enabled_dump = -1;
    gf_simulator_reply_t reply;
    write_reply(GF_SUCCESS, &reply);
    return;
}

/*
Function: gf_simulator_set_simulate_dirty_data_flag
Description: simulate dirty date
Input: dirty data in data
Output: none
Return: none
Others:
*/
void gf_simulator_set_simulate_dirty_data_flag(int32_t* data)
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    int32_t dirty = 0;
    VOID_FUNC_ENTER();
    do
    {
        char filepath[256] = { 0 };
        snprintf(filepath, sizeof(filepath), "%s/%s", GF_DEV_DIR, GF_READ_DIRTY_DATA);
        if (data != NULL)
        {
            dirty = *data;
        }
        if (write_int(filepath, dirty ? 1 : 0) < 0)
        {
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            LOG_D(LOG_TAG, "[%s] failed", __func__);
        }
    }
    while (0);

    write_reply(err, &reply);
    VOID_FUNC_EXIT();
    return;
}

/*
Function: gf_simulator_dump_device_info
Description: send dump device info command
Input: none
Output: none
Return: none
Others:
*/
void gf_simulator_dump_device_info()
{
    VOID_FUNC_ENTER();

    hal_fps_dump_chip_init_data();
    gf_simulator_reply_t reply;
    write_reply(GF_SUCCESS, &reply);

    VOID_FUNC_EXIT();
    return;
}

/*
Function: gf_simulator_start_navigate
Description: send start navigate command
Input: none
Output: none
Return: none
Others:
*/
void gf_simulator_start_navigate()
{
    gf_error_t err = GF_SUCCESS;
    gf_simulator_reply_t reply;
    VOID_FUNC_ENTER();
    err = g_hal_function.navigate(g_fingerprint_device, GF_NAV_MODE_XY);
    write_reply(err, &reply);
    VOID_FUNC_EXIT();
    return;
}

