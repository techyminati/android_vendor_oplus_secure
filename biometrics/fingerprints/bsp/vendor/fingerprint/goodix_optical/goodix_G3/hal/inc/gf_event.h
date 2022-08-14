/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_EVENT_H_
#define _GF_EVENT_H_

typedef enum
{
    EVENT_UNKNOWN = -1000,
    EVENT_SCREEN_ON,
    EVENT_SCREEN_OFF,
    EVENT_IRQ,
    EVENT_FINGER_DOWN = 1 << 1,
    EVENT_FINGER_UP = 1 << 2,
    EVENT_IRQ_RESET = 1 << 3,
    EVENT_UI_READY = 1 << 4,
} gf_event_type_t;

#endif  // _GF_EVENT_H_
