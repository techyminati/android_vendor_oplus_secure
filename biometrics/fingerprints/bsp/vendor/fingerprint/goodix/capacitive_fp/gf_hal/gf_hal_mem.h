/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _GF_HAL_MEM_H_
#define _GF_HAL_MEM_H_

#ifndef __SUPPORT_GF_MEMMGR
#define GF_MEM_MALLOC(size)  malloc(size)
#define GF_MEM_REALLOC(src, size)  realloc(src, size)
#define GF_MEM_FREE(addr)  free((void *)(addr))
#else  // __SUPPORT_GF_MEMMGR
#include "gf_memmgr.h"

#define GF_MEM_MALLOC(size)    \
        gf_hal_malloc((int32_t)(size), (const int8_t *)__FILE__, (const int8_t *)__FUNCTION__, __LINE__)
#define GF_MEM_REALLOC(src, size)    \
        gf_hal_realloc((void *)(src), (int32_t)(size), (const int8_t *)__FILE__,    \
                (const int8_t *)__FUNCTION__, __LINE__)
#define GF_MEM_FREE(addr)    \
        gf_hal_free((void *)(addr))

#define GF_HAL_START_MEMORY_CHECK()    \
    do    \
    {    \
        gf_memmgr_config_t config;    \
        config.debug_enable = 1;    \
        config.memmgr_enable = 1;    \
        config.dump_time_enable = 1;    \
        config.erase_mem_pool_when_free = 1;    \
        config.match_best_mem_pool_enable = 1;    \
        config.memmgr_pool_size = 10*1024*1024;    \
        gf_memmgr_save_config(&config);    \
        gf_memmgr_create_entry_point();   \
    }    \
    while (0)

#define GF_HAL_CLOSE_MEMORY_CHECK()  \
    gf_memmgr_destroy_entry_point()

#ifdef __cplusplus
extern "C"
{
#endif  // #ifdef __cplusplus

/**
 * \brief Allocates memory blocks.
 *
 * \param size Bytes to allocate.
 *
 * \param pFilename The filename of the function call.
 *
 * \param pFuncName The function name of the function call.
 *
 * \parma line The line number of the function call.
 *
 * \return A void pointer to the allocated space, or NULL if there is insufficient memory available.
 */
void *gf_hal_malloc(int32_t size, const int8_t *pFilename, const int8_t *pFuncName, int32_t line);

/**
 * \brief Reallocates memory blocks.
 *
 * \param pSrc Pointer to previously allocated memory block.
 *
 * \param size Bytes to allocate.
 *
 * \param pFuncName The function name of the function call.
 *
 * \param pFilename The filename of the function call.
 *
 * \parma line The line number of the function call.
 *
 * \return A void pointer to the reallocated(and possibly moved) memory block.
 *      If there is not enough available memory to expand the block to the given size,
 *      the original block is left unchanged. and NULL is returned. If size is zero, then the block
 *      pointed to by pSrc is freed; the return value is NULL. and memblock is left pointing at a freed block.
 */
void *gf_hal_realloc(void *pSrc, int32_t size, const int8_t *pFilename, const int8_t *pFuncName,
        int32_t line);

/**
 * \brief Deallocates or frees a memory block.
 *
 * \param Previously allocated memory block to be freed.
 */
void gf_hal_free(void *pMemAddr);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
#endif  // __SUPPORT_GF_MEMMGR
#endif  // _GF_HAL_MEM_H_ //
