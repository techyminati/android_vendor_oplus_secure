/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: Dump data encoder
 * History: None
 * Version: 1.0
 */
#ifndef _GF_DUMP_DATA_ENCODER_H_
#define _GF_DUMP_DATA_ENCODER_H_

#include "gf_error.h"
#include "gf_dump_data_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

// =================================================
// use below APIs to write dump buffer
// =================================================

typedef struct dump_data_encoder
{
    uint32_t            cur_file_offset;  // current writting file offset
    uint32_t            cur_write_offset;  // the whole buffer write offset
    uint32_t            buf_len;  // allocated buffer len
    uint8_t*            buf;  // allocated buffer
} dump_data_encoder_t;

/**
 * @brief: create a buffer
 */
gf_error_t gf_dump_encoder_create(dump_data_encoder_t** encoder,
                                int64_t time_stamp);

/**
 * @brief: free resource, call this function after dump finished
 */
void       gf_dump_encoder_destroy(dump_data_encoder_t* encoder);

/**
 * @brief: call this function to reuse buffer, data will be erased in buffer
 */
void       gf_dump_encoder_reset(dump_data_encoder_t* encoder);


/**
 * @brief: like fopen(), prepare to write file,
 */
gf_error_t gf_dump_encoder_fopen(dump_data_encoder_t* encoder,
                             const uint8_t* file_path,
                             data_type_t data_type);

/**
 * @brief: Add a file path to current opened file if have multi path.
 *         Path can only be added before gf_dump_encoder_fwrite called
 *         otherwise return error.
 */
gf_error_t gf_dump_encoder_add_path(dump_data_encoder_t* encoder,
                             const uint8_t* file_path);


/**
 * @brief: like fwrite(), write 8bits wide data into current operating file
 *
 */
gf_error_t gf_dump_encoder_fwrite(const void* data, uint32_t len, dump_data_encoder_t* encoder);

/**
 * @brief: like fprintf(), write formated data into current operating file,
 *         can only write less than 10kb in one time
 */
gf_error_t gf_dump_encoder_fprintf(dump_data_encoder_t* encoder, const char *fmt, ...);

/**
 * @brief: like fclose(), close current operating file if no data to write
 *
 */
gf_error_t gf_dump_encoder_fclose(dump_data_encoder_t* encoder);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_DUMP_DATA_ENCODER_H_
