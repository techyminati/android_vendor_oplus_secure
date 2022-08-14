#ifndef __PLAT_MEM_H__
#define __PLAT_MEM_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

void mem_move(void* dest, void* src, uint32_t size);
void mem_set(void* dest, uint32_t data, uint32_t size);
int mem_compare(void* buff1, void* buff2, uint32_t size);

#include <stddef.h>
#ifdef EGIS_DEBUG_MEMORY
void* sys_alloc(uint32_t count, uint32_t size, const char* file_name, int line);
#else
void* sys_alloc(size_t count, size_t size);
#endif
void* sys_realloc(void* data, size_t size);
void sys_free(void* data);
void sys_memory_init(void);

#define NAME_LENGTH 1024
#ifdef EGIS_DEBUG_MEMORY
#define plat_alloc(fmt) sys_alloc(0, fmt, __FUNCTION__, __LINE__)
#else
#define plat_alloc(fmt) sys_alloc(1, fmt)
#endif
#define plat_realloc(fmt, size) sys_realloc(fmt, size)
#define plat_free(x) sys_free(x)
#define PLAT_FREE(x)          \
	if (x != NULL) {      \
		plat_free(x); \
		x = NULL;     \
	}

#ifdef __cplusplus
}
#endif

#endif
