/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_EVENT_H_
#define _GF_EVENT_H_

typedef enum {
    EVENT_UNKNOWN = -1000,
    EVENT_SCREEN_ON,
    EVENT_SCREEN_OFF,
    EVENT_IRQ,
    EVENT_FINGER_DOWN = 1 << 1,
    EVENT_FINGER_UP = 1 << 2,
    EVENT_IRQ_RESET = 1 << 3,
} gf_event_type_t;

typedef struct gf_netlink_tp_info {
    uint8_t touch_state;
    uint8_t area_rate;
    uint16_t x;
    uint16_t y;
}gf_netlink_tp_info_t;

typedef struct gf_netlink_msg_info {
    uint8_t netlink_cmd;
    gf_netlink_tp_info_t fp_info;
}gf_netlink_msg_info_t;


#endif  // _GF_EVENT_H_
