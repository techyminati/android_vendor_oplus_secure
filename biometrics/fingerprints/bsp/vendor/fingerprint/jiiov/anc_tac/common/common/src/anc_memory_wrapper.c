#define LOG_TAG "[ANC_COMMON][MemoryWrapper]"

#include "anc_memory_wrapper.h"
#include "anc_log.h"

#ifdef ANC_DEBUG
#include "anc_memory_debug.h"
#endif

#include "anc_memory.h"

void *AncMemoryWrapperMalloc(size_t size, const char *p_func_name, int line) {
    void *p_addr = NULL;

    if (0 == size) {
        return NULL;
    } else {
        p_addr = AncPlatformMalloc(size);
        if(NULL == p_addr) {
            ANC_LOGE("function name is %s, line is %d", p_func_name, line);
            return NULL;
        }
    }

#ifdef ANC_DEBUG
    AncRegisterAllocation(p_addr, size, p_func_name, line);
#endif

    return p_addr;
}

void AncMemoryWrapperFree(void *p_addr) {
    if (NULL == p_addr) {
        return;
    }

#ifdef ANC_DEBUG
    AncUnRegisterAllocation(p_addr);
#endif
    if (NULL != p_addr) {
        AncPlatformFree(p_addr);
        p_addr = NULL;
    }
}

ANC_RETURN_TYPE AncMemoryWrapperCheck() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
#ifdef ANC_DEBUG
    ret_val = AncMemoryCheck();
#endif
    return ret_val;
}

ANC_RETURN_TYPE AncMemoryWrapperReleaseChecker() {
#ifdef ANC_DEBUG
    AncMemDebugRelease();
#endif
    return ANC_OK;
}

void *AncMemoryWrapperMemset(void *s, int ch, size_t n) {
    return AncPlatformMemset(s, ch, n);
}

void *AncMemoryWrapperMemcpy(void *destin, const void *source, size_t n) {
    return AncPlatformMemcpy(destin, source, n);
}

int AncMemoryWrapperMemcmp(const void *s1, const void *s2, size_t n) {
    return AncPlatformMemcmp(s1, s2, n);
}

size_t AncMemoryWrapperMemscpy(void *dest, size_t dest_size, const void *src, size_t src_size) {
    return AncPlatformMemscpy(dest, dest_size, src, src_size);
}

