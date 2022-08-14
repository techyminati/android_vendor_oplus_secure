/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: Dump data buffer
 * History: None
 * Version: 1.0
 */
#ifndef _GF_DUMP_DATA_BUFFER_H_
#define _GF_DUMP_DATA_BUFFER_H_

#include <stdint.h>

#define GF_DUMP_FILE_PATH_MAX_LEN   (256)
#define GF_DUMP_NOT_ENCRYPTED       (-1)

// this enum value is used to write decoded data file
typedef enum
{
    DATA_TYPE_UNKNOWN = -1,
    DATA_TYPE_RAW_DATA,  // raw data is 16bits data and written into csv file, size = 2*sensor_width*sensor_height
    DATA_TYPE_NAV_RAW_DATA,  // size = 2*nav_width*nav_height
    DATA_TYPE_IMAGE_DATA,  // image data is 8bits data and written into bmp or csv file
    DATA_TYPE_NORMAL_FILE_DATA,  // normal and other file types data is written into file directly
    DATA_TYPE_MAX,
} data_type_t;

typedef struct
{
    int32_t     type;  // data type, raw_data_type_t
    uint32_t    path_count;  // path count
    uint32_t    data_size;  // raw data size.
    uint8_t     data[0];  // put path in data, data offset = sizeof(raw_data_file_t)  + path_count*256
} dump_data_t;

typedef struct
{
    int32_t     encrypt_info;  // -1 not encrypted, > -1 encryptor version

    // the lower 4 bytes of int64_t time stamp
    int32_t     ts_low;
    // the higher 4 bytes of int64_t time stamp
    // int64_t time_stamp = (int64_t)ts_low | (int64_t)ts_high << 32
    int32_t     ts_high;

    uint32_t    buf_len;  // total buf len including header
    uint32_t    sensor_width;  // defined in HAL, used to generate csv or bmp file
    uint32_t    sensor_height;  // defined in HAL, used to generate csv or bmp file
    uint32_t    nav_width;  // defined in HAL, used to generate csv or bmp file
    uint32_t    nav_height;  // defined in HAL, used to generate csv or bmp file
    dump_data_t data_file[0];  // raw data
} dump_data_buffer_header_t;


#endif  // _GF_DUMP_DATA_BUFFER_H_

