/******************************************************************************
 * @file   silead_bmp.c
 * @brief  Contains Bitmap file operate functions.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
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
 * Jack Zhang  2018/4/2    0.1.0      Init version
 * Rich Li     2018/6/7    0.1.1      Support dump image
 *
 *****************************************************************************/

#define FILE_TAG "silead_img"
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_bmp.h"
#include "silead_util.h"

#define BMP_TYPE 0x4d42
#define BMP_MAGIC1 0x0511
#define BMP_MAGIC2 0x1EAD
#define BMP_EXT2_MAGIC1 0x4d4d
#define BMP_EXT2_MAGIC2 0x4242

#define BIT_PER_BYTE_SHIFT 3 // 8 bit per byte
#define BMP_BYTE_ALIGN     4 // 4 bytes alignment
#define BMP_DATA_BYTE_8BIT 1

#define BMP_REBOOT_TIMES_FILE "bmp_times.dat"
#define BMP_REBOOT_TIMES_NO_GET ((uint32_t)-1)

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

// bmp head + bmp info head + color table
#define BITMAP_IMAGE_HEADER_SIZE (256*4 + sizeof(bitmap_file_header_t) + sizeof(bitmap_info_header_t))
#define BITMAP_IMAGE_EXT_HEADER_SIZE (sizeof(bitmap_info_ext_header_t))
#define BITMAP_IMAGE_EXT2_HEADER_SIZE (sizeof(bitmap_info_ext2_header_t))

static unsigned char m_color_map[256*4] = {0};
static int32_t m_color_inited = 0;

static int32_t _bmp_calc_checksum(const void *in, uint32_t len, uint32_t *checksum)
{
    uint32_t i = 0;
    unsigned long int sum = 0;
    const unsigned char *p;

    if (in == NULL || checksum == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    p = (const unsigned char *)in;
    for (i = 0; i < len; i++) {
        sum += *p++;
    }

    while (sum >> 16) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    *checksum = ~sum;

    return SL_SUCCESS;
}

static uint8_t _bmp_get_correct_bitcount(uint8_t bitcount)
{
    uint32_t bit = 0;

    bit = ((bitcount >> BIT_PER_BYTE_SHIFT) & 0xFF);
    if (bit == 0) {
        bit = BMP_DATA_BYTE_8BIT;
    }
    bit = bit << BIT_PER_BYTE_SHIFT;

    return bit;
}

static uint32_t _bmp_get_ext_size(uint32_t w, uint32_t h, uint8_t bitcount)
{
    uint32_t bytes = 0;
    uint32_t len = 0;

    bytes = ((_bmp_get_correct_bitcount(bitcount) >> BIT_PER_BYTE_SHIFT) & 0xFF);
    len = BITMAP_IMAGE_EXT_HEADER_SIZE;
    len += w * h * bytes;

    return len;
}

static uint32_t _bmp_get_ext2_size(uint32_t size)
{
    return (BITMAP_IMAGE_EXT2_HEADER_SIZE + size);
}

static uint32_t _bmp_get_normal_size(uint32_t w, uint32_t h)
{
    uint32_t len = 0;
    uint32_t fixed_w = (w + BMP_BYTE_ALIGN - 1) / BMP_BYTE_ALIGN * BMP_BYTE_ALIGN;

    len = BITMAP_IMAGE_HEADER_SIZE;
    len += fixed_w*h;
    return len;
}

uint32_t silfp_bmp_get_size(uint32_t w, uint32_t h, uint8_t bitcount)
{
    uint32_t len = 0;

    len = _bmp_get_normal_size(w, h);
    len += _bmp_get_ext_size(w, h, bitcount);

    return len;
}

int32_t silfp_bmp_get_img(void *dest, uint32_t size, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, uint32_t normal)
{
    uint32_t i = 0;
    uint32_t j = 0;

    uint32_t fixed_w = 0;
    const unsigned char *p = NULL;
    unsigned char *pdest = (unsigned char *)dest;
    uint32_t checksum = 0;

    bitmap_file_header_t bf;
    {
        bf.bf_type = BMP_TYPE;
        bf.bf_size = _bmp_get_normal_size(w, h);
        bf.bf_reserved1 = 0;
        bf.bf_reserved2 = 0;
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
    bitmap_info_ext_header_t bie;
    {
        bie.bie_magic1 = BMP_MAGIC1;
        bie.bie_magic2 = BMP_MAGIC2;
        bie.bie_checksum = 0;
        bie.bie_width = w;
        bie.bie_height = h;
        bie.bie_bitcount = _bmp_get_correct_bitcount(bitcount);
    };
    bitmap_info_ext2_header_t bie2;
    {
        bie2.bie_magic1 = BMP_EXT2_MAGIC1;
        bie2.bie_magic2 = BMP_EXT2_MAGIC2;
        bie2.bie_size = extsize;
        bie2.bie_checksum = 0;
    }

    if (!normal) {
        bf.bf_size += _bmp_get_ext_size(w, h, bitcount);
        bf.bf_offbits += _bmp_get_ext_size(w, h, bitcount);
        if (extbuf != NULL && extsize > 0) {
            bf.bf_size += _bmp_get_ext2_size(extsize);
            bf.bf_offbits += _bmp_get_ext2_size(extsize);
        }
    }

    uint32_t bytes_per_pixel = ((bie.bie_bitcount >> BIT_PER_BYTE_SHIFT) & 0xFF);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    if (dest == NULL || size < bf.bf_size || buf == NULL || len < raw_data_len || w == 0 || h == 0 || bytes_per_pixel == 0) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, bf_size=%d, src_size=%d, raw=%d, w=%d, h=%d, bytes_per_pixel=%d", size, bf.bf_size, len, raw_data_len, w, h, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    if (!m_color_inited) {
        for (i = 0; i < 256; i ++) {
            m_color_map[i*4] = m_color_map[i*4+1] = m_color_map[i*4+2] = i;
            m_color_map[i*4+3] = 0;
        }
        m_color_inited = 1;
    }

    memcpy(pdest, &bf, sizeof(bf));
    pdest += sizeof(bf);

    memcpy(pdest, &bi, sizeof(bi));
    pdest += sizeof(bi);

    memcpy(pdest, &m_color_map, sizeof(m_color_map));
    pdest += sizeof(m_color_map);

    if (!normal) {
        _bmp_calc_checksum(buf, raw_data_len, &checksum);
        bie.bie_checksum = checksum;
        memcpy(pdest, &bie, sizeof(bie));
        pdest += sizeof(bie);

        memcpy(pdest, buf, raw_data_len);
        pdest += raw_data_len;

        if (extbuf != NULL && extsize > 0) {
            _bmp_calc_checksum(extbuf, extsize, &checksum);
            bie2.bie_checksum = checksum;
            memcpy(pdest, &bie2, sizeof(bie2));
            pdest += sizeof(bie2);

            memcpy(pdest, extbuf, extsize);
            pdest += extsize;
        }
    }

    fixed_w = (w + BMP_BYTE_ALIGN - 1) / BMP_BYTE_ALIGN * BMP_BYTE_ALIGN;
    p = (const unsigned char *)buf;
    p += (w * (h - 1) * bytes_per_pixel);
    for (i = 0; i < h; i++) {
        if (bytes_per_pixel == BMP_DATA_BYTE_8BIT) {
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

        if (fixed_w > w) {
            memset(pdest, 0, fixed_w - w);
            pdest += (fixed_w - w);
        }
    }

    return (pdest - (unsigned char *)dest);
}

int32_t silfp_bmp_get_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    uint32_t fixed_w = 0;
    uint32_t i = 0;
    uint32_t checksum = 0xFFFFFFFF;
    int8_t bmp_ext_exist = -1;

    const unsigned char *p = (const unsigned char *)buf;
    unsigned char *pdest = (unsigned char *)dest;

    bitcount = _bmp_get_correct_bitcount(bitcount);
    uint32_t bytes_per_pixel = ((bitcount >> BIT_PER_BYTE_SHIFT) & 0xFF);
    uint32_t raw_data_len = (w * h * bytes_per_pixel);

    if (pdest == NULL || size < raw_data_len || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: dst_size=%d, raw_data_len=%d, w=%d, h=%d, src_size=%d, bytes_per_pixel=%d", size, raw_data_len, w, h, len, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    bitmap_file_header_t *bf = (bitmap_file_header_t *)p;
    bitmap_info_header_t *bi = (bitmap_info_header_t *)(p + sizeof(bitmap_file_header_t));
    bitmap_info_ext_header_t *bie = (bitmap_info_ext_header_t *)(p + BITMAP_IMAGE_HEADER_SIZE);

    if (bf->bf_type != BMP_TYPE || bi->bi_width != w || bi->bi_height != h || bi->bi_bitcount != 8) {
        LOG_MSG_ERROR("bf_type=0x%x, bi_width=(%d:%d), bi_height=(%d:%d), bi_bitcount=%d", bf->bf_type, bi->bi_width, w, bi->bi_height, h, bi->bi_bitcount);
        return -SL_ERROR_BAD_PARAMS;
    }

    fixed_w = (w + BMP_BYTE_ALIGN - 1) / BMP_BYTE_ALIGN * BMP_BYTE_ALIGN;
    if ((len < fixed_w * h + bf->bf_offbits)) {
        LOG_MSG_ERROR("bf_offbits=%d, len=%d (%dx%d)", bf->bf_offbits, len, fixed_w, h);
        return -SL_ERROR_BAD_PARAMS;
    }

    if ((len >= silfp_bmp_get_size(w, h, bitcount)) && (bf->bf_offbits >= BITMAP_IMAGE_HEADER_SIZE + _bmp_get_ext_size(w, h, bitcount))) { // bmp with extra data
        if ((bie->bie_magic1 == BMP_MAGIC1) && (bie->bie_magic2 == BMP_MAGIC2) && (bie->bie_width == w) && (bie->bie_height == h) && (bie->bie_bitcount == bitcount)) {
            bmp_ext_exist = 1;
        } else {
            LOG_MSG_ERROR("bie_magic1=0x%x, bie_magic2=0x%x, bie_width=(%d:%d), bie_height=(%d:%d), bie_bitcount=(%d:%d)",
                          bie->bie_magic1, bie->bie_magic2, bie->bie_width, w, bie->bie_height, h, bie->bie_bitcount, bitcount);
        }
    } else if ((len >= _bmp_get_normal_size(w, h)) && (bf->bf_offbits >= BITMAP_IMAGE_HEADER_SIZE)) { // normal bmp
        if (bi->bi_bitcount == bitcount) {
            bmp_ext_exist = 0;
        } else {
            LOG_MSG_ERROR("bi_bitcount=(%d:%d)", bi->bi_bitcount, bitcount);
        }
    }

    if (bmp_ext_exist < 0) {
        LOG_MSG_ERROR("verify param invalid: bf_offbits=%d", bf->bf_offbits);
        return -SL_ERROR_BAD_PARAMS;
    }

    LOG_MSG_INFO("bmp extra info %d", bmp_ext_exist);
    if (bmp_ext_exist) {
        p += (BITMAP_IMAGE_HEADER_SIZE + BITMAP_IMAGE_EXT_HEADER_SIZE);
        memcpy(pdest, p, raw_data_len);
        pdest += raw_data_len;

        _bmp_calc_checksum(dest, raw_data_len, &checksum);
        if (checksum != bie->bie_checksum) {
            LOG_MSG_ERROR("verify invalid");
            return -SL_ERROR_BAD_PARAMS;
        }
    } else {
        p += bf->bf_offbits;
        p += (fixed_w * (h - 1));
        for (i = 0; i < h; i++) {
            memcpy(pdest, p, w);
            pdest += w;
            p -= fixed_w;
        }
    }

    return (pdest - (unsigned char *)dest);
}

int32_t silfp_bmp_get_ext2(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount)
{
    uint32_t fixed_w = 0;

    int32_t bmp_ext2_offset = 0;
    uint32_t checksum = 0xFFFFFFFF;
    bitmap_info_ext2_header_t *bie2 = NULL;

    const unsigned char *p = (const unsigned char *)buf;

    bitcount = _bmp_get_correct_bitcount(bitcount);
    uint32_t bytes_per_pixel = ((bitcount >> BIT_PER_BYTE_SHIFT) & 0xFF);

    if (dest == NULL || w == 0 || h == 0 || buf == NULL || len < BITMAP_IMAGE_HEADER_SIZE) {
        LOG_MSG_ERROR("data invalid: w=%d, h=%d, src_size=%d, bytes_per_pixel=%d", w, h, len, bytes_per_pixel);
        return -SL_ERROR_BAD_PARAMS;
    }

    bitmap_file_header_t *bf = (bitmap_file_header_t *)p;
    bitmap_info_header_t *bi = (bitmap_info_header_t *)(p + sizeof(bitmap_file_header_t));

    if (bf->bf_type != BMP_TYPE || bi->bi_width != w || bi->bi_height != h || bi->bi_bitcount != 8) {
        LOG_MSG_ERROR("bf_type=0x%x, bi_width=(%d:%d), bi_height=(%d:%d), bi_bitcount=%d", bf->bf_type, bi->bi_width, w, bi->bi_height, h, bi->bi_bitcount);
        return -SL_ERROR_BAD_PARAMS;
    }

    fixed_w = (w + BMP_BYTE_ALIGN - 1) / BMP_BYTE_ALIGN * BMP_BYTE_ALIGN;
    if ((len < fixed_w * h + bf->bf_offbits)) {
        LOG_MSG_ERROR("bf_offbits=%d, len=%d (%dx%d)", bf->bf_offbits, len, fixed_w, h);
        return -SL_ERROR_BAD_PARAMS;
    }

    bmp_ext2_offset = BITMAP_IMAGE_HEADER_SIZE + _bmp_get_ext_size(w, h, bitcount);
    p += bmp_ext2_offset;
    bie2 = (bitmap_info_ext2_header_t *)p;
    if (bie2->bie_magic1 != BMP_EXT2_MAGIC1 || bie2->bie_magic2 != BMP_EXT2_MAGIC2 || bie2->bie_size <= 0) {
        LOG_MSG_ERROR("bie2_magic1=0x%x, bie2_magic2=0x%x, size=%d", bie2->bie_magic1, bie2->bie_magic2, bie2->bie_size);
        return -SL_ERROR_BAD_PARAMS;
    }

    if (bf->bf_offbits < bmp_ext2_offset + _bmp_get_ext2_size(bie2->bie_size)) {
        LOG_MSG_ERROR("verify param invalid: bf_offbits=%d, offset=%d, size=%d", bf->bf_offbits, bmp_ext2_offset, _bmp_get_ext2_size(bie2->bie_size));
        return -SL_ERROR_BAD_PARAMS;
    }

    p += BITMAP_IMAGE_EXT2_HEADER_SIZE;
    _bmp_calc_checksum(p, bie2->bie_size, &checksum);
    if (checksum != bie2->bie_checksum) {
        LOG_MSG_ERROR("verify invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (size < bie2->bie_size) {
        LOG_MSG_ERROR("not have enough buffer %d, but %d", bie2->bie_size, size);
        return -SL_ERROR_BAD_PARAMS;
    }

    memcpy(dest, p, bie2->bie_size);
    return bie2->bie_size;
}

static uint32_t _bmp_get_reboot_times(const char *dir)
{
    static uint32_t s_reboot_times = BMP_REBOOT_TIMES_NO_GET;

    int32_t ret = 0;
    char path[MAX_PATH_LEN] = {0};
    char *buf = NULL;
    int32_t len = 0;

    if (dir == NULL) {
        return 0;
    }

    if (s_reboot_times == BMP_REBOOT_TIMES_NO_GET) {
        snprintf(path, sizeof(path), "%s/%s", dir, BMP_REBOOT_TIMES_FILE);
        len = silfp_util_file_get_size(path);
        if (len < 0) {
            len = 0;
        }

        buf = malloc(len + 16);
        if (buf != NULL) {
            memset(buf, 0, len + 16);
            if (len > 0) {
                ret = silfp_util_file_load(dir, BMP_REBOOT_TIMES_FILE, buf, len);
                if (ret > 0) {
                    s_reboot_times = strtoul(buf, NULL, 10);
                }
            }
            s_reboot_times++;
            snprintf(buf, sizeof(path), "%u", s_reboot_times);
            silfp_util_file_save(dir, BMP_REBOOT_TIMES_FILE, buf, strlen(buf));
            free(buf);
        }
    }

    if (s_reboot_times == BMP_REBOOT_TIMES_NO_GET) {
        return 0;
    }

    return s_reboot_times;
}

int32_t silfp_bmp_get_save_path(char *path, uint32_t len, const char *dir, const char *prefix)
{
    static int32_t s_bmp_index = 0;
    char datastr[64] = {0};
    uint32_t reboot_times = 0;

    uint64_t second = silfp_util_get_seconds();
    silfp_util_seconds_to_date(second, datastr, sizeof(datastr));

    if (path != NULL && len > 0 && dir != NULL) {
        reboot_times = _bmp_get_reboot_times(dir);
        if (prefix != NULL) {
            snprintf(path, len, "%s/%04u-%04d-%s-%s.bmp", dir, reboot_times, s_bmp_index++, prefix, datastr);
        } else {
            snprintf(path, len, "%s/%04u-%04d-%s.bmp", dir, reboot_times, s_bmp_index++, datastr);
        }
        return 0;
    }

    return -SL_ERROR_BAD_PARAMS;
}

int32_t silfp_bmp_save(const char *path, const void *buf, uint32_t size, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, uint32_t normal)
{
    int32_t ret = 0;
    void *pbuff = NULL;
    int32_t len = 0;
    int32_t fd = -1;

    if (buf == NULL || size < (w * h)) {
        LOG_MSG_ERROR("data invalid: size=%d, w=%d, h=%d", size, w, h);
        return -SL_ERROR_BAD_PARAMS;
    }

    len = silfp_bmp_get_size(w, h, bitcount);
    if (extbuf != NULL && extsize > 0) {
        len += _bmp_get_ext2_size(extsize);
    }

    pbuff = malloc(len);
    if (pbuff == NULL) {
        LOG_MSG_ERROR("malloc fail");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memset(pbuff, 0, len);
    do {
        len = silfp_bmp_get_img(pbuff, len, buf, size, w, h, bitcount, extbuf, extsize, normal);
        if (len <= 0) {
            break;
        }

        fd = silfp_util_open_file(path, 0);
        if (fd < 0) {
            break;
        }
        LOG_MSG_INFO("file:%s", path);

        ret = silfp_util_write_file(fd, pbuff, len);
    } while (0);

    if (fd >= 0) {
        silfp_util_close_file(fd);
    }

    if (pbuff != NULL) {
        free(pbuff);
        pbuff = NULL;
    }
    return 0;
}