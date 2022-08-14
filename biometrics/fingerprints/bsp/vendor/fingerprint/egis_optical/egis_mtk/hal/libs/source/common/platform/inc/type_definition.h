//#pragma once
#ifndef __TYPE_DEFINITION__
#define __TYPE_DEFINITION__

#ifdef _WINDOWS
#include <Windows.h>
#include <stdio.h>
#define snprintf _snprintf
#define PATH_MAX MAX_PATH
#elif defined(__LINUX__) || defined(TZ_MODE)

#if defined(TZ_MODE) && defined(__TRUSTONIC__)
#define malloc malloc_NOT_SUPPORTED
#define sprintf sprintf_NOT_SUPPORTED
#define vsnprintf vsnprintf_NOT_SUPPORTED
#define snprintf snprintf_NOT_SUPPORTED
#endif

#include <wchar.h>

#define BYTE_DEFINED

#define LONG long
#define SHORT short
#define USHORT unsigned short
#define INT int
#define FLOAT float
#define BYTE unsigned char
#define CHAR char
#define DWORD unsigned long
#define WORD unsigned short
#define BOOL int
#define VOID void
#define UINT unsigned int
#define WCHAR wchar_t
#define HANDLE int
#define UINT8 unsigned char
#define UINT32 unsigned int

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#ifndef NULL
#define NULL 0
#endif
#endif

#endif
