
#include "egis_log.h"
#define LOG_TAG "sys_heap"

#include "plat_mem.h"
#include "plat_log.h"
#include <stdlib.h>

void* mem_alloc(uint32_t size) { return malloc(size); }
void* mem_realloc(void* ptr, uint32_t new_size)
{
	return realloc(ptr, new_size);
}
void mem_free(void* ptr) { free(ptr); }
void mem_move(void* dest, void* src, uint32_t size) { memcpy(dest, src, size); }
void mem_set(void* dest, uint32_t data, uint32_t size)
{
	memset(dest, data, size);
}
int mem_compare(void* buff1, void* buff2, uint32_t size)
{
	return memcmp(buff1, buff2, size);
}

#if defined(HEAP_LOG)

#define HEAP_RECORD_MAX 5000
struct heap_info {
	unsigned long max_size;
	unsigned long current_size;
	int heap_max_index;
	void* heap_address[HEAP_RECORD_MAX];
	unsigned long heap_size[HEAP_RECORD_MAX];
};

struct heap_info g_heap_info;

void SHOW_HEAP_ALLOC_INFO(void* buf, size_t size)
{
	int i;

	if (buf == NULL) return;

	if (g_heap_info.heap_max_index == HEAP_RECORD_MAX) {
		egislog_e("heap_info buffer is full, 0x%x, %u", buf, size);
		return;
	}

	g_heap_info.current_size += size;
	if (g_heap_info.current_size > g_heap_info.max_size)
		g_heap_info.max_size = g_heap_info.current_size;

	for (i = 0; i <= g_heap_info.heap_max_index; i++) {
		if ((i == 0 &&
		     (uintptr_t)buf < (uintptr_t)g_heap_info.heap_address[i]) ||

		    (i > 0 &&
		     (uintptr_t)buf >
			 (uintptr_t)g_heap_info.heap_address[i - 1] &&
		     (uintptr_t)buf <=
			 (uintptr_t)g_heap_info.heap_address[i]) ||

		    (i == g_heap_info.heap_max_index &&
		     (uintptr_t)buf >
			 (uintptr_t)g_heap_info.heap_address[i - 1])) {
			memmove(
			    &g_heap_info.heap_address[i + 1],
			    &g_heap_info.heap_address[i],
			    sizeof(void*) * (g_heap_info.heap_max_index - i));

			memmove(&g_heap_info.heap_size[i + 1],
				&g_heap_info.heap_size[i],
				sizeof(unsigned long) *
				    (g_heap_info.heap_max_index - i));

			g_heap_info.heap_address[i] = buf;
			g_heap_info.heap_size[i] = size;
			break;
		}
	}

	g_heap_info.heap_max_index++;
}

void SHOW_HEAP_FREE_INFO(void* buf)
{
	int i;

	if (buf == NULL) return;

	for (i = 0; i < g_heap_info.heap_max_index; i++) {
		if (buf == g_heap_info.heap_address[i]) {
			g_heap_info.current_size -= g_heap_info.heap_size[i];

			memmove(
			    &g_heap_info.heap_address[i],
			    &g_heap_info.heap_address[i + 1],
			    sizeof(void*) * (g_heap_info.heap_max_index - i));

			memmove(&g_heap_info.heap_size[i],
				&g_heap_info.heap_size[i + 1],
				sizeof(unsigned long) *
				    (g_heap_info.heap_max_index - i));

			g_heap_info.heap_max_index--;
			break;
		}

		if (i == g_heap_info.heap_max_index - 1) {
			egislog_e("heap_info free address not found 0x%x", buf);
		}
	}
}

void sys_heap_info(void)
{
	int i;

	// for (i = 0; i < g_heap_info.heap_max_index; i++) {
	// egislog_d("heap_info 0x%x %d",
	// g_heap_info.heap_address[i], g_heap_info.heap_size[i]);

	// if (i != g_heap_info.heap_max_index - 1)
	// egislog(LOG_INFO, "heap_info free_size %dKB",
	// ((uintptr_t)g_heap_info.heap_address[i + 1] -
	// (uintptr_t)g_heap_info.heap_address[i] -
	// g_heap_info.heap_size[i]) >> 10);
	// }

	// egislog_d("heap_info --------------");
	egislog_d("heap_info = %dKB, max_size = %dKB",
		  g_heap_info.current_size / 1024, g_heap_info.max_size / 1024);
}

#else
#define SHOW_HEAP_ALLOC_INFO(a, b)
#define SHOW_HEAP_FREE_INFO(a)
void sys_heap_info(void) {}
#endif

void* sys_alloc(size_t count, size_t size)
{
	void* buf;

	buf = calloc(count, size);
	SHOW_HEAP_ALLOC_INFO(buf, size);

	return buf;
}
void* sys_realloc(void* data, size_t size)
{
	void* buf;

	SHOW_HEAP_FREE_INFO(data);
	buf = realloc(data, size);
	SHOW_HEAP_ALLOC_INFO(buf, size);

	return buf;
}
void sys_free(void* data)
{
	SHOW_HEAP_FREE_INFO(data);
	free(data);
}
