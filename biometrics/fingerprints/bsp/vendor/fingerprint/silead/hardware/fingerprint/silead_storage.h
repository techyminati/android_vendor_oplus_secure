/******************************************************************************
 * @file   silead_storage.h
 * @brief  Contains storage functions header file.
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
 * Luke Ma     2018/4/2    0.1.0      Init version
 * Luke Ma     2018/5/13   0.1.1      Add config load/save I/F
 *
 *****************************************************************************/

#ifndef __SILEAD_STORAGE_H__
#define __SILEAD_STORAGE_H__

#define TPL_MAX_ST 10
#define TPL_MAX	5
#define TPLID_S		0x7F000000  /* Finger ID, range start */
#define TPLID_E		0xFFFFFFFF  /* Finger ID, range end */
#define ID_VALID(x) ((x) >= 128 && (x) <= TPLID_E)

const char *silfp_storage_get_tpl_path();
void silfp_storage_set_tpl_path(const char *path);

int32_t silfp_storage_get_idlist(uint32_t *idlist, int32_t force);
int32_t silfp_storage_save(const char *buf, const uint32_t len, int64_t sec_id, uint32_t *fid);
int32_t silfp_storage_update(const uint32_t id, const char *buf, const uint32_t len);
int32_t silfp_storage_load(const uint32_t id, char *buf, const uint32_t len);
int32_t silfp_storage_remove(const uint32_t id);
int32_t silfp_storage_remove_all(void);
void silfp_storage_release(void);

uint32_t silfp_storage_get_next_fid(void);
int32_t silfp_storage_inc_fail_count(const uint32_t id);
int64_t silfp_storage_load_db_id(void);
int64_t silfp_storage_get_sec_id(uint32_t fid);

int32_t silfp_storage_remove_file(const char *fname);
int32_t silfp_storage_get_file_size(const char *fname);
int32_t silfp_storage_load_config(const char *path, const char *name, char *buf, const uint32_t len);
int32_t silfp_storage_save_config(const char *path, const char *name, const char *pbuf, const uint32_t len);

#endif /* __SILEAD_STORAGE_H__ */

