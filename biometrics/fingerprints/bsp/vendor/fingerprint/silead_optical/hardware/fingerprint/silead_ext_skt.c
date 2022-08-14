/******************************************************************************
 * @file   silead_fingerext.c
 * @brief  Contains fingerprint extension operate functions.
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

#define FILE_TAG "silead_ext_skt"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_cust.h"
#include "silead_ext_hub.h"
#include "silead_worker.h"
#include "silead_impl.h"
#include "silead_ext_skt.h"
#include "silead_ext.h"
#include "silead_version.h"
#include "silead_bmp.h"
#include "silead_dev.h"

#ifdef SIL_FP_EXT_SKT_SERVER_ENABLE

#define EXT_RET_TO_NONE 0
#define EXT_RET_TO_IDLE 1
#define EXT_RET_TO_WAIT 2

#ifndef CHECK_DATA_MAGIC_SUPPORT
#define CHECK_DATA_MAGIC_SUPPORT 1
#endif

#define DATA_MAGIC_STRING "slfp"
#define DATA_MAGIC_LENGTH 4

#define DATA_TYPE_PARAM 1
#define DATA_TYPE_IMAGE 2
enum _data_id {
    DATA_ID_QUALITY = 0,
    DATA_ID_AREA,
    DATA_ID_ORIG,
    DATA_ID_STEP,
    DATA_ID_ISTPL,
    DATA_ID_GREY_AVG,
    DATA_ID_GREY_MAX,
    DATA_ID_MAX,
};

typedef struct _ext_cmd_data {
    uint32_t cmd_id;
    int32_t req_len;
    int32_t offset;
    uint8_t *req;
} ext_skt_cmd_data_t;

typedef struct _ext_cmd_reponse {
    int32_t rsp_len;
    uint8_t *rsp;
} ext_skt_cmd_reponse_t;

typedef struct {
    uint32_t cmd;
    int32_t (*dispatch) (uint32_t cmd);
} ext_skt_cmd_info_t;

static uint8_t m_test_image_bitcount = 0;
static uint8_t m_test_image_bitcount_orig = 0;
static uint32_t m_test_image_w = 0;
static uint32_t m_test_image_h = 0;
static uint32_t m_test_image_w_ori = 0;
static uint32_t m_test_image_h_ori = 0;
static uint32_t m_test_image_buffer_size = 0;
static char *m_test_image_buffer = NULL;

static pthread_mutex_t m_ext_skt_data_lock;
static ext_skt_cmd_data_t m_ext_skt_cmd_data;
static ext_skt_cmd_reponse_t m_ext_skt_cmd_rsp;

static int32_t _ext_skt_test_cmd_finger_down(uint32_t addition);
static int32_t _ext_skt_test_cmd_finger_up(uint32_t addition);
static int32_t _ext_skt_test_cmd_icon_ready(uint32_t addition);
static const ext_skt_cmd_info_t ext_skt_sync_cmd[] = {
    {EXT_CMD_SEND_FINGER_DOWN,  _ext_skt_test_cmd_finger_down},
    {EXT_CMD_SEND_FINGER_UP,    _ext_skt_test_cmd_finger_up},
    {EXT_CMD_ICON_READY,        _ext_skt_test_cmd_icon_ready},
};


static int32_t _ext_skt_test_get_image(uint32_t cmd_id);
static int32_t _ext_skt_test_send_image(uint32_t cmd_id);
static int32_t _ext_skt_get_version(uint32_t cmd_id);
static int32_t _ext_skt_get_chipid(uint32_t cmd_id);
static int32_t _ext_skt_reset(uint32_t cmd_id);
static int32_t _ext_skt_test_deadpixel(uint32_t cmd_id);
static int32_t _ext_skt_test_speed(uint32_t cmd_id);
static int32_t _ext_skt_test_image_finish(uint32_t cmd_id);
static int32_t _ext_skt_test_calibrate(uint32_t cmd_id);
static int32_t _ext_skt_test_calibrate_step(uint32_t cmd_id);
static int32_t _ext_skt_test_self(uint32_t cmd_id);
static int32_t _ext_skt_test_flash(uint32_t cmd_id);
static int32_t _ext_skt_test_otp(uint32_t cmd_id);
static const ext_skt_cmd_info_t ext_skt_cmd[] = {
    {EXT_CMD_GET_IMAGE,         _ext_skt_test_get_image},
    {EXT_CMD_SEND_IMAGE,        _ext_skt_test_send_image},
    {EXT_CMD_SEND_IMAGE_CLEAR,  _ext_skt_test_image_finish},
    {EXT_CMD_GET_VERSION,       _ext_skt_get_version},
    {EXT_CMD_SPI_TEST,          _ext_skt_get_chipid},
    {EXT_CMD_TEST_RESET_PIN,    _ext_skt_reset},
    {EXT_CMD_TEST_DEAD_PIXEL,   _ext_skt_test_deadpixel},
    {EXT_CMD_TEST_SPEED,        _ext_skt_test_speed},
    {EXT_CMD_TEST_FINISH,       _ext_skt_test_image_finish},
    {EXT_CMD_CALIBRATE,         _ext_skt_test_calibrate},
    {EXT_CMD_CALIBRATE_STEP,    _ext_skt_test_calibrate_step},
    {EXT_CMD_SELF_TEST,         _ext_skt_test_self},
    {EXT_CMD_TEST_FLASH,        _ext_skt_test_flash},
    {EXT_CMD_TEST_OTP,          _ext_skt_test_otp},
};

//=====================================================================
static uint32_t _ext_get_bmp_report_size(uint32_t w, uint32_t h, uint8_t bitcount)
{
    return silfp_bmp_get_size(w, h, bitcount) + 32;
}

static int32_t _ext_get_image_size(void)
{
    int32_t ret = 0;
    uint32_t max_size = 0;

    ret = silfp_ext_test_get_image_info(&m_test_image_w, &m_test_image_h, &max_size, &m_test_image_w_ori, &m_test_image_h_ori, &m_test_image_bitcount, &m_test_image_bitcount_orig);
    LOG_MSG_DEBUG("ret %d size(%d) (%d:%d:%d) (%d:%d:%d)", ret, max_size, m_test_image_w, m_test_image_h, m_test_image_bitcount, m_test_image_w_ori, m_test_image_h_ori, m_test_image_bitcount_orig);

    if (max_size == 0 || m_test_image_w == 0 || m_test_image_h == 0) {
        LOG_MSG_DEBUG("get the config from ta is invalid");
        ret = -SL_ERROR_CONFIG_INVALID;
    }

    if (m_test_image_w_ori == 0) {
        m_test_image_w_ori = m_test_image_w;
    }
    if (m_test_image_h_ori == 0) {
        m_test_image_h_ori = m_test_image_h;
    }

    if (ret >= 0) {
        if (m_test_image_buffer == NULL) {
            m_test_image_buffer_size = max_size;
            m_test_image_buffer = (char *)malloc(m_test_image_buffer_size);
            if (m_test_image_buffer == NULL) {
                m_test_image_buffer_size = 0;
                ret = -SL_ERROR_OUT_OF_MEMORY;
            }
        }
    }

    if (ret >= 0) {
        ret = _ext_get_bmp_report_size(m_test_image_w_ori, m_test_image_h_ori, m_test_image_bitcount_orig);
    }

    return ret;
}

static int32_t _ext_get_img_raw_buffer()
{
    int32_t ret = 0;

    if (m_test_image_buffer != NULL) {
        return 0;
    }

    ret = _ext_get_image_size();

    return ret;
}

static void _ext_buffer_release(void)
{
    if (m_test_image_buffer != NULL) {
        free(m_test_image_buffer);
        m_test_image_buffer = NULL;
    }
    m_test_image_bitcount = 0;
    m_test_image_bitcount_orig = 0;
    m_test_image_w = 0;
    m_test_image_h = 0;
    m_test_image_w_ori = 0;
    m_test_image_h_ori = 0;
    m_test_image_buffer_size = 0;
}

int32_t silfp_ext_skt_init(void)
{
    memset(&m_ext_skt_cmd_data, 0, sizeof(m_ext_skt_cmd_data));
    memset(&m_ext_skt_cmd_rsp, 0, sizeof(m_ext_skt_cmd_rsp));
    pthread_mutex_init(&m_ext_skt_data_lock, NULL);

    silfp_ext_hub_init();

    return 0;
}

int32_t silfp_ext_skt_deinit(void)
{
    silfp_ext_hub_deinit();
    _ext_buffer_release();

    if (NULL != m_ext_skt_cmd_data.req) {
        free(m_ext_skt_cmd_data.req);
        m_ext_skt_cmd_data.req = NULL;
    }
    if (NULL != m_ext_skt_cmd_rsp.rsp) {
        free(m_ext_skt_cmd_rsp.rsp);
        m_ext_skt_cmd_rsp.rsp = NULL;
    }
    pthread_mutex_destroy(&m_ext_skt_data_lock);

    return 0;
}

//====================================================================
static inline int32_t _ext_skt_send_response(uint32_t cmdid, const void *data, size_t data_size)
{
    return silfp_ext_hub_send_response_raw(cmdid, data, data_size);
}

static int32_t _ext_set_version(void *buffer, uint32_t offset, uint32_t size, void *value, uint32_t vsize)
{
    uint32_t len = vsize;
    unsigned char *p = (unsigned char *)buffer;

    if (p == NULL) {
        return offset;
    }

    if (len + offset + 1 > size) {
        len = size - offset - 1;
    }
    p[offset++] = len;
    if (len > 0 && value != NULL) {
        memcpy(p + offset, value, len);
    }
    offset += len;
    if (offset >= size) {
        offset = size;
    }

    return offset;
}

static int32_t _ext_set_versions(void *buffer, uint32_t offset, uint32_t size, void *value, uint32_t len, int32_t first)
{
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size > 4) {
        if (first != 0) {
            p[offset++] = 0;
            p[offset++] = 0;
            p[offset++] = 0;
            p[offset++] = 0;
        }
        offset = _ext_set_version(buffer, offset, size, value, len);
    }
    return offset;
}

static int32_t _ext_set_chipid(void *buffer, uint32_t size, uint32_t chipid, uint32_t subId)
{
    uint32_t offset = 0;
    const uint32_t chip_id_len = 21;
    char *p = (char *)buffer;

    if (p != NULL && size >= chip_id_len + 5) {
        // err code (4 Byte)
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;

        // chip id length (1 Byte)
        p[offset++] = chip_id_len;

        // chip id
        snprintf(p + offset, size - offset, "0x%08X,0x%08X", chipid, subId);
        offset += chip_id_len;
    }

    return offset;
}

static int32_t _ext_set_data_param(void *buffer, uint32_t size, uint32_t offset, void *data, uint32_t len, uint32_t type)
{
    unsigned char *p = (unsigned char *)buffer;
    unsigned char *pdata = (unsigned char *)data;

    if (p != NULL && size >= len + 8 && pdata != NULL && len >= 1) {
        p[offset++] = (type & 0xFF);
        p[offset++] = (len >> 24) & 0xFF;
        p[offset++] = (len >> 16) & 0xFF;
        p[offset++] = (len >> 8) & 0xFF;
        p[offset++] = len & 0xFF;

        memcpy(p + offset, pdata, len);
        offset += len;
    }
    return offset;
}

static int32_t _ext_set_img_data(void *buffer, uint32_t size, uint32_t offset, const void *data, uint32_t len, uint32_t w, uint32_t h, uint8_t bitcount, uint32_t type)
{
    int32_t ret = 0;
    uint32_t pos = offset;
    unsigned char *p = (unsigned char *)buffer;

    if (buffer == NULL || offset + 5 > size || data == NULL) {
        return offset;
    }

    p[offset++] = (type & 0xFF);
    offset += 4;

    ret = silfp_bmp_get_img(p + offset, size - offset, data, len, w, h, bitcount, NULL, 0, 0);
    if (ret > 0) {
        pos++;
        p[pos++] = (ret >> 24) & 0xFF;
        p[pos++] = (ret >> 16) & 0xFF;
        p[pos++] = (ret >> 8) & 0xFF;
        p[pos++] = ret & 0xFF;

        offset += ret;
    } else {
        offset = pos;
    }

    return offset;
}

static int32_t _ext_set_data_result_with_index(void *buffer, uint32_t size, void *data, uint32_t len, uint32_t index)
{
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;
    unsigned char *pdata = (unsigned char *)data;

    if (p != NULL && size >= len + 8 && pdata != NULL && len >= 1) {
        // err code (4 Byte)
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;

        p[offset++] = (index >> 24) & 0xFF;
        p[offset++] = (index >> 16) & 0xFF;
        p[offset++] = (index >> 8) & 0xFF;
        p[offset++] = index & 0xFF;

        memcpy(p + offset, pdata, len);
        offset += len;
    }
    return offset;
}

static int32_t _ext_set_data_result(void *buffer, uint32_t size, const void *data, uint32_t len)
{
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;
    unsigned char *pdata = (unsigned char *)data;

    if (p != NULL && size >= len + 4 && pdata != NULL && len >= 1) {
        // err code (4 Byte)
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;
        p[offset++] = 0;

        memcpy(p + offset, pdata, len);
        offset += len;
    }
    return offset;
}

static int32_t _ext_set_err_with_result_data(void *buffer, uint32_t size, int32_t err, int32_t result, uint32_t *data, uint32_t count)
{
    uint32_t i = 0;
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size >= 5 + count*sizeof(uint32_t)) {
        // err code (4 Byte)
        p[offset++] = ((err >> 24) & 0xFF);
        p[offset++] = ((err >> 16) & 0xFF);
        p[offset++] = ((err >> 8) & 0xFF);
        p[offset++] = (err & 0xFF);

        p[offset++] = result;
        for (i = 0; i < count; i++) {
            p[offset++] = ((data[i] >> 24) & 0xFF);
            p[offset++] = ((data[i] >> 16) & 0xFF);
            p[offset++] = ((data[i] >> 8) & 0xFF);
            p[offset++] = (data[i] & 0xFF);
        }
    }
    return offset;
}

static int32_t _ext_set_err_with_uint_2_bytes(void *buffer, uint32_t size, int32_t err, uint32_t *data, uint32_t len)
{
    uint32_t i = 0;
    int32_t offset = 0;
    unsigned char *p = (unsigned char *)buffer;

    if (p != NULL && size >= (4 + len)) {
        if (err < 0) {
            err = -err;
        }

        // err code (4 Byte)
        p[offset++] = ((err >> 24) & 0xFF);
        p[offset++] = ((err >> 16) & 0xFF);
        p[offset++] = ((err >> 8) & 0xFF);
        p[offset++] = (err & 0xFF);

        for (i = 0; i < len; i++) {
            p[offset++] = (data[i] & 0xFF);
        }
    }
    return offset;
}

static void _ext_skt_send_error_notice(int32_t cmd, int32_t err)
{
    char buffer[32] = {0};
    int32_t len = _ext_set_err_with_uint_2_bytes(buffer, sizeof(buffer), err, NULL, 0);
    _ext_skt_send_response(cmd, (int8_t *)buffer, len);
}

static void _ext_send_icon_change_notify(int32_t cmd, int32_t step)
{
    char buffer[32] = {0};
    int32_t offset = 0;
    int32_t err = -100;
    uint8_t data[DATA_ID_MAX] = {0};

    buffer[offset++] = (err >> 24) & 0xFF;
    buffer[offset++] = (err >> 16) & 0xFF;
    buffer[offset++] = (err >> 8) & 0xFF;
    buffer[offset++] = err & 0xFF;

    memset(data, 0, sizeof(data));
    data[DATA_ID_STEP] = (step & 0xFF);
    offset = _ext_set_data_param(buffer, sizeof(buffer), offset, data, DATA_ID_MAX, DATA_TYPE_PARAM);

    _ext_skt_send_response(cmd, (int8_t *)buffer, offset);
}

//====================================================================
static int32_t _ext_data_verify(const void *buffer, uint32_t len, uint32_t *offset)
{
    int32_t ret = 0;
    uint32_t i = 0;
    const unsigned char *p = (const unsigned char *)buffer;
    uint32_t start = 0;

    if (NULL != offset) {
        start = *offset;
    }

    if (p == NULL || len < DATA_MAGIC_LENGTH + start) {
        ret = -SL_ERROR_BAD_PARAMS;
    } else {
        for (i = 0; i < DATA_MAGIC_LENGTH; i++) {
            if ((p[start+i] & 0xFF) != DATA_MAGIC_STRING[i]) {
                break;
            }
        }
        if (i >= DATA_MAGIC_LENGTH) { // match
            if (offset) {
                *offset = start + DATA_MAGIC_LENGTH;
            }
            ret = DATA_MAGIC_LENGTH;
        } else {
            ret = -SL_ERROR_BAD_PARAMS;
        }
    }

    return ret;
}

int32_t silfp_ext_skt_test_cmd(const uint8_t *param, int32_t len)
{
    int32_t ret = 0;
    uint32_t cmd_id = 0;
    uint32_t offset = 0;
    uint32_t i = 0;

    if (len < 4) {
        ret = -SL_ERROR_BAD_PARAMS;
        _ext_skt_send_error_notice(cmd_id, ret);
        return ret;
    }

    cmd_id = ((param[offset++] << 24) & 0xFF000000);
    cmd_id |= ((param[offset++] << 16) & 0x00FF0000);
    cmd_id |= ((param[offset++] << 8) & 0x0000FF00);
    cmd_id |= (param[offset++] & 0x000000FF);

#if CHECK_DATA_MAGIC_SUPPORT
    ret = _ext_data_verify(param, len, &offset);
#endif
    if (ret < 0) { // verify data failed
        _ext_skt_send_error_notice(cmd_id, ret);
        LOG_MSG_ERROR("verify data failed (%d)", ret);

        if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
            LOG_MSG_DEBUG("verify data failed, goto idle status");
            silfp_worker_set_state_no_signal(STATE_IDLE);
        }
        return ret;
    }

    len -= offset;

    for (i = 0; i < NUM_ELEMS(ext_skt_sync_cmd); i++) {
        if (ext_skt_sync_cmd[i].cmd == cmd_id) {
            if (len > 0) {
                LOG_MSG_INFO("dispatch sync cmd (0x%x: 0x%x)", cmd_id, param[offset]);
                return ext_skt_sync_cmd[i].dispatch(param[offset] & 0xFF);
            } else {
                LOG_MSG_INFO("dispatch sync cmd (0x%x)", cmd_id);
                return ext_skt_sync_cmd[i].dispatch(0);
            }
        }
    }

    pthread_mutex_lock(&m_ext_skt_data_lock);
    do {
        if (m_ext_skt_cmd_data.req_len < len) { // not have enough buffer, free
            if (m_ext_skt_cmd_data.req) {
                free(m_ext_skt_cmd_data.req);
                m_ext_skt_cmd_data.req = NULL;
            }
        }
        m_ext_skt_cmd_data.cmd_id = cmd_id;
        m_ext_skt_cmd_data.req_len = len;
        m_ext_skt_cmd_data.offset = 0;

        if (len > 0) {
            if (NULL == m_ext_skt_cmd_data.req) { // malloc memory
                m_ext_skt_cmd_data.req = (uint8_t *)malloc(len);
            }
            if (NULL == m_ext_skt_cmd_data.req) { // malloc fail
                m_ext_skt_cmd_data.req_len = 0;
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }
            memcpy(m_ext_skt_cmd_data.req, param + offset, len);
        }
    } while (0);
    pthread_mutex_unlock(&m_ext_skt_data_lock);

    if (ret < 0) {
        LOG_MSG_ERROR("error cmd_id:0x%x, err:%d", cmd_id, ret);
        _ext_skt_send_error_notice(cmd_id, ret);
    } else {
        silfp_worker_set_state(STATE_TEST);
    }

    return ret;
}

int32_t silfp_ext_skt_commond(void)
{
    int32_t ret = 0;
    uint32_t i = 0;
    uint32_t cmd = 0;

    pthread_mutex_lock(&m_ext_skt_data_lock);
    cmd = m_ext_skt_cmd_data.cmd_id;
    pthread_mutex_unlock(&m_ext_skt_data_lock);

    LOG_MSG_INFO("test------------- 0x%x", cmd);

    for (i = 0; i < NUM_ELEMS(ext_skt_cmd); i++) {
        if (ext_skt_cmd[i].cmd == cmd) {
            LOG_MSG_INFO("dispatch cmd (0x%x)", cmd);
            ret = ext_skt_cmd[i].dispatch(cmd);
            break;
        }
    }

    if (i >= NUM_ELEMS(ext_skt_cmd)) {
        _ext_skt_send_error_notice(cmd, -SL_ERROR_REQ_CMD_UNSUPPORT);
        ret = EXT_RET_TO_IDLE;
    }

    if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
        if (ret == EXT_RET_TO_WAIT) {
            silfp_worker_set_state_no_signal(STATE_WAIT);
        } else if (ret == EXT_RET_TO_IDLE) {
            silfp_worker_set_state_no_signal(STATE_IDLE);
        }
    }

    return 0;
}

//====================================================================
static int32_t _ext_skt_test_cmd_finger_down(uint32_t addition)
{
    silfp_impl_sync_finger_status_optic(1);
    _ext_skt_test_cmd_icon_ready(addition);
    return 0;
}

static int32_t _ext_skt_test_cmd_finger_up(uint32_t addition)
{
    _ext_skt_test_cmd_icon_ready(addition);
    silfp_impl_sync_finger_status_optic(0);
    return 0;
}

static int32_t _ext_skt_test_cmd_icon_ready(uint32_t addition)
{
    if (IS_UI_FLAG(addition)) {
        if (IS_UI_READY_FLAG(addition)) {
            silfp_cust_notify_ui_ready(GET_UI_READY_FLAG(addition));
        }
        if (IS_UI_DUMP_FLAG(addition)) {
            silfp_impl_set_capture_dump_flag(GET_UI_READY_FLAG(addition));
        }
    }
    return 0;
}

// get image
static int32_t _ext_image_capture_inter(uint32_t mode, int32_t step, void *buffer, uint32_t size)
{
    int32_t ret = 0;
    int32_t offset = 0;
    uint32_t len = 0;
    uint8_t data[DATA_ID_MAX] = {0};
    data[DATA_ID_ORIG] = !!(EXT_IMG_FEATURE_ORIG_MASK & mode);
    data[DATA_ID_STEP] = (step & 0xFF);

    ret = _ext_get_img_raw_buffer();
    if (ret < 0) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    len = m_test_image_buffer_size;
    ret = silfp_ext_test_image_test(mode, m_test_image_buffer, &len, &data[DATA_ID_QUALITY], &data[DATA_ID_AREA], &data[DATA_ID_ISTPL], &data[DATA_ID_GREY_AVG], &data[DATA_ID_GREY_MAX]);
    if (ret < 0) {
        if (data[DATA_ID_ORIG]) {
            offset = _ext_set_err_with_uint_2_bytes(buffer, size, 0, NULL, 0);
        } else {
            offset = _ext_set_err_with_uint_2_bytes(buffer, size, ret, NULL, 0);
        }
    } else {
        offset = _ext_set_err_with_uint_2_bytes(buffer, size, 0, NULL, 0);
    }

    offset = _ext_set_data_param(buffer, size, offset, data, DATA_ID_MAX, DATA_TYPE_PARAM);

    if (mode & EXT_IMG_FEATURE_DATA_MASK) {
        if (data[DATA_ID_ORIG]) {
            LOG_MSG_DEBUG("original w:%u, h:%u, size:%u, bitcount:%d", m_test_image_w_ori, m_test_image_h_ori, len, m_test_image_bitcount_orig);
            offset = _ext_set_img_data(buffer, size, offset, (void *)m_test_image_buffer, len, m_test_image_w_ori, m_test_image_h_ori, m_test_image_bitcount_orig, DATA_TYPE_IMAGE);
        } else {
            LOG_MSG_DEBUG("w:%u, h:%u, size:%u, bitcount:%d", m_test_image_w, m_test_image_h, len, m_test_image_bitcount);
            offset = _ext_set_img_data(buffer, size, offset, (void *)m_test_image_buffer, len, m_test_image_w, m_test_image_h, m_test_image_bitcount, DATA_TYPE_IMAGE);
        }
    }

    return offset;
}

static int32_t _ext_skt_test_get_image(uint32_t cmd_id)
{
    int32_t ret = 0;
    int32_t i = 0;
    int32_t number_of_onepress = 1;
    int32_t type = 0;
    int32_t delay = 0;
    int32_t wait_finger = 1;

    uint32_t mode = /*EXT_IMG_FEATURE_GEN_TPL_MASK |*/ EXT_IMG_FEATURE_DATA_MASK | EXT_IMG_FEATURE_QAULITY_MASK;
    uint32_t mode_ori = EXT_IMG_FEATURE_ORIG_MASK | EXT_IMG_FEATURE_DATA_MASK | EXT_IMG_FEATURE_QAULITY_MASK;

    pthread_mutex_lock(&m_ext_skt_data_lock);
    if (m_ext_skt_cmd_data.req_len >= m_ext_skt_cmd_data.offset + 3) {
        type = (m_ext_skt_cmd_data.req[m_ext_skt_cmd_data.offset] & 0xFF);
        number_of_onepress = (m_ext_skt_cmd_data.req[m_ext_skt_cmd_data.offset + 1] & 0xFF);
        delay = (m_ext_skt_cmd_data.req[m_ext_skt_cmd_data.offset + 2] & 0xFF);
    }
    pthread_mutex_unlock(&m_ext_skt_data_lock);

    if (number_of_onepress > 0) {
        mode &= (~EXT_IMG_FEATURE_GEN_TPL_MASK);
    }
    if (number_of_onepress == 0) {
        number_of_onepress = 1;
    }

    if (number_of_onepress == 0xFF) {
        wait_finger = 0;
        number_of_onepress = 1;
    }

    LOG_MSG_DEBUG("mode:0x%x, number_of_onepress:%d, type:%d, wait_finger:%d", mode, number_of_onepress, type, wait_finger);

    ret = _ext_get_image_size();
    if (m_ext_skt_cmd_rsp.rsp == NULL) {
        if (ret > 0) {
            m_ext_skt_cmd_rsp.rsp_len = ret;
            m_ext_skt_cmd_rsp.rsp = (uint8_t *)malloc(m_ext_skt_cmd_rsp.rsp_len);
        }
        if (m_ext_skt_cmd_rsp.rsp == NULL) {
            m_ext_skt_cmd_rsp.rsp_len = 0;
            ret = -SL_ERROR_OUT_OF_MEMORY;
        }
    }
    if (ret < 0) {
        _ext_skt_send_error_notice(cmd_id, ret);
        return EXT_RET_TO_IDLE;
    }

    if (!wait_finger) {
        silfp_ext_test_reset();
    }

    silfp_cust_set_hbm_mode(1);
    do {
        if (wait_finger) {
            ret = silfp_impl_get_finger_down_with_cancel();
        }

        if (ret < 0) {
            continue;
        }

        for (i = 0; i < number_of_onepress; i++) {
            if (delay > 0) {
                if (wait_finger) {
                    _ext_send_icon_change_notify(cmd_id, i);
                }
                usleep(10000*delay);
            }

            silfp_ext_test_image_test_init(IMG_CAPTURE_CAPTURE_TEST, number_of_onepress, i);

            ret = silfp_impl_capture_image_pre();
            if (ret < 0) {
                break;
            }

            if (number_of_onepress > 1) {
                ret = silfp_impl_capture_image_raw(IMG_CAPTURE_CAPTURE_TEST, i);
            } else {
                ret = silfp_impl_capture_image_raw(IMG_CAPTURE_OTHER, i);
            }

            if (ret < 0) {
                continue;
            }

            if (type == 1 || type == 2) {
                memset(m_ext_skt_cmd_rsp.rsp, 0, m_ext_skt_cmd_rsp.rsp_len);
                ret = _ext_image_capture_inter(mode_ori, i, (char *)m_ext_skt_cmd_rsp.rsp, m_ext_skt_cmd_rsp.rsp_len);
                if (ret >= 0) {
                    _ext_skt_send_response(cmd_id, (int8_t *)m_ext_skt_cmd_rsp.rsp, ret);
                } else {
                    _ext_skt_send_error_notice(cmd_id, ret);
                }
            }

            if (type == 0 || type == 2) {
                if (ret >= 0) {
                    ret = silfp_impl_capture_image_after(IMG_CAPTURE_CAPTURE_TEST, 0);
                    if (ret >= 0) {
                        memset(m_ext_skt_cmd_rsp.rsp, 0, m_ext_skt_cmd_rsp.rsp_len);
                        ret = _ext_image_capture_inter(mode, i, (char *)m_ext_skt_cmd_rsp.rsp, m_ext_skt_cmd_rsp.rsp_len);
                        if (ret >= 0) {
                            _ext_skt_send_response(cmd_id, (int8_t *)m_ext_skt_cmd_rsp.rsp, ret);
                        } else {
                            _ext_skt_send_error_notice(cmd_id, ret);
                        }
                    }
                }
            }
        }

        if (wait_finger && ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
            silfp_impl_wait_finger_up_with_cancel();
        }
    } while (!silfp_worker_is_canceled());
    silfp_cust_restore_hbm();

    if (ret == -SL_ERROR_CANCELED || silfp_worker_is_canceled()) {
        silfp_ext_test_image_finish();
        _ext_buffer_release();
    }

    return EXT_RET_TO_NONE;
}

static int32_t _ext_image_capture_dump_inter(uint32_t mode, int32_t step, void *buffer, uint32_t size, int32_t enroll, int32_t result)
{
    int32_t ret = 0;
    int32_t offset = 0;
    uint32_t len = 0;
    uint8_t data[DATA_ID_MAX] = {0};
    data[DATA_ID_ORIG] = !!(EXT_IMG_FEATURE_ORIG_MASK & mode);
    data[DATA_ID_STEP] = (step & 0xFF);
    data[DATA_ID_ISTPL] = (enroll & 0xFF);

    ret = _ext_get_img_raw_buffer();
    if (ret < 0) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    len = m_test_image_buffer_size;
    ret = silfp_ext_test_image_test(mode, m_test_image_buffer, &len, &data[DATA_ID_QUALITY], &data[DATA_ID_AREA], NULL, &data[DATA_ID_GREY_AVG], &data[DATA_ID_GREY_MAX]);
    offset = _ext_set_err_with_uint_2_bytes(buffer, size, result, NULL, 0);

    offset = _ext_set_data_param(buffer, size, offset, data, DATA_ID_MAX, DATA_TYPE_PARAM);

    if (mode & EXT_IMG_FEATURE_DATA_MASK) {
        if (data[DATA_ID_ORIG]) {
            LOG_MSG_DEBUG("original w:%u, h:%u, size:%u, bitcount:%d", m_test_image_w_ori, m_test_image_h_ori, len, m_test_image_bitcount_orig);
            offset = _ext_set_img_data(buffer, size, offset, (void *)m_test_image_buffer, len, m_test_image_w_ori, m_test_image_h_ori, m_test_image_bitcount_orig, DATA_TYPE_IMAGE);
        } else {
            LOG_MSG_DEBUG("w:%u, h:%u, size:%u, bitcount:%d", m_test_image_w, m_test_image_h, len, m_test_image_bitcount);
            offset = _ext_set_img_data(buffer, size, offset, (void *)m_test_image_buffer, len, m_test_image_w, m_test_image_h, m_test_image_bitcount, DATA_TYPE_IMAGE);
        }
    }

    return offset;
}

int32_t silfp_ext_skt_capture_dump(int32_t orig, int32_t enroll, uint32_t step, int32_t result)
{
    int32_t ret = 0;
    uint32_t mode = EXT_IMG_FEATURE_DATA_MASK | EXT_IMG_FEATURE_QAULITY_MASK;

    if (orig) {
        mode |= EXT_IMG_FEATURE_ORIG_MASK;
    }

    ret = _ext_get_image_size();
    if (m_ext_skt_cmd_rsp.rsp == NULL) {
        if (ret > 0) {
            m_ext_skt_cmd_rsp.rsp_len = ret;
            m_ext_skt_cmd_rsp.rsp = (uint8_t *)malloc(m_ext_skt_cmd_rsp.rsp_len);
        }
        if (m_ext_skt_cmd_rsp.rsp == NULL) {
            m_ext_skt_cmd_rsp.rsp_len = 0;
            ret = -SL_ERROR_OUT_OF_MEMORY;
        }
    }
    if (ret < 0) {
        return ret;
    }

    memset(m_ext_skt_cmd_rsp.rsp, 0, m_ext_skt_cmd_rsp.rsp_len);
    ret = _ext_image_capture_dump_inter(mode, step, (char *)m_ext_skt_cmd_rsp.rsp, m_ext_skt_cmd_rsp.rsp_len, enroll, result);
    if (ret >= 0) {
        _ext_skt_send_response(EXT_CMD_GET_IMAGE, (int8_t *)m_ext_skt_cmd_rsp.rsp, ret);
    }
    return ret;
}

// send image
static int32_t _ext_send_group_image_inter(const void *buffer, uint32_t offset, const uint32_t len, void *rsp, const uint32_t maxlen)
{
    int32_t ret = 0;
    uint32_t size = 0;
    uint32_t orig = 0;
    uint32_t frr = 0;
    uint32_t imgtype = 0;

    const unsigned char *p = (const unsigned char *)buffer;

    ret = _ext_get_img_raw_buffer();
    if (ret < 0) {
        return ret;
    }

    if (p != NULL && len >= offset + 3) {
        orig = (p[offset++] & 0x000000FF);
        frr = (p[offset++] & 0x000000FF);
        imgtype = (p[offset++] & 0x000000FF);
        if (orig) {
            ret = silfp_bmp_get_data((void *)m_test_image_buffer, m_test_image_buffer_size, m_test_image_w_ori, m_test_image_h_ori, p + offset, len - offset, m_test_image_bitcount_orig);
        } else {
            ret = silfp_bmp_get_data((void *)m_test_image_buffer, m_test_image_buffer_size, m_test_image_w, m_test_image_h, p + offset, len - offset, m_test_image_bitcount);
        }

        if (ret > 0 && ret <= (int32_t)m_test_image_buffer_size) {
            size = ret;
            ret = silfp_ext_test_send_group_image(orig, frr, imgtype, m_test_image_buffer, &size);
        } else {
            LOG_MSG_DEBUG("parse img data failed (%d)", ret);
            ret = -SL_ERROR_BAD_PARAMS;
        }
    } else {
        LOG_MSG_DEBUG("cmd buffer is invalid");
        ret = -SL_ERROR_BAD_PARAMS;
    }

    if (ret >= 0 && size > 0 && size <= m_test_image_buffer_size) {
        ret = _ext_set_data_result_with_index(rsp, maxlen, m_test_image_buffer, size, 0);
    }

    return ret;
}

static int32_t _ext_skt_test_send_image(uint32_t cmd_id)
{
    int32_t ret = 0;
    char buffer[256] = {0};

    pthread_mutex_lock(&m_ext_skt_data_lock);
    m_ext_skt_cmd_data.offset += 4; // skip index
    ret = _ext_send_group_image_inter((const char *)m_ext_skt_cmd_data.req, m_ext_skt_cmd_data.offset, m_ext_skt_cmd_data.req_len, (char *)buffer, sizeof(buffer));
    pthread_mutex_unlock(&m_ext_skt_data_lock);

    if (ret >= 0) {
        _ext_skt_send_response(cmd_id, (int8_t *)buffer, ret);
    } else {
        _ext_skt_send_error_notice(cmd_id, ret);
    }

    return EXT_RET_TO_WAIT;
}

// get version
static int32_t _ext_get_versions_inter(void *buffer, uint32_t size)
{
    int32_t ret = 0;
    char ver[32] = {0};
    uint32_t offset = 0;
    uint32_t algoVer, taVer;

    do {
        // add hal version
        offset = _ext_set_versions(buffer, offset, size, (char*)FP_HAL_VERSION, strlen(FP_HAL_VERSION), 1);
        if (offset >= size) {
            break;
        }

        // add dev version
        ret = silfp_dev_get_ver(ver, sizeof(ver));
        if (ret >= 0) {
            offset = _ext_set_versions(buffer, offset, size, ver, strlen(ver), 0);
        } else {
            offset = _ext_set_versions(buffer, offset, size, NULL, 0, 0);
        }
        if (offset >= size) {
            break;
        }

        //add algo & ta version
        ret = silfp_ext_test_get_ta_ver(&algoVer, &taVer);
        if (ret >= 0) {
            memset(ver, 0, sizeof(ver));
            snprintf(ver, sizeof(ver), "v%d", algoVer);
            offset = _ext_set_versions(buffer, offset, size, ver, strlen(ver), 0);

            memset(ver, 0, sizeof(ver));
            snprintf(ver, sizeof(ver), "v%d", taVer);
            offset = _ext_set_versions(buffer, offset, size, ver, strlen(ver), 0);
        }
    } while (0);

    return offset;
}

static int32_t _ext_skt_get_version(uint32_t cmd_id)
{
    int32_t ret = 0;
    char buffer[256] = {0};

    ret = _ext_get_versions_inter(buffer, sizeof(buffer));
    if (ret > 0) {
        _ext_skt_send_response(cmd_id, (int8_t *)buffer, ret);
    } else {
        _ext_skt_send_error_notice(cmd_id, ret);
    }

    return EXT_RET_TO_IDLE;
}

// spi test
static int32_t _ext_skt_get_chipid(uint32_t cmd_id)
{
    int32_t ret = 0;
    char buffer[256] = {0};
    uint32_t chipId = 0;
    uint32_t subId = 0;

    ret = silfp_ext_test_spi(&chipId, &subId);
    if (ret >= 0) {
        ret = _ext_set_chipid(buffer, sizeof(buffer), chipId, subId);
    }

    if (ret > 0) {
        _ext_skt_send_response(cmd_id, (int8_t *)buffer, ret);
    } else {
        _ext_skt_send_error_notice(cmd_id, ret);
    }

    return EXT_RET_TO_IDLE;
}

// reset test
static int32_t _ext_skt_reset(uint32_t cmd_id)
{
    int32_t ret = 0;

    ret = silfp_impl_download_normal();
    if (ret >= 0) {
        ret = 0;
    }

    _ext_skt_send_error_notice(cmd_id, ret);

    return EXT_RET_TO_IDLE;
}

// deadpixel test
static int32_t _ext_skt_test_deadpixel(uint32_t cmd_id)
{
    int32_t ret = 0;
    char buffer[256] = {0};
    uint32_t result = 0;
    uint32_t data[2] = {0};

    int32_t optic = silfp_impl_is_optic();

    ret = silfp_ext_test_deadpx(optic, &result, &data[0], &data[1]);
    if (ret >= 0) {
        ret = _ext_set_err_with_result_data(buffer, sizeof(buffer), 0, result, data, NUM_ELEMS(data));
    }

    if (ret > 0) {
        _ext_skt_send_response(cmd_id, (int8_t *)buffer, ret);
    } else {
        _ext_skt_send_error_notice(cmd_id, ret);
    }

    return EXT_RET_TO_IDLE;
}

// speed test
static int32_t _ext_skt_test_speed_inter(void *buffer, uint32_t size)
{
    int32_t ret = 0;
    char response[256] = {0};
    uint32_t len = sizeof(response);

    ret = silfp_ext_test_speed(response, &len);
    if (len == 0 || len > sizeof(response)) {
        ret = -SL_ERROR_BAD_PARAMS;
    }

    if (ret >= 0 ) {
        ret = _ext_set_data_result(buffer, size, response, len);
    }

    return ret;
}

static int32_t _ext_skt_test_speed(uint32_t cmd_id)
{
    int32_t ret = 0;
    char buffer[256] = {0};

    silfp_cust_set_hbm_mode(1);
    do {
        ret = silfp_impl_get_finger_down_with_cancel();
        if (ret < 0) {
            continue;
        }

        ret = _ext_skt_test_speed_inter(buffer, sizeof(buffer));
        if (ret > 0) {
            _ext_skt_send_response(cmd_id, (int8_t *)buffer, ret);
        } else {
            _ext_skt_send_error_notice(cmd_id, ret);
        }

        if (ret != -SL_ERROR_CANCELED && !silfp_worker_is_canceled()) {
            silfp_impl_wait_finger_up_with_cancel();
        }
    } while (!silfp_worker_is_canceled());
    silfp_cust_restore_hbm();

    return EXT_RET_TO_NONE;
}

// test image finish
static int32_t _ext_skt_test_image_finish(uint32_t cmd_id)
{
    silfp_ext_test_image_finish();
    _ext_buffer_release();
    _ext_skt_send_error_notice(cmd_id, 0);

    return EXT_RET_TO_IDLE;
}

// calibrate
static int32_t _ext_skt_test_calibrate(uint32_t cmd_id)
{
    int32_t ret = 0;

    ret = silfp_impl_calibrate();
    if (ret >= 0) {
        ret = 0;
    }
    _ext_skt_send_error_notice(cmd_id, ret);

    return EXT_RET_TO_IDLE;
}

// calibrate step 1 ~ 4
static int32_t _ext_skt_test_calibrate_step_inter(int32_t step, void *buffer, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_impl_cal_step(step);
    if (ret >= 0) {
        ret = 0;
    }
    ret = _ext_set_err_with_uint_2_bytes(buffer, len, ret, (uint32_t *)(&step), 1);

    return ret;
}

static int32_t _ext_skt_test_snr_inter(int32_t step, void *buffer, uint32_t len)
{
    int32_t ret = 0;
    uint32_t snr_result = 0;
    uint32_t data[4] = {0};
    data[0] = NUM_ELEMS(data) - 1;

    ret = silfp_ext_optic_test_snr(&snr_result, &data[1], &data[2], &data[3]);
    if (ret < 0) {
        ret = -SL_ERROR_SNR_TEST_FAILED;
    } else {
        ret = 0;
    }
    ret = _ext_set_err_with_result_data(buffer, len, ret, step, data, NUM_ELEMS(data));
    return ret;
}

static int32_t _ext_skt_test_calibrate_step(uint32_t cmd_id)
{
    int32_t ret = 0;
    int32_t step = 0;
    char buffer[256] = {0};
    uint32_t len = sizeof(buffer);

    pthread_mutex_lock(&m_ext_skt_data_lock);
    if (m_ext_skt_cmd_data.req_len >= m_ext_skt_cmd_data.offset + 1) {
        step = (m_ext_skt_cmd_data.req[m_ext_skt_cmd_data.offset] & 0xFF);
    } else {
        ret = -SL_ERROR_BAD_PARAMS;
    }
    pthread_mutex_unlock(&m_ext_skt_data_lock);

    LOG_MSG_DEBUG("cal step %d", step);

    if (ret >= 0) {
        switch (step) {
        case TEST_SUB_CMD_CAL_STEP_1: {
            silfp_cust_set_hbm_mode(1);
            ret = _ext_skt_test_calibrate_step_inter(step, buffer, len);
            silfp_cust_restore_hbm();
            break;
        }
        case TEST_SUB_CMD_CAL_STEP_2: {
            silfp_cust_set_hbm_mode(1);
            ret = _ext_skt_test_calibrate_step_inter(step, buffer, len);
            silfp_cust_restore_hbm();
            break;
        }
        case TEST_SUB_CMD_CAL_STEP_3: {
            silfp_cust_set_hbm_mode(0);
            silfp_cust_set_brightness(BRIGHTNESS_ALL);
            ret = _ext_skt_test_calibrate_step_inter(step, buffer, len);
            silfp_cust_restore_hbm();
            silfp_cust_restore_brightness();
            break;
        }
        case TEST_SUB_CMD_CAL_STEP_4: {
            silfp_cust_set_hbm_mode(1);
            ret = _ext_skt_test_snr_inter(step, buffer, len);
            silfp_cust_restore_hbm();
            break;
        }
        default: {
            LOG_MSG_ERROR("test %d not implement", step);
            ret = _ext_set_err_with_uint_2_bytes(buffer, len, ret, (uint32_t *)(&step), 1);
            break;
        }
        }
    } else {
        ret = _ext_set_err_with_uint_2_bytes(buffer, len, ret, (uint32_t *)(&step), 1);
    }

    if (ret > 0) {
        _ext_skt_send_response(cmd_id, (int8_t *)buffer, ret);
    } else {
        _ext_skt_send_error_notice(cmd_id, ret);
    }

    return EXT_RET_TO_IDLE;
}

static int32_t _ext_skt_test_self(uint32_t cmd_id)
{
    int32_t ret = 0;
    uint32_t result = 0;

    int32_t optic = silfp_impl_is_optic();
    ret = silfp_ext_test_self(optic, &result);

    if (ret < 0 || result != EXT_RESULT_PASS) {
        ret = -1;
    } else {
        ret = 0;
    }

    _ext_skt_send_error_notice(cmd_id, ret);
    return EXT_RET_TO_IDLE;
}

static int32_t _ext_skt_test_flash(uint32_t cmd_id)
{
    int32_t ret = 0;

    ret = silfp_ext_test_flash();

    _ext_skt_send_error_notice(cmd_id, ret);
    return EXT_RET_TO_IDLE;
}

static int32_t _ext_skt_test_otp(uint32_t cmd_id)
{
    int32_t ret = 0;
    char buffer[256] = {0};
    uint32_t otp[3] = {0};
    uint32_t offset = 0;

    ret = silfp_ext_test_otp(&otp[0], &otp[1], &otp[2]);
    if (ret >= 0) {
        ret = _ext_set_err_with_result_data(buffer, sizeof(buffer), 0, NUM_ELEMS(otp), otp, NUM_ELEMS(otp));
        if (ret >= 0) {
            offset = ret;
            ret = silfp_cust_otp_parse(buffer, sizeof(buffer), offset, otp, NUM_ELEMS(otp));
            if (ret < 0) {
                ret = offset;
            }
        }
    }

    if (ret > 0) {
        _ext_skt_send_response(cmd_id, (int8_t *)buffer, ret);
    } else {
        _ext_skt_send_error_notice(cmd_id, ret);
    }
    return EXT_RET_TO_IDLE;
}

#else
// not enable socket server
int32_t silfp_ext_skt_commond(void)
{
    return 0;
}
#endif // SIL_FP_EXT_SKT_SERVER_ENABLE