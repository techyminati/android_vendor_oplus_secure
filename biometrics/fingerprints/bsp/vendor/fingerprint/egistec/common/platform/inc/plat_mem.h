#ifndef __PLAT_MEM_H__
#define __PLAT_MEM_H__

#include <stdint.h>
#include <stdlib.h>

void* mem_alloc(uint32_t size);
void* mem_realloc(void* ptr, uint32_t new_size);
void mem_free(void* ptr);
void mem_move(void* dest, void* src, uint32_t size);
void mem_set(void* dest, uint32_t data, uint32_t size);
int mem_compare(void* buff1, void* buff2, uint32_t size);

#include <stddef.h>

void* sys_alloc(size_t count, size_t size);
void* sys_realloc(void* data, size_t size);
void sys_free(void* data);
void sys_heap_info(void);

#define plat_alloc(fmt) sys_alloc(1, fmt)
#define plat_realloc(fmt, size) sys_realloc(fmt, size)
#define plat_free(x) sys_free(x)
#define PLAT_FREE(x)          \
	if (x != NULL) {      \
		plat_free(x); \
		x = NULL;     \
	}

#endif
