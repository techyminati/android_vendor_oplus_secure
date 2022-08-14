/******************************************************************************
 * @file   silead_log.h
 * @brief  Contains log message header file.
 *
 *
 * Copyright (c) 2016-2019 Silead Inc.
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
 * calvin wang     2018/8/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_LOG_H__
#define __SILEAD_LOG_H__

#ifdef SIL_DEBUG_LOG_DUMP_DYNAMIC

#include <pthread.h>

#define LOG_EVENTS_MAX 3
#define LOG_EVENT_WAKEUP 0
#define LOG_EVENT_CA_LOG 1
#define LOG_EVENT_TA_LOG 2

#define LOG_DUMP_BUF_CACHE_SIZE (12*1024)
#define LOG_DUMP_BUF_FLUSH_SIZE (10*1024)

typedef void (*log_event_cb)(int32_t fd, int32_t timeout);
typedef int32_t (*log_event_timeout)(void);

typedef struct _log_event {
    int32_t fd;
    log_event_cb func;
    log_event_timeout timeout;
} log_event_t;

void silfp_log_event_set(log_event_t *ev, int32_t fd, log_event_cb func, log_event_timeout timeout);
void silfp_log_event_add(int32_t index, log_event_t *ev);
void silfp_log_event_del(int32_t index);

int32_t silfp_log_is_event_thread(pthread_t tid);
void silfp_log_file_flush(const char *pname, int32_t *pbak_idx, const void *pbuf, uint32_t len);

int32_t silfp_log_ca_dump_init(void);
void silfp_log_ca_dump_deinit(void);
void silfp_log_ca_dump_to_file(int32_t enable);

int32_t silfp_log_ta_dump_init(void);
void silfp_log_ta_dump_deinit(void);
void silfp_log_ta_dump_to_file(int32_t enable);

#define LOG_DUMP_TO_FILE_NONE   (0x00)
#define LOG_DUMP_TO_FILE_CA     (0x01)
#define LOG_DUMP_TO_FILE_TA     (0x02)
#define LOG_DUMP_TO_LOGC_TA     (0x04)
void silfp_log_dump_to_file(int32_t enable);

int32_t silfp_dbg_check_ca_to_file_mode(void);

#endif /* SIL_DEBUG_LOG_DUMP_DYNAMIC */

#endif /* __SILEAD_LOG_H__ */