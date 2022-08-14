/******************************************************************************
 * @file   silead_cmd.c
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
 * <author>    <date>   <version>     <desc>
 * David Wang  2018/7/20    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_cmd"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_fp.h"
#include "silead_cmd.h"
#include "silead_bmp.h"
#include "silead_dump.h"
#include "silead_worker.h"
#include "silead_dev.h"

static const silead_fp_handle_t *m_fp_impl_handler = NULL;

int32_t silfp_cmd_wait_finger_status(uint32_t status, uint32_t charging)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_download_mode != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_download_mode(status, charging);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_download_mode");
        }
    }
    return ret;
}

int32_t silfp_cmd_capture_image(int32_t type, uint32_t step)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_capture_image != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_capture_image(type, step);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_capture_image");
        }
    }
    return ret;
}

int32_t silfp_cmd_nav_capture_image(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_capture_image != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_nav_capture_image();
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_capture_image");
        }
    }
    return ret;
}

int32_t silfp_cmd_auth_start(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_auth_start != NULL) {
            ret = m_fp_impl_handler->fp_auth_start();
        } else {
            LOG_MSG_DEBUG("No implement fp_auth_start");
        }
    }
    return ret;
}

int32_t silfp_cmd_auth_step(uint64_t op_id, uint32_t step, uint32_t is_pay, uint32_t *fid)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_auth_step != NULL) {
            ret = m_fp_impl_handler->fp_auth_step(op_id, step, is_pay, fid);
        } else {
            LOG_MSG_DEBUG("No implement fp_auth_step");
        }
    }
    return ret;
}

int32_t silfp_cmd_auth_end(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_auth_end != NULL) {
            ret = m_fp_impl_handler->fp_auth_end();
        } else {
            LOG_MSG_DEBUG("No implement fp_auth_end");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_enroll_num(uint32_t *num)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_enroll_num != NULL) {
            ret = m_fp_impl_handler->fp_get_enroll_num(num);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_enroll_num");
        }
    }
    return ret;
}

int32_t silfp_cmd_enroll_start(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_enroll_start != NULL) {
            ret = m_fp_impl_handler->fp_enroll_start();
        } else {
            LOG_MSG_DEBUG("No implement fp_enroll_start");
        }
    }
    return ret;
}

int32_t silfp_cmd_enroll_step(uint32_t *remaining)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_enroll_step != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_enroll_step(remaining);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_enroll_step");
        }
    }
    return ret;
}

int32_t silfp_cmd_enroll_end(int32_t status, uint32_t *fid)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_enroll_end != NULL) { // clean enroll env
            ret = m_fp_impl_handler->fp_enroll_end(status, fid);
        } else {
            LOG_MSG_DEBUG("No implement fp_enroll_end");
        }
    }
    return ret;
}

int32_t silfp_cmd_nav_support(uint32_t *type)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_support != NULL) {
            ret = m_fp_impl_handler->fp_nav_support(type);
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_support");
        }
    }
    return ret;
}

int32_t silfp_cmd_nav_start(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_start != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_nav_start();
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_start");
        }
    }
    return ret;
}

int32_t silfp_cmd_nav_step(uint32_t *key)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_step != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_nav_step(key);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_step");
        }
    }
    return ret;
}

int32_t silfp_cmd_nav_end(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_nav_end != NULL) {
            ret = m_fp_impl_handler->fp_nav_end();
        } else {
            LOG_MSG_DEBUG("No implement fp_nav_end");
        }
    }
    return ret;
}

int32_t silfp_cmd_deep_sleep_mode(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_download_mode != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_download_mode(DEEP_SLEEP, 0);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_download_mode");
        }
    }
    return ret;
}

int32_t silfp_cmd_download_normal(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (silfp_dev_hw_reset(0) < 0) {
        LOG_MSG_ERROR("reset fp chip fail");
    }

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_download_mode != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_download_mode(NORMAL, 0);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_download_mode");
        }
    }
    return ret;
}

int32_t silfp_cmd_set_env(const void *ta_name)
{
    if (m_fp_impl_handler == NULL) {
        m_fp_impl_handler = silfp_get_impl_handler(ta_name);
    }
    return 0;
}

int32_t silfp_cmd_init(fp_dev_conf_t *dev_conf, uint32_t *chipid, uint32_t *subid, uint32_t *vid, uint32_t *update_cfg)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    uint32_t data1 = 0;
    uint32_t data2 = 0;
    uint32_t data3 = 0;
    uint32_t data4 = 0;
    const char *dev_path = NULL;

    if (dev_conf == NULL) {
        return -SL_ERROR_BAD_PARAMS;
    }

    data1 = (dev_conf->mode & 0x000000FF);
    data1 <<= 8;
    data1 |= (dev_conf->bits & 0x000000FF);
    data1 <<= 16;
    data1 |= (dev_conf->delay & 0x0000FFFF);

    data2 = dev_conf->speed;

    data3 = (dev_conf->dev_id & 0x000000FF);
    data3 <<= 16;
    data3 |= (dev_conf->reserve & 0x0000FFFF);

    data4 = dev_conf->reg;

    if (dev_conf->dev[0] != 0 && strlen(dev_conf->dev) > 0) {
        dev_path = dev_conf->dev;
    }

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_init != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_init(data1, data2, data3, data4, dev_path, chipid, subid, vid, update_cfg);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_init");
        }
    }
    return ret;
}

int32_t silfp_cmd_deinit(void)
{
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_deinit != NULL) {
            m_fp_impl_handler->fp_deinit();
        } else {
            LOG_MSG_DEBUG("No implement fp_deinit");
        }
    }
    m_fp_impl_handler = NULL;
    return SL_SUCCESS;
}

int32_t silfp_cmd_set_gid(uint32_t gid, uint32_t serial)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_gid != NULL) {
            ret = m_fp_impl_handler->fp_set_gid(gid, serial);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_gid");
        }
    }
    return ret;
}

int32_t silfp_cmd_load_user_db(const char *db_path, uint32_t *need_sync)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_load_user_db != NULL) {
            ret = m_fp_impl_handler->fp_load_user_db(db_path, need_sync);
        } else {
            LOG_MSG_DEBUG("No implement fp_load_user_db");
        }
    }
    return ret;
}

int32_t silfp_cmd_remove_finger(uint32_t fid)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_remove_finger != NULL) {
            ret = m_fp_impl_handler->fp_remove_finger(fid);
        } else {
            LOG_MSG_DEBUG("No implement fp_remove_finger");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_db_count(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_db_count != NULL) {
            ret = m_fp_impl_handler->fp_get_db_count();
        } else {
            LOG_MSG_DEBUG("No implement fp_get_db_count");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_finger_prints(uint32_t *ids, uint32_t count)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_finger_prints != NULL) {
            ret = m_fp_impl_handler->fp_get_finger_prints(ids, count);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_finger_prints");
        }
    }
    return ret;
}

int64_t silfp_cmd_load_enroll_challenge(void)
{
    int64_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_load_enroll_challenge != NULL) {
            ret = m_fp_impl_handler->fp_load_enroll_challenge();
        } else {
            LOG_MSG_DEBUG("No implement fp_load_enroll_challenge");
        }
    }
    return ret;
}

int32_t silfp_cmd_set_enroll_challenge(uint64_t challenge)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_enroll_challenge != NULL) {
            ret = m_fp_impl_handler->fp_set_enroll_challenge(challenge);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_enroll_challenge");
        }
    }
    return ret;
}

int32_t silfp_cmd_verify_enroll_challenge(const void *hat, uint32_t size)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_verify_enroll_challenge != NULL) {
            ret = m_fp_impl_handler->fp_verify_enroll_challenge(hat, size);
        } else {
            LOG_MSG_DEBUG("No implement fp_verify_enroll_challenge");
        }
    }
    return ret;
}

int64_t silfp_cmd_load_auth_id(void)
{
    int64_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_load_auth_id != NULL) {
            ret = m_fp_impl_handler->fp_load_auth_id();
        } else {
            LOG_MSG_DEBUG("No implement fp_load_auth_id");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_hw_auth_obj(void *buffer, uint32_t length)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_hw_auth_obj != NULL) {
            ret = m_fp_impl_handler->fp_get_hw_auth_obj(buffer, length);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_hw_auth_obj");
        }
    }
    return ret;
}

int32_t silfp_cmd_update_cfg(const void *buffer, uint32_t len)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_update_cfg != NULL) {
            ret = m_fp_impl_handler->fp_update_cfg(buffer, len);
        } else {
            LOG_MSG_DEBUG("No implement fp_update_cfg");
        }
    }
    return ret;
}

int32_t silfp_cmd_init2(const void *buffer, const uint32_t len, uint32_t *feature, uint32_t *tpl_size)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_init2 != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_init2(buffer, len, feature, tpl_size);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_init2");
        }
    }
    return ret;
}

int32_t silfp_cmd_load_template(uint32_t fid, const void *buffer, uint32_t len)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_load_template != NULL) {
            ret = m_fp_impl_handler->fp_load_template(fid, buffer, len);
        } else {
            LOG_MSG_DEBUG("No implement fp_load_template");
        }
    }
    return ret;
}


int32_t silfp_cmd_save_template(void *buffer, uint32_t *plen)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_save_template != NULL) {
            ret = m_fp_impl_handler->fp_save_template(buffer, plen);
        } else {
            LOG_MSG_DEBUG("No implement fp_save_template");
        }
    }
    return ret;
}

int32_t silfp_cmd_update_template(void *buffer, uint32_t *plen, uint32_t *fid)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_update_template != NULL) {
            ret = m_fp_impl_handler->fp_update_template(buffer, plen, fid);
        } else {
            LOG_MSG_DEBUG("No implement fp_update_template");
        }
    }
    return ret;
}

int32_t silfp_cmd_set_log_mode(uint8_t tam, uint8_t agm)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_log_mode != NULL) {
            ret = m_fp_impl_handler->fp_set_log_mode(tam, agm);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_log_mode");
        }
    }
    return ret;
}

int32_t silfp_cmd_set_nav_mode(uint32_t mode)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_set_nav_mode != NULL) {
            ret = m_fp_impl_handler->fp_set_nav_mode(mode);
        } else {
            LOG_MSG_DEBUG("No implement fp_set_nav_mode");
        }
    }
    return ret;
}

int32_t silfp_cmd_capture_image_pre(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_capture_image_pre != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_capture_image_pre();
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_capture_image_pre");
        }
    }
    return ret;
}

int32_t silfp_cmd_capture_image_raw(int32_t type, uint32_t step)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_capture_image_raw != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_capture_image_raw(type, step);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_capture_image_raw");
        }
    }
    return ret;
}

int32_t silfp_cmd_capture_image_after(int32_t type, uint32_t step)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_capture_image_after != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_capture_image_after(type, step);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_capture_image_after");
        }
    }
    return ret;
}

int32_t silfp_cmd_check_esd(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_check_esd != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_check_esd();
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_check_esd");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_finger_status(uint32_t *status, uint32_t *addition)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_finger_status != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_get_finger_status(status, addition);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_get_finger_status");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_otp(uint32_t *otp1, uint32_t *otp2, uint32_t *otp3)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_otp != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_get_otp(otp1, otp2, otp3);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_get_otp");
        }
    }
    return ret;
}

int32_t silfp_cmd_send_cmd_with_buf(uint32_t cmd, const void *buffer, uint32_t len)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    uint8_t tmp = 0;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_send_cmd_with_buf != NULL) {
            silfp_dev_enable();
            if (buffer != NULL && len > 0) {
                ret = m_fp_impl_handler->fp_send_cmd_with_buf(cmd, (void *)buffer, len);
            } else {
                ret = m_fp_impl_handler->fp_send_cmd_with_buf(cmd, (void *)(&tmp), sizeof(tmp));
            }
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_send_cmd_with_buf");
        }
    }
    return ret;
}

int32_t silfp_cmd_send_cmd_with_buf_and_get(uint32_t cmd, void *buffer, uint32_t *plen, uint32_t *result)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_send_cmd_with_buf_and_get != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_send_cmd_with_buf_and_get(cmd, buffer, plen, result);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_send_cmd_with_buf_and_get");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_config(void *buffer, uint32_t *plen)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_config != NULL) {
            ret = m_fp_impl_handler->fp_get_config(buffer, plen);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_config");
        }
    }
    return ret;
}

int32_t silfp_cmd_calibrate(void *buffer, uint32_t len)
{
    uint32_t tmp = 0;
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;

    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_calibrate != NULL) {
            silfp_dev_enable();
            if (buffer != NULL && len > 0) {
                ret = m_fp_impl_handler->fp_calibrate(buffer, len);
            } else {
                ret = m_fp_impl_handler->fp_calibrate((void *)(&tmp), sizeof(tmp));
            }
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_calibrate");
        }
    }
    return ret;
}

int32_t silfp_cmd_calibrate2(void)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_calibrate2 != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_calibrate2();
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_calibrate2");
        }
    }
    return ret;
}

int32_t silfp_cmd_calibrate_optic(uint32_t step, void *buffer, uint32_t *plen, uint32_t flag)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_calibrate_optic != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_calibrate_optic(step, buffer, plen, flag);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_calibrate_optic");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_ta_ver(uint32_t *algoVer, uint32_t *taVer)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_version != NULL) {
            ret = m_fp_impl_handler->fp_get_version(algoVer, taVer);
        } else {
            LOG_MSG_DEBUG("No implement fp_get_version");
        }
    }
    return ret;
}

int32_t silfp_cmd_get_chipid(uint32_t *chipId, uint32_t *subId)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_get_chip_id != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_get_chip_id(chipId, subId);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_get_chip_id");
        }
    }
    return ret;
}

int32_t silfp_cmd_test_get_image_info(uint32_t *w, uint32_t *h, uint32_t *max_size, uint32_t *w_ori, uint32_t *h_ori, uint8_t *bitcount, uint8_t *bitcount_orig)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    uint8_t type_16bit = 0;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_get_image_info != NULL) {
            ret = m_fp_impl_handler->fp_test_get_image_info(w, h, max_size, w_ori, h_ori, bitcount, bitcount_orig, &type_16bit);
            if (ret >= 0) {
                silfp_bmp_set_img_16bit_type(type_16bit);
            }
        } else {
            LOG_MSG_DEBUG("No implement fp_test_get_image_info");
        }
    }
    return ret;
}

int32_t silfp_cmd_test_dump_data(uint32_t type, uint32_t step, void *buffer, uint32_t len, uint32_t *remain, uint32_t *size, uint32_t *w, uint32_t *h, uint8_t *bitcount)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_dump_data != NULL) {
            ret = m_fp_impl_handler->fp_test_dump_data(type, step, buffer, len, remain, size, w, h, bitcount);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_dump_data");
        }
    }
    return ret;
}

int32_t silfp_cmd_test_image_capture(uint32_t mode, void *buffer, uint32_t *len, uint8_t *quality, uint8_t *area, uint8_t *istpl, uint8_t *greyavg, uint8_t *greymax)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_image_capture != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_test_image_capture(mode, buffer, len, quality, area, istpl, greyavg, greymax);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_test_image_capture");
        }
    }
    return ret;
}

int32_t silfp_cmd_test_send_group_image(uint32_t orig, uint32_t frr, uint32_t imgtype, void *buffer, uint32_t *len)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_send_group_image != NULL) {
            ret = m_fp_impl_handler->fp_test_send_group_image(orig, frr, imgtype, buffer, len);
        } else {
            LOG_MSG_DEBUG("No implement fp_test_send_group_image");
        }
    }
    return ret;
}

int32_t silfp_cmd_test_image_finish(void)
{
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_image_finish != NULL) {
            m_fp_impl_handler->fp_test_image_finish();
        } else {
            LOG_MSG_DEBUG("No implement fp_test_image_finish");
        }
    }
    return SL_SUCCESS;
}

int32_t silfp_cmd_test_deadpx(uint32_t *result, uint32_t *deadpx, uint32_t *badline)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_deadpx != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_test_deadpx(result, deadpx, badline);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_test_deadpx");
        }
    }
    return ret;
}

int32_t silfp_cmd_test_speed(void *buffer, uint32_t *len)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_test_speed != NULL) {
            silfp_dev_enable();
            ret = m_fp_impl_handler->fp_test_speed(buffer, len);
            silfp_dev_disable();
        } else {
            LOG_MSG_DEBUG("No implement fp_test_speed");
        }
    }
    return ret;
}

int32_t silfp_cmd_sync_ta_log(int32_t enable)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    if (m_fp_impl_handler != NULL) {
        if (m_fp_impl_handler->fp_sync_ta_log != NULL) {
            ret = m_fp_impl_handler->fp_sync_ta_log(enable);
        } else {
            LOG_MSG_DEBUG("No implement fp_sync_ta_log");
        }
    }
    return ret;
}

int32_t silfp_cmd_check_broken(worker_state_t state)
{
    int32_t ret = SL_SUCCESS;
    int32_t optic = 0;
    uint8_t retry = 8;
    uint32_t result = 0;
    silfp_broken_info *binfo = NULL;
    uint32_t broken_info_len = sizeof(silfp_broken_info);

    binfo = (silfp_broken_info *)malloc(broken_info_len);
    if (NULL == binfo) {
        LOG_MSG_ERROR("binfo malloc failed !!!!!!!!!!!!!!");
        return ret;
    }
    memset(binfo, 0x0, broken_info_len);
    optic = silfp_impl_is_optic();
    if (!optic && (state == STATE_ESD_CHK||state == STATE_SCAN)) {//(state == STATE_NAV)
        LOG_MSG_INFO("silfp_cmd_check_broken enter ...state:%d",state);
        do {
            retry --;
            if (silfp_cmd_download_normal() < 0) {
                LOG_MSG_ERROR("silfp_cmd_download_normal fail");
            }
            if (m_fp_impl_handler != NULL) {
                if (m_fp_impl_handler->fp_check_broken != NULL) {
                    silfp_dev_enable();
                    ret = m_fp_impl_handler->fp_check_broken();
                    silfp_dev_disable();
                    silfp_dump_data(DUMP_IMG_OTHER_ORIG);
                    if (ret >= 0) {
                        LOG_MSG_DEBUG("check broken success, break ##################");
                        break;
                    } else {
                        LOG_MSG_DEBUG("check_broken failed... and power reset");
                        silfp_dev_pwdn(SIFP_PWDN_FLASH);
                        usleep(1000*100);
                    }
                } else {
                    LOG_MSG_DEBUG("No implement fp_check_broken");
                }
            }
            LOG_MSG_ERROR("check broken failed......retry times:%d",retry);
        } while (retry > 0);
        silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_GET_BROKEN_INFO, binfo, &broken_info_len, &result);
        silfp_notify_send_dcsmsg(SILFP_CHECK_RESULT, -ret, (7-retry), binfo);
    }
    if (binfo != NULL) {
        free(binfo);
        binfo = NULL;
    }
    return ret;
}