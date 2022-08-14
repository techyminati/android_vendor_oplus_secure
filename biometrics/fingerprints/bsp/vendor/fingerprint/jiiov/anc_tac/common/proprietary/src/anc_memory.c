#include "anc_memory.h"

#include <stdlib.h>


void* AncPlatformMalloc(size_t size) {
    return malloc(size);
}

void AncPlatformFree(void *ptr) {
    free((void *)ptr);
}

void* AncPlatformMemset(void *s, int ch, size_t n) {
    return memset(s , ch , n);
}

void* AncPlatformMemcpy(void *destin, const void *source, size_t n) {
    return memcpy(destin , source , n);
}

int AncPlatformMemcmp(const void *s1, const void *s2, size_t n) {
    return memcmp(s1 , s2 , n);
}

size_t AncPlatformMemscpy(void *dest, size_t dest_size, const void *src, size_t src_size) {
    size_t copy_size = 0;

    if ((NULL != dest) && (NULL != src)) {
        copy_size = (dest_size <= src_size) ? dest_size : src_size;
        AncPlatformMemcpy(dest, src, copy_size);
    }

    return copy_size;
}