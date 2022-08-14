/************************************************************************************
 ** File: - fpc\fpc_hal\tee_hal\fpc_tee_hal.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE hal for FPC (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Ziqing.guo   2017/10/13        create the file
 **  Ziqing.guo   2017/10/21        customization for homekey
 **  Ziqing.guo   2017/10/26        add the new apis
 **  Ziqing.guo   2017/10/27        customization for wait finger down and wait finger lost
 **  Ziqing.guo   2017/10/28        customization for reporting touch down/touch up
 **  Ziqing.guo   2017/10/29        add enroll lock
 **  Ziqing.guo   2017/10/31        remove the second onRemoved
 **  Ziqing.guo   2017/11/01        remove the cancel error message
 **  Ziqing.guo   2017/11/01        add for identify at enroll
 **  Ziqing.guo   2017/11/06        add for retry
 **  Ziqing.guo   2017/11/06        add template backup and restore
 **  Ziqing.guo   2017/11/07        add the api detect count, detect threshold
 **  Ziqing.guo   2017/11/09        add to report engineering mode
 **  Ziqing.guo   2017/11/30        add for 1270
 **  Ran.Chen     2018/01/08        add for fpc images_store
 **  Ran.Chen     2018/01/17        add fpc1023 threshold (32) and fingerlost msg
 **  Ran.Chen     2018/01/29        modify for fp_id, Code refactoring
 **  Ran.Chen     2018/03/15        add for SNR test
 **  Ran.Chen     2018/03/21        add for fpc_monitor
 **  Ran.Chen     2018/03/26        add touch_up msg after pause_enroll
 **  Yang.Tan     2018/12/11        add fingerprint image quality pass  in engineeringinfo
 **  Long.Liu     2018/12/19         modify for P80 18151 donot create input device
 **  Long.Liu     2018/02/14        add for template_status
 ************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <fcntl.h>
#include "container_of.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>

#include "fpc_tee.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_hw_auth.h"
#include "fpc_log.h"
#include "fpc_tee_hal.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_tee_engineering.h"
#include "fpc_tee_sensortest.h"
#include "fpc_tee_kpi.h"
#include "fpc_worker.h"
#include "fpc_hal_input_device.h"
#include "fingerprint_type.h"
#include "fpc_monitor.h"
#include "fingerprint.h"

#ifdef FPC_CONFIG_QC_AUTH
#include "fpc_hal_ext_authenticator_service.h"
#endif
#ifdef FPC_CONFIG_ENGINEERING
#include "fpc_hal_ext_engineering_service.h"
#include "fpc_ta_bio_interface.h"
#endif
#ifdef FPC_CONFIG_NAVIGATION
#include "fpc_hal_ext_navigation_service.h"
#include "fpc_hal_navigation.h"
#endif
#ifdef FPC_CONFIG_SENSORTEST
#include "fpc_hal_ext_sensortest_service.h"
#endif
#if (FPC_CONFIG_FORCE_SENSOR == 1)
#include "fpc_hal_ext_sense_touch_service.h"
#include "fpc_hal_sense_touch.h"
#endif

#define TEMPLATE_POSTFIX                "/user.db"

#define PROPERTY_FPC_SW_VERSION "oplus.fpc.sw.version"

#define ALGO_VERSION    "v30.0.0.11"

static char g_lib_version[128] = {'\0'};
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"

//ziqing add the variables for "pause_enroll" "pause_identify"
static bool stop_enroll = false;
static pthread_mutex_t enroll_lock;
static pthread_cond_t enroll_cond;
static bool flag_wait_touch_down = false;
identify_lock_t identify_locker = {
        .stop_identify = false,
};

#define FPC_DETECTOR_COUNT_GENERAL 1
#define FPC_DETECTOR_COUNT_LOCK_MODE 100
#define FPC_THRESHOLD_GENERAL 100
#define FPC_THRESHOLD_LOCK_MODE 200
#define FPC_1022_THRESHOLD_LOCK_MODE 60
#define FPC_1023_THRESHOLD_LOCK_MODE 32

//for identify_retry
static uint32_t identify_failed_counter = 0;
static uint32_t g_enroll_finger_counts = 0;
uint32_t waiting_finger_state = 0;
uint32_t alive_test_retry_count = 0;

static void do_sensor_alive_check(void *data);

#ifdef FPC_TEE_STORE_IMAGE
// The size of images
static char fp_manu[FP_ID_MAX_LENGTH] ;

#define CAPTURE_FILE_RAW "/data/images/fpc_capture_%04d%02d%02d_%02d%02d%02d_%.5d_%s.fmi"
#define IDENTIFY_SUCCESS_FILE_RAW "/data/images/fpc_identify_%04d%02d%02d_%02d%02d%02d_%.5d_success_%s.fmi"
#define IDENTIFY_FAIL_FILE_RAW "/data/images/fpc_identify_%04d%02d%02d_%02d%02d%02d_%.5d_fail_%s.fmi"
#define RUBBER_SUCCESS_FILE_RAW "/data/images/fpc_rubber_%04d%02d%02d_%02d%02d%02d_%.5d_success_%s.fmi"
#define RUBBER_FAIL_FILE_RAW "/data/images/fpc_rubber_%04d%02d%02d_%02d%02d%02d_%.5d_fail_%s.fmi"
#define ENROLL_FILE_RAW "/data/images/fpc_enroll_%04d%02d%02d_%02d%02d%02d_%.5d_%s.fmi"
#define IDENTIFY_SUCCESS_FILE_UPDATE_RAW "/data/images/fpc_identify_%04d%02d%02d_%02d%02d%02d_%.5d_success_update_%s.fmi"
#define CAPTURE_FILE_PP "/data/images/fpc_capture_%04d%02d%02d_%02d%02d%02d_%.5d_pp_%s.raw"
#define IDENTIFY_SUCCESS_FILE_PP "/data/images/fpc_identify_%04d%02d%02d_%02d%02d%02d_%.5d_success_pp_%s.raw"
#define IDENTIFY_FAIL_FILE_PP "/data/images/fpc_identify_%04d%02d%02d_%02d%02d%02d_%.5d_fail_pp_%s.raw"
#define RUBBER_SUCCESS_FILE_PP "/data/images/fpc_rubbe_%04d%02d%02d_%02d%02d%02d_%.5dr_success_pp_%s.raw"
#define RUBBER_FAIL_FILE_PP "/data/images/fpc_rubber_%04d%02d%02d_%02d%02d%02d_%.5d_fail_pp_%s.raw"
#define ENROLL_FILE_PP "/data/images/fpc_enroll_%04d%02d%02d_%02d%02d%02d_%.5d_pp_%s.raw"
#define IDENTIFY_SUCCESS_UPDATE_PP "/data/images/fpc_identify_%04d%02d%02d_%02d%02d%02d_%.5d_success_update_pp_%s.raw"
#define TYPE_CAPTURE 0
#define TYPE_IDENTIFY_SUCCESS 1
#define TYPE_IDENTIFY_FAIL 2
#define TYPE_RUBBER_SUCCESS 3
#define TYPE_RUBBER_FAIL 4
#define TYPE_ENROLL 5
#define TYPE_IDENTIFY_SUCCESS_UPDATE 6
static int image_count=0;
static int do_enumerate(fpc_hal_common_t* dev);



int store_capture_image(fpc_hal_common_t* dev, int type, int images) {
    int ret = 0;
    uint32_t write_result;
    FILE* file_raw = NULL;
    FILE* file_pp = NULL;
    DIR *dirptr = NULL;
    uint8_t* image;
    uint32_t image_size = fp_config_info_init.fp_image_size;
    char filename_raw[100];
    char filename_pp[100];
    time_t nowtime = time(NULL);
    struct tm *now = localtime(&nowtime);
    uint32_t image_data_size = 0;
    uint8_t *image_data_buffer = NULL;
    image = (uint8_t*) malloc(image_size);
    if (NULL == image) {
        ret = FPC_ERROR_MEMORY;
        goto error;
    }

    switch(type)
    {
        case TYPE_ENROLL:
            sprintf(filename_raw, ENROLL_FILE_RAW,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            sprintf(filename_pp, ENROLL_FILE_PP,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            LOGE("Enroll image file used %s, %s", filename_raw, filename_pp);
            break;
        case TYPE_CAPTURE:
            sprintf(filename_raw, CAPTURE_FILE_RAW,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            sprintf(filename_pp, CAPTURE_FILE_PP,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            LOGE("Capture image file used %s, %s", filename_raw, filename_pp);
            break;
        case TYPE_IDENTIFY_SUCCESS:
            sprintf(filename_raw, IDENTIFY_SUCCESS_FILE_RAW,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            sprintf(filename_pp, IDENTIFY_SUCCESS_FILE_PP,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            LOGE("Identify image file used %s, %s", filename_raw, filename_pp);
            break;
        case TYPE_IDENTIFY_FAIL:
            sprintf(filename_raw, IDENTIFY_FAIL_FILE_RAW,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            sprintf(filename_pp, IDENTIFY_FAIL_FILE_PP,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            LOGE("Identify image file used %s, %s", filename_raw, filename_pp);
            break;
        case TYPE_RUBBER_SUCCESS:
            sprintf(filename_raw, RUBBER_SUCCESS_FILE_RAW,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            sprintf(filename_pp, RUBBER_SUCCESS_FILE_PP,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            LOGE("Rubber test success image file used %s, %s", filename_raw, filename_pp);
            break;
        case TYPE_RUBBER_FAIL:
            sprintf(filename_raw, RUBBER_FAIL_FILE_RAW,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            sprintf(filename_pp, RUBBER_FAIL_FILE_PP,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            LOGE("Rubber test fail image file used %s, %s", filename_raw, filename_pp);
            break;
        case TYPE_IDENTIFY_SUCCESS_UPDATE:
            sprintf(filename_raw, IDENTIFY_SUCCESS_FILE_UPDATE_RAW,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            sprintf(filename_pp, IDENTIFY_SUCCESS_UPDATE_PP,
                    now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, images, fp_manu);
            LOGE("Identify success update image file used %s, %s", filename_raw, filename_pp);
            break;
        default:
            break;
    }
    file_raw = fopen(filename_raw, "w");
    file_pp = fopen(filename_pp, "w");
    if ((NULL == file_raw) || (NULL == file_pp)) {
        LOGE("Error: Cannot open file.\n");
        ret = FPC_ERROR_MEMORY;
        goto error;
    }
    fpc_tee_debug_get_raw_size(dev->tee_handle, &image_data_size);
    if (image_data_size == 0) {
        LOGE("Error: Cannot get imagedata size.\n");
        ret = FPC_ERROR_MEMORY;
        goto error;
    }
    image_data_buffer = (uint8_t*) malloc(image_data_size);
    if (NULL == image_data_buffer) {
        ret = FPC_ERROR_MEMORY;
        goto error;
    }
    fpc_tee_debug_retrieve(dev->tee_handle,
            FPC_TEE_ENGINEERING_TYPE_RAW,
            (void *)image_data_buffer,
            image_data_size);
    { //FMI header
        uint32_t fmi_header[5];
        fmi_header[0] = 0x494D462E;
        fmi_header[1] = 0x4;
        fmi_header[2] = 0x00010000;
        fmi_header[3] = 0x00000001;
        fmi_header[4] = image_data_size;
        write_result = fwrite(fmi_header, 1, 20, file_raw);
        if (write_result != 20) {
            LOGE("fmi file header not written %d bytes instead of %d "
                    "bytes err: %d", write_result, 20, ferror(file_raw));
            ret = FPC_ERROR_IO;
            goto error;
        }
    }
    write_result = fwrite(image_data_buffer, 1, image_data_size, file_raw);
    if (write_result != image_data_size) {
        LOGE("Entire file not written %d bytes instead of %d "
                "bytes err: %d", write_result, image_data_size, ferror(file_raw));
        ret = FPC_ERROR_IO;
        goto error;
    }
    fpc_tee_debug_retrieve(dev->tee_handle,
            FPC_TEE_ENGINEERING_TYPE_ENHANCED_IMAGE,
            (void *)image,
            image_size);
    write_result = fwrite(image, 1, image_size, file_pp);
    if (write_result != image_size) {
        LOGE("Entire file not written %d bytes instead of %d "
                "bytes err: %d", write_result, image_size, ferror(file_pp));
        ret = FPC_ERROR_IO;
        goto error;
    }
error:
    if (file_raw != NULL) {
        fclose(file_raw);
        file_raw = NULL;
    }
    if (file_pp != NULL) {
        fclose(file_pp);
        file_pp = NULL;
    }
    closedir(dirptr);
    LOGD("Filesize %d", image_size);
    if (image != NULL) {
        free(image);
    }
    if (image_data_buffer != NULL) {
        free(image_data_buffer);
    }
    return ret;
}

#endif
#ifdef FPC_TA_HEAP_DEBUG
//Ziqing.GUO Add for heap debug
#define HEAP_USAGE_TABLE "/data/heap_usage.csv"

enum  CsvWriteMode {
        CSV_ONELINE = 1,
        CSV_OTHERLINE,
};

int putString2Csv(char str[], char filename[], int mode) {
        FILE *fp;
        //try to open file
        if ((fp = fopen(filename, "a")) == NULL) {
                LOGE("can't open file %s !!!", filename);
                return -1;
        }
        switch (mode) {
                case CSV_ONELINE:
                        {
                                fputs(str, fp);
                        }
                        break;
                case CSV_OTHERLINE:
                        {
                                fputs("\n", fp);
                        }
                        break;
                default:
                        break;
        }
        if (fclose(fp) != 0) {
                LOGE("can't close file %s !!!", filename);
                return -1;
        }
        return 0;
}
#endif

static void hal_work_func(void* arg) {
        fpc_hal_common_t* dev = (fpc_hal_common_t*) arg;
        dev->current_task.func(dev->current_task.arg);
#ifdef FPC_CONFIG_NAVIGATION
        fpc_navigation_resume(dev->ext_navigation);
#endif
}

void fingerprint_hal_do_async_work(fpc_hal_common_t* dev,
                void (*func)(void*), void* arg,
                fpc_task_owner_t owner) {
        fpc_worker_join_task(dev->worker);
#ifdef FPC_CONFIG_NAVIGATION
        fpc_navigation_pause(dev->ext_navigation);
#endif
        dev->current_task.func = func;
        dev->current_task.arg = arg;
        dev->current_task.owner = owner;
        fpc_worker_run_task(dev->worker, hal_work_func, dev);
}

void oplus_fingerprint_set_property() {
        char val[PROPERTY_VALUE_MAX];
        property_set(PROPERTY_FPC_SW_VERSION, "23");
        property_get(PROPERTY_FPC_SW_VERSION, val, "0");
        int flag = atoi(val);
        LOGD("%s flag:%d", __func__, flag);
}

void oplus_fingerprint_clear_property() {
        char val[PROPERTY_VALUE_MAX];
        property_set(PROPERTY_FPC_SW_VERSION, "0");
        property_get(PROPERTY_FPC_SW_VERSION, val, "0");
        int flag = atoi(val);
        LOGD("%s flag:%d", __func__, flag);
}

//ziqing.guo add the threads created by oplus
static void oplus_worker_init(void) {
        pthread_cond_init(&enroll_cond, NULL);
        pthread_mutex_init(&enroll_lock, NULL);

        //for pause identify
        pthread_cond_init(&identify_locker.identify_cond, NULL);
        pthread_mutex_init(&identify_locker.identify_lock, NULL);

        oplus_fingerprint_set_property();

        return;
}

static void oplus_worker_destroy(void) {
        //for pause enroll
        pthread_cond_destroy(&enroll_cond);
        pthread_mutex_destroy(&enroll_lock);

        //for pause identify
        pthread_cond_destroy(&identify_locker.identify_cond);
        pthread_mutex_destroy(&identify_locker.identify_lock);

        oplus_fingerprint_clear_property();

        return;
}


void fingerprint_hal_goto_idle(fpc_hal_common_t* dev) {
        if (dev->sensor) {
                fpc_tee_set_cancel(dev->sensor);
        }

        //ziqing.guo add for clear the flags while cancel
        if (flag_wait_touch_down) {
                flag_wait_touch_down = false;
        }

        pthread_mutex_lock(&enroll_lock);
        if (stop_enroll) {
                LOGD("%s get_enroll_lock", __func__);
                pthread_cond_signal(&enroll_cond);
                stop_enroll = false;
        }
        pthread_mutex_unlock(&enroll_lock);

        pthread_mutex_lock(&identify_locker.identify_lock);
        if (identify_locker.stop_identify) {
                LOGD("%s get_identify_lock", __func__);
                pthread_cond_signal(&identify_locker.identify_cond);
                identify_locker.stop_identify = false;
        }
        pthread_mutex_unlock(&identify_locker.identify_lock);

        monitor_power_state_clear(&fp_monitor.power);
#ifdef FPC_CONFIG_ENGINEERING
        if (dev->ext_engineering) {
                dev->ext_engineering->cancel_image_injection(dev->ext_engineering);
        }
#endif

        if (dev->worker) {
                fpc_worker_join_task(dev->worker);
        }
#ifdef FPC_CONFIG_NAVIGATION
        if (dev->ext_navigation) {
            fpc_navigation_pause(dev->ext_navigation);
        }
#endif

        if (dev->sensor) {
                fpc_tee_clear_cancel(dev->sensor);
        }
}

void fingerprint_hal_resume(fpc_hal_common_t* dev) {
        fpc_worker_join_task(dev->worker);
#ifdef FPC_CONFIG_NAVIGATION
        fpc_navigation_resume(dev->ext_navigation);
#endif
}

static int32_t capture_image(fpc_hal_common_t* dev,
                bool need_wait_finger_up,
                bool need_wait_finger_down,
                fingerprint_mode_t mode) {
        int32_t status = 0;

#ifdef FPC_CONFIG_ENGINEERING
        if (dev->ext_engineering->is_img_inj_enabled(dev->ext_engineering)) {
                status = dev->ext_engineering->handle_image_injection(dev->ext_engineering);
        } else {
#endif
                for (;;) {
                        status = fpc_tee_capture_image(&(dev->sensor), need_wait_finger_up, need_wait_finger_down, mode, &identify_locker);

                        switch (status) {
                                case FPC_ERROR_NONE: //FPC_CAPTURE_OK:
                                        goto out;
                                case FPC_STATUS_FINGER_LOST://FPC_CAPTURE_FINGER_LOST:
                                        if (identify_failed_counter == 0) {
                                            LOGE("%s FPC_CAPTURE_FINGER_LOST ", __func__);
                                            dev->callback->on_acquired(dev->callback_context, HAL_COMPAT_ACQUIRED_TOO_FAST);
                                                if (mode == IDENTIFY_MODE) {
                                                        if (0 == fpc_tee_wait_finger_lost(&dev->sensor)) {
                                                                LOGD("%s send touch_up", __func__);
                                                                dev->callback->on_touch_up(dev->callback_context);
                                                        }
                                                }
                                                break;
                                        }
                                        goto out;
                                case -FPC_ERROR_CANCELLED:
                                        goto out;
                                case -FPC_ERROR_IO:
                                        goto out;
                                default:
                                        goto out;
                        }
                }
#ifdef FPC_CONFIG_ENGINEERING
        }
#endif
out:
        return status;
}

#if FPC_CONFIG_FORCE_SENSOR == 1
static int sense_touch_capture_image(fpc_hal_common_t* dev) {
        const fpc_sense_touch_config_t* st_config = fpc_sense_touch_get_config();
        int status = capture_image(dev);

        if (status == FPC_CAPTURE_OK && st_config != NULL && st_config->auth_enable_down_force) {
                int result = fpc_tee_wait_for_button_down_force(dev->sensor,
                                st_config->auth_button_timeout_ms,
                                st_config->trigger_threshold);
                if (result == FPC_CAPTURE_OK) {
                        report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS, FPC_HAL_INPUT_KEY_DOWN);
                } else if (result == -FPC_ERROR_TIMEDOUT || result == FPC_CAPTURE_FINGER_LOST) {
                        status = FPC_CAPTURE_RETRY;
                } else {
                        status = result;
                }
        }
        if (status == FPC_CAPTURE_OK && st_config != NULL && st_config->auth_enable_up_force) {
                int result = fpc_tee_wait_for_button_up_force(dev->sensor,
                                st_config->auth_button_timeout_ms,
                                st_config->untrigger_threshold);
                if (result == FPC_CAPTURE_OK) {
                        report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS, FPC_HAL_INPUT_KEY_UP);
                } else if (result == -FPC_ERROR_TIMEDOUT) {
                        report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS, FPC_HAL_INPUT_KEY_UP);
                        status = FPC_CAPTURE_RETRY;
                } else {
                        status = result;
                }
        }
        return status;
}
#endif

#if 0
static int do_enumerate(fpc_hal_common_t* dev) {
        LOGD("%s", __func__);
        int status = 0;
        const int max_templates = 5;
        uint32_t indices_count = max_templates;
        uint32_t* indices = (uint32_t*)malloc(max_templates * sizeof(uint32_t));
        indices_count = max_templates;
        memset(indices, 0, max_templates * sizeof(uint32_t));
        status = fpc_tee_get_finger_ids(dev->bio, &indices_count, indices);
        if (status) {
                dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
                LOGE("%s failed %s", __func__, fpc_error_str(status));
                status = -EIO;
                goto out;
        }
        LOGD("%s ids %d", __func__, indices_count);

        dev->callback->on_sync_templates(dev->callback_context,
                        indices, indices_count, dev->current_gid);
        g_enroll_finger_counts = indices_count;
out:
        if (indices) {
                free(indices);
        }
        if (status != 0) {
                return -1;
        }
        return 0;
}
#else
static int do_enumerate(fpc_hal_common_t* dev) {
        const int max_templates = 5;
        uint32_t ids[max_templates];
        uint32_t size = max_templates;

        int status = fpc_tee_get_finger_ids(dev->bio, &size, ids);
        if (status) {
                dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
                LOGE("%s failed %s", __func__, fpc_error_str(status));
                return status;
        }

        if (size == 0) {
                dev->callback->on_enumerate(dev->callback_context, 0,
                                dev->current_gid, 0);
                return 0;
        }

        for (unsigned i = 0; i < size; ++i) {
                dev->callback->on_enumerate(dev->callback_context, ids[i],
                                dev->current_gid, (size -1) - i);
        }
        g_enroll_finger_counts = size;
        return 0;
}
#endif


static void do_authenticate(void* data) {
        LOGD("%s", __func__);
        int status;
        /* Image qualifier check is enabled during authentication so we can make an early
           exit and disregard the image if sensor gets activated while in pocket. */
        uint32_t id = 0;
        uint32_t update = 0;
#ifdef FPC_CHECK_BROKEN
        uint32_t result = 0;
#endif
        fpc_hal_common_t* dev = (fpc_hal_common_t*) data;

#ifdef FPC_TA_HEAP_DEBUG
        unsigned int heap_usage;
        char strtemp[256];
#endif /* FPC_TA_HEAP_DEBUG */

        //ziqing.guo add the variables for retry
        bool identify_need_wait_finger_up = true;
        bool identify_need_wait_finger_down = true;
        fpc_hal_compat_acquired_t identify_acquired_status = HAL_COMPAT_ACQUIRED_GOOD;

        fpc_ta_bio_identify_statistics_t identify_data;
        fpc_ta_bio_identify_statistics_t identify_data_tmp; //for send image info for monitor

        fingerprint_auth_dcsmsg_t auth_context; //for send dcsmsg
        memset(&auth_context, 0, sizeof(auth_context));

#ifdef FPC_CONFIG_HW_AUTH
        status = fpc_tee_set_auth_challenge(dev->tee_handle, dev->challenge);
        if (status) {
                goto out;
        }
#endif

        for (;;) {
                fpc_tee_kpi_start(dev->tee_handle);

#if FPC_CONFIG_FORCE_SENSOR == 1
                status = sense_touch_capture_image(dev);
                if (status == FPC_CAPTURE_RETRY) {
                        continue;
                }
#else
                status = capture_image(dev,
                                identify_need_wait_finger_up,
                                identify_need_wait_finger_down,
                                IDENTIFY_MODE);
#endif
                if (status != FPC_ERROR_NONE && status != FPC_STATUS_FINGER_LOST) {//(status != FPC_CAPTURE_OK && status != FPC_CAPTURE_FINGER_LOST) {
#ifdef FPC_CONFIG_ENGINEERING
                        if (status == FPC_STATUS_BAD_QUALITY) { //FPC_CAPTURE_BAD_QUALITY
                                dev->ext_engineering->handle_image_subscription_auth(dev->ext_engineering,
                                                status, 0, COVERAGE_UNKNOWN, QUALITY_UNKNOWN, id);
                        }
#endif
                        LOGE("%s capture_image failed with status = %d", __func__, status);
                        goto out;
                }

                dev->callback->on_acquired(dev->callback_context,
                                HAL_COMPAT_ACQUIRED_GOOD);

                if (status == FPC_STATUS_FINGER_LOST) { //FPC_CAPTURE_FINGER_LOST
                        /*while the function capture_image returns finger lost,  clear identify data and do not identify
                          capture returns finger lost frequently in identify retry mode, so we need go out to send the result*/
                        memset(&identify_data, 0 , sizeof(identify_data));
                        LOGD("%s clear identify data\n", __func__);
                } else {
                        status = fpc_tee_identify_and_update(dev->bio, &id, &identify_data, &update);
                        LOGE("%s identify_fpc status = %d", __func__, status);
                        if (status == FPC_STATUS_NOT_FINGER) {
                        LOGE("%s identify found not finger, status = %d", __func__, status);
                        continue;
                        }
                        if (status) {
                                /* A real error */
                                LOGE("%s fpc_tee_identify failed with status = %d", __func__, status);
                                goto out;
                        }
                }

                LOGD("%s fpc_tac_identify result %u %u %u %d %d %u", __func__,
                                identify_data.result,
                                identify_data.score,
                                identify_data.index,
                                identify_data.quality,
                                identify_data.coverage,
                                update);

#ifdef FPC_CONFIG_ENGINEERING
                fpc_ta_bio_identify_statistics_t stat;
                memset(&stat, 0, sizeof(fpc_ta_bio_identify_statistics_t));
                int status_get_statistics = fpc_tee_get_identify_statistics(dev->bio, &stat);
                if (status_get_statistics) {
                        LOGD("%s failed to get identify statistics\n", __func__);
                }
                dev->ext_engineering->handle_image_subscription_auth(dev->ext_engineering, 0, status,
                                stat.coverage, stat.quality, id);
#endif

#ifdef FPC_TEE_STORE_IMAGE
                if(id == 0) {
                        store_capture_image(dev, TYPE_IDENTIFY_FAIL, image_count++);
                } else {
                        store_capture_image(dev, TYPE_IDENTIFY_SUCCESS, image_count++);
                }
#endif
                if (id != 0) {
                        /*send dcsmsg*/
                        auth_context.auth_result = identify_data.result; //1:pass, 0:fail
                        auth_context.fail_reason = identify_acquired_status;
                        auth_context.quality_score = identify_data.quality;
                        auth_context.match_score = identify_data.score;
                        auth_context.signal_value = 0; //UNUSED
                        auth_context.img_area = identify_data.coverage;
                        auth_context.retry_times = identify_failed_counter; //identify_failed_counter
                        memcpy(auth_context.algo_version, g_lib_version, strlen(g_lib_version));
                        auth_context.chip_ic = fp_config_info_init.fp_ic_type; //UNUSED
                        auth_context.factory_id = 0; //UNUSED
                        auth_context.module_type = 0; //UNUSED
                        auth_context.lense_type = 0; //UNUSED
                        auth_context.dsp_availalbe = 0; //UNUSED
                        dev->callback->on_dcsmsg(auth_context);

                        uint8_t hat[69];
#ifdef FPC_CONFIG_HW_AUTH
                        status = fpc_tee_get_auth_result(dev->tee_handle, hat, sizeof(hat));

                        if (status) {
                                goto out;
                        }
#endif
                        dev->callback->on_authenticated(dev->callback_context,
                                        id, dev->current_gid,
                                        hat, sizeof(hat));

                        if (update != 0) {
                                //ziqing modify for do not backup template in case of template update
                                finger_store_template_config_t template_config = { dev->current_db_file , UPDATE_TEMPLATE_MODE};
                                fpc_tee_store_template_db(dev->bio, &template_config);
                        }

#ifdef FPC_TA_HEAP_DEBUG
                        status = fpc_tee_get_ta_heap_usage(dev->sensor, &heap_usage);
                        if (status) {
                                LOGE("%s error with status = %u", __func__, status);
                        }
                        LOGD("%s heap_usage after updating template = %u bytes", __func__, heap_usage);
                        sprintf(strtemp, "%u", heap_usage);
                        putString2Csv(strtemp, HEAP_USAGE_TABLE, CSV_ONELINE);
                        putString2Csv("", HEAP_USAGE_TABLE, CSV_OTHERLINE);
#endif /* FPC_TA_HEAP_DEBUG */

                        break;
                } else {
                        //Haitao.Zhou add for recording identify info when first dismatch
                        if (identify_failed_counter == 0) {
                                memset(&identify_data_tmp, 0 , sizeof(identify_data_tmp));
                                memcpy(&identify_data_tmp, &identify_data, sizeof(identify_data_tmp));

                                if (identify_data.quality < IDENTIFY_QUALITY_LIMIT) {
                                        identify_acquired_status = HAL_COMPAT_ACQUIRED_IMAGER_DIRTY;
                                }
                                else if (identify_data.coverage < IDENTIFY_COVERAGE_LIMIT) {
                                        identify_acquired_status = HAL_COMPAT_ACQUIRED_PARTIAL;
                                }
                        }

                        //ziqing.guo modify becasue we will report "touch up" later, so we don't need wait finger lost again in the function "capture_image"
                        identify_need_wait_finger_up = false;

                        //ziqing.guo add the variables for identify fail retry
                        if (++identify_failed_counter < 2) {
                                identify_need_wait_finger_down = false;
                                LOGD("%s identify failed!! try again", __func__);

                                continue;
                        } else {
                                identify_failed_counter = 0;
                                identify_need_wait_finger_down = true;
                        }

                        //Haitao.Zhou add for checking TP touch and acquire info when identify failed
                        if ((TP_TOUCH_DOWN == fpc_tee_get_tp_protect_result()) ||
                                        (FINGERPRINT_SCREEN_OFF == fpc_tee_get_screen_state() &&
                                        identify_acquired_status == HAL_COMPAT_ACQUIRED_IMAGER_DIRTY)) {
                                if (TP_TOUCH_DOWN == fpc_tee_get_tp_protect_result()) {
                                        monitor_tp_protect_set_mode(&fp_monitor.tp_protect, IDENTIFY_MODE);
                                        monitor_trigger(&dev->sensor, &fp_monitor, TP_PROTECT_MONITOR);
                                }
                                if (identify_acquired_status == HAL_COMPAT_ACQUIRED_IMAGER_DIRTY) {
                                        LOGD("%s protected identify_acquired_status %d", __func__, identify_acquired_status);
                                        monitor_power_set_trigger_flag(&fp_monitor.power);
                                }

                                identify_acquired_status = HAL_COMPAT_ACQUIRED_GOOD;

                                if (0 == fpc_tee_wait_finger_lost(&dev->sensor)) {
                                        LOGD("%s send touch_up", __func__);
                                        dev->callback->on_touch_up(dev->callback_context);
                                }
                                continue;
                        }

                        switch (identify_acquired_status) {
                                case HAL_COMPAT_ACQUIRED_GOOD:
                                        identify_acquired_status = HAL_COMPAT_ACQUIRED_GOOD;
                                        break;
                                case HAL_COMPAT_ACQUIRED_IMAGER_DIRTY:
                                        identify_acquired_status = HAL_COMPAT_ACQUIRED_IMAGER_DIRTY;
                                        break;
                                case HAL_COMPAT_ACQUIRED_PARTIAL:
                                        identify_acquired_status = HAL_COMPAT_ACQUIRED_PARTIAL;
                                        break;
                                default:
                                        identify_acquired_status = HAL_COMPAT_ACQUIRED_GOOD;
                                        break;
                        }

                        /*send dcsmsg*/
                        auth_context.auth_result = identify_data.result; //1:pass, 0:fail
                        auth_context.fail_reason = identify_acquired_status;
                        auth_context.quality_score = identify_data.quality;
                        auth_context.match_score = identify_data.score;
                        auth_context.signal_value = 0; //UNUSED
                        auth_context.img_area = identify_data.coverage;
                        auth_context.retry_times = identify_failed_counter; //identify_failed_counter
                        memcpy(auth_context.algo_version, g_lib_version, strlen(g_lib_version));
                        auth_context.chip_ic = fp_config_info_init.fp_ic_type;
                        auth_context.factory_id = 0; //UNUSED
                        auth_context.module_type = 0; //UNUSED
                        auth_context.lense_type = 0; //UNUSED
                        auth_context.dsp_availalbe = 0; //UNUSED
                        dev->callback->on_dcsmsg(auth_context);

                        LOGD("%s send to monitor %u %u", __func__, identify_data_tmp.score, identify_data_tmp.quality);

                        identify_acquired_status = HAL_COMPAT_ACQUIRED_GOOD;


                        dev->callback->on_authenticated(dev->callback_context,
                                        0, dev->current_gid, NULL, 0);

                        if (update != 0) {
                                //ziqing modify for do not backup template in case of template update
                                finger_store_template_config_t template_config = { dev->current_db_file , UPDATE_TEMPLATE_MODE};
                                fpc_tee_store_template_db(dev->bio, &template_config);
                        }

#ifdef FPC_TA_HEAP_DEBUG
                        status = fpc_tee_get_ta_heap_usage(dev->sensor, &heap_usage);
                        if (status) {
                                LOGE("%s error with status = %u", __func__, status);
                        }
                        LOGD("%s heap_usage after identify fail= %u bytes", __func__, heap_usage);
                        sprintf(strtemp, "%u", heap_usage);
                        putString2Csv(strtemp, HEAP_USAGE_TABLE, CSV_ONELINE);
                        putString2Csv("", HEAP_USAGE_TABLE, CSV_OTHERLINE);
#endif /* FPC_TA_HEAP_DEBUG */

                        if (0 == fpc_tee_wait_finger_lost(&dev->sensor)) {
                                LOGD("%s send touch_up", __func__);
                                dev->callback->on_touch_up(dev->callback_context);
                        }
                }

                /* Recycle KPI to flush the kpi buffers */
                fpc_tee_kpi_stop(dev->tee_handle);
        }


out:
        /* Will not harm to stop again even if it's already stopped */
        fpc_tee_kpi_stop(dev->tee_handle);

        if (status && status !=FPC_STATUS_NOT_FINGER) {
                LOGE("%s failed %s\n", __func__, fpc_error_str(status));
                switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
                        case -FPC_ERROR_CANCELLED:
                                break;
                        default:
#ifndef FPC_CHECK_BROKEN
                                dev->callback->on_error(dev->callback_context,
                                                HAL_COMPAT_ERROR_HW_UNAVAILABLE);
#else
                                result = alive_check(dev->sensor);
                                if (result) {
                                    LOGE("%s failed on alive_check", __func__);
                                    dev->callback->on_error(dev->callback_context,
                                                    HAL_COMPAT_ERROR_ALIVE_CHECK + result);
                                    auth_context.fail_reason = result; //when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
                                } else {
                                    dev->callback->on_error(dev->callback_context,
                                                HAL_COMPAT_ERROR_HW_UNAVAILABLE);
                                    auth_context.fail_reason = 0; //when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
                                }
                                /*send dcsmsg*/
                                auth_context.auth_result = 999; //1:pass, 0:fail, 999: HW_ERRO
                                //auth_context.fail_reason = 0;//when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
                                auth_context.quality_score = 0;
                                auth_context.match_score = 0;
                                auth_context.signal_value = 0; //UNUSED
                                auth_context.img_area = 0;
                                auth_context.retry_times = alive_test_retry_count; //identify_failed_counter
                                memcpy(auth_context.algo_version, g_lib_version, strlen(g_lib_version));
                                auth_context.chip_ic = fp_config_info_init.fp_ic_type; //UNUSED
                                auth_context.factory_id = 0; //UNUSED
                                auth_context.module_type = 0; //UNUSED
                                auth_context.lense_type = 0; //UNUSED
                                auth_context.dsp_availalbe = 0; //UNUSED
                                dev->callback->on_dcsmsg(auth_context);
#endif
                                break;
                }
        }

        //ziqing.guo add for clear these flags after identify sucessed
        identify_need_wait_finger_up = true;
        identify_need_wait_finger_down = true;
        identify_failed_counter = 0;

        if (0 == fpc_tee_wait_finger_lost(&dev->sensor)) {
                LOGD("%s send touch_up", __func__);
                dev->callback->on_touch_up(dev->callback_context);
        }

        //add for fingerprint sensor goto deep sleep
        fpc_tee_deep_sleep(dev->sensor);
}

static void do_enroll(void* data) {
        int status = 0;
        fpc_hal_common_t* dev = (fpc_hal_common_t*) data;
#ifdef FPC_CHECK_BROKEN
        uint32_t result = 0;
#endif
        uint32_t remaining_samples = 0;
        fpc_algo_enroll_statistics_t enrol_data;

        fingerprint_auth_dcsmsg_t auth_context; //for send dcsmsg
        memset(&auth_context, 0, sizeof(auth_context));

        do_sensor_alive_check(data);
#ifdef FPC_CONFIG_HW_AUTH
        status = fpc_tee_authorize_enrol(dev->tee_handle,
                        dev->hat, sizeof(dev->hat));
        if (status) {
                goto out;
        }
#endif

        fpc_tee_kpi_start(dev->tee_handle);

        status = fpc_tee_begin_enrol(dev->bio);

        if (status) {
                goto out;
        }

        for (;;) {
                //ziqing.guo, modify for pause enroll
                pthread_mutex_lock(&enroll_lock);
                if (stop_enroll) {
                        LOGD("%s enroll stop in", __func__);
                        pthread_cond_wait(&enroll_cond, &enroll_lock);
                        LOGD("%s enroll stop out", __func__);
                        pthread_mutex_unlock(&enroll_lock);
                        continue;
                }
                pthread_mutex_unlock(&enroll_lock);

                status = capture_image(dev, false, true, ENROLL_MODE);
                if (status) {
#ifdef FPC_CONFIG_ENGINEERING
                        if (status == FPC_STATUS_BAD_QUALITY) {//FPC_CAPTURE_BAD_QUALITY
                                dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering,
                                                status, 0, REMAINING_UNKNOWN, 0);
                        }
#endif
                        goto out;
                }

                status = fpc_tee_enrol(dev->bio, &remaining_samples, &enrol_data);
                if (status < 0) {
                        LOGE("%s fpc_tee_enrol failed status = %d", __func__, status);
                        goto out;
                }

                LOGD("%s status %d, Enroll data %d %d %d %d", __func__,
                                status,
                                enrol_data.progress,
                                enrol_data.quality,
                                enrol_data.enrolled_template_size,
                                enrol_data.coverage);


#ifdef FPC_CONFIG_ENGINEERING
                if (status != FPC_ERROR_NONE) {//FPC_ENROL_COMPLETED
                        dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering, 0,
                                        status, remaining_samples, 0);
                }
#endif

#ifdef FPC_TEE_STORE_IMAGE
                store_capture_image(dev, TYPE_ENROLL, image_count++);
#endif
                uint32_t id = 0;
                switch (status) {
                        case FPC_ERROR_NONE://FPC_ENROL_COMPLETED:
                                status  = fpc_tee_end_enrol(dev->bio, &id);
                                if (status) {
                                        goto out;
                                }

                                //ziqing modify for backup template after enroll
                                finger_store_template_config_t template_config = { dev->current_db_file , ADD_TEMPLATE_MODE};
                                status = fpc_tee_store_template_db(dev->bio, &template_config);

                                if (status) {
                                        fpc_tee_load_template_db(dev->bio, dev->current_db_file);
                                        goto out;
                                }

                                dev->callback->on_enroll_result(dev->callback_context,
                                                id, dev->current_gid, 0);

                                status = do_enumerate(dev);
                                if (status != 0) {
                                        LOGE("%s: do enumerate failed with status = %d", __func__, status);
                                }

                                status = fpc_tee_get_template_db_id(dev->bio, &dev->authenticator_id);

                                if (status) {
                                        LOGE("%s failed to get auth id %i\n", __func__, status);
                                        dev->authenticator_id = 0;
                                }
                                status = 0;
#ifdef FPC_CONFIG_ENGINEERING
                                dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering,
                                                0, FPC_ERROR_NONE, 0, id);//FPC_ENROL_COMPLETED
#endif
                                goto out;
                        case FPC_STATUS_ENROLL_PROGRESS://FPC_ENROL_PROGRESS:
                                dev->callback->on_acquired(dev->callback_context,
                                                HAL_COMPAT_ACQUIRED_GOOD);
                                break;
                        //case FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS://FPC_ENROL_FAILED_COULD_NOT_COMPLETE:
                        //        dev->callback->on_error(dev->callback_context,
                        //                        HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
                        //        status = 0;
                        //        goto out;
                        case FPC_STATUS_ENROLL_TOO_SIMILAR://FPC_ENROL_IMAGE_TOO_SIMILAR:
                        case FPC_STATUS_ENROLL_LOW_MOBILITY:
                                dev->callback->on_acquired(dev->callback_context,
                                                HAL_COMPAT_ACQUIRED_TOO_SIMILAR);
                                break;
                        case FPC_STATUS_FINGER_ALREADY_ENROLLED://FPC_ENROL_FAILED_ALREADY_ENROLED:
                                LOGE("%s FPC_ENROL_FAILED_ALREADY_ENROLED", __func__);
                                dev->callback->on_acquired(dev->callback_context,
                                                HAL_COMPAT_ACQUIRED_ALREADY_ENROLLED);
                                remaining_samples = 9999;
                                break;
                        case FPC_STATUS_ENROLL_LOW_QUALITY://FPC_ENROL_IMAGE_LOW_QUALITY:
                                dev->callback->on_acquired(dev->callback_context,
                                                HAL_COMPAT_ACQUIRED_IMAGER_DIRTY);
                                break;
                        case FPC_STATUS_ENROLL_LOW_COVERAGE://FPC_ENROL_IMAGE_LOW_COVERAGE:
                                dev->callback->on_acquired(dev->callback_context,
                                                HAL_COMPAT_ACQUIRED_PARTIAL);
                                break;
                        default:
                                LOGE("%s, unexpected enroll_status %d", __func__, status);
                                break;
                }

                dev->callback->on_enroll_result(dev->callback_context,
                                0, dev->current_gid,
                                remaining_samples);

                status = fpc_tee_wait_finger_lost(&dev->sensor);
                if (0 == status) {
                        dev->callback->on_touch_up(dev->callback_context);
                        LOGD("%s send fingerprint TOUCH UP\n", __func__);
                } else if (status == -FPC_ERROR_CANCELLED) {
                        goto out;
                }
        }

out:

        //add for fingerprint sensor goto deep sleep
        fpc_tee_deep_sleep(dev->sensor);

        if (FAILED(status)) {
                LOGE("%s failed %s\n", __func__, fpc_error_str(status));
                switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
                        case -FPC_ERROR_CANCELLED:
                                break;
                        case -FPC_ERROR_TIMEDOUT:
                                dev->callback->on_error(dev->callback_context,
                                                HAL_COMPAT_ERROR_TIMEOUT);
                                break;
                        case -FPC_ERROR_STORAGE://-FPC_ERROR_NOSPACE:
                                dev->callback->on_error(dev->callback_context,
                                                HAL_COMPAT_ERROR_NO_SPACE);
                                break;
                        case -FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS://FPC_ENROL_FAILED_COULD_NOT_COMPLETE:
                        case -FPC_ERROR_PARAMETER:
                                dev->callback->on_error(dev->callback_context,
                                                HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
                                break;
                        default:
#ifndef FPC_CHECK_BROKEN
                                dev->callback->on_error(dev->callback_context,
                                                HAL_COMPAT_ERROR_HW_UNAVAILABLE);
#else
                                result = alive_check(dev->sensor);
                                if (result) {
                                    LOGE("%s failed on alive_check", __func__);
                                    dev->callback->on_error(dev->callback_context,
                                                    HAL_COMPAT_ERROR_ALIVE_CHECK + result);
                                    auth_context.fail_reason = result;//when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
                                } else {
                                    dev->callback->on_error(dev->callback_context,
                                                    HAL_COMPAT_ERROR_HW_UNAVAILABLE);
                                    auth_context.fail_reason = 0;//when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
                                }

                                /*send dcsmsg*/
                                auth_context.auth_result = 999; //1:pass, 0:fail, 999: HW_ERRO
                                //auth_context.fail_reason = 0; //when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
                                auth_context.quality_score = 0;
                                auth_context.match_score = 0;
                                auth_context.signal_value = 0; //UNUSED
                                auth_context.img_area = 0;
                                auth_context.retry_times = alive_test_retry_count; //identify_failed_counter
                                memcpy(auth_context.algo_version, g_lib_version, strlen(g_lib_version));
                                auth_context.chip_ic = fp_config_info_init.fp_ic_type; //UNUSED
                                auth_context.factory_id = 0; //UNUSED
                                auth_context.module_type = 0; //UNUSED
                                auth_context.lense_type = 0; //UNUSED
                                auth_context.dsp_availalbe = 0; //UNUSED

                                dev->callback->on_dcsmsg(auth_context);
#endif
                                break;
                }
        }

        fpc_tee_kpi_stop(dev->tee_handle);
}

uint64_t fpc_pre_enroll(fpc_hal_common_t* dev) {
        LOGD("%s\n", __func__);
        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);
        uint64_t challenge = 0;
#ifdef FPC_CONFIG_HW_AUTH
        int status = fpc_tee_get_enrol_challenge(dev->tee_handle, &challenge);

        if (status) {
                LOGE("%s failed %i\n", __func__, status);
                challenge = 0;
        }

#endif
        LOGD("%s challenge %" PRIu64 "\n", __func__, challenge);

        fingerprint_hal_resume(dev);
        pthread_mutex_unlock(&dev->lock);

        return challenge;
}

int fpc_post_enroll(fpc_hal_common_t* dev) {
        LOGD("%s\n", __func__);
        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);
        int status = 0;
#ifdef FPC_CONFIG_HW_AUTH
        uint64_t challenge = 0;
        status = fpc_tee_get_enrol_challenge(dev->tee_handle, &challenge);
        if (status) {
                LOGE("%s failed %i\n", __func__, status);
                status = -EIO;
        }

#endif
        fingerprint_hal_resume(dev);
        pthread_mutex_unlock(&dev->lock);

        return status;
}

uint64_t fpc_get_authenticator_id(fpc_hal_common_t* dev) {
        LOGD("%s\n", __func__);

        pthread_mutex_lock(&dev->lock);
        uint64_t id = dev->authenticator_id;
        pthread_mutex_unlock(&dev->lock);

        return id;
}

int fpc_set_active_group(fpc_hal_common_t* dev, uint32_t gid,
                const char *store_path, uint32_t* template_status) {
        int status = 0;
        bool is_template_restored = false;

        LOGD("%s\n", __func__);

        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);
        int length = snprintf(dev->current_db_file,
                        sizeof(dev->current_db_file), "%s%s",
                        store_path, TEMPLATE_POSTFIX);

        if (length < 0 || (unsigned) length >= sizeof(dev->current_db_file)) {
                status = -EINVAL;
                goto out;
        }

        status = fpc_tee_load_template_db(dev->bio, dev->current_db_file);
        if (status != 0) {
                LOGE("%s: fpc_tac_load_user_db failed first with error %s", __func__, fpc_error_str(status));
                //1. if failed in loading db, rename the db "user.db.bad", (added by ziqing)
                char temp_path[PATH_MAX];
                struct stat stat_info;
                *template_status = HAL_LOAD_TEMPLATE_RESTORED_SUCCEE;//user.db fail ,resetore succ, (added by chenran)
                if (snprintf(temp_path, PATH_MAX, "%s.bad", dev->current_db_file) >= PATH_MAX) {
                        LOGE("%s input:path too long", __func__);
                        status = -1;
                        goto out;
                }
                if (stat(dev->current_db_file, &stat_info) == 0) {
                        if (rename(dev->current_db_file, temp_path)) {
                                LOGE("%s stat failed with error %s", __func__, strerror(errno));
                                status = -1;
                                goto out;
                        }
                }

                //2. create a new "user.db" which is the same to "user.db.bak0", (added by ziqing)
                if (fpc_create_backup_db(dev->current_db_file, FP_TEMPLATE_DB_BACKUP_0)) {
                        LOGE("%s create bak0 failed with error %s", __func__, strerror(errno));
                        status = -1;
                        goto out;
                }

                //3. load the new "user.db"(bak0), (added by ziqing)
                status = fpc_tee_load_template_db(dev->bio, dev->current_db_file);
                if (status != 0) {
                        LOGE("%s: fpc_tac_load_user_db bak0 failed with error %s", __func__, fpc_error_str(status));

                        //4. if failed in loading new "user.db"(bak0), so create a new "user.db" which is the same to "user.db.bak1" (added by ziqing)
                        if (fpc_create_backup_db(dev->current_db_file, FP_TEMPLATE_DB_BACKUP_1)) {
                                status = -1;
                                goto out;
                        }

                        //5. load the new "user.db"(bak1), and if failed in loading this db, then load empty db, (added by ziqing)
                        status = fpc_tee_load_backup_template_db(dev->bio, dev->current_db_file, template_status);
                        if (status != 0) {
                                LOGE("%s: fpc_tac_load_user_db bak1 failed with error %s", __func__, fpc_error_str(status));
                                status = -1;
                                goto out;
                        }
                }

                is_template_restored = true;
        }

        status = fpc_tee_set_gid(dev->bio, gid);
        if (status) {
                goto out;
        }

        dev->current_gid = gid;

        status = fpc_tee_get_template_db_id(dev->bio, &dev->authenticator_id);

        if (status) {
                LOGE("%s failed to get auth id %i\n", __func__, status);
                dev->authenticator_id = 0;
        }

        //6. if failed in loading "user.db" previously, then sychronize the template db with framework, (added by ziqing)
        //if (is_template_restored) {
                //status = do_enumerate(dev);
                //if (status != 0) {
                        //LOGE("%s: do enumerate failed with status = %d", __func__, status);
                //}
        //}

out:
        if (status) {
                LOGE("%s failed %s\n", __func__, fpc_error_str(status));
                if (0 != (*template_status) ){
                        *template_status = HAL_LOAD_TEMPLATE_RESTORED_FAIL_EMPTYDB_FAIL;//resetore fail ,load_empty_db  fail (added by chenran)
                }
                status = -1;
        }

        fingerprint_hal_resume(dev);
        pthread_mutex_unlock(&dev->lock);

        return status;
}

int fpc_get_enrollment_total_times(fpc_hal_common_t* dev) {
        LOGD("%s\n", __func__);
        (void)dev; // Unused
        int total_times = fp_config_info_init.total_enroll_times;
        return total_times;

}
int fpc_pause_enroll(fpc_hal_common_t* dev) {
        LOGI("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        stop_enroll = true;
        pthread_mutex_unlock(&dev->lock);

        return 0;
}

int fpc_continue_enroll(fpc_hal_common_t* dev) {
        LOGI("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        pthread_mutex_lock(&enroll_lock);
        if (stop_enroll) {
                pthread_cond_signal(&enroll_cond);
                stop_enroll = false;
                LOGD("%s, signal enroll continue.", __func__);
        }
        pthread_mutex_unlock(&enroll_lock);
        pthread_mutex_unlock(&dev->lock);

        return 0;
}

int fpc_pause_identify(fpc_hal_common_t* dev) {
        LOGD("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        identify_locker.stop_identify = true;
        pthread_mutex_unlock(&dev->lock);

        return 0;
}

int fpc_continue_identify(fpc_hal_common_t* dev) {
        LOGD("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        pthread_mutex_lock(&identify_locker.identify_lock);
        if (identify_locker.stop_identify) {
                pthread_cond_signal(&identify_locker.identify_cond);
                identify_locker.stop_identify = false;
                LOGD("%s, signal identify continue.", __func__);
        }
        pthread_mutex_unlock(&identify_locker.identify_lock);
        pthread_mutex_unlock(&dev->lock);

        return 0;
}

int fpc_get_alikey_status(fpc_hal_common_t* dev) {
        LOGI("%s", __func__);
        (void)dev; // Unused
        return 0;
}

int fpc_clean_up(fpc_hal_common_t* dev) {
        LOGI("%s", __func__);
        (void)dev; // Unused
        return 0;
}

static void fpc_enable_low_sensitivity_mode(fpc_hal_common_t* dev, bool enabled) {
        if (enabled) {
                fpc_tee_set_detect_count(dev->sensor, FPC_DETECTOR_COUNT_LOCK_MODE);
        } else {
                fpc_tee_set_detect_count(dev->sensor, FPC_DETECTOR_COUNT_GENERAL);
        }
}

static void do_wait_finger_down(void* data) {
        fpc_hal_common_t* dev =(fpc_hal_common_t*) data;
        LOGD("%s", __func__);

        fpc_enable_low_sensitivity_mode(dev, true);
        while (flag_wait_touch_down) {
                if (0 == fpc_tee_wait_finger_down(&(dev->sensor), LOCK_MODE, &identify_locker)) {
                        if (0 == fpc_tee_wait_finger_lost(&dev->sensor)) {
                                dev->callback->on_touch_up(dev->callback_context);
                                LOGD("%s send fingerprint TOUCH UP\n", __func__);
                        }
                }
        }

        fpc_enable_low_sensitivity_mode(dev, false);
        //add for fingerprint sensor goto deep sleep in lock mode
        fpc_tee_deep_sleep(dev->sensor);
}

int fpc_wait_finger_down(fpc_hal_common_t* dev) {
        LOGD("%s", __func__);
        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);
        flag_wait_touch_down = true;
        fingerprint_hal_do_async_work(dev, do_wait_finger_down, dev, FPC_TASK_HAL);
        pthread_mutex_unlock(&dev->lock);
        return 0;
}

int fpc_set_screen_state(fpc_hal_common_t* dev, int screen_state) {
        LOGD("%s", __func__);
        pthread_mutex_lock(&dev->lock);
        fpc_tee_set_screen_state(screen_state);
        pthread_mutex_unlock(&dev->lock);
#ifdef FPC_CHECK_BROKEN
        if (screen_state == 0) {
            if (g_enroll_finger_counts == 0) {
                fpc_sensor_alive_check(dev);
            }
            else {
                if (waiting_finger_state == 1) {
                    fpc_set_waiting_finger_alive_check(dev, 1);
                }
            }
        }
        LOGD("%s out", __func__);
#endif
        return 0;
}

typedef struct OppoQrCodeInfo{
    uint32_t token1;
    uint32_t errcode;
    uint32_t token2;
    uint32_t qrcode_length;
    char qrcode[32];
}OppoQrCodeInfo_t;


int fpc_hal_notify_qrcode(fpc_hal_common_t* dev, int32_t cmdId) {
    LOGI("fpc_hal_notify_qrcode enter");
    OppoQrCodeInfo_t qrcodeinfo;
    char val[PROPERTY_VALUE_MAX];
    qrcodeinfo.token1 = 1;
    qrcodeinfo.errcode = 0;
    qrcodeinfo.token2 = qrcodeinfo.token1;
    property_get(PROPERTY_FINGERPRINT_QRCODE_VALUE, val, "0");
    memcpy(qrcodeinfo.qrcode, val, sizeof(qrcodeinfo.qrcode));
    LOGD("%s qrcode %s ", __func__, qrcodeinfo.qrcode);
    qrcodeinfo.qrcode_length = (uint32_t)sizeof(qrcodeinfo.qrcode);

    dev->callback->on_fingerprintcmd(dev->callback_context, cmdId, (uint8_t *)&qrcodeinfo, (uint32_t)sizeof(qrcodeinfo));
    return 0;
}

int fpc_keymode_enable(fpc_hal_common_t* dev, int enable) {
#ifdef FPC_CONFIG_NAVIGATION
        LOGI("fpc_keymode_enable enter...");
        LOGI("%s enable = %d", __func__,enable);
        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);
        if (enable == 1) {
          dev->ext_navigation->set_enabled(dev->ext_navigation, true);
        } else {
          dev->ext_navigation->set_enabled(dev->ext_navigation, false);
        }
        fingerprint_hal_resume(dev);
        pthread_mutex_unlock(&dev->lock);
        return 0;
#else
        (void)dev;
        (void)enable;
        return 0;
#endif

}

int fpc_open_log(fpc_hal_common_t* dev, int on) {
        LOGI("%s", __func__);
        (void)dev; // Unused
        (void)on; // Unused
        return 0;
}

//ziqing.guo add for engineering mode
static void do_get_image_quality(void* data) {
        int status = 0;
        fpc_hal_common_t* dev = (fpc_hal_common_t*) data;
        fpc_algo_enroll_statistics_t enrol_data;
        uint32_t remaining_samples = 0;
        LOGD("%s", __func__);
        engineering_info_t engineering;

        memset(&enrol_data, 0, sizeof(enrol_data));

        status = fpc_tee_enable_duplicate_finger_detect(dev->bio, false);
        if (status) {
                LOGE("%s fpc_tee_disable_duplicate_finger_detect failed with %d", __func__, status);
        }

        status = fpc_tee_begin_enrol(dev->bio);
        if (status) {
                LOGE("%s fpc_tac_begin_enroll failed with %d", __func__, status);
                goto out;
        }

        status = capture_image(dev, true, true, SELFTEST_MODE);
        if (status < 0) {
                LOGE("%s Get Image Quality Cancel Enroll failed.", __func__);
                goto out;
        }

        status = fpc_tee_enrol(dev->bio, &remaining_samples, &enrol_data);
        if (status < 0) {
                LOGE("%s fpc_tee_enrol failed with %d.", __func__, status);
                goto out;
        }

        LOGD("%s status %d, Enroll data %d %d %d %d", __func__,
                        status,
                        enrol_data.progress,
                        enrol_data.quality,
                        enrol_data.enrolled_template_size,
                        enrol_data.coverage);

        status = fpc_tee_cancel_enrol(dev->bio);
        if (status) {
                LOGE("%s, cancel enroll failed.", __func__);
                goto out;
        }

out:
        LOGD("%s status %d", __func__, status);

        if (status < 0) {
                engineering.quality.successed = 0;
                engineering.quality.image_quality = 0;
                engineering.quality.quality_pass = 0;
        } else {
                if(enrol_data.quality < fp_config_info_init.fp_image_quality_pass_score){
                        engineering.quality.successed = 1;
                        engineering.quality.image_quality = enrol_data.quality;
                        engineering.quality.quality_pass = 0;
                }else {
                        engineering.quality.successed = 1;
                        engineering.quality.image_quality = enrol_data.quality;
                        engineering.quality.quality_pass = 1;
                    }
        }

        engineering.type = FINGERPRINT_IMAGE_QUALITY;
        dev->callback->on_engineeringinfo_updated(dev->callback_context, engineering);

        status = fpc_tee_enable_duplicate_finger_detect(dev->bio, true);
        if (status) {
                LOGE("%s fpc_tee_enable_duplicate_finger_detect failed with %d", __func__, status);
        }
}

static int fpc_module_selfTest(fpc_hal_common_t* dev) {
        int32_t ret = 0;
        uint32_t result = 0;
        engineering_info_t engineering;
        LOGD("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);

        ret = fpc_tee_sensortest_self_test(dev->sensor, &result);
        if (ret  || result) { //selfttest result should be 1 from FPC
                LOGE("Self Test error %d, found %d dead pixels.", ret, result);
                ret = -1;
                goto out;
        }

        engineering.type = FINGERPRINT_INAGE_SELF_TEST;
        engineering.self_test_result = ret;
        dev->callback->on_engineeringinfo_updated(dev->callback_context, engineering);

        ret = fpc_tee_checkerboardtest(&(dev->sensor), &result);
        if (ret  || result) {
                LOGE("Checkerboardtest Test error %d, found %d dead pixels.", ret, result);
                ret = -1;
                goto out;
        }
out:
        //add for fingerprint sensor goto deep sleep in lock mode
        fpc_tee_deep_sleep(dev->sensor);
        fingerprint_hal_resume(dev);
        pthread_mutex_unlock(&dev->lock);
        if (ret != 0) {
                return -1;
        }
        return 0;
}

static int fpc_get_image_quality(fpc_hal_common_t* dev) {
        LOGD("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);

        fingerprint_hal_do_async_work(dev, do_get_image_quality, dev, FPC_TASK_HAL);

        pthread_mutex_unlock(&dev->lock);
        return 0;
}
static void do_get_image_snr(void* data) {
        static double last_snr_value = 0.0;
        int status = 0;
        fpc_hal_common_t* dev = (fpc_hal_common_t*) data;
        engineering_info_t engineering;
        uint32_t result = 0;
        double snr_value = 0;
        status = fpc_tee_run_mqt_test(&(dev->sensor), &result, &snr_value) ;
        LOGD("%s status %d value %f", __func__, status, snr_value);
        if(status || result) {
            if(fpc_tee_check_finger_lost(&(dev->sensor)) && (last_snr_value != 0.0)){
                LOGE("%s: finger is lost or io error", __func__);
                engineering.snr.snr_successed = 1;
                engineering.snr.image_snr = last_snr_value;
            } else {
                engineering.snr.snr_successed = 0;
                engineering.snr.image_snr = 0.0;
                last_snr_value = 0.0;
            }
        } else {
            engineering.snr.snr_successed = 1;
            engineering.snr.image_snr = snr_value;
            last_snr_value = snr_value;
        }
        engineering.type = FINGERPRINT_IMAGE_SNR;
        dev->callback->on_engineeringinfo_updated(dev->callback_context, engineering);

}

static int fpc_get_image_snr(fpc_hal_common_t *dev) {
        LOGD("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        fingerprint_hal_goto_idle(dev);

        fingerprint_hal_do_async_work(dev, do_get_image_snr, dev, FPC_TASK_HAL);

        pthread_mutex_unlock(&dev->lock);
        return 0;
}

int fpc_get_engineering_Info(fpc_hal_common_t* dev, uint32_t type) {
        LOGI("%s type = %d", __func__, type);
        int status = 0;
        if (type == FINGERPRINT_SELF_TEST) {
                status = fpc_module_selfTest(dev);
        }else if (type == FINGERPRINT_GET_IMAGET_QUALITY) {
                status = fpc_get_image_quality(dev);
        }else if(type == FINGERPRINT_GET_IMAGE_SNR){
                status = fpc_get_image_snr(dev);
        }
        return status;
}

int fpc_authenticate(fpc_hal_common_t* dev,
                uint64_t operation_id, uint32_t gid) {
        LOGD("%s operation_id %" PRIu64 "\n", __func__, operation_id);

        int status = 0;
        pthread_mutex_lock(&dev->lock);

        if (gid != dev->current_gid) {
                LOGD("%s finger.gid != current_gid\n", __func__);
                status = -1;
                goto out;
        }

        fingerprint_hal_goto_idle(dev);
        dev->challenge = operation_id;
        fingerprint_hal_do_async_work(dev, do_authenticate, dev, FPC_TASK_HAL);
out:
        pthread_mutex_unlock(&dev->lock);
        return status;
}

int fpc_enroll(fpc_hal_common_t* dev, const uint8_t* hat, uint32_t size_hat,
                uint32_t gid, uint32_t timeout_sec) {
        (void)timeout_sec; // Unused
        LOGD("%s", __func__);

        int status = 0;
        pthread_mutex_lock(&dev->lock);

        if (gid != dev->current_gid) {
                LOGD("%s finger.gid != current_gid\n", __func__);
                status = -1;
                goto out;
        }

        if (size_hat != sizeof(dev->hat)) {
                LOGD("%s hat size mismatch %d", __func__, size_hat);
                status = -1;
                goto out;
        }

        memcpy(dev->hat, hat, size_hat);

        fingerprint_hal_goto_idle(dev);
        fingerprint_hal_do_async_work(dev, do_enroll, dev, FPC_TASK_HAL);

out:
        pthread_mutex_unlock(&dev->lock);
        return status;
}

int fpc_set_waiting_finger_alive_check(fpc_hal_common_t *dev, int enable)
{
    int status = 0;
    LOGI("%s", __func__);
    if (enable) {
        status = fpc_tee_enable_alive_check(dev->sensor);

        fingerprint_auth_dcsmsg_t auth_context; //for send dcsmsg
        memset(&auth_context, 0, sizeof(auth_context));

        auth_context.auth_result = 999; //1:pass, 0:fail, 999: HW_ERRO
        auth_context.fail_reason = 20; //when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
        auth_context.quality_score = 0;
        auth_context.match_score = 0;
        auth_context.signal_value = 0; //UNUSED
        auth_context.img_area = 0;
        auth_context.retry_times = alive_test_retry_count; //identify_failed_counter
        memcpy(auth_context.algo_version, g_lib_version, strlen(g_lib_version));
        auth_context.chip_ic = fp_config_info_init.fp_ic_type; //UNUSED
        auth_context.factory_id = 0; //UNUSED
        auth_context.module_type = 0; //UNUSED
        auth_context.lense_type = 0; //UNUSED
        auth_context.dsp_availalbe = 0; //UNUSED

        dev->callback->on_dcsmsg(auth_context);
    }
    else {
        status = fpc_tee_disable_alive_check(dev->sensor);
    }
    return status;
}

static void do_sensor_alive_check(void *data)
{
    fpc_hal_common_t *dev = (fpc_hal_common_t *) data;
    uint32_t result = 0;
    result = alive_check(dev->sensor);

    if (result) {
        //fpc_hal_close(dev);
        dev->callback->on_error(dev->callback_context,
                        HAL_COMPAT_ERROR_ALIVE_CHECK + result);

        LOGD("%s alive_check failed", __func__);
    }

    /*send dcsmsg*/
    fingerprint_auth_dcsmsg_t auth_context; //for send dcsmsg
    memset(&auth_context, 0, sizeof(auth_context));

    auth_context.auth_result = 999; //1:pass, 0:fail, 999: HW_ERRO
    auth_context.fail_reason = result;//when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
    auth_context.quality_score = 0;
    auth_context.match_score = 0;
    auth_context.signal_value = 0; //UNUSED
    auth_context.img_area = 0;
    auth_context.retry_times = alive_test_retry_count; //identify_failed_counter
    memcpy(auth_context.algo_version, g_lib_version, strlen(g_lib_version));
    auth_context.chip_ic = fp_config_info_init.fp_ic_type; //UNUSED
    auth_context.factory_id = 0; //UNUSED
    auth_context.module_type = 0; //UNUSED
    auth_context.lense_type = 0; //UNUSED
    auth_context.dsp_availalbe = 0; //UNUSED

    dev->callback->on_dcsmsg(auth_context);
}


int fpc_sensor_alive_check(fpc_hal_common_t *dev)
{
    LOGD("%s", __func__);

    pthread_mutex_lock(&dev->lock);

    fingerprint_hal_goto_idle(dev);
    fingerprint_hal_do_async_work(dev, do_sensor_alive_check, dev, FPC_TASK_HAL);

    pthread_mutex_unlock(&dev->lock);
    return 0;
}

int fpc_cancel(fpc_hal_common_t* dev) {
        LOGD("%s", __func__);

        pthread_mutex_lock(&dev->lock);
        if (dev->current_task.owner == FPC_TASK_HAL) {
                fingerprint_hal_goto_idle(dev);
                fingerprint_hal_resume(dev);
        }
        /*miss up messege lead to power cannot lock, bug id: 776861*/
        dev->callback->on_touch_up(dev->callback_context);
        dev->callback->on_error(dev->callback_context, HAL_COMPAT_ERROR_CANCELED);

        pthread_mutex_unlock(&dev->lock);
        return 0;
}

static void do_remove(void* device) {
        fpc_hal_common_t* dev = (fpc_hal_common_t*) device;

        uint32_t ids[5];
        uint32_t size = 5;

        int status = 0;

        if (dev->remove_fid == 0) {
                status = fpc_tee_get_finger_ids(dev->bio, &size, ids);
                if (status) {
                        status = -EIO;
                        goto out;
                }
                if (size == 0) {
                        dev->callback->on_removed(dev->callback_context,
                                0, dev->current_gid, 0);
                }

                for (unsigned i = 0; i < size; ++i) {
                        status = fpc_tee_delete_template(dev->bio, ids[i]);
                        if (status != 0 && FPC_ERROR_GET_EXTERNAL_ERROR(status) != -FPC_ERROR_NOT_FOUND) {//status != -FPC_ERROR_NOENTITY) {
                                LOGE("%s delete_tempalte failed %i", __func__, status);
                                status = -EIO;
                                goto out;
                        }
                        dev->callback->on_removed(dev->callback_context,
                                ids[i], dev->current_gid,
                                (size -1) - i);
                }
                status = 0;
        } else {
                status = fpc_tee_delete_template(dev->bio, dev->remove_fid);
                if (status != 0 && FPC_ERROR_GET_EXTERNAL_ERROR(status) != -FPC_ERROR_NOT_FOUND) {//status != -FPC_ERROR_NOENTITY) {
                        LOGE("%s delete_tempalte failed %i", __func__, status);
                        status = -EIO;
                        goto out;
                }

                dev->callback->on_removed(dev->callback_context,
                                dev->remove_fid, dev->current_gid,
                                0);
        }

        status = do_enumerate(dev);
        if (status != 0) {
                LOGE("%s: do enumerate failed with status = %d", __func__, status);
        }

        //ziqing modify for backup template after remove
        finger_store_template_config_t template_config = { dev->current_db_file , REMOVE_TEMPLATE_MODE};
        status = fpc_tee_store_template_db(dev->bio, &template_config);
        if (status) {
                LOGE("%s store_template_db failed %i", __func__, status);
        }

out:

        if (status) {
                dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_UNABLE_TO_REMOVE);

                LOGE("%s failed %i, reloading db\n", __func__, status);
                status = fpc_tee_load_template_db(dev->bio, dev->current_db_file);
                if (status != 0) {
                        LOGE("%s: fpc_tac_load_user_db failed with error %s", __func__,
                                        fpc_error_str(status));
                }
        }
}

int fpc_remove(fpc_hal_common_t* dev, uint32_t gid, uint32_t fid) {
        int status = 0;

        LOGD("%s(fid=%d gid=%d)", __func__, fid, gid);

        pthread_mutex_lock(&dev->lock);

        if (gid != dev->current_gid) {
                LOGD("%s gid != current_gid, nothing to remove\n", __func__);
                status = -2;
                goto out;
        }

        dev->remove_fid = fid;

        fingerprint_hal_goto_idle(dev);
        fingerprint_hal_do_async_work(dev, do_remove, dev, FPC_TASK_HAL);


out:
        pthread_mutex_unlock(&dev->lock);

        return status;
}

int fpc_enumerate(fpc_hal_common_t* dev) {
        LOGD("%s", __func__);
        int status = 0;
        status = do_enumerate(dev);
        if (status != 0) {
                LOGE("%s: do enumerate failed with status = %d", __func__, status);
        }
#if 0
        (void) dev; // unused
        pthread_mutex_lock(&dev->lock);

        fingerprint_hal_goto_idle(dev);
        fingerprint_hal_do_async_work(dev, do_enumerate, dev, FPC_TASK_HAL);

        pthread_mutex_unlock(&dev->lock);
#endif
        return 0;
}

void fpc_hal_close(fpc_hal_common_t* device) {
        LOGD("%s", __func__);

        if (!device) {
                return;
        }

        fpc_hal_common_t* dev = (fpc_hal_common_t*) device;

        pthread_mutex_lock(&dev->lock);

        if (dev->tee_handle) {
                fingerprint_hal_goto_idle(dev);
        }

        destroy_input_device();
        fpc_worker_destroy(dev->worker);

        oplus_worker_destroy();

#ifdef FPC_CONFIG_SENSORTEST
        fpc_sensortest_destroy(dev->ext_sensortest);
#endif

#ifdef FPC_CONFIG_ENGINEERING
        fpc_engineering_destroy(dev->ext_engineering);
#endif

#ifdef FPC_CONFIG_QC_AUTH
        fpc_authenticator_destroy(dev->ext_authenticator);
#endif

#ifdef FPC_CONFIG_NAVIGATION
        fpc_navigation_destroy(dev->ext_navigation);
#endif

#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
        fpc_calibration_destroy(dev->ext_calibration);
#endif

#ifdef FPC_CONFIG_APNS
        fpc_recalibration_destroy(dev->ext_recalibration);
#endif

#if (FPC_CONFIG_FORCE_SENSOR == 1)
        fpc_sense_touch_destroy(dev->ext_sensetouch);
#endif

        fpc_tee_sensor_release(dev->sensor);

        fpc_tee_bio_release(dev->bio);

        fpc_tee_release(dev->tee_handle);

        pthread_mutex_unlock(&dev->lock);
        pthread_mutex_destroy(&dev->lock);
        free(device);
        LOGD("%s end", __func__);
}

int fpc_hal_open(fpc_hal_common_t** device,
                const fpc_hal_compat_callback_t* callback,
                void* callback_context) {
        int status = -1;
        char lib_version[128] = {'\0'};

        LOGD("%s", __func__);

        *device = NULL;

        fpc_hal_common_t *dev = (fpc_hal_common_t*)
                malloc(sizeof(fpc_hal_common_t));

        if (!dev) {
                return -ENOMEM;
        }

        memset(dev, 0, sizeof(fpc_hal_common_t));

        dev->callback = callback;
        dev->callback_context = callback_context;
        pthread_mutex_init(&dev->lock, NULL);

        oplus_worker_init();
        fpc_tee_set_1v8_on();

        dev->tee_handle = fpc_tee_init();
        if (!dev->tee_handle) {
                LOGE("%s Failed to init tee.", __func__);
                goto err;
        }
#ifdef FPC_CONFIG_NAVIGATION
        if (create_input_device()) {
                LOGE("%s Failed to create input device.", __func__);
                goto err;
        }
#endif
        if (fpc_tee_log_build_info(dev->tee_handle)) {
                LOGD("%s, An error occured print build info.\n", __func__);
        }

        memset(g_lib_version, 0, sizeof(g_lib_version));
        memset(lib_version, 0, sizeof(lib_version));
        if (fpc_tee_get_lib_version(dev->tee_handle, lib_version)) {
            LOGD("%s, An error occurred in fpc_tee_get_lib_version.\n", __func__);
        }

        if (strlen(lib_version) > 0) {
            memcpy(g_lib_version, lib_version, strlen(lib_version));
        } else {
            memcpy(g_lib_version, ALGO_VERSION, strlen(ALGO_VERSION));
        }
        LOGD("%s, lib version:%s, g_lib_version:%s", __func__, lib_version, g_lib_version);

        property_set(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, g_lib_version);

        dev->sensor = fpc_tee_sensor_init(dev->tee_handle);
        /*send dcsmsg*/
        fingerprint_auth_dcsmsg_t auth_context; //for send dcsmsg
        memset(&auth_context, 0, sizeof(auth_context));
        auth_context.auth_result = 999; //1:pass, 0:fail, 999: HW_ERRO
        auth_context.fail_reason = 0; //when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
        auth_context.quality_score = 0;
        auth_context.match_score = 0;
        auth_context.signal_value = 0; //UNUSED
        auth_context.img_area = 0;
        auth_context.retry_times = alive_test_retry_count; //identify_failed_counter
        memcpy(auth_context.algo_version, g_lib_version, strlen(g_lib_version));
        auth_context.chip_ic = fp_config_info_init.fp_ic_type; //UNUSED
        auth_context.factory_id = 0; //UNUSED
        auth_context.module_type = 0; //UNUSED
        auth_context.lense_type = 0; //UNUSED
        auth_context.dsp_availalbe = 0; //UNUSED

        if (!dev->sensor) {
                LOGE("%s Failed to init sensor.", __func__);
                dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_ALIVE_CHECK + 1);
                status = 99;

                auth_context.auth_result = 999; //1:pass, 0:fail, 999: HW_ERRO
                auth_context.fail_reason = 999; //when HW_ERRO:0:PASS 1~16 erro  20:waiting_finger 999:open_hw_erro
                dev->callback->on_dcsmsg(auth_context);

                goto err;
        }
        dev->callback->on_dcsmsg(auth_context);

        dev->bio = fpc_tee_bio_init(dev->tee_handle);
        if (!dev->bio) {
                LOGE("%s Failed to init bio.", __func__);
                goto err;
        }

#ifdef FPC_CONFIG_HW_AUTH
        if (fpc_tee_init_hw_auth(dev->tee_handle)) {
                goto err;
        }
#endif
        fpc_tee_set_detect_threshold(dev->sensor, fp_config_info_init.fp_threshold_count[0]);
        if(!fp_config_info_init.fp_threshold_count[1]){
                fpc_tee_set_detect_count(dev->sensor, fp_config_info_init.fp_threshold_count[1]); 
        }
        LOGE("%s threshold =%d, count =%d", __func__, fp_config_info_init.fp_threshold_count[0], fp_config_info_init.fp_threshold_count[1]);

        dev->worker = fpc_worker_new();
        if (!dev->worker) {
                LOGE("%s Failed to new worker.", __func__);
                goto err;
        }

#ifdef FPC_CONFIG_ENGINEERING
        dev->ext_engineering = fpc_engineering_new(dev);
        if (!dev->ext_engineering) {
                goto err;
        }
        //add_engineering_service(dev->ext_engineering);
#endif

#ifdef FPC_CONFIG_SENSORTEST
        dev->ext_sensortest = fpc_sensortest_new(dev);
        if (!dev->ext_sensortest) {
                goto err;
        }
#endif

#ifdef FPC_CONFIG_QC_AUTH
        dev->ext_authenticator = fpc_authenticator_new(dev);
        if (!dev->ext_authenticator) {
                goto err;
        }
        add_authenticator_service(dev->ext_authenticator);
#endif

#if (FPC_CONFIG_FORCE_SENSOR == 1)
        dev->ext_sensetouch = fpc_sense_touch_new(dev);
        fpc_sense_touch_load_config();
        if (!dev->ext_sensetouch) {
                goto err;
        }
        add_sense_touch_service(dev->ext_sensetouch);
#endif

#ifdef FPC_CONFIG_NAVIGATION
        dev->ext_navigation = fpc_navigation_new(dev);
        if (!dev->ext_navigation) {
                goto err;
        }
#endif

#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
        dev->ext_calibration = fpc_calibration_new(dev);
        if (!dev->ext_calibration) {
                goto err;
        }
        add_calibration_service(dev->ext_calibration);
#endif

#ifdef FPC_CONFIG_APNS
        dev->ext_recalibration = fpc_recalibration_new(dev);
        if (!dev->ext_recalibration) {
                goto err;
        }
        add_recalibration_service(dev->ext_recalibration);

        fpc_load_pn(dev);
#endif

        *device = dev;

        fingerprint_hal_resume(dev);

        return 0;
err:
        LOGE("%s failed\n", __func__);
        fpc_tee_set_1v8_off();
        fpc_hal_close(dev);
        return status;
}
