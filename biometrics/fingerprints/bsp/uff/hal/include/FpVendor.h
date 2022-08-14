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
#ifndef FP_VENDOR_H
#define FP_VENDOR_H

#include "FpEnum.h"

#ifdef __cplusplus
extern "C" {
#endif
/****************************** struct begin ******************************/
typedef struct __attribute__((packed)) fp_hw_auth_token {
    unsigned char version;  // Current version is 0
    unsigned long long challenge;
    unsigned long long user_id;  // secure user ID, not Android user ID
    unsigned long long authenticator_id;  // secure authenticator ID
    unsigned int authenticator_type;  // hw_authenticator_type_t, in network order
    unsigned long long timestamp;  // in network order
    unsigned char hmac[32];
} fp_hw_auth_token_t;

/*[vendor begin]*/
typedef struct fp_temperature_info {
    unsigned int temperature_info[4];
} fp_temperature_info_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_value {
    fp_value_id_t value_id;
    void* value;
    fp_value_type_t value_type;
    int value_len;
} fp_value_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_common_config {
    char reserved[CMD_RESERVERD_LEN];
} fp_common_config_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_result {
    int  result_type;
    int  result_value;
    char reserved[CMD_RESERVERD_LEN]; // version
} fp_common_result_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_tp_info {
    unsigned char touch_state;
    unsigned char area_rate;
    unsigned short x;
    unsigned short y;
    unsigned short touchMajor;
    unsigned short touchMinor;
    unsigned short touchOrientation;
    unsigned char other_info[8];
} fp_tp_info_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_template {
    unsigned int fid;
    unsigned int size;
    unsigned char* tpl;
} fp_template_t;
/*[vendor end]*/
typedef struct fp_injection_image {
    char img_name[256];  //for distinguishing vendor and chip type. example: 1.gx_g3s 1.jv_0301
    unsigned int img_len;
    unsigned char img_addr;
} fp_injection_image_t;
/*[vendor begin]*/
/*  init */
typedef struct fp_init_config {
    char project_id[TA_STR_LEN];
    char lcd_id[TA_STR_LEN];
    char sensor_id[TA_STR_LEN];
    char reserved[32];
} fp_init_config_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_init_result {
    fp_common_result_t result_info;
    int sensor_id;
    int cali_state;
    char algo_version[64];
    char qrcode[64];
    char facotry_type[64];
    char reserved[32];
} fp_init_result_t;
/*[vendor end]*/

/*[vendor begin]*/
/* getfeature */
typedef struct fp_get_feature_config {
    unsigned int retry_index;
    fp_mode_t capture_mode;
    char reserved[32];
} fp_get_feature_config_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_get_feature_result {
    fp_common_result_t result_info;
    char reserved[32];
} fp_get_feature_result_t;
/*[vendor end]*/

/* capture */
typedef struct fp_capture_config {
    unsigned int retry_index;
    fp_mode_t capture_mode;
    //fp_tp_info_t tp_info;
    fp_screen_status_t screen_status;
    fp_temperature_info_t temperature_info;
    char reserved[32];
} fp_capture_config_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_capture_result {
    fp_common_result_t result_info;
    char reserved[32];
} fp_capture_result_t;
/*[vendor end]*/

/*[vendor begin]*/
/* enroll down */
typedef struct fp_enroll_down_config {
    fp_tp_info_t tp_info;
    unsigned char reserve[32];
} fp_enroll_down_config_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_enroll_down_result {
    fp_common_result_t result_info;
} fp_enroll_down_result_t;
/*[vendor end]*/

/*[vendor begin]*/
/* enroll image */
typedef struct fp_enroll_image_config {
    unsigned int retry_index;
    char reserved[32];
} fp_enroll_image_config_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_enroll_image_result {
    fp_common_result_t result_info;
    unsigned int fail_reason;
    unsigned int group_id;
    unsigned int finger_id;
    unsigned int duplicate_finger_id;
    unsigned int remaining;
    char reserved[32];
} fp_enroll_image_result_t;
/*[vendor end]*/

/* authenticate compare */
/*[vendor begin]*/
typedef struct fp_auth_compare_config {
    unsigned int retry_index;
    unsigned char reserve[32];
} fp_auth_compare_config_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef struct fp_auth_compare_result {
    fp_common_result_t result_info;
    unsigned int study_flag;
    unsigned int finger_id;
    unsigned int group_id;
    fp_hw_auth_token_t token;
    int compare_result_flag[4];
    int quality;
    int score;
    unsigned int area;
    unsigned int rawdata;
} fp_auth_compare_result_t;
/*[vendor end]*/

/* dump data buffer*/
/*[vendor begin]*/
typedef struct fp_dump_data_config {
    unsigned char preversionFlag;
    fp_mode_t fp_current_mode;
    dump_stage_t dump_stage;
    int dump_file_index;
    int data_size;
    int cmd_to_data_offset;
    int result_to_data_offset;
} fp_dump_data_config_t;
/*[vendor end]*/

/*[vendor begin]*/
#define MAX_FP_DUMP_FILE_PATH_LENGTH 128
typedef struct fp_dump_data_result {
    fp_common_result_t result_info;
    /* for dump stage 1 */
    unsigned int file_count;

    /* for dump stage 2 */
    unsigned int data_size;

    /* for dump stage 3 */
    char file_path[MAX_FP_DUMP_FILE_PATH_LENGTH];
    long long data_addr;
    long long offset;
    /* data_buffer must be last one, it will convert the share buffer for CA-TA */
    //char* data_buffer;
} fp_dump_data_result_t;
/*[vendor end]*/

/*[vendor begin]*/
#define MESSAGE_SIZE (10 * 1024)
typedef struct {
    int cmd_id;
    vnd_cmd_t code;
    char vnd_description[128];
    int response_value;
    char response_description[128];
    int need_reported;
    char apk_msg[MESSAGE_SIZE];
} vnd_code_t;
/*[vendor end]*/

typedef struct fp_deinit_config {
    fp_common_config_t config_info;
} fp_deinit_config_t;

typedef struct fp_deinit_result {
    fp_common_result_t result_info;
} fp_deinit_result_t;

/* set active group */
typedef struct fp_set_active_group_config {
    unsigned int group_id;
    char path[128];
    char reserved[32];
} fp_set_active_group_config_t;

typedef struct fp_set_active_group_result {
    fp_common_result_t result_info;
} fp_set_active_group_result_t;

/* pre enroll */
typedef struct fp_pre_enroll_config {
    fp_common_config_t config_info;
} fp_pre_enroll_config_t;

typedef struct fp_pre_enroll_result {
    fp_common_result_t result_info;
    unsigned long long challenge;
} fp_pre_enroll_result_t;

/* enroll */
typedef struct fp_enroll_config {
    unsigned int group_id;
    unsigned char system_token_version;
    fp_hw_auth_token_t token;
    unsigned int timeoutSec;
    char reserved[32];
} fp_enroll_config_t;

typedef struct fp_enroll_result {
    fp_common_result_t result_info;
} fp_enroll_result_t;


/* save template */
typedef struct fp_save_template_config {
    unsigned int group_id;
    unsigned int finger_id;
    char reserved[32];
} fp_save_template_config_t;

typedef struct fp_save_template_result {
    fp_common_result_t result_info;
} fp_save_template_result_t;

/* post enroll */
typedef struct fp_post_enroll_config {
    fp_common_config_t config_info;
} fp_post_enroll_config_t;

typedef struct fp_post_enroll_result {
    fp_common_result_t result_info;
} fp_post_enroll_result_t;

/* get authenticator id */
typedef struct fp_get_authenticator_id_config {
    fp_common_config_t config_info;
} fp_get_authenticator_id_config_t;

typedef struct fp_get_authenticator_id_result {
    fp_common_result_t result_info;
    unsigned long long authenticator_id;
} fp_get_authenticator_id_result_t;

/* authenticate */
typedef struct fp_authenticate_config {
    unsigned int group_id;
    unsigned long long operation_id;
    unsigned char reserve[32];
} fp_authenticate_config_t;

typedef struct fp_authenticate_result {
    fp_common_result_t result_info;
} fp_authenticate_result_t;

/* authenticate down */
typedef struct fp_auth_down_config {
    fp_tp_info_t tp_info;
    fp_screen_status_t screen_status;
    fp_temperature_info_t temperature_info;
    unsigned char reserve[32];
} fp_auth_down_config_t;

typedef struct fp_auth_down_result {
    fp_common_result_t result_info;
} fp_auth_down_result_t;

/* study */
typedef struct fp_study_config {
    fp_common_config_t config_info;
    unsigned int finger_id;
} fp_study_config_t;

typedef struct fp_study_result {
    fp_common_result_t result_info;
} fp_study_result_t;

/* auth finish */
typedef struct fp_auth_finish_config {
    fp_common_config_t config_info;
} fp_auth_finish_config_t;

typedef struct fp_auth_finish_result {
    fp_common_result_t result_info;
} fp_auth_finish_result_t;

/* enumerate */
typedef struct fp_enumerate_config {
    unsigned int size;
    unsigned int group_id;
    unsigned char reserve[32];
} fp_enumerate_config_t;

typedef struct fp_enumerate_result {
    fp_common_result_t result_info;
    unsigned int finger_count;
    unsigned int finger_id[MAX_FINGERS_PER_USER];
} fp_enumerate_result_t;

/* cancel */
typedef struct fp_cancel_config {
    fp_common_config_t config_info;
} fp_cancel_config_t;

typedef struct fp_cancel_result {
    fp_common_result_t result_info;
} fp_cancel_result_t;

/* remove*/
typedef struct fp_remove_config {
    unsigned int group_id;
    unsigned int finger_id;
    unsigned char reserve[32];
} fp_remove_config_t;

typedef struct fp_remove_result {
    fp_common_result_t result_info;
} fp_remove_result_t;

/* set hmackey*/
typedef struct fp_set_hmackey_config {
    unsigned char hmackey_buffer[QSEE_HMAC_KEY_MAX_LEN];
} fp_set_hmackey_config_t;

/* get dcs info*/
typedef struct fp_get_dcsinfo_config {
    fp_send_cmdid_t dcs_event_type;
    int dcsinfo_length;
} fp_get_dcsinfo_config_t;

typedef struct fp_get_dcsinfo_result {
    char buffer[MAX_DCS_TA_DATA_LEN];
} fp_get_dcsinfo_result_t;

/* set sensor mode*/
typedef struct fp_set_sensor_mode_config {
    fp_module_id_t module_id;
} fp_set_sensor_mode_config_t;

typedef struct fp_set_sensor_mode_result {
    fp_common_result_t result_info;
} fp_set_sensor_mode_result_t;

/* send cmd*/
typedef struct fp_send_cmd_config {
    int cmd_id;
    void* cmd_buffer;
    int cmd_buffer_length;
} fp_send_cmd_config_t;

typedef struct fp_send_cmd_result {
    fp_common_result_t result_info;
    void* result_buffer;
    int result_buffer_length;
} fp_send_cmd_result_t;

#ifdef __cplusplus
}
#endif
#endif  // FP_TYPE_H

