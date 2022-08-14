/******************************************************************************
 * @file   silead_key.h
 * @brief  Contains Navigation key definitions.
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
 * David Wang  2018/4/2    0.1.0      Init version
 * Jack Li     2018/6/12   0.1.1      Add more navi mode
 *
 *****************************************************************************/

/* This file must be the same between CA and TA APP */

#ifndef __SILEAD_KEY_H__
#define __SILEAD_KEY_H__

enum _nav_type {
    NAV_TYPE_NONE = 0,
    NAV_TYPE_NORMAL,
    NAV_TYPE_LITE,
    NAV_TYPE_DIR_UPDOWN,
    NAV_TYPE_DIR_LR,
    NAV_TYPE_CLICK,
    NAV_TYPE_MAX,
};

enum _nav_key_flag {
    NAV_KEY_FLAG_UP = 0,
    NAV_KEY_FLAG_DOWN,
    NAV_KEY_FLAG_CLICK,
};

enum _nav_key_value {
    NAV_KEY_UNKNOWN = 0,
    NAV_KEY_UP,
    NAV_KEY_DOWN,
    NAV_KEY_RIGHT,
    NAV_KEY_LEFT,
    NAV_KEY_CLICK,
    NAV_KEY_DCLICK,
    NAV_KEY_LONGPRESS,
    // for lite nav
    NAV_KEY_CLICK_DOWN,
    NAV_KEY_CLICK_UP,
    NAV_KEY_MAX,
    NAV_KEY_WAITMORE = 1000,
};

#define IS_KEY_VALID(k) ((k) > NAV_KEY_UNKNOWN && (k) < NAV_KEY_MAX)
#define IS_NAVI_KEY(k) ((k) == NAV_KEY_UP || (k) == NAV_KEY_DOWN || (k) == NAV_KEY_RIGHT || (k) == NAV_KEY_LEFT)

static inline const char *silead_key_get_des(int32_t key)
{
    static const char *keyString[] = {
        "NAV_KEY_UNKNOWN",
        "NAV_KEY_UP",
        "NAV_KEY_DOWN",
        "NAV_KEY_RIGHT",
        "NAV_KEY_LEFT",
        "NAV_KEY_CLICK",
        "NAV_KEY_DCLICK",
        "NAV_KEY_LONGPRESS",
        "NAV_KEY_CLICK_DOWN",
        "NAV_KEY_CLICK_UP",
    };

    if (IS_KEY_VALID(key)) {
        return keyString[key];
    }

    return "UNKNOWN";
}

#endif /* __SILEAD_KEY_H__ */
