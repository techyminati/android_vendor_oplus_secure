/******************************************************************************
 * @file   silead_dev.c
 * @brief  Contains /dev/silead_fp operate functions.
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
 * Jack Zhang  2018/4/2    0.1.0      Init version
 * David Wang  2018/5/28   0.1.1      Support poll/read if netlink id invalid
 * David Wang  2018/6/5    0.1.2      Support wakelock & pwdn
 *
 *****************************************************************************/

#define FILE_TAG "silead_dev"
#include "log/logmsg.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "silead_const.h"
#include "silead_msg.h"
#include "silead_netlink.h"
#include "silead_dev.h"
#include "silead_error.h"
#include "silead_key.h"
#include "silead_util.h"

#define ENOENT 2

#define SIL_CLEAR_IRQ_ENABLED

static int32_t m_res_inited = 0;
static fp_dev_conf_t m_dev_conf;
static int32_t m_need_spi_op = 1;
static int32_t m_irq_op_mode = 0;
static int32_t m_dev_fd = -1;

static int32_t m_dev_stub = -1;

int32_t silfp_dev_get_ver(char *ver, uint32_t len)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (ver && len >= 10) {
        memset(ver, 0, len);
        ret = ioctl(m_dev_fd, SIFP_IOC_GET_VER, ver);
        if (ret < 0) {
            ret = -SL_ERROR_DEV_IOCTL_FAILED;
        }
    }

    return ret;
}

int32_t silfp_dev_enable(void)
{
    int32_t ret = 0;

    if (!m_need_spi_op) {
        return 0;
    }

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    LOG_MSG_VERBOSE("acq spi");
    ret = ioctl(m_dev_fd, SIFP_IOC_ACQ_SPI, NULL);
    if (ret < 0) {
        if (ENOENT == errno) {
            LOG_MSG_INFO("dev_spi_op not supported!");
            m_need_spi_op = 0;
            ret = 0;
        } else {
            ret = -SL_ERROR_DEV_IOCTL_FAILED;
        }
    }

    return ret;
}

int32_t silfp_dev_disable(void)
{
    int32_t ret = 0;

    if (!m_need_spi_op) {
        return 0;
    }

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    LOG_MSG_VERBOSE("release spi");

    ret = ioctl(m_dev_fd, SIFP_IOC_RLS_SPI, NULL);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

static void _dev_stub_deinit(void)
{
    int32_t ret = -1;

    if (m_dev_stub >= 0) {
        if (ioctl(m_dev_stub, SIL_STUB_IOC_DEINIT, NULL) < 0) {
            LOG_MSG_VERBOSE("SIL_STUB_IOC_DEINIT failed");
            ret = -SL_ERROR_DEV_IOCTL_FAILED;
        }

        close(m_dev_stub);
        m_dev_stub = -1;
    }
}

static int32_t _dev_stub_init(void)
{
    int32_t ret = 0;

    if (m_dev_stub >= 0) {
        return ret;
    }

    do {
        m_dev_stub = open(SIL_STUB_DEV, O_RDWR);
        if (m_dev_stub < 0) {
            if (errno != 2) { //errno = 2,means no such file or directory
                LOG_MSG_ERROR("open dev:%s (%d:%s)", SIL_STUB_DEV, errno, strerror(errno));
                ret = -SL_ERROR_DEV_OPEN_FAILED;
            } else {
                LOG_MSG_ERROR("open dev:%s fail (%s), directly use silead_fp dev", SIL_STUB_DEV, strerror(errno));
                ret = 0;
            }
            break;
        }

        ret = ioctl(m_dev_stub, SIL_STUB_IOC_INIT, NULL);
        if (ret < 0) {
            LOG_MSG_ERROR("ioctl :%s fail (%d:%s)", SIL_STUB_DEV, errno, strerror(errno));
            close(m_dev_stub);
            m_dev_stub = -1;
            ret = -SL_ERROR_DEV_IOCTL_FAILED;
            break;
        }
    } while(0);

    return ret;
}

int32_t silfp_dev_init(fp_dev_conf_t *t)
{
    int32_t ret = 0;
    char ver[11] = {0};

    LOG_MSG_VERBOSE("init");

    ret = _dev_stub_init();
    if(ret < 0) {
        LOG_MSG_ERROR("dev stub:%s fail", SIL_FP_SENSOR_DEVICE);
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (m_dev_fd < 0) {
        memset(&m_dev_conf, 0, sizeof(m_dev_conf));
        m_dev_fd = open(SIL_FP_SENSOR_DEVICE, O_RDWR);
    }
    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("FP Device %s Open Failed (%d:%s)", SIL_FP_SENSOR_DEVICE, errno, strerror(errno));
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    if (m_res_inited) {
        if (t != NULL) {
            memcpy(t, &m_dev_conf, sizeof(m_dev_conf));
        }
        return 1;
    }

    if (silfp_dev_get_ver(ver, sizeof(ver)) >= 0) {
        LOG_MSG_INFO("dev_ver version: %s", ver);
    }

    //ret = ioctl(m_dev_fd, SIFP_IOC_DEINIT, NULL);
    ret = ioctl(m_dev_fd, SIFP_IOC_INIT, t);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    if (ret >= 0 && !m_res_inited && t != NULL) {
        memcpy(&m_dev_conf, t, sizeof(m_dev_conf));
        silfp_msg_init();
        silfp_nl_init(t->nl_id, m_dev_fd);
        m_res_inited = 1;
    }

    return ret;
}

int32_t silfp_dev_deinit(void)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        _dev_stub_deinit();
        return 0;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_DEINIT, NULL);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    silfp_dev_disable();
    silfp_nl_deinit();
    silfp_msg_deinit();
    m_res_inited = 0;

    close(m_dev_fd);
    m_dev_fd = -1;

    _dev_stub_deinit();
    LOG_MSG_VERBOSE("deinit");

    return ret;
}

int32_t silfp_dev_hw_reset(uint8_t delayms)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_RESET, &delayms);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

static int32_t _dev_enable_irq(void)
{
    int32_t ret = 0;

    if (SIFP_FINGER_STATUS_IRQ != m_irq_op_mode) {
        return 0;
    }

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_ENABLE_IRQ, NULL);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

static int32_t _dev_disable_irq(void)
{
    int32_t ret = 0;

    if (SIFP_FINGER_STATUS_IRQ != m_irq_op_mode) {
        return 0;
    }

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_DISABLE_IRQ, NULL);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_clear_irq(void)
{
#ifdef SIL_CLEAR_IRQ_ENABLED
    LOG_MSG_VERBOSE("clear irq");

    _dev_enable_irq();
    usleep(1000*5);
    _dev_disable_irq();
#endif

    return SL_SUCCESS;
}

int32_t silfp_dev_get_screen_status(uint8_t *status)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_SCR_STATUS, status);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_set_screen_cb(screen_cb listen, void *param)
{
    return silfp_nl_set_screen_cb(listen, param);
}

int32_t silfp_dev_set_finger_status_mode(int32_t mode)
{
    m_irq_op_mode = mode;
    LOG_MSG_INFO("irq mode: %d", m_irq_op_mode);
    return silfp_nl_set_finger_status_mode(mode);
}

int32_t silfp_dev_wait_finger_status(int32_t irq, int32_t down, int32_t up, int32_t cancel)
{
    int32_t type = 0;

    if (m_irq_op_mode == SIFP_FINGER_STATUS_ANDROID) {
        if ((silfp_msg_is_finger_down() && down) || (!silfp_msg_is_finger_down() && up)) {
            return SL_SUCCESS;
        }
    }

    if (_dev_enable_irq() < 0) {
        LOG_MSG_ERROR("IOC_ENABLE_IRQ fail");
    }

    type = silfp_msg_wait_finger_status(irq, down, up, cancel);
    LOG_MSG_VERBOSE("wait_finger_status: %d", type);

    if (_dev_disable_irq() < 0) {
        LOG_MSG_ERROR("IOC_DISABLE_IRQ fail");
    }

    if (type == SIFP_MSG_CANCEL) {
        return -SL_ERROR_CANCELED;
    }

    return SL_SUCCESS;
}

void silfp_dev_wait_clean(void)
{
    silfp_msg_clean();
}

void silfp_dev_cancel(void)
{
    if (m_res_inited) {
        silfp_msg_send(SIFP_MSG_CANCEL);
    }
}

void silfp_dev_sync_finger_down(void)
{
    if (m_res_inited) {
        LOG_MSG_DEBUG("sync finger down");
        silfp_msg_send(SIFP_MSG_DOWN);
    }
}

void silfp_dev_sync_finger_up(void)
{
    if (m_res_inited) {
        LOG_MSG_DEBUG("sync finger up");
        silfp_msg_send(SIFP_MSG_UP);
    }
}

int32_t silfp_dev_is_finger_down(void)
{
    if (SIFP_FINGER_STATUS_IRQ == m_irq_op_mode) {
        return 1;
    }
    return silfp_msg_is_finger_down();
}

void silfp_dev_reset_finger_status(void)
{
    silfp_msg_reset_finger_status();
}

int32_t silfp_dev_send_key(uint32_t key)
{
    int32_t ret = 0;

    fp_dev_key_t k = {
        .key = key,
        .value = NAV_KEY_FLAG_CLICK,
    };

    LOG_MSG_DEBUG("should report key = %s", silead_key_get_des((int32_t)key));

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_KEY_EVENT, &k);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_set_log_level(uint8_t lvl)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_DBG_LEVEL, &lvl);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }
    return ret;
}

int32_t silfp_dev_set_feature(uint8_t feature)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_SET_FEATURE, &feature);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }
    return ret;
}

int32_t silfp_dev_wakelock(uint8_t lock)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_WAKELOCK, &lock);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_pwdn(uint8_t avdd_op)
{
    int32_t ret = 0;

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

    ret = ioctl(m_dev_fd, SIFP_IOC_PWDN, &avdd_op);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}

int32_t silfp_dev_create_proc_node(const char *chipname)
{
    int32_t ret = 0;
    char name[PROC_VND_ID_LEN];

    if (m_dev_fd < 0) {
        LOG_MSG_ERROR("NO DEV");
        return -SL_ERROR_DEV_OPEN_FAILED;
    }

#ifndef SIL_FP_PROC_VALUE
    if (chipname == NULL) {
        return -SL_ERROR_DEV_IOCTL_FAILED;
    }

    memset(name, 0x0, sizeof(name));
    silfp_util_strcpy(name, sizeof(name), chipname, strlen(chipname));
#else
    UNUSED(chipname);
    memset(name, 0x0, sizeof(name));
    silfp_util_strcpy(name, sizeof(name), SIL_FP_PROC_VALUE, strlen(SIL_FP_PROC_VALUE));
#endif

    ret = ioctl(m_dev_fd, SIFP_IOC_PROC_NODE, name);
    if (ret < 0) {
        ret = -SL_ERROR_DEV_IOCTL_FAILED;
    }

    return ret;
}
