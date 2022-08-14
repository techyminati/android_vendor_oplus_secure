#include "anc_utils.h"

#include <stdio.h>


int AncSnprintf(char *p_str, size_t size, const char *p_format, ...) {
    va_list arg;
    int n;
    va_start(arg, p_format);
    n = AncVsnprintf(p_str, size, p_format, arg);
    va_end(arg);
    return n;
}

int AncVsnprintf(char* const dest, size_t size, const char* format, va_list ap) {
    return vsnprintf(dest, size, format, ap);
}

size_t AncStrlcat(char* dst, const char* src, size_t size) {
    return strlcat(dst, src, size);
}
