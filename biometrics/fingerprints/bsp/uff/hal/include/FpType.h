/********************************************************************************************
** Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
**
** File: fp_type.h
** Description: define base type for fp
** Version: 1.0
** Date : 2021-3-29
** Author: wangzhi(Kevin) wangzhi12
** TAG: BSP.Fingerprint.Basic
** -----------------------------Revision History: -----------------------?
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/03/29        create the file
*********************************************************************************************/
#ifndef FP_TYPE_H
#define FP_TYPE_H

#include "FpEnum.h"
#include "FpVendor.h"
#include <string>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fp_sensor_info {
    unsigned int sensor_info[4];
} fp_sensor_info_t;

typedef struct fp_ta_cmd_header {
    int module_id;
    int cmd_id;
    int result;
    char reset_flag;
    char reserved[12];
} fp_ta_cmd_header_t;

typedef struct fp_icon_info {
    unsigned int icon_size;
    unsigned int icon_location;
    unsigned char icon_number;
    unsigned int sensor_location;
    unsigned int sensor_rotation;
} fp_icon_info_t;

typedef struct fp_oplus_info {
    fp_icon_info_t icon_info;
    lcd_type_t lcd_type_info;
    unsigned int project_info;
    unsigned int other_info;
    unsigned int log_level;
} fp_oplus_info_t;

typedef struct {
    unsigned int template_buffer_size;
    void* ptemplate_buffer;
} fp_template_info_t;

/****************************** cmd type define begin ******************************/
/* All ca-ta cmd keep the format:
 * [| header | [in] config | [out]  result |]
 */

/* fp init */
typedef struct fp_init {
    fp_ta_cmd_header_t header;
    fp_init_config_t config;
    fp_init_result_t result;
} fp_init_t;

/* fp deinit */
typedef struct fp_deinit {
    fp_ta_cmd_header_t header;
    fp_deinit_config_t config;
    fp_deinit_result_t result;
} fp_deinit_t;

/* set active group */
typedef struct fp_set_active_group {
    fp_ta_cmd_header_t header;
    fp_set_active_group_config_t config;
    fp_set_active_group_result_t result;
} fp_set_active_group_t;

/* getfeature */
typedef struct fp_get_feature {
    fp_ta_cmd_header_t header;
    fp_get_feature_config_t config;
    fp_get_feature_result_t result;
} fp_get_feature_t;

/* capture */
typedef struct fp_capture_image {
    fp_ta_cmd_header_t header;
    fp_capture_config_t config;
    fp_capture_result_t result;
} fp_capture_image_t;

/* pre enroll */
typedef struct fp_pre_enroll {
    fp_ta_cmd_header_t header;
    fp_pre_enroll_config_t config;
    fp_pre_enroll_result_t result;
} fp_pre_enroll_t;

/* enroll */
typedef struct fp_enroll {
    fp_ta_cmd_header_t header;
    fp_enroll_config_t config;
    fp_enroll_result_t result;
} fp_enroll_t;

/* enroll down */
typedef struct fp_enroll_down {
    fp_ta_cmd_header_t header;
    fp_enroll_down_config_t config;
    fp_enroll_down_result_t result;
} fp_enroll_down_t;

/* enroll image */
typedef struct fp_enroll_image {
    fp_ta_cmd_header_t header;
    fp_enroll_image_config_t config;
    fp_enroll_image_result_t result;
} fp_enroll_image_t;

/* save template */
typedef struct fp_save_template {
    fp_ta_cmd_header_t header;
    fp_save_template_config_t config;
    fp_save_template_result_t result;
} fp_save_template_t;

/* post enroll */
typedef struct fp_post_enroll {
    fp_ta_cmd_header_t header;
    fp_post_enroll_config_t config;
    fp_post_enroll_result_t result;
} fp_post_enroll_t;

/* get authenticator id */
typedef struct fp_get_authenticator_id {
    fp_ta_cmd_header_t header;
    fp_get_authenticator_id_config_t config;
    fp_get_authenticator_id_result_t result;
} fp_get_authenticator_id_t;

/* authenticate */
typedef struct fp_authenticate {
    fp_ta_cmd_header_t header;
    fp_authenticate_config_t config;
    fp_authenticate_result_t result;
} fp_authenticate_t;

/* authenticate down */
typedef struct fp_auth_down {
    fp_ta_cmd_header_t header;
    fp_auth_down_config_t config;
    fp_auth_down_result_t result;
} fp_auth_down_t;

/* authenticate compare */
typedef struct fp_auth_compare {
    fp_ta_cmd_header_t header;
    fp_auth_compare_config_t config;
    fp_auth_compare_result_t result;
} fp_auth_compare_t;

/* authenticate study */
typedef struct fp_auth_study {
    fp_ta_cmd_header_t header;
    fp_study_config_t config;
    fp_study_result_t result;
} fp_auth_study_t;

/* authenticate finish */
typedef struct fp_auth_finish {
    fp_ta_cmd_header_t header;
    fp_auth_finish_config_t config;
    fp_auth_finish_result_t result;
} fp_auth_finish_t;

typedef struct fp_enroll_finish {
    fp_ta_cmd_header_t header;
} fp_enroll_finish_t;

/* enumerate */
typedef struct fp_enumerate {
    fp_ta_cmd_header_t header;
    fp_enumerate_config_t config;
    fp_enumerate_result_t result;
} fp_enumerate_t;

/* remove*/
typedef struct fp_remove {
    fp_ta_cmd_header_t header;
    fp_remove_config_t config;
    fp_remove_result_t result;
} fp_remove_t;

/* cancel*/
typedef struct fp_cancel {
    fp_ta_cmd_header_t header;
} fp_cancel_t;

/* set hmackey*/
typedef struct fp_set_hmackey {
    fp_ta_cmd_header_t header;
    fp_set_hmackey_config_t config;
} fp_set_hmackey_t;

/* custom enc*/

/* get dcs info*/
typedef struct fp_get_dcsinfo {
    fp_ta_cmd_header_t header;
    fp_get_dcsinfo_config_t config;
    fp_get_dcsinfo_result_t result;
} fp_get_dcsinfo_t;

/* set sensor mode*/
typedef struct fp_set_sensor_mode {
    fp_ta_cmd_header_t header;
    fp_set_sensor_mode_config_t config;
    fp_set_sensor_mode_result_t result;
} fp_set_sensor_mode_t;

/* send cmd*/
typedef struct fp_send_cmd {
    fp_ta_cmd_header_t header;
    fp_send_cmd_config_t config;
    fp_send_cmd_result_t result;
} fp_send_cmd_t;

/* dump data buffer */
typedef struct fp_dump {
    fp_ta_cmd_header_t header;
    fp_dump_data_config_t config;
    fp_dump_data_result_t result;
} fp_dump_t;

typedef struct fp_user_data {
    char filePath[MAX_FILE_NAME_LENGTH];
    char enrollRenamePath[MAX_FILE_NAME_LENGTH];
    /* data buf addr */
    const char* data;
    /* data buf length */
    int dataLen;
    int withFormat;
    unsigned int gid;
    /* authenticate, enroll... result */
    unsigned int result;
    unsigned int fingerId;
    fp_mode_t mode;
    std::string mEnrollPath;
} fp_user_data_t;

typedef struct {
    unsigned int total_size;
    unsigned int offset;
    unsigned int read_size;
    char ca_buffer[];
} factory_data_t;

typedef struct {
    fp_ta_cmd_header_t header;
    vnd_code_t vnd_code;
    factory_data_t data;
} fp_factory_t;

/* inject data */
typedef struct fp_inject {
    fp_ta_cmd_header_t header;
    fp_injection_mode_t config;
    fp_injection_image_t image;
} fp_inject_t;

typedef struct chip_info {
    int chip_id;
    char buf[128];  // reserve for Unexpected requirement
    unsigned int len;
} fp_chip_info_t;

typedef struct {
    fp_ta_cmd_header_t header;
    fp_ta_name_t name;
    fp_chip_info_t info;
} fp_chip_select_t;

#ifdef __cplusplus
}
#endif
#endif  // FP_TYPE_H

