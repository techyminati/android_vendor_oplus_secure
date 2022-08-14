/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <errno.h>
#include <android/log.h>
#include <sys/ptrace.h>
#include "gf_ca_entry.h"
#include "gf_type_define.h"
#include "gf_common.h"
#include "gf_hal_log.h"

#include "gf_sec_ptrace.h"

#define LOG_TAG "[GF_HAL][gf_sec_ptrace]"

/**
 * Function: gf_sec_ptrace_self
 * Description: Enable ptrace in child process.
 * Input: None
 * Output: None
 * Return: void
 */
void gf_sec_ptrace_self()
{
    VOID_FUNC_ENTER();

    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
    {
        LOG_E(LOG_TAG, "[%s] Couldn't PTRACE_TRACEME errno = %d\n", __func__, errno);
    }
    VOID_FUNC_EXIT();
    return;
}


