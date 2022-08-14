/******************************************************************************
 * @file   silead_dump.c
 * @brief  Contains dump image functions.
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
 * <author>        <date>   <version>     <desc>
 * Luke Ma         2018/7/2    0.1.0      Init version
 * Bangxiong.Wu    2019/3/10   1.0.0      Add for saving calibrate data
 *****************************************************************************/
#define FILE_TAG "silead_dump"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "silead_const.h"
#include "silead_bmp.h"
#include "silead_cmd.h"
#include "silead_util.h"
#include "silead_dump.h"

#ifdef SIL_DUMP_IMAGE

#ifndef SIL_DUMP_DATA_PATH//data path,can be erased
#define SIL_DUMP_DATA_PATH "/data/vendor/silead"
#endif

#ifndef SIL_FP_CONFIG_PATH//config path,cannot be erased
#define SIL_FP_CONFIG_PATH "/mnt/vendor/persist/fingerprint/silead"
#endif

#define TEST_DUMP_DATA_TYPE_IMG         0x51163731
#define TEST_DUMP_DATA_TYPE_NAV         0x51163732
#define TEST_DUMP_DATA_TYPE_RAW         0x51163733
#define TEST_DUMP_DATA_TYPE_FT_QA       0x51163734
#define TEST_DUMP_DATA_TYPE_SNR         0x51163735
#define TEST_DUMP_DATA_TYPE_ENROLL_NEW  0x51163736
#define TEST_DUMP_DATA_TYPE_CAL         0x51163737
#define TEST_DUMP_DATA_TYPE_AUTH        0x51163738

#define DUMP_STEP_MAX 30
#define DUMP_ENABLED 0x64736377

static char* m_test_dump_buffer = NULL;
static uint32_t m_test_dump_buffer_size = 0;

static char m_str_dump_path[MAX_PATH_LEN] = {0};
static const char *_dump_get_path(void)
{
    if (m_str_dump_path[0] != '\0') {
        return m_str_dump_path;
    } else {
        return SIL_DUMP_DATA_PATH;
    }
}

static const char *_dump_get_config_path(void)
{
    if (m_str_dump_path[0] != '\0') {
        return m_str_dump_path;
    } else {
        return SIL_FP_CONFIG_PATH;
    }
}

void silfp_dump_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_dump_path, sizeof(m_str_dump_path), path, len);
    if (ret < 0) {
        memset(m_str_dump_path, 0, sizeof(m_str_dump_path));
    }
    LOG_MSG_VERBOSE("path = %s", m_str_dump_path);
}

const char *_dump_get_prefix(uint32_t idx)
{
    static const char *dump_prefix[] = {
        "auth_succ",
        "auth_fail",
        "enroll_succ",
        "enroll_fail",
        "nav_succ",
        "nav_fail",
        "shot_succ",
        "shot_fail",
        "raw",
        "cal",
        "ft",
        "auth_orig",
        "enroll_orig",
        "other_orig",
        "snr",
        "enroll_new",
    };

    if (idx < DUMP_IMG_MAX) {
        return dump_prefix[idx];
    }
    return "unknow";
}

#ifdef SIL_DUMP_IMAGE_DYNAMIC
int32_t _dump_get_level(char __unused *log, char __unused *lvl)
{
    int32_t dump_data_support = -1;
    static char m_test_dump_prop[32] = {0};
    int32_t prop = 0;

    if (m_test_dump_prop[0] == 0) {
#ifndef SIL_DUMP_SWITCH_PROP
        snprintf(m_test_dump_prop, sizeof(m_test_dump_prop), "%s.%s.%s.%s", "log", "tag", log, lvl);
#else
        snprintf(m_test_dump_prop, sizeof(m_test_dump_prop), "%s", SIL_DUMP_SWITCH_PROP);
#endif
    }
    prop = silfp_util_get_str_value(m_test_dump_prop, (uint8_t)DUMP_ENABLED);
    if (prop == DUMP_ENABLED) {
        dump_data_support = 1;
    }

    return dump_data_support;
}
#else
int32_t _dump_get_level(char __unused *log, char __unused *lvl)
{
    return 1;
}
#endif /* SIL_DUMP_IMAGE_DYNAMIC */

static int32_t _dump_init()
{
    int32_t ret = 0;
    uint32_t size = 0;
    int32_t dump_support = 0;

    dump_support = _dump_get_level("data", "dump");
    if (dump_support <= 0) {
        return -1;
    }

    ret = 0;
    if (m_test_dump_buffer == NULL) {
        ret = silfp_cmd_test_get_image_info(NULL, NULL, &size, NULL, NULL, NULL, NULL);
        if (size == 0) {
            LOG_MSG_ERROR("get the config from ta is invalid");
            ret = -1;
        }

        if (ret >= 0) {
            m_test_dump_buffer_size = size;
            m_test_dump_buffer = (char *)malloc(m_test_dump_buffer_size);
            if (m_test_dump_buffer == NULL) {
                m_test_dump_buffer_size = 0;
                ret = -1;
            } else {
                memset(m_test_dump_buffer, 0, m_test_dump_buffer_size);
            }
        }
    }
    LOG_MSG_INFO("dump support, note: should not be enabled in release version");

    return ret;
}

static int32_t _dump_get_ext_info(void *path, uint32_t size, void *extbuf, uint32_t *extsize)
{
    int32_t ret = 0;
    char *buf = NULL;
    uint32_t len = 1024;
    uint32_t result = 0;
    uint32_t result_path_len = 0;
    uint32_t result_ext_len = 0;

    if (path == NULL || size == 0) {
        return -1;
    }

    buf = malloc(len);
    if (buf == NULL) {
        LOG_MSG_ERROR("malloc buffer failed");
        return -1;
    }

    memset(buf, 0, len);
    silfp_util_path_copy(buf, len, path, strlen(path));

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DUMP_FILE_EXT, (uint8_t *)buf, (uint32_t *)&len, &result);
    if ((ret < 0) || (result == 0)) {
        LOG_MSG_ERROR("not have ext info (%d %d)", ret, result);
        ret = -1;
    } else {
        result_path_len = (result & 0x0000FFFF);
        result_ext_len = ((result >> 16) & 0x0000FFFF);
        if (result_path_len + result_ext_len > len) {
            LOG_MSG_ERROR("param invalid, (%u %u:%u)", len, result_path_len, result_ext_len);
            ret = -1;
        } else {
            if (result_path_len > 0) {
                silfp_util_path_copy(path, size, buf, result_path_len);
            }
            if (result_ext_len > 0 && extbuf != NULL && extsize != NULL && *extsize > 0) {
                if (*extsize > result_ext_len) {
                    *extsize = result_ext_len;
                } else {
                    LOG_MSG_ERROR("not have enough buffer (%d but %d), split", result_ext_len, *extsize);
                }
                memcpy(extbuf, buf + result_path_len, *extsize);
            } else {
                ret = -1;
            }
        }
    }

    free(buf);
    return ret;
}

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
void silfp_dump_algorithm_parameter(uint32_t step, algotirhm_paramater_t *para, int32_t reserve)
{
    char path[MAX_PATH_LEN] = {0};
    uint32_t ret = -1;

    ret = silfp_txt_get_save_path(path, sizeof(path), _dump_get_config_path(), "calibration");
    if (ret < 0) {
        LOG_MSG_ERROR("algo get save path failed");
        return ;
    }

    ret = silfp_txt_save(path, para->ft, step, reserve);
    if (ret < 0) {
        LOG_MSG_ERROR("algo save failed");
    }

    return ;
}
#endif

void silfp_dump_data(e_mode_dump_img_t type)
{
    int32_t ret = 0;
    uint32_t w = 0;
    uint32_t h = 0;
    uint32_t imgsize = 0;
    uint8_t bitcount = 0;
    uint32_t dump_step = 0;
    uint32_t dump_remaining = 0;
    uint32_t mode = TEST_DUMP_DATA_TYPE_IMG;
    char path[MAX_PATH_LEN] = {0};
    char *ext_buf = NULL;
    uint32_t ext_max_size = 256;
    uint32_t ext_size = 0;

    ret = _dump_init();
    if (ret < 0) {
        return;
    }

    if (m_test_dump_buffer == NULL) {
        LOG_MSG_ERROR("dump buffer NULL");
        return;
    }

    ext_buf = malloc(ext_max_size);
    if (ext_buf == NULL) {
        LOG_MSG_ERROR("malloc ext image info buffer failed");
        return;
    }

    switch(type) {
    case DUMP_IMG_NAV_SUCC:
    case DUMP_IMG_NAV_FAIL: {
        mode = TEST_DUMP_DATA_TYPE_NAV;
        break;
    }
    case DUMP_IMG_RAW:
    case DUMP_IMG_AUTH_ORIG:
    case DUMP_IMG_ENROLL_ORIG:
    case DUMP_IMG_OTHER_ORIG: {
        mode = TEST_DUMP_DATA_TYPE_RAW;
        break;
    }
    case DUMP_IMG_CAL: {
        mode = TEST_DUMP_DATA_TYPE_CAL;
        break;
    }
    case DUMP_IMG_FT_QA: {
        mode = TEST_DUMP_DATA_TYPE_FT_QA;
        break;
    }
    case DUMP_IMG_SNR: {
        mode = TEST_DUMP_DATA_TYPE_SNR;
        break;
    }
    case DUMP_IMG_ENROLL_NEW: {
        mode = TEST_DUMP_DATA_TYPE_ENROLL_NEW;
        break;
    }
#ifndef SIL_CODE_COMPATIBLE // ????
    case DUMP_IMG_AUTH_SUCC:
    case DUMP_IMG_AUTH_FAIL: {
        mode = TEST_DUMP_DATA_TYPE_AUTH;
        break;
    }
#endif
    default: {
        mode = TEST_DUMP_DATA_TYPE_IMG;
        break;
    }
    }

    do {
        memset(m_test_dump_buffer, 0, m_test_dump_buffer_size);
        ret = silfp_cmd_test_dump_data(mode, dump_step, m_test_dump_buffer, m_test_dump_buffer_size, &dump_remaining, &imgsize, &w, &h, &bitcount);
        if (imgsize > 0) {
            LOG_MSG_DEBUG("%d (%d:%d:%d)", imgsize, w, h, bitcount);
            if (silfp_bmp_get_save_path(path, sizeof(path), _dump_get_path(), _dump_get_prefix(type)) >= 0) {
                ext_size = ext_max_size;
                memset(ext_buf, 0, ext_size);
                if (_dump_get_ext_info(path, sizeof(path), ext_buf, &ext_size) >= 0) {
                    silfp_bmp_save(path, m_test_dump_buffer, imgsize, w, h, bitcount, ext_buf, ext_size, 0);
                } else {
                    silfp_bmp_save(path, m_test_dump_buffer, imgsize, w, h, bitcount, NULL, 0, 0);
                }
            }
        }
        dump_step++;
    } while (ret >= 0 && dump_step < DUMP_STEP_MAX && dump_remaining > 0);

    if (ext_buf != NULL) {
        free(ext_buf);
        ext_buf = NULL;
    }
}

void silfp_dump_deinit(void)
{
    if (m_test_dump_buffer != NULL) {
        free(m_test_dump_buffer);
    }
    m_test_dump_buffer = NULL;
    m_test_dump_buffer_size = 0;
}

void silfp_dump_deadpx_result(const char *desc, uint32_t value)
{
    char path[MAX_PATH_LEN] = {0};
    FILE *fp = NULL;

    if (desc == NULL) {
        LOG_MSG_ERROR("desc is NULL");
        return;
    }

    snprintf(path, sizeof(path), "%s/%s", _dump_get_path(), "deadpx.dat");

    fp = fopen(path, "a+");
    if (fp == NULL) {
        LOG_MSG_ERROR("open file failed");
        return;
    }

    fprintf(fp, "%s %d\n", desc, value);
    fclose(fp);
}

#else

void silfp_dump_set_path(const void __unused *path, uint32_t __unused len)
{
}

void silfp_dump_deadpx_result(const char __unused *desc, uint32_t __unused value)
{
}
#endif /* SIL_DUMP_IMAGE */
