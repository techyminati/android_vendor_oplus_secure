/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: Since dump data is written into encoder buffer, we need a decoder to decode buffer
 *              and write data to file system.
 * History:
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "gf_error.h"
#include "gf_dump_data_decoder.h"
#include "gf_dump_data_buffer.h"

#include "dump_decoder_posix.h"

#define GF_DUMP_ENCRYPTED_DATA_DIR "/gf_data/encrypted_data/"

#define FUNC_EXIT(err)                               \
    do                                           \
    {                                             \
        if (GF_SUCCESS != (err))                    \
        {                                           \
            LOG_D(LOG_TAG, "[%s] exit. errno=%d", __func__, (err)); \
        }                                            \
    }                                               \
    while (0)


#define SUFFIX_CSV ".csv"
#define SUFFIX_BMP ".bmp"

typedef struct
{
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t off_bits;
}
__attribute__((packed)) bmp_file_header_t;

typedef struct
{
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    uint32_t x_pels_permeter;
    uint32_t y_pels_permeter;
    uint32_t clr_used;
    uint32_t clr_important;
} bmp_info_header_t;

static bmp_file_header_t g_bmp_file_header = { 0x4d42, 0, 0, 0, 0 };  // bmp file header, for writing use
static bmp_info_header_t g_bmp_info_header = { 0, 0, 0, 1, 8, 0, 0, 0, 0, 0, 0 };  // bmp info header, for writing use

/**
 *Function: mk_dump_dir
 *Description: make dump dir if not exist
 *Input: root_dir, root dir
 *       filepath, relative path got from encoder
 *       abs_filepath, the absolute path in current file system
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t mk_dump_dir(const uint8_t* root_dir, const uint8_t* filepath, uint8_t* abs_filepath)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t* slash_pos = NULL;
    const uint8_t* tmp = NULL;
    int32_t ret = 0;

    do
    {
        if (NULL == filepath)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "[%s] path<%s>", __func__, filepath);

        if ('/' == *filepath && 1 == strlen((char*)filepath))
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        slash_pos = (uint8_t*)strrchr((char*)filepath, '/');
        if (NULL == slash_pos || filepath == slash_pos)
        {
            LOG_D(LOG_TAG, "[%s] write file <%s> to root dir", __func__, filepath);
            snprintf((char*)abs_filepath, GF_DUMP_FILE_PATH_MAX_LEN,
                NULL == slash_pos ? "%s/%s" : "%s%s", root_dir, (char*)filepath);
            break;
        }
        snprintf((char*)abs_filepath, GF_DUMP_FILE_PATH_MAX_LEN,
            "%s%s", root_dir, (char*)filepath);
        tmp = ('/' == *filepath ? filepath + 1 : filepath);
        memcpy(dir, tmp, slash_pos - tmp + 1);
        ret = mkdirs_relative((char*)dir, 0775, (char*)root_dir);
        if (ret < 0)
        {
            LOG_D(LOG_TAG, "[%s] make directory(%s) fail, ret=%d", __func__, dir, ret);
            err = GF_ERROR_MKDIR_FAILED;
            break;
        }
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: write_raw_data_file
 *Description: write DATA_TYPE_RAW_DATA and DATA_TYPE_NAV_RAW_DATA type data to file
 *Input: data_buf, encoder buffer header
 *       file_buf, file buffer in buffer
 *       root_dir, root dir in file system
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t write_raw_data_file(const dump_data_buffer_header_t* data_buf,
                                               const dump_data_t* file_buf,
                                               uint8_t* root_dir)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t abs_file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t line[1024];
    uint16_t* data = NULL;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t i;
    uint32_t j;
    int32_t k = 0;
    FILE *file = NULL;

    do
    {
        if (NULL == data_buf || NULL == file_buf || NULL == root_dir || 0 == file_buf->path_count)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        data = (uint16_t*)(file_buf->data + file_buf->path_count * GF_DUMP_FILE_PATH_MAX_LEN);
        if (DATA_TYPE_NAV_RAW_DATA == file_buf->type)
        {
            width = data_buf->nav_width;
            height = data_buf->nav_height;
        }
        else
        {
            width = data_buf->sensor_width;
            height = data_buf->sensor_height;
        }

        for (k = 0; k < file_buf->path_count; k++)
        {
            memcpy(file_path, file_buf->data + k * GF_DUMP_FILE_PATH_MAX_LEN,
                    GF_DUMP_FILE_PATH_MAX_LEN);

            err = mk_dump_dir(root_dir, file_path, abs_file_path);
            if (err != GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] mk dir failed<%s>", __func__, file_path);
                break;
            }

            if (0 == file_buf->data_size)
            {
                LOG_D(LOG_TAG, "[%s] data size is 0, just make file<%s>", __func__, abs_file_path);
                continue;
            }

            file = fopen((char*)abs_file_path, "wb");
            if (NULL == file)
            {
                LOG_D(LOG_TAG, "[%s] open file (%s) fail\n", __func__, abs_file_path);
                err = GF_ERROR_FILE_OPEN_FAILED;
                break;
            }

            LOG_D(LOG_TAG, "[%s] writting file<%s>\n", __func__, abs_file_path);

            memset(line, 0, 1024);

            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    snprintf((char*)line, sizeof(line), "%4d,", *data);
                    data++;
                    fwrite(line, sizeof(char), strlen((char*)line), file);
                }

                snprintf((char*)line, sizeof(line), "\n");
                fwrite(line, sizeof(char), strlen((char*)line), file);
            }

            fflush(file);
            fclose(file);
            file = NULL;
        }  // for (k = 0; k < file_buf->path_count; k++)
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: write_image_to_bmp_file
 *Description: write image data to file
 *Input: data_buf, encoder buffer header
 *       file_buf, file buffer in buffer
 *       root_dir, root dir in file system
 *Output: file in file system
 *Return: gf_error_t
 */
gf_error_t write_image_to_bmp_file(const uint8_t* file_path,
                                        const uint8_t* image,
                                        uint32_t width,
                                        uint32_t height)
{
    gf_error_t err = GF_SUCCESS;
    FILE *file = NULL;
    uint32_t line_width = 0;
    uint8_t alpha = 0;
    int32_t i;

    do
    {
        if (NULL == file_path || NULL == image || width == 0 || height == 0)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        line_width = (width + 3) / 4 * 4;
        g_bmp_file_header.off_bits = sizeof(bmp_file_header_t)
                                     + sizeof(bmp_info_header_t)
                                     + 4 * 256;
        g_bmp_file_header.size = g_bmp_file_header.off_bits + line_width * height;
        g_bmp_info_header.size = sizeof(bmp_info_header_t);
        g_bmp_info_header.width = width;
        g_bmp_info_header.height = height;
        g_bmp_info_header.size_image = line_width * height;
        file = fopen((char*)file_path, "wb");

        if (NULL == file)
        {
            LOG_D(LOG_TAG, "[%s] open file (%s) fail", __func__, file_path);
            err = GF_ERROR_FILE_OPEN_FAILED;
            break;
        }

        fwrite(&g_bmp_file_header.type, 1, sizeof(g_bmp_file_header.type), file);
        fwrite(&g_bmp_file_header.size, 1, sizeof(g_bmp_file_header.size), file);
        fwrite(&g_bmp_file_header.reserved1, 1, sizeof(g_bmp_file_header.reserved1),
               file);
        fwrite(&g_bmp_file_header.reserved2, 1, sizeof(g_bmp_file_header.reserved2),
               file);
        fwrite(&g_bmp_file_header.off_bits, 1, sizeof(g_bmp_file_header.off_bits),
               file);
        fwrite(&g_bmp_info_header, 1, sizeof(bmp_info_header_t), file);

        for (i = 0; i < 256; i++)
        {
            fwrite(&i, 1, sizeof(uint8_t), file);
            fwrite(&i, 1, sizeof(uint8_t), file);
            fwrite(&i, 1, sizeof(uint8_t), file);
            fwrite(&alpha, 1, sizeof(uint8_t), file);
        }

        for (i = height - 1; i >= 0; i--)
        {
            fwrite(image + i * width, 1, width, file);

            if (line_width > width)
            {
                uint8_t line_align[4] = { 0 };
                fwrite(line_align, 1, line_width - width, file);
            }
        }
    }  // do...
    while (0);

    if (file != NULL)
    {
        fflush(file);
        fclose(file);
        file = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: write_image_to_csv_file
 *Description: write image data to cvs file
 *Input: file_path, file path for the csv
 *       data, data to be written
 *       width, image width
 *       height, image height
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t write_image_to_csv_file(const uint8_t *file_path,
                                                    const uint8_t *data,
                                                    uint32_t width,
                                                    uint32_t height)
{
    gf_error_t err = GF_SUCCESS;
    FILE *file = NULL;
    char line[1024] = { 0 };
    uint32_t i;
    uint32_t j;

    do
    {
        if (NULL == file_path || NULL == data || width == 0 || height == 0)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        file = fopen((char*)file_path, "wb");

        if (NULL == file)
        {
            LOG_D(LOG_TAG, "[%s] open file (%s) fail\n", __func__, file_path);
            err = GF_ERROR_FILE_OPEN_FAILED;
            break;
        }


        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                snprintf(line, sizeof(line), "%3d,", *data);
                data++;
                fwrite(line, sizeof(char), strlen(line), file);
            }

            snprintf(line, sizeof(line), "\n");
            fwrite(line, sizeof(char), strlen(line), file);
        }
    }  // do...
    while (0);

    if (file != NULL)
    {
        fflush(file);
        fclose(file);
        file = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: write_file
 *Description: util funtion to write data to file
 *Input: abs_file_path, absolute file path in file system
 *       data, data to be written
 *       size, data size
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t write_file(const uint8_t* abs_file_path,
                                const uint8_t* data,
                                const uint32_t size)
{
    gf_error_t err = GF_SUCCESS;
    FILE *file = NULL;

    do
    {
        if (NULL == abs_file_path || NULL == data || 0 == size)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        file = fopen((char*)abs_file_path, "wb");

        if (NULL == file)
        {
            LOG_D(LOG_TAG, "[%s] open file (%s) fail", __func__, abs_file_path);
            err = GF_ERROR_FILE_OPEN_FAILED;
            break;
        }
        LOG_D(LOG_TAG, "[%s] writing <%d> bytes to %s", __func__, size, abs_file_path);
        fwrite(data, sizeof(uint8_t), size, file);
    }
    while (0);
    if (file != NULL)
    {
        fflush(file);
        fclose(file);
        file = NULL;
    }
    FUNC_EXIT(err);
    return err;
}

/**
 *Function: write_image_data_file
 *Description: write image data file
 *Input: data_buf, encoder buffer header
 *       file_buf, file buffer in encoder buffer
 *       root_dir, root dir in file system
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t write_image_data_file(const dump_data_buffer_header_t* data_buf,
                                               const dump_data_t* file_buf,
                                               const uint8_t* root_dir)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t abs_file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t* suffix = NULL;
    const uint8_t* data = NULL;
    uint32_t file_size = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    int32_t i = 0;

    do
    {
        if (NULL == data_buf || NULL == file_buf || NULL == root_dir || 0 == file_buf->path_count)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        file_size = file_buf->data_size;
        data = file_buf->data + file_buf->path_count * GF_DUMP_FILE_PATH_MAX_LEN;
        width = data_buf->sensor_width;
        height = data_buf->sensor_height;
        for (i = 0; i < file_buf->path_count; i++)
        {
            memcpy(file_path, file_buf->data + i * GF_DUMP_FILE_PATH_MAX_LEN,
                    GF_DUMP_FILE_PATH_MAX_LEN);

            LOG_D(LOG_TAG, "[%s] writting file<%s>", __func__, file_path);
            err = mk_dump_dir(root_dir, file_path, abs_file_path);
            if (err != GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] mk dir failed<%s>", __func__, file_path);
                break;
            }

            if (0 == file_buf->data_size)
            {
                LOG_D(LOG_TAG, "[%s] data size is 0, just make file<%s>", __func__, abs_file_path);
                continue;
            }

            suffix = (uint8_t*)strrchr((char*)abs_file_path, '.');
            if (suffix != NULL && 0 == strcmp((char*)suffix, SUFFIX_CSV))
            {
                err = write_image_to_csv_file(abs_file_path, data, width, height);
            }
            else if (suffix != NULL && 0 == strcmp((char*)suffix, SUFFIX_BMP))
            {
                err = write_image_to_bmp_file(abs_file_path, data, width, height);
            }
            else
            {
                err = write_file(abs_file_path, data, file_size);
            }
        }  // for (i = 0; i < file_buf->path_count; i++)
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: write_normal_file
 *Description: just write data to file, no format
 *Input: data_buf, encoder buffer header
 *       file_buf, file buffer in encoder buffer
 *       root_dir, root dir in file system
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t write_normal_file(const dump_data_buffer_header_t* data_buf,
                                           const dump_data_t* file_buf,
                                           const uint8_t* root_dir)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t abs_file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t file_size = 0;
    const uint8_t* data = NULL;
    int32_t i = 0;

    do
    {
        if (NULL == data_buf || NULL == file_buf || NULL == root_dir || 0 == file_buf->path_count)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        file_size = file_buf->data_size;
        data = file_buf->data + file_buf->path_count * GF_DUMP_FILE_PATH_MAX_LEN;
        for (i = 0; i < file_buf->path_count; i++)
        {
            memcpy(file_path, file_buf->data + i * GF_DUMP_FILE_PATH_MAX_LEN,
                    GF_DUMP_FILE_PATH_MAX_LEN);
            LOG_D(LOG_TAG, "[%s] writting file<%s>", __func__, file_path);
            err = mk_dump_dir(root_dir, file_path, abs_file_path);
            if (err != GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] mk dir failed<%s>", __func__, file_path);
                break;
            }

            if (0 == file_buf->data_size)
            {
                LOG_D(LOG_TAG, "[%s] data size is 0, just make file<%s>", __func__, abs_file_path);
                continue;
            }

            err = write_file(abs_file_path, data, file_size);
            if (err != GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] write file failed<%s>\n", __func__, file_path);
                break;
            }
        }  // for (i = 0; i < file_buf->path_count; i++)
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: write_file_buffer_to_storage
 *Description: write data to file according to file data type
 *Input: data_buf, encoder buffer header
 *       file_buf, file buffer in encoder buffer
 *       root_dir, root dir in file system
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t write_file_buffer_to_storage(const dump_data_buffer_header_t* data_buf,
                                               const dump_data_t* file_buf,
                                               uint8_t* root_dir)
{
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == file_buf)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "[%s]writting data: type<%d>,path_count<%u>,size<%u>", __func__,
                file_buf->type, file_buf->path_count, file_buf->data_size);

        switch (file_buf->type)
        {
            case DATA_TYPE_RAW_DATA:
            case DATA_TYPE_NAV_RAW_DATA:
            {
                err = write_raw_data_file(data_buf, file_buf, root_dir);
                break;
            }

            case DATA_TYPE_IMAGE_DATA:
            {
                err = write_image_data_file(data_buf, file_buf, root_dir);
                break;
            }

            case DATA_TYPE_NORMAL_FILE_DATA:
            {
                err = write_normal_file(data_buf, file_buf, root_dir);
                break;
            }

            default:
            {
                err = write_normal_file(data_buf, file_buf, root_dir);
                break;
            }
        }
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: validate_abs_file_path
 *Description: validate if a given path exist, if exist, add a suffix for the file path.
 *Input: abs_file_path, absolute path in file system.
 *Output: validated file path
 *Return: none
 */
static void validate_abs_file_path(uint8_t* abs_file_path)
{
    FILE* file = NULL;
    uint8_t tmp_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t suffix[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t inc_str[10] = { 0 };
    uint8_t* dot_pos = NULL;
    int32_t path_len = 0;
    int32_t suffix_len = 0;
    uint32_t index = 1;
    int32_t inc_max = GF_DUMP_FILE_PATH_MAX_LEN;
    int32_t ret = 0;

    if (NULL == abs_file_path)
    {
        return;
    }

    // most of the time, filse not exist
    file = fopen((char*)abs_file_path, "r");
    if (NULL == file)
    {
        // file not exist
        return;
    }
    fclose(file);
    file = NULL;

    dot_pos = (uint8_t*)strrchr((char*)abs_file_path, '.');
    // do more check here?
    if (NULL == dot_pos)
    {
        snprintf((char*)tmp_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s", (char*)abs_file_path);
    }
    else
    {
        memcpy(tmp_path, abs_file_path, dot_pos - abs_file_path);
        snprintf((char*)suffix, GF_DUMP_FILE_PATH_MAX_LEN, "%s", dot_pos);
    }

    path_len = strlen((char*)tmp_path);
    suffix_len = strlen((char*)suffix);
    do
    {
        snprintf((char*)inc_str, sizeof(inc_str), "_%u", index);
        if (path_len + strlen((char*)inc_str) + suffix_len >= GF_DUMP_FILE_PATH_MAX_LEN)
        {
            // just in case, remove existed file
            ret = remove((char*)abs_file_path);
            if (ret != 0)
            {
                LOG_D(LOG_TAG, "[%s] remove file (%s) fail", __func__, abs_file_path);
            }
            break;
        }
        suffix_len != 0 ?
            snprintf((char*)abs_file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s%s", tmp_path, inc_str, suffix) :
            snprintf((char*)abs_file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s", tmp_path, inc_str);

        file = fopen((char*)abs_file_path, "r");
        if (NULL == file)
        {
            // file not exist
            break;
        }
        fclose(file);
        file = NULL;
    }
    while (++index <= inc_max);

    return;
}

/**
 *Function: write_encrypted_buf_to_file
 *Description: write encrypted buffer to file, file name is generated by time stamp in encoder.
 *Input: data, encoder data buffer.
 *       root_dir, root dir in file system.
 *Output: file in file system
 *Return: gf_error_t
 */
static gf_error_t write_encrypted_buf_to_file(dump_data_buffer_header_t *data, const uint8_t* root_dir)
{
    gf_error_t err = GF_SUCCESS;
    FILE* file;
    uint8_t file_path[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    int64_t time_stamp = 0;
    uint8_t* tmp = NULL;
    int32_t ret = -1;

    do
    {
        if (NULL == data || NULL == root_dir
            || GF_DUMP_NOT_ENCRYPTED == data->encrypt_info)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        time_stamp = ((int64_t)data->ts_low & 0xFFFFFFFF) | ((int64_t)data->ts_high << 32);
        snprintf((char*)file_path, GF_DUMP_FILE_PATH_MAX_LEN, "%s%s%"PRId64".dat", (char*)root_dir,
            GF_DUMP_ENCRYPTED_DATA_DIR, (int64_t)time_stamp);
        LOG_D(LOG_TAG, "[%s] writting encryptped file<%s>", __func__, file_path);

        tmp = (uint8_t*)GF_DUMP_ENCRYPTED_DATA_DIR;
        // mkdirs_relative doesn't accept dir start with '/' for relative path
        tmp++;
        snprintf((char*)dir, GF_DUMP_FILE_PATH_MAX_LEN, "%s", (char*)tmp);
        ret = mkdirs_relative((char*)dir, 0755, (char*)root_dir);
        if (ret < 0)
        {
            LOG_D(LOG_TAG, "[%s] make dir(%s) fail, ret=%d", __func__, dir, ret);
            err = GF_ERROR_FILE_OPEN_FAILED;
            break;
        }

        validate_abs_file_path(file_path);

        file = fopen((char*)file_path, "wb");

        if (NULL == file)
        {
            LOG_D(LOG_TAG, "[%s] open file (%s) fail", __func__, file_path);
            err = GF_ERROR_FILE_OPEN_FAILED;
            break;
        }

        fwrite(data, sizeof(uint8_t), data->buf_len, file);
        fflush(file);
        fclose(file);
        file = NULL;
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 *Function: gf_decode_and_write_file
 *Description: interface function for decoder
 *Input: buf, encoder buffer
 *       buf_len, buffer length
 *       root_dir, root dir in file system.
 *Output: files in file system
 *Return: gf_error_t
 */
int32_t gf_decode_and_write_file(const void* buf, uint32_t buf_len, const uint8_t* root_dir)
{
    gf_error_t err = GF_SUCCESS;
    uint8_t dir[GF_DUMP_FILE_PATH_MAX_LEN] = { 0 };
    uint32_t read_offset = 0;
    uint32_t len = 0;

    do
    {
        if (NULL == buf || NULL == root_dir)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "[%s]root_dir: %s", __func__, root_dir);

        dump_data_buffer_header_t* data_buf = (dump_data_buffer_header_t*)buf;

        LOG_D(LOG_TAG, "[%s] en<%d>,ts_l<%d>,ts_h<%d>,len<%u>,sw<%u>,sh<%u>,nw<%u>,nh<%u>\n", __func__,
            data_buf->encrypt_info, data_buf->ts_low, data_buf->ts_high, data_buf->buf_len,
            data_buf->sensor_width, data_buf->sensor_height, data_buf->nav_width, data_buf->nav_height);

        if (data_buf->buf_len != buf_len
            || data_buf->buf_len <= sizeof(dump_data_buffer_header_t))
        {
            LOG_D(LOG_TAG, "[%s] buf len incorrect", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        len = strlen((char*)root_dir);
        if (len <= 0 || len >= GF_DUMP_FILE_PATH_MAX_LEN)
        {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if ('/' == root_dir[len - 1]  && len > 1)
        {
            len--;
        }
        strncpy((char*)dir, (char*)root_dir, len);

        if (data_buf->encrypt_info != GF_DUMP_NOT_ENCRYPTED)
        {
            err = write_encrypted_buf_to_file(data_buf, dir);
            break;
        }

        read_offset = sizeof(dump_data_buffer_header_t);
        dump_data_t* file_buf = NULL;

        while (read_offset < data_buf->buf_len)
        {
            file_buf = (dump_data_t*) ((uint8_t*)data_buf + read_offset);
            err = write_file_buffer_to_storage(data_buf, file_buf, dir);
            if (err != GF_SUCCESS)
            {
                break;
            }
            read_offset += (sizeof(dump_data_t) +
                file_buf->path_count * GF_DUMP_FILE_PATH_MAX_LEN + file_buf->data_size);
        }
    }  // do...
    while (0);

    FUNC_EXIT(err);
    return err;
}
