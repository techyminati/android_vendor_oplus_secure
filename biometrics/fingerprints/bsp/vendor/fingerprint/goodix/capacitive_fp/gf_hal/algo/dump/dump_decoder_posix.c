/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description: posix make directory support
 * History:
 */

#if (defined __android__) || (defined __linux__)
#define _GNU_SOURCE 1

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif  // #if (defined __android__) || (defined __linux__)

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "dump_decoder_posix.h"

#if (defined __android__) || (defined __linux__)
/**
 *Function: mkpathat
 *Description: make path at specified fd
 *Input: atfd, make path at atfd
 *       path, path name to be made
 *       mode, path dir
 *Output:none
 *Return: 0 success, -1 failed
 */
static int32_t mkpathat(int32_t atfd, const char* path, mode_t mode)
{
    struct stat buf;
    int32_t len = 0;
    char* path_buf = NULL;
    char* s = NULL;

    if (!fstatat(atfd, path, &buf, 0) && !S_ISDIR(buf.st_mode))
    {
        errno = EEXIST;
        LOG_D(LOG_TAG, "Existing not a directory: %s", path);
        return -1;
    }

    len = strlen(path)+1;
    path_buf = (char*)malloc(len);
    if (!path_buf)
    {
        LOG_D(LOG_TAG, "malloc failed!");
        return -1;
    }

    memcpy(path_buf, path, len);
    s = path_buf;

    for (;; s++)
    {
        char save = 0;
        if (*s == '/' && s != path_buf)
        {
            save = *s;
            *s = 0;
        }
        else if (*s)
        {
            continue;
        }

        if (mkdirat(atfd, path_buf, mode))
        {
            if (errno != EEXIST)
            {
                LOG_D(LOG_TAG, "mkdir %s err: %s", path_buf, strerror(errno));
                free(path_buf);
                return -errno;
            }
        }
        else
        {
            LOG_D(LOG_TAG, "Created dir ok: %s", path_buf);
        }

        if (!(*s = save))
        {
            break;
        }
    }  // for (;; s++)

    free(path_buf);
    return 0;
}

/**
 *Function: mkdirs_relative
 *Description: make directories at specified path recursively
 *Input: path, path name to be made
 *       mode, path dir
 *       root_path, root directory
 *Output:none
 *Return: 0 success, -1 failed
 */
int32_t mkdirs_relative(const char* path, mode_t mode, const char* root_path)
{
    char old_path[256];
    int32_t atfd = -1;
    int32_t ret = 0;

    if (!path || !root_path)
    {
        LOG_D(LOG_TAG, "err input, path is NULL");
        return -1;
    }

    // need x permission
    if (getcwd(old_path, sizeof(old_path)) == NULL)
    {
        LOG_D(LOG_TAG, "getcwd err: %s", strerror(errno));
        return -1;
    }
    // LOG_D(LOG_TAG, "old path: %s", old_path);

    if (chdir(root_path) < 0)
    {
        LOG_D(LOG_TAG, "chdir %s err: %s", root_path, strerror(errno));
        return -1;
    }

    atfd = open("." , O_RDONLY);
    if (atfd < 0)
    {
        LOG_D(LOG_TAG, "open %s err: %s", root_path, strerror(errno));
        return -1;
    }

    ret = mkpathat(atfd, path, mode);
    close(atfd);

    // fixed bugs: root_path is ./data
    if (chdir(old_path) < 0)
    {
        LOG_D(LOG_TAG, "chdir %s err: %s", old_path, strerror(errno));
        return -1;
    }

    return ret;
}

/**
 *Function: mkdirs
 *Description: make dirs recursively at current dir
 *Input: path, path name to be made
 *       mode, path dir
 *Output:none
 *Return: 0 success, -1 failed
 */
int32_t mkdirs(const char* path, mode_t mode)
{
    int32_t atfd = -1;
    int32_t ret = 0;

    if (!path)
    {
        LOG_D(LOG_TAG, "err input, path is NULL");
        return -1;
    }

    atfd = open("." , O_RDONLY);
    if (atfd < 0)
    {
        LOG_D(LOG_TAG, "open %s err: %s", path, strerror(errno));
        return -1;
    }

    ret = mkpathat(atfd, path, mode);
    close(atfd);

    return ret;
}

// windows or other OS
#else  // #if (defined __android__) || (defined __linux__)
#include <io.h>

/**
 *Function: mkpaths
 *Description: make paths recursively according to given path
 *Input: path, path name to be made
 *Output:none
 *Return: 0 success, -1 failed
 */
int32_t mkpaths(const char* path)
{
    char* s = NULL;
    int32_t len = strlen(path) + 1;
    char* path_buf = (char*)malloc(len);

    if (!path_buf)
    {
        LOG_D(LOG_TAG, "malloc failed!");
        return -1;
    }

    memcpy(path_buf, path, len);

    s = path_buf;
    for (;; s++)
    {
        char save = 0;
        if (*s == '/' && s != path_buf)
        {
            save = *s;
            *s = 0;
        }
        else if (*s)
        {
            continue;
        }

        if (_mkdir(path_buf))
        {
            if (errno != EEXIST)
            {
                LOG_D(LOG_TAG, "mkdir %s err: %s", path_buf, strerror(errno));
                free(path_buf);
                return -errno;
            }
        }
        else
        {
            LOG_D(LOG_TAG, "Created dir ok: %s", path_buf);
        }

        if (!(*s = save))
        {
            break;
        }
    }  // for (;; s++)

    free(path_buf);
    return 0;
}

/**
 *Function: mkdirs
 *Description:
 *Input: path, path name to be made
 *       mode, path mode
 *Output:none
 *Return: 0 success, -1 failed
 */
int32_t mkdirs(const char* path, mode_t mode)
{
    return -1;
}

/**
 *Function: mkdirs_relative
 *Description: make directories at specified path recursively
 *Input: path, path name to be made
 *       mode, path dir
 *       root_path, root directory
 *Output:none
 *Return: 0 success, -1 failed
 */
int32_t mkdirs_relative(const char* path, mode_t mode, const char* root_path)
{
    char old_path[256];
    int32_t ret = 0;

    if (!path || !root_path)
    {
        LOG_D(LOG_TAG, "err input, path is NULL");
        return -1;
    }

    // need x permission
    if (_getcwd(old_path, sizeof(old_path)) == NULL)
    {
        LOG_D(LOG_TAG, "getcwd err: %s", strerror(errno));
        return -1;
    }
    // LOG_D(LOG_TAG, "old path: %s", old_path);

    if (_chdir(root_path) < 0)
    {
        LOG_D(LOG_TAG, "chdir %s err: %s", root_path, strerror(errno));
        return -1;
    }

    ret = mkpaths(path);

    // fixed bugs: root_path is ./data
    if (_chdir(old_path) < 0)
    {
        LOG_D(LOG_TAG, "chdir %s err: %s", old_path, strerror(errno));
        return -1;
    }

    return ret;
}
#endif  // #if (defined __android__) || (defined __linux__)
