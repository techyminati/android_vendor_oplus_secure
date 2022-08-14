/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: device layer simulator chip
 * History:
 * Version: 1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cutils/fs.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include "gf_hal_common.h"
#include "gf_hal_log.h"
#include "gf_hal_device.h"
#include "gf_hal.h"
#include "gf_queue.h"
#include "gf_ca_entry.h"
#include "gf_hal_simulator_native.h"
#include "gf_hal_simulator_type.h"

#define LOG_TAG "[GF_HAL][gf_hal_fps_device]"

#define SOCKET_NAME "fps_socket"

#define UINPUT_DEV "/dev/uinput"
#define FP_UINPUT_DEV_NAME "uinput_nav"
#define UINPUT_DEV_VENDOR 0x1
#define UINPUT_DEV_PRODUCT 0x1
#define UINPUT_DEV_VERSION 1
#define QUIT_CONTROL "quit"

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

typedef void (*notify)(const gf_fingerprint_msg_t *msg);
notify g_old_notify = NULL;  // nodify callback

static int32_t g_uinput_fd = -1;  // uinput device handle
static pthread_t g_netlink_thread = 0;  // netlink thread
static pthread_t g_handle_thread = 0;  // handle thread
static sem_t g_netlink_sem;  // netlink sem
static int32_t g_client_socket = -1;  // client socket indicate a connection
static int32_t g_pipe[2];  // the pipe for data transfer to ensure socket send/receive  in same thread

/*
Function: read_data
Description: read data from simulator socket
Input: client_socket
Output:
Return: none
Others:
*/
static void read_data(int32_t* client_socket)
{
    VOID_FUNC_ENTER();
    do
    {
        char buffer[1024] = { 0 };
        int32_t ret = 0;
        ret = read(*client_socket, &buffer, sizeof(buffer));
        if (ret < 0)
        {
            LOG_E(LOG_TAG, "[%s]read data error.", __func__);
            break;
        }
        else if (ret == 0)
        {
            LOG_E(LOG_TAG, "[%s]Client closed.", __func__);
            close(*client_socket);
            *client_socket = -1;
            g_client_socket = -1;
        }
        else
        {
            int32_t* p = (int32_t*) buffer;
            int32_t data_type = *p;
            p++;
            switch (data_type)
            {
                case IRQ_TYPE:
                {
                    gf_error_t err = gf_simulator_invoke_irq(p);
                    if (err == GF_SUCCESS)
                    {
                        sem_post(&g_netlink_sem);
                        usleep(1000);
                    }
                    break;
                }

                case NAV_ORIENTATION:
                {
                    gf_simulator_set_nav_oritension(p);
                    break;
                }

                case ENROLL:
                {
                    gf_simulator_enroll(p);
                    break;
                }

                case AUTHENTICATE:
                {
                    gf_simulator_authenticate(p);
                    break;
                }

                case REMOVE:
                {
                    gf_simulator_remove_finger(p);
                    break;
                }
                case ENUMERATE:
                {
                    gf_simulator_enumerate(p);
                    break;
                }

                case SET_ACTIVE_GROUP:
                {
                    gf_simulator_set_active_group(p);
                    break;
                }

                case GET_AUTH_ID:
                {
                    gf_simulator_get_auth_id(p);
                    break;
                }

                case ENUMERATE_WITH_CALLBACK:
                {
                    gf_simulator_enumerate_with_callback(p);
                    break;
                }

                case REMOVE_WITH_CALLBACK:
                {
                    gf_simulator_remove_with_callback(p);
                    break;
                }

                case MEMORY_CHECK:
                {
                    gf_simulator_check_memory(p);
                    break;
                }

                case CLEAR:
                {
                    gf_simulator_clear(p);
                    break;
                }
                case FINGER_QUICK_UP_OR_MISTAKE_TOUCH:
                {
                    gf_simulator_finger_quick_up_or_mistake_touch(p);
                    break;
                }

                case AUTHENTICATE_FIDO:
                {
                    gf_simulator_authenticate_fido(p);
                    break;
                }

                case PRE_ENROLL:
                {
                    gf_simulator_pre_enroll(p);
                    break;
                }

                case POST_ENROLL:
                {
                    gf_simulator_post_enroll(p);
                    break;
                }

                case CANCEL:
                {
                    gf_simulator_cancel(p);
                    break;
                }

                case SET_DUMP_CONFIG:
                {
                    gf_simulator_set_dump_config((gf_dump_config_t*)p);
                    break;
                }

                case DUMP_CMD:
                {
                    gf_simulator_dump_cmd(p);
                    break;
                }

                case RESET_APK_ENABLED_DUMP:
                {
                    gf_simulator_reset_apk_enable_dump_flag();
                    break;
                }

                case SIMULATE_DIRTY_DATA:
                {
                    gf_simulator_set_simulate_dirty_data_flag(p);
                    break;
                }

                case DUMP_DEVICE_INFO:
                {
                    gf_simulator_dump_device_info();
                    break;
                }

                case START_NAVIGATE:
                {
                    gf_simulator_start_navigate();
                    break;
                }

                default:
                {
                    break;
                }
            }  // end switch
        }  // end else
    }  // end do
    while (0);

    VOID_FUNC_EXIT();
}

/*
Function: verify_client
Description: set g_old_notify and g_fingerprint_device->notify when socket connect
Input: client_socket
Output:
Return: none
Others:
*/
static void verify_client(int32_t* client_socket)
{
    int32_t client_type = 0;
    int32_t ret = 0;
    ret = read(*client_socket, &client_type, sizeof(int32_t));

    if (ret < 0)
    {
        LOG_E(LOG_TAG, "[%s]Read client type error.", __func__);
        close(*client_socket);
        *client_socket = -1;
    }
    else
    {
        if (g_old_notify == NULL)
        {
            g_old_notify = g_fingerprint_device->notify;
        }
        if (client_type == SR_CLIENT)
        {
            g_fingerprint_device->notify = notify_for_simulate;
            g_fingerprint_device->test_notify = notify_for_simulate;
        }
        else if (client_type == SR_SHELL)
        {
            g_fingerprint_device->notify = g_old_notify;
        }
        else
        {
            LOG_E(LOG_TAG, "[%s]Unkown client type.", __func__);
            close(*client_socket);
            *client_socket = -1;
        }
    }
}

/*
Function: hal_enter_listener
Description: listen socket and pipe message
Input: client_socket
Output:
Return: none
Others:
*/
static void hal_enter_listener(int32_t listen_socket)
{
    struct sockaddr_un addr;
    int32_t addr_len = 0;
    int32_t max = -1;
    int32_t ret = -1;
    int32_t client_socket = -1;
    fd_set readfds;

    VOID_FUNC_ENTER();

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(listen_socket, &readfds);
        max = listen_socket;
        if (client_socket > 0)
        {
            if (client_socket > max)
            {
                max = client_socket;
            }
            FD_SET(client_socket, &readfds);
        }

        if (g_pipe[0] > max)
        {
            max = g_pipe[0];
        }
        FD_SET(g_pipe[0], &readfds);

        ret = select(max + 1, &readfds, 0, 0, NULL);
        if (ret > 0)
        {
            if (FD_ISSET(listen_socket, &readfds))
            {
                LOG_D(LOG_TAG, "[%s]Client connect....", __func__);
                client_socket = accept(listen_socket, (struct sockaddr *) &addr,
                        (socklen_t*) &addr_len);
                if (client_socket < 0)
                {
                    LOG_E(LOG_TAG, "[%s] Accept socket error = %s", __func__, strerror(errno));
                    continue;
                }
                else if (g_client_socket > 0)
                {
                    LOG_E(LOG_TAG, "[%s] The socket server has been connected, Drop last client",
                            __func__);
                    close(g_client_socket);
                    g_client_socket = client_socket;
                }
                verify_client(&client_socket);
                if (client_socket == -1)
                {
                    continue;
                }
            }
            else if (FD_ISSET(client_socket, &readfds))
            {
                read_data(&client_socket);
            }
            else if (FD_ISSET(g_pipe[0], &readfds))
            {
                char buffer[5*1024] = { 0 };
                int32_t actual_len = -1;
                actual_len = read(g_pipe[0], buffer, sizeof(buffer));
                if (actual_len < 0)
                {
                    LOG_E(LOG_TAG, "[%s]read pipe error = %s.", __func__, strerror(errno));
                }
                else
                {
                    if (memcmp(buffer, QUIT_CONTROL, sizeof(QUIT_CONTROL)) == 0)
                    {
                        LOG_D(LOG_TAG, "[%s]Server quit.",  __func__);
                        break;
                    }
                    ret = write(client_socket, buffer, actual_len);
                    if (ret < 0)
                    {
                        LOG_E(LOG_TAG, "[%s]write data from  pipe error = %s.", __func__, strerror(errno));
                    }
                }
            }
        }  // end if
        else
        {
            LOG_E(LOG_TAG, "[%s]Select error = %s.", __func__, strerror(errno));
        }
    }  // end while

    VOID_FUNC_EXIT();
}

/*
Function: gf_hal_netlink_recv
Description: init socket and listen
Input: handle
Output:
Return: none
Others:
*/
void *gf_hal_netlink_recv(void *handle)
{
    struct sockaddr_un addr;
    int32_t fd = 0;
    int32_t ret = 0;

    VOID_FUNC_ENTER();
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG_E(LOG_TAG, "[%s] Create socket error = %s", __func__, strerror(errno));
        return NULL;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = 0;
    snprintf(&addr.sun_path[1], sizeof(addr.sun_path) - 1, "%s", SOCKET_NAME);
    int32_t addr_len = strlen(SOCKET_NAME) + offsetof(struct sockaddr_un, sun_path) + 1;

    ret = bind(fd, (struct sockaddr *) &addr, addr_len);
    if (ret)
    {
        LOG_E(LOG_TAG, "[%s] Bind socket error = %s", __func__, strerror(errno));
        close(fd);
        fd = -1;
    }

    if (listen(fd, 5) < 0)
    {
        LOG_E(LOG_TAG, "[%s] Listen socket error = %s", __func__, strerror(errno));
        close(fd);
        fd = -1;
    }
    hal_enter_listener(fd);

    if (fd > 0)
    {
        close(fd);
        fd = -1;
    }
    VOID_FUNC_EXIT();
    return NULL;
}

/*
Function: gf_handle_thread
Description: deal irq in dequeue
Input: handle
Output:
Return: none
Others:
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
            gf_hal_screen_off();
        }
        else if (GF_NETLINK_SCREEN_ON == value)
        {
            gf_hal_screen_on();
        }
    }

    pthread_exit((void *) 0);
    g_handle_thread = 0;
    VOID_FUNC_EXIT();
    return 0;
}

/*
Function: gf_hal_device_open
Description: open device
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_device_open()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    do
    {
        // if kill fingerprintd, netlink recv will sem_post after device open
        // init g_netlink_sem to make sem_post effective
        if (0 != sem_init(&g_netlink_sem, 0, 0))
        {
            LOG_E(LOG_TAG, "[%s] init semaphore failed", __func__);
            err = GF_ERROR_HAL_GENERAL_ERROR;
            break;
        }
        gf_queue_init();
        gf_hal_uinput_device_open();
        pipe(g_pipe);
        gf_simulator_set_channel(g_pipe[1]);
    } while (0);
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_uinput_device_open
Description: open uinput device
Input:
Output:
Return: error code
Others:
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
            ioctl(g_uinput_fd, UI_DEV_DESTROY);
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
                ioctl(g_uinput_fd, UI_DEV_DESTROY);
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
            ioctl(g_uinput_fd, UI_DEV_DESTROY);
            close(g_uinput_fd);
            g_uinput_fd = -1;
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        if (ioctl(g_uinput_fd, UI_DEV_CREATE) < 0)
        {
            LOG_E(LOG_TAG, "[%s] Failed to set UI_DEV_CREATE. err=%s", __func__,
                  strerror(errno));
            ioctl(g_uinput_fd, UI_DEV_DESTROY);
            close(g_uinput_fd);
            g_uinput_fd = -1;
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }
    }  // end do
    while (0);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_uinpt_reportkey
Description: report key
Input: fd, type, keycode, value
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_uinpt_reportkey(int32_t fd, uint16_t type, uint16_t keycode, int32_t value)
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

/*
Function: convert_nav_code
Description: convert navigation code
Input: code
Output:
Return: key code
Others:
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

/*
Function: gf_hal_send_uinput_nav_event
Description: send uinput navigation event
Input: code
Output:
Return: error code
Others:
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
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_device_enable
Description: create handle thread and netlink recv thread
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_device_enable(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do
    {
        if (pthread_create(&g_handle_thread, NULL, gf_handle_thread, NULL) != 0)
        {
            LOG_E(LOG_TAG, "[%s] pthread_create failed", __func__);
            err = GF_ERROR_HAL_GENERAL_ERROR;
            break;
        }

        if (0 == g_netlink_thread)
        {
            if (pthread_create(&g_netlink_thread, NULL, gf_hal_netlink_recv, NULL) != 0)
            {
                LOG_E(LOG_TAG, "[%s] pthread_create failed", __func__);
                err = GF_ERROR_HAL_GENERAL_ERROR;
                break;
            }
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_reset_chip
Description: reset chip
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_reset_chip(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_get_fw_info
Description: get fw info
Input: buf
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_get_fw_info(uint8_t *buf)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_enter_sleep_mode
Description: enter sleep mode
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_enter_sleep_mode(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_control_spi_clock
Description: control spi clock
Input: enable
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_control_spi_clock(uint8_t enable)
{
    gf_error_t err = GF_SUCCESS;
    return err;
}

/*
Function: gf_hal_send_nav_event
Description: send navigation event
Input: code
Output:
Return: error code
Others:
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

/*
Function: gf_hal_send_key_event
Description: send key event
Input: code, status
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_send_key_event(gf_key_code_t code, gf_key_status_t status)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_send_key_event
Description: destory handle thread
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_device_disable(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (g_handle_thread != 0)
    {
        LOG_I(LOG_TAG, "[%s] destory handle thread", __func__);
        pthread_detach(g_handle_thread);
        g_handle_thread = 0;
    }
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_device_close
Description: close device, pipe, uniput
Input:
Output:
Return:
Others:
*/
void gf_hal_device_close(void)
{
    VOID_FUNC_ENTER();
    close(g_pipe[0]);
    close(g_pipe[1]);
    write(g_pipe[1], QUIT_CONTROL, strlen(QUIT_CONTROL));
    if (g_uinput_fd >= 0)
    {
        close(g_uinput_fd);
        g_uinput_fd = -1;
    }
    sem_destroy(&g_netlink_sem);
    VOID_FUNC_EXIT();
}

/*
Function: gf_hal_enable_power
Description: enable power
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_enable_power(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_disable_power
Description: disable power
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_disable_power(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_enable_irq
Description: enable irq
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_enable_irq(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_disable_irq
Description: disable irq
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_disable_irq(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_device_remove
Description: remove device
Input:
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_device_remove(void)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

/*
Function: gf_hal_chip_info
Description: chip info
Input: chip info
Output:
Return: error code
Others:
*/
gf_error_t gf_hal_chip_info(gf_ioc_chip_info_t info)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

