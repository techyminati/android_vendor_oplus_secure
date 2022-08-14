/******************************************************************************
 * @file   silead_debug.c
 * @brief  Contains debug functions.
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
 * Luke Ma     2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/
#define FILE_TAG "silead_debug"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_util.h"
#include "silead_stats.h"
#include "silead_cmd.h"
#include "silead_dev.h"
#include "silead_debug.h"

#define LOG_KERNEL_ERR 0
#define LOG_KERNEL_DEBUG 1
#define LOG_KERNEL_VERBOSE 2
#define LOG_TA_VERBOSE 0
#define LOG_TA_DEBUG 1
#define LOG_TA_INFO 2
#define LOG_TA_WARNING 3
#define LOG_TA_ERR 4
#define LOG_ALG_ERR 0
#define LOG_ALG_DEBUG 1
#define LOG_ALG_VERBOSE 2
#define LOG_STATS_DISABLE 0
#define LOG_STATS_ENABLE 1
#define LOG_STATS_RESET 2

#define DEBUG_ENABLE  0x64736377
#define DEBUG_DEFAULT 0x6E736377

#ifdef SIL_DEBUG_ALL_LOG
#ifdef SIL_DEBUG_LOG_DYNAMIC

static int32_t _dbg_get_all_log_level(char *log, char *lvl, uint8_t special)
{
    static char s_log_prop[32] = {0};
    int32_t prop = -1;

    if (s_log_prop[0] == 0) {
        snprintf(s_log_prop, sizeof(s_log_prop), "%s.%s.%s.%s.%s", "persist", "log", "tag", log, lvl);
    }

    prop = silfp_util_get_str_value(s_log_prop, special);
    if (prop == 0) {
        prop = silfp_util_get_str_value(s_log_prop + 8, special);
    }
    return prop;
}

static int32_t _dbg_update_all_log_level(uint8_t *logs, uint32_t size)
{
    static uint8_t s_log_level[4] = {LOG_KERNEL_VERBOSE, LOG_TA_VERBOSE, LOG_ALG_VERBOSE, LOG_STATS_ENABLE};
    static int32_t s_all_level = -1;

    int32_t i = 0;
    uint8_t lvl = 0;

    int32_t all_level = _dbg_get_all_log_level("fp", "lvl", (uint8_t)DEBUG_ENABLE);
    if (all_level != s_all_level) {
        s_all_level = all_level;
        if (DEBUG_ENABLE == all_level) {
            s_log_level[0] = LOG_KERNEL_VERBOSE;
            s_log_level[1] = LOG_TA_VERBOSE;
            s_log_level[2] = LOG_ALG_VERBOSE;
            s_log_level[3] = LOG_STATS_ENABLE;
            silfp_log_set_level(0);
        } else if (DEBUG_DEFAULT == all_level) {
            s_log_level[0] = LOG_KERNEL_DEBUG;
            s_log_level[1] = LOG_TA_INFO;
            s_log_level[2] = LOG_ALG_ERR;
            s_log_level[3] = LOG_STATS_ENABLE;
            silfp_log_set_level(-1);
        } else if ((all_level & 0x51000000) == 0x51000000) {
            for (i = 0; i < 4; i++) {
                lvl = (all_level >> (i*6)) & 0x0000003F;
                if (((lvl & 0x30) == 0x30) && (s_log_level[i] != (lvl & 0x0F))) {
                    s_log_level[i] = (lvl & 0x0F);
                }
            }
        }

        if (logs != NULL && size >= 4) {
            memcpy(logs, s_log_level, 4);
            return 0;
        }
    }

    return -1;
}
#endif
#endif

static int32_t _dbg_set_log_mode(uint8_t krm, uint8_t tam, uint8_t agm)
{
    int32_t ret = 0;

    LOG_MSG_VERBOSE("krm = %d, tam = %d, agm = %d", krm, tam, agm);

    ret = silfp_dev_set_log_level(krm);
    ret = silfp_cmd_set_log_mode(tam, agm);

    return ret;
}

static int32_t _dbg_get_to_file_mode(char *log, char *lvl, uint8_t special)
{
    static char s_log_to_file_prop[32] = {0};
    int32_t prop = -1;

    if (s_log_to_file_prop[0] == 0) {
        snprintf(s_log_to_file_prop, sizeof(s_log_to_file_prop), "%s.%s.%s.%s.%s", "persist", "log", "tag", log, lvl);
    }

    prop = silfp_util_get_str_value(s_log_to_file_prop, special);
    return prop;
}

static void _dbg_update_to_file_mode(void)
{
    static int32_t s_dump_to_file = 0;
    uint8_t enable = 0;

    int32_t prop = _dbg_get_to_file_mode("fp", "file", (uint8_t)DEBUG_ENABLE);
    if (s_dump_to_file != prop) {
        s_dump_to_file = prop;
        if (DEBUG_ENABLE == prop) {
            enable = 1;
        } else {
            enable = 0;
        }
        silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_DUMP_TA_LOG_ENABLE, &enable, sizeof(enable));
        silfp_log_dump_to_file(enable);
    }
}

int32_t silfp_dbg_update_all_log_level(void)
{
#ifdef SIL_DEBUG_ALL_LOG
#ifdef SIL_DEBUG_LOG_DYNAMIC
    uint8_t log_level[4] = {LOG_KERNEL_VERBOSE, LOG_TA_VERBOSE, LOG_ALG_VERBOSE, LOG_STATS_ENABLE};
    if (_dbg_update_all_log_level(log_level, 4) >= 0) {
        if (log_level[3] == LOG_STATS_DISABLE || (log_level[3] == LOG_STATS_ENABLE)) {
            silfp_stats_set_enabled(log_level[3]);
        } else if (log_level[3] == LOG_STATS_RESET) {
            silfp_stats_reset();
        }
        _dbg_set_log_mode(log_level[0], log_level[1], log_level[2]);
    }
#else
    static int32_t already_set = 0;
    if (!already_set) {
        _dbg_set_log_mode(LOG_KERNEL_VERBOSE, LOG_TA_VERBOSE, LOG_ALG_VERBOSE);
        silfp_stats_set_enabled(LOG_STATS_ENABLE);
        already_set = 1;
    }
#endif
#else
    static int32_t already_set = 0;
    if (!already_set) {
        _dbg_set_log_mode(LOG_KERNEL_ERR, LOG_TA_INFO, LOG_ALG_ERR);
        already_set = 1;
    }
#endif

    _dbg_update_to_file_mode();

    return 0;
}
