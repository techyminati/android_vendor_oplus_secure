/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL layer simulator native
 * History:
 * Version: 1.0
 */


#ifndef _GF_HAL_SIMULATOR_NATIVE_H_
#define _GF_HAL_SIMULATOR_NATIVE_H_

#include "gf_common.h"
#include "stdint.h"
#include "gf_hal_simulator_type.h"
#include "gf_fingerprint.h"
#include "gf_dump_config.h"

extern gf_dump_config_t g_dump_config;  // a copy of dump config
extern int32_t g_apk_enabled_dump;  // if apk called enable/disable dump

void notify_for_simulate(const gf_fingerprint_msg_t *msg);
void gf_simulator_set_channel(int32_t write_pipe);
gf_error_t gf_simulator_enroll(int32_t* data);
gf_error_t gf_simulator_authenticate(int32_t* data);
gf_error_t gf_simulator_authenticate_fido(int32_t* data);
gf_error_t gf_simulator_remove_finger(int32_t* data);
gf_error_t gf_simulator_invoke_irq(int32_t* data);
gf_error_t gf_simulator_set_nav_oritension(int32_t *data);
gf_error_t gf_simulator_enumerate(int32_t* data);
gf_error_t gf_simulator_set_active_group(int32_t* groupid);
gf_error_t gf_simulator_get_auth_id(int32_t* data);
gf_error_t gf_simulator_enumerate_with_callback(int32_t* data);
gf_error_t gf_simulator_remove_with_callback(int32_t* data);
gf_error_t gf_simulator_check_memory(int32_t* data);
gf_error_t gf_simulator_clear(int32_t* data);
gf_error_t gf_simulator_finger_quick_up_or_mistake_touch(int32_t* data);
gf_error_t gf_simulator_pre_enroll(int32_t* data);
gf_error_t gf_simulator_post_enroll(int32_t* data);
gf_error_t gf_simulator_cancel(int32_t* data);
void gf_simulator_set_dump_config(const gf_dump_config_t* dump_cfg);
void gf_simulator_dump_cmd(int32_t* cmd_data);
void gf_simulator_reset_apk_enable_dump_flag();
void gf_simulator_set_simulate_dirty_data_flag(int32_t* data);
void gf_simulator_dump_device_info();
void gf_simulator_start_navigate();

#endif  // _GF_HAL_SIMULATOR_NATIVE_H_
