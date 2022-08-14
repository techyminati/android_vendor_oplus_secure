/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "HalLog.h"
#include "IonMemory_mtk.h"
#include <linux/mtk_ion.h>
#define LOG_TAG "[gf_IonMemory]"

#define ION_MEMORY_SIZE (4096)

namespace goodix {
    IonMemory::IonMemory() {
        ion_malloc_flag = false;
        ion_fd = -1;
        ion_handle = 0;
    }

    IonMemory::~IonMemory() {
        destroy();
    }

    gf_error_t IonMemory::doAllocate() {
        gf_error_t err = GF_SUCCESS;
        int32_t ret = 0;

        FUNC_ENTER();

        do {
            if (!ion_malloc_flag) {
                ion_fd = ion_open();
                if (ion_fd <= 0) {
                    err = GF_ERROR_BAD_PARAMS;
                    LOG_E(LOG_TAG, "[%s] ion_open failed, fd=%d", __func__, ion_fd);
                    break;
                }

                ret = ion_alloc(ion_fd, ION_MEMORY_SIZE, 0, ION_HEAP_MULTIMEDIA_SDSP_SHARED_MASK, 0, &ion_handle);
                if (0 != ret) {
                    err = GF_ERROR_BAD_PARAMS;
                    LOG_E(LOG_TAG, "[%s] ion_alloc failed, ret=%d", __func__, ret);
                    break;
                }

                ion_malloc_flag = true;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t IonMemory::doFree() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (ion_malloc_flag) {
            ion_close(ion_fd);
            ion_malloc_flag = false;
        }

        FUNC_EXIT(err);

        return err;
    }

    gf_error_t IonMemory::init() {
        gf_error_t err = GF_SUCCESS;
        ion_malloc_flag = false;
        ion_fd = -1;
        FUNC_ENTER();

        err = doAllocate();

        FUNC_EXIT(err);

        return err;
    }

    gf_error_t IonMemory::destroy() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        err = doFree();

        FUNC_EXIT(err);

        return err;
    }
}  // namespace goodix