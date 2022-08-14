/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_hal common header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_HAL_COMMON_H_
#define _GF_HAL_COMMON_H_

#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include "gf_common.h"
#include "gf_error.h"
#include "gf_fingerprint.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#define RE_POWER_EN (1)  // enable re power on
#define RE_POWER_DIS (0)  // disable re power on
#define THREAD_BLOCK_EN  (1)  // enable thread block
#define THREAD_BLOCK_DIS (0)  // disable thread block

typedef struct timeval timeval_t;

typedef enum
{
    STATE_IDLE = 0,
    STATE_ENROLL,
    STATE_AUTHENTICATE,
    STATE_REMOVE
} gf_work_state_t;

typedef enum
{
    STATE_TEST_IDLE = 0,
    STATE_TEST_WORKING = 1,
    STATE_TOUCH_WORKING = 2,
} gf_custom_state_t;

typedef enum
{
    GF_SCREEN_OFF = 0,
    GF_SCREEN_ON = 1,
} gf_screen_state_t;

enum gf_spi_speed
{
    GF_SPI_LOW_SPEED = 0,
    GF_SPI_HIGH_SPEED = 1,
};

typedef struct gf_hal_function
{
    gf_error_t (*init)(void *dev);
    gf_error_t (*close)(void *dev);
    gf_error_t (*cancel)(void *dev);
    gf_error_t (*test_cancel)(void *dev);
    gf_error_t (*test_prior_cancel)(void *dev);

    uint64_t (*pre_enroll)(void *dev);
    gf_error_t (*enroll)(void *dev, const void *hat, uint32_t group_id,
                         uint32_t timeout_sec);
    gf_error_t (*post_enroll)(void *dev);
    gf_error_t (*authenticate)(void *dev, uint64_t operation_id, uint32_t group_id);
    uint64_t (*get_auth_id)(void *dev);
    gf_error_t (*remove)(void *dev, uint32_t group_id, uint32_t finger_id);
    gf_error_t (*set_active_group)(void *dev, uint32_t group_id);

    gf_error_t (*enumerate)(void *dev, void *results, uint32_t *max_size);
    gf_error_t (*enumerate_with_callback)(void *dev);
    gf_error_t (*irq)();
    gf_error_t (*screen_on)();
    gf_error_t (*screen_off)();

    gf_error_t (*set_safe_class)(void *dev, gf_safe_class_t safe_class);
    gf_error_t (*navigate)(void *dev, gf_nav_mode_t nav_mode);

    gf_error_t (*enable_fingerprint_module)(void *dev, uint8_t enable_flag);
    gf_error_t (*camera_capture)(void *dev);
    gf_error_t (*enable_ff_feature)(void *dev, uint8_t enable_flag);

    gf_error_t (*enable_bio_assay_feature)(void *dev, uint8_t enable_flag);
    gf_error_t (*start_hbd)(void *dev);
    gf_error_t (*reset_lockout)();
    gf_error_t (*lockout)();
    gf_error_t (*sync_finger_list)(void *dev, uint32_t *list, int32_t count);

    gf_error_t (*invoke_command)(uint32_t operation_id, gf_cmd_id_t cmd_id,
                                 void *buffer, int32_t len);
    gf_error_t (*user_invoke_command)(uint32_t cmd_id, void *buffer, int32_t len);
    gf_error_t (*dump_invoke_command)(uint32_t cmd_id, void *buffer, int32_t len);
    bool (*dump_chip_init_data)();
    bool (*dump_chip_operation_data)(gf_dump_data_t *dump_data,
                                      gf_operation_type_t operation,
                                      struct timeval* tv,
                                      gf_error_t error_code,
                                      gf_chip_type_t chip_type);

    gf_error_t (*authenticate_fido)(void *dev, uint32_t group_id, uint8_t *aaid,
                                    uint32_t aaid_len,
                                    uint8_t *challenge, uint32_t challenge_len);
    gf_error_t (*is_id_valid)(void *dev, uint32_t group_id, uint32_t finger_id);
    int32_t (*get_id_list)(void *dev, uint32_t group_id, uint32_t *list,
                           int32_t *count);

    gf_error_t (*pause_enroll)();
    gf_error_t (*resume_enroll)();
    gf_error_t (*set_finger_screen)(uint32_t sreen_t);
} gf_hal_function_t;

/* extern global variable for other files */
extern pthread_mutex_t g_sensor_mutex;
extern uint8_t g_enable_fingerprint_module;
extern timer_t g_enroll_timer_id;
extern uint8_t g_key_down_flag;
extern uint32_t g_spi_speed;
extern timer_t g_long_pressed_timer_id;
extern int64_t g_down_irq_time;
extern int64_t g_image_irq_time;
extern uint16_t g_enroll_invalid_template_num;
extern pthread_mutex_t g_hal_mutex;
extern uint32_t g_failed_attempts;
extern uint8_t g_is_only_dump_broken_check;
extern uint32_t g_hal_inited;
extern volatile uint8_t g_set_active_group_done;  // for gf_hal.c
extern uint8_t g_screen_status;  // for gf_hal_dump.c
extern uint32_t g_enroll_timer_sec;

extern timer_t g_esd_timer_id;
extern uint8_t g_esd_check_flag;

extern gf_mode_t g_mode;
extern gf_operation_type_t g_operation;

extern uint32_t g_sensor_row;
extern uint32_t g_sensor_col;
extern uint32_t g_sensor_nav_row;
extern uint32_t g_sensor_nav_col;

extern uint32_t g_nav_times;
extern uint32_t g_nav_frame_index;
extern gf_nav_click_status_t g_nav_click_status;
extern timer_t g_nav_double_click_timer_id;
extern timer_t g_nav_long_press_timer_id;
extern pthread_mutex_t g_nav_click_status_mutex;
extern uint32_t g_fpc_config_had_downloaded;

extern uint32_t g_auth_retry_times;
extern uint8_t g_enum_fingers;
extern uint8_t g_sensor_broken_check_mode;
extern void hal_enroll_timer_thread(union sigval v);


gf_error_t gf_hal_function_init(gf_hal_function_t *hal_function,
                                gf_chip_series_t chip_series);

void gf_hal_dump_performance(const char *func_name,
                             gf_operation_type_t operation,
                             gf_test_performance_t *dump_performance);

void gf_hal_notify_acquired_info(gf_fingerprint_acquired_info_t acquired_info);
void gf_hal_notify_error_info(gf_fingerprint_error_t err_code);
void gf_hal_notify_enrollment_progress(uint32_t group_id, uint32_t finger_id,
                                       uint32_t samples_remaining);
void gf_hal_notify_authentication_succeeded(uint32_t group_id,
                                            uint32_t finger_id,
                                            gf_hw_auth_token_t *auth_token);
void gf_hal_notify_authentication_failed();
void gf_hal_notify_authentication_fido_failed();
void gf_hal_notify_remove_succeeded(uint32_t group_id, uint32_t finger_id, uint32_t remaining_templates);

gf_error_t gf_hal_save(uint32_t group_id, uint32_t finger_id);
gf_error_t gf_hal_update_stitch(uint32_t group_id, uint32_t finger_id);

gf_error_t gf_hal_reinit();
gf_error_t gf_hal_invoke_command(gf_cmd_id_t cmd_id, void *buffer, int32_t len);
gf_error_t gf_hal_invoke_command_ex(gf_cmd_id_t cmd_id);

void gf_hal_nav_code_convert(gf_nav_code_t nav_code,
                             gf_nav_code_t *converted_nav_code);
gf_error_t gf_hal_download_fw();
gf_error_t gf_hal_download_cfg();
void gf_hal_create_and_set_esd_timer();
void gf_hal_long_pressed_timer_thread(union sigval v);
void gf_hal_nav_listener(gf_nav_code_t nav_code);
void gf_hal_nav_reset();
void gf_hal_nav_double_click_timer_thread(union sigval v);
void gf_hal_nav_long_press_timer_thread(union sigval v);
gf_error_t gf_hal_nav_complete(void);
void gf_hal_nav_assert_config_interval();
int64_t gf_hal_current_time_microsecond(void);
int64_t gf_hal_current_time_second(void);
gf_error_t gf_hal_init_finished();

gf_error_t gf_hal_common_close(void *dev);
gf_error_t gf_hal_common_cancel(void *dev);
uint64_t gf_hal_common_pre_enroll(void *dev);
gf_error_t gf_hal_common_enroll(void *dev, const void *hat, uint32_t group_id,
                                uint32_t timeout_sec);
gf_error_t gf_hal_common_post_enroll(void *dev);
gf_error_t gf_hal_common_authenticate(void *dev, uint64_t operation_id,
                                      uint32_t group_id);
uint64_t gf_hal_common_get_auth_id(void *dev);
gf_error_t gf_hal_common_remove(void *dev, uint32_t group_id,
                                uint32_t finger_id);
gf_error_t gf_hal_common_remove_without_callback(uint32_t group_id,
                                                 uint32_t finger_id);
gf_error_t gf_hal_common_remove_for_sync_list(void *dev, uint32_t group_id,
                                              uint32_t finger_id);
gf_error_t gf_hal_common_set_active_group(void *dev, uint32_t group_id);
gf_error_t gf_hal_common_enumerate(void *dev, void *results,
                                   uint32_t *max_size);
gf_error_t gf_hal_common_enumerate_with_callback(void *dev);
gf_error_t gf_hal_common_screen_on();
gf_error_t gf_hal_common_screen_off();
gf_error_t gf_hal_common_set_safe_class(void *dev, gf_safe_class_t safe_class);
gf_error_t gf_hal_common_navigate(void *dev, gf_nav_mode_t nav_mode);
gf_error_t gf_hal_common_enable_fingerprint_module(void *dev,
                                                   uint8_t enable_flag);
gf_error_t gf_hal_common_camera_capture(void *dev);
gf_error_t gf_hal_common_enable_ff_feature(void *dev, uint8_t enable_flag);
gf_error_t gf_hal_common_reset_lockout();
gf_error_t gf_hal_common_lockout();
gf_error_t gf_hal_common_sync_finger_list(void *dev, uint32_t *list,
                                          int32_t count);
gf_error_t gf_hal_common_user_invoke_command(uint32_t cmd_id, void *buffer,
                                             int32_t len);
gf_error_t gf_hal_common_authenticate_fido(void *dev, uint32_t group_id,
                                           uint8_t *aaid,
                                           uint32_t aaid_len, uint8_t *challenge, uint32_t challenge_len);
gf_error_t gf_hal_common_is_id_valid(void *dev, uint32_t group_id,
                                     uint32_t finger_id);
gf_error_t gf_hal_common_get_id_list(void *dev, uint32_t group_id,
                                     uint32_t *list, int32_t *count);


gf_error_t gf_hal_common_pause_enroll();
gf_error_t gf_hal_common_resume_enroll();

/**
 * This API always return SUCCESS, *otp_buf_len > 0, means that successfully load backup OTP info
 */
gf_error_t gf_hal_common_load_otp_info_from_sdcard(uint8_t *otp_buf,
                                                   uint32_t *otp_len);
gf_error_t gf_hal_common_save_otp_info_into_sdcard(uint8_t *otp_buf,
                                                   uint32_t otp_len);


/**
  * Test relative method below
  */
gf_error_t gf_hal_test_invoke_command(gf_cmd_id_t cmd_id, void *buffer,
                                      int32_t len);
gf_error_t gf_hal_test_invoke_command_ex(gf_cmd_id_t cmd_id);

void gf_hal_notify_test_acquired_info(gf_fingerprint_acquired_info_t
                                      acquired_info);
void gf_hal_notify_test_error_info(gf_fingerprint_error_t err_code);
void gf_hal_notify_test_enrollment_progress(uint32_t group_id,
                                            uint32_t finger_id,
                                            uint32_t samples_remaining);
void gf_hal_notify_test_authentication_succeeded(uint32_t group_id,
                                                 uint32_t finger_id,
                                                 gf_hw_auth_token_t *auth_token);
void gf_hal_notify_test_authentication_failed();
void gf_hal_notify_test_remove_succeeded(uint32_t group_id, uint32_t finger_id);

/**
  * Dump relative method below
  */
void gf_hal_dump_data_by_operation(gf_operation_type_t operation, gf_error_t error_code);
gf_error_t gf_hal_dump_template(uint32_t group_id, uint32_t finger_id);
gf_error_t gf_hal_common_dump_invoke_command(uint32_t cmd_id, void *buffer,
                                             int32_t len);

void gf_hal_common_enroll_success(gf_irq_t *cmd, gf_error_t *err_code);
void gf_hal_common_authenticate_success(gf_irq_t *cmd, gf_error_t *err_code);
void gf_hal_common_authenticate_fido_success(gf_irq_t *cmd,
                                             gf_error_t *err_code);
void gf_hal_common_authenticate_not_match(gf_irq_t *cmd, gf_error_t err_code);

void gf_hal_common_irq_key(gf_irq_t *cmd);
void gf_hal_common_irq_finger_long_press(gf_irq_t *cmd, gf_error_t err_code);

int do_enumerate(void *dev, uint32_t group_id, uint32_t finger_id);
#ifdef FP_HYPNUSD_ENABLE
void gf_set_hypnus(int32_t action_type, int32_t action_timeout);
#endif
void gf_bind_bigcore_bytid();

void gf_hal_notify_msg_info(fingerprint_msg_type_t msg_info);
void gf_hal_notify_image_info(fingerprint_acquired_info_t acquired_info);
void gf_hal_dump_save_cur_time(uint8_t index);
void gf_hal_notify_send_auth_dcsmsg(fingerprint_auth_dcsmsg_t auth_context);
gf_error_t gf_hal_common_get_qr_code(uint8_t *qr_buf, uint8_t buf_len);
gf_error_t gf_hal_detect_sensor_broken(void);
void gf_hal_post_sem_detect_broken(void);
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_COMMON_H_
