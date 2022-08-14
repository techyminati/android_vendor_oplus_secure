#ifndef __ANC_TYPE_H__
#define __ANC_TYPE_H__

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#ifndef ANC_UNUSED
#define ANC_UNUSED(x) (void)(x)
#endif

#undef ANC_FALSE
#define ANC_FALSE 0

#undef ANC_TRUE
#define ANC_TRUE (!ANC_FALSE)

typedef char ANC_BOOL;

#endif
