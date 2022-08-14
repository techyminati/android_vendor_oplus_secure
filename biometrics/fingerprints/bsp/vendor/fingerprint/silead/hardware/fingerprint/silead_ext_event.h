/******************************************************************************
 * @file   silead_fingerext_event.h
 * @brief  Contains fingerprint extension event operate functions header file.
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
 *
 *****************************************************************************/

#ifndef __SILEAD_EXT_EVENT_H__
#define __SILEAD_EXT_EVENT_H__

#ifdef SIL_FP_EXT_SKT_SERVER_ENABLE

#define MAX_FD_EVENTS 16

typedef void (*event_cb)(int32_t fd, short events, void *userdata);

typedef struct _ext_event {
    struct _ext_event *next;
    struct _ext_event *prev;

    int32_t fd;
    int32_t index;
    int32_t persist;
    event_cb func;
    void *param;
} ext_event_t;

void silfp_ext_event_init();
void silfp_ext_event_deinit();

void silfp_ext_event_set(ext_event_t *ev, int32_t fd, int32_t persist, event_cb func, void *param);

void silfp_ext_event_add(ext_event_t *ev);
void silfp_ext_event_del(ext_event_t *ev);

void silfp_ext_event_loop();
void silfp_ext_event_exit();

#endif // SIL_FP_EXT_SKT_SERVER_ENABLE

#endif // __SILEAD_EXT_EVENT_H__