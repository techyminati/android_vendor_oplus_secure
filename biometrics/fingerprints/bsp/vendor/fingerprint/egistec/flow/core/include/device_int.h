#ifndef __DEVICE_INT_H__
#define __DEVICE_INT_H__

#include "type_def.h"

BOOL wait_trigger(int try_count,int timeout);

// implementation is on captain.c
BOOL check_cancelable();
BOOL check_need_pause(void);

#endif
