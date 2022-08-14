/******************************************************************************
 * @file   silead_worker.h
 * @brief  Contains fingerprint operate functions header file.
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

#ifndef __SILEAD_WORKER_H__
#define __SILEAD_WORKER_H__

typedef enum worker_state {
    STATE_IDLE = 0,
    STATE_ENROLL,
    STATE_SCAN,
    STATE_NAV,
    STATE_EXIT,
    STATE_TEST,
    STATE_WAIT,
    STATE_EXT_CB,
    STATE_LOCK,
    STATE_BREAK,
    STATE_ESD_CHK,
    STATE_MAX,
} worker_state_t;

void silfp_worker_wait_condition(int32_t timeout_msec);
void silfp_worker_wakeup_condition(void);

void silfp_worker_set_state_no_signal(worker_state_t state);
void silfp_worker_set_state(worker_state_t state);
worker_state_t silfp_worker_get_state(void);
int32_t silfp_worker_is_canceled(void);

int32_t silfp_worker_get_screen_state(void);
void silfp_worker_set_screen_state_default(int32_t screenon);
void silfp_worker_screen_state_callback(int32_t screenon, void *param);

int32_t silfp_worker_init(void);
int32_t silfp_worker_run(void);
int32_t silfp_worker_deinit(void);

void silfp_worker_set_to_break_mode(void);

#endif // __SILEAD_WORKER_H__