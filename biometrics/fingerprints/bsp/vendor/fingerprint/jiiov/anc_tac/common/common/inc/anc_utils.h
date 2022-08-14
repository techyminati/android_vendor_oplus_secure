#ifndef __ANC_UTILS_H__
#define __ANC_UTILS_H__

#include "anc_memory_wrapper.h"
#include "anc_type.h"

int AncSnprintf(char *p_str, size_t size, const char *p_format, ...);
int AncVsnprintf(char* const dest, size_t size, const char* format,
                  va_list ap);
size_t AncStrlcat(char* dst, const char* src, size_t size);

#endif