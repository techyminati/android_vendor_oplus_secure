/************************************************************************************
 ** File: - fpc\fpc_tac\normal\src\fpc_tee_sensor.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE SENSOR PART for FPC (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,14/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Ziqing.guo   2017/10/14        create the file
 **  Ziqing.guo   2017/10/21        customization for homekey
 **  Ziqing.guo   2017/10/26        remove the macro FPC_CONFIG_FINGER_LOST_INTERRUPT
 **  Ziqing.guo   2017/10/29        add to report touch down
 **  Ziqing.guo   2017/11/06        add for retry
 **  Ziqing.guo   2017/11/07        add for tp protect 16051
 **  Ran.Chen     2018/01/29        modify for fp_id, Code refactoring
 **  Ran.Chen     2018/02/12        modify for fp_back_fingerprint flag err
 **  Ran.Chen     2018/03/21        add for fpc_monitor
 **  Long.Liu     2018/12/01        open hypnus function
 ************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <cutils/properties.h>

#include "fpc_types.h"
#include "fpc_tac.h"
#include "fpc_log.h"
#include "fpc_ta_targets.h"
#include "fpc_ta_interface.h"
#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_irq_device.h"
#include "fpc_reset_device.h"
#include "fpc_ta_sensor_interface.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_sensor_internal.h"
#include "fpc_tee_hal.h"
#include "fingerprint_type.h"
#include "fpc_monitor.h"
#include "fpc_tee_sensortest.h"

extern uint32_t waiting_finger_state;
extern uint32_t alive_test_retry_count;
#define CHECK_FINGER_UP_SLEEP_TIME_MS   20
#define USECS_PER_MSEC                  1000
#define NS_PER_MSEC                     1000000l
#define FPC_MS_PER_SEC                  1000
#define MAX_FINGER_LOST_CHECK_TIME_MS   2000l

/* Retry 10 times for qualify capture, this gives a limit of about 500 ms to place the finger */
#define MAX_QUALIFY_CAPTURE_RETRIES 10

/* Number of times to retry the communciation */
#ifndef MAX_TRANSFER_RETRIES
#define MAX_TRANSFER_RETRIES           2
#endif

/*
 * Increment in ms for the sleep time for each communcation attempt,
 * that means the total wait time is
 * MAX_TRANSFER_RETRIES * (MAX_TRANSFER_RETRIES - 1) / 2 * TRANSFER_RETRY_SLEEP_INC_MS
 */
#ifndef TRANSFER_RETRY_SLEEP_INC_MS
#define TRANSFER_RETRY_SLEEP_INC_MS     10
#endif

#ifndef TRANSFER_RETRY_MAX_SLEEP_TIME
#define TRANSFER_RETRY_MAX_SLEEP_TIME   500
#endif

#define TP_PROTECTED_COUNTER            25

//Haitao.Zhou Add for TP protect
static fingerprint_screen_state_type_t g_screen_state = FINGERPRINT_SCREEN_ON;

static int fpc_tee_tp_protect_trigger(void);

static int read_proc_int_data(char *path) {
        //int fd = -1;
        char sys_buffer_data[128] = {0};

        if (read_proc_string_data(path, sys_buffer_data) == NULL) {
                return -1;
        }
        return atoi(sys_buffer_data);
}

//Haitao.zhou add write proc data func for TP touch
static int write_proc_string_data(char *path, const char *buf) {
        int fd = -1;
        int size = strlen(buf);

        LOGD("%s", __func__);
        if ((fd = open(path, O_WRONLY)) >= 0) {
                if (write(fd, buf, size) != size) {
                        LOGE("fpc %s write %s error!!! (%s)", __func__, path, strerror(errno));
                        close(fd);
                        return -1;
                }
                close(fd);
        } else {
                LOGE(" fpc %s open error!!! (%s)", __func__, strerror(errno));
                return -1;
        }
        return 0;
}

//Haitao.Zhou Add for TP protect
static int fpc_tee_tp_protect_trigger(void) {
        if(fp_config_info_init.fp_type == 3){
                return 0;
        }else{
                return write_proc_string_data("/proc/touchpanel/finger_protect_trigger", "1");
        }
}

fingerprint_tp_protect_state_t fpc_tee_get_tp_protect_result(void) {
        int counter = TP_PROTECTED_COUNTER;
        fingerprint_tp_protect_state_t ret = TP_TOUCH_UP;
        finger_protect_status_t tp_state = FINGER_PROTECT_NOTREADY;

        if(fp_config_info_init.fp_type == 3){
                (void)counter;
                (void)ret;
                (void)tp_state;
                return TP_TOUCH_UP;
        }

        if (FINGERPRINT_SCREEN_OFF == fpc_tee_get_screen_state()) {
                do {
                        tp_state = read_proc_int_data("/proc/touchpanel/finger_protect_result");
                        if (tp_state != FINGER_PROTECT_NOTREADY) {
                                break;
                        }

                        LOGD("%s wait for TP data ready, counter %d.", __func__, counter);
                        usleep(10 * 1000);
                        counter--;
                } while (tp_state == FINGER_PROTECT_NOTREADY && counter);

                if (tp_state == FINGER_PROTECT_TOUCH_DOWN) {
                        LOGD("%s TP protected!", __func__);
                        ret = TP_TOUCH_DOWN;
                } else if (tp_state == FINGER_PROTECT_TOUCH_UP) {
                        LOGD("%s TP no protected!", __func__);
                        ret = TP_TOUCH_UP;
                } else {
                        LOGD("%s TP still no ready for read!!!", __func__);
                }
        }

        return ret;
}

/*** internal api ***/
int fpc_tee_sensor_cancelled(fpc_tee_sensor_t* sensor) {
        pthread_mutex_lock(&sensor->mutex);
        int cancelled = sensor->cancelled;
        pthread_mutex_unlock(&sensor->mutex);
        return cancelled;
}

int fpc_tee_set_screen_state(fingerprint_screen_state_type_t screen_state) {
        g_screen_state = screen_state;
        LOGD("%s %d", __func__, g_screen_state);
        return 0;
}

fingerprint_screen_state_type_t fpc_tee_get_screen_state(void) {
        LOGD("%s %d", __func__, g_screen_state);
        return g_screen_state;
}

static int sensor_transfer(fpc_tee_sensor_t* sensor) {
    int32_t status = -1;
    int32_t sleep_time = 0;

    fpc_tee_t* tee = sensor->tee;

    for (int i = 0; i < MAX_TRANSFER_RETRIES; i++) {

        status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

        /* Only reset and retry on hardware error */
        if (FPC_ERROR_GET_EXTERNAL_ERROR(status) != -FPC_ERROR_SPI) {
            goto out;
        }

#ifdef FPC_CONFIG_NORMAL_SPI_RESET
        status = fpc_reset_spi(sensor->reset);
        if (status) {
            LOGE("%s, spi hardware reset failed", __func__);
        }
#endif

#ifdef FPC_CONFIG_NORMAL_SENSOR_RESET
        status = fpc_reset_sensor(sensor->reset);
        if (status) {
            LOGE("%s, spi hardware reset failed", __func__);
        }
#endif

        status = -FPC_ERROR_SENSOR;

        /* Increase the wait time slightly to increase the chance of
         * recovery */
        if (sleep_time >= TRANSFER_RETRY_MAX_SLEEP_TIME) {
            sleep_time = TRANSFER_RETRY_MAX_SLEEP_TIME;
        } else {
            sleep_time += TRANSFER_RETRY_SLEEP_INC_MS;
        }
        usleep(sleep_time * USECS_PER_MSEC);
    }

out:
    return status;
}

int fpc_tee_wait_irq(fpc_tee_sensor_t* sensor, int irq_value)
{
    return fpc_irq_wait(sensor->irq, irq_value);
}

int fpc_tee_status_irq(fpc_tee_sensor_t* sensor)
{
    return fpc_irq_status(sensor->irq);
}

static int sensor_command(fpc_tee_sensor_t* sensor, int32_t command_id)
{

    if (!sensor) {
        return -FPC_ERROR_PARAMETER;
    }

    fpc_tee_t* tee = sensor->tee;

    if (!tee || !tee->shared_buffer) {
        return -FPC_ERROR_PARAMETER;
    }

    fpc_ta_sensor_command_t* command = (fpc_ta_sensor_command_t*) tee->shared_buffer->addr;
    command->header.command = command_id;
    command->header.target = TARGET_FPC_TA_SENSOR;

    int status = sensor_transfer(sensor);
    if (FAILED(status)) {
        LOGE("%s, Failed to send command: %d to TA, status code: %d", __func__, command_id, status);
    }

    return status;
}

#if 0
#if (FPC_CONFIG_FINGER_LOST_INTERRUPT != 1) || (FPC_CONFIG_FORCE_SENSOR == 1)
static long fpc_get_ms_diff(struct timespec *ts_start, struct timespec *ts_stop)
{
    time_t sec_diff = ts_stop->tv_sec - ts_start->tv_sec;
    long ms_diff = (ts_stop->tv_nsec - ts_start->tv_nsec) / NS_PER_MSEC;

    if (sec_diff > 0) {
        ms_diff += FPC_MS_PER_SEC * sec_diff;
    }
    return ms_diff;
}
#endif
#endif

int fpc_tee_set_1v8_off()
{
    //TODO: add 1.8V power off control
    LOGD("%s ", __func__);
    return fpc_power_disable_by_sysfs();
}
int fpc_tee_set_1v8_on() {
    //TODO: add 1.8V power on control
    LOGD("%s ", __func__);
    return fpc_power_enable_by_sysfs();
}

int alive_check(fpc_tee_sensor_t *sensor)
{
    uint32_t result = 0;
    int status = 0;
    int fail_retry = 5;
    //struct timespec start_time;
    //struct timespec current_time;
    LOGD("%s", __func__);
    alive_test_retry_count = 0;
    if (!sensor) {
        return -FPC_ERROR_PARAMETER;
    }
    //clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    do {
        status = fpc_tee_sensortest_self_test(sensor, &result);
        alive_test_retry_count = alive_test_retry_count + 1;
        if (alive_test_retry_count == fail_retry) {
            break;
        }
        usleep(1000*10);
    }while(result || status);

    if (result) {
        LOGD("%s failed, result = %d", __func__, result);
        fpc_tee_set_1v8_off();
    } else {
        if (0 == strcmp("F_1541", fp_config_info_init.fp_id_string)) {
            status = fpc_tee_sensortest_force_defeat_pixel_test(sensor, &result);
            if (status || result) {
                LOGD("%s failed, result = %d", __func__, result);
                fpc_tee_set_1v8_off();
                return result;
            }
        }
        LOGD("%s ok", __func__);
    }
    fpc_tee_deep_sleep(sensor);
    //clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    //LOGD("%s cost time: %ld", __func__, fpc_get_ms_diff(&start_time, &current_time));
    return result;
}

int fpc_tee_enable_alive_check(fpc_tee_sensor_t *sensor)
{
    LOGI("%s", __func__);

    if (!sensor) {
        return -FPC_ERROR_PARAMETER;
    }
    pthread_mutex_lock(&sensor->mutex);
    sensor->alive_check = 1;
    //sensor->cancelled = 1;
    fpc_irq_set_cancel(sensor->irq);
    pthread_mutex_unlock(&sensor->mutex);
    return 0;
}
int fpc_tee_disable_alive_check(fpc_tee_sensor_t *sensor)
{
    LOGI("%s", __func__);

    if (!sensor) {
        return -FPC_ERROR_PARAMETER;
    }
    pthread_mutex_lock(&sensor->mutex);
    sensor->alive_check = 0;
    pthread_mutex_unlock(&sensor->mutex);
    //if(sensor->cancelled)
        fpc_tee_clear_cancel(sensor);
    return 0;
}

static int fpc_tee_wait_finger_lost_with_interrupt(fpc_tee_sensor_t** sensor)
{
    static const int status_finger_lost    = 1,
                     status_finger_present = 0;

    int status;

    for (;;) {
        status = sensor_command(*sensor, FPC_TA_SENSOR_FINGER_LOST_WAKEUP_SETUP_CMD);
        if (status) {
            goto out;
        }
#ifdef FPC_CHECK_BROKEN
        if (fpc_tee_sensor_cancelled(*sensor)) {
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }
        waiting_finger_state = 1;
        status = fpc_irq_wait((*sensor)->irq, 1);
        waiting_finger_state = 0;
        if (status) {
            LOGE("%s, irq wait error status: %d", __func__, status);
            if (-FPC_ERROR_CANCELLED == status) {
                if ((*sensor)->alive_check) {
                    fpc_tee_disable_alive_check((*sensor));
                    if (alive_check((*sensor))) {
                        status = -FPC_ERROR_SENSOR;
                        goto out;
                    }
                    continue;
                }
                goto out;
            }
            else
                goto out;
        }
#else
        status = fpc_irq_wait((*sensor)->irq, 1);
        if (status) {
            goto out;
        }
#endif

        status = sensor_command(*sensor, FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD);
        if (status == status_finger_lost) {
            status = 0;
            break;
        } else if (status == status_finger_present) {
            continue;
        } else {
            goto out;
        }
	}
        monitor_power_get_touchup_time(&fp_monitor.power);
        monitor_trigger(sensor, &fp_monitor, POWER_MONITOR);
out:
        return status;
}

static int fpc_tee_wait_finger_lost_without_interrupt(fpc_tee_sensor_t** sensor) {
        int status = 0;
        LOGD("%s", __func__);
#ifdef FPC_CONFIG_WAKE_LOCK
        fpc_wakeup_enable((*sensor)->irq);
#endif /*OPLUS_FEATURE_FINGERPRINT*/

        for (;;) {
                if (fpc_tee_sensor_cancelled(*sensor)) {
                        status = -FPC_ERROR_CANCELLED;
                        goto out;
                }

                status = sensor_command(*sensor, FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD);
                if (status > 0) {
                        status = 0;
                        monitor_power_get_touchup_time(&fp_monitor.power);
                        monitor_trigger(sensor, &fp_monitor, POWER_MONITOR);
                        break;
                } else if (status < 0) {
                        goto out;
                } else {
                        usleep(CHECK_FINGER_UP_SLEEP_TIME_MS * USECS_PER_MSEC);
                }
        }

out:
#ifdef FPC_CONFIG_WAKE_LOCK
        fpc_wakeup_disable((*sensor)->irq);
#endif /*OPLUS_FEATURE_FINGERPRINT*/
        return status;
}

int fpc_tee_wait_finger_lost(fpc_tee_sensor_t** sensor) {
        if (!fp_config_info_init.fp_up_irq) {
                return fpc_tee_wait_finger_lost_without_interrupt(sensor);
        } else {
                return fpc_tee_wait_finger_lost_with_interrupt(sensor);
        }
}

int fpc_tee_check_finger_lost(fpc_tee_sensor_t** sensor) {
        int status = 0;
        status = sensor_command(*sensor, FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD);
        if (status == 1) {
                return 1;
        } else {
                if (status < 0) {
                        LOGE("%s, Failed to check finger lost, status code: %d", __func__, status);
                }
                LOGD("%s, status : %d", __func__, status);
                return 0;
        }
}

int fpc_tee_set_zone(fpc_tee_sensor_t* sensor, uint32_t nZoneLimit) {
        int status = 0;

        fpc_ta_set_zone_command_t* command =
                (fpc_ta_set_zone_command_t*) sensor->tee->shared_buffer->addr;

        command->val = nZoneLimit;
        status = sensor_command(sensor, FPC_TA_TEE_SET_ZONE_CMD);

        if (status < 0) {
                LOGE("%s, Failed to set zone, status code: %d", __func__, status);
                goto out;
        }
        LOGD("%s, status code: %d", __func__, status);
        //status = command->response;
out:
        return status;
}

int fpc_tee_check_finger_present(fpc_tee_sensor_t* sensor) {
        return fpc_tee_set_zone(sensor, ZONE_LIMIT_HOMEKEY);
}

int fpc_tee_wait_finger_down(fpc_tee_sensor_t** sensor, fingerprint_mode_t mode, identify_lock_t* locker) {
        int status = 0;

        fpc_hal_common_t* dev = p_fpc_hal_common(sensor);
        LOGD("%s", __func__);

        for (;;) {
                LOGD("%s  first for", __func__);
                if (fpc_tee_sensor_cancelled(*sensor)) {
                        status = -FPC_ERROR_CANCELLED;
                        LOGE("%s fpc_tee_sensor_cancelled", __func__);
                        return status;
                }
#ifdef FPC_CHECK_BROKEN
            do {
                LOGD("%s  sec do while", __func__);
                status = sensor_command(*sensor, FPC_TA_SENSOR_WAKEUP_SETUP_CMD);

                if (status) {
                        return status;
                }
                waiting_finger_state = 1;
                status = fpc_irq_wait((*sensor)->irq, 1);
                waiting_finger_state = 0;
                if (status) {
                    LOGE("%s, irq wait error status: %d", __func__, status);
                    if (-FPC_ERROR_CANCELLED == status) {
                        if ((*sensor)->alive_check) {
                            fpc_tee_disable_alive_check((*sensor));
                            if (alive_check((*sensor))) {
                                status = -FPC_ERROR_SENSOR;
                                return status;
                            }
                            continue;
                        }
                        else
                            return status;
                    }
                    else
                        return status;
                }
                break;
            }while(1);
                //if (status) {
                //        LOGE("%s fpc_irq_wait failed status = %d", __func__, status);
                //        return status;
                //}
#else
                status = sensor_command(*sensor, FPC_TA_SENSOR_WAKEUP_SETUP_CMD);

                if (status) {
                        return status;
                }

                status = fpc_irq_wait((*sensor)->irq, 1);

                if (status) {
                        LOGE("%s fpc_irq_wait failed status = %d", __func__, status);
                        return status;
                }
#endif
                if (locker) {
                        pthread_mutex_lock(&locker->identify_lock);
                        if ((mode == IDENTIFY_MODE) && locker->stop_identify) {
                                LOGD("%s identify stop in", __func__);
                                sensor_command(*sensor, FPC_TA_SENSOR_DEEP_SLEEP_CMD);
                                pthread_cond_wait(&locker->identify_cond, &locker->identify_lock);
                                LOGD("%s identify stop out", __func__);
                                pthread_mutex_unlock(&locker->identify_lock);
                                continue;
                        }
                        pthread_mutex_unlock(&locker->identify_lock);
                }

                // Qualify image
                status = sensor_command(*sensor, FPC_TA_WAKEUP_QUALIFICATION_CMD);

                if (status < 0) {
                        return status;
                } else if (status == 0) {
                        //jie.cheng Add hypnus action for fingerprintd
                        LOGD("%s start fpc_bind_bigcore_bytid", __func__);
                        dev->callback->fpc_bind_bigcore_bytid(gettid());

#ifdef FP_HYPNUSD_ENABLE
                        LOGD("%s start hypnusSetAction", __func__);
                        dev->callback->hypnusSetAction();
#endif /* FP_HYPNUSD_ENABLE */


                        //Haitao.zhou add write proc data func for TP touch
                        //write 1 for trigger TP driver(3508) check finger
                        //Ziqing modify for trigger should be under screen off condition
                        if (FINGERPRINT_SCREEN_OFF == fpc_tee_get_screen_state()) {
                                if (0 != fpc_tee_tp_protect_trigger()) {
                                        LOGE("%s fpc_tee_tp_protect_trigger failed!!!", __func__);
                                }
                        }

                        //16037 16027 16103 use incell display (if there are more projects using incell, add here), cannot check TP protect when LOCK mode here
                        if (mode == LOCK_MODE) {
                                if (TP_TOUCH_DOWN == fpc_tee_get_tp_protect_result()) {
                                        monitor_tp_protect_set_mode(&fp_monitor.tp_protect, LOCK_MODE);
                                        monitor_trigger(sensor, &fp_monitor, TP_PROTECT_MONITOR);
                                        status = fpc_tee_wait_finger_lost(sensor);
                                        if (status != 0) {
                                                return status;
                                        }
                                        else {
                                                continue;
                                        }
                                }
                        }

                        dev->callback->on_touch_down(dev->callback_context);
                        monitor_power_get_touchdown_time(&fp_monitor.power);
                        return 0;
                }
        }
}

int fpc_tee_capture_image(fpc_tee_sensor_t** sensor, bool need_wait_finger_up,
                bool need_wait_finger_down, fingerprint_mode_t mode, identify_lock_t* locker) {
        LOGD("%s", __func__);
        (*sensor)->cac_result = 0;

        int status = fpc_irq_wakeup_enable((*sensor)->irq);
        if (status) {
                goto out;
        }

        if (need_wait_finger_up) {
                status = fpc_tee_wait_finger_lost(sensor);
                if (status) {
                        LOGE("%s fpc_tee_wait_finger_lost failed status = %d", __func__, status);
                        goto out;
                }
        }

        // wait for irq(finger down)->qualify->capture
        if (need_wait_finger_down) {
                status = fpc_tee_wait_finger_down(sensor, mode, locker);
                if (status) {
                        LOGE("%s fpc_tee_wait_finger_down failed status = %d", __func__, status);
                        goto out;
                }
        }

        // Qualify and capture image
        status = sensor_command(*sensor, FPC_TA_CAPTURE_IMAGE_CMD);
        fpc_ta_sensor_capture_info_t* command =
                (fpc_ta_sensor_capture_info_t*) (*sensor)->tee->shared_buffer->addr;
        (*sensor)->cac_result = command->cac_result;

out:
        fpc_irq_wakeup_disable((*sensor)->irq);

        if (status) {
                sensor_command(*sensor, FPC_TA_SENSOR_DEEP_SLEEP_CMD);
        }

        return status;
}

int fpc_tee_set_detect_count(fpc_tee_sensor_t* sensor, uint16_t detect_count) {
        LOGD("%s", __func__);
        int status = 0;

        fpc_ta_fp_sensitivity_command_t* command =
                (fpc_ta_fp_sensitivity_command_t*) sensor->tee->shared_buffer->addr;

        command->val = detect_count;
        status = sensor_command(sensor, FPC_TA_TEE_SET_DET_COUNT_CMD);

        if (status) {
                LOGE("%s, Failed to set detect counter, status code: %d", __func__, status);
                goto out;
        }

        //status = command->response;
out:
        return status;
}

int fpc_tee_get_detect_count(fpc_tee_sensor_t* sensor, uint16_t *detect_count) {
        LOGD("%s", __func__);
        int status = 0;

        fpc_ta_fp_sensitivity_command_t* command =
                (fpc_ta_fp_sensitivity_command_t*) sensor->tee->shared_buffer->addr;

        status = sensor_command(sensor, FPC_TA_TEE_GET_DET_COUNT_CMD);

        if (status) {
                LOGE("%s, Failed to get detect counter, status code: %d", __func__, status);
                goto out;
        }
        *detect_count = command->val;
        //status = command->response;
out:
        return status;
}

int fpc_tee_set_detect_threshold(fpc_tee_sensor_t* sensor, uint16_t detect_threshold) {
        LOGD("%s", __func__);
        int status = 0;
        fpc_ta_fp_sensitivity_command_t* command =
                (fpc_ta_fp_sensitivity_command_t*) sensor->tee->shared_buffer->addr;

        command->val = detect_threshold;
        status = sensor_command(sensor, FPC_TA_TEE_SET_DET_THRESHOLD_CMD);

        if (status) {
                LOGE("%s, Failed to set detect threshold, status code: %d", __func__, status);
                goto out;
        }

        //status = command->response;
out:
        return status;
}

int fpc_tee_get_detect_threshold(fpc_tee_sensor_t* sensor, uint16_t *detect_threshold) {
        LOGD("%s", __func__);
        int status = 0;

        fpc_ta_fp_sensitivity_command_t* command =
                (fpc_ta_fp_sensitivity_command_t*) sensor->tee->shared_buffer->addr;

        status = sensor_command(sensor, FPC_TA_TEE_GET_DET_THRESHOLD_CMD);
        if (status) {
                LOGE("%s, Failed to get detect counter, status code: %d", __func__, status);
                goto out;
        }

        *detect_threshold = command->val;
       // status = command->response;
out:
        return status;
}

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_early_stop_ctrl(fpc_tee_sensor_t *sensor, uint8_t *ctrl)
{
    int status;

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_sensor_command_t* command = (fpc_ta_sensor_command_t*) tee->shared_buffer->addr;
    command->header.command = FPC_TA_SENSOR_EARLY_STOP_CTRL_CMD;
    command->header.target = TARGET_FPC_TA_SENSOR;
    command->early_stop_ctrl.ctrl = *ctrl;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        LOGE("%s, Failed to send command: EARLY_STOP_CTRL_CMD to TA, status code: %d",
                __func__, status);
        goto out;
    }

    *ctrl = command->early_stop_ctrl.ctrl;

out:
    return status;
}
#endif

int32_t fpc_tee_get_last_cac_result(fpc_tee_sensor_t *sensor)
{
    return sensor->cac_result;
}

int fpc_tee_deep_sleep(fpc_tee_sensor_t* sensor) {
    LOGD("%s", __func__);
    return sensor_command(sensor, FPC_TA_SENSOR_DEEP_SLEEP_CMD);
}


int fpc_tee_get_sensor_force_value(fpc_tee_sensor_t* sensor, uint8_t* value) {
    LOGD("%s", __func__);

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_sensor_command_t *command =
        (fpc_ta_sensor_command_t *) tee->shared_buffer->addr;
    command->header.command = FPC_TA_SENSOR_GET_FORCE_VALUE;
    command->header.target = TARGET_FPC_TA_SENSOR;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (status) {
        LOGE("%s, Failed to fetch sensor force value, status code: %d", __func__, status);
        goto out;
    }

    *value = command->get_force_value.value;

out:
    return status;
}

int fpc_tee_is_sensor_force_supported(fpc_tee_sensor_t *sensor, uint8_t *is_supported)
{
    LOGD("%s", __func__);

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_cmd_header_t *command = (fpc_ta_cmd_header_t *) tee->shared_buffer->addr;

    command->command = FPC_TA_SENSOR_IS_SENSOR_FORCE_SUPPORTED;
    command->target = TARGET_FPC_TA_SENSOR;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (FAILED(status)) {
        LOGE("%s, Failed to check sensor force support, status code: %d", __func__, status);
        goto out;
    }

    *is_supported = status;

out:
    return status;
}

#if FPC_CONFIG_FORCE_SENSOR == 1

int fpc_tee_wait_for_button_down_force(fpc_tee_sensor_t* sensor,
                                       uint32_t force_button_down_timeout_ms,
                                       uint8_t force_button_down_threshold)
{
    uint8_t force = 0;
    int force_result = 0;

    struct timespec start;
    struct timespec now;
    uint32_t delta_time_ms = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (force <= force_button_down_threshold &&
           delta_time_ms < force_button_down_timeout_ms) {
        force_result = fpc_tee_get_sensor_force_value(sensor, &force);
        if (force_result) {
            return force_result;
        }
        LOGD("%s force:%u th:%u\n", __func__, force, force_button_down_threshold);

        int status = sensor_command(sensor, FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD);
        if (status < 0) {
            return status;
        } else if (status > 0) {
            return FPC_STATUS_FINGER_LOST;
        }
        if (fpc_tee_sensor_cancelled(sensor))
        {
            return -FPC_ERROR_CANCELLED;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_time_ms = fpc_get_ms_diff(&start, &now);
    }
    if (delta_time_ms >= force_button_down_timeout_ms) {
        return -FPC_ERROR_TIMEDOUT;
    }
    return 0;
}

int fpc_tee_wait_for_button_up_force(fpc_tee_sensor_t* sensor,
                                     uint32_t force_button_up_timeout_ms,
                                     uint8_t force_button_up_threshold)
{
    uint8_t force = 255;
    int force_result = 0;

    struct timespec start;
    struct timespec now;
    uint32_t delta_time_ms = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (force >= force_button_up_threshold &&
           delta_time_ms < force_button_up_timeout_ms) {
        force_result = fpc_tee_get_sensor_force_value(sensor, &force);
        if (force_result) {
            return force_result;
        }
        LOGD("%s force:%u th:%u\n", __func__, force, force_button_up_threshold);
        if (fpc_tee_sensor_cancelled(sensor))
        {
            return -FPC_ERROR_CANCELLED;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_time_ms = fpc_get_ms_diff(&start, &now);
    }

    if (delta_time_ms >= force_button_up_timeout_ms) {
        return -FPC_ERROR_TIMEDOUT;
    }
    return 0;
}
#endif

/*********************************************************
 * Fetches sensor OTP information from TA.
 *
 * @param[IN]: sensor - handle containg the HAL/TA transfer buffer.
 * @param[OUT]: otp_info - Output data.
 *
 * @return: On Success returns 0.
 *          On Failure returns non-zero error code.
 *********************************************************/
int fpc_tee_get_sensor_otp_info(fpc_tee_sensor_t* sensor, fpc_hw_module_info_t* otp_info)
{
    LOGD("%s", __func__);
    int status = 0;

    fpc_ta_sensor_otp_info_t* command =
        (fpc_ta_sensor_otp_info_t*) sensor->tee->shared_buffer->addr;

    command->data = *otp_info;
    status = sensor_command(sensor, FPC_TA_SENSOR_GET_OTP_INFO_CMD);

    if (status) {
        LOGE("%s, Failed to fetch sensor otp data, status code: %d", __func__, status);
        goto out;
    }

    *otp_info = command->data;
out:
    return status;
}

int fpc_tee_get_mqt_limits(fpc_tee_sensor_t* sensor,
                           fpc_module_quality_test_limits_t* mqt_limits)
{
    LOGD("%s", __func__);
    int status = 0;

    fpc_ta_sensor_mqt_limits_t* command =
        (fpc_ta_sensor_mqt_limits_t*) sensor->tee->shared_buffer->addr;

    command->data = *mqt_limits;
    status = sensor_command(sensor, FPC_TA_SENSOR_GET_MQT_LIMITS_CMD);

    if (status) {
        LOGE("%s, Failed to get MQT Limits, status code: %d", __func__, status);
        goto out;
    }

    *mqt_limits = command->data;
out:
    return status;
}

fpc_tee_sensor_t* fpc_tee_sensor_init(fpc_tee_t* tee) {

    fpc_tee_sensor_t* sensor;
    uint8_t qr_code[32];
    waiting_finger_state = 0;
    if (!tee) {
        return NULL;
    }

    sensor = malloc(sizeof(fpc_tee_sensor_t));
    if (!sensor) {
        return NULL;
    }

    memset(sensor, 0, sizeof(*sensor));
    sensor->tee = tee;
    pthread_mutex_init(&sensor->mutex, NULL);

    if (sensor_command(sensor, FPC_TA_SENSOR_DEVICE_INIT_CMD)) {
        goto err;
    }

    sensor->irq = fpc_irq_init();
    if (!sensor->irq) {
        goto err;
    }
    sensor->alive_check = 0;

#if defined(FPC_CONFIG_NORMAL_SPI_RESET) || defined(FPC_CONFIG_NORMAL_SENSOR_RESET)
    sensor->reset = fpc_reset_init();
    if (!sensor->reset) {
        goto err;
    }
#endif
    if (0 != fpc_tee_deep_sleep(sensor)) {
        goto err;
    }

    fpc_tee_get_sn(sensor, qr_code);
    LOGE("%s,  qr_code %s", __func__, qr_code);

    property_set(PROPERTY_FINGERPRINT_QRCODE_VALUE, (const char *)qr_code);
    property_set(PROPERTY_FINGERPRINT_QRCODE, "1");
    alive_check(sensor);
    return sensor;

err:
    pthread_mutex_destroy(&sensor->mutex);
    if (sensor->irq) {
        fpc_irq_release(sensor->irq);
    }
#if defined(FPC_CONFIG_NORMAL_SPI_RESET) || defined(FPC_CONFIG_NORMAL_SENSOR_RESET)
    if (sensor->reset) {
        fpc_reset_release(sensor->reset);
    }
#endif
    free(sensor);
    return NULL;
}

void fpc_tee_sensor_release(fpc_tee_sensor_t* sensor)
{
    if (!sensor) {
        return;
    }

    fpc_tee_deep_sleep(sensor);

    pthread_mutex_destroy(&sensor->mutex);
    fpc_irq_release(sensor->irq);
#if defined(FPC_CONFIG_NORMAL_SPI_RESET) || defined(FPC_CONFIG_NORMAL_SENSOR_RESET)
    fpc_reset_release(sensor->reset);
#endif
     free(sensor);
}

int fpc_tee_set_cancel(fpc_tee_sensor_t* sensor)
{
    if (!sensor) {
        return -FPC_ERROR_PARAMETER;
    }

    LOGD("%s", __func__);
    pthread_mutex_lock(&sensor->mutex);
    sensor->cancelled = 1;
    int status = fpc_irq_set_cancel(sensor->irq);
    pthread_mutex_unlock(&sensor->mutex);
    return status;
}

int fpc_tee_clear_cancel(fpc_tee_sensor_t* sensor)
{
    if (!sensor) {
        return -FPC_ERROR_PARAMETER;
    }

    LOGD("%s", __func__);
    pthread_mutex_lock(&sensor->mutex);
    sensor->cancelled = 0;
    int status = fpc_irq_clear_cancel(sensor->irq);
    pthread_mutex_unlock(&sensor->mutex);
    return status;
}

#define HWCODE_LEN 7

static int convert_otp_to_sn(uint8_t* otp_data, uint8_t * sn)
{
    char buf[18];
    int ret, index;

    sn[0] = otp_data[0];

    ret = sprintf(buf, "%d", otp_data[1] << 16 | otp_data[2] << 8 | otp_data[3]);
    if (ret != HWCODE_LEN) {
        LOGE("%s, Error HWCode len : %d", __func__, ret);
        return -1;
    } else {
        memcpy(sn+1, buf, HWCODE_LEN);
    }

    ret = sprintf(buf, "%X", 0xF & (otp_data[4] >> 4));
    if (ret != 1) {
        LOGE("%s, Error Year len : %d", __func__, ret);
        return -1;
    } else {
        memcpy(sn+8, buf, 1);
    }

    for (index = 0; index < 8; index++) {
        sn[index+9] = (otp_data[index+4]&0xf) << 4 | (otp_data[index+5] & 0xf0) >> 4;
    }

    ret = sprintf(buf, "%x", otp_data[12] & 0xf);
    if (ret != 1) {
        LOGE("%s, Error Line len : %d", __func__, ret);
        return -1;
    } else {
        memcpy(sn+17, buf, 1);
    }
    LOGE("%s, sn: %s", __func__, sn);
    return 0;
}

int fpc_tee_get_sn(fpc_tee_sensor_t* sensor, uint8_t *sn)
{
    int status = FPC_ERROR_NONE;
    fpc_hw_module_info_t otp_info;
    uint8_t otp_data[13];
    uint8_t otp_sn[18];
    status = fpc_tee_get_sensor_otp_info(sensor, &otp_info);
    if (status) {
        LOGE("%s, Failed to fetch sn, status code: %d", __func__, status);
        return status;
    }

    if (otp_info.vendor_otp_info.valid_field != 0) {
        memcpy(otp_data, otp_info.vendor_otp_info.vendor_data, 13);
        LOGE("%s, otpdata %s", __func__, otp_data);
        status = convert_otp_to_sn(otp_data, otp_sn);
        LOGE("%s,  otp_sn %s", __func__, otp_sn);
        memcpy(sn, otp_sn, sizeof(otp_sn));
        if (status) {
            LOGE("%s, conert vendor_data failed", __func__);
        }
    }
    else {
        LOGE("%s, No vendor_data in OTP", __func__);
        return FPC_ERROR_OTP;
    }
    return status;
}
