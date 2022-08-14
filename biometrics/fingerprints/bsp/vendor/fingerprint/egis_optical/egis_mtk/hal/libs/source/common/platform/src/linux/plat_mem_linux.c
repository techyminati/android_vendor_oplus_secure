
#include "egis_log.h"
#define LOG_TAG "sys_heap"

#include <stdlib.h>
#include "plat_log.h"
#include "plat_mem.h"

static void* mem_alloc(uint32_t size);
static void* mem_realloc(void* ptr, uint32_t new_size);
static void mem_free(void* ptr);

static void* mem_alloc(uint32_t size) {
    return malloc(size);
}
static void* mem_realloc(void* ptr, uint32_t new_size) {
    return realloc(ptr, new_size);
}
static void mem_free(void* ptr) {
    free(ptr);
}
void mem_move(void* dest, void* src, uint32_t size) {
    memcpy(dest, src, size);
}
void mem_set(void* dest, uint32_t data, uint32_t size) {
    memset(dest, data, size);
}
int mem_compare(void* buff1, void* buff2, uint32_t size) {
    return memcmp(buff1, buff2, size);
}

#ifdef EGIS_DEBUG_MEMORY
#include "egis_mem_debug.h"
void* sys_alloc(uint32_t count, uint32_t size, const char* file_name, int line) {
    void* buf;

    buf = ((size) ? mem_alloc(size) : 0);

    if (count == 0) {
        // egislog_d("*-*-* %s, %p:%d", file_name, buf, size);
        char name[NAME_LENGTH];
        egis_strncpy(name, file_name, NAME_LENGTH);
        register_mem(buf, size, name, line);
    } else {
        // egislog_d("*-*-* G3, %p:%d", buf, size);
        register_mem(buf, size, "ALGO", 1);
    }

    return buf;
}
void* sys_realloc(void* data, size_t size) {
    void* buf;

    buf = mem_realloc(data, size);

    // egislog_d("*-*-* realloc, %x, %p:%d", data, buf, size);
    unregister_mem(data);
    if (size > 0 && buf != NULL) {
        register_mem(buf, size, "realloc", 1);
    } else {
        egislog_d("## realloc, buf=NULL new_size=%d", size);
    }

    return buf;
}
void sys_free(void* data) {
    if (data == NULL) {
        // egislog_d("## sys_free addr=NULL");
        return;
    }

    mem_free(data);

    unregister_mem(data);
}
#else
void* sys_alloc(size_t count, size_t size) {
    void* buf;

    buf = mem_alloc(size * count);

    return buf;
}
void* sys_realloc(void* data, size_t size) {
    void* buf;

    buf = mem_realloc(data, size);

    return buf;
}
void sys_free(void* data) {
    mem_free(data);
}
#endif
