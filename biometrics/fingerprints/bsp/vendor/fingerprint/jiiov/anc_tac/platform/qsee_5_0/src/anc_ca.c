#define LOG_TAG "[ANC_TAC][CA]"

#include "anc_ca.h"

#include <string.h>
#include "QSEEComAPI.h"
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <cutils/trace.h>

#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif
#include <linux/dma-buf.h>

#include "anc_error.h"
#include "anc_log.h"
#include "anc_memory_wrapper.h"

#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS

// same as QSEECOM_ALIGN, but fixed implicit conversion warning by gcc
#ifdef ANC_EXPLICIT_CONVERSION
#ifdef ANC_QSEECOM_ALIGN
#undef ANC_QSEECOM_ALIGN
#endif
#define ANC_QSEECOM_ALIGN(x)	\
	((x + QSEECOM_ALIGN_MASK) & (~(unsigned int)QSEECOM_ALIGN_MASK))
#else
#define ANC_QSEECOM_ALIGN QSEECOM_ALIGN
#endif

#define ION_BUFFER_LENGTH (1 * 1024 * 1024)
typedef struct {
	int32_t ion_fd;
	int32_t ifd_data_fd;
#ifndef TARGET_ION_ABI_VERSION
    struct ion_handle_data ion_alloc_handle;
#endif
	unsigned char * ion_sbuffer;
	uint32_t sbuf_len;
} AncIonInfo;

static struct QSEECom_handle *gp_qseecom_handle = NULL;


typedef struct {
    AncIonInfo ihandle;
    struct QSEECom_ion_fd_info ion_fd_info;
    uint32_t share_buffer_size;
} AncQseeIonData;

static AncQseeIonData g_ion_data = {
    .share_buffer_size = ION_BUFFER_LENGTH,
};
static AncQseeIonData *gp_ion_data = &g_ion_data;


static pthread_mutex_t g_share_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_ca_ta_transmit_mutex = PTHREAD_MUTEX_INITIALIZER;


/**@brief:  Implement simple application start
 *
 * @param[in/out]	handle.
 * @param[in]		appname.
 * @param[in]		buffer size.
 * @return	zero on success or error count on failure.
 */
static ANC_RETURN_TYPE AncStartTa(struct QSEECom_handle **p_qseecom_handle, uint32_t buf_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int ret = 0;
    int retry_cnt = 5;

    /* start the application */
    /* load split images */
    while(retry_cnt) {
        ret = QSEECom_start_app(p_qseecom_handle, ANC_TA_DEFAULT_PATH, ANC_TA_DEFAULT_NAME, buf_size);
        if (0 != ret) {
            ret_val = ANC_FAIL_LOAD_TA;
            retry_cnt --;
            ANC_LOGE("fail to loading ta : %s, retry_cnt is %d", ANC_TA_DEFAULT_NAME, retry_cnt);
        } else {
            ANC_LOGD("success to loading ta : %s", ANC_TA_DEFAULT_NAME);
            ret_val = ANC_OK;
            break;
        }
    }

    return ret_val;
}

/**@brief:  Implement simple shutdown app
 * @param[in]	handle.
 * @return	zero on success or error count on failure.
 */
static ANC_RETURN_TYPE AncShutdownTa(struct QSEECom_handle **p_qseecom_handle) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int ret = 0;

    /* shutdown the application */
    if (NULL != *p_qseecom_handle) {
        ret = QSEECom_shutdown_app(p_qseecom_handle);
        if (0 != ret) {
            ANC_LOGE("shutdown app failed, ret value = %d\n", ret);
            ret_val = ANC_FAIL;
        } else {
            ANC_LOGD("shutdown app: pass");
            ret_val = ANC_OK;
        }
    } else {
        ANC_LOGE("cannot shutdown as the handle is NULL");
    }

    return ret_val;
}

#define PAGE_SIZE_4KB 4096
#define PAGE_SIZE_4KB_MASK (PAGE_SIZE_4KB -1)
// fixed implicit conversion warning by gcc
#ifdef ANC_EXPLICIT_CONVERSION
#define PAGE_SIZE_4KB_ALIGN(x)	\
	((x + PAGE_SIZE_4KB_MASK) & (~(unsigned int)PAGE_SIZE_4KB_MASK))
#else
#define PAGE_SIZE_4KB_ALIGN(x)	\
	((x + PAGE_SIZE_4KB_MASK) & (~PAGE_SIZE_4KB_MASK))
#endif

#if TARGET_ION_ABI_VERSION >= 2
static ANC_RETURN_TYPE AncIonMemalloc(AncIonInfo *p_handle, uint32_t size) {
    int32_t ret = 0;
    unsigned char *v_addr;
    int32_t ion_fd = -1;
    int32_t map_fd = -1;
    uint32_t len = PAGE_SIZE_4KB_ALIGN(size);
    uint32_t align = 0;
    uint32_t flags = 0;
    struct dma_buf_sync buf_sync;
    unsigned int heap_id = ION_QSECOM_HEAP_ID;

    if (p_handle == NULL) {
        ANC_LOGE("null handle received");
        return ANC_FAIL;
    }

    ion_fd = ion_open();
    if (ion_fd < 0) {
        ANC_LOGE("cannot open ION device");
        return ANC_FAIL;
    }

    ret = ion_alloc_fd(ion_fd, len, align, ION_HEAP(heap_id), flags, &map_fd);
    if (ret) {
        ANC_LOGE("Error::ion_alloc_fd for heap %u size %u failed ret = %d", heap_id, size, ret);
        goto FAIL_ALLOC;
    }

    v_addr = (unsigned char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, map_fd, 0);
    if (v_addr == MAP_FAILED) {
        ANC_LOGE("Error::mmap failed");
        goto FAIL_MAP;
    }

    p_handle->ion_fd = ion_fd;
    p_handle->ifd_data_fd = map_fd;
    p_handle->ion_sbuffer = v_addr;
    p_handle->sbuf_len = size;

    buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    ret = ioctl(map_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (ret) {
        ANC_LOGE("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d", ret);
        goto FAIL_SYNC;
    }

	return ANC_OK;

FAIL_SYNC:
	if (v_addr) {
		munmap(v_addr, len);
		p_handle->ion_sbuffer = NULL;
	}

FAIL_MAP:
	if (map_fd >= 0) {
		ion_close(map_fd);
		p_handle->ifd_data_fd = -1;
	}

FAIL_ALLOC:
    if (ion_fd >= 0) {
        ion_close(ion_fd);
        p_handle->ion_fd = -1;
    }

    return ANC_FAIL;
}

static ANC_RETURN_TYPE AncIonDealloc(AncIonInfo *handle) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    struct dma_buf_sync buf_sync;
    uint32_t len = PAGE_SIZE_4KB_ALIGN(handle->sbuf_len);
    int ret = 0;

    buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
    ret = ioctl(handle->ifd_data_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (ret) {
        ANC_LOGE("Error:: DMA_BUF_IOCTL_SYNC start failed, ret = %d", ret);
        ret_val = ANC_FAIL;
    }

    if (handle->ion_sbuffer) {
        munmap(handle->ion_sbuffer, len);
        handle->ion_sbuffer = NULL;
    }
    if (handle->ifd_data_fd >= 0 ) {
        ion_close(handle->ifd_data_fd);
        handle->ifd_data_fd= -1;
    }
    if (handle->ion_fd >= 0 ) {
        ion_close(handle->ion_fd);
        handle->ion_fd = -1;
    }

    return ret_val;
}
#endif

#ifndef TARGET_ION_ABI_VERSION
static ANC_RETURN_TYPE AncIonMemalloc(AncIonInfo *p_handle, uint32_t size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int32_t ret = 0;
    int32_t iret = 0;
    unsigned char *p_v_addr;
    struct ion_allocation_data ion_alloc_data;
    int32_t ion_fd;
    int32_t rc;
    struct ion_fd_data ifd_data;
    struct ion_handle_data handle_data;

    /* open ION device for memory management
    * O_DSYNC -> uncached memory
    */
    if (p_handle == NULL) {
        ANC_LOGE("null handle received");
        return ANC_FAIL;
    }
    ion_fd = open("/dev/ion", O_RDONLY);
    if (ion_fd < 0) {
        ANC_LOGE("cannot open ION device");
        return ANC_FAIL;
    }
    p_handle->ion_sbuffer = NULL;
    p_handle->ifd_data_fd = 0;

    /* Size of allocation */
    ion_alloc_data.len = PAGE_SIZE_4KB_ALIGN(size);

    /* 4K aligned */
    ion_alloc_data.align = PAGE_SIZE_4KB;

    /* memory is allocated from EBI heap */
    ion_alloc_data.heap_id_mask= ION_HEAP(ION_QSECOM_HEAP_ID);

    /* Set the memory to be uncached */
    ion_alloc_data.flags = 0;

    /* IOCTL call to ION for memory request */
    rc = ioctl(ion_fd, (int)ION_IOC_ALLOC, &ion_alloc_data);
    if (0 != rc) {
        ANC_LOGE("error while trying to allocate data");
        goto FAIL_ALLOC;
    }

    if (ion_alloc_data.handle) {
        ifd_data.handle = ion_alloc_data.handle;
    } else {
        ANC_LOGE("ION alloc data returned a NULL");
        goto FAIL_ALLOC;
    }
    /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
    rc = ioctl(ion_fd, (int)ION_IOC_MAP, &ifd_data);
    if (0 != rc) {
        ANC_LOGE("fail to call ION_IOC_MAP");
        goto FAIL_IOCTL;
    }

    /* Make the ion mmap call */
    p_v_addr = (unsigned char *)mmap(NULL, ion_alloc_data.len,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED, ifd_data.fd, 0);
    if (p_v_addr == MAP_FAILED) {
        ANC_LOGE("ION MMAP failed");
        goto FAIL_MAP;
    }
    p_handle->ion_fd = ion_fd;
    p_handle->ifd_data_fd = ifd_data.fd;
    p_handle->ion_sbuffer = p_v_addr;
    p_handle->ion_alloc_handle.handle = ion_alloc_data.handle;
    p_handle->sbuf_len = size;

    return ret_val;

FAIL_MAP:
    if (p_handle->ion_sbuffer != NULL) {
        ret = munmap(p_handle->ion_sbuffer, ion_alloc_data.len);
        if (0 != ret) {
            ANC_LOGE("fail to unmap memory for load image. ret = %d",
                  ret);
        }
    }

FAIL_IOCTL:
    handle_data.handle = ion_alloc_data.handle;
    if (p_handle->ifd_data_fd) {
        close(p_handle->ifd_data_fd);
    }
    iret = ioctl(ion_fd, (int)ION_IOC_FREE, &handle_data);
    if (0 != iret) {
        ANC_LOGE("ION FREE ioctl returned error = %d",iret);
    }

FAIL_ALLOC:
    if (0 != ion_fd) {
        close(ion_fd);
    }

    ret_val = ANC_FAIL;

    return ret_val;
}

/** @brief: Deallocate ION memory
 *
 *
 */
static ANC_RETURN_TYPE AncIonDealloc(AncIonInfo *handle) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    struct ion_handle_data handle_data;
    int32_t ret = 0;

    /* Deallocate the memory for the listener */
    ret = munmap(handle->ion_sbuffer, PAGE_SIZE_4KB_ALIGN(handle->sbuf_len));
    if (0 != ret) {
        ANC_LOGE("fail to unmapping ION Buffer, ret : %d\n", ret);
        ret_val = ANC_FAIL;
    }

    handle_data.handle = handle->ion_alloc_handle.handle;
    close(handle->ifd_data_fd);
    ret = ioctl(handle->ion_fd, (int)ION_IOC_FREE, &handle_data);
    if (0 != ret) {
        ANC_LOGE("fail to free ION Memory by ioctl, ret : %d\n", ret);
        ret_val = ANC_FAIL;
    }
    close(handle->ion_fd);

    return ret_val;
}
#endif

ANC_RETURN_TYPE AncGetIonSharedBuffer(uint8_t **p_share_buffer, uint32_t *p_share_buffer_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    *p_share_buffer = (uint8_t *)gp_ion_data->ihandle.ion_sbuffer;
    *p_share_buffer_size = gp_ion_data->share_buffer_size;
    if (NULL == p_share_buffer) {
        ANC_LOGE("share buffer is NULL");
        return ANC_FAIL;
    }
    if (*p_share_buffer_size <= 0) {
        ANC_LOGE("share buffer size : %d", *p_share_buffer_size);
        return ANC_FAIL;
    }

    AncMemset(*p_share_buffer, 0, *p_share_buffer_size);

    return ret_val;
}

static ANC_RETURN_TYPE AncInitIon() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    gp_ion_data->ihandle.ion_fd = 0;
#ifndef TARGET_ION_ABI_VERSION
    gp_ion_data->ihandle.ion_alloc_handle.handle = 0;
#endif
    ret_val = AncIonMemalloc(&gp_ion_data->ihandle, gp_ion_data->share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to allocate memory in ion\n");
        return ANC_FAIL;
    }

    return ret_val;
}

static ANC_RETURN_TYPE AncDeinitIon() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ret_val = AncIonDealloc(&gp_ion_data->ihandle);
    if(ANC_OK != ret_val) {
        ANC_LOGE("return value of dealloc is %d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE AncCaLockSharedBuffer(uint8_t *p_function_name) {
    pthread_mutex_lock(&g_share_buffer_mutex);
#ifdef ANC_DEBUG
    ANC_LOGW("anc ca lock shared buffer, %s", p_function_name);
#else
    ANC_UNUSED(p_function_name);
#endif

    return ANC_OK;
}

ANC_RETURN_TYPE AncCaUnlockSharedBuffer(uint8_t *p_function_name) {
    pthread_mutex_unlock(&g_share_buffer_mutex);
#ifdef ANC_DEBUG
    ANC_LOGW("anc ca unlock shared buffer, %s", p_function_name);
#else
    ANC_UNUSED(p_function_name);
#endif

    return ANC_OK;
}

static ANC_RETURN_TYPE AncTransmitModifiedMessage(struct QSEECom_handle *l_QSEEComHandle,
                               AncSendCommand *send_cmd, AncSendCommandRespond *p_send_rsp_cmd) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint32_t req_len = 0;
    uint32_t rsp_len = 0;
    AncSendCommand *msg_req;	/* request data sent to QSEE */
    AncSendCommandRespond *msg_rsp;	/* response data sent from QSEE */
    struct qseecom_app_info app_info;
    void *send_buf = NULL;
    int ret = 0;


    ret = QSEECom_get_app_info(l_QSEEComHandle, &app_info);
    if (0 != ret) {
        ANC_LOGE("Error to get app info\n");
        return ANC_FAIL_TA_TRANSMIT;
    }

    /* Populate the send data structure */
    msg_req = (AncSendCommand *)l_QSEEComHandle->ion_sbuffer;
    AncMemcpy(msg_req, send_cmd, sizeof(AncSendCommand));
    msg_req->share_buffer_length = gp_ion_data->share_buffer_size;
    send_buf = (void *)msg_req;
    req_len = sizeof(AncSendCommand);
    rsp_len = sizeof(AncSendCommandRespond);

    AncMemset(&gp_ion_data->ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
    gp_ion_data->ion_fd_info.data[0].fd = gp_ion_data->ihandle.ifd_data_fd;
    gp_ion_data->ion_fd_info.data[0].cmd_buf_offset = offsetof(AncSendCommand, share_buffer_ptr);

    if (req_len & QSEECOM_ALIGN_MASK) {
        req_len = ANC_QSEECOM_ALIGN(req_len);
    }

    if (rsp_len & QSEECOM_ALIGN_MASK) {
        rsp_len = ANC_QSEECOM_ALIGN(rsp_len);
    }

    msg_rsp = (AncSendCommandRespond *)(l_QSEEComHandle->ion_sbuffer + req_len);
    msg_rsp->status = 0;
    /* send request from HLOS to QSEApp */
    ATRACE_BEGIN("qseecom_send_modified_cmd");
    if (!app_info.is_secure_app_64bit) {
#ifdef ANC_DEBUG
        ANC_LOGD("send cmd to 32bit app, command id = %d", msg_req->id);
#endif
        ret = QSEECom_send_modified_cmd(l_QSEEComHandle, send_buf,
                                        req_len, msg_rsp,
                                        rsp_len, &gp_ion_data->ion_fd_info);
    } else {
#ifdef ANC_DEBUG
        ANC_LOGD("send cmd to 64bit app, command id = %d", msg_req->id);
#endif
        ret = QSEECom_send_modified_cmd_64(l_QSEEComHandle, send_buf,
                                        req_len, msg_rsp,
                                        rsp_len, &gp_ion_data->ion_fd_info);
    }
    ATRACE_END();
    if (0 != ret) {
        ANC_LOGE("send modified command failed with ret value = %d\n", ret);
        ret_val = ANC_FAIL_TA_TRANSMIT;
    } else {
        if (NULL != p_send_rsp_cmd) {
            AncMemcpy(p_send_rsp_cmd, msg_rsp, sizeof(AncSendCommandRespond));
        }
        ret_val = ANC_OK;
    }

    return ret_val;
}

/**@brief:  Exercise send command
 * @param[in]	handle.
 * @param[in]	data to be send to secure app.
 * @return	zero on success or error count on failure.
 */
static ANC_RETURN_TYPE AncTransmitMessage(struct QSEECom_handle *l_QSEEComHandle,
                               AncSendCommand *send_cmd, AncSendCommandRespond *p_send_rsp_cmd) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint32_t req_len = 0;
    uint32_t rsp_len = 0;
    AncSendCommand *msgreq;	/* request data sent to QSEE */
    AncSendCommandRespond *msgrsp;	/* response data sent from QSEE */
    struct qseecom_app_info app_info;
    void *send_buf = NULL;
    int ret = 0;


    ret = QSEECom_get_app_info(l_QSEEComHandle, &app_info);
    if (0 != ret) {
        ANC_LOGE("Error to get app info\n");
        return ANC_FAIL_TA_TRANSMIT;
    }
    /* populate the data in shared buffer */
    msgreq = (AncSendCommand *)l_QSEEComHandle->ion_sbuffer;
    AncMemcpy(msgreq, send_cmd, sizeof(AncSendCommand));
    send_buf = (void *)msgreq;
    req_len = sizeof(AncSendCommand);
    rsp_len = sizeof(AncSendCommandRespond);
#ifdef ANC_DEBUG
    if (!app_info.is_secure_app_64bit) {
        ANC_LOGD("send cmd to 32bit app, command id = %d", msgreq->id);
    } else {
        ANC_LOGD("send cmd to 64bit app, command id = %d", msgreq->id);
    }
#endif

    if (req_len & QSEECOM_ALIGN_MASK) {
        req_len = ANC_QSEECOM_ALIGN(req_len);
    }

    if (rsp_len & QSEECOM_ALIGN_MASK) {
        rsp_len = ANC_QSEECOM_ALIGN(rsp_len);
    }

    msgrsp = (AncSendCommandRespond *)l_QSEEComHandle->ion_sbuffer;

    /* send request from HLOS to QSEApp */
    ret = QSEECom_send_cmd(l_QSEEComHandle,
                            send_buf,
                            req_len,
                            msgrsp,
                            rsp_len);
    if (0 != ret) {
        ANC_LOGE("send command failed with ret value = %d\n", ret);
        ret_val = ANC_FAIL_TA_TRANSMIT;
    } else {
        if (NULL != p_send_rsp_cmd) {
            AncMemcpy(p_send_rsp_cmd, msgrsp, sizeof(AncSendCommandRespond));
        }
        ret_val = ANC_OK;
    }

    return ret_val;
}

ANC_RETURN_TYPE InitAncCa() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    gp_qseecom_handle = NULL;
    ret_val = AncStartTa(&gp_qseecom_handle, 9216);
    if (ANC_OK != ret_val) {
        ANC_LOGE("Start ta: fail");
    }

    if (ANC_OK != (ret_val = AncInitIon())) {
        ANC_LOGE("fail to init ion");
    }

    return ret_val;
}

ANC_RETURN_TYPE DeinitAncCa() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (ANC_OK != (ret_val = AncDeinitIon())) {
        ANC_LOGE("fail to deinit ion");
    }

    ret_val = AncShutdownTa(&gp_qseecom_handle);
    if (ANC_OK != ret_val) {
        ANC_LOGE("Failed to shutdown app, ret value = %d\n", ret_val);
    }
    gp_qseecom_handle = NULL;

    return ret_val;
}

ANC_RETURN_TYPE AncCaTransmit(AncSendCommand *p_send_cmd, AncSendCommandRespond *p_send_rsp_cmd) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    pthread_mutex_lock(&g_ca_ta_transmit_mutex);
    ANC_LOGW("anc ca lock transmit , %s", (uint8_t *)__func__);

    p_send_cmd->share_buffer_length = 0;
    p_send_cmd->share_buffer_ptr = 0;

    ret_val = AncTransmitMessage(gp_qseecom_handle, p_send_cmd, p_send_rsp_cmd);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to transmit message id : %d", p_send_cmd->id);
    }

    pthread_mutex_unlock(&g_ca_ta_transmit_mutex);
    ANC_LOGW("anc ca unlock transmit , %s", (uint8_t *)__func__);

    return ret_val;
}

ANC_RETURN_TYPE AncCaTransmitModified(AncSendCommand *p_send_cmd, AncSendCommandRespond *p_send_rsp_cmd) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    pthread_mutex_lock(&g_ca_ta_transmit_mutex);
    ANC_LOGW("anc ca lock transmit , %s", (uint8_t *)__func__);

    ret_val = AncTransmitModifiedMessage(gp_qseecom_handle, p_send_cmd, p_send_rsp_cmd);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to transmit message id : %d", p_send_cmd->id);
    }

    pthread_mutex_unlock(&g_ca_ta_transmit_mutex);
    ANC_LOGW("anc ca unlock transmit , %s", (uint8_t *)__func__);

    return ret_val;
}

/*************************test share buffer*************************/
static ANC_RETURN_TYPE AncTestShareBufferTransmit() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSendCommand anc_send_command;
    AncSendCommandRespond anc_send_respond;

    AncMemset(&anc_send_respond, 0, sizeof(anc_send_respond));
    AncMemset(&anc_send_command, 0, sizeof(anc_send_command));
    anc_send_command.size = sizeof(AncSendCommand);
    anc_send_command.id = ANC_CMD_TEST_SHARE_BUFFER;
    ret_val = AncCaTransmitModified(&anc_send_command, &anc_send_respond);

    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send test share buffer command, ret value:%d", ret_val);
    } else {
        ret_val = (ANC_RETURN_TYPE)anc_send_respond.status;
    }

    return ret_val;
}

#define SHARED_BUF_PATTERN_LEN 16
ANC_RETURN_TYPE AncTestSharedBuffer() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;
    char *p_verify = NULL;
    uint32_t i = 0;

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        return ANC_FAIL;
    }

    ANC_LOGD("anc malloc verify");
    p_verify = AncMalloc(share_buffer_size);
    if(NULL == p_verify) {
        ANC_LOGE("fail to anc malloc for verifying\n");
        return ANC_FAIL;
    }

    ANC_LOGD("set verify value");
    for (i=0; i<share_buffer_size; i++) {
        *(p_share_buffer + i) = (char)(i % 255);
        *(p_verify + i) = (char)(i % 255);
    }

    ret_val = AncTestShareBufferTransmit();
    /* Verify the first 16 bytes of response data.
        It should be req_data + 10
    */
    if(ANC_OK == ret_val) {
        for (i=0; i<SHARED_BUF_PATTERN_LEN; i++) {
            *(p_share_buffer + i) = *(p_share_buffer + i)-10;
            if(*(p_share_buffer + i) != (char)(i % 255)) {
                ANC_LOGD("Modified buffer check @ %d = %x", i, *(p_share_buffer + i));
                break;
            }
        }
        if (SHARED_BUF_PATTERN_LEN == i) {
            ANC_LOGD("success to Verify");
        }
    }

    ANC_LOGD("anc free verify");
    if (NULL == p_verify) {
        AncFree(p_verify);
    }

    return ret_val;
}
