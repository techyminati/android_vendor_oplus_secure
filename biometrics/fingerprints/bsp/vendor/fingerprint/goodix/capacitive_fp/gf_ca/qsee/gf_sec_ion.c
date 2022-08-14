/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_sec_ion file
 * History:
 * Version: 1.0
 */
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <android/log.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/msm_ion.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <linux/ion.h>
#include <ion/ion.h>
#include <linux/dma-buf.h>
#endif  // end TARGET_ION_ABI_VERSION >= 2
#include "gf_user_type_define.h"
#include "gf_keymaster.h"
#include "QSEEComAPI.h"
#include "gf_error.h"
#include "gf_ca_entry.h"
#include "gf_type_define.h"
#include "gf_sec_ion.h"
#include "gf_common.h"

static qsc_ion_info_t g_ion_handle;  // ion handle
static bool g_ion_malloc_flag = false;  // ion malloc flag
#define GF_MALLOC_MAX_LENGTH (sizeof(gf_dump_template_t) + (20 * 1024))


#define LOG_TAG "[gf_sec_ion]"
#define LOG_D(...) (__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOG_E(...) (__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#if TARGET_ION_ABI_VERSION >= 2
typedef struct dma_buf_sync dma_buf_sync_t;
#endif  // #if TARGET_ION_ABI_VERSION >= 2

static int32_t qsc_ion_dealloc(struct qsc_ion_info *handle);

/**
 * Function: gf_sec_ion_malloc
 * Description: malloc memory for handle
 * Input: handle, data_size
 * Output: none
 * Return:
 * 0   if succeed to malloc memory for handle
 * -1  if fail to malloc memory for handle
 */
int32_t gf_sec_ion_malloc(struct qsc_ion_info *handle, int32_t data_size)
{
    int32_t retval = 0;
    int32_t ret = 0;
    LOG_D("[%s] enter", __func__);

    do
    {
        if (NULL == handle)
        {
            LOG_E("[%s] gf_sec_ion_malloc handle is NULL\n", __func__);
            ret = -1;
            break;
        }

        handle->ion_local_flag = false;
        LOG_D("[%s] gf_sec_ion_malloc data_size = %d\n", __func__, data_size);

        if (GF_MALLOC_MAX_LENGTH < data_size)
        {
            retval = qsc_ion_memalloc(handle, data_size);

            if (0 != retval)
            {
                LOG_E("[%s] qsc_ion_memalloc failed with retval = %d\n", __func__, retval);
                ret = -1;
                break;
            }

            handle->ion_local_flag = true;
        }
        else
        {
            if (!g_ion_malloc_flag)
            {
                retval = qsc_ion_memalloc(&g_ion_handle, GF_MALLOC_MAX_LENGTH);

                if (0 != retval)
                {
                    LOG_E("[%s] qsc_ion_memalloc failed with retval = %d\n", __func__, retval);
                    ret = -1;
                    break;
                }

                g_ion_malloc_flag = true;
            }

            memcpy(handle, &g_ion_handle, sizeof(qsc_ion_info_t));
        }

        ret = 0;
    } while (0);  // do...

    LOG_D("[%s] exit", __func__);
    return ret;
}

/**
 * Function: gf_sec_ion_free
 * Description: free memory for handle
 * Input: handle
 * Output: none
 * Return: none
 */
void gf_sec_ion_free(struct qsc_ion_info *handle)
{
    LOG_D("[%s] enter", __func__);

    do
    {
        if (NULL == handle)
        {
            LOG_E("[%s] gf_sec_ion_free handle is NULL\n", __func__);
            break;
        }

        if (!handle->ion_local_flag)
        {
            LOG_D("[%s] gf_sec_ion_free ion_local_flag is not true\n", __func__);
            break;
        }

        qsc_ion_dealloc(handle);
    }
    while (0);

    LOG_D("[%s] exit", __func__);
    return;
}

/**
 * Function: gf_sec_ion_init
 * Description: init gf_sec_ion
 * Input: none
 * Output: none
 * Return: none
 */
void gf_sec_ion_init()
{
    int32_t retval = 0;
    LOG_D("[%s] enter", __func__);

    do
    {
        g_ion_malloc_flag = false;
        memset(&g_ion_handle, 0, sizeof(qsc_ion_info_t));
        retval = qsc_ion_memalloc(&g_ion_handle, GF_MALLOC_MAX_LENGTH);

        if (retval)
        {
            LOG_E("[%s] gf_sec_ion_init failed with retval = %d\n", __func__, retval);
            break;
        }

        g_ion_malloc_flag = true;
    }
    while (0);

    LOG_D("[%s] exit", __func__);
    return;
}

/**
 * Function: gf_sec_ion_destroy
 * Description: destroy gf_sec_ion
 * Input: none
 * Output: none
 * Return: none
 */
void gf_sec_ion_destroy()
{
    LOG_D("[%s] enter", __func__);

    do
    {
        if (g_ion_malloc_flag)
        {
            qsc_ion_dealloc(&g_ion_handle);
            g_ion_malloc_flag = false;
            memset(&g_ion_handle, 0, sizeof(qsc_ion_info_t));
        }
    }
    while (0);

    LOG_D("[%s] exit", __func__);
    return;
}

#if TARGET_ION_ABI_VERSION < 2
/**
 * Function: qsc_ion_memalloc
 * Description: allocate memory for qsc_ion
 * Input: handle, size
 * Output: none
 * Return:
 * 0       if succeed to allocate memory for qsc_ion
 * others  if fail to allocate memory for qsc_ion
 */
int32_t qsc_ion_memalloc(struct qsc_ion_info *handle, uint32_t size)
{
    int32_t ret = 0;
    int32_t iret = 0;
    int32_t fd = 0;
    unsigned char *v_addr = NULL;
    struct ion_allocation_data ion_alloc_data = { 0 };
    int32_t ion_fd = 0;
    int32_t rc = 0;
    struct ion_fd_data ifd_data = { 0 };
    struct ion_handle_data handle_data = { 0 };
    /* open ION device for memory management
     * O_DSYNC -> uncached memory
     */
    LOG_D("[%s] enter", __func__);

    do
    {
        if (NULL == handle)
        {
            LOG_E("[%s] Error:: null handle received", __func__);
            ret = -1;
            break;
        }

        ion_fd = open("/dev/ion", O_RDONLY);

        if (ion_fd < 0)
        {
            LOG_E("[%s] Error::Cannot open ION device\n", __func__);
            ret = -1;
            break;
        }

        handle->ion_sbuffer = NULL;
        handle->ifd_data_fd = 0;
        /* Size of allocation */
        ion_alloc_data.len = (size + 4095) & (~4095);
        /* 4K aligned */
        ion_alloc_data.align = 4096;
        /* memory is allocated from EBI heap */
        ion_alloc_data.heap_id_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
        /* Set the memory to be uncached */
        ion_alloc_data.flags = 0;
        /* IOCTL call to ION for memory request */
        rc = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);

        if (rc)
        {
            LOG_E("[%s] Error::Error while trying to allocate data\n", __func__);
            ret = -1;
            if (ion_fd)
            {
                close(ion_fd);
            }
            break;
        }

        if (ion_alloc_data.handle)
        {
            ifd_data.handle = ion_alloc_data.handle;
        }
        else
        {
            LOG_E("[%s] Error::ION alloc data returned a NULL\n", __func__);
            ret = -1;
            if (ion_fd)
            {
                close(ion_fd);
            }
            break;
        }

        /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
        rc = ioctl(ion_fd, ION_IOC_MAP, &ifd_data);

        if (rc)
        {
            LOG_E("[%s] Error::Failed doing ION_IOC_MAP call\n", __func__);
            ret = -1;
            handle_data.handle = ion_alloc_data.handle;

            if (handle->ifd_data_fd)
            {
                close(handle->ifd_data_fd);
            }

            iret = ioctl(ion_fd, ION_IOC_FREE, &handle_data);

            if (iret)
            {
                LOG_E("[%s] Error::ION FREE ioctl returned error = %d\n", __func__, iret);
            }
            break;
        }

        /* Make the ion mmap call */
        v_addr = (unsigned char *)mmap(NULL, ion_alloc_data.len,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, ifd_data.fd, 0);

        if (v_addr == MAP_FAILED)
        {
            LOG_E("[%s] Error::ION MMAP failed\n", __func__);
            ret = -1;
            if (handle->ion_sbuffer != NULL)
            {
                ret = munmap(handle->ion_sbuffer, ion_alloc_data.len);

                if (ret)
                {
                    LOG_E("[%s] Error::Failed to unmap memory for load image. ret = %d\n",
                        __func__, ret);
                }
            }
            break;
        }

        handle->ion_fd = ion_fd;
        handle->ifd_data_fd = ifd_data.fd;
        handle->ion_sbuffer = v_addr;
        handle->ion_alloc_handle.handle = ion_alloc_data.handle;
        handle->sbuf_len = size;
        break;
    } while (0);  // do...

    LOG_D("[%s] exit", __func__);
    return ret;
}

/**
 * Function: qsc_ion_dealloc
 * Description: deallocate memory for qsc_ion
 * Input: handle
 * Output: none
 * Return:
 * 0       if succeed to deallocate memory for qsc_ion
 * others  if fail to deallocate memory for qsc_ion
 */
int32_t qsc_ion_dealloc(struct qsc_ion_info *handle)
{
    struct ion_handle_data handle_data;
    int32_t ret = 0;
    LOG_D("[%s] enter", __func__);

    do
    {
        /* Deallocate the memory for the listener */
        ret = munmap(handle->ion_sbuffer,
                     (handle->sbuf_len + 4095) & (~4095));

        if (ret)
        {
            LOG_E("[%s] Error::Unmapping ION Buffer failed with ret = %d\n",
                  __func__, ret);
        }

        handle_data.handle = handle->ion_alloc_handle.handle;
        close(handle->ifd_data_fd);
        ret = ioctl(handle->ion_fd, ION_IOC_FREE, &handle_data);

        if (ret)
        {
            LOG_E("[%s] Error::ION Memory FREE ioctl failed with ret = %d\n",
                  __func__, ret);
        }

        close(handle->ion_fd);
    }
    while (0);

    LOG_D("[%s] exit", __func__);
    return ret;
}

#else  // #if TARGET_ION_ABI_VERSION < 2
/* uses the new version of ION */

/**
 * Function: qsc_ion_memalloc
 * Description: allocate memory for qsc_ion
 * Input: handle, size
 * Output: none
 * Return:
 * 0       if succeed to allocate memory for qsc_ion
 * others  if fail to allocate memory for qsc_ion
 */
int32_t qsc_ion_memalloc(struct qsc_ion_info *handle,
                uint32_t size)
{
    int32_t ret = 0;
    uint8_t *v_addr = NULL;
    int32_t ion_fd = -1;
    int32_t map_fd = -1;
    int32_t retry = 0;
    uint32_t len = (size + 4095) & (~4095);
    uint32_t align = 0;
    uint32_t flags = 0;
    dma_buf_sync_t buf_sync;
    uint32_t heap_id = ION_QSECOM_HEAP_ID;
    LOG_D("[%s] enter", __func__);

    do
    {
        ion_fd  = ion_open();
        if (ion_fd < 0)
        {
            LOG_E("Error::Cannot open ION device\n");
            ret = -1;
            break;
        }

        ret = ion_alloc_fd(ion_fd, len, align, ION_HEAP(heap_id), flags, &map_fd);
        if (ret)
        {
            LOG_E("Error::ion_alloc_fd for heap %u size %u failed ret = %d, errno = %d\n",
                heap_id, size, ret, errno);
            ret = -1;
            break;
        }

        v_addr = (unsigned char *)mmap(NULL, len, PROT_READ | PROT_WRITE,
                        MAP_SHARED, map_fd, 0);
        if (v_addr == MAP_FAILED)
        {
            LOG_E("Error::mmap failed\n");
            ret = -1;
            break;
        }

        handle->ion_fd = ion_fd;
        handle->ifd_data_fd = map_fd;
        handle->ion_sbuffer = v_addr;
        handle->sbuf_len = size;

        buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
        ret = ioctl(map_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
        if (ret)
        {
            LOG_E("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d\n",
                ret, errno);
            ret = -1;
            break;
        }
    }  // end do
    while (0);

    if (-1 == ret)
    {
        if (v_addr)
        {
            munmap(v_addr, len);
            handle->ion_sbuffer = NULL;
        }
        if (map_fd >= 0)
        {
            ion_close(map_fd);
            handle->ifd_data_fd = -1;
        }
        if (ion_fd >= 0)
        {
            ion_close(ion_fd);
            handle->ion_fd = -1;
        }
    }

    LOG_D("[%s] exit", __func__);
    return ret;
}

/**
 * Function: qsc_ion_dealloc
 * Description: deallocate memory for qsc_ion
 * Input: handle
 * Output: none
 * Return:
 * 0       if succeed to deallocate memory for qsc_ion
 * others  if fail to deallocate memory for qsc_ion
 */
int32_t qsc_ion_dealloc(struct qsc_ion_info *handle)
{
    dma_buf_sync_t buf_sync;
    uint32_t len = (handle->sbuf_len + 4095) & (~4095);
    int32_t ret = 0;
    LOG_D("[%s] enter", __func__);

    buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
    ret = ioctl(handle->ifd_data_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (ret)
    {
        LOG_E("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d\n",
                ret, errno);
    }

    if (handle->ion_sbuffer)
    {
        munmap(handle->ion_sbuffer, len);
        handle->ion_sbuffer = NULL;
    }
    if (handle->ifd_data_fd >= 0)
    {
        ion_close(handle->ifd_data_fd);
        handle->ifd_data_fd = -1;
    }
    if (handle->ion_fd >= 0)
    {
        ion_close(handle->ion_fd);
        handle->ion_fd = -1;
    }
    LOG_D("[%s] exit", __func__);
    return ret;
}
#endif  // #if TARGET_ION_ABI_VERSION < 2

