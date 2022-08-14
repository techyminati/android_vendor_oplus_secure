/******************************************************************************
 * @file   silead_dump.h
 * @brief  Contains dump image header file.
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

#ifndef __SILEAD_DUMP_H__
#define __SILEAD_DUMP_H__

typedef enum {
    DUMP_IMG_AUTH_SUCC = 0,
    DUMP_IMG_AUTH_FAIL,
    DUMP_IMG_ENROLL_SUCC,
    DUMP_IMG_ENROLL_FAIL,
    DUMP_IMG_NAV_SUCC,
    DUMP_IMG_NAV_FAIL,
    DUMP_IMG_SHOT_SUCC,
    DUMP_IMG_SHOT_FAIL,
    DUMP_IMG_RAW,
    DUMP_IMG_CAL,
    DUMP_IMG_FT_QA,
    DUMP_IMG_AUTH_ORIG,
    DUMP_IMG_ENROLL_ORIG,
    DUMP_IMG_OTHER_ORIG,
    DUMP_IMG_SNR,
    DUMP_IMG_ENROLL_NEW,
    DUMP_IMG_AUTH_MUL_RAW,
    DUMP_IMG_MAX,
} e_mode_dump_img_t;

#define DUMP_NAME_MODE_PARENT_DIR_NULL      0x0001  // parent dir is null
#define DUMP_NAME_MODE_SUB_DIR_SEPARATE     0x0002  // separate dir by img type
#define DUMP_NAME_MODE_TIMESTAMP_NONE       0x0004  // no date & time in file name
#define DUMP_NAME_MODE_TIMESTAMP_DATE_ONLY  0x0008  // just date in file name
#define DUMP_NAME_MODE_TIMESTAMP_TIME_ONLY  0x0010  // just time in file name
#define DUMP_NAME_MODE_REBOOT_TIMES_NONE    0x0020  // no reboot times in file name
#define DUMP_NAME_MODE_CAL_TIMESTAMP_NONE   0x0040  // no data & time in cal dump file name

/* maxlen should set to -1, if no length limited (include the ext len) */
void silfp_dump_set_name_mode(uint32_t mode, int32_t maxlen, const void *ext, uint32_t len);
void silfp_dump_set_path(const void *path, uint32_t len);

void silfp_dump_data(e_mode_dump_img_t type);
void silfp_dump_deinit(void);

void silfp_dump_test_result(const char *name, const void *content, uint32_t len);
void silfp_dump_test_get_path(e_mode_dump_img_t type);

#endif /* __SILEAD_DUMP_H__ */