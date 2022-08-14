/************************************************************************************
 ** File: - fpc_hal_navigation.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE hal nav for FPC (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Ziqing.guo   2017/10/13        create the file
 **  Ziqing.guo   2017/10/21        customization for homekey
 **  Ziqing.guo   2017/10/22        restart nav_loop(20 times max) while it exits with error IO as returning value
 ************************************************************************************/
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "fpc_hal_ext_navigation.h"
#include "fpc_hal_navigation.h"
#include "fpc_log.h"
#include "fpc_worker.h"
#include "fpc_irq_device.h"
#include "fpc_tee_nav.h"
#include "fpc_types.h"
#include "fpc_nav_types.h"
#include "fpc_tee.h"
#include "fpc_hal_input_device.h"
#include "fpc_tee_hal.h"
#ifdef FPC_CONFIG_FORCE_SENSOR
#include "fpc_hal_sense_touch.h"
#endif
#ifdef FPC_CONFIG_NAVIGATION_FORCE_SW
#include "fpc_hal_sense_touch_types.h"
#endif
#ifndef SIDE_FPC_ENABLE
#include "fpc_hal_private.h"
#endif
#ifndef FPC_NAV_DEBUG_SAVE_FILE_PATH
#define FPC_NAV_DEBUG_SAVE_FILE_PATH "data/fpc/"
#endif

#define USECS_EVERY_MSEC 1000
#define HOMEKEY_RETRY_MAX_TIMES 20
#define HOMEKEY_EXIT_ERROR_SLEEP_TIME_MS 200

#ifdef SIDE_FPC_ENABLE
#define FINGERPRINT_TOUCH_CAMERA
#endif

typedef struct {
    fpc_navigation_t navigation;
    pthread_mutex_t mutex;
    pthread_mutex_t cancel_mutex;
    fpc_tee_t* tee;
    fpc_irq_t* irq;
    fpc_worker_t* worker_thread;
    bool enabled;
    bool paused;
    bool cancel;
    fpc_nav_config_t config;
    bool configured;
#ifdef FPC_CONFIG_FORCE_SENSOR
    const fpc_sense_touch_config_t* st_config;
#endif
    bool sensetouch_enabled;
    uint32_t debug_buffer_size;
    uint8_t *debug_buffer;
    fpc_hal_common_t* hal;
} nav_module_t;

#ifndef FINGERPRINT_TOUCH_CAMERA
static uint64_t current_u_time(void)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    return (t1.tv_sec * 1000 * 1000) + (t1.tv_usec);
}

static int nav_debug_check_dir(void)
{
    struct stat st;
    memset(&st, 0, sizeof(st));
    int status = stat(FPC_NAV_DEBUG_SAVE_FILE_PATH, &st);
    if (status) {
        LOGE("%s %s not available %d", __func__, FPC_NAV_DEBUG_SAVE_FILE_PATH, errno);
    }
    return status;
}

static int nav_debug_save_data(uint8_t *nav_data, uint32_t nav_data_size)
{
    char curr_time[] = "00-00-00-00-00-00.000000";
    char fname[] = FPC_NAV_DEBUG_SAVE_FILE_PATH "fpc_" "00-00-00-00-00-00.000000" ".nav_dat";
    struct timeval t1;

    if (gettimeofday(&t1, NULL)) {
        LOGE("%s unable to get wall time, skip saving debug buffer %d", __func__, errno);
        return -1;
    }

    strftime(curr_time, sizeof(curr_time), "%y-%m-%d-%H-%M-%S", localtime(&t1.tv_sec));
    snprintf(fname, sizeof(fname), FPC_NAV_DEBUG_SAVE_FILE_PATH "fpc_%s.%06ld.nav_dat", curr_time, t1.tv_usec);
    FILE *debug_fd = fopen(fname, "wb");
    if (!debug_fd) {
        LOGE("%s unable to open file for writing %d", __func__, errno);
        return -1;
    }

    size_t bytes_written = fwrite(nav_data, 1, nav_data_size, debug_fd);
    LOGD("%s created new file %s, OK(1=yes,0=no): %d", __func__, fname,
         (bytes_written == nav_data_size));
    fclose(debug_fd);

    return (bytes_written == nav_data_size) ? 0 : -1;
}

static void nav_debug_retrieve_and_save_data(nav_module_t *module)
{
    int status = 0;
    LOGD("%s retrieving and saving debug data for nav", __func__);
    if (nav_debug_check_dir() && !module->debug_buffer) {
        goto exit;
    }

    memset(module->debug_buffer, 0, module->debug_buffer_size);

    status = fpc_tee_nav_get_debug_buffer(module->tee,
                                          module->debug_buffer,
                                          &module->debug_buffer_size);
    if (status) {
        LOGE("%s failed to retrieve debug buffer: %d", __func__, status);
        goto exit;
    }

    status = nav_debug_save_data(module->debug_buffer, module->debug_buffer_size);
    if (status) {
        LOGE("%s failed to save debug buffer: %d", __func__, status);
    }

exit:
    return;
}
#endif

static void do_nav(void* data);

static int nav_loop(nav_module_t* module) {
    LOGD("%s", __func__);

#ifdef SIDE_FPC_ENABLE
    int status = fpc_tee_nav_init(module->tee);
#else
    int status = fpc_tee_wait_finger_lost(&module->hal->sensor);
#endif
    if (status) {
        goto out;
    }
#ifdef SIDE_FPC_ENABLE
    status = fpc_tee_wait_finger_lost(&module->hal->sensor);
#else
    status = fpc_tee_nav_init(module->tee);
#endif
    if (status) {
        goto out;
    }

    for (;;) {
        pthread_mutex_lock(&module->cancel_mutex);
        bool cancel = module->cancel;
        pthread_mutex_unlock(&module->cancel_mutex);

        if (cancel) {
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }

        status = nav_wait_finger_down(module->tee, module->irq);
        if (status) {
            goto out;
        }
#ifdef SIDE_FPC_ENABLE
        report_input_event(FPC_CAMERAKEY_EVENT, FPC_NAV_EVENT_NONE, FPC_HAL_INPUT_KEY_DOWN);

        status = nav_wait_finger_up(module->tee, &module->hal->sensor, module->irq);
        if (status) {
            report_input_event(FPC_CAMERAKEY_EVENT, FPC_NAV_EVENT_NONE, FPC_HAL_INPUT_KEY_UP);
            goto out;
        }

        report_input_event(FPC_CAMERAKEY_EVENT, FPC_NAV_EVENT_NONE, FPC_HAL_INPUT_KEY_UP);
    
#else
        report_input_event(FPC_HOMEKEY_EVENT, FPC_NAV_EVENT_NONE, FPC_HAL_INPUT_KEY_DOWN);

        status = nav_wait_finger_up(module->tee, &module->hal->sensor, module->irq);
        if (status) {
            report_input_event(FPC_HOMEKEY_EVENT, FPC_NAV_EVENT_NONE, FPC_HAL_INPUT_KEY_UP);
            goto out;
        }

        report_input_event(FPC_HOMEKEY_EVENT, FPC_NAV_EVENT_NONE, FPC_HAL_INPUT_KEY_UP);
    
#endif
    }
out:
    fpc_tee_nav_exit(module->tee);
    return status;
}

static void do_nav(void* data) {
    int status = 0;
    int homekey_errio_counter = 0;
    nav_module_t* module = (nav_module_t*) data;

    for (homekey_errio_counter = 1 ; homekey_errio_counter <= HOMEKEY_RETRY_MAX_TIMES ; homekey_errio_counter++) {
        LOGD("%s executing nav_loop the %dth time", __func__ , homekey_errio_counter);
        status = nav_loop(module);
        if (status == -FPC_ERROR_IO) {
            LOGE("%s retry with error IO", __func__);
            usleep(HOMEKEY_EXIT_ERROR_SLEEP_TIME_MS * USECS_EVERY_MSEC);
        } else {
            if (status && status != -FPC_ERROR_CANCELLED) {
                LOGE("%s exit with error %i", __func__, status);
            }
            break;
        }
    }
}

static void set_config(fpc_navigation_t *self, const fpc_nav_config_t *config)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);
    nav->config = *config;
    nav->configured = true;
    pthread_mutex_unlock(&nav->mutex);
}

static void get_config(fpc_navigation_t *self, fpc_nav_config_t *config)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);
    *config = nav->config;
    pthread_mutex_unlock(&nav->mutex);
}

static void cancel(nav_module_t *nav)
{
    /* Set cancel states */
    fpc_irq_set_cancel(nav->irq);
    pthread_mutex_lock(&nav->cancel_mutex);
    nav->cancel = true;
    pthread_mutex_unlock(&nav->cancel_mutex);
    fpc_worker_join_task(nav->worker_thread);

    /* Reset cancel states */
    pthread_mutex_lock(&nav->cancel_mutex);
    nav->cancel = false;
    pthread_mutex_unlock(&nav->cancel_mutex);
    fpc_irq_clear_cancel(nav->irq);
}

static void set_enabled(fpc_navigation_t *self, bool enabled)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);

    bool is_running = nav->enabled && !nav->paused;
    bool should_run = enabled && !nav->paused;

    if (should_run && !is_running) {
        fpc_worker_run_task(nav->worker_thread, do_nav, nav);
    } else if (!should_run && is_running) {
        cancel(nav);
    }

    nav->enabled = enabled;

    pthread_mutex_unlock(&nav->mutex);
}

static bool get_enabled(fpc_navigation_t *self)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);
    bool enabled = nav->enabled;
    pthread_mutex_unlock(&nav->mutex);
    return enabled;
}

void fpc_navigation_resume(fpc_navigation_t *self)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);

    if (nav->enabled && nav->paused) {
        fpc_worker_run_task(nav->worker_thread, do_nav, nav);
    }
    nav->paused = false;
    pthread_mutex_unlock(&nav->mutex);
}

void fpc_navigation_pause(fpc_navigation_t *self)
{
    LOGD("%s", __func__);
    nav_module_t* nav = (nav_module_t*) self;
    pthread_mutex_lock(&nav->mutex);

    if (nav->enabled && !nav->paused) {
        cancel(nav);
    }

    nav->paused = true;
    pthread_mutex_unlock(&nav->mutex);
}

fpc_navigation_t *fpc_navigation_new(fpc_hal_common_t *hal)
{
    nav_module_t *module = (nav_module_t *) calloc(sizeof(nav_module_t), 1);
    if (!module) {
        return NULL;
    }

    pthread_mutex_init(&module->mutex, NULL);
    pthread_mutex_init(&module->cancel_mutex, NULL);

    module->irq = fpc_irq_init();
    if (!module->irq) {
        goto err;
    }

    module->tee = hal->tee_handle;
#ifdef FPC_CONFIG_FORCE_SENSOR
    module->st_config = fpc_sense_touch_get_config();
#endif

    module->worker_thread = fpc_worker_new();
    if (!module->worker_thread) {
        goto err;
    }

    module->hal = hal;
    module->navigation.set_config = set_config;
    module->navigation.get_config = get_config;
    module->navigation.set_enabled = set_enabled;
    module->navigation.get_enabled = get_enabled;
#ifdef SIDE_FPC_ENABLE
    module->enabled = false;
#else
    module->enabled = true;
#endif
    module->paused = true;

    return (fpc_navigation_t *) module;

err:
    fpc_navigation_destroy((fpc_navigation_t*) module);

    return NULL;
}

void fpc_navigation_destroy(fpc_navigation_t *self)
{
    if (!self) {
        return;
    }

    nav_module_t* module = (nav_module_t*) self;

    fpc_irq_release(module->irq);
    fpc_worker_destroy(module->worker_thread);
    pthread_mutex_destroy(&module->mutex);
    pthread_mutex_destroy(&module->cancel_mutex);
    free(module);
}
