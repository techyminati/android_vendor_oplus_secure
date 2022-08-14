/******************************************************************************
 * @file   silead_screen.h
 * @brief  Contains screen callback definitions.
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
 * Jack Zhang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_SCREEN_H__
#define __SILEAD_SCREEN_H__

typedef void (*screen_cb)(int32_t, void*);
typedef void (*touch_down_pre_action)();
typedef void (*touch_up_pre_action)();

typedef enum _finger_status_mode {
    SIFP_FINGER_STATUS_IRQ = 0,
    SIFP_FINGER_STATUS_TP = 1,
    SIFP_FINGER_STATUS_ANDROID = 2,
} finger_status_mode_t;

#endif /* __SILEAD_SCREEN_H__ */
