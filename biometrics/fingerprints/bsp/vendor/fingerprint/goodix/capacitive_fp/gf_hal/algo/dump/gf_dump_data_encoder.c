/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: Dump data encoder is a memory buffer which is filled with all dump data for
 *              dump operation. Dump files are created an wrote after encoder buffer fill
 *              completed. See dump_data_buffer_header_t in gf_dump_data_buffer.h for buffer
 *              data structure.
 * History:
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <cutils/fs.h>
#include <string.h>

#include "gf_dump_data_encoder.h"
#include "gf_common.h"
#include "gf_hal_log.h"
#include "gf_dump_data.h"
#include "gf_dump_data_utils.h"

#define LOG_TAG "[GF_HAL][gf_dump_encoder]"

// limit buffer size less than 10m
#define GF_DUMP_BUF_MAX_SIZE (10*1024*1024)
// 400KB
#define GF_DUMP_BUF_GROWTH_SIZE (400*1024)
// 530280 bytes + 409600, dump data + file path + added string
#define GF_DUMP_BUF_INIT_SIZE ((sizeof(gf_dump_data_t)) + (GF_DUMP_BUF_GROWTH_SIZE))

/**
 *Function: gf_dump_encoder_create
 *Description: create encoder buffer
 *Input: encoder, encoder pointer
 *       time_stamp, time stamp used to generate encrypted file name
 *Output: encoder buffer memory
 *Return: gf_error_t
 */
gf_error_t gf_dump_encoder_create(dump_data_encoder_t** encoder,
                                int64_t time_stamp)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_encoder_t* tmp = NULL;
    FUNC_ENTER();

    do
    {
        if (NULL == encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        tmp = (dump_data_encoder_t*) malloc(sizeof(dump_data_encoder_t));
        if (NULL == tmp)
        {
            LOG_E(LOG_TAG, "[%s] allocate memory fail, size<%ud>", __func__, (uint32_t)GF_DUMP_BUF_INIT_SIZE);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(tmp, 0, sizeof(dump_data_encoder_t));
        // malloc dump data buffer
        tmp->buf = (uint8_t*) malloc(GF_DUMP_BUF_INIT_SIZE);
        if (NULL == tmp->buf)
        {
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(tmp->buf, 0, GF_DUMP_BUF_INIT_SIZE);
        tmp->buf_len = GF_DUMP_BUF_INIT_SIZE;

        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) tmp->buf;
        buf->encrypt_info = GF_DUMP_NOT_ENCRYPTED;
        buf->ts_low = time_stamp & 0xFFFFFFFF;
        buf->ts_high = (time_stamp >> 32) & 0xFFFFFFFF;
        buf->buf_len = sizeof(dump_data_buffer_header_t);
        buf->sensor_width = gf_get_sensor_width();
        buf->sensor_height = gf_get_sensor_height();
        buf->nav_width = gf_get_nav_width();
        buf->nav_height = gf_get_nav_height();
        tmp->cur_write_offset = sizeof(dump_data_buffer_header_t);

        *encoder = tmp;
    }  // do...
    while (0);

    if (err != GF_SUCCESS)
    {
        gf_dump_encoder_destroy(tmp);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: gf_dump_encoder_destroy
 *Description: destroy encoder and free buffer
 *Input: encoder, encoder to be destroied
 *Output: none
 *Return: none
 */
void gf_dump_encoder_destroy(dump_data_encoder_t* encoder)
{
    if (NULL == encoder)
    {
        return;
    }
    if (encoder->buf != NULL)
    {
        free(encoder->buf);
        encoder->buf = NULL;
    }
    free(encoder);
    return;
}

/**
 *Function: gf_dump_encoder_reset
 *Description: reset encoder
 *Input: encoder, encoder to be reset
 *Output: none
 *Return: none
 */
void gf_dump_encoder_reset(dump_data_encoder_t* encoder)
{
    struct timeval tv = { 0 };
    int64_t time_stamp = 0;
    if (NULL == encoder)
    {
        return;
    }
    if (encoder->buf != NULL && encoder->buf_len > 0)
    {
        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) encoder->buf;
        memset(buf->data_file, 0, encoder->buf_len - sizeof(dump_data_buffer_header_t));
        // get time stamp again to avoid duplicate file name of encrypted dat file
        gettimeofday(&tv, NULL);
        time_stamp = gf_get_time_stamp(&tv);

        buf->encrypt_info = GF_DUMP_NOT_ENCRYPTED;
        buf->ts_low = time_stamp & 0xFFFFFFFF;
        buf->ts_high = (time_stamp >> 32) & 0xFFFFFFFF;
        buf->buf_len = sizeof(dump_data_buffer_header_t);
        encoder->cur_write_offset = sizeof(dump_data_buffer_header_t);
        encoder->cur_file_offset = 0;
    }
    return;
}

/**
 *Function: check_offset
 *Description: check if offset is correct
 *Input: encoder
 *Output: none
 *Return: gf_error_t
 */
gf_error_t check_offset(dump_data_encoder_t* encoder)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_t* file = NULL;

    do
    {
        if (NULL == encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (0 == encoder->cur_file_offset)
        {
            break;
        }
        file = (dump_data_t*)(encoder->buf + encoder->cur_file_offset);
        if (encoder->cur_write_offset != encoder->cur_file_offset
            + sizeof(dump_data_t) + file->path_count * GF_DUMP_FILE_PATH_MAX_LEN + file->data_size)
        {
            LOG_E(LOG_TAG, "[%s] offset incorrect!, write_offset<%u>,file_offset<%u>,"
                "path_count<%d>,data_size<%u>",
                __func__, encoder->cur_write_offset, encoder->cur_file_offset,
                file->path_count, file->data_size);
            err = GF_ERROR_GENERIC;
            break;
        }
    }
    while (0);

    return err;
}

/**
 *Function: check_buf_size
 *Description: check if encoder has enough space to write the given length data
 *Input: encoder, encoder to be check
 *       supposed_write_len, supposed write length
 *Output: none
 *Return: gf_error_t
 */
gf_error_t check_buf_size(dump_data_encoder_t* encoder, uint32_t supposed_write_len)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t* tmp = NULL;

    do
    {
        if (NULL == encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (encoder->cur_write_offset + supposed_write_len >= encoder->buf_len)
        {
            uint32_t growth_size = supposed_write_len > GF_DUMP_BUF_GROWTH_SIZE ?
                                    supposed_write_len : GF_DUMP_BUF_GROWTH_SIZE;
            LOG_D(LOG_TAG, "[%s] dump buf size not enough, current size=%d, growth size=%d, ",
                    __func__, encoder->buf_len, growth_size);
            if (growth_size + encoder->buf_len > GF_DUMP_BUF_MAX_SIZE)
            {
                LOG_E(LOG_TAG, "[%s] buffer grows larger than max limit 5m, return",
                        __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            tmp = (uint8_t*)malloc(encoder->buf_len + growth_size);
            if (NULL == tmp)
            {
                LOG_E(LOG_TAG, "[%s] grow dump buffer failed, current size=%d, growth size=%d",
                        __func__, encoder->buf_len, growth_size);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            memset(tmp, 0, encoder->buf_len + growth_size);
            memcpy(tmp, encoder->buf, encoder->cur_write_offset);
            free(encoder->buf);
            encoder->buf = tmp;
            encoder->buf_len += growth_size;
        }  // if (encoder->cur_write_offset + supposed_write_len >= encoder->buf_len)
    }  // do...
    while (0);

    return err;
}

/**
 *Function: gf_dump_encoder_fopen
 *Description: like fopen(), encoder prepares to write file data
 *Input: encoder, encoder to be written
 *       file_path, file name
 *       data_type, data type for the file to be written
 *Output: none
 *Return: gf_error_t
 */
gf_error_t gf_dump_encoder_fopen(dump_data_encoder_t* encoder,
                             const uint8_t* file_path,
                             data_type_t data_type)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_t* file = NULL;
    uint32_t data_offset = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == encoder || NULL == file_path)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (encoder->cur_file_offset != 0)
        {
            LOG_D(LOG_TAG, "[%s] we have file is being operated, close it",
                    __func__);
            gf_dump_encoder_fclose(encoder);
        }
        LOG_D(LOG_TAG, "[%s] open<%s>, typ<%d>, ",
                    __func__, file_path, (int32_t)data_type);

        err = check_buf_size(encoder, sizeof(dump_data_t) + GF_DUMP_FILE_PATH_MAX_LEN);
        if (GF_SUCCESS != err)
        {
            break;
        }

        file = (dump_data_t*)(encoder->buf + encoder->cur_write_offset);
        encoder->cur_file_offset = encoder->cur_write_offset;
        file->type = data_type;
        file->path_count = 1;
        file->data_size = 0;
        data_offset = encoder->cur_file_offset + sizeof(dump_data_t);
        snprintf((char*)(encoder->buf + data_offset), GF_DUMP_FILE_PATH_MAX_LEN, "%s", (char*)file_path);

        encoder->cur_write_offset += (sizeof(dump_data_t) + GF_DUMP_FILE_PATH_MAX_LEN);
        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) encoder->buf;
        buf->buf_len += (sizeof(dump_data_t) + GF_DUMP_FILE_PATH_MAX_LEN);
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: gf_dump_encoder_add_path
 *Description: Add a file path to current opened file if have multi path.
 *         Path can only be added before gf_dump_encoder_fwrite called
 *         otherwise return error.
 *Input: encoder, encoder to be written
 *       file_path, file path to add
 *Output: none
 *Return: gf_error_t
 */
gf_error_t gf_dump_encoder_add_path(dump_data_encoder_t* encoder, const uint8_t* file_path)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_t* file = NULL;
    uint32_t data_offset = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == encoder || NULL == file_path)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_E(LOG_TAG, "[%s] add path <%s>",
                    __func__, file_path);
        if (0 == encoder->cur_file_offset)
        {
            LOG_E(LOG_TAG, "[%s] no file opened, add path fail",
                    __func__);
            break;
        }

        file = (dump_data_t*)(encoder->buf + encoder->cur_file_offset);

        err = check_offset(encoder);
        if (GF_SUCCESS != err)
        {
            break;
        }

        if (file->data_size != 0)
        {
            LOG_E(LOG_TAG, "[%s] cannot add path<%s> since file is writting",
                    __func__, file_path);
            break;
        }

        err = check_buf_size(encoder, GF_DUMP_FILE_PATH_MAX_LEN);
        if (GF_SUCCESS != err)
        {
            break;
        }

        data_offset = encoder->cur_file_offset + sizeof(dump_data_t) +
            file->path_count * GF_DUMP_FILE_PATH_MAX_LEN;

        snprintf((char*)(encoder->buf + data_offset), GF_DUMP_FILE_PATH_MAX_LEN, "%s", (char*)file_path);
        file->path_count++;

        encoder->cur_write_offset += GF_DUMP_FILE_PATH_MAX_LEN;
        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) encoder->buf;
        buf->buf_len += GF_DUMP_FILE_PATH_MAX_LEN;
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: gf_dump_encoder_fdelete
 *Description: Delete current writting file.
 *Input: encoder, encoder to be written
 *Output: none
 *Return: gf_error_t
 */
gf_error_t gf_dump_encoder_fdelete(dump_data_encoder_t* encoder)
{
    gf_error_t err = GF_SUCCESS;
    uint32_t clear_size = 0;

    do
    {
        if (NULL == encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (0 == encoder->cur_file_offset)
        {
            break;
        }

        LOG_D(LOG_TAG, "[%s] delete file<%s>, offset<%d>",
                        __func__, encoder->buf + encoder->cur_file_offset + sizeof(dump_data_t),
                        encoder->cur_file_offset);
        err = check_offset(encoder);

        clear_size = encoder->cur_write_offset - encoder->cur_file_offset;
        memset(encoder->buf + encoder->cur_file_offset, 0, clear_size);
        encoder->cur_write_offset -= clear_size;
        encoder->cur_file_offset = 0;
        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) encoder->buf;
        buf->buf_len -= clear_size;
    }
    while (0);

    return err;
}

/**
 *Function: gf_dump_encoder_fprintf
 *Description: like fprintf(), write formated data into current operating file,
 *             can only write less than 10kb in one time
 *Input: encoder, encoder to be written
 *       fmt, format
 *Output: none
 *Return: gf_error_t
 */
gf_error_t gf_dump_encoder_fprintf(dump_data_encoder_t* encoder, const char *fmt, ...)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_t* file = NULL;
    int32_t n = 0;
    va_list ap;

    do
    {
        if (NULL == encoder || NULL == fmt)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 == encoder->cur_file_offset)
        {
            LOG_E(LOG_TAG, "[%s] no file opened",
                    __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        err = check_offset(encoder);
        if (GF_SUCCESS != err)
        {
            break;
        }

        // if remained buffer is less than 10kb, allocate more 20kb
        if (encoder->buf_len - encoder->cur_write_offset <= GF_DUMP_BUF_GROWTH_SIZE)
        {
            err = check_buf_size(encoder, GF_DUMP_BUF_GROWTH_SIZE * 2);
            if (GF_SUCCESS != err)
            {
                break;
            }
        }

        va_start(ap, fmt);
        n = vsnprintf((char*)encoder->buf + encoder->cur_write_offset,
                GF_DUMP_BUF_GROWTH_SIZE, fmt, ap);
        va_end(ap);

        if (n <= 0)
        {
            LOG_E(LOG_TAG, "[%s] snprintf failed return<%d>",
                    __func__, n);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        file = (dump_data_t*)(encoder->buf + encoder->cur_file_offset);
        file->data_size += n;
        encoder->cur_write_offset += n;

        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) encoder->buf;
        buf->buf_len += n;
    }  // do...
    while (0);

    return err;
}

/**
 *Function: gf_dump_encoder_fwrite
 *Description: like fwrite(), write 8bits wide data into current operating file
 *Input: data, data to write
 *       len, data len
 *       encoder, encoder to be written
 *Output: none
 *Return: gf_error_t
 */
gf_error_t gf_dump_encoder_fwrite(const void* data, uint32_t len, dump_data_encoder_t* encoder)
{
    gf_error_t err = GF_SUCCESS;
    dump_data_t* file = NULL;

    do
    {
        if (NULL == encoder || NULL == data || len <= 0)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (0 == encoder->cur_file_offset)
        {
            LOG_E(LOG_TAG, "[%s] no file opened",
                    __func__);
            err = GF_ERROR_INVALID_DATA;
            break;
        }

        err = check_offset(encoder);
        if (GF_SUCCESS != err)
        {
            break;
        }

        err = check_buf_size(encoder, len);
        if (GF_SUCCESS != err)
        {
            break;
        }
        memcpy(encoder->buf + encoder->cur_write_offset, (uint8_t*)data, len);

        file = (dump_data_t*)(encoder->buf + encoder->cur_file_offset);
        file->data_size += len;
        encoder->cur_write_offset += len;

        dump_data_buffer_header_t* buf = (dump_data_buffer_header_t*) encoder->buf;
        buf->buf_len += len;
    }  // do...
    while (0);

    return err;
}

/**
 *Function: gf_dump_encoder_fwrite
 *Description: like fclose(), close current operating file if no data to write
 *Input: encoder
 *Output: none
 *Return: gf_error_t
 */
gf_error_t gf_dump_encoder_fclose(dump_data_encoder_t* encoder)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (NULL == encoder)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        encoder->cur_file_offset = 0;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

