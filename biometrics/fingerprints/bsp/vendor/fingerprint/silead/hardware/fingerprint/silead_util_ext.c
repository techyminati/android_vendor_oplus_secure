/******************************************************************************
 * @file   silead_util_ext.c
 * @brief  Contains fingerprint utilities functions file.
 *
 *
 * Copyright (c) 2016-2019 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * calvin wang  2018/1/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_util_ext"
#include "log/logmsg.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "silead_util_ext.h"

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 256
#endif

#ifdef HOST_OS_WINDOWS

int32_t silfp_util_make_dir(const char *path)
{
    return mkdir(path);
}

int32_t silfp_util_dir_get_type(char *path, struct dirent *pEntry)
{
    struct stat st;
    char dir_path[MAX_PATH_LEN] = {0};

    snprintf(dir_path, sizeof(dir_path), "%s/%s", path, pEntry->d_name);
    if (stat(dir_path, &st) < 0) {
        LOG_MSG_ERROR("getsize %s failed (%d:%s)", path, errno, strerror(errno));
        return 0;
    }

    if (S_ISDIR(st.st_mode)) {
        return DT_DIR;
    }

    if (S_ISREG(st.st_mode)) {
        return DT_REG;
    }

    return 0;
}

#else /* !HOST_OS_WINDOWS*/

int32_t silfp_util_make_dir(const char *path)
{
    return mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

int32_t silfp_util_dir_get_type(char *path, struct dirent *pEntry)
{
    (void)path;
    return pEntry->d_type;
}

#endif /* HOST_OS_WINDOWS */