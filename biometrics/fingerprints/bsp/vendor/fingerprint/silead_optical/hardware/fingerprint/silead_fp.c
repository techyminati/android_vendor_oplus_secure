/******************************************************************************
 * @file   silead_fp.c
 * @brief  Contains CA send command functions.
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
 * <author>      <date>   <version>     <desc>
 * David Wang   2018/7/2    0.1.0      Init version
 * Bangxiong.Wu 2019/06/21  1.0.0      modify for sync tpl to tee
 *
 *****************************************************************************/

#define FILE_TAG "silead_fp"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>

#include "tz_cmd.h"
#include "silead_fp.h"
#include "silead_error.h"
#include "silead_ca.h"

// 0: send buffer 1: get buffer 2: send & get buffer 3:always get even error 4: send & always get
#define _tac_send_modified_command(cmd, p, l)                                    silfp_ca_send_modified_command(cmd, p, l, 0, 0, 0, NULL, NULL)
#define _tac_send_modified_command_1r(cmd, p, l, r)                              silfp_ca_send_modified_command(cmd, p, l, 0, 0, 0, r, NULL)
#define _tac_send_modified_command_1d(cmd, p, l, d)                              silfp_ca_send_modified_command(cmd, p, l, 0, d, 0, NULL, NULL)
#define _tac_send_modified_command_2r(cmd, p, l, r1, r2)                         silfp_ca_send_modified_command(cmd, p, l, 0, 0, 0, r1, r2)
#define _tac_send_modified_command_and_get(cmd, p, l)                            silfp_ca_send_modified_command(cmd, p, l, 2, 0, 0, NULL, NULL)
#define _tac_send_modified_command_1d_1r_and_get(cmd, p, l, d1, r1)              silfp_ca_send_modified_command(cmd, p, l, 2, d1, 0, r1, NULL)
#define _tac_send_modified_command_1d_2r_and_get(cmd, p, l, d1, r1, r2)          silfp_ca_send_modified_command(cmd, p, l, 2, d1, 0, r1, r2)
#define _tac_send_modified_command_1d_2r_and_get_always(cmd, p, l, d, r1, r2)    silfp_ca_send_modified_command(cmd, p, l, 4, d, 0, r1, r2)
#define _tac_send_modified_command_2d_1r_and_get(cmd, p, l, d1, d2, r1)          silfp_ca_send_modified_command(cmd, p, l, 2, d1, d2, r1, NULL)
#define _tac_send_modified_command_get(cmd, p, l)                                silfp_ca_send_modified_command(cmd, p, l, 1, 0, 0, NULL, NULL)
#define _tac_send_modified_command_get_1r(cmd, p, l, r)                          silfp_ca_send_modified_command(cmd, p, l, 1, 0, 0, r, NULL)
#define _tac_send_modified_command_get_2r(cmd, p, l, r1, r2)                     silfp_ca_send_modified_command(cmd, p, l, 1, 0, 0, r1, r2)
#define _tac_send_modified_command_get_1d_2r(cmd, p, l, d, r1, r2)               silfp_ca_send_modified_command(cmd, p, l, 1, d, 0, r1, r2)
#define _tac_send_modified_command_get_2d_2r(cmd, p, l, d1, d2, r1, r2)          silfp_ca_send_modified_command(cmd, p, l, 1, d1, d2, r1, r2)
#define _tac_send_modified_command_get_1d_2r_always(cmd, p, l, d, r1, r2)        silfp_ca_send_modified_command(cmd, p, l, 3, d, 0, r1, r2)
#define _tac_send_normal_command(cmd)                                    silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, NULL, NULL, NULL)
#define _tac_send_normal_command_1d(cmd, d)                              silfp_ca_send_normal_command(cmd, d, 0, 0, 0, NULL, NULL, NULL)
#define _tac_send_normal_command_2d(cmd, d1, d2)                         silfp_ca_send_normal_command(cmd, d1, d2, 0, 0, NULL, NULL, NULL)
#define _tac_send_normal_command_1r(cmd, p1)                             silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, p1, NULL, NULL)
#define _tac_send_normal_command_2r(cmd, p1, p2)                         silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, p1, p2, NULL)
#define _tac_send_normal_command_3r(cmd, p1, p2, p3)                     silfp_ca_send_normal_command(cmd, 0, 0, 0, 0, p1, p2, p3)
#define _tac_send_normal_command_1d_2r(cmd, d1, p1, p2)                  silfp_ca_send_normal_command(cmd, d1, 0, 0, 0, p1, p2, NULL)
#define _tac_send_normal_command_2d_1r(cmd, d1, d2, p1)                  silfp_ca_send_normal_command(cmd, d1, d2, 0, 0, p1, NULL, NULL)
#define _tac_send_normal_command_4d_1r(cmd, d1, d2, d3, d4, p1)          silfp_ca_send_normal_command(cmd, d1, d2, d3, d4, p1, NULL, NULL)
#define _tac_send_normal_command_4d_2r(cmd, d1, d2, d3, d4, p1, p2)      silfp_ca_send_normal_command(cmd, d1, d2, d3, d4, p1, p2, NULL)
#define _tac_send_normal_command_4d_3r(cmd, d1, d2, d3, d4, p1, p2, p3)  silfp_ca_send_normal_command(cmd, d1, d2, d3, d4, p1, p2, p3)

static int32_t fp_tac_open(const void *ta_name)
{
    return silfp_ca_open(ta_name);
}

static int32_t fp_tac_close(void)
{
    return silfp_ca_close();
}

static int64_t fp_tac_get_int64_command(uint32_t cmd, uint32_t v)
{
    int64_t value = 0;
    uint32_t low = 0;
    uint32_t high = 0;

    int32_t ret = _tac_send_normal_command_1d_2r(cmd, v, &high, &low);
    if (ret < 0) {
        value = ret;
    } else {
        value = high;
        value <<= 32;
        value |= low;
    }

    return value;
}

static int32_t fp_tac_download_mode(uint32_t status, uint32_t charging)
{
    uint32_t data = (status & 0x00FF);
    if (charging > 0) {
        data |= ((charging << 8) & 0xFF00);
    }

    return _tac_send_normal_command_1d(TZ_FP_CMD_MODE_DOWNLOAD, data);
}

static int32_t fp_tac_capture_image(int32_t type, uint32_t step)
{
    return _tac_send_normal_command_2d(TZ_FP_CMD_CAPTURE_IMG, type, step);
}

static int32_t fp_tac_nav_capture_image(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_NAV_CAPTURE_IMG);
}

static int32_t fp_tac_auth_start(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_AUTH_START);
}

static int32_t fp_tac_auth_step(uint64_t op_id, uint32_t step, uint32_t is_pay, uint32_t *fid)
{
    return _tac_send_normal_command_4d_1r(TZ_FP_CMD_AUTH_STEP, (uint32_t)(op_id >> 32), (uint32_t)op_id, step, is_pay, fid);
}

static int32_t fp_tac_auth_end(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_AUTH_END);
}

static int32_t fp_tac_get_enroll_num(uint32_t *num)
{
    return _tac_send_normal_command_1r(TZ_FP_CMD_GET_ENROLL_NUM, num);
}

static int32_t fp_tac_enroll_start(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_ENROLL_START);
}

static int32_t fp_tac_enroll_step(uint32_t *remaining)
{
    return _tac_send_normal_command_1r(TZ_FP_CMD_ENROLL_STEP, remaining);
}

static int32_t fp_tac_enroll_end(int32_t status, uint32_t *fid)
{
    if (fid == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tac_send_normal_command_2d_1r(TZ_FP_CMD_ENROLL_END, status, *fid, fid);
}

static int32_t fp_tac_nav_support(uint32_t *type)
{
    return _tac_send_normal_command_1r(TZ_FP_CMD_NAV_SUPPORT, type);
}

static int32_t fp_tac_nav_start(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_NAV_START);
}

static int32_t fp_tac_nav_step(uint32_t *pkey)
{
    return _tac_send_normal_command_1r(TZ_FP_CMD_NAV_STEP, pkey);
}

static int32_t fp_tac_nav_end(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_NAV_END);
}

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
int32_t fp_tac_cal_test_cmd(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result)
{
    if (buffer == NULL || plen == NULL || *plen <= 0 || result == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_1d_2r_and_get_always(TZ_FP_CMD_CAL_TEST_CMD, buffer, *plen, cmd, plen, result);
}
#endif

static int32_t fp_tac_init(uint32_t data1, uint32_t data2, uint32_t data3, uint32_t data4, const char *dev_path,
                           uint32_t *chipid, uint32_t *subid, uint32_t *vid, uint32_t *update_cfg)
{
    int32_t ret = 0;
    uint32_t value1 = 0;

    if (dev_path != NULL && strlen(dev_path) > 0) {
        _tac_send_modified_command(TZ_FP_CMD_SET_SPI_DEV, (void *)dev_path, strlen(dev_path));
    }

    ret = _tac_send_normal_command_4d_3r(TZ_FP_CMD_INIT, data1, data2, data3, data4, chipid, subid, &value1);
    if (ret >= 0) {
        if (vid != NULL) {
            *vid = (value1 & 0x7FFFFFFF);
        }
        if (update_cfg != NULL) {
            *update_cfg = (value1 & 0x80000000) ? 1 : 0;
        }
    }
    return ret;
}

static int32_t fp_tac_deinit(void)
{
    _tac_send_normal_command(TZ_FP_CMD_DEINIT);
    return fp_tac_close();
}

static int32_t fp_tac_set_gid(uint32_t gid, uint32_t serial)
{
    return _tac_send_normal_command_2d(TZ_FP_CMD_SET_GID, gid, serial);
}

static int32_t fp_tac_load_user_db(const char *db_path, uint32_t *need_sync)
{
    if (db_path == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tac_send_modified_command_1r(TZ_FP_CMD_LOAD_USER_DB, (void *)db_path, strlen(db_path), need_sync);
}

static int32_t fp_tac_remove_finger(uint32_t fid)
{
    return _tac_send_normal_command_1d(TZ_FP_CMD_FP_REMOVE, fid);
}

static int32_t fp_tac_get_db_count(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_GET_DB_COUNT);
}

static int32_t fp_tac_get_finger_prints(uint32_t *ids, uint32_t size)
{
    int32_t ret = 0;
    uint32_t count = 0;

    if (ids == NULL || size == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tac_send_modified_command_get_1r(TZ_FP_CMD_GET_FINGERPRINTS, (void *)ids, size*sizeof(uint32_t), &count);
    if (ret >= 0) {
        ret = count;
    }

    return ret;
}

static int64_t fp_tac_load_enroll_challenge(void)
{
    return fp_tac_get_int64_command(TZ_FP_CMD_LOAD_ENROLL_CHALLENGE, 0);
}

static int32_t fp_tac_set_enroll_challenge(uint64_t __unused challenge)
{
    return _tac_send_normal_command_2d(TZ_FP_CMD_SET_ENROLL_CHALLENGE, 0, 0);
}

static int32_t fp_tac_verify_enroll_challenge(const void *hat, uint32_t size)
{
    int32_t ret = 0;
    void *buffer = NULL;

    if (hat == NULL || size == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    buffer = malloc(size);
    if (buffer == NULL) {
        LOG_MSG_ERROR("malloc failed");
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    memcpy(buffer, hat, size);
    ret = _tac_send_modified_command(TZ_FP_CMD_VERIFY_ENROLL_CHALLENGE, (void *)buffer, size);

    free(buffer);
    return ret;
}

static int64_t fp_tac_load_auth_id(void)
{
    return fp_tac_get_int64_command(TZ_FP_CMD_LOAD_AUTH_ID, 0);
}

static int32_t fp_tac_get_hw_auth_obj(void *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tac_send_modified_command_and_get(TZ_FP_CMD_GET_AUTH_OBJ, buffer, length);
}

static int32_t fp_tac_update_cfg(const void *buffer, uint32_t len)
{
    return _tac_send_modified_command(TZ_FP_CMD_UPDATE_CFG, (void *)buffer, len);
}

static int32_t fp_tac_init2(const void *buffer, uint32_t len, uint32_t *feature, uint32_t *tpl_size)
{
    int32_t ret = 0;
    uint32_t value1 = 0;
    void *ptoken = NULL;
    int32_t token_len = 0;

    if (buffer == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tac_send_modified_command_2r(TZ_FP_CMD_INIT2, (void *)buffer, len, &value1, tpl_size);
    if (ret >= 0) {
        if (!(value1 & 0x80000000)) {
            token_len = silfp_ca_keymaster_get(&ptoken);
            if (token_len > 0) {
                _tac_send_modified_command(TZ_FP_CMD_SET_KEY_DATA, ptoken, token_len);
                free(ptoken);
            }
        }

        if (feature != NULL) {
            *feature = (value1 & 0x7FFFFFFF);
        }
    }

    return ret;
}

static int32_t fp_tac_load_template(uint32_t fid, const void *buffer, uint32_t len)
{
    if (buffer == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tac_send_modified_command_1d(TZ_FP_CMD_LOAD_TEMPLATE, (void *)buffer, len, fid);
}

static int32_t fp_tac_save_template(void *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_get_1r(TZ_FP_CMD_SAVE_TEMPLATE, buffer, *plen, plen);
}

static int32_t fp_tac_update_template(void *buffer, uint32_t *plen, uint32_t *fid)
{
    if (buffer == NULL || plen == NULL || fid == NULL || *plen <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_get_2r(TZ_FP_CMD_UPDATE_TEMPLATE, buffer, *plen, plen, fid);
}

static int32_t fp_tac_set_log_mode(uint8_t tam, uint8_t agm)
{
    return _tac_send_normal_command_2d(TZ_FP_CMD_SET_LOG_MODE, tam, agm);
}

static int32_t fp_tac_set_nav_mode(uint32_t mode)
{
    return _tac_send_normal_command_1d(TZ_FP_CMD_SET_NAV_MODE, mode);
}

static int32_t fp_tac_capture_image_pre(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_CAPTURE_IMG_PRE);
}

static int32_t fp_tac_capture_image_raw(int32_t type, uint32_t step)
{
#ifdef SIL_CODE_COMPATIBLE // ????
    int32_t ret = 0;
    do {
        ret = _tac_send_normal_command_2d(TZ_FP_CMD_CAPTURE_IMG_RAW, type, step);
    } while(ret > 0);

    return ret;
#else
    return _tac_send_normal_command_2d(TZ_FP_CMD_CAPTURE_IMG_RAW, type, step);
#endif
}

static int32_t fp_tac_capture_image_after(int32_t type, uint32_t step)
{
#ifdef SIL_CODE_COMPATIBLE // ????
    return _tac_send_normal_command_2d(TZ_FP_CMD_CAPTURE_IMG_AFTER, (type == 1) ? 1 : 0, step);
#else
    return _tac_send_normal_command_2d(TZ_FP_CMD_CAPTURE_IMG_AFTER, type, step);
#endif
}

static int32_t fp_tac_check_esd(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_CHECK_ESD);
}

static int32_t fp_tac_get_finger_status(uint32_t *status, uint32_t *addition)
{
    return _tac_send_normal_command_2r(TZ_FP_CMD_GET_FINGER_STATUS, status, addition);
}

static int32_t fp_tac_get_otp(uint32_t *otp1, uint32_t *otp2, uint32_t *otp3)
{
    return _tac_send_normal_command_3r(TZ_FP_CMD_GET_OTP, otp1, otp2, otp3);
}

static int32_t fp_tac_send_cmd_with_buf(uint32_t cmd, void *buffer, uint32_t len)
{
    if (buffer == NULL || len <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_1d(TZ_FP_CMD_SEND_CMD_WITH_BUF, buffer, len, cmd);
}

static int32_t fp_tac_send_cmd_with_buf_and_get(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result)
{
    if (buffer == NULL || plen == NULL || *plen <= 0 || result == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_1d_2r_and_get_always(TZ_FP_CMD_SEND_CMD_WITH_BUF_GET, buffer, *plen, cmd, plen, result);
}

static int32_t fp_tac_get_config(void *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_get_1r(TZ_FP_CMD_GET_CONFIG, buffer, *plen, plen);
}

static int32_t fp_tac_calibrate(void *buffer, uint32_t len)
{
    if (buffer == NULL || len <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_and_get(TZ_FP_CMD_CALIBRATE, (void *)buffer, len);
}

static int32_t fp_tac_calibrate2(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_CALIBRATE2);
}

static int32_t fp_tac_calibrate_optic(uint32_t step, void *buffer, uint32_t *plen, uint32_t flag)
{
    uint32_t data = (step & 0x000F);
    data |= ((flag << 4) & 0x00F0);

    if (buffer == NULL || plen == NULL || *plen <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_1d_1r_and_get(TZ_FP_CMD_CALIBRATE_OPTIC, (void *)buffer, *plen, data, plen);
}

static int32_t fp_tac_get_version(uint32_t *algo, uint32_t *ta)
{
    return _tac_send_normal_command_2r(TZ_FP_CMD_GET_VERSIONS, algo, ta);
}

static int32_t fp_tac_get_chip_id(uint32_t *chipid, uint32_t *subid)
{
    return _tac_send_normal_command_2r(TZ_FP_CMD_GET_CHIPID, chipid, subid);
}

static int32_t fp_tac_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *max_size, uint32_t *w_ori, uint32_t *h_ori, uint8_t *bitcount, uint8_t *bitcount_orig)
{
    int32_t ret = 0;
    uint32_t data1 = 0;
    uint32_t data2 = 0;
    uint32_t data3 = 0;

    ret = _tac_send_normal_command_3r(TZ_FP_CMD_TEST_GET_IMG_INFO, &data1, &data2, &data3);
    if (ret >= 0) {
#ifdef SIL_CODE_COMPATIBLE // ????
        if ((data1 & 0xFFFF0000) == 0 || (data2 & 0xFFFF0000) == 0) {
            if (w != NULL) {
                *w = (data1 & 0x0000FFFF);
            }
            if (h != NULL) {
                *h = (data2 & 0x0000FFFF);
            }
            if (w_ori != NULL) {
                *w_ori = (data1 & 0x0000FFFF);
            }
            if (h_ori != NULL) {
                *h_ori = (data2 & 0x0000FFFF);
            }
            if (max_size != NULL) {
                *max_size = (data3 & 0x00FFFFFF);
            }
            if (bitcount != NULL) {
                *bitcount = 8;
            }
            if (bitcount_orig != NULL) {
                *bitcount_orig = 8;
            }
        } else {
            if (max_size != NULL) {
                *max_size = (data3 & 0x00FFFFFF);
            }
            if (bitcount != NULL) {
                *bitcount = (((data3 >> 24) & 0x0000000F) << 3);
            }
            if (bitcount_orig != NULL) {
                *bitcount_orig = (((data3 >> 28) & 0x0000000F) << 3);
            }
            if (bitcount != NULL && *bitcount != 0) {
                if (w != NULL) {
                    *w = (data1 & 0x0000FFFF);
                }
                if (h != NULL) {
                    *h = ((data1 >> 16) & 0x0000FFFF);
                }
                if (w_ori != NULL) {
                    *w_ori = (data2 & 0x0000FFFF);
                }
                if (h_ori != NULL) {
                    *h_ori = ((data2 >> 16) & 0x0000FFFF);
                }
            } else {
                if (h != NULL) {
                    *h = (data1 & 0x0000FFFF);
                }
                if (w != NULL) {
                    *w = ((data1 >> 16) & 0x0000FFFF);
                }
                if (h_ori != NULL) {
                    *h_ori = (data2 & 0x0000FFFF);
                }
                if (w_ori != NULL) {
                    *w_ori = ((data2 >> 16) & 0x0000FFFF);
                }
            }
        }
#else
        if (w != NULL) {
            *w = (data1 & 0x0000FFFF);
        }
        if (h != NULL) {
            *h = ((data1 >> 16) & 0x0000FFFF);
        }
        if (w_ori != NULL) {
            *w_ori = (data2 & 0x0000FFFF);
        }
        if (h_ori != NULL) {
            *h_ori = ((data2 >> 16) & 0x0000FFFF);
        }
        if (max_size != NULL) {
            *max_size = (data3 & 0x00FFFFFF);
        }
        if (bitcount != NULL) {
            *bitcount = (((data3 >> 24) & 0x0000000F) << 3);
        }
        if (bitcount_orig != NULL) {
            *bitcount_orig = (((data3 >> 28) & 0x0000000F) << 3);
        }
#endif
    }
    return ret;
}

static int32_t fp_tac_test_dump_data(uint32_t type, uint32_t step, void *buffer, uint32_t len, uint32_t *remain, uint32_t *size, uint32_t *w, uint32_t *h, uint8_t *bitcount)
{
    int32_t ret = 0;
    uint32_t data = 0;
    uint32_t data2 = 0;

    if (buffer == NULL || len == 0 || size == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tac_send_modified_command_get_2d_2r(TZ_FP_CMD_TEST_DUMP_DATA, buffer, len, type, step, &data, &data2);
    if (ret >= 0) {
        if (w != NULL) {
            *w = (data2 & 0x0000FFFF);
        }
        if (h != NULL) {
            *h = ((data2 >> 16) & 0x0000FFFF);
        }
        if (size != NULL) {
            *size = (data & 0x00FFFFFF);
        }
        if (remain != NULL) {
            *remain = ((data >> 24) & 0x0000003F);
        }
        if (bitcount != NULL) {
            *bitcount = (((data >> 30) & 0x0000003) << 3);
        }
    }

    return ret;
}

static int32_t fp_tac_test_image_capture(uint32_t mode, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl, uint8_t *greyavg, uint8_t *greymax)
{
    int32_t ret = 0;
    uint32_t data = 0;
    uint32_t data2 = 0;

    if (buffer == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _tac_send_modified_command_get_1d_2r_always(TZ_FP_CMD_TEST_IMAGE_CAPTURE, buffer, *len, mode, &data, &data2);
    if (quality != NULL) {
        *quality = (data2 & 0x000000FF);
    }
    if (area != NULL) {
        *area = ((data2 >> 8) & 0x000000FF);
    }
    if (istpl != NULL) {
        *istpl = ((data2 >> 16) & 0x000000FF);
    }
    if (greyavg != NULL) {
        *greyavg = (data2 >> 24 & 0x000000FF);
    }
    if (greymax != NULL) {
        *greymax = (data >> 24) & 0x000000FF;
    }
    if (len != NULL) {
        *len = (data & 0x00FFFFFF);
    }

    return ret;
}

static int32_t fp_tac_test_send_group_image(uint32_t orig, uint32_t frr, uint32_t imgtype, void *buffer, uint32_t *plen)
{
    uint32_t mode = (frr & 0x00FF);
    if (orig) {
        mode |= ((0x01 << 8) & 0xFF00);
    }

    if (buffer == NULL || plen == NULL || *plen <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }
    return _tac_send_modified_command_2d_1r_and_get(TZ_FP_CMD_TEST_SEND_GP_IMG, (void *)buffer, *plen, mode, imgtype, plen);
}

static int32_t fp_tac_test_image_finish(void)
{
    return _tac_send_normal_command(TZ_FP_CMD_TEST_IMAGE_FINISH);
}

static int32_t fp_tac_test_deadpx(uint32_t *result, uint32_t *deadpx, uint32_t *badline)
{
    if (result== NULL || deadpx == NULL || badline == NULL) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_normal_command_3r(TZ_FP_CMD_TEST_DEADPX, result, deadpx, badline);
}

int32_t fp_tac_test_speed(void *buffer, uint32_t *plen)
{
    if (buffer == NULL || plen == NULL || *plen <= 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    return _tac_send_modified_command_get_1r(TZ_FP_CMD_TEST_SPEED, buffer, *plen, plen);
}

/*********************************************************************************/
static const silead_fp_handle_t s_callbacks = {
    fp_tac_download_mode,
    fp_tac_capture_image,
    fp_tac_nav_capture_image,

    fp_tac_auth_start,
    fp_tac_auth_step,
    fp_tac_auth_end,

    fp_tac_get_enroll_num,
    fp_tac_enroll_start,
    fp_tac_enroll_step,
    fp_tac_enroll_end,

    fp_tac_nav_support,
    fp_tac_nav_start,
    fp_tac_nav_step,
    fp_tac_nav_end,

    fp_tac_init,
    fp_tac_deinit,

    fp_tac_set_gid,
    fp_tac_load_user_db,
    fp_tac_remove_finger,
    fp_tac_get_db_count,
    fp_tac_get_finger_prints,

    fp_tac_load_enroll_challenge,
    fp_tac_set_enroll_challenge,
    fp_tac_verify_enroll_challenge,
    fp_tac_load_auth_id,
    fp_tac_get_hw_auth_obj,

    fp_tac_update_cfg,
    fp_tac_init2,

    fp_tac_load_template,
    fp_tac_save_template,
    fp_tac_update_template,

    fp_tac_set_log_mode,
    fp_tac_set_nav_mode,

    fp_tac_capture_image_pre,
    fp_tac_capture_image_raw,
    fp_tac_capture_image_after,

    fp_tac_check_esd,
    fp_tac_get_finger_status,
    fp_tac_get_otp,
    fp_tac_send_cmd_with_buf,
    fp_tac_send_cmd_with_buf_and_get,
    fp_tac_get_config,
    fp_tac_calibrate,
    fp_tac_calibrate2,
    fp_tac_calibrate_optic,

    fp_tac_get_version,
    fp_tac_get_chip_id,

    fp_tac_test_get_image_info,
    fp_tac_test_dump_data,

    fp_tac_test_image_capture,
    fp_tac_test_send_group_image,
    fp_tac_test_image_finish,

    fp_tac_test_deadpx,
    fp_tac_test_speed,
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
    fp_tac_cal_test_cmd,
#endif
};

const silead_fp_handle_t * silfp_get_impl_handler(const void *ta_name)
{
    if (fp_tac_open(ta_name) < 0) {
        fp_tac_deinit();
        return NULL;
    }

    return &s_callbacks;
}
