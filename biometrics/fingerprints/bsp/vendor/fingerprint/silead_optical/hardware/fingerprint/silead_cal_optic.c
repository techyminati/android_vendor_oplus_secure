/**************************************************************************************************
 * @file   silead_cal_optic.c
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
 * Bangxiong.Wu 2019/2/25  0.1.1      modify BUF_SIZE from 130*1024 to 801*1024 for new debase algo
 *
 *************************************************************************************************/

#define FILE_TAG "silead_cal_opt"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_cmd.h"
#include "silead_util.h"
#include "silead_dump.h"
#include "silead_cal.h"

#ifndef SIL_FP_CONFIG_PATH
#define SIL_FP_CONFIG_PATH "/mnt/vendor/persist/fingerprint/silead"
#endif

#define FP_OPTIC_CAL_NAME1 "fpcal1.dat"
#define FP_OPTIC_CAL_NAME2 "fpcal2.dat"
#define FP_OPTIC_CAL_NAME3 "fpcal3.dat"
#define FP_OPTIC_CAL_NAME4 "fpcal4.dat"
#define FP_OPTIC_CAL_NAME5 "fpcal5.dat"

#define BUF_SIZE 801*1024

#define CAL_INVALID 0x00
#define INIT_CAL_VALID 0x01
#define INIT_CAL_INVALID 0x02

#define CAL_MIN_SIZE 10
#define CAL_HEADER_SIZE 64

static char m_str_cal_path[MAX_PATH_LEN] = {0};
static const char *_cal_optic_get_path(void)
{
    if (m_str_cal_path[0] != '\0') {
        return m_str_cal_path;
    } else {
        return SIL_FP_CONFIG_PATH;
    }
}

void silfp_cal_optic_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_cal_path, sizeof(m_str_cal_path), path, len);
    if (ret < 0) {
        memset(m_str_cal_path, 0, sizeof(m_str_cal_path));
    }
    LOG_MSG_VERBOSE("path = %s", m_str_cal_path);
}

static const char *_cal_optic_get_name(uint32_t step)
{
    switch(step) {
    case 1:
        return FP_OPTIC_CAL_NAME1;
    case 2:
        return FP_OPTIC_CAL_NAME2;
    case 3:
        return FP_OPTIC_CAL_NAME3;
    case 4:
        return FP_OPTIC_CAL_NAME4;
    case 5:
        return FP_OPTIC_CAL_NAME5;
    default:
        return NULL;
    }
}

static int32_t _cal_optic_get_file(char *path, uint32_t size, uint32_t step)
{
    const char *filename = NULL;

    if (path == NULL || size == 0) {
        return -SL_ERROR_BAD_PARAMS;
    }

    filename = _cal_optic_get_name(step);
    if (filename == NULL) {
        return -1;
    }

    snprintf(path, size, "%s/%s", _cal_optic_get_path(), filename);
    return 0;
}

static int32_t _cal_optic_step(uint32_t step, uint32_t init)
{
    int32_t ret = 0;
    char *buf = NULL;
    int32_t len = BUF_SIZE;
    uint32_t buf_flag = CAL_INVALID;
    char path[MAX_PATH_LEN] = {0};

    if (step < 1 || step > 5) { // step 1~5
        return ret;
    }

    ret = _cal_optic_get_file(path, sizeof(path), step);
    if (init && ret >= 0) {
        len = silfp_util_file_get_size(path);
        LOG_MSG_DEBUG("get %s size %d", path, len);

        if (len < CAL_MIN_SIZE) {
            len = BUF_SIZE;
            buf_flag = INIT_CAL_INVALID;
        } else {
            len += CAL_HEADER_SIZE; // Allocate for head structure.
        }
    }

    buf = malloc(len);
    if (buf == NULL) {
        LOG_MSG_ERROR("Malloc fail!!!");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memset(buf, 0, len);

    if (init && (buf_flag != INIT_CAL_INVALID)) {
        ret = silfp_util_file_load(_cal_optic_get_path(), _cal_optic_get_name(step), buf, len - CAL_HEADER_SIZE);
        if (ret == len - CAL_HEADER_SIZE) {
            buf_flag = INIT_CAL_VALID;
        } else {
            buf_flag = INIT_CAL_INVALID;
        }
    }

    if ((step >= 1) && (step <= 3)) {
        silfp_cmd_download_normal();
    }
    LOG_MSG_DEBUG("step %d, buf_flag %d", step, buf_flag);

    ret = silfp_cmd_calibrate_optic(step, buf, (uint32_t *)&len, buf_flag);
    if ((ret >= 0) && (len > 0)) {
#ifdef SIL_CODE_COMPATIBLE // ????
        if ((!init) || (init && (ret > CAL_MIN_SIZE))) {
#else
        if ((!init) || (init && (len > CAL_MIN_SIZE))) {
#endif
            silfp_util_file_save(_cal_optic_get_path(), _cal_optic_get_name(step), buf, len);
            if ((step >= 1) && (step <= 3)) {
                if (_cal_optic_get_file(path, sizeof(path), 4) >= 0) {
                    silfp_util_file_remove(path);
                }
                if (_cal_optic_get_file(path, sizeof(path), 5) >= 0) {
                    silfp_util_file_remove(path);
                }
            }
        }
    }

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    if ((step >= 1) && (step <= 3)) {
        silfp_dump_data(DUMP_IMG_CAL);
    }

    return ret;
}

int32_t silfp_cal_optic_step(uint32_t step)
{
    return _cal_optic_step(step, 0);
}

int32_t silfp_cal_optic_calibrate(void)
{
    int32_t ret = 0;

    ret = _cal_optic_step(1, 1);
    ret |= _cal_optic_step(2, 1);
    ret |= _cal_optic_step(3, 1);
    ret |= _cal_optic_step(4, 1);
    ret |= _cal_optic_step(5, 1);

    return ret;
}
