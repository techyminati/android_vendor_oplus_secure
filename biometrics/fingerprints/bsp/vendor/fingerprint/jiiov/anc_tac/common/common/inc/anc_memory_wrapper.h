#ifndef __ANC_MEMORY_WRAPPER_H__
#define __ANC_MEMORY_WRAPPER_H__

#include "anc_type.h"
#include "anc_error.h"

void *AncMemoryWrapperMalloc(size_t size, const char *p_func_name, int line);
void AncMemoryWrapperFree(void *p_addr);
void *AncMemoryWrapperMemset(void *s, int c, size_t n);
void *AncMemoryWrapperMemcpy(void *dest, const void *src, size_t n);
int AncMemoryWrapperMemcmp(const void *s1, const void *s2, size_t n);
size_t AncMemoryWrapperMemscpy(void *dest, size_t dest_size,
                              const void *src, size_t src_size);

ANC_RETURN_TYPE AncMemoryWrapperCheck();
ANC_RETURN_TYPE AncMemoryWrapperReleaseChecker();
#define AncMalloc(size) AncMemoryWrapperMalloc(size, __FUNCTION__, __LINE__)
#define AncFree(ptr) AncMemoryWrapperFree(ptr)
#define AncMemset(s, c, n) AncMemoryWrapperMemset(s, c, n)
#define AncMemcpy(dest, src, n) AncMemoryWrapperMemcpy(dest, src, n)
#define AncMemcmp(dest, src, n) AncMemoryWrapperMemcmp(dest, src, n)
#define AncMemscpy(dest, dest_size, src, src_size) AncMemoryWrapperMemscpy(dest, dest_size, src, src_size)

#ifndef TRY_FREE
#define TRY_FREE(pointer) if(NULL != (pointer)){AncFree((pointer)); (pointer) = NULL;}
#endif

#endif
