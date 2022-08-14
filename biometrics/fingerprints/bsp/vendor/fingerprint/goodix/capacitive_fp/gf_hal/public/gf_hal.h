/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_HAL_H_
#define _GF_HAL_H_

#include "gf_error.h"
#include "gf_type_define.h"
#include "gf_fingerprint.h"
#include "gf_hal_common.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"

extern gf_config_t g_hal_config;
extern gf_fingerprint_device_t *g_fingerprint_device;
extern timer_t g_irq_timer_id;
extern gf_hal_function_t g_hal_function;
extern volatile gf_work_state_t g_work_state;
extern volatile uint8_t g_set_active_group_done;

gf_error_t gf_hal_open(void *dev);
gf_error_t gf_hal_close(void *dev);
gf_error_t gf_hal_cancel(void *dev);
int gf_keymode_enable(void *dev, int enable);

uint64_t gf_hal_pre_enroll(void *dev);
gf_error_t gf_hal_enroll(void *dev, const void *hat, uint32_t group_id,
                         uint32_t timeout_sec);
gf_error_t gf_hal_post_enroll(void *dev);
gf_error_t gf_hal_authenticate(void *dev, uint64_t operation_id,
                               uint32_t group_id);
uint64_t gf_hal_get_auth_id(void *dev);
gf_error_t gf_hal_remove(void *dev, uint32_t group_id, uint32_t finger_id);
gf_error_t gf_hal_remove_for_sync_list(void *dev, uint32_t group_id,
                                       uint32_t finger_id);
gf_error_t gf_hal_set_active_group(void *dev, uint32_t group_id, const char *store_path);

gf_error_t gf_hal_enumerate(void *dev, void *results, uint32_t *max_size);
gf_error_t gf_hal_enumerate_with_callback(void *dev);
gf_error_t gf_hal_irq(void);
gf_error_t gf_hal_screen_on(void);
gf_error_t gf_hal_screen_off(void);

gf_error_t gf_hal_set_safe_class(void *dev, gf_safe_class_t safe_class);
gf_error_t gf_hal_navigate(void *dev, gf_nav_mode_t nav_mode);

gf_error_t gf_hal_enable_fingerprint_module(void *dev, uint8_t enable_flag);
gf_error_t gf_hal_camera_capture(void *dev);
gf_error_t gf_hal_enable_ff_feature(void *dev, uint8_t enable_flag);

gf_error_t gf_hal_enable_bio_assay_feature(void *dev, uint8_t enable_flag);
gf_error_t gf_hal_start_hbd(void *dev);
gf_error_t gf_hal_reset_lockout();
gf_error_t gf_hal_lockout();
gf_error_t gf_hal_sync_finger_list(void *dev, uint32_t *list, int32_t count);

gf_error_t gf_hal_user_invoke_command(uint32_t cmd_id, void *buffer, uint32_t len);

gf_error_t gf_hal_authenticate_fido(void *dev, uint32_t group_id, uint8_t *aaid,
                                    uint32_t aaid_len, uint8_t *challenge, uint32_t challenge_len);
gf_error_t gf_hal_is_id_valid(void *dev, uint32_t group_id, uint32_t finger_id);
gf_error_t gf_hal_get_id_list(void *dev, uint32_t group_id, uint32_t *list,
                              int32_t *count);

uint32_t gf_hal_is_inited(void);

gf_error_t gf_hal_pause_enroll();
gf_error_t gf_hal_continue_enroll();

/*for oppo*/
gf_error_t gf_hal_wait_touch_down(void);
gf_error_t gf_hal_set_finger_screen(int32_t screen_state);
gf_error_t gf_open_debug(uint32_t on);
void gf_hal_get_image_quality(void);
gf_error_t gf_hal_module_test(void);
gf_error_t gf_hal_test_bad_point(void);
gf_error_t gf_hal_set_enroll_state(uint8_t new_state);
gf_error_t gf_hal_check_data_noise(void);
int gf_get_enrollment_total_times(void);
gf_error_t gf_hal_notify_qrcode(int32_t cmdId);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_H_
