/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _IONMEMORY_H_
#define _IONMEMORY_H_

#define CONFIG_ION

#if 0
#if TARGET_ION_ABI_VERSION >= 2
#include <android-base/ion.h>
#include <linux/dma-buf.h>
#endif  // end
#endif

#include "gf_error.h"
#include "gf_base_types.h"
#include <linux/msm_ion.h>

// public struct
typedef struct {
    void *buffer;
    int32_t bufferFd;
} BUFFER_INFO;

typedef int ion_user_handle_t;

struct ion_handle_data {
    ion_user_handle_t handle;
};

namespace goodix {
    class IonMemory {
    public:
        IonMemory();
        ~IonMemory();
        BUFFER_INFO *allocate(int32_t size);
        gf_error_t free(BUFFER_INFO *info);
        void getCarveoutIonHandle(int32_t *fd, int32_t *len);
        gf_error_t carveoutIonInit();
        void carveoutIonDestroy();
    private:
        gf_error_t init();
        gf_error_t destroy();
        gf_error_t doAllocate(int32_t size, BUFFER_INFO *info);
        gf_error_t doFree(BUFFER_INFO *info);
        gf_error_t createBuffer(int32_t size, BUFFER_INFO **ppInfo);
        gf_error_t carveoutIonMemalloc(uint32_t size);
        BUFFER_INFO *mpCachedInfo;
        int32_t mIonDeviceFd;
        int32_t mCarveoutIonHandleFd;  // carveout ion fd
        int32_t mCarveoutIonHandleLen;  // carveout ion len
        int32_t mIonFd;  // ion fd
    };
}

#endif  // _IONMEMORY_H_
