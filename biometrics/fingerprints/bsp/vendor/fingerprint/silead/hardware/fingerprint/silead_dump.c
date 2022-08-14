/******************************************************************************
 * @file   silead_dump.c
 * @brief  Contains dump image functions.
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
 * calvin wang  2018/1/2    0.1.0      Init version
 *
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

#ifndef SIL_DUMP_DATA_PATH
#define SIL_DUMP_DATA_PATH "/data/vendor/silead"
#endif

#ifndef SIL_DUMP_IMAGE_EXT
#define SIL_DUMP_IMAGE_EXT "bmp"
#endif

#define TEST_DUMP_DATA_TYPE_IMG         0x51163731
#define TEST_DUMP_DATA_TYPE_NAV         0x51163732
#define TEST_DUMP_DATA_TYPE_RAW         0x51163733
#define TEST_DUMP_DATA_TYPE_FT_QA       0x51163734
#define TEST_DUMP_DATA_TYPE_SNR         0x51163735
#define TEST_DUMP_DATA_TYPE_ENROLL_NEW  0x51163736
#define TEST_DUMP_DATA_TYPE_CAL         0x51163737
#define TEST_DUMP_DATA_TYPE_AUTH        0x51163738
#define TEST_DUMP_DATA_TYPE_MUL_RAW     0x51163739

#define DUMP_STEP_MAX 70
#define DUMP_ENABLED 0x64736377
#define DUMP_EXT_SIZE_MAX 512

#define CHECK_FLAG_SET(v, f) (!!((v) & (f)))
#define CHECK_FLAG_UNSET(v, f) (!((v) & (f)))

static char *m_test_dump_buffer = NULL;
static uint32_t m_test_dump_buffer_size = 0;

/* ext/mode of dump image file, max file name len */
static char m_str_dump_ext[16] = {0};
static int32_t m_dump_name_max_len = -1;
static uint32_t m_dump_path_mode = 0;
static const char *_dump_get_ext(void)
{
    if (m_str_dump_ext[0] != '\0') {
        return m_str_dump_ext;
    } else {
        return SIL_DUMP_IMAGE_EXT;
    }
}

void silfp_dump_set_name_mode(uint32_t mode, int32_t maxlen, const void *ext, uint32_t len)
{
    int32_t ret = 0;

    m_dump_name_max_len = maxlen;
    m_dump_path_mode = mode;

    ret = silfp_util_strcpy(m_str_dump_ext, sizeof(m_str_dump_ext), ext, len);
    if (ret < 0) {
        memset(m_str_dump_ext, 0, sizeof(m_str_dump_ext));
    }
    LOG_MSG_VERBOSE("ext=%s, mode=0x%x, maxlen=%d", _dump_get_ext(), m_dump_path_mode, m_dump_name_max_len);
}

/* dump dir */
static char m_str_dump_path[MAX_PATH_LEN] = {0};
static const char *_dump_get_path(void)
{
    if (m_str_dump_path[0] != '\0') {
        return m_str_dump_path;
    } else {
        return SIL_DUMP_DATA_PATH;
    }
}

void silfp_dump_set_path(const void *path, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_path_copy(m_str_dump_path, sizeof(m_str_dump_path), path, len);
    if (ret < 0) {
        memset(m_str_dump_path, 0, sizeof(m_str_dump_path));
    }
    LOG_MSG_VERBOSE("path = %s", _dump_get_path());
}

/* dump image prefix & sub dir */
static const char *_dump_get_prefix(uint32_t idx)
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
        "auth_mraw",
    };

    if (idx < DUMP_IMG_MAX) {
        return dump_prefix[idx];
    }
    return "unknow";
}

static const char *_dump_get_sub_dir(uint32_t idx)
{
    static const char *dump_sub_dir[] = {
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
        "auth_mraw",
    };

    if (idx < DUMP_IMG_MAX) {
        return dump_sub_dir[idx];
    }
    return "unknow";
}

/* dump reboot times, to distinguish the dump image when reboot the device or kill bio service */
#define DUMP_REBOOT_TIMES_FILE "bmp_times.dat"
#define DUMP_REBOOT_TIMES_NO_GET ((uint32_t)-1)
static uint32_t _dump_get_reboot_times(const char *dir)
{
    static uint32_t s_reboot_times = DUMP_REBOOT_TIMES_NO_GET;

    int32_t ret = 0;
    char path[MAX_PATH_LEN] = {0};
    void *buf = NULL;
    int32_t len = 0;

    if (s_reboot_times == DUMP_REBOOT_TIMES_NO_GET) {
        if (dir != NULL && dir[0] != '\0') {
            snprintf(path, sizeof(path), "%s/%s", dir, DUMP_REBOOT_TIMES_FILE);
        } else {
            snprintf(path, sizeof(path), "%s", DUMP_REBOOT_TIMES_FILE);
        }

        len = silfp_util_file_get_size(path);
        if (len != sizeof(uint32_t)) {
            len = 0;
        }

        buf = malloc(len + sizeof(uint32_t));
        if (buf != NULL) {
            memset(buf, 0, len + sizeof(uint32_t));
            if (len > 0) {
                ret = silfp_util_file_load(dir, DUMP_REBOOT_TIMES_FILE, buf, len);
                if (ret > 0) {
                    s_reboot_times = *((uint32_t*)buf);
                }
            }
            s_reboot_times++;
            silfp_util_file_save(dir, DUMP_REBOOT_TIMES_FILE, (void *)&s_reboot_times, sizeof(uint32_t));
            free(buf);
        }
    }

    if (s_reboot_times == DUMP_REBOOT_TIMES_NO_GET) {
        return 0;
    }

    return s_reboot_times;
}

/* get dump image file name & file path */
static int32_t _dump_get_save_path(char *path, uint32_t len, const char *dir, const char *subdir, uint32_t mode)
{
    if ((path == NULL) || (len == 0)) {
        return -1;
    }

    memset(path, 0, len);

    if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_PARENT_DIR_NULL)) {
        dir = NULL;
    }
    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_SUB_DIR_SEPARATE)) {
        subdir = NULL;
    }

    if ((dir != NULL) && (subdir != NULL)) { // dir/subdir
        snprintf(path, len, "%s/%s", dir, subdir);
    } else if (dir != NULL) { // just dir
        snprintf(path, len, "%s", dir);
    } else if (subdir != NULL) { // just subdir
        snprintf(path, len, "%s", subdir);
    }

    return 0;
}

static int32_t _dump_get_save_name(char *name, uint32_t len, const char *dir, const char *prefix, const char *ext, uint32_t mode)
{
    static int32_t s_bmp_index = 0;

    char datastr[64] = {0};
    const char *separate = NULL;
    uint32_t reboot_times = 0;
    uint64_t msecond = 0;

    if ((name == NULL) || (len == 0) || (ext == NULL)) {
        return -1;
    }

    memset(name, 0, len);

    memset(datastr, 0, sizeof(datastr));
    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_TIMESTAMP_NONE)) { // if need timestamp
        msecond = silfp_util_get_msec();
        if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_TIME_ONLY)) { // just time
            silfp_util_msec_to_date(msecond, datastr, sizeof(datastr), MODE_GET_SEC_TYPE_TIME, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_TIMESTAMP_DATE_ONLY)) { // just date
            silfp_util_msec_to_date(msecond, datastr, sizeof(datastr), MODE_GET_SEC_TYPE_DATE, MODE_GET_SEC_FORMAT_FILE_NAME);
        } else { // date & time
            silfp_util_msec_to_date(msecond, datastr, sizeof(datastr), MODE_GET_SEC_TYPE_ALL, MODE_GET_SEC_FORMAT_FILE_NAME);
        }
    }

    separate = ((datastr[0] == '\0') ? "" : "-");
    if (CHECK_FLAG_UNSET(mode, DUMP_NAME_MODE_REBOOT_TIMES_NONE)) {
        if (CHECK_FLAG_SET(mode, DUMP_NAME_MODE_PARENT_DIR_NULL)) {
            reboot_times = _dump_get_reboot_times(NULL);
        } else {
            reboot_times = _dump_get_reboot_times(dir);
        }
        if (prefix != NULL) {
            snprintf(name, len, "%04u-%04d-%s%s%s.%s", reboot_times, s_bmp_index++, prefix, separate, datastr, ext);
        } else {
            snprintf(name, len, "%04u-%04d%s%s.%s", reboot_times, s_bmp_index++, separate, datastr, ext);
        }
    } else {
        if (prefix != NULL) {
            snprintf(name, len, "%04d-%s%s%s.%s", s_bmp_index++, prefix, separate, datastr, ext);
        } else {
            snprintf(name, len, "%04d%s%s.%s", s_bmp_index++, separate, datastr, ext);
        }
    }
    return 0;
}

static int32_t _dump_get_name_offset(char *name, uint32_t size, int32_t maxlen)
{
    uint32_t len = 0;

    if ((name == NULL) || (size == 0) || (maxlen < 0)) {
        return 0;
    }

    len = strlen(name);
    if (len >= size) {
        len = size - 1;
        name[len] = '\0';
    }

    if (len <= (uint32_t)maxlen) {
        return 0;
    }

    return (len - maxlen);
}

#ifdef SIL_DUMP_IMAGE_DYNAMIC
static int32_t _dump_get_level(char *log, char *lvl)
{
    int32_t dump_data_support = -1;
    static char m_test_dump_prop[32] = {0};
    int32_t prop = 0;

    if (m_test_dump_prop[0] == 0) {
#ifndef SIL_DUMP_IMAGE_SWITCH_PROP
        snprintf(m_test_dump_prop, sizeof(m_test_dump_prop), "%s.%s.%s.%s", "log", "tag", log, lvl);
#else
        snprintf(m_test_dump_prop, sizeof(m_test_dump_prop), "%s", SIL_DUMP_IMAGE_SWITCH_PROP);
        UNUSED(log);
        UNUSED(lvl);
#endif
    }
    prop = silfp_util_get_str_value(m_test_dump_prop, (uint8_t)DUMP_ENABLED);
    if (prop == DUMP_ENABLED) {
        dump_data_support = 1;
    }

    return dump_data_support;
}

static int32_t _dump_check_level(void)
{
    return _dump_get_level("data", "dump");
}
#else
static int32_t _dump_check_level(void)
{
    return 1;
}
#endif /* SIL_DUMP_IMAGE_DYNAMIC */

static int32_t _dump_init(void)
{
    int32_t ret = 0;
    uint32_t size = 0;

    if (_dump_check_level() < 0) {
        return -1;
    }

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
    void *buf = NULL;
    uint8_t *data = NULL;
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

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_DUMP_FILE_EXT, buf, &len, &result);
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
                data = (uint8_t *)buf;
                memcpy(extbuf, data + result_path_len, *extsize);
            } else {
                ret = -1;
            }
        }
    }

    free(buf);
    return ret;
}

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
    char name[MAX_PATH_LEN] = {0};
    void *ext_buf = NULL;
    uint32_t ext_size = DUMP_EXT_SIZE_MAX;
    uint32_t dump_path_mode = 0;
    int32_t name_offset = 0;

    ret = _dump_init();
    if (ret < 0) {
        return;
    }

    if (m_test_dump_buffer == NULL) {
        LOG_MSG_ERROR("dump buffer NULL");
        return;
    }

    ext_buf = malloc(ext_size);
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
    case DUMP_IMG_AUTH_MUL_RAW: {
        mode = TEST_DUMP_DATA_TYPE_MUL_RAW;
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

            dump_path_mode = m_dump_path_mode;
            _dump_get_save_path(path, sizeof(path), _dump_get_path(), _dump_get_sub_dir(type), dump_path_mode);
            if ((TEST_DUMP_DATA_TYPE_CAL == mode) && (CHECK_FLAG_SET(dump_path_mode, DUMP_NAME_MODE_CAL_TIMESTAMP_NONE))) {
                dump_path_mode |= DUMP_NAME_MODE_TIMESTAMP_NONE;
            }

            if (_dump_get_save_name(name, sizeof(name), _dump_get_path(), _dump_get_prefix(type), _dump_get_ext(), dump_path_mode) >= 0) {
                ext_size = DUMP_EXT_SIZE_MAX;
                memset(ext_buf, 0, ext_size);
                if (_dump_get_ext_info(name, sizeof(name), ext_buf, &ext_size) < 0) {
                    ext_size = 0;
                }
                name_offset = _dump_get_name_offset(name, sizeof(name), m_dump_name_max_len);
                if (name_offset < 0) {
                    name_offset = 0;
                }
                silfp_bmp_save(path, name + name_offset, m_test_dump_buffer, imgsize, w, h, bitcount, ext_buf, ext_size, 0);
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

void silfp_dump_test_result(const char *name, const void *content, uint32_t len)
{
    char path[MAX_PATH_LEN] = {0};
    int32_t fd = -1;

    if (_dump_check_level() < 0) {
        return;
    }

    if (name == NULL || content == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return;
    }

    snprintf(path, sizeof(path), "%s/%s", _dump_get_path(), name);

    fd = silfp_util_open_file(path, 1);
    if (fd >= 0) {
        silfp_util_write_file(fd, content, len);
        silfp_util_close_file(fd);
    }
}

void silfp_dump_test_get_path(e_mode_dump_img_t type)
{
    char path[MAX_PATH_LEN] = {0};
    char name[MAX_PATH_LEN] = {0};
    uint32_t dump_path_mode = 0;
    int32_t name_offset = 0;

    dump_path_mode = m_dump_path_mode;
    _dump_get_save_path(path, sizeof(path), _dump_get_path(), _dump_get_sub_dir(type), dump_path_mode);
    if ((DUMP_IMG_CAL == type) && (CHECK_FLAG_SET(dump_path_mode, DUMP_NAME_MODE_CAL_TIMESTAMP_NONE))) {
        dump_path_mode |= DUMP_NAME_MODE_TIMESTAMP_NONE;
    }
    _dump_get_save_name(name, sizeof(name), _dump_get_path(), _dump_get_prefix(type), _dump_get_ext(), dump_path_mode);
    name_offset = _dump_get_name_offset(name, sizeof(name), m_dump_name_max_len);

    if (name_offset < 0) {
        LOG_MSG_ERROR("err: name_offset(%d)", name_offset);
    }

    LOG_MSG_ERROR("path: %s, name: %s", path, name + name_offset);
}

#else
void silfp_dump_set_name_mode(uint32_t mode, int32_t maxlen, const void *ext, uint32_t len)
{
    UNUSED(mode);
    UNUSED(maxlen);
    UNUSED(ext);
    UNUSED(len);
}

void silfp_dump_set_path(const void *path, uint32_t len)
{
    UNUSED(path);
    UNUSED(len);
}

void silfp_dump_data(e_mode_dump_img_t type)
{
    UNUSED(type);
}

void silfp_dump_deinit(void)
{
}

void silfp_dump_test_result(const char *name, const void *content, uint32_t len)
{
    UNUSED(name);
    UNUSED(content);
    UNUSED(len);
}
#endif /* SIL_DUMP_IMAGE */
