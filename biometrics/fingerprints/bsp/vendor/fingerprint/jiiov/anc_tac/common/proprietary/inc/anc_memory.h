#ifndef __ANC_MEMORY_H__
#define __ANC_MEMORY_H__

#include "anc_type.h"

void* AncPlatformMalloc(size_t size);
void AncPlatformFree(void *ptr);
void* AncPlatformMemset(void *s, int ch, size_t n);
void* AncPlatformMemcpy(void *destin, const void *source, size_t n);
int AncPlatformMemcmp(const void *s1, const void *s2, size_t n);
size_t AncPlatformMemscpy(void *dest, size_t dest_size,
                            const void *src, size_t src_size);

#endif
