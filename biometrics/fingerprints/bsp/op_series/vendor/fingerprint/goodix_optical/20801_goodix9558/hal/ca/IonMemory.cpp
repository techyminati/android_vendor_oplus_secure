/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
// solve build error in header file linux/msm_ion.h when compiler is clang/g++
// By default, void* is not allowed to converted to other pointer
// implicitly for clang/g++ compiler, but no problem for gcc.
// This macro disable the source code with problem in header file.
#define CONFIG_ION


#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <string.h>
#include "IonMemory.h"
#include "HalLog.h"
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#include <linux/dma-buf.h>
#endif  // end
#define LOG_TAG "[IonMemory]"

#if TARGET_ION_ABI_VERSION >= 2
#define CLOSE_FD ion_close
#define DEFAULT_ALLOCATE_SIZE    (2048 * 1024)
#else  // 0, 1
#define CLOSE_FD close
#define DEFAULT_ALLOCATE_SIZE    (16 * 1024)
#endif  // end
#define PAGE_ALIGN(addr)         (((addr) + PAGE_SIZE - 1) & PAGE_MASK)
#define TA_DSP_SHARE_BUFFER_SIZE (4 * 1024 * 1024)

// the size of current default buffer
static int32_t g_cur_buf_size = 0;

// private struct
typedef struct {
    BUFFER_INFO base;
    int32_t bufferHandle;
    int32_t bufferSize;
} EXTENDED_BUFFER_INFO;

namespace goodix {

    IonMemory::IonMemory() {
        mIonDeviceFd = 0;
        mpCachedInfo = NULL;
        mCarveoutIonHandleFd = 0;
        mCarveoutIonHandleLen = 0;
        mIonFd = 0;
        init();
    }

    IonMemory::~IonMemory() {
        destroy();
    }

    gf_error_t IonMemory::createBuffer(int32_t size, BUFFER_INFO **ppInfo) {
        gf_error_t err = GF_SUCCESS;
        BUFFER_INFO *pInfo = NULL;

        do {
            pInfo = (BUFFER_INFO *) new EXTENDED_BUFFER_INFO();

            if (pInfo == NULL) {
                err = GF_ERROR_OUT_OF_MEMORY;
                LOG_E(LOG_TAG, "[%s] create buffer info failed.", __func__);
                break;
            }

            memset(pInfo, 0, sizeof(EXTENDED_BUFFER_INFO));
            err = doAllocate(size, pInfo);
        } while (0);

        if (err != GF_SUCCESS) {
            if (pInfo != NULL) {
                delete pInfo;
            }
        } else {
            *ppInfo = pInfo;
        }

        return err;
    }

    BUFFER_INFO *IonMemory::allocate(int32_t size) {
        gf_error_t err = GF_SUCCESS;
        BUFFER_INFO *pInfo = NULL;
        FUNC_ENTER();

        do {
            if (size <= 0) {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] The parameter is invalid, size=%d", __func__, size);
                break;
            }

            // make sure init is done
            if (mIonDeviceFd == 0) {
                err = init();

                if (err != GF_SUCCESS) {
                    break;
                }
            }

            if (mpCachedInfo != NULL) {
                if (size <= g_cur_buf_size) {
                    pInfo = mpCachedInfo;
                    break;
                }

                // replace cached buffer with the new bigger buffer
                doFree(mpCachedInfo);
                delete mpCachedInfo;
                mpCachedInfo = NULL;
                g_cur_buf_size = 0;
            }

            err = createBuffer(size, &pInfo);
            if (err != GF_SUCCESS) {
                break;
            }
            mpCachedInfo = pInfo;
            g_cur_buf_size = size;
            LOG_I(LOG_TAG, "[%s] create new buffer success, size: %d", __func__, g_cur_buf_size);
        } while (0);

        if (err != GF_SUCCESS) {
            if (pInfo != NULL && pInfo != mpCachedInfo) {
                if (pInfo->buffer != NULL) {
                    doFree(pInfo);
                }

                delete (pInfo);
                pInfo = NULL;
            }
        }

        FUNC_EXIT(err);
        return pInfo;
    }

    gf_error_t IonMemory::free(BUFFER_INFO *info) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (info == NULL) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            EXTENDED_BUFFER_INFO *pExtInfo = (EXTENDED_BUFFER_INFO *) info;
            memset(pExtInfo->base.buffer, 0, pExtInfo->bufferSize);

            if (info == mpCachedInfo) {
                break;
            }

            err = doFree(info);
            delete info;
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void IonMemory::getCarveoutIonHandle(int32_t *fd, int32_t *len) {
        VOID_FUNC_ENTER();
        *fd = mCarveoutIonHandleFd;
        *len = mCarveoutIonHandleLen;
        VOID_FUNC_EXIT();
    }

    gf_error_t IonMemory::carveoutIonInit() {
        int32_t retval = 0;
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (0 == mCarveoutIonHandleFd) {
                retval = carveoutIonMemalloc(TA_DSP_SHARE_BUFFER_SIZE);

                if (retval) {
                    err = GF_ERROR_GENERIC;
                    LOG_E(LOG_TAG, "[%s] qsc_ion_carveout_memalloc failed with retval = %d\n", __func__, retval);
                    break;
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void IonMemory::carveoutIonDestroy() {
        VOID_FUNC_ENTER();
        VOID_FUNC_EXIT();
        return;
    }
    gf_error_t IonMemory::init() {
        gf_error_t err = GF_SUCCESS;
        BUFFER_INFO *pInfo = NULL;
        FUNC_ENTER();

        do {
            if (mIonDeviceFd != 0) {
                LOG_E(LOG_TAG, "[%s] ION device is already opened.", __func__);
                err = GF_ERROR_GENERIC;
                break;
            }

#if TARGET_ION_ABI_VERSION >= 2
            mIonDeviceFd = ion_open();
#else  // else
            mIonDeviceFd = open("/dev/ion", O_RDONLY);
#endif  // end

            if (mIonDeviceFd < 0) {
                mIonDeviceFd = 0;
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] open ION device '/dev/ion' failed.", __func__);
                break;
            }

            err = createBuffer(DEFAULT_ALLOCATE_SIZE, &pInfo);
        } while (0);

        if (err == GF_SUCCESS) {
            mpCachedInfo = pInfo;
            g_cur_buf_size = DEFAULT_ALLOCATE_SIZE;
        } else {
            if (mIonDeviceFd != 0) {
                CLOSE_FD(mIonDeviceFd);
                mIonDeviceFd = 0;
            }

            delete pInfo;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t IonMemory::destroy() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (mIonDeviceFd > 0) {
            CLOSE_FD(mIonDeviceFd);
            mIonDeviceFd = 0;
        }

        if (mpCachedInfo != NULL) {
            err = doFree(mpCachedInfo);
            delete mpCachedInfo;
            mpCachedInfo = NULL;
        }

        if (mCarveoutIonHandleFd > 0) {
            CLOSE_FD(mCarveoutIonHandleFd);
            mCarveoutIonHandleFd = 0;
        }

        if (mIonFd > 0) {
            CLOSE_FD(mIonFd);
            mIonFd = 0;
        }

        FUNC_EXIT(err);
        return err;
    }
#if TARGET_ION_ABI_VERSION >= 2
    gf_error_t IonMemory::doAllocate(int32_t size, BUFFER_INFO *info) {
        gf_error_t err = GF_SUCCESS;
        EXTENDED_BUFFER_INFO *pExtInfo = (EXTENDED_BUFFER_INFO *) info;
        int32_t ret = 0;
        unsigned char *v_addr;
        int32_t map_fd = -1;
        struct dma_buf_sync buf_sync;
        uint32_t len = PAGE_ALIGN(size);
        uint32_t align = 0;
        uint32_t flags = 0;
        unsigned int heap_id = ION_QSECOM_HEAP_ID;
        ret = ion_alloc_fd(mIonDeviceFd, len, align, ION_HEAP(heap_id), flags, &map_fd);

        if (ret) {
            LOG_E(LOG_TAG,
                  "[%s] Error::ion_alloc_fd for heap %u size %u len %u failed ret = %d, errno = %d\n",
                  __func__, heap_id, size, len, ret, errno);
            err = GF_ERROR_GENERIC;
            goto alloc_fail;
        }

        v_addr = (unsigned char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED,
                                       map_fd, 0);

        if (v_addr == MAP_FAILED) {
            LOG_E(LOG_TAG, "[%s] Error::mmap failed", __func__);
            err = GF_ERROR_GENERIC;
            goto map_fail;
        }

        pExtInfo->base.bufferFd = map_fd;  // map fd
        pExtInfo->base.buffer = v_addr;  // mmap virtual addr
        pExtInfo->bufferSize = len;  // mmap virtual buffer size
        buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
        ret = ioctl(map_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);

        if (ret) {
            LOG_E(LOG_TAG,
                  "[%s] Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d\n",
                  __func__, ret, errno);
            err = GF_ERROR_GENERIC;
            goto sync_fail;
        }

        return err;
sync_fail:

        if (v_addr) {
            munmap(v_addr, len);
        }

map_fail:

        if (map_fd > 0) {
            ion_close(map_fd);
        }

alloc_fail:

        if (mIonDeviceFd > 0) {
            ion_close(mIonDeviceFd);
        }

        memset(info, 0, sizeof(EXTENDED_BUFFER_INFO));
        return err;
    }
    gf_error_t IonMemory::doFree(BUFFER_INFO *info) {
        EXTENDED_BUFFER_INFO *pExtInfo = (EXTENDED_BUFFER_INFO *) info;
        struct dma_buf_sync buf_sync;
        uint32_t len = pExtInfo->bufferSize;
        gf_error_t err = GF_SUCCESS;
        int ret = 0;
        buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
        ret = ioctl(pExtInfo->base.bufferFd, DMA_BUF_IOCTL_SYNC, &buf_sync);

        if (ret) {
            LOG_E(LOG_TAG,
                  "[%s] Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d\n",
                  __func__, ret, errno);
            err = GF_ERROR_GENERIC;
        }

        if (pExtInfo->base.buffer) {  // mmap virtual addr
            munmap(pExtInfo->base.buffer, len);
        }

        if (pExtInfo->base.bufferFd > 0) {  // map fd
            ion_close(pExtInfo->base.bufferFd);
        }

        memset(info, 0, sizeof(EXTENDED_BUFFER_INFO));
        return err;
    }

    gf_error_t IonMemory::carveoutIonMemalloc(uint32_t size) {
        int32_t ret = -1;
        int32_t ion_fd = 0;
        int32_t map_fd = -1;
        uint32_t align = 4096;
#ifdef SHIFT_DSP_ION_MEM_FLAG
        uint32_t flags = ((1 << 31) | (1 << 29));
#else
        uint32_t flags = (1 << 29);
#endif
        uint32_t len = (size + 4095) & (~4095);
        gf_error_t err = GF_SUCCESS;

        do {
            ion_fd = ion_open();

            if (ion_fd < 0) {
                LOG_E(LOG_TAG, "[%s] Error::Cannot open ION device\n", __func__);
                err = GF_ERROR_GENERIC;
                break;
            }

            mCarveoutIonHandleFd = 0;
            ret = ion_alloc_fd(ion_fd, len, align, ION_HEAP(14), flags, &map_fd);

            if (ret || !map_fd) {
                LOG_E(LOG_TAG, "[%s] Error::ion_alloc_fd for size %u len %u failed ret = %d\n", __func__, size, len, ret);
                err = GF_ERROR_GENERIC;
                break;
            }

            mCarveoutIonHandleFd = map_fd;
            mCarveoutIonHandleLen = len;
        } while (0);

        if (GF_SUCCESS != err && ion_fd > 0) {
            ion_close(ion_fd);
        }

        if (GF_SUCCESS == err) {
            mIonFd = ion_fd;
        }

        return err;
    }
#else  // else
    gf_error_t IonMemory::doAllocate(int32_t size, BUFFER_INFO *info) {
        struct ion_allocation_data allocData = { 0 };
        struct ion_fd_data fdData = { 0 };
        void *addr = NULL;
        int32_t ret = 0;
        gf_error_t err = GF_SUCCESS;
        EXTENDED_BUFFER_INFO *pExtInfo = (EXTENDED_BUFFER_INFO *) info;

        do {
            // 1. request to allocate ION buffer
            allocData.len = PAGE_ALIGN(size);
            allocData.align = PAGE_SIZE;
            // memory is allocated from EBI heap
            allocData.heap_id_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
            // set the memory to be uncached
            allocData.flags = 0;
            ret = ioctl(mIonDeviceFd, ION_IOC_ALLOC, &allocData);

            if (ret != 0) {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] IOC Allocate memory buffer failed.", __func__);
                break;
            }

            // 2. map ION buffer handle to file descriptor
            fdData.handle = allocData.handle;
            ret = ioctl(mIonDeviceFd, ION_IOC_MAP, &fdData);

            if (ret != 0) {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] IOC map memory buffer failed.", __func__);
                break;
            }

            // 3. map file descriptor to current process, retrieve virtual address
            addr = mmap(NULL, PAGE_ALIGN(size), PROT_READ | PROT_WRITE,
                        MAP_SHARED, fdData.fd, 0);

            if (addr == MAP_FAILED) {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] mmap fd to virtual address of current proccess failed.",
                      __func__);
                break;
            }
        } while (0);

        pExtInfo->bufferHandle = allocData.handle;
        pExtInfo->bufferSize = allocData.len;
        pExtInfo->base.bufferFd = fdData.fd;
        pExtInfo->base.buffer = addr;

        if (err == GF_SUCCESS) {
            return GF_SUCCESS;
        }

        doFree((BUFFER_INFO *) pExtInfo);
        return err;
    }

    gf_error_t IonMemory::doFree(BUFFER_INFO *info) {
        EXTENDED_BUFFER_INFO *pExtInfo = (EXTENDED_BUFFER_INFO *) info;
        int32_t ret = 0;
        gf_error_t err = GF_SUCCESS;

        if (pExtInfo->base.buffer != MAP_FAILED) {
            ret = munmap(pExtInfo->base.buffer, pExtInfo->bufferSize);

            if (ret) {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] munmap ion buffer failed, ret=%d", __func__, ret);
            }
        }

        if (pExtInfo->base.bufferFd > 0) {
            close(pExtInfo->base.bufferFd);
        }

        if (pExtInfo->bufferHandle != 0) {
            struct ion_handle_data handleData = { 0 };
            handleData.handle = pExtInfo->bufferHandle;
            ret = ioctl(mIonDeviceFd, ION_IOC_FREE, &handleData);

            if (ret) {
                err = GF_ERROR_GENERIC;
                LOG_E(LOG_TAG, "[%s] free ion buffer failed, ret=%d", __func__, ret);
            }
        }


        memset(info, 0, sizeof(EXTENDED_BUFFER_INFO));
        return err;
    }

    gf_error_t IonMemory::carveoutIonMemalloc(uint32_t size) {
        int32_t iret = 0;
        struct ion_allocation_data ion_alloc_data = { 0 };
        int32_t ion_fd = 0;
        int32_t rc = 0;
        struct ion_fd_data ifd_data = { 0 };
        struct ion_handle_data handle_data = { 0 };
        gf_error_t err = GF_SUCCESS;
        /* open ION device for memory management
         * O_DSYNC -> uncached memory
         */

        do {
            ion_fd = open("/dev/ion", O_RDONLY);

            if (ion_fd < 0) {
                LOG_E(LOG_TAG, "[%s] Error::Cannot open ION device\n", __func__);
                err = GF_ERROR_GENERIC;
                break;
            }

            mCarveoutIonHandleFd = 0;
            /* Size of allocation */
            ion_alloc_data.len = (size + 4095) & (~4095);
            /* 4K aligned */
            ion_alloc_data.align = 4096;
            /*!!!!!  memory is allocated from CARVEOUT heap !!!!!!*/
            ion_alloc_data.heap_id_mask = ION_HEAP(14);  // careourt heap id :14,
            /* Set the memory to be uncached */
            ion_alloc_data.flags = ((1 << 31) | (1 << 29));
            /* IOCTL call to ION for memory request */
            rc = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);

            if (rc) {
                LOG_E(LOG_TAG, "[%s] Error::Error while trying to allocate data\n", __func__);
                close(ion_fd);
                err = GF_ERROR_GENERIC;
                break;
            }

            if (ion_alloc_data.handle) {
                ifd_data.handle = ion_alloc_data.handle;
            } else {
                LOG_E(LOG_TAG, "[%s] Error::ION alloc data returned a NULL\n", __func__);
                close(ion_fd);
                err = GF_ERROR_GENERIC;
                break;
            }

            /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
            rc = ioctl(ion_fd, ION_IOC_SHARE, &ifd_data);

            if (rc) {
                LOG_E(LOG_TAG, "[%s] Error::Failed doing ION_IOC_MAP call\n", __func__);
                err = GF_ERROR_GENERIC;
                handle_data.handle = ion_alloc_data.handle;

                if (ifd_data.fd) {
                    close(ifd_data.fd);
                }

                iret = ioctl(ion_fd, ION_IOC_FREE, &handle_data);

                if (iret) {
                    LOG_E(LOG_TAG, "[%s] Error::ION FREE ioctl returned error = %d\n", __func__, iret);
                }

                close(ion_fd);
                break;
            }

            mCarveoutIonHandleFd = ifd_data.fd;
            mCarveoutIonHandleLen = size;
        } while (0);

        if (GF_SUCCESS == err) {
            mIonFd = ion_fd;
        }

        return err;
    }
#endif  // endf
}  // namespace goodix
