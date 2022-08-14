/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _TESTUTILS_HPP_
#define _TESTUTILS_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "HalLog.h"

namespace goodix
{

#define HAL_TEST_SIZEOF_INT64         (sizeof(uint32_t) + sizeof(int64_t))
#define HAL_TEST_SIZEOF_INT32         (sizeof(uint32_t) + sizeof(int32_t))
#define HAL_TEST_SIZEOF_INT16         (sizeof(uint32_t) + sizeof(int16_t))
#define HAL_TEST_SIZEOF_INT8          (sizeof(uint32_t) + sizeof(uint8_t))
#define HAL_TEST_SIZEOF_ARRAY(len)    (sizeof(uint32_t) + sizeof(int32_t) + (len))
#define HAL_TEST_SIZEOF_FLOAT         (sizeof(uint32_t) + sizeof(int32_t) + sizeof(float))
#define HAL_TEST_SIZEOF_DOUBLE        (sizeof(uint32_t) + sizeof(int32_t) + sizeof(double))

#define DARK_PIXEL_LOWER_LEFT_CORNER   4
#define DARK_PIXEL_UPPER_RIGHT_CORNER  2

class TestUtils
{
public:
    static int8_t *testEncodeInt8(int8_t *buf, uint32_t token, int8_t value)
    {
        int8_t *current = buf;

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

    static int8_t *testEncodeInt16(int8_t *buf, uint32_t token, int16_t value)
    {
        int8_t *current = buf;

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

    static int8_t *testEncodeInt32(int8_t *buf, uint32_t token, int32_t value)
    {
        int8_t *current = buf;

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

    static int8_t *testEncodeInt64(int8_t *buf, uint32_t token, int64_t value)
    {
        if (NULL == buf)
        {
            return buf;
        }

        int8_t *current = buf;
        // token
        memcpy(current, &token, sizeof(uint32_t));
        current += sizeof(uint32_t);
        // value
        memcpy(current, &value, sizeof(uint64_t));
        current += sizeof(uint64_t);
        return current;
    }

    static int8_t *testEncodeArray(int8_t *buf, uint32_t token, int8_t *array,
                                   uint32_t size)
    {
        int8_t *current = buf;

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

    static int8_t *testEncodeFloat(int8_t *buf, uint32_t token, float value)
    {
        int8_t *current = buf;

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

    static int8_t *testEncodeDouble(int8_t *buf, uint32_t token, double value)
    {
        int8_t *current = buf;

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

    static const int8_t *testDecodeUint32(uint32_t *value, const int8_t *buf)
    {
        // little endian
        const uint8_t* tmp = (const uint8_t*) buf;
        *value = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;
        buf = buf + sizeof(uint32_t);
        return buf;
    }

    static void testMemoryCheck(const char *func_name, int8_t *start, int8_t *end, uint32_t len)
    {
        uint32_t used_len = end - start;

        if (used_len < len)
        {
            LOG_E("[GF_HAL][TestUtils]", "[%s] alloced memery larger than used, used=%u, malloc=%u",
                  func_name, used_len, len);
        }
        else if (used_len > len)
        {
            LOG_E("[GF_HAL][TestUtils]", "[%s] memory out of bounds, used=%u, malloc=%u", func_name,
                  used_len, len);
            uint32_t *p = NULL;
            *p = 0xDEAD;
        }
    }

    static gf_error_t quantizationToByte(uint16_t *buf_src, uint16_t *buf_dst, uint32_t width, uint32_t height)
    {
        gf_error_t err = GF_SUCCESS;
        uint32_t i = 0, j = 0;
        double min_val = 0.0f;
        double max_val = 0.0f;
        uint16_t *src = buf_src;
        uint16_t *temp = NULL;

        FUNC_ENTER();

        do
        {
            temp = new uint16_t[width * height * sizeof(uint16_t)] { 0 };
            if (NULL == temp)
            {
                LOG_E(LOG_TAG, "[%s] out of memory", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width; j++)
                {
                    if ((i >= DARK_PIXEL_UPPER_RIGHT_CORNER)
                            && (i < (height - DARK_PIXEL_LOWER_LEFT_CORNER))
                            && (j >= DARK_PIXEL_LOWER_LEFT_CORNER)
                            && (j < (width - DARK_PIXEL_UPPER_RIGHT_CORNER)))
                    {
                        if (src[i * width + j] > max_val && src[i * width + j] != 8191)
                        {
                            max_val = src[i * width + j];
                        }

                        if (min_val >= -0.1f && min_val <= 0.1f)
                        {
                            min_val = src[i * width + j];
                        }

                        if ((src[i * width + j] < min_val) && (src[i * width + j] != 0))
                        {
                            min_val = src[i * width + j];
                        }
                    }
                }
            }
            LOG_D(LOG_TAG, "[%s] minVal=%f, maxVal=%f", __func__, min_val, max_val);

            for (j = 0; j < width * height; j++)
            {
                if (src[j] < min_val)
                {
                    temp[j] = 0;
                    continue;
                }
                if (src[j] > max_val)
                {
                    temp[j] = 0xff;
                    continue;
                }
                temp[j] = (src[j]-min_val) * (0xff / (max_val - min_val));
            }

            memcpy(buf_dst, temp, width * height* sizeof(uint16_t));
        }
        while (0);

        if (NULL != temp)
        {
            delete []temp;
            temp = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    static gf_error_t quantizeRawdataToBmp(uint16_t *raw_data,
                                    uint8_t *bmp_data,
                                    uint32_t width, uint32_t height)
    {
        gf_error_t err = GF_SUCCESS;
        uint16_t * quantized_rawdata = NULL;
        uint32_t i = 0;

        FUNC_ENTER();

        do
        {
            if (NULL == bmp_data || NULL == raw_data || width == 0 || height == 0)
            {
                LOG_E(LOG_TAG, "[%s] bad parameters", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            quantized_rawdata = new uint16_t[width * height * sizeof(uint16_t)] { 0 };
            if (NULL == quantized_rawdata)
            {
                LOG_E(LOG_TAG, "[%s] out of memory", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            err = quantizationToByte(raw_data, quantized_rawdata, width, height);
            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] global_quantization_to_byte failed", __func__);
                break;
            }

            for (i = 0; i < width * height; i++)
            {
                bmp_data[i] = (uint8_t) (quantized_rawdata[i] & 0xFF);
            }
        }
        while (0);

        if (quantized_rawdata)
        {
            delete []quantized_rawdata;
            quantized_rawdata = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }
};

};  // namespace goodix

#endif /* _TESTUTILS_HPP_ */
