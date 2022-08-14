#include <android/log.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include "../../../../../../../../../../kernel/msm-4.19/include/uapi/linux/msm_ion.h"
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/Log.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#include <linux/dma-buf.h>
#endif
#include "QSEEComAPI.h"
#include "fp_client_lib.h"

/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "QSEECOM_FP_LIB_CLIENT: "
#ifdef LOG_NDDEBUG
#undef LOG_NDDEBUG
#endif
#define LOG_NDDEBUG 0 //Define to enable LOGD
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG  0 //Define to enable LOGV
#define NON_LEGACY_CMD_LINE_ARGS 1

#define RUN_TESTS 0
#define ION_BUFFER_LENGTH (64*1024)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "RBS_TZ_CLIENT",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "RBS_TZ_CLIENT",__VA_ARGS__)
#define IN
#define OUT

#define KEYMASTER_UTILS_CMD_ID 0x200UL

struct input_message {
    uint32_t cmd_id;
    uint32_t data;
    uint32_t data2;
    uint32_t len;
    uint32_t start_pkt;
    uint32_t end_pkt;
    uint32_t test_buf_size;
    unsigned char buffer[64];
    uint8_t egis_data[2048];
};

struct response_message {
    uint32_t data;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t result_status;
    uint8_t egis_data[2048];
};

struct qsc_ion_info {
    int32_t ion_fd;
    int32_t ifd_data_fd;
#ifndef TARGET_ION_ABI_VERSION
    struct ion_handle_data ion_alloc_handle;
#endif
    unsigned char* ion_sbuffer;
    uint32_t sbuf_len;
};

typedef enum {
    KEYMASTER_NONE = 0,
    KEYMASTER_GET_AUTH_TOKEN_KEY = KEYMASTER_UTILS_CMD_ID + 5ULL,
    KEYMASTER_ANY = (int)UINT32_MAX,
} keymaster_cmd_t;

typedef enum {
    HW_AUTH_NONE = 0,
    HW_AUTH_PASSWORD = 1 << 0,
    HW_AUTH_FINGERPRINT = 1 << 1,
    HW_AUTH_ANY = UINT32_MAX,
} hw_authenticator_type_t;

typedef struct _km_get_auth_token_req_t {
    keymaster_cmd_t cmd_id;
    hw_authenticator_type_t auth_type;
} __attribute__((packed)) km_get_auth_token_req_t;

typedef struct _km_get_auth_token_rsp_t {
    int status;
    uint32_t auth_token_key_offset;
    uint32_t auth_token_key_len;
} __attribute__((packed)) km_get_auth_token_rsp_t;

pthread_mutex_t plock;
pthread_t mThread;

struct egis_client_transfer_package {
	uint32_t process;
	uint32_t command;
	uint32_t uid;
	uint32_t fid;
	uint32_t in_data_len;
	uint8_t in_data[0];
};

#if TARGET_ION_ABI_VERSION >= 2
/* uses the new version of ION */

static int32_t qsc_ION_memalloc(struct qsc_ion_info* handle, uint32_t size) {
    int32_t ret = 0;
    unsigned char* v_addr;
    int32_t ion_fd = -1;
    int32_t map_fd = -1;
    uint32_t len = (size + 4095) & (~4095);
    uint32_t align = 0;
    uint32_t flags = 0;
    struct dma_buf_sync buf_sync;
    unsigned int heap_id = ION_QSECOM_HEAP_ID;

    ion_fd = ion_open();
    if (ion_fd < 0) {
        LOGE("Error::Cannot open ION device\n");
        return -1;
    }

    ret = ion_alloc_fd(ion_fd, len, align, ION_HEAP(heap_id), flags, &map_fd);
    if (ret) {
        LOGE("Error::ion_alloc_fd for heap %u size %u failed ret = %d, errno = %d\n", heap_id, size,
             ret, errno);
        ret = -1;
        goto alloc_fail;
    }

    v_addr = (unsigned char*)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, map_fd, 0);
    if (v_addr == MAP_FAILED) {
        LOGE("Error::mmap failed\n");
        ret = -1;
        goto map_fail;
    }

    handle->ion_fd = ion_fd;
    handle->ifd_data_fd = map_fd;
    handle->ion_sbuffer = v_addr;
    handle->sbuf_len = size;

    buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    ret = ioctl(map_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (ret) {
        LOGE("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d\n", ret, errno);
        ret = -1;
        goto sync_fail;
    }

    return ret;

sync_fail:
    if (v_addr) {
        munmap(v_addr, len);
        handle->ion_sbuffer = NULL;
    }
map_fail:
    if (map_fd >= 0) {
        ion_close(map_fd);
        handle->ifd_data_fd = -1;
    }
alloc_fail:
    if (ion_fd >= 0) {
        ion_close(ion_fd);
        handle->ion_fd = -1;
    }
    return ret;
}

static int qsc_ion_dealloc(struct qsc_ion_info* handle) {
    struct dma_buf_sync buf_sync;
    uint32_t len = (handle->sbuf_len + 4095) & (~4095);
    int ret = 0;

    buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
    ret = ioctl(handle->ifd_data_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (ret) {
        LOGE("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d, errno = %d\n", ret, errno);
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
#endif

#ifndef TARGET_ION_ABI_VERSION
static int32_t qsc_ION_memalloc(struct qsc_ion_info* handle, uint32_t size) {
    int32_t ret = 0;
    int32_t iret = 0;
    unsigned char* v_addr;
    struct ion_allocation_data ion_alloc_data;
    int32_t ion_fd;
    struct ion_fd_data ifd_data;
    struct ion_handle_data handle_data;

    /* open ION device for memory management
     * O_DSYNC -> uncached memory
     */

    if (handle == NULL) {
        LOGE("Error:: null handle received");
        return -1;
    }
    ion_fd = open("/dev/ion", O_RDONLY);
    if (ion_fd < 0) {
        LOGE("Error::Cannot open ION device\n");
        return -1;
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
    ret = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);
    if (ret) {
        LOGE("Error::Error while trying to allocate data\n");
        goto alloc_fail;
    }

    if (ion_alloc_data.handle != NULL) {
        ifd_data.handle = ion_alloc_data.handle;
    } else {
        LOGE("Error::ION alloc data returned a NULL\n");
        goto alloc_fail;
    }
    /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
    ret = ioctl(ion_fd, ION_IOC_MAP, &ifd_data);
    if (ret) {
        LOGE("Error::Failed doing ION_IOC_MAP call\n");
        goto ioctl_fail;
    }

    /* Make the ion mmap call */
    v_addr = (unsigned char*)mmap(NULL, ion_alloc_data.len, PROT_READ | PROT_WRITE, MAP_SHARED,
                                  ifd_data.fd, 0);
    if (v_addr == MAP_FAILED) {
        LOGE("Error::ION MMAP failed\n");
        ret = -1;
        goto map_fail;
    }
    handle->ion_fd = ion_fd;
    handle->ifd_data_fd = ifd_data.fd;
    handle->ion_sbuffer = v_addr;
    handle->ion_alloc_handle.handle = ion_alloc_data.handle;
    handle->sbuf_len = size;
    return ret;

map_fail:
    if (handle->ion_sbuffer != NULL) {
        ret = munmap(handle->ion_sbuffer, ion_alloc_data.len);
        if (ret) LOGE("Error::Failed to unmap memory for load image. ret = %d\n", ret);
    }

ioctl_fail:
    handle_data.handle = ion_alloc_data.handle;
    if (handle->ifd_data_fd) close(handle->ifd_data_fd);
    iret = ioctl(ion_fd, ION_IOC_FREE, &handle_data);
    if (iret) {
        LOGE("Error::ION FREE ioctl returned error = %d\n", iret);
    }

alloc_fail:
    if (ion_fd) close(ion_fd);
    return ret;
}

#endif

int32_t qsc_start_app(EGIS_TLC_HANDLE_T** p_tlcComHandle, const char* pStrAppname,
                      int32_t nBufSize) {
    int32_t nRet = 0;
    // const char* default_path = "/vendor/firmware_mnt/image";   //   "/vendor/firmware"
    const char* default_path = "/odm/vendor/firmware";   //   "/vendor/firmware"
    const char* other_path = "/vendor/firmware";

    EGIS_TLC_HANDLE_T* tlc_handle;

    if (p_tlcComHandle == NULL) {
        LOGE("p_tlcComHandle is NULL");
        return -1;
    }

    tlc_handle = (EGIS_TLC_HANDLE_T*)malloc(sizeof(EGIS_TLC_HANDLE_T));
    tlc_handle->handle = NULL;
    tlc_handle->handle_size = 0;

    LOGD("-- first, qsc_start_app Loading default path: %s", default_path);
    nRet = QSEECom_start_app((struct QSEECom_handle**)&tlc_handle->handle, default_path,
                             pStrAppname, nBufSize);

    if (nRet) {
        LOGD("Loading app %s -%s failed", default_path, pStrAppname);
        LOGD("-- second, qsc_start_app Loading other path: %s", other_path);
        nRet = QSEECom_start_app((struct QSEECom_handle**)&tlc_handle->handle, other_path,
                                 pStrAppname, nBufSize);
    }

    if (nRet) {
        LOGE("Loading app %s -%s failed", other_path, pStrAppname);
        free(tlc_handle);
    } else {
        tlc_handle->handle_size = sizeof(struct QSEECom_handle);
        *p_tlcComHandle = tlc_handle;
    }
    return nRet;
}

int32_t qsc_shutdown_app(EGIS_TLC_HANDLE_T** p_tlcComHandle) {
    int32_t nRet = 0;
    EGIS_TLC_HANDLE_T* tlc_handle = NULL;

    if (*p_tlcComHandle == NULL) {
        nRet = -1;
        goto error;
    }

    tlc_handle = *p_tlcComHandle;
    if (tlc_handle->handle == NULL) {
        goto tlc_terminate;
    }
    nRet = QSEECom_shutdown_app((struct QSEECom_handle**)&tlc_handle->handle);
    if (nRet) {
        LOGE("Shutdown app failed with ret = %d", nRet);
    } else {
        LOGD("shutdown app: pass");
    }

tlc_terminate:
    free(tlc_handle);
    *p_tlcComHandle = NULL;
    return nRet;
error:
    LOGE("cannot shutdown as the handle is NULL");
    return nRet;
}

#ifndef TARGET_ION_ABI_VERSION
/** @brief: Deallocate ION memory
 *
 *
 */
static int32_t qsc_ion_dealloc(struct qsc_ion_info* handle) {
    struct ion_handle_data handle_data;
    int32_t ret = 0;

    /* Deallocate the memory for the listener */
    ret = munmap(handle->ion_sbuffer, (handle->sbuf_len + 4095) & (~4095));
    if (ret) {
        LOGE("Error::Unmapping ION Buffer failed with ret = %d\n", ret);
    }

    handle_data.handle = handle->ion_alloc_handle.handle;
    close(handle->ifd_data_fd);
    ret = ioctl(handle->ion_fd, ION_IOC_FREE, &handle_data);
    if (ret) {
        LOGE("Error::ION Memory FREE ioctl failed with ret = %d\n", ret);
    }
    close(handle->ion_fd);

    return ret;
}
#endif

int32_t ets_issue_send_modified_cmd_req(EGIS_TLC_HANDLE_T* l_tlcComHandle,
                                        unsigned char* req_tci_data, int req_tci_size,
                                        unsigned char* rsp_tci_data, int* rsp_tci_size) {
    int32_t nRet = 0;
    int32_t req_len = 0;
    int32_t rsp_len = 0;

    struct input_message* msg_req;
    struct response_message* msg_rsp;
    struct QSEECom_handle* l_QSEEComHandle;

    LOGD("ets ,send modified cmd: start");

    l_QSEEComHandle = l_tlcComHandle->handle;
    /* Populate the send data structure */
    msg_req = (struct input_message*)l_QSEEComHandle->ion_sbuffer;

    memset(msg_req->egis_data, 0x0, req_tci_size);
    memcpy(msg_req->egis_data, req_tci_data, req_tci_size);
    msg_req->cmd_id = CLIENT_CMD_ETS_MESSAGE;
    msg_req->data = 0;
    msg_req->len = 0;
    msg_req->start_pkt = 0;
    msg_req->end_pkt = 0;
    msg_req->test_buf_size = 0;

    req_len = sizeof(struct input_message);
    rsp_len = sizeof(struct response_message);

    if (req_len & QSEECOM_ALIGN_MASK) req_len = QSEECOM_ALIGN(req_len);

    if (rsp_len & QSEECOM_ALIGN_MASK) rsp_len = QSEECOM_ALIGN(rsp_len);

    //msg_rsp = (struct response_message*)l_QSEEComHandle->ion_sbuffer;
	msg_rsp = (struct response_message *)(l_QSEEComHandle->ion_sbuffer + req_len);

    /* send request from HLOS to QSEApp */
    nRet = QSEECom_send_cmd(l_QSEEComHandle, msg_req, req_len, msg_rsp, rsp_len);

    if (nRet) {
        LOGE("qsc_issue_send_modified_cmd_req: fail cmd = %d ret = %d", msg_req->cmd_id, nRet);
    }

    memcpy(rsp_tci_data, msg_rsp->egis_data, req_tci_size);

    if (rsp_tci_size) *rsp_tci_size = req_tci_size;

    return nRet;
}
int32_t ets_qsee_cmd_req_in_data(EGIS_TLC_HANDLE_T* l_tlcComHandle, unsigned char* req_tci_data,
                                 int req_tci_size, unsigned char* rsp_tci_data, int* rsp_tci_size,
                                 unsigned char* in_data, unsigned int in_data_len) {
    int32_t nRet = 0;
    int32_t req_len = 0;
    int32_t rsp_len = 0;
    uint32_t process_id = 0;
    uint32_t command_id = 0;
    struct qsc_ion_info ihandle;
    struct QSEECom_ion_fd_info ion_fd_info;
    struct input_message* msg_req;
    struct response_message* msg_rsp;
    struct egis_client_transfer_package* egis_package_meg;
    egis_package_meg = (struct egis_client_transfer_package*)req_tci_data;
    process_id = egis_package_meg->process;
    command_id = egis_package_meg->command;
    struct QSEECom_handle* l_QSEEComHandle;
    l_QSEEComHandle = l_tlcComHandle->handle;

    LOGD("-- ets ,send process_id: %d", process_id);
    LOGD("-- ets ,send command_id: %d", command_id);
    LOGD("-- ets ,send req_tci_size: %d", req_tci_size);
    LOGD("ets_qsee_cmd_req_in_data in_data_len:%d", in_data_len);

    if (in_data != NULL && in_data_len > 0) {
        LOGD("ets ,send modified cmd [buffer]: start");
        /* allocate in_data_len bytes memory */
        ihandle.ion_fd = 0;
#ifndef TARGET_ION_ABI_VERSION
        ihandle.ion_alloc_handle.handle = 0;
#endif
        nRet = qsc_ION_memalloc(&ihandle, in_data_len);
        if (nRet) {
            LOGE("Error allocating memory in ion\n");
            return -1;
        }
    } else
        LOGD("ets ,send modified cmd: start");

    LOGD("ets_qsee_cmd_req_in_data 11");
    memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));

    /* Populate the send data structure */
    msg_req = (struct input_message*)l_QSEEComHandle->ion_sbuffer;

    LOGD("ets_qsee_cmd_req_in_data req_tci_size:%d", req_tci_size);
    memset(msg_req->egis_data, 0x0, req_tci_size);
    memcpy(msg_req->egis_data, req_tci_data, req_tci_size);

    msg_req->cmd_id = process_id;
    msg_req->start_pkt = 0;
    msg_req->end_pkt = 0;
    msg_req->test_buf_size = 0;
    LOGD("-- ets ,cmd_id: %d", msg_req->cmd_id);

    if (in_data != NULL && in_data_len > 0) {
        msg_req->len = in_data_len;
        msg_req->buffer[0] = 0x5A;
        ion_fd_info.data[0].fd = ihandle.ifd_data_fd;
        ion_fd_info.data[0].cmd_buf_offset = sizeof(uint32_t);
        ion_fd_info.data[1].fd = ihandle.ifd_data_fd;
        ion_fd_info.data[1].cmd_buf_offset = sizeof(uint32_t) * 2;
        if (ihandle.ion_sbuffer == NULL) {
            LOGD("ets_qsee_cmd_req_in_data ihandle.ion_sbuffer == NULL");
        } else {
            LOGD("ets_qsee_cmd_req_in_data ihandle.ion_sbuffer != NULL");
            memcpy(ihandle.ion_sbuffer, in_data, in_data_len);
        }
    } else {
        msg_req->data = 0;
        msg_req->len = 0;
    }

    req_len = sizeof(struct input_message);
    rsp_len = sizeof(struct response_message);

    if (req_len & QSEECOM_ALIGN_MASK) req_len = QSEECOM_ALIGN(req_len);

    if (rsp_len & QSEECOM_ALIGN_MASK) rsp_len = QSEECOM_ALIGN(rsp_len);

    //msg_rsp = (struct response_message*)l_QSEEComHandle->ion_sbuffer;
	msg_rsp = (struct response_message*)(l_QSEEComHandle->ion_sbuffer + req_len);

    nRet = QSEECom_send_modified_cmd(l_QSEEComHandle, msg_req, req_len, msg_rsp, rsp_len,
                                     &ion_fd_info);
    if (nRet) {
        LOGE("%s: fail cmd = %d, ret = %d !!!", __func__, msg_req->cmd_id, nRet);
    }

    LOGD("%s: cmd = %d, result_status = %d", __func__, msg_req->cmd_id, msg_rsp->result_status);

    memcpy(rsp_tci_data, msg_rsp->egis_data, req_tci_size);

    if (rsp_tci_size) *rsp_tci_size = req_tci_size;

    if (in_data != NULL && in_data_len > 0) {
        memcpy(in_data, ihandle.ion_sbuffer, in_data_len);
        /* De-allocate 64KB memory */
        nRet = qsc_ion_dealloc(&ihandle);
        if (nRet) {
            LOGE("ets_issue_send_modified_cmd_req_in_data:fail dealloc is %d\n", nRet);
        }
    }
    return (nRet ? nRet : msg_rsp->result_status);
}

int32_t ets_issue_send_modified_cmd_req_in_data(EGIS_TLC_HANDLE_T* l_tlcComHandle,
                                                unsigned char* req_tci_data, int req_tci_size,
                                                unsigned char* rsp_tci_data, int* rsp_tci_size,
                                                unsigned char* in_data, unsigned int in_data_len) {
    int32_t nRet = 0;
    int32_t req_len = 0;
    int32_t rsp_len = 0;
    struct qsc_ion_info ihandle;
    struct QSEECom_ion_fd_info ion_fd_info;
    struct input_message* msg_req;
    struct response_message* msg_rsp;

    LOGD("ets ,send modified cmd buffer: start");

    struct QSEECom_handle* l_QSEEComHandle;
    l_QSEEComHandle = l_tlcComHandle->handle;

    /* allocate in_data_len bytes memory */
    ihandle.ion_fd = 0;
#ifndef TARGET_ION_ABI_VERSION
    ihandle.ion_alloc_handle.handle = 0;
#endif

    nRet = qsc_ION_memalloc(&ihandle, in_data_len);
    if (nRet) {
        LOGE("Error allocating memory in ion\n");
        return -1;
    }

    memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));

    /* Populate the send data structure */
    msg_req = (struct input_message*)l_QSEEComHandle->ion_sbuffer;

    memset(msg_req->egis_data, 0x0, req_tci_size);

    memcpy(msg_req->egis_data, req_tci_data, req_tci_size);
    msg_req->cmd_id = CLIENT_CMD_ETS_MESSAGE;
    msg_req->len = in_data_len;
    msg_req->start_pkt = 0;
    msg_req->end_pkt = 0;
    msg_req->test_buf_size = 0;
    msg_req->buffer[0] = 0x5A;

    ion_fd_info.data[0].fd = ihandle.ifd_data_fd;
    ion_fd_info.data[0].cmd_buf_offset = sizeof(uint32_t);
    ion_fd_info.data[1].fd = ihandle.ifd_data_fd;
    ion_fd_info.data[1].cmd_buf_offset = sizeof(uint32_t) * 2;

    if (in_data != NULL && in_data_len > 0) memcpy(ihandle.ion_sbuffer, in_data, in_data_len);

    req_len = sizeof(struct input_message);
    rsp_len = sizeof(struct response_message);

    if (req_len & QSEECOM_ALIGN_MASK) req_len = QSEECOM_ALIGN(req_len);

    if (rsp_len & QSEECOM_ALIGN_MASK) rsp_len = QSEECOM_ALIGN(rsp_len);

    //msg_rsp = (struct response_message*)l_QSEEComHandle->ion_sbuffer;
	msg_rsp = (struct response_message *)(l_QSEEComHandle->ion_sbuffer + req_len);

    nRet = QSEECom_send_modified_cmd(l_QSEEComHandle, msg_req, req_len, msg_rsp, rsp_len,
                                     &ion_fd_info);

    if (nRet) {
        LOGE("ets_issue_send_modified_cmd_req_in_data: fail cmd = %d ret = %d", msg_req->cmd_id,
             nRet);
    }

    memcpy(rsp_tci_data, msg_rsp->egis_data, req_tci_size);

    if (in_data != NULL && in_data_len > 0) memcpy(in_data, ihandle.ion_sbuffer, in_data_len);

    if (rsp_tci_size) *rsp_tci_size = req_tci_size;

    /* De-allocate 64KB memory */
    nRet = qsc_ion_dealloc(&ihandle);

    if (nRet) {
        LOGE("ets_issue_send_modified_cmd_req_in_data:fail dealloc is %d\n", nRet);
    }

    return nRet;
}

/**@brief:  Exercise send command
 * @param[in]   handle.
 * @param[in]   data to be send to secure app.
 * @return  zero on success or error count on failure.
 */
int32_t ets_keymaster_issue_send_modified_cmd_req(EGIS_TLC_HANDLE_T* l_tlcComHandle,
                                                  unsigned char* req_tci_data, int req_tci_size,
                                                  unsigned char* rsp_tci_data, int* rsp_tci_size) {
    int32_t ret = 0;
    int32_t req_len = 0;
    int32_t rsp_len = 0;
    km_get_auth_token_req_t* msgreq; /* request data sent to QSEE */
    km_get_auth_token_rsp_t* msgrsp; /* response data sent from QSEE */
    struct QSEECom_handle* l_QSEEComHandle;
    l_QSEEComHandle = l_tlcComHandle->handle;
    LOGD("send modified cmd: start");
    LOGD("send modified cmd: 0x%p %d 0x%p 0x%p %d", req_tci_data, req_tci_size, rsp_tci_data,
         rsp_tci_size, *rsp_tci_size);
    /* populate the data in shared buffer */
    msgreq = (km_get_auth_token_req_t*)l_QSEEComHandle->ion_sbuffer;

    memset(msgreq, 0x0, req_tci_size);
    memcpy(msgreq, req_tci_data, req_tci_size);

    LOGD("send modified cmd: cmd_id = %d", msgreq->cmd_id);
    LOGD("send modified cmd: auth_type = %d", msgreq->auth_type);

    req_len = req_tci_size;
    rsp_len = *rsp_tci_size;

    if (req_len & QSEECOM_ALIGN_MASK) req_len = QSEECOM_ALIGN(req_len);

    if (rsp_len & QSEECOM_ALIGN_MASK) rsp_len = QSEECOM_ALIGN(rsp_len);

    //msgrsp = (km_get_auth_token_rsp_t*)l_QSEEComHandle->ion_sbuffer;
	msgrsp=(km_get_auth_token_rsp_t*)(l_QSEEComHandle->ion_sbuffer + req_len);

    ret = QSEECom_send_cmd(l_QSEEComHandle, msgreq, req_len, msgrsp, rsp_len);
    if (ret) {
        LOGE("send command failed with ret = %d\n", ret);
        LOGD("qsc_issue_send_modified_cmd_req: fail cmd = %d ret = %d", msgreq->cmd_id, ret);
    }

    memcpy(rsp_tci_data, msgrsp, *rsp_tci_size);

    return ret;
}
