/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include "fpc_hal_ext_sense_touch.h"
#include "fpc_hal_sense_touch.h"
#include "fpc_hal_sense_touch_types.h"
#include "fpc_tee_sensor.h"
#include "fpc_hal_private.h"
#include "fpc_log.h"
#include "fpc_types.h"

#define FPC_HAL_SENSE_TOUCH_UNTRIGGER_MODIFIER 0.80f

typedef struct {
    fpc_sense_touch_t sense_touch;
    pthread_mutex_t mutex;
    fpc_hal_common_t *hal;
} sense_touch_module_t;

static int get_force(fpc_sense_touch_t *self, uint8_t *force)
{
    LOGD("%s", __func__);

    sense_touch_module_t *module = (sense_touch_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    int status = fpc_tee_get_sensor_force_value(module->hal->sensor, force);

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    if (status) {
        LOGE("%s Error getting sensor force value", __func__);
    }

    return status;
}

static int is_supported(fpc_sense_touch_t *self, uint8_t *result)
{
    LOGD("%s", __func__);

    sense_touch_module_t *module = (sense_touch_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    int status = fpc_tee_is_sensor_force_supported(module->hal->sensor, result);

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    if (status) {
        LOGE("%s Error checking sensor force support", __func__);
    }

    return status;
}

static int store_calibration_data(uint8_t ground, uint8_t threshold)
{
    LOGD("%s(ground: %d, threshold: %d)", __func__, ground, threshold);
    int status = 0;


    char *path = SENSE_TOUCH_CALIBRATION_PATH;
    int fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        LOGE("%s  failed to open file, error: %s", __func__, strerror(errno));
        status = -FPC_ERROR_IO;
        goto exit;
    }

    fpc_sense_touch_config_t st_config;
    st_config.version = FPC_SENSE_TOUCH_VERSION_1;
    st_config.ground = ground;
    st_config.trigger_threshold = threshold;
    st_config.untrigger_threshold = (threshold * FPC_HAL_SENSE_TOUCH_UNTRIGGER_MODIFIER);
    st_config.auth_enable_down_force = 0;
    st_config.auth_enable_up_force = 0;
    st_config.auth_button_timeout_ms = 0;

    if (write(fd, &st_config, sizeof(st_config)) != sizeof(st_config)) {
        LOGE("%s failed to store calibration data %s", __func__, strerror(errno));
        status = -FPC_ERROR_IO;
        goto exit;
    }

    status = fpc_sense_touch_load_config();
exit:
    if (fd != -1) {
        close(fd);
    }

    return status;
}

static int set_auth_mode(bool enable_down_force,
                         bool enable_up_force,
                         uint32_t button_timeout_ms)
{
    LOGD("%s", __func__);
    fpc_sense_touch_set_auth_mode(enable_down_force,
                                  enable_up_force,
                                  button_timeout_ms);
    return 0;
}

static int32_t read_config(const fpc_sense_touch_config_t **st_config)
{
    LOGD("%s", __func__);
    int32_t status = 0;
    *st_config = fpc_sense_touch_get_config();

    if (!*st_config) {
        LOGE("%s Error failed to read sense touch config, no config available.", __func__);
        status = -FPC_ERROR_NOT_FOUND;
    }

    return status;
}

fpc_sense_touch_t *fpc_sense_touch_new(fpc_hal_common_t *hal)
{
    LOGD("%s", __func__);

    sense_touch_module_t *module = malloc(sizeof(sense_touch_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(sense_touch_module_t));
    module->hal = hal;
    module->sense_touch.get_force = get_force;
    module->sense_touch.is_supported = is_supported;
    module->sense_touch.store_calibration_data = store_calibration_data;
    module->sense_touch.set_auth_mode = set_auth_mode;
    module->sense_touch.read_config = read_config;

    pthread_mutex_init(&module->mutex, NULL);

    return (fpc_sense_touch_t *) module;
}

void fpc_sense_touch_destroy(fpc_sense_touch_t *self)
{
    LOGD("%s", __func__);

    if (!self) {
        return;
    }

    sense_touch_module_t *module = (sense_touch_module_t *) self;
    pthread_mutex_destroy(&module->mutex);
    free(self);
}
