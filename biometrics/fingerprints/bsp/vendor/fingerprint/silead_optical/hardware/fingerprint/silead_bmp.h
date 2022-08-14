/******************************************************************************
 * @file   silead_bmp.h
 * @brief  Contains Bitmap operate functions header file.
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

#ifndef __SILEAD_BITMAP_H__
#define __SILEAD_BITMAP_H__

uint32_t silfp_bmp_get_size(uint32_t w, uint32_t h, uint8_t bitcount);
int32_t silfp_bmp_get_img(void *dest, uint32_t size, const void *buf, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, uint32_t normal);
int32_t silfp_bmp_get_data(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount);
int32_t silfp_bmp_get_ext2(void *dest, uint32_t size, uint32_t w, uint32_t h, const void *buf, uint32_t len, uint8_t bitcount);

int32_t silfp_bmp_get_save_path(char *path, uint32_t len, const char *dir, const char *prefix);
int32_t silfp_bmp_save(const char *path, const void *buf, uint32_t size, uint32_t w, uint32_t h, uint8_t bitcount, const void *extbuf, uint32_t extsize, uint32_t normal);

#endif /* __SILEAD_BITMAP_H__ */

