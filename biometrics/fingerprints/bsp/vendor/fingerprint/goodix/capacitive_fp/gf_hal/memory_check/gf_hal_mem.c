/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_hal memory handle file
 * History:
 * Version: 1.0
 */
#include <stdlib.h>
#include <pthread.h>
#include "gf_type_define.h"
#include "gf_hal_mem.h"

#define LOG_TAG "gf_hal_memory"

#ifdef __SUPPORT_GF_MEMMGR
pthread_mutex_t g_mem_mutex = PTHREAD_MUTEX_INITIALIZER;  // hal mutex
#endif  // #ifdef __SUPPORT_GF_MEMMGR

/**
 * Function: gf_hal_malloc
 * Description: malloc new memory for gf_hal
 * Input: size, file_name, func_name, line
 * Output: none
 * Return: start address of new memory
 */
void *gf_hal_malloc(int32_t size, const int8_t *file_name,
                 const int8_t *func_name, int32_t line)
{
    void *addr = NULL;
    UNUSED_VAR(file_name);
    UNUSED_VAR(line);
    UNUSED_VAR(func_name);

#ifdef __SUPPORT_GF_MEMMGR

    pthread_mutex_lock(&g_mem_mutex);

    if (g_mem_config.memmgr_enable > 0)
    {
        addr = gf_memmgr_malloc(size, 0, file_name, func_name, line);
    }
    else
    {
        addr = malloc(size);
    }

    pthread_mutex_unlock(&g_mem_mutex);
#else  // #ifdef __SUPPORT_GF_MEMMGR
    addr =  malloc(size);
#endif  // #ifdef __SUPPORT_GF_MEMMGR
    return addr;
}

/**
 * Function: gf_hal_realloc
 * Description: malloc new memory for gf_hal, memory size may be changed
 * Input: src, size, file_name, func_name, line
 * Output: none
 * Return: start address of new memory
 */
void *gf_hal_realloc(void *src, int32_t size, const int8_t *file_name,
                  const int8_t *func_name,
                  int32_t line)
{
    void *addr = NULL;
    UNUSED_VAR(file_name);
    UNUSED_VAR(line);
    UNUSED_VAR(func_name);

#ifdef __SUPPORT_GF_MEMMGR
    pthread_mutex_lock(&g_mem_mutex);

    if (g_mem_config.memmgr_enable > 0)
    {
        addr = gf_memmgr_realloc(src, size, file_name, func_name, line);
    }
    else
    {
        addr = realloc(src, size);
    }

    pthread_mutex_unlock(&g_mem_mutex);
#else  // #ifdef __SUPPORT_GF_MEMMGR
    addr = realloc(src, size);
#endif  // #ifdef __SUPPORT_GF_MEMMGR
    return addr;
}

/**
 * Function: gf_hal_free
 * Description: free memory
 * Input: mem_addr
 * Output: none
 * Return: none
 */
void gf_hal_free(void *mem_addr)
{
    // Note: both gf_memmgr_free and gf_free check the validity of input pointer themselves,
    // so we do not need to check it again here.
    if (NULL == mem_addr)
    {
        return;
    }

#ifdef __SUPPORT_GF_MEMMGR
    pthread_mutex_lock(&g_mem_mutex);

    if (g_mem_config.memmgr_enable > 0)
    {
        gf_memmgr_free(mem_addr);
    }
    else
    {
        free(mem_addr);
    }

    pthread_mutex_unlock(&g_mem_mutex);
#else  // #ifdef __SUPPORT_GF_MEMMGR
    free(mem_addr);
#endif  // #ifdef __SUPPORT_GF_MEMMGR
}
