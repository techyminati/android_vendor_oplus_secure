
#ifndef ETS_HAL_HEADER
#define ETS_HAL_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "plat_log.h"

#define LOG_TAG "egis_fp_hal"


#define EGIS_TP_ENABLE  1
#define EGIS_TP_DISABLE  0
#define FP_ENABLE_TP_PATH "/proc/touchpanel/fp_enable"


int egis_tp_enable();
int egis_tp_disable();
#endif