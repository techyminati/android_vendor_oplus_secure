/******************************************************************************
 * @file   QSEEComFunc.c
 * @brief  Contains QSEE communication functions.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Willian Kin 2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "QSEE_WRAPPER"
#include "log/logmsg.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <ion/ion.h>

#include "qsee/QSEEComFunc.h"

#define ION_QSECOM_HEAP_ID 27

#ifndef ION_HEAP
#define ION_HEAP(bit) (1 << (bit))
#endif

#define QSEE_LIBRARY "libQSEEComAPI.so"

#ifndef SIL_QSEE_ION_BUF_V1_DISABLED
#define SIL_QSEE_ION_BUF_V1_DISABLED 0
#endif
#ifndef SIL_QSEE_ION_BUF_V2_DISABLED
#define SIL_QSEE_ION_BUF_V2_DISABLED 0
#endif

#if (SIL_QSEE_ION_BUF_V1_DISABLED) && (SIL_QSEE_ION_BUF_V2_DISABLED)
#error "SIL_QSEE_ION_BUF_V1_DISABLED & SIL_QSEE_ION_BUF_V2_DISABLED just only one can be defined at most."
#endif

#if !SIL_QSEE_ION_BUF_V2_DISABLED
#include <linux/dma-buf.h>
#endif

static int32_t m_ion_version_v2 = 1;

typedef struct {
    void *libHandle;
} _priv_data_t;

static int32_t qcom_km_ion_dealloc(qcom_km_ion_info_t *handle);
static int32_t qcom_km_ion_memalloc(qcom_km_ion_info_t *handle, uint32_t size);
static int32_t qsee_load_trustlet(qsee_handle_t *qsee_handle, struct QSEECom_handle **clnt_handle,
                              const char *path, const char *fname,uint32_t sb_size);

#define DECLARE_LIB_FUNC(x, sx, mandatory) \
    do { \
        handle->x = dlsym(data->libHandle, sx); \
        if (mandatory && (handle->x == NULL)) { \
            LOG_MSG_DEBUG("Loading %s failed (%d:%s)", sx, errno, strerror(errno)); \
            goto exit; \
        } \
    } while(0)

int32_t qsee_open_handle(qsee_handle_t **ret_handle)
{
    qsee_handle_t *handle = NULL;
    _priv_data_t *data = NULL;

    LOG_MSG_VERBOSE("Using Target Lib : %s", QSEE_LIBRARY);
    data = (_priv_data_t*)malloc(sizeof(_priv_data_t));
    if (data == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    data->libHandle = dlopen(QSEE_LIBRARY, RTLD_NOW);
    if (data->libHandle == NULL) {
        LOG_MSG_DEBUG("load QSEECom API library failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }
    LOG_MSG_VERBOSE("QSEECom API library successed (%p)", data->libHandle);

    handle = (qsee_handle_t*)malloc(sizeof(qsee_handle_t));
    if(handle == NULL) {
        LOG_MSG_DEBUG("Allocating memory failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->_data = data;

    // Setup internal functions
    handle->QCom_ion_alloc = qcom_km_ion_memalloc;
    handle->QCom_ion_free = qcom_km_ion_dealloc;
    handle->QSEECom_load_trustlet = qsee_load_trustlet;

    // Setup QSEECom Functions
    DECLARE_LIB_FUNC(QSEECom_start_app, "QSEECom_start_app", 1);
    DECLARE_LIB_FUNC(QSEECom_shutdown_app, "QSEECom_shutdown_app", 1);
    DECLARE_LIB_FUNC(QSEECom_load_external_elf, "QSEECom_load_external_elf", 1);
    DECLARE_LIB_FUNC(QSEECom_unload_external_elf, "QSEECom_unload_external_elf", 1);
    DECLARE_LIB_FUNC(QSEECom_register_listener, "QSEECom_register_listener", 1);
    DECLARE_LIB_FUNC(QSEECom_unregister_listener, "QSEECom_unregister_listener", 1);
    DECLARE_LIB_FUNC(QSEECom_send_cmd, "QSEECom_send_cmd", 1);
    DECLARE_LIB_FUNC(QSEECom_send_modified_cmd_32, "QSEECom_send_modified_cmd", 1);
    DECLARE_LIB_FUNC(QSEECom_receive_req, "QSEECom_receive_req", 1);
    DECLARE_LIB_FUNC(QSEECom_send_resp, "QSEECom_send_resp", 1);
    DECLARE_LIB_FUNC(QSEECom_set_bandwidth, "QSEECom_set_bandwidth", 1);
    DECLARE_LIB_FUNC(QSEECom_app_load_query, "QSEECom_app_load_query", 1);

    DECLARE_LIB_FUNC(QSEECom_get_app_info, "QSEECom_get_app_info", 0);
    DECLARE_LIB_FUNC(QSEECom_send_modified_cmd_64, "QSEECom_send_modified_cmd_64", 0);
    DECLARE_LIB_FUNC(QSEECom_start_app_V2, "QSEECom_start_app_V2", 0);

    if (ret_handle) {
        *ret_handle = handle;
        return 0;
    }

exit:
    if (handle != NULL) {
        free(handle);
        handle = NULL;
    }
    if (data != NULL) {
        if(data->libHandle != NULL) {
            dlclose(data->libHandle);
            data->libHandle = NULL;
        }
        free(data);
        data = NULL;
    }
    return -1;
}

int32_t qsee_free_handle(qsee_handle_t **handle_ptr)
{
    _priv_data_t *data = NULL;
    qsee_handle_t *handle = NULL;

    if ((handle_ptr != NULL) && (*handle_ptr != NULL)) {
        handle = *handle_ptr;
        data = (_priv_data_t*)handle->_data;

        if (data != NULL) {
            if(data->libHandle != NULL) {
                dlclose(data->libHandle);
                data->libHandle = NULL;
            }
            free(data);
            data = NULL;
        }
        if (handle != NULL) {
            free(handle);
            handle = NULL;
        }
        *handle_ptr = NULL;
    }

    return 0;
}

#if !SIL_QSEE_ION_BUF_V2_DISABLED
static int32_t qcom_km_ion_dealloc_v2(qcom_km_ion_info_t *handle)
{
    int32_t ret = 0;
    struct dma_buf_sync buf_sync;
    uint32_t len = (handle->sbuf_len + 4095) & (~4095);

    LOG_MSG_DEBUG("use ion ver v2");

    if (handle == NULL) {
        return -1;
    }

    if (handle->ion_alloc_handle.handle) {
        buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
        ret = ioctl(handle->ifd_data_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
        if (ret < 0) {
            LOG_MSG_ERROR("DMA SYNC end failed, %d (%d:%s)", ret, errno, strerror(errno));
        }
        handle->ion_alloc_handle.handle = 0;
    }

    if (handle->ion_sbuffer) {
        munmap(handle->ion_sbuffer, len);
        handle->ion_sbuffer = NULL;
    }

    if (handle->ifd_data_fd >= 0) {
        ion_close(handle->ifd_data_fd);
        handle->ifd_data_fd = -1;
    }

    if (handle->ion_fd >= 0) {
        ion_close(handle->ion_fd);
        handle->ion_fd = -1;
    }

    return ret;
}

static int32_t qcom_km_ion_memalloc_v2(qcom_km_ion_info_t *handle, uint32_t size)
{
    int32_t ret = 0;
    unsigned char *v_addr = NULL;
    int32_t ion_fd = -1;
    int32_t map_fd = -1;
    uint32_t len = (size + 4095) & (~4095);
    uint32_t align = 0;
    struct dma_buf_sync buf_sync;

    LOG_MSG_DEBUG("use ion ver v2");

    if (handle == NULL) {
        LOG_MSG_ERROR("null handle");
        return -1;
    }

    memset(handle, 0, sizeof(qcom_km_ion_info_t));

    ion_fd = ion_open();
    if (ion_fd < 0) {
        LOG_MSG_ERROR("Cannot open ION device");
        return -1;
    }
    handle->ion_fd = ion_fd;

    ret = ion_alloc_fd(ion_fd, len, align, ION_HEAP(ION_QSECOM_HEAP_ID), 0, &map_fd);
    if (ret < 0) {
        LOG_MSG_ERROR("ion_alloc_fd size %u failed, %d (%d:%s)", size, ret, errno, strerror(errno));
        goto exit;
    }
    handle->ifd_data_fd = map_fd;

    v_addr = (unsigned char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, map_fd, 0);
    if (v_addr == MAP_FAILED) {
        LOG_MSG_ERROR("ION MMAP failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }
    handle->ion_sbuffer = v_addr;
    handle->sbuf_len = size;

    buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    ret = ioctl(map_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (ret < 0) {
        LOG_MSG_ERROR("DMA SYNC start failed, %d (%d:%s)", ret, errno, strerror(errno));
        goto exit;
    }

    handle->ion_alloc_handle.handle = 1;

    return 0;

exit:
    qcom_km_ion_dealloc_v2(handle);
    return -1;
}
#endif

#if !SIL_QSEE_ION_BUF_V1_DISABLED
static int32_t qcom_km_ion_dealloc_v1(qcom_km_ion_info_t *handle)
{
    int32_t ret = 0;
    struct ion_handle_data handle_data;

    LOG_MSG_DEBUG("use ion ver v1");

    if (handle == NULL) {
        return -1;
    }

    /* Deallocate the memory for the listener */
    if (handle->ion_sbuffer != NULL) {
        ret = munmap(handle->ion_sbuffer, (handle->sbuf_len + 4095) & (~4095));
        if (ret < 0) {
            LOG_MSG_ERROR("ION unmap failed %d (%d:%s)", ret, errno, strerror(errno));
        }
        handle->ion_sbuffer = NULL;
    }

    if (handle->ifd_data_fd > 0) {
        close(handle->ifd_data_fd);
        handle->ifd_data_fd = -1;
    }

    handle_data.handle = handle->ion_alloc_handle.handle;
    if (handle_data.handle) {
        ret = ioctl(handle->ion_fd, ION_IOC_FREE, &handle_data);
        if (ret < 0) {
            LOG_MSG_ERROR("ION FREE failed %d (%d:%s)", ret, errno, strerror(errno));
        }
        handle->ion_alloc_handle.handle = 0;
    }

    if (handle->ion_fd > 0) {
        close(handle->ion_fd);
        handle->ion_fd = -1;
    }

    LOG_MSG_VERBOSE("dealloc");
    return ret;
}

static int32_t qcom_km_ion_memalloc_v1(qcom_km_ion_info_t *handle, uint32_t size)
{
    int32_t ret = 0;
    int32_t ion_fd;
    struct ion_allocation_data ion_alloc_data;
    struct ion_fd_data ifd_data;
    unsigned char *v_addr;

    LOG_MSG_DEBUG("use ion ver v1");

    if (handle == NULL) {
        LOG_MSG_ERROR("null handle");
        return -1;
    }

    memset(handle, 0, sizeof(qcom_km_ion_info_t));

    /* open ION device for memory management, O_DSYNC -> uncached memory */
    ion_fd = open("/dev/ion", O_RDONLY | O_DSYNC);
    if (ion_fd < 0) {
        LOG_MSG_ERROR("Cannot open ION device (%d:%s)", errno, strerror(errno));
        return -1;
    }

    handle->ion_fd = ion_fd;

    /* Size of allocation */
    ion_alloc_data.len = (size + 4095) & (~4095);
    /* 4K aligned */
    ion_alloc_data.align = 4096;
    /* memory is allocated from EBI heap */
    ion_alloc_data.heap_id_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
    /* Set the memory to be uncached */
    ion_alloc_data.flags = 0;
    /* IOCTL call to ION for memory request */
    ret = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);
    if (ret < 0) {
        LOG_MSG_ERROR("ION ALLOC failed %d (%d:%s)", ret, errno, strerror(errno));
        goto exit;
    }

    if (ion_alloc_data.handle == 0) {
        LOG_MSG_ERROR("ION alloc data returned a NULL");
        goto exit;
    }

    ifd_data.handle = ion_alloc_data.handle;
    handle->ion_alloc_handle.handle = ion_alloc_data.handle;
    LOG_MSG_VERBOSE("handle=%d", handle->ion_alloc_handle.handle);

    /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
    ret = ioctl(ion_fd, ION_IOC_MAP, &ifd_data);
    if (ret < 0) {
        LOG_MSG_ERROR("ION MAP failed %d (%d:%s)", ret, errno, strerror(errno));
        goto exit;
    }

    handle->ifd_data_fd = ifd_data.fd;

    /* Make the ion mmap call */
    v_addr = (unsigned char *)mmap(NULL, ion_alloc_data.len, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, ifd_data.fd, 0);
    if (v_addr == MAP_FAILED) {
        LOG_MSG_ERROR("ION MMAP failed (%d:%s)", errno, strerror(errno));
        goto exit;
    }

    handle->ion_sbuffer = v_addr;
    handle->sbuf_len = size;

    return 0;

exit:
    qcom_km_ion_dealloc_v1(handle);
    return -1;
}
#endif

static int32_t qcom_km_ion_dealloc(qcom_km_ion_info_t *handle)
{
    int32_t ret = 0;

#if SIL_QSEE_ION_BUF_V2_DISABLED
    ret = qcom_km_ion_dealloc_v1(handle);
#elif SIL_QSEE_ION_BUF_V1_DISABLED
    ret = qcom_km_ion_dealloc_v2(handle);
#else
    if (m_ion_version_v2) {
        ret = qcom_km_ion_dealloc_v2(handle);
    } else {
        ret = qcom_km_ion_dealloc_v1(handle);
    }
#endif

    return ret;
}

static int32_t qcom_km_ion_memalloc(qcom_km_ion_info_t *handle, uint32_t size)
{
    int32_t ret = 0;

#if SIL_QSEE_ION_BUF_V2_DISABLED
    m_ion_version_v2 = 0;
    ret = qcom_km_ion_memalloc_v1(handle, size);
#elif SIL_QSEE_ION_BUF_V1_DISABLED
    m_ion_version_v2 = 1;
    ret = qcom_km_ion_memalloc_v2(handle, size);
#else
    if (m_ion_version_v2) {
        ret = qcom_km_ion_memalloc_v2(handle, size);
    } else {
        ret = qcom_km_ion_memalloc_v1(handle, size);
    }

    if (ret < 0) {
        m_ion_version_v2 = !m_ion_version_v2;

        LOG_MSG_DEBUG("retry ion ver %s", m_ion_version_v2 ? "v2" : "v1");
        if (m_ion_version_v2) {
            ret = qcom_km_ion_memalloc_v2(handle, size);
        } else {
            ret = qcom_km_ion_memalloc_v1(handle, size);
        }
    }
#endif

    return ret;
}

const char* qsee_error_strings(int32_t err)
{
    switch (err) {
    case QSEECOM_LISTENER_REGISTER_FAIL:
        return "QSEECom: Failed to register listener";
    case QSEECOM_LISTENER_ALREADY_REGISTERED:
        return "QSEECom: Listener already registered";
    case QSEECOM_LISTENER_UNREGISTERED:
        return "QSEECom: Listener unregistered";
    case QSEECOM_APP_ALREADY_LOADED:
        return "QSEECom: Trustlet already loaded";
    case QSEECOM_APP_NOT_LOADED:
        return "QSEECom: Trustlet not loaded";
    case QSEECOM_APP_QUERY_FAILED:
        return "QSEECom: Failed to query trustlet";
    default:
        return strerror(errno);
    }
}

static int32_t qsee_load_trustlet(qsee_handle_t *qsee_handle, struct QSEECom_handle **clnt_handle,
                                  const char *path, const char *fname, uint32_t sb_size)
{
    int32_t ret = 0;
    int32_t sz = sb_size;

    if (qsee_handle == NULL || clnt_handle == NULL || path == NULL || fname == NULL || strlen(fname) == 0) {
        return -1;
    }

    if (sz < 1024) {
        LOG_MSG_DEBUG("Warning: size too small, increasing");
        sz = 1024;
    }

    LOG_MSG_VERBOSE("Starting app %s/%s", path, fname);
    ret = qsee_handle->QSEECom_start_app(clnt_handle, path, fname, sz);
    if (ret < 0) {
        LOG_MSG_DEBUG("Could not load app %s/%s (%d:%d:%s)", path, fname, ret, errno, qsee_error_strings(errno));
    } else {
        LOG_MSG_VERBOSE("TZ App loaded : %s/%s", path, fname);
    }

    return ret;
}