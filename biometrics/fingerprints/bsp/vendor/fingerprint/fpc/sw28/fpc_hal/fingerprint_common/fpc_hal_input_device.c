/************************************************************************************
 ** File: - fpc_hal_input_device.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE hal nav input for FPC (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Ziqing.guo   2017/10/13        create the file
 **  Ziqing.guo   2017/10/21        customization for homekey
 **  Ziqing.guo   2019/01/07        remove rebundant code
 ************************************************************************************/

#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef FPC_CONFIG_NAVIGATION
#include "fpc_nav_types.h"
#endif
#include "fpc_types.h"
#include "fpc_log.h"
#include "fpc_hal_input_device.h"
#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
#include "fpc_hal_sense_touch_types.h"
#endif

#define FPC_INPUT_DEVICE_NAME "uinput-fpc"
#define FPC_UINPUT_CHAR_DEVICE "/dev/uinput"

static int device_fd = -1; /* Device File Descriptior */

static int32_t send_event(struct input_event *event)
{
    int32_t status = 0;

    if (write(device_fd, event, sizeof(struct input_event)) != sizeof(struct input_event)) {
        LOGE("%s - Error: %d during write. Failed to report event type: %u, code: %u,"
                " value:%d", __func__, errno, event->type, event->code, event->value);
        status = -FPC_ERROR_IO;
        goto exit;
    }

    event->type = EV_SYN;
    event->code = SYN_REPORT;
    event->value = 0;
    if (write(device_fd, event, sizeof(struct input_event)) != sizeof(struct input_event)) {
        LOGE("%s - Error: %d during write. Failed to report event buffer.", __func__, errno);
        status = -FPC_ERROR_IO;
        goto exit;
    }

exit:
    return status;
}

int32_t create_input_device(void)
{
    int status;

    if (device_fd != -1) {
        LOGE("%s - Input device already created please revise API usage.", __func__);
        status = -FPC_ERROR_STATE;
        goto exit;
    }

    device_fd = open(FPC_UINPUT_CHAR_DEVICE, O_WRONLY | O_NONBLOCK);
    if (device_fd == -1) {
        LOGE("%s - Error during open when trying to setup interface to the linux kernel"
                " input subsystem, code: %d", __func__, errno);
        status = -FPC_ERROR_IO;
        goto exit;
    }

    unsigned int keys[] = {
        KEY_HOMEPAGE,
#ifdef SIDE_FPC_ENABLE
        KEY_CAMERA,
#endif
    };

    if (ioctl(device_fd, UI_SET_EVBIT, EV_KEY) < 0) {
        LOGE("%s - Error during ioctl when setting up key events, code: %d", __func__, errno);
        status = -FPC_ERROR_IO;
        goto destroy;
    }

    for (unsigned int *ev = keys; ev != keys + sizeof(keys) / sizeof(keys[0]); ++ev) {
        if (ioctl(device_fd, UI_SET_KEYBIT, *ev) < 0) {
            LOGE("%s - Error during ioctl when registering key event: %u, code: %d",
                    __func__, *ev, errno);
            status = -FPC_ERROR_IO;
            goto destroy;
        }
    }

    unsigned int abs[] = {
        ABS_Z,
    };

    if (ioctl(device_fd, UI_SET_EVBIT, EV_ABS) < 0) {
        LOGE("%s - Error during ioctl when setting up absolute events, code: %d",
                __func__, errno);
        status = -FPC_ERROR_IO;
        goto destroy;
    }

    for (unsigned int *ev = abs; ev != abs + sizeof(abs) / sizeof(abs[0]); ++ev) {
        if (ioctl(device_fd, UI_SET_ABSBIT, *ev) < 0) {
            LOGE("%s - Error during ioctl when registering abs event: %u, code: %d",
                    __func__, *ev, errno);
            status = -FPC_ERROR_IO;
            goto destroy;
        }
    }

    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0;
    uidev.id.product = 0;
    uidev.id.version = 0;


    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, FPC_INPUT_DEVICE_NAME);

    if (write(device_fd, &uidev, sizeof(uidev)) != sizeof(uidev)) {
        LOGE("%s - Error during write when setting up device conf, code: %d", __func__, errno);
        status = -FPC_ERROR_IO;
        goto destroy;
    }

    if (ioctl(device_fd, UI_DEV_CREATE) < 0) {
        LOGE("%s - Error during ioctl when trying to create device, code: %d", __func__, errno);
        status = -FPC_ERROR_IO;
        goto destroy;
    }

    status = 0; /* OK */
    goto exit;
destroy:
    destroy_input_device();
exit:
    return status;
}

int32_t destroy_input_device(void)
{
    if (device_fd != -1) {
        if (ioctl(device_fd, UI_DEV_DESTROY) < 0) {
            LOGD("%s - Error during ioctl when trying to destory input device, code: %d",
                    __func__, errno);
        }

        if (close(device_fd)) {
            LOGE("%s - Error when closing device_fd for input device, code: %d", __func__, errno);
        }
        device_fd = -1;
    }
    return 0;
}

int32_t report_input_event(trigger_event_type_t trigger_event_type, uint32_t trigger_event,
        int32_t event_value)
{
    int32_t status = 0;
    struct input_event event;
    memset(&event, 0, sizeof(event));

    /* Create a unique has identifier for the trigger event. */
    uint32_t trigger_code = (trigger_event_type | trigger_event);

    switch (trigger_code) {
#ifdef FPC_CONFIG_NAVIGATION
        case (FPC_NAV_EVENT | FPC_NAV_EVENT_SINGLE_CLICK):
            event.type = EV_KEY;
            event.code = BTN_A;
            event.value = event_value;
            break;
        case (FPC_NAV_EVENT | FPC_NAV_EVENT_DOUBLE_CLICK):
            event.type = EV_KEY;
            event.code = BTN_C;
            event.value = event_value;
            break;
        case (FPC_NAV_EVENT | FPC_NAV_EVENT_HOLD_CLICK):
            event.type = EV_KEY;
            event.code = BTN_B;
            event.value = event_value;
            break;
        case (FPC_NAV_EVENT | FPC_NAV_EVENT_SLIDE_UP):
            event.type = EV_KEY;
            event.code = KEY_UP;
            event.value = event_value;
            break;
        case (FPC_NAV_EVENT | FPC_NAV_EVENT_SLIDE_DOWN):
            event.type = EV_KEY;
            event.code = KEY_DOWN;
            event.value = event_value;
            break;
        case (FPC_NAV_EVENT | FPC_NAV_EVENT_SLIDE_LEFT):
            event.type = EV_KEY;
            event.code = KEY_LEFT;
            event.value = event_value;
            break;
        case (FPC_NAV_EVENT | FPC_NAV_EVENT_SLIDE_RIGHT):
            event.type = EV_KEY;
            event.code = KEY_RIGHT;
            event.value = event_value;
            break;
#endif
#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
        case (FPC_SENSE_TOUCH_EVENT | FPC_SENSE_TOUCH_RAW):
            event.type = EV_ABS;
            event.code = ABS_Z;
            event.value = event_value;
            break;
        case (FPC_SENSE_TOUCH_EVENT | FPC_SENSE_TOUCH_PRESS):
            event.type = EV_KEY;
            event.code = BTN_X;
            event.value = event_value;
            break;
        case (FPC_SENSE_TOUCH_EVENT | FPC_SENSE_TOUCH_AUTH_PRESS):
            event.type = EV_KEY;
            event.code = BTN_Y;
            event.value = event_value;
            break;
#endif
#ifdef FPC_CONFIG_NAVIGATION
        case (FPC_HOMEKEY_EVENT | FPC_NAV_EVENT_NONE):
            event.type = EV_KEY;
            event.code = KEY_HOMEPAGE;
            event.value = event_value;
            break;
#ifdef SIDE_FPC_ENABLE
            case (FPC_CAMERAKEY_EVENT | FPC_NAV_EVENT_NONE):
            event.type = EV_KEY;
            event.code = KEY_CAMERA;
            event.value = event_value;
            break;
#endif
#endif
        default:
            (void)event_value;
            LOGE("%s - Unrecognized trigger_event: %u, trigger_event_type: %u", __func__,
                    trigger_event, trigger_event_type);
            status = -FPC_ERROR_PARAMETER;
            break;
    }

    if (event.code) {
        LOGD("%s - Reporting event type: %u, code: %u, value:%d", __func__, event.type,
                event.code, event.value);
        status = send_event(&event);
    }

    return status;
}
