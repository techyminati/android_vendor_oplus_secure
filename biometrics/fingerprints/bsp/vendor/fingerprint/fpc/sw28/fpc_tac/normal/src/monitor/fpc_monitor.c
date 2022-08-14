/************************************************************************************
 ** File: - MSM8953_LA_1_0\android\vendor\qcom\proprietary\securemsm\fpc\fpc_tac\normal\src\monitor\fpc_monitor.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2016, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint monitor
 **
 ** Version: 1.0
 ** Date created: 2016/09/19, 14:31:11
 ** Author: Haitao.Zhou@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>        <data>            <desc>
 **  Haitao.Zhou     2016/09/19        NULL
 **  Haitao.Zhou     2016/10/11        add for tp protect monitor
 **  Haitao.Zhou     2016/10/18        fix the data clac bug
 **  Hongdao.yu      2017/06/28        add 1263 power monitor
 ************************************************************************************/

#include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "fpc_log.h"
#include "fpc_monitor.h"
#include "fpc_tee.h"
#include "fpc_tee_hal.h"
#include "fpc_types.h"
#include "fingerprint_type.h"

#define POWER_FIRST_STEP           (120 - 4)   //first step 120mA - 4mA
#define TIME_POWER_FIRST_STEP      (2 * 1000)  //2s
#define POWER_SEC_STEP             (108 - 4)   //second step 108mA - 4mA
#define TIME_POWER_SEC_STEP        (9 * 1000)  //9s
#define POWER_EDN_STEP             (49 - 4)    //end step 49mA - 4mA
#define MS_TO_SEC                  (1.000000 / (1000 * 3600))

#define POWER_FST_STEP_1263        (190 - 4)   //mA, power changes during 1.3s when finger down
#define TIME_POWER_FST_STEP_1263   1300        //ms
#define POWER_SEC_STEP_1263        (64 - 4)    //mA, second step if down always
#define TIME_POWER_SEC_STEP_1263   (10 * 1000) //10s
#define ENERGY_FINGER_UP_1263      0.025       //mAh, finger up energy cosumed when host sleep


fpc_monitor_t fp_monitor;

static uint64_t current_ms_time(void)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);

    return (t1.tv_sec * 1000) + (t1.tv_usec / 1000);
}

//Power monitor
int monitor_power_get_touchdown_time(monitor_power_t* power_monitor)
{
    if (power_monitor == NULL) {
        LOGE("%s power_monitor is NULL", __func__);
        return -1;
    }

    power_monitor->time_start = current_ms_time();

    return 0;
}

int monitor_power_get_touchup_time(monitor_power_t* power_monitor)
{
    if (power_monitor == NULL) {
        LOGE("%s power_monitor is NULL", __func__);
        return -1;
    }

    power_monitor->time_end = current_ms_time();

    return 0;
}

int monitor_power_set_trigger_flag(monitor_power_t* power_monitor)
{
    if (power_monitor == NULL) {
        LOGE("%s power_monitor is NULL", __func__);
        return -1;
    }

    power_monitor->flag_trigger = true;

    return 0;
}

int monitor_power_state_clear(monitor_power_t* power_monitor)
{
    if (power_monitor == NULL) {
        LOGE("%s power_monitor is NULL", __func__);
        return -1;
    }

    power_monitor->flag_trigger = false;
    power_monitor->time_start = 0;
    power_monitor->time_end = 0;
    power_monitor->battery = 0;

    return 0;
}

static int monitor_power_calc(monitor_power_t* power_monitor)
{
    double tmp_ms;

    if (power_monitor == NULL) {
        LOGE("%s power_monitor is NULL", __func__);
        return -1;
    }

    tmp_ms = power_monitor->time_end - power_monitor->time_start;
    if(!fp_config_info_init.fp_up_irq){
        LOGD("%s fingerprint a_fp touch take time %lf ms.", __func__, tmp_ms);
        if (tmp_ms <= TIME_POWER_FIRST_STEP) {
                power_monitor->battery = (POWER_FIRST_STEP * tmp_ms) * MS_TO_SEC;
                LOGD("%s a_fp take battery %lf, type 1.", __func__, power_monitor->battery);
        } else if (tmp_ms > TIME_POWER_FIRST_STEP && tmp_ms <= (TIME_POWER_SEC_STEP + TIME_POWER_FIRST_STEP)) {
                power_monitor->battery = (POWER_FIRST_STEP * TIME_POWER_FIRST_STEP +
                        (tmp_ms - TIME_POWER_FIRST_STEP) * POWER_SEC_STEP) * MS_TO_SEC;
                LOGD("%s a_fp take battery %lf, type 2.", __func__, power_monitor->battery);
        } else if (tmp_ms > (TIME_POWER_SEC_STEP + TIME_POWER_FIRST_STEP)) {
                power_monitor->battery = (POWER_FIRST_STEP * TIME_POWER_FIRST_STEP +
                        POWER_SEC_STEP * TIME_POWER_SEC_STEP +
                        (tmp_ms - TIME_POWER_FIRST_STEP - TIME_POWER_SEC_STEP) * POWER_EDN_STEP) * MS_TO_SEC;
                LOGD("%s a_fp take battery %lf, type 3.", __func__, power_monitor->battery);
        }
    }else{
        LOGD("%s fingerprint b_fp touch take time %lf ms.", __func__, tmp_ms);
        if (tmp_ms <= TIME_POWER_FST_STEP_1263) {
                power_monitor->battery = (POWER_FST_STEP_1263 * tmp_ms) * MS_TO_SEC;
                LOGD("%s b_fp take battery %lf, type 1.", __func__, power_monitor->battery);
        } else if (tmp_ms > TIME_POWER_FST_STEP_1263 && tmp_ms <= (TIME_POWER_SEC_STEP_1263 + TIME_POWER_FST_STEP_1263)) {
                power_monitor->battery = (POWER_FST_STEP_1263 * TIME_POWER_FST_STEP_1263 +
                    (tmp_ms - TIME_POWER_FST_STEP_1263) * POWER_SEC_STEP_1263) * MS_TO_SEC;
                LOGD("%s b_fp take battery %lf, type 2.", __func__, power_monitor->battery);
        } else if (tmp_ms > (TIME_POWER_SEC_STEP_1263 + TIME_POWER_FST_STEP_1263)) {
                power_monitor->battery = (POWER_FST_STEP_1263 * TIME_POWER_FST_STEP_1263 +
                    POWER_SEC_STEP_1263 * TIME_POWER_SEC_STEP_1263) * MS_TO_SEC + ENERGY_FINGER_UP_1263;
                LOGD("%s b_fp take battery %lf, type 3.", __func__, power_monitor->battery);
    	}
    }
    return 0;
}

static int monitor_power_notify(fpc_tee_sensor_t** sensor, monitor_power_t* power_monitor)
{
    fingerprint_msg_t message;

    if (sensor == NULL || *sensor == NULL || power_monitor == NULL) {
        LOGE("%s params invaild", __func__);
        return -1;
    }

    //fpc_hal_common_t* dev = p_fpc_hal_common(sensor);

    //dev->callback->on_monitor(dev->callback_context, FINGER_POWER_MONITOR, power_monitor->battery);
    LOGD("%s send send fingerprint battery %lf message", __func__, message.data.monitor.data.power.battery);

    return 0;
}

//TP protect monitor
static int monitor_tp_protect_notify(fpc_tee_sensor_t** sensor, fingerprint_mode_t mode)
{
    fingerprint_msg_t message;
    (void) mode; //unused
    if (sensor == NULL || *sensor == NULL) {
        LOGE("%s params invaild", __func__);
        return -1;
    }

    //fpc_hal_common_t* dev = p_fpc_hal_common(sensor);
    //dev->callback->on_monitor(dev->callback_context, FINGER_TP_PROTECT_MONITOR, (double)mode);
    LOGD("%s send send fingerprint battery %lf message", __func__, message.data.monitor.data.power.battery);

    return 0;
}

int monitor_tp_protect_set_mode(monitor_tp_protect_t* tp_protect_monitor, fingerprint_mode_t mode)
{
    if (tp_protect_monitor == NULL) {
        LOGE("%s params invaild", __func__);
        return -1;
    }

    tp_protect_monitor->mode = mode;

    return 0;
}

//monitor trigger
int monitor_trigger(fpc_tee_sensor_t** sensor, fpc_monitor_t* monitor, monitor_type_t type)
{
    if (sensor == NULL || *sensor == NULL || monitor == NULL) {
        LOGE("%s params invaild", __func__);
        return -1;
    }

    switch (type) {
        case POWER_MONITOR:
            if (monitor->power.time_start == 0) {
                LOGD("%s before there is no touch down", __func__);
                monitor_power_state_clear(&monitor->power);
            } else if (monitor->power.flag_trigger == false) {
                LOGD("%s no need calc power", __func__);
                monitor_power_state_clear(&monitor->power);
            } else {
                monitor_power_calc(&monitor->power);
                monitor_power_notify(sensor, &monitor->power);
                monitor_power_state_clear(&monitor->power);
            }
            break;
        case ERROR_STATE:
            break;
        case TP_PROTECT_MONITOR:
            monitor_tp_protect_notify(sensor, monitor->tp_protect.mode);
            break;
        default:
            break;
    }

    return 0;
}

