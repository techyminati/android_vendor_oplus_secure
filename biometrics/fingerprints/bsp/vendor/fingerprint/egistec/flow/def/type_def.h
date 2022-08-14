#include <stdint.h>
#ifndef __TYPE_DEF_H__
#define __TYPE_DEF_H__

#define TRUE 1
#define FALSE 0

#ifdef __LINUX__
#ifndef BOOL
#define BOOL unsigned int
#endif
#else
typedef int BOOL;
#endif

#endif
