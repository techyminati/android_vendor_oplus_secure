/******************************************************************************
 * @file   silead_bmp.h
 * @brief  Contains Bitmap operate functions header file.
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

#ifndef __SILEAD_BITMAP_H__
#define __SILEAD_BITMAP_H__

#define IMG_16BIT_DATA_NORMAL           0x00
#define IMG_16BIT_DATA_SWAP_MASK        0x01
#define IMG_16BIT_DATA_16BIT_GREY_MASK  0x02
void silfp_bmp_set_img_16bit_type(uint32_t type);
void silfp_bmp_set_img_16bit_ww(uint16_t ww);
void silfp_bmp_set_mraw_preview_enable(uint8_t enable);
void silfp_bmp_set_24bit_rgb_enable(uint8_t enable);

uint32_t silfp_bmp_get_size(uint32_t w, uint32_t h, uint8_t bitcount);
int32_t silfp_bmp_get_img(void *dest, uint32_t size, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, const void *rawbuf, uint32_t rawsize, uint32_t normal);
int32_t silfp_bmp_get_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount);
int32_t silfp_bmp_get_data_ext(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount, uint32_t *img_count);
int32_t silfp_bmp_get_ext2(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount);
int32_t silfp_img_get_size(const void *buf, uint32_t len, uint32_t *w, uint32_t *h);

int32_t silfp_bmp_save(const char *path, const char *name, const void *buf, uint32_t size, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, uint32_t normal);

#endif /* __SILEAD_BITMAP_H__ */