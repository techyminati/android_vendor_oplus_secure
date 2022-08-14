#ifndef __LIVER_IMAGE_COMMON__
#define __LIVER_IMAGE_COMMON__

#ifdef ALGO_OUT_BPP_32
#define LIVE_IMAGE_V2
#include "liver_image_v2.h"
#else
#define LIVE_IMAGE_V1
#include "liver_image_v1.h"
#endif

#endif
