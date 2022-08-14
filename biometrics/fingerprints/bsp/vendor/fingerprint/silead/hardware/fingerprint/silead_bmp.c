/******************************************************************************
 * @file   silead_bmp.c
 * @brief  Contains Bitmap file operate functions.
 *
 *
 * Copyright (c) 2016-2019 Silead Inc.
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
 * <author>    <date>   <version>     <desc>
 * calvin wang  2018/1/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_img"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_bmp.h"
#include "silead_util.h"

#define BMP_TYPE 0x4d42
#define BMP_MAGIC1 0x0511
#define BMP_MAGIC2 0x1EAD
#define BMP_EXT2_MAGIC1 0x4d4d
#define BMP_EXT2_MAGIC2 0x4242
#define BMP_EXT_MUL_RAW_MAGIC1 0x7773
#define BMP_EXT_MUL_RAW_MAGIC2 0x7261

#define BMP_DATA_SCALE_MAGIC 0x0511
#define BMP_DATA_SCALE_IN 3

#define BIT_PER_BYTE_SHIFT 3 // 8 bit per byte
#define BMP_DATA_BYTE_8BIT 1
#define BMP_DATA_BYTE_16BIT 2
#define BMP_DATA_BYTE_24BIT 3

#ifdef HOST_OS_WINDOWS
#pragma pack(1)
#endif

typedef struct __attribute__((packed)) _bitmap_file_header {
    uint16_t bf_type;
    uint32_t bf_size;
    uint16_t bf_reserved1;
    uint16_t bf_reserved2;
    uint32_t bf_offbits;
} bitmap_file_header_t;

typedef struct __attribute__((packed)) _bitmap_info_header {
    uint32_t bi_size;
    uint32_t bi_width;
    uint32_t bi_height;
    uint16_t bi_planes;
    uint16_t bi_bitcount;
    uint32_t bi_compression;
    uint32_t bi_size_image;
    uint32_t bi_x_pels_per_meter;
    uint32_t bi_y_pels_per_meter;
    uint32_t bi_clr_used;
    uint32_t bi_clr_important;
} bitmap_info_header_t;

typedef struct __attribute__((packed)) _bitmap_info_ext_header {
    uint16_t bie_magic1;
    uint16_t bie_magic2;
    uint32_t bie_checksum;
    uint32_t bie_width;
    uint32_t bie_height;
    uint32_t bie_bitcount;
} bitmap_info_ext_header_t;

typedef struct __attribute__((packed)) _bitmap_info_ext2_header {
    uint16_t bie_magic1;
    uint16_t bie_magic2;
    uint32_t bie_size;
    uint32_t bie_checksum;
} bitmap_info_ext2_header_t;

typedef struct __attribute__((packed)) _bitmap_info_ext_mul_raw_header {
    uint16_t bie_magic1;
    uint16_t bie_magic2;
    uint32_t bie_size;
    uint32_t bie_checksum;
} bitmap_info_ext_mul_raw_header_t;

#ifdef HOST_OS_WINDOWS
#pragma pack()
#endif

#define BITMAP_IMAGE_HEADER_SIZE (sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t))
#define BITMAP_IMAGE_8BIT_PALETTE_SIZE (256*4)
#define BITMAP_IMAGE_EXT_HEADER_SIZE (sizeof(bitmap_info_ext_header_t))
#define BITMAP_IMAGE_EXT2_HEADER_SIZE (sizeof(bitmap_info_ext2_header_t))
#define BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE (sizeof(bitmap_info_ext_mul_raw_header_t))

static uint8_t m_color_map[256*4] = {0};
static int32_t m_color_inited = 0;

static uint8_t m_img_mraw_enable = 1; // save mul raw data
static uint8_t m_img_mraw_preview_enable = 1; // save preview image in bmp data
static uint8_t m_img_24bit_rgb_enable = 1; // save rgb color in bmp data

static uint16_t m_img_16bit_ww = 2000;
static uint32_t m_img_16bit_type = IMG_16BIT_DATA_NORMAL;
void silfp_bmp_set_img_16bit_type(uint32_t type)
{
    LOG_MSG_DEBUG("set 16bit img type %x", type);
    m_img_16bit_type = type;
}
void silfp_bmp_set_img_16bit_ww(uint16_t ww)
{
    LOG_MSG_DEBUG("set 16bit img ww %u", ww);
    m_img_16bit_ww = ww;
}

void silfp_bmp_set_mraw_preview_enable(uint8_t enable)
{
    m_img_mraw_preview_enable = enable;
}

void silfp_bmp_set_24bit_rgb_enable(uint8_t enable)
{
    m_img_24bit_rgb_enable = enable;
}

static int32_t _bmp_calc_checksum(const void *in, uint32_t len, uint32_t *checksum)
{
    uint32_t i = 0;
    unsigned long int sum = 0;
    const uint8_t *p;

    if (in == NULL || checksum == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    p = (const uint8_t *)in;
    for (i = 0; i < len; i++) {
        sum += *p++;
    }

    while (sum >> 16) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    *checksum = ~sum;

    return SL_SUCCESS;
}

/* [filling] true: rgb bmp img, false: 8bit gray scale image */
static uint8_t _bmp_is_rgb_mode(uint8_t bytes_per_pixel)
{
    return ((bytes_per_pixel > BMP_DATA_BYTE_16BIT) && m_img_24bit_rgb_enable);
}

static uint8_t _bmp_get_correct_bitcount(uint8_t bitcount)
{
    uint8_t bit = 0;

    bit = ((bitcount >> BIT_PER_BYTE_SHIFT) & 0xFF);
    if (bit == 0) {
        bit = BMP_DATA_BYTE_8BIT;
    }
    bit = bit << BIT_PER_BYTE_SHIFT;

    return bit;
}

static uint8_t _bmp_get_bytes_per_pixel(uint8_t bitcount)
{
    uint8_t bit = 0;

    bit = ((bitcount >> BIT_PER_BYTE_SHIFT) & 0xFF);
    if (bit == 0) {
        bit = BMP_DATA_BYTE_8BIT;
    }

    return bit;
}

static uint8_t _bmp_get_bitcount(uint8_t bytes_per_pixel)
{
    return (bytes_per_pixel << BIT_PER_BYTE_SHIFT);
}

static uint32_t _bmp_get_ext_size(uint32_t w, uint32_t h, uint8_t bitcount)
{
    uint32_t bytes_per_pixel = 0;
    uint32_t len = 0;

    bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    len = BITMAP_IMAGE_EXT_HEADER_SIZE;
    len += w * h * bytes_per_pixel;

    return len;
}

static uint32_t _bmp_get_ext2_size(uint32_t size)
{
    return (BITMAP_IMAGE_EXT2_HEADER_SIZE + size);
}

static uint32_t _bmp_get_ext_mul_raw_size(uint32_t size)
{
    return (BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE + size);
}

/* [filling] */
static uint32_t _bmp_get_correct_align_width(uint32_t w, uint8_t bytes_per_pixel)
{
    uint8_t bitcount = 0;

    if (!_bmp_is_rgb_mode(bytes_per_pixel)) { // 8bit gray scale image
        bytes_per_pixel = BMP_DATA_BYTE_8BIT;
    }
    bitcount = _bmp_get_bitcount(bytes_per_pixel);

    return (((w * bitcount) + 31) >> 5) << 2;
}

/* [parse] */
static uint32_t _bmp_get_align_width(uint32_t w, uint8_t bytes_per_pixel)
{
    uint8_t bitcount = _bmp_get_bitcount(bytes_per_pixel);
    return (((w * bitcount) + 31) >> 5) << 2;
}

/* [filling] */
static uint32_t _bmp_get_normal_size(uint32_t w, uint32_t h, uint8_t bytes_per_pixel)
{
    uint32_t len = 0;
    uint32_t fixed_w = _bmp_get_correct_align_width(w, bytes_per_pixel);

    len = BITMAP_IMAGE_HEADER_SIZE; // img header
    len += fixed_w * h; // img data

    if (!_bmp_is_rgb_mode(bytes_per_pixel)) { // img palette
        len += BITMAP_IMAGE_8BIT_PALETTE_SIZE;
    }

    return len;
}

uint32_t silfp_bmp_get_size(uint32_t w, uint32_t h, uint8_t bitcount)
{
    uint32_t len = 0;
    uint8_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);

    // file size without ext2, mraw
    len = _bmp_get_normal_size(w, h, bytes_per_pixel);
    len += _bmp_get_ext_size(w, h, bitcount);

    return len;
}

static int32_t _bmp_16bit_swap_data(void *dest, void *src, uint32_t w, uint32_t h)
{
    uint32_t i = 0;
    uint8_t *pdest = (uint8_t *)dest;
    uint8_t *psrc = (uint8_t *)src;
    uint32_t size = (w * h * 2);
    uint8_t tmp = 0;

    if ((dest == NULL) || (src == NULL) || (w == 0) || (h == 0)) {
        return -1;
    }

    for (i = 0; i < size; i += 2) {
        tmp = psrc[i];
        pdest[i] = psrc[i + 1];
        pdest[i + 1] = tmp;
    }

    return size;
}

static uint16_t _bmp_16bit_get_average(void *pdata, uint32_t w, uint32_t h)
{
    uint64_t sum = 0;
    uint32_t i = 0;
    uint32_t size = (w * h);
    uint16_t *raw = (uint16_t *)pdata;

    LOG_MSG_VERBOSE("raw[%x:%x:%x:%x:%x:%x:%x:%x]", raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7]);
    for (i = 0; i < size; i++) {
        sum = sum + raw[i];
    }

    return (uint16_t)(sum / size);
}

static int32_t _bmp_16bit_convert_gray(void *pdst, void *psrc, uint32_t w, uint32_t h)
{
    uint32_t i = 0;
    uint16_t *raw_pixel = (uint16_t *)psrc;
    uint8_t *bmp_pixel = (uint8_t *)pdst;
    uint32_t size = (w * h);
    int32_t pixel_val = 0;

    uint16_t wl = 0;
    double fac = 0.0;
    double fac_min = 0.0;
    double fac_max = 0.0;

    if ((pdst == NULL) || (psrc == NULL) || (w == 0) || (h == 0)) {
        return -1;
    }

    wl = _bmp_16bit_get_average(psrc, w, h);
    fac_min = (2 * wl - m_img_16bit_ww) / 2.0 + 0.5;
    fac_max = (2 * wl + m_img_16bit_ww) / 2.0 + 0.5;
    fac = 255.0 / (double)(fac_max - fac_min);

    LOG_MSG_DEBUG("ww = %u, wl = %u, fac_min = %f, fac_max = %f, fac = %f", m_img_16bit_ww, wl, fac_min, fac_max, fac);

    for (i = 0; i < size; i++) {
        if (raw_pixel[i] < fac_min) {
            bmp_pixel[i] = 0;
            continue;
        }

        if (raw_pixel[i] > fac_max) {
            bmp_pixel[i] = 255;
            continue;
        }

        pixel_val = (raw_pixel[i] - fac_min) * fac;
        if (pixel_val < 0) {
            bmp_pixel[i] = 0;
        } else if (pixel_val > 255) {
            bmp_pixel[i] = 255;
        } else {
            bmp_pixel[i] = pixel_val;
        }
    }

    return size;
}

static void _bmp_img_scale(void *dst, int32_t dst_width, int32_t dst_height, void *src, int32_t width, int32_t height, uint8_t bytes_per_pixel)
{
    uint8_t *in_array = (uint8_t *)src;
    uint8_t *out_array = (uint8_t *)dst;

    int32_t w = 0;
    int32_t h = 0;
    int32_t pixel_point = 0;
    int32_t i = 0;

    double d_orig_img_w = 0.0;
    double d_orig_img_h = 0.0;
    int32_t i_orig_img_w = 0;
    int32_t i_orig_img_h = 0;

    double distance_to_a_x = 0.0;
    double distance_to_a_y = 0.0;

    int32_t orig_point_a = 0;
    int32_t orig_point_b = 0;
    int32_t orig_point_c = 0;
    int32_t orig_point_d = 0;

    if (!_bmp_is_rgb_mode(bytes_per_pixel)) {
        bytes_per_pixel = BMP_DATA_BYTE_8BIT;
    }

    int32_t fixed_w = _bmp_get_correct_align_width(width, bytes_per_pixel);
    int32_t out_fixed_w = _bmp_get_correct_align_width(dst_width, bytes_per_pixel);

    for (h = 0; h < dst_height; h++) {
        d_orig_img_h = ((double)h + 0.5) * height / (double)dst_height - 0.5;
        i_orig_img_h = d_orig_img_h;
        distance_to_a_y = d_orig_img_h - i_orig_img_h;

        for (w = 0; w < dst_width; w++) {
            d_orig_img_w = ((double)w + 0.5) * width / (double)dst_width - 0.5;
            i_orig_img_w = d_orig_img_w;
            distance_to_a_x = d_orig_img_w - i_orig_img_w;

            orig_point_a = i_orig_img_h * fixed_w + i_orig_img_w * bytes_per_pixel;
            orig_point_b = i_orig_img_h * fixed_w + (i_orig_img_w + 1) * bytes_per_pixel;
            orig_point_c = (i_orig_img_h + 1) * fixed_w + i_orig_img_w * bytes_per_pixel;
            orig_point_d = (i_orig_img_h + 1) * fixed_w + (i_orig_img_w + 1) * bytes_per_pixel;

            if (i_orig_img_h >= height - 1) {
                orig_point_c = orig_point_a;
                orig_point_d = orig_point_b;
            }
            if (i_orig_img_w >= width - 1) {
                orig_point_b = orig_point_a;
                orig_point_d = orig_point_c;
            }

            pixel_point = h * out_fixed_w + w * bytes_per_pixel;
            for (i = 0; i < bytes_per_pixel; i++) {
                out_array[pixel_point + i] =
                    in_array[orig_point_a + i] * (1 - distance_to_a_x) * (1 - distance_to_a_y) +
                    in_array[orig_point_b + i] * distance_to_a_x * (1 - distance_to_a_y) +
                    in_array[orig_point_c + i] * distance_to_a_y * (1 - distance_to_a_x) +
                    in_array[orig_point_d + i] * distance_to_a_y * distance_to_a_x;
            }
        }
    }
}

/*********************************************************
 * bmp image filling
 */
static void _bmp_init_8bit_palette(void)
{
    uint32_t i = 0;

    if (!m_color_inited) {
        for (i = 0; i < 256; i ++) {
            m_color_map[i*4] = m_color_map[i*4+1] = m_color_map[i*4+2] = i;
            m_color_map[i*4+3] = 0;
        }
        m_color_inited = 1;
    }
}

static int32_t _bmp_get_img_fill_bmp_header_data(void *dest, uint32_t size, uint32_t w, uint32_t h, uint8_t bytes_per_pixel, uint32_t more_exts_size, uint16_t reserved)
{
    uint8_t *pdest = (uint8_t *)dest;

    bitmap_file_header_t bf;
    {
        bf.bf_type = BMP_TYPE;
        bf.bf_size = _bmp_get_normal_size(w, h, bytes_per_pixel);
        bf.bf_reserved1 = 0;
        bf.bf_reserved2 = reserved;
        bf.bf_offbits = BITMAP_IMAGE_HEADER_SIZE;
    };
    bitmap_info_header_t bi;
    {
        bi.bi_size = sizeof(bi);
        bi.bi_width = w;
        bi.bi_height = h;
        bi.bi_planes = 1;
        bi.bi_bitcount = 8;
        bi.bi_compression = 0;
        bi.bi_size_image = 0;
        bi.bi_x_pels_per_meter = 0;
        bi.bi_y_pels_per_meter = 0;
        bi.bi_clr_used = 256;
        bi.bi_clr_important = 256;
    };

    bf.bf_size += more_exts_size;
    bf.bf_offbits += more_exts_size;

    if (_bmp_is_rgb_mode(bytes_per_pixel)) {
        bi.bi_bitcount = _bmp_get_bitcount(bytes_per_pixel);
        bi.bi_clr_used = 0;
        bi.bi_clr_important = 0;
    } else {
        _bmp_init_8bit_palette();
        bf.bf_offbits += BITMAP_IMAGE_8BIT_PALETTE_SIZE;
    }

    if (dest == NULL || size < bf.bf_size || w == 0 || h == 0 || bytes_per_pixel == 0) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, bf_size=%d, w=%d, h=%d, bytes_per_pixel=%d", size, bf.bf_size, w, h, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    LOG_MSG_ERROR("img file size: %u", bf.bf_size);

    if ((m_img_16bit_type != IMG_16BIT_DATA_NORMAL) && (bytes_per_pixel == BMP_DATA_BYTE_16BIT)) {
        bf.bf_reserved1 = m_img_16bit_type;
    }

    memcpy(pdest, &bf, sizeof(bf));
    pdest += sizeof(bf);

    memcpy(pdest, &bi, sizeof(bi));
    pdest += sizeof(bi);

    if (!_bmp_is_rgb_mode(bytes_per_pixel)) {
        memcpy(pdest, &m_color_map, sizeof(m_color_map));
        pdest += sizeof(m_color_map);
    }

    return (pdest - (uint8_t *)dest);
}

static int32_t _bmp_get_img_fill_ext_data(void *dest, uint32_t size, uint32_t offset, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, uint8_t header_only)
{
    uint32_t checksum = 0;
    uint8_t *pdest = (uint8_t *)dest;
    uint32_t data_len = offset;

    bitmap_info_ext_header_t bie;
    {
        bie.bie_magic1 = BMP_MAGIC1;
        bie.bie_magic2 = BMP_MAGIC2;
        bie.bie_checksum = 0;
        bie.bie_width = w;
        bie.bie_height = h;
        bie.bie_bitcount = _bmp_get_correct_bitcount(bitcount);
    };

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bie.bie_bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    data_len += BITMAP_IMAGE_EXT_HEADER_SIZE;
    if (!header_only) {
        data_len += raw_data_len;
    }

    if (dest == NULL || size < data_len || buf == NULL || len < raw_data_len || w == 0 || h == 0 || bytes_per_pixel == 0) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, data_size=%d, src_size=%d, raw=%d, w=%d, h=%d, bytes_per_pixel=%d",
                      size, data_len, len, raw_data_len, w, h, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    pdest += offset;

    if (!header_only) {
        _bmp_calc_checksum(buf, raw_data_len, &checksum);
        bie.bie_checksum = checksum;
    }
    memcpy(pdest, &bie, sizeof(bie));
    pdest += sizeof(bie);

    if (!header_only) {
        memcpy(pdest, buf, raw_data_len);
        pdest += raw_data_len;
    }

    return (pdest - (uint8_t *)dest);
}

static int32_t _bmp_get_img_fill_ext2_data(void *dest, uint32_t size, uint32_t offset, const void *extbuf, uint32_t extsize)
{
    uint32_t checksum = 0;
    uint8_t *pdest = (uint8_t *)dest;
    uint32_t data_len = offset;

    bitmap_info_ext2_header_t bie2;
    {
        bie2.bie_magic1 = BMP_EXT2_MAGIC1;
        bie2.bie_magic2 = BMP_EXT2_MAGIC2;
        bie2.bie_size = extsize;
        bie2.bie_checksum = 0;
    }

    if (extbuf != NULL && extsize > 0) {
        data_len += _bmp_get_ext2_size(extsize);
    }

    if (dest == NULL || size < data_len) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, data_size=%d", size, data_len);
        return -SL_ERROR_BAD_PARAMS;
    }

    pdest += offset;

    if (extbuf != NULL && extsize > 0) {
        _bmp_calc_checksum(extbuf, extsize, &checksum);
        bie2.bie_checksum = checksum;
        memcpy(pdest, &bie2, sizeof(bie2));
        pdest += sizeof(bie2);

        memcpy(pdest, extbuf, extsize);
        pdest += extsize;
    }

    return (pdest - (uint8_t *)dest);
}

static int32_t _bmp_get_img_fill_raw_data(void *dest, uint32_t size, uint32_t offset, const void *rawbuf, uint32_t rawsize)
{
    uint32_t checksum = 0;
    uint8_t *pdest = (uint8_t *)dest;
    uint32_t data_len = offset;

    bitmap_info_ext_mul_raw_header_t bie_mul_raw;
    {
        bie_mul_raw.bie_magic1 = BMP_EXT_MUL_RAW_MAGIC1;
        bie_mul_raw.bie_magic2 = BMP_EXT_MUL_RAW_MAGIC2;
        bie_mul_raw.bie_size = rawsize;
        bie_mul_raw.bie_checksum = 0;
    }

    if (rawbuf != NULL && rawsize > 0) {
        data_len += _bmp_get_ext_mul_raw_size(rawsize);
    }

    if (dest == NULL || size < data_len) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, data_size=%d", size, data_len);
        return -SL_ERROR_BAD_PARAMS;
    }

    pdest += offset;

    if (rawbuf != NULL && rawsize > 0) {
        _bmp_calc_checksum(rawbuf, rawsize, &checksum);
        bie_mul_raw.bie_checksum = checksum;
        memcpy(pdest, &bie_mul_raw, sizeof(bie_mul_raw));
        pdest += sizeof(bie_mul_raw);

        memcpy(pdest, rawbuf, rawsize);
        pdest += rawsize;
    }

    return (pdest - (uint8_t *)dest);
}

static int32_t _bmp_get_img_fill_bmp_data(void *dest, uint32_t size, uint32_t offset, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bytes_per_pixel)
{
    uint32_t i = 0;
    uint32_t j = 0;

    uint32_t fixed_w = 0;
    const uint8_t *p = NULL;
    uint8_t *pdest = (uint8_t *)dest;
    uint8_t *grey_buf = NULL;
    uint32_t data_len = offset;

    uint32_t raw_data_len = (w * h * bytes_per_pixel);
    fixed_w = _bmp_get_correct_align_width(w, bytes_per_pixel);
    data_len += fixed_w * h;

    if (dest == NULL || size < data_len || buf == NULL || len < raw_data_len || w == 0 || h == 0 || bytes_per_pixel == 0) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, data_size=%d, src_size=%d, raw=%d, w=%d, h=%d, bytes_per_pixel=%d",
                      size, data_len, len, raw_data_len, w, h, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    p = (const uint8_t *)buf;
    pdest += offset;

    if ((m_img_16bit_type != IMG_16BIT_DATA_NORMAL) && (bytes_per_pixel == BMP_DATA_BYTE_16BIT)) {
        LOG_MSG_VERBOSE("should convert img data");
        grey_buf = malloc(w * h * 2);
        if (grey_buf == NULL) {
            LOG_MSG_ERROR("malloc(%d) fail", w * h * 2);
            return -SL_ERROR_OUT_OF_MEMORY;
        }

        memcpy(grey_buf, p, w * h * 2);
        if ((m_img_16bit_type & IMG_16BIT_DATA_SWAP_MASK) == IMG_16BIT_DATA_SWAP_MASK) {
            _bmp_16bit_swap_data(grey_buf, grey_buf, w, h);
        }
        if ((m_img_16bit_type & IMG_16BIT_DATA_16BIT_GREY_MASK) == IMG_16BIT_DATA_16BIT_GREY_MASK) {
            _bmp_16bit_convert_gray(grey_buf, grey_buf, w, h);
            bytes_per_pixel = BMP_DATA_BYTE_8BIT;
        }
        p = (const uint8_t *)grey_buf;
    }

    p += (w * (h - 1) * bytes_per_pixel);
    for (i = 0; i < h; i++) {
        if (_bmp_is_rgb_mode(bytes_per_pixel)) { // rgb bmp data
            memcpy(pdest, p, w * bytes_per_pixel);
            pdest += w * bytes_per_pixel;
            p -= w * bytes_per_pixel;
        } else if (bytes_per_pixel == BMP_DATA_BYTE_24BIT) { // 24bit data to gray scale image
            for (j = 0; j < w; j++) {
                *pdest = (77 * (*(p + j * bytes_per_pixel + 2)) + 151 * (*(p + j * bytes_per_pixel + 1)) + 28 * (*(p + j * bytes_per_pixel))) >> 8;
                pdest++;
            };
            p -= w * bytes_per_pixel;
        } else if (bytes_per_pixel == BMP_DATA_BYTE_8BIT) {
            memcpy(pdest, p, w);
            pdest += w;
            p -= w;
        } else {
            for (j = 0; j < w; j++) {
                *pdest = *(p + ((j + 1) * bytes_per_pixel - 1));
                pdest++;
            }
            p -= w * bytes_per_pixel;
        }

        if (_bmp_is_rgb_mode(bytes_per_pixel)) {
            if (fixed_w > w * bytes_per_pixel) {
                memset(pdest, 0, fixed_w - w * bytes_per_pixel);
                pdest += (fixed_w - w * bytes_per_pixel);
            }
        } else {
            if (fixed_w > w) {
                memset(pdest, 0, fixed_w - w);
                pdest += (fixed_w - w);
            }
        }
    }

    if (grey_buf != NULL) {
        free(grey_buf);
    }

    return (pdest - (uint8_t *)dest);
}

int32_t silfp_bmp_get_img(void *dest, uint32_t size, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, const void *rawbuf, uint32_t rawsize, uint32_t normal)
{
    int32_t offset = 0;

    uint32_t more_exts_size = 0;

    if (!normal) {
        more_exts_size += _bmp_get_ext_size(w, h, bitcount);
        if (extbuf != NULL && extsize > 0) {
            more_exts_size += _bmp_get_ext2_size(extsize);
        }
        if (rawbuf != NULL && rawsize > 0) {
            more_exts_size += _bmp_get_ext_mul_raw_size(rawsize);
        }
    }

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    bitcount = _bmp_get_bitcount(bytes_per_pixel);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    if (dest == NULL || size == 0 || buf == NULL || len < raw_data_len || w == 0 || h == 0 || bytes_per_pixel == 0) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, src_size=%d, raw=%d, w=%d, h=%d, bytes_per_pixel=%d",
                      size, len, raw_data_len, w, h, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    offset = _bmp_get_img_fill_bmp_header_data(dest, size, w, h, bytes_per_pixel, more_exts_size, 0);
    if (offset < 0) {
        return offset;
    }

    if (!normal) {
        offset = _bmp_get_img_fill_ext_data(dest, size, offset, buf, len, w, h, bitcount, 0);
        if (offset < 0) {
            return offset;
        }

        offset = _bmp_get_img_fill_ext2_data(dest, size, offset, extbuf, extsize);
        if (offset < 0) {
            return offset;
        }

        offset = _bmp_get_img_fill_raw_data(dest, size, offset, rawbuf, rawsize);
        if (offset < 0) {
            return offset;
        }
    }

    offset = _bmp_get_img_fill_bmp_data(dest, size, offset, buf, len, w, h, bytes_per_pixel);
    LOG_MSG_VERBOSE("get img data size: %d", offset);

    return offset;
}

static int32_t _bmp_mraw_preview_get_img(void *dest, uint32_t size, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize)
{
    uint8_t *pdest = (uint8_t *)dest;
    int32_t offset = 0;

    uint32_t more_exts_size = 0;
    uint32_t scale_width = w / BMP_DATA_SCALE_IN;
    uint32_t scale_height = h / BMP_DATA_SCALE_IN;

    void *img_data = NULL;
    int32_t img_data_size = 0;
    uint32_t fixed_w = 0;
    uint32_t scale_fixed_w = 0;

    more_exts_size += BITMAP_IMAGE_EXT_HEADER_SIZE;
    if (extbuf != NULL && extsize > 0) {
        more_exts_size += _bmp_get_ext2_size(extsize);
    }
    if (buf != NULL && len > 0) {
        more_exts_size += _bmp_get_ext_mul_raw_size(len);
    }

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    bitcount = _bmp_get_bitcount(bytes_per_pixel);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    if (dest == NULL || size == 0 || buf == NULL || len < raw_data_len || w == 0 || h == 0 || bytes_per_pixel == 0) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, src_size=%d, raw=%d, w=%d, h=%d, bytes_per_pixel=%d",
                      size, len, raw_data_len, w, h, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    offset = _bmp_get_img_fill_bmp_header_data(dest, size, scale_width, scale_height, bytes_per_pixel, more_exts_size, BMP_DATA_SCALE_MAGIC);
    if (offset < 0) {
        return offset;
    }

    offset = _bmp_get_img_fill_ext_data(dest, size, offset, buf, len, w, h, bitcount, 1);
    if (offset < 0) {
        return offset;
    }

    offset = _bmp_get_img_fill_ext2_data(dest, size, offset, extbuf, extsize);
    if (offset < 0) {
        return offset;
    }

    offset = _bmp_get_img_fill_raw_data(dest, size, offset, buf, len);
    if (offset < 0) {
        return offset;
    }

    // get preview image data
    fixed_w = _bmp_get_correct_align_width(w, bytes_per_pixel);
    scale_fixed_w = _bmp_get_correct_align_width(scale_width, bytes_per_pixel);

    if (size < offset + scale_fixed_w * h) {
        LOG_MSG_ERROR("buf not enough %u but %u", offset + scale_fixed_w * h, size);
        return -SL_ERROR_BAD_PARAMS;
    }

    img_data_size = fixed_w * h;
    img_data = malloc(img_data_size);
    if (img_data == NULL) {
        LOG_MSG_ERROR("malloc(%d) fail", img_data_size);
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memset(img_data, 0, img_data_size);
    img_data_size = _bmp_get_img_fill_bmp_data(img_data, img_data_size, 0, buf, len, w, h, bytes_per_pixel);
    if (img_data_size < 0) {
        free(img_data);
        return img_data_size;
    }

    pdest += offset;
    _bmp_img_scale(pdest, scale_width, scale_height, img_data, w, h, bytes_per_pixel);
    offset += scale_fixed_w * h;
    free(img_data);

    LOG_MSG_VERBOSE("get mraw preview img data size: %d", offset);

    return offset;
}

static int32_t _bmp_mraw_only_get_img(void *dest, uint32_t size, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize)
{
    int32_t offset = 0;

    uint32_t more_exts_size = 0;

    more_exts_size += BITMAP_IMAGE_EXT_HEADER_SIZE;
    if (extbuf != NULL && extsize > 0) {
        more_exts_size += _bmp_get_ext2_size(extsize);
    }
    if (buf != NULL && len > 0) {
        more_exts_size += _bmp_get_ext_mul_raw_size(len);
    }

    if (dest == NULL || size < more_exts_size || buf == NULL || len == 0 || w == 0 || h == 0) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, data_size=%d, src_size=%d, w=%d, h=%d", size, more_exts_size, len, w, h);
        return -SL_ERROR_BAD_PARAMS;
    }

    offset = _bmp_get_img_fill_ext_data(dest, size, offset, buf, len, w, h, bitcount, 1);
    if (offset < 0) {
        return offset;
    }

    offset = _bmp_get_img_fill_ext2_data(dest, size, offset, extbuf, extsize);
    if (offset < 0) {
        return offset;
    }

    offset = _bmp_get_img_fill_raw_data(dest, size, offset, buf, len);
    if (offset < 0) {
        return offset;
    }

    LOG_MSG_VERBOSE("get mraw only img data size: %d", offset);

    return offset;
}

/*********************************************************
 * bmp image parse
 */
static int32_t _bmp_parse_bmp_header_data(uint32_t w, uint32_t h, const void *buf, uint32_t len, uint32_t *bf_offbits, uint8_t *bi_bitcount, uint16_t reserved)
{
    const uint8_t *p = (const uint8_t *)buf;
    uint32_t fixed_w = 0;
    int32_t offset = 0;
    uint8_t bytes_per_pixel = 0;

    if (w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: w=%d, h=%d, src_size=%d", w, h, len);
        return -SL_ERROR_BAD_PARAMS;
    }

    bitmap_file_header_t *bf = (bitmap_file_header_t *)p;
    bitmap_info_header_t *bi = (bitmap_info_header_t *)(p + sizeof(bitmap_file_header_t));

    if (bf->bf_type != BMP_TYPE || ((bi->bi_width != w || bi->bi_height != h) && (bf->bf_reserved2 != BMP_DATA_SCALE_MAGIC)) || bi->bi_bitcount == 0) {
        LOG_MSG_ERROR("data invalid: bf_type=0x%x, bi_width=(%d:%d), bi_height=(%d:%d), bi_bitcount=%d, flag:0x%x",
                      bf->bf_type, bi->bi_width, w, bi->bi_height, h, bi->bi_bitcount, bf->bf_reserved2);
        return -SL_ERROR_BAD_PARAMS;
    }

    if ((reserved == BMP_DATA_SCALE_MAGIC) && (reserved != bf->bf_reserved2)) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (bf->bf_offbits < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: bf_offbits=%d less then %u", bf->bf_offbits, BITMAP_IMAGE_HEADER_SIZE);
        return -SL_ERROR_BAD_PARAMS;
    }

    bytes_per_pixel = _bmp_get_bytes_per_pixel(bi->bi_bitcount);
    if (bytes_per_pixel == BMP_DATA_BYTE_8BIT) {
        if (bf->bf_offbits < BITMAP_IMAGE_HEADER_SIZE + BITMAP_IMAGE_8BIT_PALETTE_SIZE) {
            LOG_MSG_ERROR("data invalid: bf_offbits=%d less then %u", bf->bf_offbits, BITMAP_IMAGE_HEADER_SIZE + BITMAP_IMAGE_8BIT_PALETTE_SIZE);
            return -SL_ERROR_BAD_PARAMS;
        }
    }

    fixed_w = _bmp_get_align_width(bi->bi_width, bytes_per_pixel);
    if ((len < fixed_w * bi->bi_height + bf->bf_offbits)) {
        LOG_MSG_ERROR("data invalid: bf_offbits=%d, len=%d (%dx%d)", bf->bf_offbits, len, fixed_w, bi->bi_height);
        return -SL_ERROR_BAD_PARAMS;
    }

    offset = BITMAP_IMAGE_HEADER_SIZE;
    if (bytes_per_pixel == BMP_DATA_BYTE_8BIT) {
        offset += BITMAP_IMAGE_8BIT_PALETTE_SIZE;
    }

    if (bf_offbits != NULL) {
        *bf_offbits = bf->bf_offbits;
    }
    if (bi_bitcount != NULL) {
        *bi_bitcount = bi->bi_bitcount;
    }

    LOG_MSG_ERROR("img file size: %u", bf->bf_size);

    return offset;
}

static int32_t _bmp_parse_ext_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount, uint32_t bf_offbits, uint32_t offset)
{
    uint32_t checksum = 0xFFFFFFFF;

    const uint8_t *p = (const uint8_t *)buf;
    bitmap_info_ext_header_t *bie = (bitmap_info_ext_header_t *)(p + offset);

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);
    bitcount = _bmp_get_bitcount(bytes_per_pixel);

    if ((len < offset + BITMAP_IMAGE_EXT_HEADER_SIZE) || (bf_offbits < offset + BITMAP_IMAGE_EXT_HEADER_SIZE)) { // not have ext data
        return 0;
    }

    if ((bie->bie_magic1 != BMP_MAGIC1) || (bie->bie_magic2 != BMP_MAGIC2) || (bie->bie_width != w) || (bie->bie_height != h) || (bie->bie_bitcount != bitcount)) {
        LOG_MSG_ERROR("no ext data: bie_magic1=0x%x, bie_magic2=0x%x, bie_width=(%d:%d), bie_height=(%d:%d), bie_bitcount=(%d:%d)",
                      bie->bie_magic1, bie->bie_magic2, bie->bie_width, w, bie->bie_height, h, bie->bie_bitcount, bitcount);
        return 0;
    }

    if (bf_offbits < offset + _bmp_get_ext_size(w, h, bitcount)) { // have ext, but data invalid
        LOG_MSG_ERROR("data invalid: bf_offbits=%d less then %u", bf_offbits, offset + _bmp_get_ext_size(w, h, bitcount));
        return -SL_ERROR_BAD_PARAMS;
    }

    p += (offset + BITMAP_IMAGE_EXT_HEADER_SIZE);
    _bmp_calc_checksum(p, raw_data_len, &checksum);
    if (checksum != bie->bie_checksum) {
        LOG_MSG_ERROR("verify ext data invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (dest != NULL) {
        if (size < raw_data_len) {
            LOG_MSG_ERROR("not have enough buffer %d, but %d", raw_data_len, size);
            return -SL_ERROR_BAD_PARAMS;
        }

        memcpy(dest, p, raw_data_len);
        LOG_MSG_VERBOSE("get one raw data in ext");
    }

    return _bmp_get_ext_size(w, h, bitcount);
}

static int32_t _bmp_mraw_preview_parse_ext_data(uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount, uint32_t bf_offbits, uint32_t offset)
{
    const uint8_t *p = (const uint8_t *)buf;
    bitmap_info_ext_header_t *bie = (bitmap_info_ext_header_t *)(p + offset);

    if ((len < BITMAP_IMAGE_EXT_HEADER_SIZE) || (bf_offbits < offset + BITMAP_IMAGE_EXT_HEADER_SIZE)) {
        return -SL_ERROR_BAD_PARAMS;
    }

    bitcount = _bmp_get_correct_bitcount(bitcount);
    if ((bie->bie_magic1 != BMP_MAGIC1) || (bie->bie_magic2 != BMP_MAGIC2) || (bie->bie_width != w) || (bie->bie_height != h) || (bie->bie_bitcount != bitcount)) {
        LOG_MSG_ERROR("mraw no ext data: bie_magic1=0x%x, bie_magic2=0x%x, bie_width=(%d:%d), bie_height=(%d:%d), bie_bitcount=(%d:%d)",
                      bie->bie_magic1, bie->bie_magic2, bie->bie_width, w, bie->bie_height, h, bie->bie_bitcount, bitcount);
        return -SL_ERROR_BAD_PARAMS;
    }

    return BITMAP_IMAGE_EXT_HEADER_SIZE;
}

static int32_t _bmp_parse_ext2_data(void *dest, uint32_t size, const void *buf, uint32_t len, uint32_t bf_offbits, uint32_t offset)
{
    uint32_t checksum = 0xFFFFFFFF;

    const uint8_t *p = (const uint8_t *)buf;
    bitmap_info_ext2_header_t *bie2 = (bitmap_info_ext2_header_t *)(p + offset);

    if ((len < offset + BITMAP_IMAGE_EXT2_HEADER_SIZE) || (bf_offbits < offset + BITMAP_IMAGE_EXT2_HEADER_SIZE)) { // not have ext2 data
        return 0;
    }

    if (bie2->bie_magic1 != BMP_EXT2_MAGIC1 || bie2->bie_magic2 != BMP_EXT2_MAGIC2 || bie2->bie_size <= 0) {
        LOG_MSG_ERROR("no ext data: bie2_magic1=0x%x, bie2_magic2=0x%x, size=%d", bie2->bie_magic1, bie2->bie_magic2, bie2->bie_size);
        return 0;
    }

    if (bf_offbits < offset + _bmp_get_ext2_size(bie2->bie_size)) { // have ext2, but data invalid
        LOG_MSG_ERROR("data invalid: bf_offbits=%d less then %u", bf_offbits, offset + _bmp_get_ext2_size(bie2->bie_size));
        return -SL_ERROR_BAD_PARAMS;
    }

    p += (offset + BITMAP_IMAGE_EXT2_HEADER_SIZE);
    _bmp_calc_checksum(p, bie2->bie_size, &checksum);
    if (checksum != bie2->bie_checksum) {
        LOG_MSG_ERROR("verify invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (dest != NULL) {
        if (size < bie2->bie_size) {
            LOG_MSG_ERROR("not have enough buffer %d, but %d", bie2->bie_size, size);
            return -SL_ERROR_BAD_PARAMS;
        }

        memcpy(dest, p, bie2->bie_size);
        LOG_MSG_VERBOSE("get ext2 data, size=%u", bie2->bie_size);
    }

    return _bmp_get_ext2_size(bie2->bie_size);
}

static int32_t _bmp_parse_mul_raw_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount, uint32_t bf_offbits, uint32_t offset)
{
    uint32_t checksum = 0xFFFFFFFF;

    const uint8_t *p = (const uint8_t *)buf;
    bitmap_info_ext_mul_raw_header_t *bie_mul_raw = (bitmap_info_ext_mul_raw_header_t *)(p + offset);

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    if ((len < offset + BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE) || (bf_offbits < offset + BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE)) { // not have mul_raw data
        return 0;
    }

    if (bie_mul_raw->bie_magic1 != BMP_EXT_MUL_RAW_MAGIC1 || bie_mul_raw->bie_magic2 != BMP_EXT_MUL_RAW_MAGIC2 || bie_mul_raw->bie_size <= 0) {
        return 0;
    }

    if (bf_offbits < offset + _bmp_get_ext_mul_raw_size(bie_mul_raw->bie_size)) { // have mul_raw, but data invalid
        LOG_MSG_ERROR("data invalid: bf_offbits=%d less then %u", bf_offbits, offset + _bmp_get_ext_mul_raw_size(bie_mul_raw->bie_size));
        return -SL_ERROR_BAD_PARAMS;
    }

    p += (offset + BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE);
    _bmp_calc_checksum(p, bie_mul_raw->bie_size, &checksum);
    if (checksum != bie_mul_raw->bie_checksum) {
        LOG_MSG_ERROR("verify invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if ((bie_mul_raw->bie_size % raw_data_len) != 0) {
        LOG_MSG_ERROR("data invalid: len=%u, raw_data_len=%u", bie_mul_raw->bie_size, raw_data_len);
        return -SL_ERROR_BAD_PARAMS;
    }

    if (dest != NULL) {
        if (size < bie_mul_raw->bie_size) {
            LOG_MSG_ERROR("buf not enough %u but %u", bie_mul_raw->bie_size, size);
            return -SL_ERROR_BAD_PARAMS;
        }

        memcpy(dest, p, bie_mul_raw->bie_size);
    }

    return _bmp_get_ext_mul_raw_size(bie_mul_raw->bie_size);
}

static int32_t _bmp_parse_img_data(void *dest, uint32_t size, uint32_t w, uint32_t h, uint32_t offset, const void *buf, uint32_t len, uint32_t bitcount)
{
    const uint8_t *p = (const uint8_t *)buf;
    uint8_t *pdest = (uint8_t *)dest;
    uint32_t i = 0;

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);
    bitcount = _bmp_get_bitcount(bytes_per_pixel);
    uint32_t fixed_w = _bmp_get_align_width(w, bytes_per_pixel);

    if (dest == NULL || size < raw_data_len || w == 0 || h == 0 || buf == NULL || len < offset + fixed_w * h) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, raw_data_len=%d, w=%d, h=%d, src_size=%d, data_len=%d", size, raw_data_len, w, h, len, offset + fixed_w * h);
        return -SL_ERROR_BAD_PARAMS;
    }

    p += offset + (fixed_w * (h - 1));
    if (bytes_per_pixel == BMP_DATA_BYTE_8BIT) {
        for (i = 0; i < h; i++) {
            memcpy(pdest, p, w);
            pdest += w;
            p -= fixed_w;
        }
    } else {
        for (i = 0; i < h; i++) {
            memcpy(pdest, p, w * bytes_per_pixel);
            pdest += w * bytes_per_pixel;
            p -= fixed_w;
        }
    }

    LOG_MSG_VERBOSE("get one raw data in bmp data");

    return raw_data_len;
}

int32_t silfp_bmp_get_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    int32_t ret = 0;
    uint32_t bf_offbits = 0;
    uint8_t bi_bitcount = 0;
    uint32_t data_offset = 0;
    uint32_t all_data_len = 0;

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);
    bitcount = _bmp_get_bitcount(bytes_per_pixel);

    uint8_t *pdest = (uint8_t *)dest;

    LOG_MSG_VERBOSE("bmp img parse");

    if (dest == NULL || size < raw_data_len || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, raw_data_len=%d, w=%d, h=%d, src_size=%d, bytes_per_pixel=%d", size, raw_data_len, w, h, len, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _bmp_parse_bmp_header_data(w, h, buf, len, &bf_offbits, &bi_bitcount, 0);
    if (ret < 0) { // bmp data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset = ret;

    ret = _bmp_parse_ext_data(dest, size, w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret < 0) { // ext data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    if (ret == 0) { // no ext data, try to get the raw data from bmp data
        if (bi_bitcount != bitcount) {
            LOG_MSG_ERROR("data invalid: bf_offbits=%d bi_bitcount=(%d:%d)", bf_offbits, bi_bitcount, bitcount);
            return -SL_ERROR_BAD_PARAMS;
        }

        ret = _bmp_parse_img_data(dest, size, w, h, bf_offbits, buf, len, bitcount);
        if (ret < 0) {
            return -SL_ERROR_BAD_PARAMS;
        }

        return ret;
    }

    all_data_len = (ret - BITMAP_IMAGE_EXT_HEADER_SIZE);
    ret = _bmp_parse_ext2_data(NULL, 0, buf, len, bf_offbits, data_offset);
    if (ret < 0) { // ext2 data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_mul_raw_data(pdest + all_data_len, size - all_data_len, w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret < 0) { // mul_raw data invalid
        return -SL_ERROR_BAD_PARAMS;
    }

    if (ret > 0) {
        ret -= BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE;
    }
    ret += all_data_len;

    return ret;
}

static int32_t _bmp_mraw_preview_get_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    int32_t ret = 0;
    uint32_t bf_offbits = 0;
    uint8_t bi_bitcount = 0;
    uint32_t data_offset = 0;

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    LOG_MSG_VERBOSE("mraw_preview parse");

    if (dest == NULL || size < raw_data_len || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, raw_data_len=%d, w=%d, h=%d, src_size=%d, bytes_per_pixel=%d", size, raw_data_len, w, h, len, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _bmp_parse_bmp_header_data(w, h, buf, len, &bf_offbits, &bi_bitcount, BMP_DATA_SCALE_MAGIC);
    if (ret < 0) { // bmp data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset = ret;

    ret = _bmp_mraw_preview_parse_ext_data(w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret < 0) { // ext data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_ext2_data(NULL, 0, buf, len, bf_offbits, data_offset);
    if (ret < 0) { // ext2 data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_mul_raw_data(dest, size, w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret <= 0) { // mul_raw data invalid
        return -SL_ERROR_BAD_PARAMS;
    }

    return ret - BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE;
}

static int32_t _bmp_mraw_only_get_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    int32_t ret = 0;
    uint32_t bf_offbits = len;
    uint32_t data_offset = 0;

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    LOG_MSG_VERBOSE("mraw_only parse");

    if (dest == NULL || size < raw_data_len || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_EXT_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, raw_data_len=%d, w=%d, h=%d, src_size=%d, bytes_per_pixel=%d", size, raw_data_len, w, h, len, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _bmp_mraw_preview_parse_ext_data(w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret < 0) { // ext data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_ext2_data(NULL, 0, buf, len, bf_offbits, data_offset);
    if (ret < 0) { // ext2 data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_mul_raw_data(dest, size, w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret <= 0) { // mul_raw data invalid
        return -SL_ERROR_BAD_PARAMS;
    }

    return ret - BITMAP_IMAGE_EXT_MUL_RAW_HEADER_SIZE;
}

int32_t silfp_bmp_get_data_ext(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount, uint32_t *img_count)
{
    int32_t ret = 0;

    uint32_t bytes_per_pixel = _bmp_get_bytes_per_pixel(bitcount);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);
    bitcount = _bmp_get_bitcount(bytes_per_pixel);

    LOG_MSG_VERBOSE("get raw data start");

    ret = _bmp_mraw_only_get_data(dest, size, w, h, buf, len, bitcount);
    if (ret > 0) {
        LOG_MSG_ERROR("get mraw_only data: ");
    }

    if (ret <= 0) {
        ret = _bmp_mraw_preview_get_data(dest, size, w, h, buf, len, bitcount);
        if (ret > 0) {
            LOG_MSG_ERROR("get mraw_preview data: ");
        }
    }

    if (ret <= 0) {
        ret = silfp_bmp_get_data(dest, size, w, h, buf, len, bitcount);
        if (ret > 0) {
            LOG_MSG_ERROR("get img raw data: ");
        }
    }

    LOG_MSG_VERBOSE("get raw data (ret: %d) end", ret);

    if (ret <= 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (img_count != NULL) {
        *img_count = ret / raw_data_len;
    }

    LOG_MSG_DEBUG("size:%u count:%d, raw_len:%d", ret, (img_count != NULL) ? *img_count : 0, raw_data_len);

    return ret;
}

static int32_t _bmp_get_ext2(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    int32_t ret = 0;
    uint32_t bf_offbits = 0;
    uint8_t bi_bitcount = 0;
    uint32_t data_offset = 0;

    if (dest == NULL || size == 0 || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, w=%d, h=%d, src_size=%d", size, w, h, len);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _bmp_parse_bmp_header_data(w, h, buf, len, &bf_offbits, &bi_bitcount, 0);
    if (ret < 0) { // bmp data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset = ret;

    ret = _bmp_parse_ext_data(NULL, 0, w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret < 0) { // ext data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_ext2_data(dest, size, buf, len, bf_offbits, data_offset);
    if (ret <= 0) { // ext2 data invalid
        return -SL_ERROR_BAD_PARAMS;
    }

    return ret - BITMAP_IMAGE_EXT2_HEADER_SIZE;
}

static int32_t _bmp_mraw_preview_get_ext2(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    int32_t ret = 0;
    uint32_t bf_offbits = 0;
    uint8_t bi_bitcount = 0;
    uint32_t data_offset = 0;

    if (dest == NULL || size == 0 || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, w=%d, h=%d, src_size=%d", size, w, h, len);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _bmp_parse_bmp_header_data(w, h, buf, len, &bf_offbits, &bi_bitcount, BMP_DATA_SCALE_MAGIC);
    if (ret < 0) { // bmp data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset = ret;

    ret = _bmp_mraw_preview_parse_ext_data(w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret < 0) { // ext data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_ext2_data(dest, size, buf, len, bf_offbits, data_offset);
    if (ret <= 0) { // ext2 data invalid
        return -SL_ERROR_BAD_PARAMS;
    }

    return ret - BITMAP_IMAGE_EXT2_HEADER_SIZE;
}

static int32_t _bmp_mraw_only_get_ext2(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    int32_t ret = 0;
    uint32_t bf_offbits = 0;
    uint32_t data_offset = 0;

    if (dest == NULL || size == 0 || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_EXT_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, w=%d, h=%d, src_size=%d", size, w, h, len);
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _bmp_mraw_preview_parse_ext_data(w, h, buf, len, bitcount, bf_offbits, data_offset);
    if (ret < 0) { // ext data invalid
        return -SL_ERROR_BAD_PARAMS;
    }
    data_offset += ret;

    ret = _bmp_parse_ext2_data(dest, size, buf, len, bf_offbits, data_offset);
    if (ret <= 0) { // ext2 data invalid
        return -SL_ERROR_BAD_PARAMS;
    }

    return ret - BITMAP_IMAGE_EXT2_HEADER_SIZE;
}

int32_t silfp_bmp_get_ext2(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    int32_t ret = 0;

    LOG_MSG_VERBOSE("get ext2 data start");

    ret = _bmp_mraw_only_get_ext2(dest, size, w, h, buf, len, bitcount);
    if (ret < 0) {
        ret = _bmp_mraw_preview_get_ext2(dest, size, w, h, buf, len, bitcount);
    }
    if (ret < 0) {
        ret = _bmp_get_ext2(dest, size, w, h, buf, len, bitcount);
    }

    LOG_MSG_VERBOSE("get ext2 data (len:%d) end", ret);

    return ret;
}

/*********************************************************
 * bmp image get size param
 */
static int32_t _bmp_get_size(const void *buf, uint32_t len, uint32_t *w, uint32_t *h)
{
    const uint8_t *p = (const uint8_t *)buf;

    bitmap_file_header_t *bf = (bitmap_file_header_t *)p;
    bitmap_info_header_t *bi = (bitmap_info_header_t *)(p + sizeof(bitmap_file_header_t));

    if ((buf == NULL) || (len < BITMAP_IMAGE_HEADER_SIZE) || (bf->bf_type != BMP_TYPE)) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (bf->bf_reserved2 == BMP_DATA_SCALE_MAGIC) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (w != NULL) {
        *w = bi->bi_width;
    }
    if (h != NULL) {
        *h = bi->bi_height;
    }

    LOG_MSG_DEBUG("get img width:%u, height:%u", bi->bi_width, bi->bi_height);
    return 0;
}

static int32_t _bmp_mraw_preview_get_size(const void *buf, uint32_t len, uint32_t *w, uint32_t *h)
{
    int32_t ret = 0;
    const uint8_t *p = (const uint8_t *)buf;
    bitmap_info_ext_header_t *bie = NULL;
    uint32_t bf_offbits = 0;

    ret = _bmp_parse_bmp_header_data(-1, -1, buf, len, &bf_offbits, NULL, BMP_DATA_SCALE_MAGIC);
    if (ret < 0) { // bmp data invalid
        return -SL_ERROR_BAD_PARAMS;
    }

    bie = (bitmap_info_ext_header_t *)(p + ret);
    if ((len < BITMAP_IMAGE_EXT_HEADER_SIZE) || (bf_offbits < ret + BITMAP_IMAGE_EXT_HEADER_SIZE)) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if ((bie->bie_magic1 != BMP_MAGIC1) || (bie->bie_magic2 != BMP_MAGIC2)) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (w != NULL) {
        *w = bie->bie_width;
    }
    if (h != NULL) {
        *h = bie->bie_height;
    }

    LOG_MSG_DEBUG("get img width:%u, height:%u", bie->bie_width, bie->bie_height);
    return 0;
}

static int32_t _bmp_mraw_only_get_size(const void *buf, uint32_t len, uint32_t *w, uint32_t *h)
{
    const uint8_t *p = (const uint8_t *)buf;

    bitmap_info_ext_header_t *bie = (bitmap_info_ext_header_t *)p;

    if ((buf == NULL) || (len < BITMAP_IMAGE_EXT_HEADER_SIZE)) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if ((bie->bie_magic1 != BMP_MAGIC1) || (bie->bie_magic2 != BMP_MAGIC2)) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if (w != NULL) {
        *w = bie->bie_width;
    }
    if (h != NULL) {
        *h = bie->bie_height;
    }

    LOG_MSG_DEBUG("get mraw_only width:%u, height:%u", bie->bie_width, bie->bie_height);
    return 0;
}

int32_t silfp_img_get_size(const void *buf, uint32_t len, uint32_t *w, uint32_t *h)
{
    int32_t ret = 0;

    ret = _bmp_mraw_only_get_size(buf, len, w, h);

    if (ret < 0) {
        ret = _bmp_mraw_preview_get_size(buf, len, w, h);
    }

    if (ret < 0) {
        ret = _bmp_get_size(buf, len, w, h);
    }

    return ret;
}

int32_t silfp_bmp_save(const char *path, const char *name, const void *buf, uint32_t size, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, uint32_t normal)
{
    int32_t ret = 0;
    void *pbuff = NULL;
    int32_t len = 0;
    uint32_t count = 0;
    uint32_t img_len = 0;
    const uint8_t *rawbuf = NULL;

    bitcount = _bmp_get_correct_bitcount(bitcount);
    uint32_t bytes_per_pixel = ((bitcount >> BIT_PER_BYTE_SHIFT) & 0xFF);

    if (name == NULL || buf == NULL || size < (w * h) || w == 0 || h == 0) {
        LOG_MSG_ERROR("data invalid: size=%d, w=%d, h=%d", size, w, h);
        return -SL_ERROR_BAD_PARAMS;
    }

    len = silfp_bmp_get_size(w, h, bitcount);
    if (extbuf != NULL && extsize > 0) {
        len += _bmp_get_ext2_size(extsize);
    }

    if ((size % (w * h * bytes_per_pixel)) == 0) {
        img_len = w * h * bytes_per_pixel;
        count = size / img_len;
        if (count > 1) {
            len += _bmp_get_ext_mul_raw_size(size - img_len);
        }
    }

    pbuff = malloc(len);
    if (pbuff == NULL) {
        LOG_MSG_ERROR("malloc fail");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memset(pbuff, 0, len);

    if (count > 1) {
        if (m_img_mraw_preview_enable) {
            len = _bmp_mraw_preview_get_img(pbuff, len, buf, size, w, h, bitcount, extbuf, extsize);
        } else if (m_img_mraw_enable) {
            len = _bmp_mraw_only_get_img(pbuff, len, buf, size, w, h, bitcount, extbuf, extsize);
        } else {
            rawbuf = (const uint8_t *)buf;
            rawbuf += img_len;
            len = silfp_bmp_get_img(pbuff, len, buf, size, w, h, bitcount, extbuf, extsize, rawbuf, size - img_len, normal);
        }
    } else {
        len = silfp_bmp_get_img(pbuff, len, buf, size, w, h, bitcount, extbuf, extsize, NULL, 0, normal);
    }

    if (len > 0) {
        LOG_MSG_INFO("file:%s%s%s", (path == NULL) ? "" : path, (path == NULL) ? "" : "/", name);
        ret = silfp_util_file_save_dump_img(path, name, pbuff, len);
    }

    free(pbuff);
    pbuff = NULL;

    return 0;
}