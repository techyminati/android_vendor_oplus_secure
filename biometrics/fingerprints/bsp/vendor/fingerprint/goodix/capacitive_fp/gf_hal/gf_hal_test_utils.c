/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf_hal_test_utils.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_common.h"
#define LOG_TAG "[GF_HAL][gf_hal_test_utils]"

/**
 * Function: hal_test_encode_int8
 * Description: Join token(32bit) and value(8bit) to byte stream.
 * Input: buf, token, value
 * Output: buf
 * Return: uint8_t
 */
uint8_t *hal_test_encode_int8(uint8_t *buf, uint32_t token, int8_t value)
{
    uint8_t *current = buf;

    if (NULL == buf)
    {
        return buf;
    }

    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // value
    memcpy(current, &value, sizeof(int8_t));
    current += sizeof(int8_t);
    return current;
}

/**
 * Function: hal_test_encode_int16
 * Description: Join token(32bit) and value(16bit) to byte stream.
 * Input: buf, token, value
 * Output: buf
 * Return: uint8_t
 */
uint8_t *hal_test_encode_int16(uint8_t *buf, uint32_t token, int16_t value)
{
    uint8_t *current = buf;

    if (NULL == buf)
    {
        return buf;
    }

    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // value
    memcpy(current, &value, sizeof(int16_t));
    current += sizeof(int16_t);
    return current;
}

/**
 * Function: hal_test_encode_int32
 * Description: Join token(32bit) and value(32bit) to byte stream.
 * Input: buf, token, value
 * Output: buf
 * Return: uint8_t
 */
uint8_t *hal_test_encode_int32(uint8_t *buf, uint32_t token, int32_t value)
{
    uint8_t *current = buf;

    if (NULL == buf)
    {
        return buf;
    }

    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // value
    memcpy(current, &value, sizeof(int32_t));
    current += sizeof(int32_t);
    return current;
}

/******************************************************************************************
 * Function: hal_test_encode_float32
 * Description: copy token and value to buf
 * Input: buf token value
 * Output: buf
 * Return: buf
********************************************************************************************/
uint8_t *hal_test_encode_float32(uint8_t *buf, uint32_t token, float value)
{
    uint8_t *current = buf;

    if (NULL == buf)
    {
        return buf;
    }

    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // value
    memcpy(current, &value, sizeof(float));
    current += sizeof(float);
    return current;
}

/**
 * Function: hal_test_encode_int64
 * Description: Join token(32bit) and value(64bit) to byte stream.
 * Input: buf, token, value
 * Output: buf
 * Return: uint8_t
 */
uint8_t *hal_test_encode_int64(uint8_t *buf, uint32_t token, int64_t value)
{
    if (NULL == buf)
    {
        return buf;
    }

    uint8_t *current = buf;
    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // value
    memcpy(current, &value, sizeof(uint64_t));
    current += sizeof(uint64_t);
    return current;
}

/**
 * Function: hal_test_encode_array
 * Description: Join token(32bit) and array to byte stream.
 * Input: buf, token, array, size
 * Output: buf
 * Return: uint8_t*
 */
uint8_t *hal_test_encode_array(uint8_t *buf, uint32_t token, uint8_t *array,
                               uint32_t size)
{
    uint8_t *current = buf;

    if (NULL == buf || NULL == array)
    {
        return buf;
    }

    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // size
    memcpy(current, &size, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // array
    memcpy(current, array, size);
    current += size;
    return current;
}

/**
 * Function: hal_test_encode_float
 * Description: Join token(32bit) and value(float) to byte stream.
 * Input: buf, token, value
 * Output: buf
 * Return: uint8_t
 */
uint8_t *hal_test_encode_float(uint8_t *buf, uint32_t token, float value)
{
    uint8_t *current = buf;

    if (NULL == buf)
    {
        return buf;
    }

    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // size
    uint32_t size = sizeof(float);
    memcpy(current, &size, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // value
    memcpy(current, &value, size);
    current += size;
    return current;
}

/**
 * Function: hal_test_encode_double
 * Description: Join token(32bit) and value(double) to byte stream.
 * Input: buf, token, value
 * Output: buf
 * Return: uint8_t
 */
uint8_t *hal_test_encode_double(uint8_t *buf, uint32_t token, double value)
{
    uint8_t *current = buf;

    if (NULL == buf)
    {
        return buf;
    }

    // token
    memcpy(current, &token, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // size
    uint32_t size = sizeof(double);
    memcpy(current, &size, sizeof(uint32_t));
    current += sizeof(uint32_t);
    // value
    memcpy(current, &value, size);
    current += size;
    return current;
}

/**
 * Function: hal_test_decode_uint32
 * Description: Decode value(4bytes) from byte stream.
 * Input: buf, value
 * Output: value
 * Return: uint8_t
 */
const uint8_t *hal_test_decode_uint32(uint32_t *value, const uint8_t *buf)
{
    *value = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
    buf = buf + sizeof(uint32_t);
    return buf;
}

/**
 * Function: hal_notify_test_memory_check
 * Description: Notify information of memory check.
 * Input: func_name, start, end,len
 * Output: value
 * Return: void
 */
void hal_notify_test_memory_check(const char *func_name, uint8_t *start,
                                  uint8_t *end,
                                  uint32_t len)
{
    uint32_t used_len = 0;

    if (NULL == func_name || NULL == start || NULL == end)
    {
        LOG_E(LOG_TAG, "[%s] func_name or start or end is null", __func__);
        return;
    }

    used_len = end - start;

    if (used_len < len)
    {
        LOG_E(LOG_TAG, "[%s] alloced memery larger than used, used=%u, malloc=%u",
              func_name, used_len, len);
    }
    else if (used_len > len)
    {
        LOG_E(LOG_TAG, "[%s] memory out of bounds, used=%u, malloc=%u", func_name,
              used_len, len);
        uint32_t *p = NULL;
        *p = 0xDEAD;
    }
}
