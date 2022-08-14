/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/netlink.h>
#include <linux/socket.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "gf_hal_common.h"
#include "gf_hal_log.h"
#include "gf_hal_device.h"
#include "gf_hal.h"
#include "gf_queue.h"
#include "gf_ca_entry.h"
#include "gf_hal_mem.h"
#ifdef SUPPORT_FUNCTIONAL_TEST
#include "gf_ft.h"
#endif  // SUPPORT_FUNCTIONAL_TEST

#define LOG_TAG "[GF_HAL][gf_hal_device]"

#define GF_IOC_MAGIC    'g'      // define magic number
/* define commands */
#define GF_IOC_INIT             (_IOR(GF_IOC_MAGIC, 0, uint8_t))
#define GF_IOC_EXIT             (_IO(GF_IOC_MAGIC, 1))
#define GF_IOC_RESET            (_IO(GF_IOC_MAGIC, 2))

#define GF_IOC_ENABLE_IRQ       (_IO(GF_IOC_MAGIC, 3))
#define GF_IOC_DISABLE_IRQ      (_IO(GF_IOC_MAGIC, 4))
#define GF_IOC_ENABLE_SPI_CLK   (_IOW(GF_IOC_MAGIC, 5, uint32_t))
#define GF_IOC_DISABLE_SPI_CLK  (_IO(GF_IOC_MAGIC, 6))

#define GF_IOC_ENABLE_POWER     (_IO(GF_IOC_MAGIC, 7))
#define GF_IOC_DISABLE_POWER    (_IO(GF_IOC_MAGIC, 8))

#define GF_IOC_INPUT_KEY_EVENT  (_IOW(GF_IOC_MAGIC, 9, gf_key_event_t))

/* fp chip has change to sleep mode while screen off */
#define GF_IOC_ENTER_SLEEP_MODE (_IO(GF_IOC_MAGIC, 10))
#define GF_IOC_GET_FW_INFO      (_IOR(GF_IOC_MAGIC, 11, uint8_t))
#define GF_IOC_REMOVE           (_IO(GF_IOC_MAGIC, 12))
#define GF_IOC_CHIP_INFO        (_IOW(GF_IOC_MAGIC, 13, gf_ioc_chip_info_t))

#define GF_IOC_NAV_EVENT        (_IOW(GF_IOC_MAGIC, 14, gf_nav_code_t))
// #define GF_IOC_SET_SPI_SPEED      (_IOW(GF_IOC_MAGIC, 17, uint32_t))
#define GF_IOC_POWER_RESET     (_IO(GF_IOC_MAGIC, 17))

/* netlink feature support */
#define MAX_NL_MSG_LEN 16

#define GF_DEV_NAME "/dev/goodix_fp"
#define UINPUT_DEV "/dev/uinput"
#define FP_UINPUT_DEV_NAME "uinput_nav"
#define UINPUT_DEV_VENDOR 0x1
#define UINPUT_DEV_PRODUCT 0x1
#define UINPUT_DEV_VERSION 1

static gf_nav_code_t g_nav_codes[] = {  // nave code
    GF_NAV_DOWN,
    GF_NAV_UP,
    GF_NAV_LEFT,
    GF_NAV_RIGHT,
    GF_NAV_CLICK,
    GF_NAV_LONG_PRESS,
    GF_NAV_DOUBLE_CLICK
};

#if defined(__ANDROID_O) || defined(__ANDROID_P)
// FIXME: use system constant value insead of magic number after publishing android o
static uint16_t g_key_codes[] = {   // key codes
    0x6c,
    0x67,
    0x6a,
    0x69,
    KEY_VOLUMEDOWN,
    KEY_SEARCH,
    KEY_VOLUMEUP
};
#else  //  not __ANDROID_O
static uint16_t g_key_codes[] = {  // key codes
    KEY_DOWN,
    KEY_UP,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_VOLUMEDOWN,
    KEY_SEARCH,
    KEY_VOLUMEUP
};
#endif  //  end #if defined(__ANDROID_O)

static int32_t g_fd = -1;  // device handle
static int32_t g_uinput_fd = -1;  // uinput device handle
static pthread_t g_netlink_thread = 0;  // netlink thread
static pthread_t g_handle_thread = 0;  // handle thread
static sem_t g_netlink_sem;  // netlink sem
static int32_t g_netlink_sock_id = 0;  // id of netlink socket
static uint8_t g_netlink_route = 29;  // netlink route
uint8_t g_sensor_power_down = RE_POWER_DIS;  // power down flag
uint8_t g_sensor_disable = 0;

/**
 * Function: gf_hal_netlink_recv
 * Description: receive message from netlink socket.
 * Input: handle
 * Output: None
 * Return: void
 */
void *gf_hal_netlink_recv(void *handle)
{
    struct nlmsghdr *nlh = NULL;
    VOID_FUNC_ENTER();
    TEST_STUB(gf_hal_netlink_recv, handle);

    LOG_D(LOG_TAG, "[%s] GF netlink thread started, handle=%p", __func__, handle);

    do
    {
        struct sockaddr_nl local; /*used to describe local address*/
        struct sockaddr_nl dest;
        struct iovec iov;
        struct msghdr msg;
        int32_t ret = 0;
        uint8_t value = 0;
        /* init socket and bind */
        g_netlink_sock_id = socket(AF_NETLINK, SOCK_RAW, g_netlink_route);

        if (g_netlink_sock_id < 0)
        {
            LOG_E(LOG_TAG, "[%s] socket failed. err=%s, errno=%d", __func__,
                  strerror(errno), errno);
            break;
        }

        memset(&local, 0, sizeof(struct sockaddr_nl));
        local.nl_family = AF_NETLINK;
        local.nl_pid = getpid();/*local process id*/
        LOG_D(LOG_TAG, "[%s] native process pid=%d", __func__, local.nl_pid);
        // local.nl_pad = 0;
        local.nl_groups = 0;
        ret = bind(g_netlink_sock_id, (struct sockaddr *) &local,
                   sizeof(struct sockaddr_nl));

        if (ret != 0)
        {
            LOG_E(LOG_TAG, "[%s] bind failed. err=%s, errno=%d", __func__, strerror(errno),
                  errno);
            break;
        }

        LOG_D(LOG_TAG, "[%s] bind done", __func__);
        /* send init message */
        memset(&dest, 0, sizeof(struct sockaddr_nl));
        dest.nl_family = AF_NETLINK;
        dest.nl_pid = 0; /*destination is kernel so set to 0*/
        dest.nl_groups = 0;
        nlh = (struct nlmsghdr *) GF_MEM_MALLOC(NLMSG_SPACE(MAX_NL_MSG_LEN));

        if (NULL == nlh)
        {
            LOG_E(LOG_TAG, "[%s] nlh out of memory", __func__);
            break;
        }

        nlh->nlmsg_len = NLMSG_SPACE(MAX_NL_MSG_LEN);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;
        strncpy(NLMSG_DATA(nlh), "GF", strlen("GF") + 1);
        iov.iov_base = (void *) nlh;
        iov.iov_len = nlh->nlmsg_len;
        memset(&msg, 0, sizeof(struct msghdr));
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (void *) &dest;
        msg.msg_namelen = sizeof(struct sockaddr_nl);

        if (sendmsg(g_netlink_sock_id, &msg, 0) < 0)
        {
            LOG_E(LOG_TAG, "[%s] sendmsg failed. err=%s, erron=%d", __func__,
                  strerror(errno), errno);
            break;
        }

        LOG_D(LOG_TAG, "[%s] send init msg to kernel", __func__);
        /* loop for recv */
        memset(nlh, 0, NLMSG_SPACE(MAX_NL_MSG_LEN));

        while (1)
        {
            ret = recvmsg(g_netlink_sock_id, &msg, 0);

            if (ret < 0)
            {
                LOG_E(LOG_TAG, "[%s] recvmsg failed, ret=%d", __func__, ret);
                continue;
            }

            if (0 == ret)
            {
                LOG_E(LOG_TAG, "[%s] recvmsg failed, ret=%d", __func__, ret);
                continue;
            }

            value = *((char *) NLMSG_DATA(nlh));

            if (GF_NETLINK_TEST == value)
            {
                // for netlink test use only
                LOG_D(LOG_TAG, "[%s] received GF_NETLINK_TEST command", __func__);
            }
            else if (GF_NETLINK_IRQ == value || GF_NETLINK_SCREEN_OFF == value
                     || GF_NETLINK_SCREEN_ON == value)
            {
                if (gf_enqueue(value) == GF_SUCCESS)
                {
                    sem_post(&g_netlink_sem);
                    LOG_D(LOG_TAG, "[%s] send message value=%u", __func__, value);
                }
            }
            else
            {
                LOG_E(LOG_TAG, "[%s] wrong netlink command value=%u", __func__, value);
            }
        }  // while(1)
    }  // do gf_hal_netlink_recv
    while (0);

    LOG_D(LOG_TAG, "[%s] GF netlink thread finish.", __func__);

    if (nlh != NULL)
    {
        GF_MEM_FREE(nlh);
    }

    if (g_netlink_sock_id > 0)
    {
        close(g_netlink_sock_id);
        g_netlink_sock_id = 0;
    }

    pthread_exit((void *) 0);
    g_netlink_thread = 0;
    VOID_FUNC_EXIT();
    return 0;
}

/**
 * Function: gf_handle_thread
 * Description: Handle the message from kernel driver.
 * Input: handle
 * Output: None
 * Return: void
 */
void *gf_handle_thread(void *handle)
{
    uint8_t value = 0;
    gf_error_t err = GF_SUCCESS;
    VOID_FUNC_ENTER();

    LOG_D(LOG_TAG, "[%s] GF handler thread started, handle=%p", __func__, handle);

    while (1)
    {
        sem_wait(&g_netlink_sem);
        err = gf_dequeue(&value);

        if (err != GF_SUCCESS)
        {
            continue;
        }

        if (GF_NETLINK_IRQ == value)
        {
            gf_hal_irq();
        }
        else if (GF_NETLINK_SCREEN_OFF == value)
        {
            //gf_hal_screen_off();
        }
        else if (GF_NETLINK_SCREEN_ON == value)
        {
            //gf_hal_screen_on();
        }
        else if (GF_NETLINK_DETECT_BROKEN == value)
        {
            gf_hal_detect_sensor_broken();
        }
    }

    pthread_exit((void *) 0);
    g_handle_thread = 0;
    VOID_FUNC_EXIT();
    return 0;
}

/**
 * Function: gf_hal_device_open
 * Description: Open device node, initialize netlink semaphore and message queue.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_device_open()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        g_fd = open(GF_DEV_NAME, O_RDWR);

        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s] Failed to open device(%s):%s", __func__, GF_DEV_NAME,
                  strerror(errno));
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            g_fd = -1;
            break;
        }

        gf_ca_set_handle(g_fd);

        // if kill fingerprintd, netlink recv will sem_post after device open
        // init g_netlink_sem to make sem_post effective
        if (0 != sem_init(&g_netlink_sem, 0, 0))
        {
            LOG_E(LOG_TAG, "[%s] init semaphore failed", __func__);
            err = GF_ERROR_HAL_GENERAL_ERROR;
            break;
        }

        gf_queue_init();
        //err = gf_hal_uinput_device_open();
    }
    while (0);


    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_uinput_device_open
 * Description: Open and config user mode input device.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_uinput_device_open()
{
    gf_error_t err = GF_SUCCESS;
    struct uinput_user_dev uidev;
    uint32_t i = 0;
    FUNC_ENTER();

    do
    {
        g_uinput_fd = open(UINPUT_DEV, O_WRONLY | O_NONBLOCK);

        if (g_uinput_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s] Failed to open device (%s). err=%s", __func__,
                  UINPUT_DEV, strerror(errno));
            g_uinput_fd = -1;
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        if (ioctl(g_uinput_fd, UI_SET_EVBIT, EV_KEY) < 0)
        {
            LOG_E(LOG_TAG, "[%s] Failed to set UI_SET_EVBIT. err=%s", __func__,
                  strerror(errno));
            if (ioctl(g_uinput_fd, UI_DEV_DESTROY) < 0)
            {
                LOG_E(LOG_TAG, "[%s] UI_DEV_DESTROY ioctl failed", __func__);
            }

            close(g_uinput_fd);
            g_uinput_fd = -1;
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        for (i = 0; i < sizeof(g_key_codes) / sizeof(g_key_codes[0]); i++)
        {
            if (ioctl(g_uinput_fd, UI_SET_KEYBIT, g_key_codes[i]) < 0)
            {
                LOG_E(LOG_TAG, "[%s] Failed to set keycode %u. err=%s", __func__, g_key_codes[i],
                      strerror(errno));
                if (ioctl(g_uinput_fd, UI_DEV_DESTROY) < 0)
                {
                    LOG_E(LOG_TAG, "[%s] UI_DEV_DESTROY ioctl failed", __func__);
                }
                close(g_uinput_fd);
                g_uinput_fd = -1;
                err = GF_ERROR_OPEN_DEVICE_FAILED;
                break;
            }
        }

        if (GF_SUCCESS != err)
        {
            break;
        }

        memset(&uidev, 0, sizeof(uidev));
        snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, FP_UINPUT_DEV_NAME);
        uidev.id.bustype = BUS_USB;
        uidev.id.vendor = UINPUT_DEV_VENDOR;
        uidev.id.product = UINPUT_DEV_PRODUCT;
        uidev.id.version = UINPUT_DEV_VERSION;

        if (write(g_uinput_fd, &uidev, sizeof(uidev)) < 0)
        {
            LOG_E(LOG_TAG, "[%s] Failed to write uidev. err=%s", __func__, strerror(errno));
            if (ioctl(g_uinput_fd, UI_DEV_DESTROY) < 0)
            {
                LOG_E(LOG_TAG, "[%s] UI_DEV_DESTROY ioctl failed", __func__);
            }
            close(g_uinput_fd);
            g_uinput_fd = -1;
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        if (ioctl(g_uinput_fd, UI_DEV_CREATE) < 0)
        {
            LOG_E(LOG_TAG, "[%s] Failed to set UI_DEV_CREATE. err=%s", __func__,
                  strerror(errno));
            if (ioctl(g_uinput_fd, UI_DEV_DESTROY) < 0)
            {
                LOG_E(LOG_TAG, "[%s] UI_DEV_DESTROY ioctl failed", __func__);
            }
            close(g_uinput_fd);
            g_uinput_fd = -1;
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }
    }  // do gf_hal_uinput_device_open
    while (0);

    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_uinpt_reportkey
 * Description: Report key event.
 * Input: fd, type, keycode, value
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_uinpt_reportkey(int32_t fd, uint16_t type, uint16_t keycode,
                                  int32_t value)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    struct input_event ev;
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = type;
    ev.code = keycode;
    ev.value = value;

    if (write(fd, &ev, sizeof(struct input_event)) < 0)
    {
        LOG_E(LOG_TAG, "[%s] write failed", __func__);
        err = GF_ERROR_HAL_IOCTL_FAILED;
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: convert_nav_code
 * Description: Convert the event code of navigation.
 * Input: code
 * Output: None
 * Return: uint16_t
 */
static uint16_t convert_nav_code(gf_nav_code_t code)
{
    uint16_t key_code = 0;
    uint32_t i = 0;

    for (; i < sizeof(g_nav_codes) / sizeof(g_nav_codes[0]); i++)
    {
        if (code  == g_nav_codes[i])
        {
            key_code = g_key_codes[i];
            break;
        }
    }

    return key_code;
}

/**
 * Function: gf_hal_send_uinput_nav_event
 * Description: Report navigation event.
 * Input: code
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_send_uinput_nav_event(gf_nav_code_t code)
{
    gf_error_t err = GF_SUCCESS;
    uint16_t key_code = 0;

    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] nav=%s, nav_code=%d", __func__, gf_strnav(code), code);

    do
    {
        if (g_uinput_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, UINPUT_DEV);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        key_code = convert_nav_code(code);

        if (0 != key_code)
        {
            gf_hal_uinpt_reportkey(g_uinput_fd, EV_KEY, key_code, 1);
            gf_hal_uinpt_reportkey(g_uinput_fd, EV_SYN, SYN_REPORT, 0);
            gf_hal_uinpt_reportkey(g_uinput_fd, EV_KEY, key_code, 0);
            gf_hal_uinpt_reportkey(g_uinput_fd, EV_SYN, SYN_REPORT, 0);
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_device_enable
 * Description: Enable netlink and IRQ for device.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_device_enable(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (pthread_create(&g_handle_thread, NULL, gf_handle_thread, NULL) != 0)
        {
            LOG_E(LOG_TAG, "[%s] pthread_create failed", __func__);
            err = GF_ERROR_HAL_GENERAL_ERROR;
            break;
        }

        if (ioctl(g_fd, GF_IOC_INIT, &g_netlink_route) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_INIT ioctl failed", __func__);
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        LOG_I(LOG_TAG, "[%s] g_netlink_route=%u", __func__, g_netlink_route);

        if (0 == g_netlink_thread)
        {
            if (pthread_create(&g_netlink_thread, NULL, gf_hal_netlink_recv, NULL) != 0)
            {
                LOG_E(LOG_TAG, "[%s] pthread_create failed", __func__);
                err = GF_ERROR_HAL_GENERAL_ERROR;
                break;
            }
        }

        if (ioctl(g_fd, GF_IOC_ENABLE_IRQ) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }
    }  // do gf_hal_device_enable
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_reset_chip
 * Description: Reset chip.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_reset_chip(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_RESET) != 0)
        {
            LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_power_reset
 * Description: Reset chip.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_power_reset(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (g_fd < 0) {
            // delete log
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }
        if (ioctl(g_fd, GF_IOC_POWER_RESET) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
        g_sensor_power_down = 0;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_get_fw_info
 * Description: Read firmware information.
 * Input: None
 * Output: buf
 * Return: gf_error_t
 */
gf_error_t gf_hal_get_fw_info(uint8_t *buf)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (NULL == buf)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_GET_FW_INFO, buf) != 0)
        {
            LOG_E(LOG_TAG, "gf_hal_get_fw_info ioctl faild");
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_enter_sleep_mode
 * Description: Enter sleep mode for low power level.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enter_sleep_mode(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_ENTER_SLEEP_MODE) != 0)
        {
            LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_control_spi_clock
 * Description: Enable or disable hardware clock for SPI.
 * Input: enable
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_control_spi_clock(uint8_t enable)
{
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (enable > 0)
        {
            if (ioctl(g_fd, GF_IOC_ENABLE_SPI_CLK, &g_spi_speed) != 0)
            {
                LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }

            /*
             //samsung exynos8890 spi clock only  set by linux and spi clock need set after enable spi clock.
             if (ioctl(g_fd, GF_IOC_SET_SPI_SPEED, &g_spi_speed) != 0) {
             LOG_E(LOG_TAG, "[%s] :set spi speed failed, g_spi_speed=%d.", __func__, g_spi_speed);
             err = GF_ERROR_HAL_IOCTL_FAILED;
             break;
             }
             */
        }
        else
        {
            if (ioctl(g_fd, GF_IOC_DISABLE_SPI_CLK) != 0)
            {
                LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
    }  // do gf_hal_control_spi_clock
    while (0);

    return err;
}

/**
 * Function: gf_hal_send_nav_event
 * Description: Send navigation event.
 * Input: code
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_send_nav_event(gf_nav_code_t code)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] nav=%s, nav_code=%d", __func__, gf_strnav(code), code);

    err = gf_hal_send_uinput_nav_event(code);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_send_key_event
 * Description: Send key event.
 * Input: code, status
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_send_key_event(gf_key_code_t code, gf_key_status_t status)
{
    gf_error_t err = GF_SUCCESS;
    gf_key_event_t key_event = { 0 };
    FUNC_ENTER();
    LOG_D(LOG_TAG, "key=%s, key_code=%d, state=%d", gf_strkey(code), code, status);

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        key_event.key = code;
        key_event.status = status;

        if (ioctl(g_fd, GF_IOC_INPUT_KEY_EVENT, &key_event) != 0)
        {
            LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_device_disable
 * Description: Destroy the input device and netlink thread, disable IRQ.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_device_disable(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_DISABLE_IRQ) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        if (ioctl(g_fd, GF_IOC_EXIT) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_EXIT ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }

        if (g_uinput_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, UINPUT_DEV);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_uinput_fd, UI_DEV_DESTROY) < 0)
        {
            LOG_E(LOG_TAG, "[%s] UI_DEV_DESTROY ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }  // do gf_hal_device_disable
    while (0);

    if (g_handle_thread != 0)
    {
        LOG_I(LOG_TAG, "[%s] destory handle thread", __func__);
        pthread_detach(g_handle_thread);
        g_handle_thread = 0;
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_device_close
 * Description: Release resources,and close device handle.
 * Input: None
 * Output: None
 * Return: None
 */
void gf_hal_device_close(void)
{
    VOID_FUNC_ENTER();
    gf_queue_exit();

    if (g_fd >= 0)
    {
        close(g_fd);
        g_fd = -1;
    }

    if (g_uinput_fd >= 0)
    {
        close(g_uinput_fd);
        g_uinput_fd = -1;
    }

    sem_destroy(&g_netlink_sem);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_enable_power
 * Description: Power on.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enable_power(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_ENABLE_POWER) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_POWER ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
        g_sensor_power_down = RE_POWER_DIS;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_disable_power
 * Description: Power Off.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_disable_power(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_DISABLE_POWER) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_POWER ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
        g_sensor_power_down = RE_POWER_EN;
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_enable_irq
 * Description: Enable IRQ.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_enable_irq(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_ENABLE_IRQ) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_enable_irq
 * Description: Disable IRQ.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_disable_irq(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_DISABLE_IRQ) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_device_remove
 * Description: Remove device node.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_device_remove(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(g_fd, GF_IOC_REMOVE) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_REMOVE ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_chip_info
 * Description: Read chip information from sensor.
 * Input: info
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_chip_info(gf_ioc_chip_info_t info)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (g_fd < 0)
        {
            LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        LOG_D(LOG_TAG, "[%s] vendor_id=0x%X, mode=0x%X", __func__, info.vendor_id,
              info.mode);

        if (ioctl(g_fd, GF_IOC_CHIP_INFO, &info) != 0)
        {
            LOG_E(LOG_TAG, "[%s] GF_IOC_CHIP_INFO ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}


/*
 * Function: gf_hal_post_sem_detect_broken
 * Description: post sem to decect sensor broken.
 * Input: Null
 * Output: None
 * Return: None
 */
void gf_hal_post_sem_detect_broken(void)
{
	if (gf_enqueue(GF_NETLINK_DETECT_BROKEN) == GF_SUCCESS)
	{
		sem_post(&g_netlink_sem);
		LOG_D(LOG_TAG, "[%s] send broken message", __func__);
	}
}



