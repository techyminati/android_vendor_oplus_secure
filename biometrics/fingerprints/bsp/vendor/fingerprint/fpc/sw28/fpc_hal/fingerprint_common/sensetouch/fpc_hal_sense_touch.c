/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/


#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "fpc_hal_sense_touch.h"
#include "fpc_log.h"
#include "container_of.h"
#include "fpc_tee_sensor.h"
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include "fpc_tee.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_worker.h"
#include "fpc_hal_private.h"
#include <cutils/properties.h>

static fpc_sense_touch_config_t sense_touch_config;
static bool config_loaded = false;

static void read_properties(fpc_sense_touch_config_t *st_config)
{
    char auth_mode[PROPERTY_VALUE_MAX];
    char button_timeout_ms[PROPERTY_VALUE_MAX];
    const char *default_value = "";

    property_get("persist.vendor.fpc.st.authmode", auth_mode, default_value);
    property_get("persist.vendor.fpc.st.timeout", button_timeout_ms, default_value);

    if (strncmp(default_value, auth_mode, PROPERTY_VALUE_MAX) != 0) {
        if (strncmp("normal", auth_mode, PROPERTY_VALUE_MAX) == 0) {
            st_config->auth_enable_down_force = 0;
            st_config->auth_enable_up_force = 0;
        } else if (strncmp("trigger", auth_mode, PROPERTY_VALUE_MAX) == 0) {
            st_config->auth_enable_down_force = 1;
            st_config->auth_enable_up_force = 0;
        } else if (strncmp("release", auth_mode, PROPERTY_VALUE_MAX) == 0) {
            st_config->auth_enable_down_force = 1;
            st_config->auth_enable_up_force = 1;
        } else {
            st_config->auth_enable_down_force = 0;
            st_config->auth_enable_up_force = 0;
            LOGD("%s Non supported SenseTouch authmode set, fall back to normal\n", __func__);
        }
    }

    if (strncmp(default_value, button_timeout_ms, PROPERTY_VALUE_MAX) != 0) {
        uint32_t timeout_ms = strtol(button_timeout_ms, NULL, 10);
        st_config->auth_button_timeout_ms = timeout_ms;
    }
}

int32_t fpc_sense_touch_load_config(void)
{
    LOGD("%s", __func__);
    int status = 0;
    char *path = SENSE_TOUCH_CALIBRATION_PATH;
    int fd = open(path, O_RDONLY);
    memset(&sense_touch_config, 0, sizeof(fpc_sense_touch_config_t));

    if (fd == -1) {
        LOGE("%s failed to open file, error: %s\n", __func__, strerror(errno));
        return -FPC_ERROR_IO;
    }

    if (read(fd, &sense_touch_config, sizeof(sense_touch_config)) != sizeof(sense_touch_config)) {
        LOGE("%s unable to load sense touch configuration: %s\n", __func__, strerror(errno));
        status = -FPC_ERROR_IO;
        goto exit;
    }

    if (sense_touch_config.version != FPC_SENSE_TOUCH_VERSION_1) {
        LOGE("%s loading aborted - version mismatch\n", __func__);
        status = -FPC_ERROR_IO;
        goto exit;
    }

    read_properties(&sense_touch_config);

    LOGD("%s loading complete - ground=%d, threshold=%d", __func__, sense_touch_config.ground,
         sense_touch_config.trigger_threshold);
    config_loaded = true;
exit:
    if (fd != -1) {
        if (close(fd)) {
            LOGE("%s failed to close file, error: %s\n", __func__, strerror(errno));
        }
    }

    return status;
}

const fpc_sense_touch_config_t *fpc_sense_touch_get_config(void)
{
    LOGD("%s", __func__);
    if (!config_loaded) {
        LOGE("%s failed to get sense touch config, config not loaded.", __func__);
        return NULL;
    }

    return &sense_touch_config;
}

void fpc_sense_touch_set_auth_mode(bool enable_down_force,
                                   bool enable_up_force,
                                   uint32_t button_timeout_ms)
{
    LOGD("%s", __func__);
    sense_touch_config.auth_enable_down_force = enable_down_force;
    sense_touch_config.auth_enable_up_force = enable_up_force;
    sense_touch_config.auth_button_timeout_ms = button_timeout_ms;
}
