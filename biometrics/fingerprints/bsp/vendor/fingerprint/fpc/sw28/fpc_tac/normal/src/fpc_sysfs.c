/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fpc_sysfs.h"
#include "fpc_log.h"
#include "fpc_types.h"

int fpc_sysfs_path_by_attr(const char *attr, const char *attr_val,
                                 const char *base, char *path, int path_max)
{
    char aval[32];
    int rc;
    int found = 0;
    int fd;
    DIR *dir;
    struct dirent *item;

    dir = opendir(base);
    if (!dir) {
        LOGE("Unable to open '%s'", base);
        return 0;
    }
    while (NULL != (item = readdir(dir))) {
        if (item->d_type != DT_DIR && item->d_type != DT_LNK)
            continue;
        if (item->d_name[0] == '.')
            continue;
        rc = snprintf(path, path_max, "%s/%s/%s",
                          base, item->d_name, attr);
        if (rc >= path_max) {
            LOGD("Entry name truncated '%s'", path);
            continue;
        }
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            LOGE("Unable to open '%s'", path);
            continue;
        }
        rc = read(fd, aval, sizeof(aval) - 1);
        close(fd);
        if (rc < 0) {
            LOGE("Unable to read '%s'", path);
            continue;
        }
        aval[rc] = 0;
        found = (strstr(aval, attr_val) != NULL);
        if (found) {
            path[strlen(path) - strlen(attr)] = 0;
            LOGD("'%s'='%s' found on  path '%s'",
                          attr, aval, path);
            break;
        }
    }
    closedir(dir);
    return found;
}

int fpc_sysfs_node_write(int base_fd, const char* name, const char* value)
{
    int status = 0;

    int fd = openat(base_fd, name, O_WRONLY);
    if (fd == -1) {
        LOGE("%s openat: %s failed, %s", __func__, name, strerror(errno));
        status = -FPC_ERROR_IO;
        goto out;
    }

    int size = strlen(value);

    if (write(fd, value, size) != size) {
        LOGE("%s write failed %i", __func__, errno);
        status = -FPC_ERROR_IO;
        goto out;
    }

out:
    if (fd != -1) {
        close(fd);
    }

    return status;
}
