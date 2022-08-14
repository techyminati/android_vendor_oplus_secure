/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_hal frr database header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_HAL_FRR_DATABASE_H_
#define _GF_HAL_FRR_DATABASE_H_

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "gf_error.h"
#include "gf_common.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#define PROTOCOL_VERSION 0001

#define FRR_DATABASE_FILE "/data/vendor/fingerprint/gf_data/frr_database.db"
#define FRR_DATABASE_DIR "/data/vendor/fingerprint/gf_data/"
/*ensure the database file must be shorter than 10KB*/
#define FRR_DATABASE_METADATA_MAX 800

#define AUTH_RATIO_FILENAME  "/data/vendor/fingerprint/gf_data/auth_ratio_database.db"
#define AUTH_RATIO_FILENAME_BAK  "/data/vendor/fingerprint/gf_data/auth_ratio_database.db.bak"
#define AUTH_RATIO_READ_COUNT 10

// 24 byte, include timestamp(64 bit), result(32 bit), match_score(32 bit), finger_id(32 bit), template_size(32 bit)
#define AUTH_RATIO_RECORD_LEN (sizeof(uint64_t) + sizeof(uint32_t) * 4)
#define AUTH_RATIO_BUF_LEN (AUTH_RATIO_READ_COUNT * AUTH_RATIO_RECORD_LEN)
#define AUTH_RATIO_FILE_MAX 10240

typedef struct stat gf_stat_t;

typedef enum
{
    TAG_PACKAGE_VERSION = 0,
    TAG_PROTOCOL_VERSION,
    TAG_CHIP_TYPE,
    TAG_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT,
    TAG_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT,
    TAG_CHIP_SUPPORT_BIO,
    TAG_IS_BIO_ENABLE,
    TAG_AUTHENTICATED_WITH_BIO_SUCCESS_COUNT,
    TAG_AUTHENTICATED_WITH_BIO_FAILED_COUNT,
    TAG_AUTHENTICATED_SUCCESS_COUNT,
    TAG_AUTHENTICATED_FAILED_COUNT,
    TAG_BUF_FULL,
    TAG_UPDATE_POS
} gf_frr_tag_t;

typedef enum
{
    TEST_TOKEN_AUTH_DATA = 1500,
    TEST_TOKEN_AUTH_COUNT,
}gf_auth_ratio_t;

extern gf_error_t gf_hal_handle_frr_database(gf_error_t err,
                                             int32_t image_quality,
                                             int32_t image_area);
extern uint32_t gf_hal_get_tag_data_pos(gf_frr_tag_t tag_idx);
extern int32_t gf_hal_get_tag_data(FILE *fp, uint32_t pos);
extern gf_error_t gf_hal_judge_delete_frr_database(gf_chip_type_t chip_type,
                                                   gf_chip_series_t chip_series);

extern void gf_hal_save_auth_ratio_record(gf_irq_t* cmd);
extern gf_error_t gf_hal_get_auth_ratio_record(uint8_t* buff, uint32_t* len);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_FRR_DATABASE_H_

