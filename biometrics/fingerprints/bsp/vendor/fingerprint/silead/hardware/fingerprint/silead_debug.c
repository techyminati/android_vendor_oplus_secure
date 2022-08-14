/******************************************************************************
 * @file   silead_debug.c
 * @brief  Contains debug functions.
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
 * calvin wang     2018/7/2    0.1.0      Init version
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
#include "silead_log.h"

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

/**
 * dump log:
 *      0x61736377	----kernel(verbose) ta(verbose) alg(verbose) ca(verbose) stat(enable)
 *      0x64736377	----kernel(verbose) ta(debug)   alg(verbose) ca(debug)   stat(enable)
 *      0x6E736377	----kernel(debug)   ta(info)    alg(error)   ca(default) stat(disable)
 * dump log to file:
 *      0x61736377	----dump ca & ta log to file
 *      0x64736377	----dump ta log to file
 *      0x6E736377	----dump ta log to logcat
 *      other		----disable dump
 */
#define DEBUG_ALL     0x61736377
#define DEBUG_ENABLE  0x64736377
#define DEBUG_DEFAULT 0x6E736377

#define LOG_DUMP_TO_FILE_ALL_PROP DEBUG_ALL
#define LOG_DUMP_TO_FILE_TA_PROP  DEBUG_ENABLE
#define LOG_DUMP_TO_LOGC_TA_PROP  DEBUG_DEFAULT

#ifdef SIL_DEBUG_ALL_LOG
#ifdef SIL_DEBUG_LOG_DYNAMIC
static int32_t _dbg_get_all_log_level(char *plog, char *plvl, uint8_t special)
{
    static char s_log_prop[32] = {0};
    int32_t prop = -1;

    if (s_log_prop[0] == 0) {
        snprintf(s_log_prop, sizeof(s_log_prop), "%s.%s.%s.%s.%s", "persist", "log", "tag", plog, plvl);
    }

    prop = silfp_util_get_str_value(s_log_prop + 8, special);
    if (prop == 0) {
        prop = silfp_util_get_str_value(s_log_prop, special);
    }
    return prop;
}

static int32_t _dbg_update_all_log_level(uint8_t *plogs, uint32_t size)
{
    static uint8_t s_log_level[4] = {LOG_KERNEL_DEBUG, LOG_TA_INFO, LOG_ALG_ERR, LOG_STATS_DISABLE};
    static int32_t s_all_level = -1;

    int32_t i = 0;
    uint8_t lvl = 0;

    int32_t all_level = _dbg_get_all_log_level("fp", "lvl", (uint8_t)DEBUG_ENABLE);
    if (all_level != s_all_level) {
        s_all_level = all_level;
        if (DEBUG_ALL == all_level) {
            s_log_level[0] = LOG_KERNEL_VERBOSE;
            s_log_level[1] = LOG_TA_VERBOSE;
            s_log_level[2] = LOG_ALG_VERBOSE;
            s_log_level[3] = LOG_STATS_ENABLE;
            silfp_log_set_level(0);
        } else if (DEBUG_ENABLE == all_level) {
            s_log_level[0] = LOG_KERNEL_VERBOSE;
            s_log_level[1] = LOG_TA_DEBUG;
            s_log_level[2] = LOG_ALG_VERBOSE;
            s_log_level[3] = LOG_STATS_ENABLE;
            silfp_log_set_level(1);
        } else if (DEBUG_DEFAULT == all_level) {
            s_log_level[0] = LOG_KERNEL_DEBUG;
            s_log_level[1] = LOG_TA_INFO;
            s_log_level[2] = LOG_ALG_ERR;
            s_log_level[3] = LOG_STATS_DISABLE;
            silfp_log_set_level(-1);
        } else if ((all_level & 0x51000000) == 0x51000000) {
            for (i = 0; i < 4; i++) {
                lvl = (all_level >> (i*6)) & 0x0000003F;
                if (((lvl & 0x30) == 0x30) && (s_log_level[i] != (lvl & 0x0F))) {
                    s_log_level[i] = (lvl & 0x0F);
                }
            }
        }

        if (plogs != NULL && size >= 4) {
            memcpy(plogs, s_log_level, 4);
            return 0;
        }
    }

    return -1;
}
#endif /* SIL_DEBUG_LOG_DYNAMIC */
#endif /* SIL_DEBUG_ALL_LOG */

static int32_t _dbg_set_log_mode(uint8_t krm, uint8_t tam, uint8_t agm)
{
    int32_t ret = 0;

    LOG_MSG_VERBOSE("krm = %d, tam = %d, agm = %d", krm, tam, agm);

    ret = silfp_dev_set_log_level(krm);
    ret = silfp_cmd_set_log_mode(tam, agm);

    return ret;
}

#ifdef SIL_DEBUG_LOG_DUMP_DYNAMIC
static int32_t _dbg_get_to_file_mode(char *plog, char *plvl, uint8_t special)
{
    static char s_log_to_file_prop[32] = {0};
    int32_t prop = -1;

    if (s_log_to_file_prop[0] == 0) {
        snprintf(s_log_to_file_prop, sizeof(s_log_to_file_prop), "%s.%s.%s.%s.%s", "persist", "log", "tag", plog, plvl);
    }

    prop = silfp_util_get_str_value(s_log_to_file_prop + 8, special);
    if (prop == 0) {
        prop = silfp_util_get_str_value(s_log_to_file_prop, special);
    }
    return prop;
}

static void _dbg_update_to_file_mode(void)
{
    static int32_t s_dump_to_file = 0;
    int32_t enable = LOG_DUMP_TO_FILE_NONE;

    int32_t prop = _dbg_get_to_file_mode("fp", "file", (uint8_t)DEBUG_ENABLE);
    if (s_dump_to_file != prop) {
        s_dump_to_file = prop;
        if (LOG_DUMP_TO_FILE_ALL_PROP == prop) {
            enable = (LOG_DUMP_TO_FILE_CA | LOG_DUMP_TO_FILE_TA);
        } else if (LOG_DUMP_TO_FILE_TA_PROP == prop) {
            enable = LOG_DUMP_TO_FILE_TA;
        } else if (LOG_DUMP_TO_LOGC_TA_PROP == prop) {
            enable = LOG_DUMP_TO_LOGC_TA;
        } else {
            enable = LOG_DUMP_TO_FILE_NONE;
        }
        silfp_log_dump_to_file(enable);
    }
}

int32_t silfp_dbg_check_ca_to_file_mode(void)
{
    int32_t prop = _dbg_get_to_file_mode("fp", "file", (uint8_t)DEBUG_ENABLE);
    if (prop == LOG_DUMP_TO_FILE_ALL_PROP) {
        return 1;
    }
    return -1;
}
#endif /* SIL_DEBUG_LOG_DUMP_DYNAMIC */

int32_t silfp_dbg_update_all_log_level(void)
{
#ifdef SIL_DEBUG_ALL_LOG
#ifdef SIL_DEBUG_LOG_DYNAMIC
    uint8_t log_level[4] = {LOG_KERNEL_DEBUG, LOG_TA_INFO, LOG_ALG_ERR, LOG_STATS_DISABLE};
    if (_dbg_update_all_log_level(log_level, 4) >= 0) {
        if (log_level[3] == LOG_STATS_DISABLE || (log_level[3] == LOG_STATS_ENABLE)) {
            silfp_stats_set_enabled(log_level[3]);
        } else if (log_level[3] == LOG_STATS_RESET) {
            silfp_stats_reset();
        }
        _dbg_set_log_mode(log_level[0], log_level[1], log_level[2]);
    }
#else /* !SIL_DEBUG_LOG_DYNAMIC */
    static int32_t already_set = 0;
    if (!already_set) {
        _dbg_set_log_mode(LOG_KERNEL_VERBOSE, LOG_TA_VERBOSE, LOG_ALG_VERBOSE);
        silfp_stats_set_enabled(LOG_STATS_ENABLE);
        already_set = 1;
    }
#endif /* SIL_DEBUG_LOG_DYNAMIC */
#else /* !SIL_DEBUG_ALL_LOG */
    static int32_t already_set = 0;
    if (!already_set) {
        _dbg_set_log_mode(LOG_KERNEL_ERR, LOG_TA_INFO, LOG_ALG_ERR);
        already_set = 1;
    }
#endif /* SIL_DEBUG_ALL_LOG */

#ifdef SIL_DEBUG_LOG_DUMP_DYNAMIC
    _dbg_update_to_file_mode();
#endif /* SIL_DEBUG_LOG_DUMP_DYNAMIC */

    return 0;
}