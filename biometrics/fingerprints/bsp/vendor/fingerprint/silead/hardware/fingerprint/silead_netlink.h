/******************************************************************************
 * @file   silead_netlink.h
 * @brief  Contains netlink communication functions header file.
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
 * David Wang  2018/5/28   0.1.1      Support poll/read if netlink id invalid
 *
 *****************************************************************************/

#ifndef __SILEAD_NETLINK_H__
#define __SILEAD_NETLINK_H__

#include "silead_screen.h"

#define MAX_NL_MSG_LEN 16

typedef enum _netlink_cmd {
    SIFP_NETLINK_START = 0,
    SIFP_NETLINK_IRQ = 1,
    SIFP_NETLINK_SCREEN_OFF,
    SIFP_NETLINK_SCREEN_ON,
    SIFP_NETLINK_CONNECT,
    SIFP_NETLINK_DISCONNECT,
    SIFP_NETLINK_TOUCH_DOWN,
    SIFP_NETLINK_TOUCH_UP,
    SIFP_NETLINK_MAX,
} netlink_cmd_t;

int32_t silfp_nl_init(uint8_t nl_id, int32_t fd);
int32_t silfp_nl_deinit(void);

int32_t silfp_nl_set_screen_cb(screen_cb listen, void *param);
int32_t silfp_nl_set_finger_status_mode(int32_t mode);

#endif /* __SILEAD_NETLINK_H__ */

