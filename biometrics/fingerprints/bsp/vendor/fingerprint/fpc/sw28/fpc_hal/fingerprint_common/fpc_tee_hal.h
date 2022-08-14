/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TEE_HAL_H
#define FPC_TEE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>
#include <limits.h>
#include <container_of.h>
#include "fingerprint.h"

typedef enum {
    HAL_COMPAT_ERROR_NO_ERROR = 0,
    HAL_COMPAT_ERROR_HW_UNAVAILABLE = 1,
    HAL_COMPAT_ERROR_UNABLE_TO_PROCESS = 2,
    HAL_COMPAT_ERROR_TIMEOUT = 3,
    HAL_COMPAT_ERROR_NO_SPACE = 4,
    HAL_COMPAT_ERROR_CANCELED = 5,
    HAL_COMPAT_ERROR_UNABLE_TO_REMOVE = 6,
    HAL_COMPAT_ERROR_LOCKOUT = 7,
} fpc_hal_compat_error_t;

#define HAL_COMPAT_VENDOR_BASE 1000
#define HAL_COMPAT_ACQUIRED_TOO_SIMILAR (HAL_COMPAT_VENDOR_BASE + 1)
#define HAL_COMPAT_ACQUIRED_ALREADY_ENROLLED (HAL_COMPAT_VENDOR_BASE +2)
#define HAL_COMPAT_ERROR_ALIVE_CHECK 9000

typedef enum {
    HAL_COMPAT_ACQUIRED_GOOD = 0,
    HAL_COMPAT_ACQUIRED_PARTIAL = 1,
    HAL_COMPAT_ACQUIRED_INSUFFICIENT = 2,
    HAL_COMPAT_ACQUIRED_IMAGER_DIRTY = 3,
    HAL_COMPAT_ACQUIRED_TOO_SLOW = 4,
    HAL_COMPAT_ACQUIRED_TOO_FAST = 5,
} fpc_hal_compat_acquired_t;

typedef enum {
    HAL_LOAD_TEMPLATE_NORMAL = 0,
    HAL_LOAD_TEMPLATE_RESTORED_SUCCEE = 1,
    HAL_LOAD_TEMPLATE_RESTORED_FAIL_EMPTYDB_SUCC = 2,
    HAL_LOAD_TEMPLATE_RESTORED_FAIL_EMPTYDB_FAIL = 3,
} fpc_load_template_Status;

typedef struct {
    void (*on_enroll_result)(void* context, uint32_t fid, uint32_t gid,
            uint32_t remaining);
    void (*on_acquired)(void* context, int code);
    void (*on_authenticated)(void* context, uint32_t fid, uint32_t gid,
            const uint8_t* token, uint32_t size_token);
    void (*on_error)(void* context, int code);
    void (*on_removed)(void* context, uint32_t fid, uint32_t gid,
            uint32_t remaining);
    void (*on_enumerate)(void* context, uint32_t fid, uint32_t gid,
            uint32_t remaining);
    void (*on_touch_down)(void* context);
    void (*on_touch_up)(void* context);
    void (*on_imageinfo_acquired)(void* context, uint32_t type, uint32_t quality, uint32_t match_score);
    void (*on_sync_templates)(void* context, const uint32_t* fids, uint32_t size_fid, uint32_t gid);
    void (*on_engineeringinfo_updated)(void* context, engineering_info_t engineering);
    void (*on_monitor)(void* context, uint32_t type, double data);
    void (*on_dcsmsg)(fingerprint_auth_dcsmsg_t auth_context);
    void (*on_fingerprintcmd)(void* context, uint32_t cmdId, uint8_t* result, uint32_t resultLen);
    void (*hypnusSetAction)(void);
    void (*fpc_bind_bigcore_bytid)(uint32_t tid);
} fpc_hal_compat_callback_t;

#define IDENTIFY_QUALITY_LIMIT 30
#define IDENTIFY_COVERAGE_LIMIT 60

typedef enum {
    FPC_TASK_HAL = 0, /* Standard android tasks */
    FPC_TASK_HAL_EXT, /* FPC extension tasks */
} fpc_task_owner_t;

typedef struct fpc_hal_common {

    struct fpc_worker* worker;
    struct {
        void (*func)(void*);
        void* arg;
        fpc_task_owner_t owner;
    } current_task;

    pthread_mutex_t lock;

    struct fpc_hal_ext_sensortest* ext_sensortest;
    struct fpc_engineering* ext_engineering;
    struct fpc_authenticator* ext_authenticator;
    struct fpc_navigation* ext_navigation;
    struct fpc_calibration* ext_calibration;
    struct fpc_recalibration* ext_recalibration;
    struct fpc_sense_touch* ext_sensetouch;

    struct fpc_tee* tee_handle;
    struct fpc_tee_sensor* sensor;
    struct fpc_tee_bio* bio;
    uint32_t current_gid;
    uint64_t challenge;
    uint64_t user_id;
    uint64_t authenticator_id;
    uint32_t remove_fid;
    uint8_t hat[69];
    char current_db_file[PATH_MAX];
    const fpc_hal_compat_callback_t* callback;
    void* callback_context;
}fpc_hal_common_t;


void fingerprint_hal_resume(fpc_hal_common_t* dev);

void fingerprint_hal_do_async_work(fpc_hal_common_t* dev,
        void (*func)(void*), void* arg,
        fpc_task_owner_t owner);

void fingerprint_hal_goto_idle(fpc_hal_common_t* dev);
int fpc_hal_open(fpc_hal_common_t** device,
        const fpc_hal_compat_callback_t* callback, void* callback_context);
void fpc_hal_close(fpc_hal_common_t* device);
uint64_t fpc_pre_enroll(fpc_hal_common_t* device);
int fpc_post_enroll(fpc_hal_common_t* device);
uint64_t fpc_get_authenticator_id(fpc_hal_common_t* device);
int fpc_set_active_group(fpc_hal_common_t* device, uint32_t gid,
        const char* store_path, uint32_t* template_status);
int fpc_authenticate(fpc_hal_common_t* device,
        uint64_t operation_id, uint32_t gid);
int fpc_enroll(fpc_hal_common_t* device, const uint8_t* hat, uint32_t size_hat,
        uint32_t gid, uint32_t timeout_sec);
int fpc_cancel(fpc_hal_common_t* device);
int fpc_remove(fpc_hal_common_t* device, uint32_t gid, uint32_t fid);
int fpc_enumerate(fpc_hal_common_t* device);
int fpc_sensor_alive_check(fpc_hal_common_t *device);
int fpc_set_waiting_finger_alive_check(fpc_hal_common_t *device, int enable);
int fpc_get_enrollment_total_times(fpc_hal_common_t* dev);
int fpc_pause_enroll(fpc_hal_common_t* dev);
int fpc_continue_enroll(fpc_hal_common_t* dev);
int fpc_pause_identify(fpc_hal_common_t* dev);
int fpc_continue_identify(fpc_hal_common_t* dev);
int fpc_get_alikey_status(fpc_hal_common_t* dev);
int fpc_clean_up(fpc_hal_common_t* dev);
int fpc_wait_finger_down(fpc_hal_common_t* dev);
int fpc_set_screen_state(fpc_hal_common_t* dev, int screen_state);
int fpc_keymode_enable(fpc_hal_common_t* dev, int enable);
int fpc_open_log(fpc_hal_common_t* dev, int on);
int fpc_get_engineering_Info(fpc_hal_common_t* dev, uint32_t type);
int fpc_hal_notify_qrcode(fpc_hal_common_t* dev, int32_t cmdId);


#define p_fpc_hal_common(pc) container_of(pc, struct fpc_hal_common, sensor)


#ifdef __cplusplus
}
#endif

#endif // FPC_TEE_HAL_H
