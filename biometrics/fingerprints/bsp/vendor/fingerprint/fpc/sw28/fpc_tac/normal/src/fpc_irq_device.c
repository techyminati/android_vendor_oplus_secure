/************************************************************************************
 ** File: - fpc_irq_device.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      fpc irq API  (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,21/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 **
 ** --------------------------- Revision History: --------------------------------
 ** 	<author>	     <data>			<desc>
 **    Ziqing.guo      2017/10/21       create file
 **    Ziqing.guo      2017/10/22       add acquire and release wakelock to avoid going to sleep
 **    Long.Liu        2018/11/19       modify for 18531 static test
 ************************************************************************************/

#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "fpc_tee_sensor.h"
#include "fpc_irq_device.h"
#include "fpc_log.h"
#include "fpc_types.h"

#include "fpc_sysfs.h"

struct fpc_irq {
    int sysfs_fd;
    int cancel_fds[2];
};

void fpc_irq_release(fpc_irq_t* device)
{
    if (!device) {
        return;
    }

    if (device->sysfs_fd != -1) {
        close(device->sysfs_fd);
    }

    if (device->cancel_fds[0] != -1) {
        close(device->cancel_fds[0]);
    }

    if (device->cancel_fds[1] != -1) {
        close(device->cancel_fds[1]);
    }

    free(device);
}

fpc_irq_t* fpc_irq_init(void)
{
    fpc_irq_t* device = malloc(sizeof(fpc_irq_t));

    if (!device) {
        goto err;
    }

    device->sysfs_fd = -1;
    device->cancel_fds[0] = -1;
    device->cancel_fds[1] = -1;

    char path[PATH_MAX];
    if (!fpc_sysfs_path_by_attr(FPC_REE_DEVICE_ALIAS_FILE, FPC_REE_DEVICE_NAME, FPC_REE_DEVICE_PATH,
                                path, PATH_MAX)) {
        LOGE("%s Error didn't find phys path device", __func__);
        goto err;
    }

    device->sysfs_fd = open(path, O_RDONLY);

    if (device->sysfs_fd == -1) {
        LOGE("%s open %s failed %i", __func__, path, errno);
        goto err;
    }

    if (pipe(device->cancel_fds)) {
        goto err;
    }

    return device;

err:

    fpc_irq_release(device);
    return NULL;
}

#ifdef FPC_CONFIG_SEND_RESET
static uint64_t current_ms_time(void)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);

    return (t1.tv_sec * 1000) + (t1.tv_usec / 1000);
}
#endif

int fpc_irq_wait(fpc_irq_t* device, int irq_value)
{
    int irq_fd = -1;
    int status = 0;
#ifdef FPC_CONFIG_SEND_RESET
    int has_real_interrupt = 0;
    uint64_t start_time = 0;
    uint64_t end_time = 0;
    uint64_t cost_time = 0;
#endif
    fpc_sysfs_node_write(device->sysfs_fd, "irq_enable", "1");

    for(;;) {
        irq_fd = openat(device->sysfs_fd, "irq", O_RDONLY | O_NONBLOCK);
        if (irq_fd == -1) {
            LOGE("%s openat failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        }

        char value = 0;

        status = read(irq_fd, &value, sizeof(value));

        if (status < 0) {
            LOGE("%s read failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        } else if (status == 0) {
            status = -FPC_ERROR_IO;
            goto out;
        }

        if ((value - '0') == irq_value) {
            status = 0;
            goto out;
        }

#ifdef FPC_CONFIG_SEND_RESET
        if (irq_value == 1 && has_real_interrupt == 1) {
            has_real_interrupt = 0;
            fpc_sysfs_node_write(device->sysfs_fd, "irq_unexpected", "1");
            LOGE("%s, abnormal interrupt reset completed status=%d\n", __func__, status);
        }
        start_time = current_ms_time();
#endif

        struct pollfd pfd[2];
        pfd[0].fd = irq_fd;
        pfd[0].events = POLLERR | POLLPRI;
        pfd[0].revents = 0;
        pfd[1].fd = device->cancel_fds[0];
        pfd[1].events = POLLIN;
        pfd[1].revents = 0;
        //ziqing.guo add for enable fingerprint irq
        //fpc_sysfs_node_write(device->sysfs_fd, "irq_enable", "1");

        status = poll(pfd, 2, -1);

#ifdef FPC_CONFIG_SEND_RESET
        end_time = current_ms_time();
        has_real_interrupt = 0;
        cost_time = end_time - start_time;
        if (cost_time > 20) {
            has_real_interrupt = 1;
        }
#endif

        /* acknowledge that poll returned, can be used to
         * measure latency from the kernel driver. */
        fpc_sysfs_node_write(device->sysfs_fd, "irq", "1");

        if (status == -1) {
            LOGE("%s poll failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        } else if (pfd[1].revents) {
            LOGE("%s {{%d,%d,%x},{%d,%d,%x}}", __func__,
                 pfd[0].fd, pfd[0].events, pfd[0].revents,
                 pfd[1].fd, pfd[1].events, pfd[1].revents);
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }
        close(irq_fd);
    }
out:
    if (irq_fd != -1) {
        close(irq_fd);
    }

    if (status != 0) {
        LOGE("%s error %i", __func__, status);
    }
    //ziqing.guo add for disable fingerprint irq
    fpc_sysfs_node_write(device->sysfs_fd, "irq_enable", "0");

    return status;
}

int fpc_irq_wait_timeout(fpc_irq_t *device, int irq_value, int timeout)
{
    int irq_fd = -1;
    int status = 0;

    for (;;) {
        irq_fd = openat(device->sysfs_fd, "irq", O_RDONLY | O_NONBLOCK);
        if (irq_fd == -1) {
            LOGE("%s openat failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        }

        char value = 0;

        status = read(irq_fd, &value, sizeof(value));

        if (status < 0) {
            LOGE("%s read failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        } else if (status == 0) {
            status = -FPC_ERROR_IO;
            goto out;
        }

        if ((value - '0') == irq_value) {
            status = 0;
            goto out;
        }

        struct pollfd pfd[2];
        pfd[0].fd = irq_fd;
        pfd[0].events = POLLERR | POLLPRI;
        pfd[0].revents = 0;
        pfd[1].fd = device->cancel_fds[0];
        pfd[1].events = POLLIN;
        pfd[1].revents = 0;

        status = poll(pfd, 2, timeout);
        LOGE("%s poll return status %d error %i", __func__, status, -errno);

        /* acknowledge that poll returned, can be used to
         * measure latency from the kernel driver. */
        fpc_sysfs_node_write(device->sysfs_fd, "irq", "1");

        if (status == -1) {
            LOGE("%s poll failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        } else if (status == 0) {
            LOGE("%s poll timeout with error %i", __func__, -errno);
            status = -FPC_ERROR_TIMEDOUT;
            goto out;
        } else if (pfd[1].revents) {
            LOGE("%s {{%d,%d,%x},{%d,%d,%x}}", __func__,
                 pfd[0].fd, pfd[0].events, pfd[0].revents,
                 pfd[1].fd, pfd[1].events, pfd[1].revents);
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }
        close(irq_fd);
    }
out:
    if (irq_fd != -1) {
        close(irq_fd);
    }

    if (status != 0) {
        LOGE("%s error %i", __func__, status);
    }

    return status;
}

int fpc_irq_set_cancel(fpc_irq_t* device)
{
    int status = 0;
    uint8_t byte = 1;
    LOGI("%s", __func__);
    if (write(device->cancel_fds[1], &byte, sizeof(byte)) != sizeof(byte)) {
        LOGE("%s write failed %i", __func__, errno);
        status = -FPC_ERROR_IO;
    }

    return status;
}

int fpc_irq_clear_cancel(fpc_irq_t* device)
{
    int status = 0;
    uint8_t byte;
    LOGI("%s", __func__);
    if (read(device->cancel_fds[0], &byte, sizeof(byte)) < 0) {
        LOGE("%s read failed %i", __func__, errno);
        status = -FPC_ERROR_IO;
    }

    return status;
}

int fpc_irq_status(fpc_irq_t* device)
{
    int irq_fd = -1;
    int status = 0;
    char value = 0;

    irq_fd = openat(device->sysfs_fd, "irq", O_RDONLY | O_NONBLOCK);
    if (irq_fd == -1) {
        LOGE("%s openat failed with error %s", __func__, strerror(-errno));
        status = -FPC_ERROR_IO;
        goto out;
    }

    // Value will be the IRQ pin value, status will be the amount of bytes read
    status = read(irq_fd, &value, sizeof(value));
    if (status < 0) {
        LOGE("%s read failed with error %s", __func__, strerror(-errno));
        status = -FPC_ERROR_IO;
        goto out;
    } else if (status == 0) {
        LOGE("%s no bytes available to read from irq node", __func__);
        status = -FPC_ERROR_IO;
        goto out;
    }

    status = value - '0';
    if (status < 0 || status > 9) {
        LOGE("%s got a strange value from irq node: %d", __func__, status);
        status = -FPC_ERROR_IO;
    }

out:
    if (irq_fd != -1) {
        close(irq_fd);
    }

    return status;
}

int fpc_irq_wakeup_enable(fpc_irq_t *device) {
    //return fpc_sysfs_node_write(device->sysfs_fd, "wakeup_enable", "enable");
    (void)device;
    return 0;
}

int fpc_irq_wakeup_disable(fpc_irq_t *device) {
    //return fpc_sysfs_node_write(device->sysfs_fd, "wakeup_enable", "disable");
    (void)device;
    return 0;
}

#ifdef FPC_CONFIG_WAKE_LOCK

int fpc_wakeup_enable(fpc_irq_t *device) {
    return fpc_sysfs_node_write(device->sysfs_fd, "wakelock_enable", WAKELOCK_ENABLE);
}

int fpc_wakeup_disable(fpc_irq_t *device) {
    return fpc_sysfs_node_write(device->sysfs_fd, "wakelock_enable", WAKELOCK_DISABLE);
}

#endif /*OPLUS_FEATURE_FINGERPRINT*/
static int fpc_sysfs_get_base_fd(void)
{
    int sysfs_base_fd;
    char path[PATH_MAX];

    if (!fpc_sysfs_path_by_attr(FPC_REE_DEVICE_ALIAS_FILE, FPC_REE_DEVICE_NAME, FPC_REE_DEVICE_PATH,
                                path, PATH_MAX)) {
        LOGE("%s Error finding phys path device\n", __func__);
        return -1;
    }

    sysfs_base_fd = open(path, O_RDONLY);
    if (sysfs_base_fd == -1) {
        LOGE("%s Error opening %s\n", __func__, path);
        return -1;
    }

    return sysfs_base_fd;
}

int fpc_power_enable_by_sysfs(void)
{
    int sysfs_fd;
    int rc;

    sysfs_fd = fpc_sysfs_get_base_fd();
    if (sysfs_fd == -1) {
        return -FPC_ERROR_IO;
    }

    rc = fpc_sysfs_node_write(sysfs_fd, "regulator_enable", "1");
    close(sysfs_fd);
    return rc;
}

int fpc_power_disable_by_sysfs(void)
{
    int sysfs_fd;
    int rc;

    sysfs_fd = fpc_sysfs_get_base_fd();
    if (sysfs_fd == -1) {
        return -FPC_ERROR_IO;
    }

    rc = fpc_sysfs_node_write(sysfs_fd, "regulator_enable", "0");
    close(sysfs_fd);
    return rc;
}

