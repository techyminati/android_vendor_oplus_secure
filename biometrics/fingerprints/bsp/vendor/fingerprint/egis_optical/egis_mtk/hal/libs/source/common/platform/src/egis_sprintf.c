#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "plat_log.h"

#define LOG_TAG "RBS"
#ifdef QSEE
#define strcpy(buf, dst) strlcpy(buf, dst, sizeof(buf))
#endif

#ifdef _WINDOWS
#define snprintf _snprintf
#endif

#if defined(QSEE) && defined(TZ_MODE)
extern unsigned int qsee_strlcpy(char* pcDst, const char* pszSrc, unsigned int nDestSize);
#endif

char* plat_strncpy(char* dest, const char* src, size_t size) {
#if defined(QSEE) && defined(TZ_MODE)
    qsee_strlcpy(dest, src, size);
#else
    strncpy(dest, src, size);
#endif
    dest[size - 1] = '\0';

    return dest;
}

#if defined(__TRUSTONIC__) && defined(TZ_MODE)
#define MAX_NUM_SIZE 20
static void reverse(char* s) {
    for (int i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        int c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void egis_itoa(int n, char* str) {
    char s[MAX_NUM_SIZE + 2];
    int i, sign;
    if ((sign = n) < 0) {
        n = -n;
    }
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) {
        s[i++] = '-';
    }
    s[i] = '\0';
    reverse(s);
    strcpy(str, s);
}

int vsnprintf(char* buf, size_t size, const char* format, va_list arg) {
    int i = 0, m = 0, val, div, p, idx = 0;
    char ac, dst[512], *tmp, hex;

    while ((ac = format[i]) != '\0') {
        switch (m) {
            case 0:  // normal digit or control digit
                if (ac != '%') {
                    dst[idx++] = ac;
                } else
                    m = 1;
                break;
            case 1:  // control digit
                if (ac == 'd') {
                    div = 1000000000;
                    val = va_arg(arg, int);
                    for (; div >= 0; div /= 10)
                        if (val / div != 0) break;
                    for (; div > 0; div /= 10) {
                        p = val / div;
                        dst[idx++] = (char)p + '0';
                        val -= p * div;
                    }
                    m = 0;
                } else if (ac == 'x' || ac == 'X') {
                    if (ac == 'x')
                        hex = 'a';
                    else
                        hex = 'A';
                    div = 0x10000000;
                    val = va_arg(arg, int);
                    for (; div >= 0; div /= 16)
                        if (val / div != 0) break;
                    for (; div > 0; div /= 16) {
                        p = val / div;
                        dst[idx++] = (char)p + (p < 10 ? '0' : hex - 10);
                        val -= p * div;
                    }
                } else if (ac == 's') {
                    tmp = va_arg(arg, char*);
                    strcpy(dst + idx, tmp);
                    idx += strlen(tmp);
                } else
                    dst[idx++] = ac;
                m = 0;
                break;
        }

        i++;
    }

    dst[idx] = '\0';
    strcpy(buf, dst);
    return strlen(buf);
}
#endif

int egist_snprintf(char* str, size_t size, const char* format, ...) {
    va_list arg;
    int n;
    va_start(arg, format);
    n = vsnprintf(str, size, format, arg);
    va_end(arg);
    return n;
}

int egist_strnlen(const char* p, size_t num) {
    const char* beg = p;
    while (*p && num--) ++p;
    return p - beg;
}

int plat_set_template_path_prefix(char* template_path_prefix, size_t size, const char* path,
                                  const char* prefix) {
    int count;
#if defined(QSEE)
    count = egist_snprintf(template_path_prefix, size, "%s/%s", path, prefix);
#elif defined(__TRUSTONIC__)
    if (strlen(path) + strlen(prefix) + 1 + 1 < size) {
        strcpy(template_path_prefix, path);
        strcat(template_path_prefix, "_");
        strcat(template_path_prefix, prefix);
        count = strlen(template_path_prefix);
    } else {
        egislog_e("%s size %d is not enough!", __func__, (int)size);
        if (size > 0) {
            template_path_prefix[0] = '\0';
        }
        count = 0;
    }
#else
    count = snprintf(template_path_prefix, size, "%s/%s", path, prefix);
#endif
    return count;
}

int plat_set_ifaa_path_prefix(char* ifaa_path_prefix, size_t size, const char* path,
                              const char* prefix) {
    int count;
#if defined(QSEE)
    count = egist_snprintf(ifaa_path_prefix, size, "%s/%s", path, prefix);
#elif defined(__TRUSTONIC__)
    if (strlen(path) + strlen(prefix) + 1 + 1 < size) {
        strcpy(ifaa_path_prefix, path);
        strcat(ifaa_path_prefix, "/");
        strcat(ifaa_path_prefix, prefix);
        count = strlen(ifaa_path_prefix);
    } else {
        egislog_e("%s size %d is not enough!", __func__, (int)size);
        if (size > 0) {
            ifaa_path_prefix[0] = '\0';
        }
        count = 0;
    }
#else
    count = snprintf(ifaa_path_prefix, size, "%s/%s", path, prefix);
#endif
    return count;
}

// Return the total number of characters written
int get_app_ctx_path(char* path, size_t size, const char* app_ctx_path, int index) {
    int count;
#if defined(TZ_MODE)
#if defined(QSEE)
    count = egist_snprintf(path, size, "%s_%d", app_ctx_path, index);
#elif defined(__TRUSTONIC__)
    strcpy(path, "");
    strcat(path, app_ctx_path);
    strcat(path, "_");
    char str_index[MAX_NUM_SIZE];
    egis_itoa(index, str_index);
    strcat(path, str_index);
    count = strlen(path);
#else
    count = snprintf(path, size, "%s_%d", app_ctx_path, index);
#endif
#else
    count = snprintf(path, size, "%s_%d", app_ctx_path, index);
#endif
    return count;
}

// Return the total number of characters written
int get_version_string(char* data, unsigned int size, const char* lib_version,
                       const char* egis_lib_version, unsigned int minor, unsigned int build) {
    int count;
#if defined(TEEIGP) || !defined(TZ_MODE)
    count = snprintf(data, size, "%s.%s.%u.%u", lib_version, egis_lib_version, minor, build);
#elif defined(__TRUSTONIC__)
    strcpy(data, "");
    strcat(data, lib_version);
    strcat(data, egis_lib_version);
    char str_index[MAX_NUM_SIZE];
    egis_itoa(minor, str_index);
    strcat(data, str_index);
    egis_itoa(build, str_index);
    strcat(data, str_index);
    count = strlen(data);
#else
    count = egist_snprintf(data, size, "%s.%s.%u.%u", lib_version, egis_lib_version, minor, build);
#endif
    return count;
}
