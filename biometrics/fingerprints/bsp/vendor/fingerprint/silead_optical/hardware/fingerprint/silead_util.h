/******************************************************************************
 * @file   silead_util.h
 * @brief  Contains fingerprint utilities functions header file.
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
 * David Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_UTIL_H__
#define __SILEAD_UTIL_H__

int32_t silfp_util_get_str_value(const char *name, uint8_t v);
int32_t silfp_util_set_str_value(const char *name, uint8_t v);

uint64_t silfp_util_get_seconds(void);
int32_t silfp_util_seconds_to_date(uint64_t seconds, char *datastr, uint32_t len);

int32_t silfp_util_open_file(const char *path, int32_t append);
int32_t silfp_util_write_file(int32_t fd, const void *buf, uint32_t len);
int32_t silfp_util_close_file(int32_t fd);

int32_t silfp_util_strcpy(void *dst, uint32_t size, const void *src, uint32_t len);
int32_t silfp_util_path_copy(void *dst, uint32_t size, const void *src, uint32_t len);

int32_t silfp_util_file_get_size(const char *fname);
int32_t silfp_util_file_remove(const char *fname);
int32_t silfp_util_file_load(const char *path, const char *name, char *buf, uint32_t len);
int32_t silfp_util_file_save(const char *path, const char *name, const char *buf, uint32_t len);
int32_t silfp_util_file_rename(const char *srcname, const char *dstname);

#endif /* __SILEAD_UTIL_H__ */
