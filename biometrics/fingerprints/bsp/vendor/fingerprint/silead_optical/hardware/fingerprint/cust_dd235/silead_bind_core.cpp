/************************************************************************************
 ** File: - silead_bind_core.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      silead bind big core
 **
 ** Version: 1.0
 ** Date created: 23:03:11,02/12/2019
 ** Author: Bangxiong.Wu@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>        <data>            <desc>
 **  Bangxiong.Wu    2019/02/12        add for silead bind big core
 **  Bangxiong.Wu    2019/03/19        correct big core setting(4~7 -> 6~7)
 ************************************************************************************/
#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE

extern "C" {
#include "silead_bind_core.h"
}

void sl_bind_big_core(void) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(6, &mask);
    CPU_SET(7, &mask);
    sched_setaffinity(getpid(), sizeof(cpu_set_t), (cpu_set_t *)&mask);
}

#endif
