/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_hal memory pool adapter file
 * History:
 * Version: 1.0
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include "gf_tee_storage.h"
#include "gf_common.h"
#include "gf_hal_log.h"
#include "gf_memmgr.h"

#define LOG_TAG "[GF_HAL][gf_mempool_adapter]"

#define  FILE_PATH  "/data/"
#define  PATH_LENGTH  1024
static char g_path[PATH_LENGTH];  // global path

/**
 * Function: cpl_sprintf
 * Description: write formated string to buffer
 * Input: buffer, format, ...
 * Output: none
 * Return: buffer size, -1 means fail
 */
int32_t cpl_sprintf(int8_t *buffer, const int8_t *format, ...)
{
    int32_t ret = 0;

    if (NULL == buffer || NULL == format)
    {
        LOG_E(LOG_TAG, "[%s] buffer or format is null", __func__);
        ret = -1;
        return ret;
    }

    va_list args;
    va_start(args, format);
    ret = vsnprintf((char*) buffer, INT_MAX, (char*) format, args);
    va_end(args);
    return ret;
}

/**
 * Function: cpl_strlen
 * Description: get length of string
 * Input: str
 * Output: none
 * Return: length of string
 */
uint32_t cpl_strlen(const int8_t *str)
{
    if (NULL == str)
    {
        LOG_E(LOG_TAG, "[%s] str is null", __func__);
        return 0;
    }

    return strlen((const char *)str);
}

/**
 * Function: gf_tee_open_object
 * Description: open tee object
 * Input: object_id, mode, so_handle
 * Output: none
 * Return:
 * GF_SUCCESS  if succeed
 * others      if fail
 */
gf_error_t gf_tee_open_object(uint8_t *object_id, gf_tee_mode_t mode,
                              GF_TEE_HANDLE *so_handle)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        FILE *handle = NULL;
        uint8_t file_flags[5];
        gf_tee_mode_t tmp;

        if ((NULL == object_id) || (NULL == so_handle)
            || ((SO_MODE_READ != mode) && (SO_MODE_WRITE != mode)))
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, object_id=%p, mode=%d, so_handle=%p",
                    __func__, (void *)object_id, mode, (void *)so_handle);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        memset(file_flags, 0, 5);
        tmp = mode & SO_MODE_WRITE;

        if (tmp != 0)
        {
            snprintf((char*)file_flags, strlen("wb+"), "wb+");
        }
        else
        {
            snprintf((char*)file_flags, strlen("rb"), "rb");
        }

        memset(g_path, 0, PATH_LENGTH);
        strncpy(g_path, FILE_PATH, strlen(FILE_PATH) + 1);
        strncat(g_path, (char *) object_id, strlen((char *) object_id) + 1);
        // we will not fix this gclint detected error:
        // File pointer handle is not closed.
        // In one function every fopen need a fclose to match,so that the open file be closed safety!
        // since it is closed
        handle = fopen((const char *)g_path, (char *) file_flags);
        if (NULL != handle)
        {
            *so_handle = (void *)handle;
        }
        else
        {
            err = GF_ERROR_OPEN_SECURE_OBJECT_FAILED;
            break;
        }
    } while (0);  // do...

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_tee_get_object_size
 * Description: get tee object size
 * Input: so_handle, object_size
 * Output: object_size
 * Return:
 * GF_SUCCESS  if succeed
 * others      if fail
 */
gf_error_t gf_tee_get_object_size(GF_TEE_HANDLE so_handle,
                                  uint32_t *object_size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        uint32_t len = 0;

        if ((NULL == so_handle) || (NULL == object_size))
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, so_handle=%p, object_size=%p",
                    __func__, (void *)so_handle, (void *)object_size);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        FILE *file = (FILE *)so_handle;
        fseek(file, 0, SEEK_END);
        // we will not fix this gclint detected error:
        // Use int16/int32/int64/etc, rather than the C type long
        // since the bit width for type long is not clear in this scene
        len = (uint32_t)ftell(file);
        fseek(file, 0, SEEK_SET);

        if (len == 0)
        {
            err = GF_ERROR_READ_SECURE_OBJECT_FAILED;
            break;
        }

        *object_size = len;
    } while (0);  // do...

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_tee_read_object_data
 * Description: get tee object data
 * Input: so_handle, buffer, size
 * Output: buffer
 * Return:
 * GF_SUCCESS  if succeed
 * others      if fail
 */
gf_error_t gf_tee_read_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer,
                                   uint32_t size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        uint32_t read_bytes = 0;
        if ((NULL == so_handle) || (NULL == buffer))
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, so_handle=%p, buffer=%p",
                    __func__, (void *)so_handle, (void *)buffer);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        FILE *file = (FILE *)so_handle;
        read_bytes = fread((char *) buffer, 1, (int32_t) size, file);
        if (read_bytes != size)
        {
            LOG_E(LOG_TAG, "[%s] read so failed, size=%u, read_bytes=%d",
                    __func__, size, read_bytes);
            err = GF_ERROR_READ_SECURE_OBJECT_FAILED;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_tee_write_object_data
 * Description: write data to tee object
 * Input: so_handle, buffer, size
 * Output: none
 * Return:
 * GF_SUCCESS  if succeed
 * others      if fail
 */
gf_error_t gf_tee_write_object_data(GF_TEE_HANDLE so_handle, uint8_t *buffer,
                                    uint32_t size)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        int32_t reval = 0;

        if ((NULL == so_handle) || (NULL == buffer) || (size == 0))
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, so_handle=%p, buffer=%p, size=%u",
                    __func__, (void *)so_handle, (void *)buffer, size);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        FILE *file = (FILE *)so_handle;

        reval = fwrite((const char *) buffer, 1, (int32_t) size, file);

        if (0 > reval)
        {
            LOG_E(LOG_TAG, "[%s] write so failed", __func__);
            err = GF_ERROR_WRITE_SECURE_OBJECT_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_tee_delete_object
 * Description: delete tee object
 * Input: object_id
 * Output: none
 * Return:
 * GF_SUCCESS  if succeed
 * others      if fail
 */
gf_error_t gf_tee_delete_object(uint8_t *object_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        int32_t ret = 0;

        if (NULL == object_id)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, object_id=%p", __func__,
                    (void *)object_id);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        memset(g_path, 0, PATH_LENGTH);
        strncpy((char*)g_path, FILE_PATH, strlen(FILE_PATH) + 1);
        strncat((char*)g_path, (char *) object_id, strlen((char *) object_id) + 1);
        ret = unlink((const char *) g_path);

        if (ret != 0)
        {
            err = GF_ERROR_WRITE_SECURE_OBJECT_FAILED;
            break;
        }

        err = GF_SUCCESS;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_tee_close_object
 * Description: close tee object
 * Input: so_handle
 * Output: none
 * Return: none
 */
void gf_tee_close_object(GF_TEE_HANDLE so_handle)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == so_handle)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter", __func__);
            break;
        }

        fclose((FILE *) so_handle);
    }
    while (0);

    VOID_FUNC_EXIT();
}


