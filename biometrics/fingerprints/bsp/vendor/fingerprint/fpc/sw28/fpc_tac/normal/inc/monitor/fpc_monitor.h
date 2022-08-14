/************************************************************************************
 ** File: - MSM8953_LA_1_0\android\vendor\qcom\proprietary\securemsm\fpc\fpc_tac\normal\src\monitor\fpc_monitor.h
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
 **  <author>     <data>            <desc>
 **  Haitao.Zhou  2016/09/19        NULL
 **  Haitao.Zhou  2016/10/11        add for tp protect monitor
 ************************************************************************************/
#ifndef _FPC_MONITOR_H_
#define _FPC_MONITOR_H_

#include "fpc_tee_sensor.h"

typedef enum monitor_type {
    POWER_MONITOR = 0,
    ERROR_STATE = 1,
    TP_PROTECT_MONITOR = 2,
} monitor_type_t;

//power monitor
typedef struct monitor_power {
    bool flag_trigger;
    uint64_t time_start;
    uint64_t time_end;
    double battery;
} monitor_power_t;

//tp protect monitor
typedef struct monitor_tp_protect {
    fingerprint_mode_t mode;
} monitor_tp_protect_t;

//fpc_monitor
typedef struct fpc_monitor {
    monitor_power_t power;
    monitor_tp_protect_t tp_protect;
} fpc_monitor_t;

//for monitor
extern fpc_monitor_t fp_monitor;

//Power monitor API
int monitor_power_get_touchdown_time(monitor_power_t* power_monitor);
int monitor_power_get_touchup_time(monitor_power_t* power_monitor);
int monitor_power_set_trigger_flag(monitor_power_t* power_monitor);
int monitor_power_state_clear(monitor_power_t* power_monitor);

//TP protect API
int monitor_tp_protect_set_mode(monitor_tp_protect_t* tp_protect_monitor, fingerprint_mode_t mode);

//trigger API
int monitor_trigger(fpc_tee_sensor_t** sensor, fpc_monitor_t* monitor, monitor_type_t type);

#endif
