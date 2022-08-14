/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _IONMEMORY_H_
#define _IONMEMORY_H_

#include <sys/mman.h>
#include <ion/ion.h>
//#include "linux/mtk_ion.h"
#include "gf_error.h"

namespace goodix {

    class IonMemory {
    public:
        gf_error_t destroy();
        gf_error_t init();
        IonMemory();
        ~IonMemory();

    private:
        gf_error_t doAllocate();
        gf_error_t doFree();
        int32_t ion_fd;
        bool ion_malloc_flag;
        ion_user_handle_t ion_handle;
    };
}  // namespace goodix

#endif  // #ifndef _IONMEMORY_H_
