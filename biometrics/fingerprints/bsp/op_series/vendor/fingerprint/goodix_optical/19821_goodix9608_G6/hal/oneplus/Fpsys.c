#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include "HalLog.h"

#define LOG_TAG "[GF_HAL][Fpsys]"

#define FP_CONFIG_ENABLE_TPIRQ_PATH "/proc/touchpanel/fp_enable"
#define FP_CONFIG_SCREEN_AOD_PATH  "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/drm/card0/card0-DSI-1/aod"
#define FP_CONFIG_SCREEN_DIM_PATH  "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/drm/card0/card0-DSI-1/notify_dim"

static uint32_t mFingerReady = 0;  // mFingerReady

int32_t fp_set_tpirq_enable(uint32_t mode) {
    char buff[2];
    int fd = open(FP_CONFIG_ENABLE_TPIRQ_PATH, O_WRONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "fp_set_touch_irq_enable fail!");
        return fd;
    }

    LOG_D(LOG_TAG, "fp_set_tpirq_enable %d", mode);
    snprintf(buff, sizeof(buff), "%d", (int)mode);
    write(fd, buff, 2);
    close(fd);
    return fd;
}

int32_t notify_finger_ready() {
    LOG_D(LOG_TAG, "notify finger ready");
    mFingerReady = 1;
    return 0;
}

int32_t clear_finger_ready_flag() {
    LOG_D(LOG_TAG, "clear finger ready flag");
    mFingerReady = 0;
    return 0;
}

int32_t is_finger_ready() {
    return mFingerReady;
}

gf_error_t wait_for_finger_ready() {
    int iRetry = 200;
    while (iRetry > 0) {
        if (is_finger_ready()) {
            // clear_finger_ready_flag();
            return GF_SUCCESS;
        }
        iRetry--;
        usleep(100 * 25);  // 2.5ms
    }
    iRetry--;
    if (iRetry < 0) {
        LOG_D(LOG_TAG, "UI timeout");
    }
    clear_finger_ready_flag();

    return GF_ERROR_UI_READY_TIMEOUT;
}

int32_t fp_read_aod_mode(void) {
    int32_t fd = -1;
    int32_t status = 0;
    char value[8] = "0";
    fd = open(FP_CONFIG_SCREEN_AOD_PATH, O_RDONLY);
    if (fd == -1) {
        status = -errno;
        LOG_D(LOG_TAG, "open aod node failed %d", status);
        goto out;
    }
    status = read(fd, value, sizeof(value));
    if (status < 0) {
        status = -errno;
        LOG_D(LOG_TAG, "read aod node failed");
        goto out;
    } else if (status == 0) {
        status = -ENOSYS;
        LOG_D(LOG_TAG, "read aod node err");
        goto out;
    } else {
        LOG_D(LOG_TAG, "read aod node sucessed %s", value);
        status = atoi(value);
    }

out:
    if (fd != -1) {
        close(fd);
    }

    return status;
}

int32_t fp_set_dim_layer(uint32_t value) {
    char buff[50];
    int32_t fd = -1;

    LOG_D(LOG_TAG, "fp_set_dim_layer %d", value);

    fd = open(FP_CONFIG_SCREEN_DIM_PATH, O_WRONLY);
    if (fd < 0) {
        LOG_D(LOG_TAG, "open failed,dim node not exist!");
        return -1;
    }

    snprintf(buff, sizeof(buff), "%d", (int)value);
    write(fd, buff, 50);
    close(fd);

    return 0;
}

