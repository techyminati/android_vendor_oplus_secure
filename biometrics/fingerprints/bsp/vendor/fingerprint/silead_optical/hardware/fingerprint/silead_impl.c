/******************************************************************************
 * @file   silead_impl.c
 * @brief  Contains CA implements.
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
 * David Wang  2018/4/2    0.1.0      Init version
 * David Wang  2018/5/15   0.1.1      Support get finger status
 * John Zhang  2018/5/15   0.1.2      Support load/save config
 * Jack Zhang  2018/5/17   0.1.3      Change test process to simplify app use
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * David Wang  2018/6/5    0.1.6      Support wakelock & pwdn
 * Rich Li     2018/6/7    0.1.7      Support dump image
 * Jack Zhang  2018/6/15   0.1.8      Add read OTP I/F.
 * Rich Li     2018/7/2    0.1.9      Add algo set param command ID.
 * Bangxiong.Wu 2019/2/24  0.2.1      Fix rapid lifting bug
 * Bangxiong.Wu 2019/3/10  0.2.2      Ensure enroll image quality
 * Bangxiong.Wu 2019/3/13  1.0.0      Move touch down notify to recv touch down from kernel
 * Bangxiong.Wu 2019/04/02 1.0.1      Move touch up notify to recv touch up from kernel
 * Bangxiong.Wu 2019/04/20 1.0.2      Add notify touch up for enroll
 * Bangxiong.Wu 2019/06/21 1.0.3      Add for sync tpl to tee
 *
 *****************************************************************************/

#define FILE_TAG "silead_impl"
#include "log/logmsg.h"

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_config.h"
#include "silead_xml.h"
#include "silead_cmd.h"
#include "silead_dev.h"
#include "silead_debug.h"
#include "silead_dump.h"
#include "silead_storage.h"
#include "silead_version.h"
#include "silead_cust.h"
#include "silead_worker.h"
#include "silead_cal.h"
#include "silead_impl.h"
#include "silead_ext.h"
#include "silead_util.h"
#include "silead_ext_skt.h"

// 0x0008 0x0010 0x0020 for cal, 0x0400 for dump ta log
#define FP_FEATURE_STORE_NORMAL_MASK 0x0001
#define FP_FEATURE_STORE_AUTH_UPDATE_MASK 0x0002
#define FP_FEATURE_NEED_REINIT_AFTER_IRQ_MASK 0x0004
#define FP_FEATURE_NEED_FINGER_LOOP_MASK 0x0040
#define FP_FEATURE_NEED_IRQ_PWDN_MASK 0x0080
#define FP_FEATURE_NEED_SHUTDOWN_MASK 0x0100
#define FP_FEATURE_OPTIC_MASK 0x0200

static uint32_t m_storage_normal = 0;
static uint32_t m_tpl_max_size = 0;
static uint32_t m_tpl_update_support = 0;
static uint32_t m_need_reinit_after_irq = 0;
static uint32_t m_need_finger_loop = 0;
static uint32_t m_need_finger_loop_bak = 0;
static uint32_t m_need_irq_pwdn = 0;

static uint32_t m_shutdown_by_avdd = 0;
static uint32_t m_is_optic = 0;

static int32_t m_need_wait_finger_up = 0;
static int32_t m_capture_dump_enable = 0;

static int64_t m_sec_user_id = 0;

static char m_str_ta_name[DEVNAME_LEN] = {0};

//Bangxiong.wu<2019-01-09>Add for hypnus
#if 0
static const char* action_info = "/sys/kernel/hypnus/action_info";
#define ACTION_IO 12
#define ACTION_TIMEOUT 1000
#define MAX_LEN 20

void sileadHypnusSetAction()
{
    char buf[MAX_LEN];
    int fd = open(action_info, O_WRONLY);
    if (fd < 0) {
        LOG_MSG_ERROR("SetAction open err :%d",fd);
        return;
    }
    sprintf(buf, "%d %d", ACTION_IO, ACTION_TIMEOUT);
    write(fd, buf, MAX_LEN);
    close(fd);
    LOG_MSG_DEBUG("Fingerprint SetAction for silead optical");
    return;
}
#endif
//Bangxiong.wu<2019-01-09>Add for hypnus

//add heng
fingerprint_tp_info_t m_pre_tp_touch_info;
fingerprint_tp_info_t m_later_tp_touch_info;
int32_t sl_copy_tp_touch_info(tp_touch_info_t *kernel_info,fingerprint_tp_info_t *user_info);
int32_t sl_assigning_tp_touch_info(tp_touch_info_t *tp_info,fingerprint_tp_info_t *fp_tp_info,uint8_t mode);

void silfp_impl_set_ta_name(const void *name, uint32_t len)
{
    int32_t ret = 0;

    ret = silfp_util_strcpy(m_str_ta_name, sizeof(m_str_ta_name), name, len);
    if (ret < 0) {
        memset(m_str_ta_name, 0, sizeof(m_str_ta_name));
    }
    LOG_MSG_VERBOSE("name = %s", m_str_ta_name);
}

void silfp_impl_set_capture_dump_flag(int32_t addition)
{
    if (IS_UI_DUMP_ENABLE(addition)) {
        m_capture_dump_enable = 1;
    } else if (IS_UI_DUMP_DISABLE(addition)) {
        m_capture_dump_enable = 0;
    }
}

int32_t silfp_impl_set_wait_finger_up_need(int32_t need)
{
    m_need_wait_finger_up = need;
    return 0;
}

int32_t silfp_impl_is_wait_finger_up_need(void)
{
    return m_need_wait_finger_up;
}

inline int32_t silfp_impl_get_screen_status(uint8_t *status)
{
    return silfp_dev_get_screen_status(status);
}

inline int32_t silfp_impl_set_screen_cb(screen_cb listen, void *param)
{
    return silfp_dev_set_screen_cb(listen, param);
}

int32_t silfp_impl_set_finger_status_mode(int32_t mode)
{
    return silfp_dev_set_finger_status_mode(mode);
}

inline void silfp_impl_wait_clean(void)
{
    silfp_dev_wait_clean();
}

inline void silfp_impl_cancel(void)
{
    silfp_dev_cancel();
}

void silfp_impl_sync_finger_status_optic(int32_t down)
{
    if (down) {
        silfp_dev_sync_finger_down();
    } else if (!down) {
        silfp_dev_sync_finger_up();
    }
}

static int32_t _impl_fp_dev_restart()
{
    // Reset 300ms
    silfp_dev_pwdn(SIFP_PWDN_FLASH);
    usleep(1000*300);

    return 0;
}

static int32_t _impl_need_reinit_after_irq(void)
{
    return m_need_reinit_after_irq;
}

static int32_t _impl_wait_finger_status(uint32_t status, int32_t irq, int32_t down, int32_t up, int32_t cancel)
{
    int32_t ret = 0;
    int16_t retry = 8;
    int32_t charge_state = silfp_cust_get_sys_charging_state();

    LOG_MSG_VERBOSE("wait finger status %d(%d:%d:%d:%d)", status, irq, down, up, cancel);

    do {
        if (m_need_irq_pwdn) {
            silfp_impl_chip_pwdn();
        } else {
            silfp_impl_download_normal();
            ret = silfp_cmd_wait_finger_status(status, charge_state);
            retry --;
            if ((ret == -SL_ERROR_INT_INVALID) && (retry > 0)) {
                _impl_fp_dev_restart();
                continue;
            }
        }

        if ((ret < 0) && (ret != -SL_ERROR_INT_INVALID)) {
            return ret;
        }

        silfp_dev_wait_clean();
        if (silfp_worker_is_canceled() && cancel) {
            ret = -SL_ERROR_CANCELED;
            break;
        } else {
            ret = silfp_dev_wait_finger_status(irq, down, up, cancel);
        }

        if (_impl_need_reinit_after_irq() && (status == IRQ_DOWN || status == IRQ_NAV) && (ret != -SL_ERROR_CANCELED)) {
            silfp_cust_finger_down_pre_action();
            silfp_impl_download_normal();
        } else if (!_impl_need_reinit_after_irq() && (ret != -SL_ERROR_CANCELED)) {
            ret = silfp_cmd_check_esd();
            if ((ret == -SL_ERROR_DETECTED_ESD) && (retry > 0)) {
                _impl_fp_dev_restart();
                continue;
            }
        }
    } while (((ret == -SL_ERROR_DETECTED_ESD) || (ret == -SL_ERROR_INT_INVALID)) && (retry > 0));

    if ((ret != -SL_ERROR_CANCELED) && (ret != -SL_ERROR_DETECTED_ESD)) {
        if (status == IRQ_DOWN || status == IRQ_NAV) {
            silfp_cal_update_cfg(FP_CONFIG_DOWN_BIT, 0);
        } else {
            silfp_cal_update_cfg(FP_CONFIG_UP_BIT, 0);
        }
    }
    return ret;
}

int32_t silfp_impl_wait_finger_down(void)
{
    int ret = 0;
    ret = _impl_wait_finger_status(IRQ_DOWN, 0, 1, 0, 0);

    return ret;
}

int32_t silfp_impl_wait_finger_down_with_cancel(void)
{
    int32_t ret = 0;
    ret = _impl_wait_finger_status(IRQ_DOWN, 0, 1, 0, 1);

    return ret;
}

int32_t silfp_impl_wait_finger_up(void)
{
    int32_t ret = 0;
    ret = _impl_wait_finger_status(IRQ_UP, 0, 0, 1, 0);
    if (ret >= 0) {
        silfp_impl_set_wait_finger_up_need(0);
    }

    return ret;
}

int32_t silfp_impl_wait_finger_up_with_cancel(void)
{
    int32_t ret = 0;
    if (silfp_impl_is_finger_down()) {
        ret = _impl_wait_finger_status(IRQ_UP, 0, 0, 1, 1);
    }
    if (ret >= 0) {
        silfp_impl_set_wait_finger_up_need(0);
    }

    return ret;
}

int32_t silfp_impl_wait_finger_up_with_enroll(void)
{
    int32_t ret = 0;
    if (silfp_impl_is_finger_down()) {
        ret = _impl_wait_finger_status(IRQ_UP, 0, 0, 1, 1);
    }
    if (ret >= 0) {
        silfp_cust_finger_up_after_action();
        silfp_impl_set_wait_finger_up_need(0);
    }

    return ret;
}

int32_t silfp_impl_wait_finger_nav(void)
{
    int32_t ret = 0;

    ret = _impl_wait_finger_status(IRQ_NAV, 0, 1, 0, 1);

    return ret;
}

int32_t silfp_impl_wait_irq_with_cancel(void)
{
    int32_t ret = 0;

    ret = _impl_wait_finger_status(IRQ_ESD, 1, 0, 0, 1);

    return ret;
}

//add heng
int32_t sl_init_tp_touch_info()
{
    memset(&m_later_tp_touch_info, 0, sizeof(m_later_tp_touch_info));
    memset(&m_pre_tp_touch_info, 0, sizeof(m_pre_tp_touch_info));
    return 0;
}

int32_t sl_get_tp_touch_info(uint8_t mode)
{
    int32_t ret = -1;
    int32_t touch_state;
    fingerprint_tp_info_t m_fingerprint_tp_info;
    tp_touch_info_t m_tp_touch_info;
    uint32_t len = sizeof(m_fingerprint_tp_info);
    LOG_MSG_DEBUG("sl_get_tp_touch_info start mode= %d\n",mode);
    ret = silfp_dev_get_tp_touch_info( &m_tp_touch_info);

    if(ret<0){
            LOG_MSG_ERROR("get tp touch info failed!");
            return ret;
    }

    sl_assigning_tp_touch_info(&m_tp_touch_info,&m_fingerprint_tp_info,mode);

    silfp_cmd_send_cmd_with_buf(1, &m_fingerprint_tp_info, len);
	
	
    touch_state = m_tp_touch_info.touch_state;
	LOG_MSG_DEBUG("sl_get_tp_touch_info start mode= %d,touch_state = %d\n",mode,touch_state);
    return touch_state;
}

int32_t sl_assigning_tp_touch_info(tp_touch_info_t *tp_info,fingerprint_tp_info_t *fp_tp_info,uint8_t mode)
{
    fp_tp_info->image_state = mode;
    fp_tp_info->touch_state = tp_info->touch_state;
    fp_tp_info->touch_positon_x = tp_info->touch_positon_x;
    fp_tp_info->touch_positon_y = tp_info->touch_positon_y;
    fp_tp_info->touch_coverage = tp_info->coverrage_state;
    LOG_MSG_DEBUG("later_tp_touch:state=%d,pos_x=%d,pos_y=%d,coverage:%d,img_state=%d",fp_tp_info->touch_state, fp_tp_info->touch_positon_x, fp_tp_info->touch_positon_y, fp_tp_info->touch_coverage,fp_tp_info->image_state);
    if(0==mode){
        m_pre_tp_touch_info = *fp_tp_info;
    }else if(1==mode){
        m_later_tp_touch_info = *fp_tp_info;
    }
    return 0;
}

int32_t sl_copy_tp_touch_info(tp_touch_info_t *kernel_info,fingerprint_tp_info_t *user_info)
{
    user_info->touch_state = kernel_info->touch_state;
    user_info->touch_positon_x = kernel_info->touch_positon_x;
    user_info->touch_positon_y = kernel_info->touch_positon_y;
    user_info->touch_coverage = kernel_info->coverrage_state;
    LOG_MSG_DEBUG("touch_state: %d, pos_x: %d, pos_y:%d, coverage:%d.",
        user_info->touch_state, user_info->touch_positon_x, user_info->touch_positon_y,user_info->touch_coverage);
    return 0;
}

//add heng

static int32_t _impl_get_finger_status(uint32_t *status)
{
    int32_t ret = -SL_ERROR_TA_OPEN_FAILED;
    uint32_t addition = 0;

    ret = silfp_cmd_get_finger_status(status, &addition);
    if (ret == -SL_ERROR_DETECTED_ESD) {
        _impl_fp_dev_restart();
        silfp_impl_download_normal();
    }

    if (ret >= 0) {
        if (status) {
            if (*status > 0) {
                silfp_cal_update_cfg(FP_CONFIG_DOWN_BIT, 0);
            } else if (*status == 0) {
                silfp_cal_update_cfg(FP_CONFIG_UP_BIT, 0);
            }
        }
        if (addition & 0x40000000) { // update finish
            m_need_finger_loop = 0;
        }
        if (addition & 0x80000000) { // need save
            LOG_MSG_DEBUG("addition = 0x%x", addition);
            silfp_cal_update_cfg(FP_CONFIG_CHG_BIT, 1);
        }
    }

    return ret;
}

int32_t silfp_impl_get_finger_down_with_cancel(void)
{
    int32_t ret = -SL_ERROR_CANCELED;
    uint32_t status = 0;

    if (m_need_finger_loop) {
        silfp_impl_download_normal();

        while (!silfp_worker_is_canceled()) {
            ret = _impl_get_finger_status(&status);
            if (ret < 0) {
                break;
            }

            if (status > 0) {
                silfp_cust_finger_down_after_action();
                break;
            } else {
                ret = -SL_ERROR_CANCELED;
            }
        }
    } else {
        ret = silfp_impl_wait_finger_down_with_cancel();
    }

    return ret;
}

int32_t silfp_impl_is_finger_down(void)
{
    return silfp_dev_is_finger_down();
}

int32_t silfp_impl_capture_image(int32_t type, uint32_t step)
{
    int32_t ret = 0;

    if (silfp_impl_is_optic()) {
        sl_init_tp_touch_info();
        silfp_cust_capture_get_tp_info(0);

        ret = silfp_impl_capture_image_pre();
        if (ret < 0) {
            return ret;
        }

        if(!silfp_impl_is_finger_down() && 0 == step){
            ret = -SL_ERROR_MOVE_TOO_FAST;
            LOG_MSG_INFO("finger up before capture image in step %d", step);
            return ret;
       }

        ret = silfp_impl_capture_image_raw(type, step);
        silfp_dump_data((type == IMG_CAPTURE_ENROLL) ? DUMP_IMG_ENROLL_ORIG : ((type == IMG_CAPTURE_AUTH) ? DUMP_IMG_AUTH_ORIG : DUMP_IMG_OTHER_ORIG));
        if (m_capture_dump_enable && ((type == IMG_CAPTURE_ENROLL) || (type == IMG_CAPTURE_AUTH))) {
            silfp_ext_skt_capture_dump(1, (type == IMG_CAPTURE_ENROLL) ? 1 : 0, step, 0);
        }
        if (ret < 0) {
            return ret;
        }

        if(IMG_CAPTURE_ENROLL == type) {
            usleep(1000 * 100); //100ms
        }

        silfp_cust_capture_get_tp_info(1);

        if(IMG_CAPTURE_ENROLL == type && (0 == silfp_impl_is_finger_down())) {
            ret = -SL_ERROR_MOVE_TOO_FAST;
            LOG_MSG_DEBUG("finger up, enroll: err(%d)", ret);
        }

        ret = silfp_impl_capture_image_after(type, step);
    } else {
        ret = silfp_cmd_capture_image(type, step);
    }

    if (ret < 0) {
        silfp_dump_data(DUMP_IMG_SHOT_FAIL);
    }
    if (m_capture_dump_enable && ((type == IMG_CAPTURE_ENROLL) || (type == IMG_CAPTURE_AUTH))) {
        silfp_ext_skt_capture_dump(0, (type == IMG_CAPTURE_ENROLL) ? 1 : 0, step, ret);
    }

    return ret;
}

int32_t silfp_impl_nav_capture_image(void)
{
    int32_t ret = 0;

    ret = silfp_cmd_nav_capture_image();
    silfp_dump_data((ret >=0) ? DUMP_IMG_NAV_SUCC : DUMP_IMG_NAV_FAIL);

    return ret;
}

inline int32_t silfp_impl_auth_start(void)
{
    return silfp_cmd_auth_start();
}

inline int32_t silfp_impl_auth_step(uint64_t op_id, uint32_t step, uint32_t is_pay, uint32_t *fid)
{
    return silfp_cmd_auth_step(op_id, step, is_pay, fid);
}

static int32_t _impl_auth_tpl_upd_normal(void)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = m_tpl_max_size;
    uint32_t fid = 0;

    if (m_storage_normal && m_tpl_update_support) {
        do {
            buf = malloc(len);
            if (buf == NULL) {
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(buf, 0, len);
            ret = silfp_cmd_update_template(buf, &len, &fid);
        } while (0);

        if (ret >= 0 && len > 0 && len <= m_tpl_max_size) {
            silfp_storage_update(fid, buf, len);
        }
        free(buf);
        buf = NULL;
    }
    return ret;
}

inline int32_t silfp_impl_auth_end(void)
{
    int32_t ret = 0;

    _impl_auth_tpl_upd_normal();
    ret = silfp_cmd_auth_end();

    return ret;
}

inline int32_t silfp_impl_get_enroll_num(uint32_t *num)
{
    return silfp_cmd_get_enroll_num(num);
}

inline int32_t silfp_impl_enroll_start(void)
{
    return silfp_cmd_enroll_start();
}

inline int32_t silfp_impl_enroll_step(uint32_t *remaining)
{
    return silfp_cmd_enroll_step(remaining);
}

static int32_t _impl_enroll_tpl_save_normal(uint32_t *fid)
{
    int32_t ret = 0;
    void *buf = NULL;
    uint32_t len = m_tpl_max_size;

    if (m_storage_normal) {
        do {
            buf = malloc(len);
            if (buf == NULL) {
                ret = -SL_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(buf, 0, len);
            ret = silfp_cmd_save_template(buf, &len);
        } while (0);

        if (ret >= 0 && len > 0 && len <= m_tpl_max_size) {
            ret = silfp_storage_save(buf, len, m_sec_user_id, fid);
        } else {
            ret = -SL_ERROR_STO_OP_FAILED;
        }

        if (buf != NULL) {
            free (buf);
        }
    }
    return ret;
}

int32_t silfp_impl_enroll_end(uint32_t *fid)
{
    int32_t ret = 0;
    int32_t status = -1;

    silfp_dump_data(DUMP_IMG_ENROLL_NEW);

    ret = _impl_enroll_tpl_save_normal(fid);
    if (ret >= 0) {
        status = 0; // save ok or no normal storage
    } else {
        status = -1; // save failed
    }

    ret = silfp_cmd_enroll_end(status, fid);
    if (ret >= 0 && status < 0) {
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

inline int32_t silfp_impl_nav_support(uint32_t *type)
{
    return silfp_cmd_nav_support(type);
}

inline int32_t silfp_impl_nav_start(void)
{
    int32_t ret = 0;

    if (silfp_cal_need_nav_cal()) {
        silfp_impl_download_normal();
    }

    ret = silfp_cmd_nav_start();

    return ret;
}

inline int32_t silfp_impl_nav_step(uint32_t *key)
{
    int32_t ret = 0;

    ret = silfp_cmd_nav_step(key);
    silfp_dump_data((ret >=0) ? DUMP_IMG_NAV_SUCC : DUMP_IMG_NAV_FAIL);

    return ret;
}

inline int32_t silfp_impl_nav_end(void)
{
    return silfp_cmd_nav_end();
}

inline int32_t silfp_impl_send_key(uint32_t key)
{
    return silfp_dev_send_key(key);
}

inline int32_t silfp_impl_nav_set_mode(uint32_t mode)
{
    return silfp_cmd_set_nav_mode(mode);
}

inline int32_t silfp_impl_download_normal()
{
    return silfp_cmd_download_normal();
}

static int32_t _impl_init2()
{
    int32_t ret = 0;
    uint32_t feature = 0;
    uint32_t algoVer = 0;
    uint32_t taVer = 0;

    char device_info[] = "silead@fp";
    uint32_t len = strlen(device_info);

    ret = silfp_cmd_init2(device_info, len, &feature, &m_tpl_max_size);
    if (ret >= 0) {
        m_storage_normal = !!(feature & FP_FEATURE_STORE_NORMAL_MASK);
        m_tpl_update_support = !!(feature & FP_FEATURE_STORE_AUTH_UPDATE_MASK);
        m_need_reinit_after_irq = !!(feature & FP_FEATURE_NEED_REINIT_AFTER_IRQ_MASK);
        m_need_finger_loop = !!(feature & FP_FEATURE_NEED_FINGER_LOOP_MASK);
        m_need_finger_loop_bak = m_need_finger_loop;
        m_need_irq_pwdn = !!(feature & FP_FEATURE_NEED_IRQ_PWDN_MASK);
        m_shutdown_by_avdd = !!(feature & FP_FEATURE_NEED_SHUTDOWN_MASK);
        m_is_optic = !!(feature & FP_FEATURE_OPTIC_MASK);
        silfp_cal_init(feature, silfp_impl_is_optic());
        silfp_log_dump_set_feature(feature);
    }

    // maybe some ta miss the value, should not happend, just in case
    if (m_storage_normal && m_tpl_max_size == 0) {
        m_tpl_max_size = (500 * 1024);
    }

    silfp_cmd_get_ta_ver(&algoVer, &taVer);
    LOG_MSG_INFO("ta version:v%d, algo version: v%d", taVer, algoVer);

    silfp_impl_get_otp();

    LOG_MSG_VERBOSE("m_storage_normal=%d, m_tpl_max_size=%d, m_need_reinit_after_irq=%d", m_storage_normal, m_tpl_max_size, m_need_reinit_after_irq);
    LOG_MSG_VERBOSE("finger loop=%d, avdd=%d, optic=%d", m_need_finger_loop, m_shutdown_by_avdd, m_is_optic);

    silfp_log_dump_ta_log();

    return ret;
}

static int32_t _impl_update_cfg(const uint32_t chipid, const uint32_t subid, const uint32_t virtualid)
{
    int32_t ret = 0;

    void *buffer = NULL;
    int32_t len = 0;

    cf_set_t *pconfig = NULL;

    pconfig = silfp_cfg_malloc();
    if (pconfig == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    pconfig->common.id = chipid;
    pconfig->common.sid = subid;
    pconfig->common.vid = virtualid;

    do {
        ret = silfp_xml_get_sysparams(pconfig);
        if (ret < 0) {
            ret = 0;
            break;
        }

        len = silfp_cfg_get_update_length(pconfig);
        if (len <= 0) {
            ret = 0;
            break;
        }

        buffer = malloc(len);
        if (buffer == NULL) {
            LOG_MSG_ERROR("allocation failed");
            ret = -SL_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(buffer, 0, len);
        if(len != silfp_cfg_get_update_buffer(buffer, len, pconfig)) {
            ret= -SL_ERROR_BAD_PARAMS;
            break;
        }

        ret = silfp_cmd_update_cfg(buffer, len);
    } while(0);

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    silfp_cfg_free(pconfig);
    pconfig = NULL;

    return ret;
}

static int32_t _impl_init()
{
    int32_t ret = 0;

    uint32_t chipid = 0;
    uint32_t subid = 0;
    uint32_t vid = 0;
    uint32_t update_cfg = 0;

    fp_dev_conf_t dev_init;
    char name[32] = {0};

    memset(&dev_init, 0, sizeof(dev_init));
    ret = silfp_dev_init(&dev_init);
    if (ret < 0) {
        LOG_MSG_ERROR("init dev fail");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (silfp_dev_hw_reset(0) < 0) {
        LOG_MSG_ERROR("reset fingerprint chip fail");
    }

    if (m_str_ta_name[0] != '\0') {
        silfp_cmd_set_env(m_str_ta_name);
    } else {
        silfp_cmd_set_env((dev_init.ta[0] != '\0') ? dev_init.ta : NULL);
    }

    silfp_dbg_update_all_log_level();

    do {
        ret = silfp_cmd_init(&dev_init, &chipid, &subid, &vid, &update_cfg);
        LOG_MSG_VERBOSE("chipid=%x,%x,%x %d (%d)", chipid, subid, vid, update_cfg, ret);
        if (ret < 0) {
            break;
        }

        if (update_cfg) {
            _impl_update_cfg(chipid, subid, vid);
        }

        ret = _impl_init2();
    } while (0);

    if (ret >= 0) {
        snprintf(name, sizeof(name), "gsl%04x\n", (chipid >> 16));
        silfp_dev_create_proc_node(name);
    }

    return ret;
}

int32_t silfp_impl_init()
{
    int32_t ret = 0;
    int32_t i = 0;
    const int32_t count = 3;

#ifdef GIT_SHA1_ID
    LOG_MSG_INFO("fp hal version: %s, tag: %s", FP_HAL_VERSION, GIT_SHA1_ID);
#else
    LOG_MSG_INFO("fp hal version: %s", FP_HAL_VERSION);
#endif

    for (i = 0; i < count; i++) {
        ret = _impl_init();
        if (ret >= 0) {
            break;
        }
        usleep(1000*300);
    }

    return ret;
}

int32_t silfp_impl_deinit(void)
{
    silfp_dev_deinit();
    silfp_cmd_deinit();

    if (m_storage_normal) {
        silfp_storage_release();
    }
    silfp_dump_deinit();
    silfp_cal_deinit();

    LOG_MSG_VERBOSE("close");
    return SL_SUCCESS;
}

int32_t silfp_impl_set_gid(uint32_t gid)
{
    int32_t ret = 0;
    uint32_t serial = 0xFDCA;

    LOG_MSG_DEBUG("serial = 0x%x", serial);

    ret = silfp_cmd_set_gid(gid, serial);

    return ret;
}

static int32_t _impl_load_user_db_sync_to_ta(const char *db_path)
{
    int32_t ret = 0;

    void *buf = NULL;
    uint8_t *pdata = NULL;
    const uint32_t buf_len = m_tpl_max_size;
    uint32_t tpl_len = 0;
    int32_t load_count = 0;

    int64_t sid = m_sec_user_id;
    uint32_t id[TPL_MAX_ST] = {0};
    int32_t id_count = 0;
    int32_t i = 0;

    buf = malloc(buf_len + sizeof(sid));
    if (buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    silfp_storage_set_tpl_path(db_path);
    pdata = (uint8_t *)buf;
    pdata += sizeof(sid);

    id_count = silfp_storage_get_idlist(id, 1);
    for (i = 0; i < id_count; i++) {
        memset(buf, 0, buf_len);
        ret = silfp_storage_load(id[i], (void *)pdata, buf_len);
        if (ret >= 0) {
            sid = silfp_storage_get_sec_id(id[i]);
            memcpy(buf, &sid, sizeof(sid));
            tpl_len = ret;
            if (tpl_len <= buf_len) {
                ret = silfp_cmd_load_template(id[i], buf, tpl_len + sizeof(sid));
            } else {
                LOG_MSG_ERROR("tpl size failed (%d)", tpl_len);
                ret = -SL_ERROR_TEMPLATE_INVALID;
            }

            silfp_storage_remove(id[i]);
        }
    }

    pdata = (uint8_t *)buf;
    pdata[0] = 0xFF;
    pdata[1] = 0xFF;
    silfp_cmd_load_template(0, buf, 2);
    silfp_storage_remove_all();

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return load_count;
}


static int32_t _impl_load_user_db_normal(const char *db_path)
{
    int32_t ret = 0;

    void *buf = NULL;
    const uint32_t buf_len = m_tpl_max_size;
    uint32_t tpl_len = 0;
    int32_t load_count = 0;

    uint32_t id[TPL_MAX_ST] = {0};
    int32_t id_count = 0;
    int32_t i = 0;

    buf = malloc(buf_len);
    if (buf == NULL) {
        return -SL_ERROR_OUT_OF_MEMORY;
    }

    silfp_storage_set_tpl_path(db_path);

    id_count = silfp_storage_get_idlist(id, 1);
    for (i = 0; i < id_count; i ++) {
        memset(buf, 0, buf_len);
        ret = silfp_storage_load(id[i], buf, buf_len);
        if (ret >= 0) {
            tpl_len = ret;
            if (tpl_len <= buf_len) {
                ret = silfp_cmd_load_template(id[i], buf, tpl_len);
            } else {
                LOG_MSG_ERROR("tpl size failed (%d)", tpl_len);
                ret = -SL_ERROR_TEMPLATE_INVALID;
            }

            if (ret == -SL_ERROR_TEMPLATE_INVALID) {
                silfp_storage_inc_fail_count(id[i]);
            } else if (ret >= 0) {
                load_count++;
            }
        }
    }

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return load_count;
}

int32_t silfp_impl_load_user_db(const char *db_path)
{
    int32_t ret = 0;
    uint32_t sync_ca_tpl = 0;

    ret = silfp_cmd_load_user_db(db_path, &sync_ca_tpl);
    if (ret < 0) {
        LOG_MSG_ERROR("load user db %s failed (%d)", db_path, ret);
        return -SL_ERROR_STO_OP_FAILED;
    }

    if (m_storage_normal) {
        LOG_MSG_DEBUG("load user db by normal, m_storage_normal = %d", m_storage_normal);
        ret = _impl_load_user_db_normal(db_path);
    } else if (sync_ca_tpl) {
        LOG_MSG_DEBUG("move user db to tee, sync_ca_tpl = %d", sync_ca_tpl);
        ret = _impl_load_user_db_sync_to_ta(db_path);
    }

    return ret;
}

int32_t silfp_impl_remove_finger(uint32_t fid)
{
    int32_t ret = 0;

    if (m_storage_normal) {
        ret = silfp_storage_remove(fid);
        if (ret < 0) {
            return ret;
        }
    }

    ret = silfp_cmd_remove_finger(fid);

    return ret;
}

inline int32_t silfp_impl_get_db_count(void)
{
    return silfp_cmd_get_db_count();
}

inline int32_t silfp_impl_get_finger_prints(uint32_t *ids, uint32_t count)
{
    return silfp_cmd_get_finger_prints(ids, count);
}

inline int64_t silfp_impl_load_enroll_challenge(void)
{
    return silfp_cmd_load_enroll_challenge();
}

inline int32_t silfp_impl_set_enroll_challenge(uint64_t challenge)
{
    return silfp_cmd_set_enroll_challenge(challenge);
}

inline int32_t silfp_impl_verify_enroll_challenge(const void *hat, uint32_t size, int64_t sid)
{
    m_sec_user_id = sid;
    return silfp_cmd_verify_enroll_challenge(hat, size);
}

inline int64_t silfp_impl_load_auth_id(void)
{
    return silfp_cmd_load_auth_id();
}

inline int32_t silfp_impl_get_hw_auth_obj(void *buffer, uint32_t length)
{
    return silfp_cmd_get_hw_auth_obj(buffer, length);
}

inline int64_t silfp_impl_get_sec_id(uint32_t fid)
{
    return silfp_storage_get_sec_id(fid);
}

inline int32_t silfp_impl_capture_image_pre(void)
{
    return silfp_cmd_capture_image_pre();
}

inline int32_t silfp_impl_capture_image_raw(int32_t type, uint32_t step)
{
    return silfp_cmd_capture_image_raw(type, step);
}

inline int32_t silfp_impl_capture_image_after(int32_t type, uint32_t step)
{
    return silfp_cmd_capture_image_after(type, step);
}

inline int32_t silfp_impl_set_touch_info(void *buffer, uint32_t len)
{
    return silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_TOUCH_INFO, buffer, len);
}

int32_t silfp_impl_get_otp(void)
{
    int32_t ret = 0;
    uint32_t otp[3] = {0};

    ret = silfp_cmd_get_otp(&otp[0], &otp[1], &otp[2]);
    if (ret >= 0) {
        LOG_MSG_INFO("OTP 0x%08X 0x%08X 0x%08X", otp[0], otp[1], otp[2]);
    }

    return ret;
}

int32_t silfp_impl_chip_pwdn(void)
{
    int32_t ret = 0;
    uint8_t onoff = 0;

    silfp_dev_pwdn(m_shutdown_by_avdd ? SIFP_PWDN_POWEROFF : SIFP_PWDN_NONE);
    ret = silfp_cmd_send_cmd_with_buf(REQ_CMD_SUB_ID_SPI_CTRL, &onoff, sizeof(onoff));
    //silfp_cmd_deep_sleep_mode(); //????

    return ret;
}

inline int32_t silfp_impl_wakelock(uint8_t lock)
{
    return silfp_dev_wakelock(lock);
}

int32_t silfp_impl_is_optic(void)
{
    int32_t ret = silfp_cust_is_optic();
    if (ret < 0) {
        ret = m_is_optic;
    }
    return ret;
}

int32_t silfp_impl_calibrate(void)
{
    int32_t ret = silfp_cal_calibrate();
    silfp_log_dump_ta_log();
    return ret;
}

int32_t silfp_impl_cal_base_sum(void)
{
    return silfp_cal_base_sum();
}

int32_t silfp_impl_cal_step(uint32_t step)
{
    int32_t ret = silfp_cal_step(step);
    silfp_log_dump_ta_log();
    return ret;
}

int32_t silfp_impl_cal_reset(void)
{
    int32_t ret = silfp_cal_reset();
    if (ret >= 0) {
        m_need_finger_loop = m_need_finger_loop_bak;
    }
    return 0;
}

void silfp_impl_cal_set_path(const void *path, uint32_t len)
{
    silfp_cal_set_path(path, len);
}
