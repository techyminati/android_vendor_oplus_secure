/******************************************************************************
 * @file   silead_calibrate.c
 * @brief  Contains fingerprint calibrate operate functions.
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

#define FILE_TAG "silead_cal_norm"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_cmd.h"
#include "silead_util.h"
#include "silead_cal.h"

#define FP_FEATURE_NEED_CALIBRATE_MASK 0x0008
#define FP_FEATURE_NEED_CALIBRATE2_WITH_NOFINGER_MASK 0x0010
#define FP_FEATURE_NEED_CALIBRATE2_MASK 0x0020

#define FP_CONFIG_UPDATE_MASK (FP_CONFIG_CHG_BIT | FP_CONFIG_UP_BIT | FP_CONFIG_DOWN_BIT)
#define FP_CONFIG_NEED_UPDATE(x) (FP_CONFIG_UPDATE_MASK == ((x) & FP_CONFIG_UPDATE_MASK))

#define FP_CONFIG_PATH "/data/vendor/silead"
#define FP_CONFIG_NAME "fpconfig.dat"

extern int32_t silfp_impl_wait_finger_up(void);

static uint32_t m_cal_cfg_max_size = 128*128;

static uint32_t m_need_calibrate = 0;
static uint32_t m_need_calibrate2_with_nofinger = 0;
static uint32_t m_need_calibrate2 = 0;

static uint32_t m_config_update = 0;

static char m_str_cal_path[MAX_PATH_LEN] = {0};
static const char *_cal_normal_get_path(void)
{
    if (m_str_cal_path[0] != '\0') {
        return m_str_cal_path;
    } else {
        return FP_CONFIG_PATH;
    }
}

void silfp_cal_normal_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_cal_path, sizeof(m_str_cal_path), path, len);
    if (ret < 0) {
        memset(m_str_cal_path, 0, sizeof(m_str_cal_path));
    }
    LOG_MSG_VERBOSE("path = %s", m_str_cal_path);
}

int32_t silfp_cal_normal_init(uint32_t feature)
{
    m_config_update = 0;

    m_need_calibrate = (feature & FP_FEATURE_NEED_CALIBRATE_MASK) ? 1 : 0;
    m_need_calibrate2_with_nofinger = (feature & FP_FEATURE_NEED_CALIBRATE2_WITH_NOFINGER_MASK) ? 1 : 0;
    m_need_calibrate2 = (feature & FP_FEATURE_NEED_CALIBRATE2_MASK) ? 1 : 0;

    LOG_MSG_VERBOSE("calibrate=%d, calibrate2_nofinger=%d, calibrate2=%d, m_cal_cfg_max_size=%d",
                    m_need_calibrate, m_need_calibrate2_with_nofinger, m_need_calibrate2, m_cal_cfg_max_size);

    return 0;
}

int32_t silfp_cal_normal_deinit(void)
{
    m_config_update = 0;

    m_need_calibrate = 0;
    m_need_calibrate2_with_nofinger = 0;
    m_need_calibrate2 = 0;

    return 0;
}

static int32_t _cal_normal_save_config(void)
{
    int32_t ret = 0;
    char *buf = NULL;
    uint32_t len = 0;

    if (!FP_CONFIG_NEED_UPDATE(m_config_update)) {
        return ret;
    }

    m_config_update = 0;

    if (m_need_calibrate || m_need_calibrate2) {
        len = m_cal_cfg_max_size;
        buf = malloc(len);
        if (buf == NULL) {
            LOG_MSG_ERROR("Malloc fail!!!");
            return -SL_ERROR_OUT_OF_MEMORY;
        }

        memset(buf, 0, len);
        ret = silfp_cmd_get_config(buf, &len);
        if (ret >= 0 && len > 4) {
            ret = silfp_util_file_save(_cal_normal_get_path(), FP_CONFIG_NAME, buf, len);
        }
        LOG_MSG_DEBUG("save config, ret = %d", ret);
        free(buf);
    }
    return ret;
}

static int32_t _cal_normal_get_config_data(void **buffer)
{
    int32_t ret = 0;
    void *buf = NULL;
    int32_t len = 0;

    char path[MAX_PATH_LEN] = {0};
    snprintf(path, sizeof(path), "%s/%s", _cal_normal_get_path(), FP_CONFIG_NAME);

    len = silfp_util_file_get_size(path);
    LOG_MSG_DEBUG("get %s size %d", path, len);
    if (len <= 0) {
        return 0;
    }

    buf = malloc(len);
    if (buf == NULL) {
        LOG_MSG_ERROR("Malloc fail!!!");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memset(buf, 0, len);
    ret = silfp_util_file_load(_cal_normal_get_path(), FP_CONFIG_NAME, buf, len);
    if (ret < 0) {
        free(buf);
        buf = NULL;
        len = 0;
    } else if (buffer != NULL) {
        *buffer = buf;
    }

    return len;
}

int32_t silfp_cal_normal_calibrate(void)
{
    int32_t ret = 0;
    void *buffer = NULL;

    if (m_need_calibrate) {
        silfp_cmd_download_normal();
        ret = _cal_normal_get_config_data(&buffer);
        if (ret >= 0) {
            ret = silfp_cmd_calibrate(buffer, ret);
        }
        if (buffer != NULL) {
            free(buffer);
            buffer = NULL;
        }
    }

    if (ret == -SL_ERROR_CONFIG_INVALID) {
        m_config_update = FP_CONFIG_CHG_BIT;
        if (m_need_calibrate2_with_nofinger) {
            ret = silfp_impl_wait_finger_up();
        }
    }

    if (m_need_calibrate2) {
        silfp_cmd_download_normal();
        ret = silfp_cmd_calibrate2();
    }

    if (m_need_calibrate || m_need_calibrate2) {
        silfp_cmd_download_normal();
    }

    return ret;
}

int32_t silfp_cal_normal_update_cfg(uint32_t status, int32_t reset)
{
    if (m_need_calibrate && m_need_calibrate2) {
        if (reset) {
            m_config_update = status;
        } else {
            m_config_update |= status;
            _cal_normal_save_config();
        }
    }

    return 0;
}

int32_t silfp_cal_normal_need_nav_cal(void)
{
    return (m_config_update & FP_CONFIG_CHG_BIT);
}

int32_t silfp_cal_normal_reset(void)
{
    char path[MAX_PATH_LEN] = {0};

    int32_t count = silfp_cmd_get_db_count();
    if (count > 0) {
        return -1;
    }

    LOG_MSG_DEBUG("no fingers, reset cal");
    m_config_update = 0;

    snprintf(path, sizeof(path), "%s/%s", _cal_normal_get_path(), FP_CONFIG_NAME);
    silfp_util_file_remove(path);

    return 0;
}