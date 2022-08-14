/************************************************************************************
 ** Copyright (C), 2008-2021, fp Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Ca/QseeCa.cpp
 **
 ** Description:
 **      QseeCa for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 **  Jiaqi.Wu   2021/04/25        add ca buffer tansfer
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][QseeCa]"

#ifdef TEE_QSEE

#include <cutils/trace.h>
#include <errno.h>
#include <hardware/hw_auth_token.h>
#include <ion/ion.h>
#include <linux/dma-buf.h>
#include <linux/ion.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "FpCommon.h"
#include "HalLog.h"
#include "QSEEComAPI.h"
#include "QseeCa.h"
#include "FpType.h"


#define FP_TA_PATH "odm/vendor/firmware"
#define FP_TA_QSEE_NAME "uff_gx"
#define FP_CHIPID_TA_QSEE_NAME "fp_spi"

#define AlignedPageSize(size) (((size) + 4095) & (~4095))
#define TA_LOG_DUMP_BUFFER_SIZE (32 * 1024)
#define ION_QSECOM_HEAP_ID 27
#define ION_HEAP(bit) (1 << (bit))

// keymaster command definition
#define KEYMASTER_TA_PATH "odm/vendor/firmware"
#define KEYMASTER_TA_NAME "keymaster64"

#define BASE_SHARED_BUFFER_LEN 4096
#define KEYMASTER_UTILS_CMD_ID 0x200UL
#define KEYMASTER_GET_AUTH_TOKEN_KEY (KEYMASTER_UTILS_CMD_ID + 5UL)

typedef struct {
    int32_t cmd_id;
    hw_authenticator_type_t auth_type;
} __attribute__((packed)) KM_GET_AUTH_TOKEN_REQ_T;

typedef struct {
    int32_t status;
    uint32_t key_offset;
    uint32_t key_len;
} __attribute__((packed)) KM_GET_AUTH_TOKEN_RSP_T;

typedef struct fp_ta_info {
    fp_ta_name_t index;
    const char* name;
} fp_ta_info_t;

fp_ta_info_t taNames[] = {
    {FP_SPI_TA, "uff_spi"},
    {FP_GOODIX_TA, "uff_gx"},
    {FP_JV0301_TA, "uff_jv"},
    {FP_JV0302_TA, "uff_jv"},
    {FP_SILEAD_TA, "uff_silead"},
    {FP_EGIS_TA, "uff_egis"},
};

typedef struct {
    uint32_t size;
    uint64_t addr;
    uint32_t ta_log_level;
    fp_return_type_t status;  // 64 bit makes enough room for both 32 bit and 64 bit trustzone
} __attribute__((packed)) fp_qsee_cmd_t;

typedef struct {
    QSEECom_handle* taHandle;
    //TalogDump* taLogDump;
    uint32_t sbuffer_size;
} fp_qsee_context;

typedef struct fpshared_mem {
    void* ionbuffer;
} fpshared_mem_t;

typedef struct {
    ion_allocation_data allocdata;
    int mmap_fd;
    fpshared_mem_t shared_mem;
    uint32_t ion_open_fd;
} fp_qsee_ion_t;

namespace android {
typedef struct {
    QSEECom_handle* taHandle;
    // TalogDump* taLogDump;
    uint32_t sbuffer_size;
} QSEE_CONTEXT;

QseeCa::QseeCa() : mContext(nullptr) {
    LOG_I(LOG_TAG, "[%s] start ", __func__);
}

QseeCa::~QseeCa() {
    if (nullptr != mContext) {
        delete static_cast<QSEE_CONTEXT*>(mContext);
    }
    mContext = nullptr;
}

fp_return_type_t QseeCa::startTa(fp_ta_name_t name) {
    fp_return_type_t err = FP_ERROR;
    int status           = 0;
    FUNC_ENTER();
    pthread_mutex_init(&cmd_lock, NULL);
    if (nullptr != mContext) {
        delete static_cast<QSEE_CONTEXT*>(mContext);
        mContext = nullptr;
    }
    if (name > sizeof(taNames)/sizeof(taNames[0]) - 1) {
        LOG_E(LOG_TAG, "[%s] %d not match tanames", __func__, (int)name);
        return FP_SUCCESS;
    }
    mContext = new QSEE_CONTEXT();
    memset(mContext, 0, sizeof(QSEE_CONTEXT));
    QSEE_CONTEXT* qcontext = static_cast<QSEE_CONTEXT*>(mContext);
    qcontext->sbuffer_size
        = QSEECOM_ALIGN(sizeof(fp_qsee_cmd_t)) + QSEECOM_ALIGN(sizeof(fp_qsee_cmd_t));

    status = QSEECom_start_app(
        &qcontext->taHandle, FP_TA_PATH, taNames[name].name, qcontext->sbuffer_size);

    LOG_I(LOG_TAG, "sbuffer_size:%u QSEECom_start_app:%s %s", qcontext->sbuffer_size, FP_TA_PATH,
        taNames[name].name);
    if (status) {
        LOG_E(LOG_TAG, "%s QSEECom_start_app failed: %d", __func__, status);
        goto exit;
    }
    err = FP_SUCCESS;
exit:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t QseeCa::closeTa() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    QSEE_CONTEXT* qcontext = static_cast<QSEE_CONTEXT*>(mContext);
    if (nullptr != qcontext && qcontext->taHandle != nullptr) {
        int32_t status = QSEECom_shutdown_app(&qcontext->taHandle);
        if (status) {
            err = FP_ERROR;
            LOG_E(LOG_TAG, "[%s] Unload: %s failed, status=%d, errno=%d.", __func__,
                FP_TA_QSEE_NAME, status, errno);
        } else {
            qcontext->taHandle = nullptr;
            LOG_D(LOG_TAG, "[%s] Unload: %s succeeded.", __func__, FP_TA_QSEE_NAME);
        }
    } else {
        LOG_D(LOG_TAG, "[%s] qcontext->taHandle is already nullptr.", __func__);
    }
    if (nullptr != qcontext) {
        delete qcontext;
    }
    mContext = nullptr;
    pthread_mutex_destroy(&cmd_lock);
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t QseeCa::sendCommand(void* cmd, uint32_t len) {
    fp_return_type_t err = FP_SUCCESS;
    int status;
    FUNC_ENTER();
    pthread_mutex_lock(&cmd_lock);

    QSEE_CONTEXT* qcontext             = static_cast<QSEE_CONTEXT*>(mContext);
    uint32_t taLogDumpLeavel           = 0;
    uint32_t taLogBufferLen            = 0;
    uint32_t qseeCmdLen                = sizeof(fp_qsee_cmd_t);
    uint32_t bufferlen                 = 0;
    fp_qsee_ion_t* buffershare         = (fp_qsee_ion_t*)malloc(sizeof(fp_qsee_ion_t));
    fp_qsee_cmd_t* header              = (fp_qsee_cmd_t*)qcontext->taHandle->ion_sbuffer;
    fp_ta_cmd_header_t* qseecmd_header = (fp_ta_cmd_header_t*)cmd;
    LOG_D(LOG_TAG, "[%s] ta command module_id: %d, cmd id: %d", __func__, qseecmd_header->module_id,
        qseecmd_header->cmd_id);
    memset(header, 0, sizeof(fp_qsee_cmd_t));
    if (taLogDumpLeavel > 0) {
        taLogBufferLen = TA_LOG_DUMP_BUFFER_SIZE;
    }
    bufferlen = len + taLogBufferLen;
    if (!buffershare) {
        err = FP_ERROR;
        goto exit;
    }
    buffershare->allocdata.len          = AlignedPageSize(bufferlen);
    buffershare->allocdata.align        = 0;
    buffershare->allocdata.heap_id_mask = ION_HEAP(ION_QSECOM_HEAP_ID);
    buffershare->allocdata.flags        = 0;  //not ION_IOC_ALLOC
    buffershare->ion_open_fd            = -1;
    buffershare->mmap_fd                = -1;
    buffershare->shared_mem.ionbuffer   = MAP_FAILED;
    buffershare->ion_open_fd            = ion_open();
    if (buffershare->ion_open_fd == -1) {
        LOG_E(LOG_TAG, "[%s] open failed with error %i", __func__, errno);
        goto exit;
    }
    status = ion_alloc_fd(buffershare->ion_open_fd, buffershare->allocdata.len,
        buffershare->allocdata.align, buffershare->allocdata.heap_id_mask,
        buffershare->allocdata.flags, &buffershare->mmap_fd);
    if (status) {
        LOG_E(LOG_TAG, "%s IOC_ALLOC failed with error %i, bufferlen=%lu %s", __func__, errno,
            buffershare->allocdata.len, strerror(errno));
        goto exit;
    }
    buffershare->shared_mem.ionbuffer = (unsigned char*)mmap(NULL, buffershare->allocdata.len,
        PROT_READ | PROT_WRITE, MAP_SHARED, buffershare->mmap_fd, 0);
    if (buffershare->shared_mem.ionbuffer == MAP_FAILED) {
        LOG_E(LOG_TAG, "[%s]ION MMAP failed (%d:%s)", __func__, errno, strerror(errno));
        goto exit;
    }
    header->size = len;
    struct QSEECom_ion_fd_info ion_fd_info;
    memset(&ion_fd_info, 0, sizeof(ion_fd_info));
    ion_fd_info.data[0].fd             = buffershare->mmap_fd;
    ion_fd_info.data[0].cmd_buf_offset = offsetof(fp_qsee_cmd_t, addr);
    memcpy(buffershare->shared_mem.ionbuffer, cmd, len);
    status = QSEECom_send_modified_cmd(qcontext->taHandle, header, QSEECOM_ALIGN(qseeCmdLen),
        header, QSEECOM_ALIGN(qseeCmdLen), &ion_fd_info);
    if (status != 0) {
        LOG_E(LOG_TAG, "[%s] QSEECom_send_modified_cmd failed, status=%d", __func__, status);
        err = FP_ERR_TA_DEAD;
        goto exit;
    }
    memcpy(cmd, buffershare->shared_mem.ionbuffer, len);
    if (header->status) {
        LOG_E(LOG_TAG, "%s send_cmd failed ,ta->status :%i", __func__, header->status);
        err = header->status;
        goto exit;
    }
exit:
    if (buffershare != nullptr) {
        if (buffershare->shared_mem.ionbuffer != MAP_FAILED) {
            if (munmap(buffershare->shared_mem.ionbuffer, buffershare->allocdata.len)) {
                LOG_E(LOG_TAG, "%s munmap failed with error %i", __func__, -errno);
            }
        }

        if (buffershare->mmap_fd != -1) {
            close(buffershare->mmap_fd);
        }

        if (buffershare->ion_open_fd != -1) {
            ion_close(buffershare->ion_open_fd);
        }

        free(buffershare);
    }
    if (buffershare->ion_open_fd != -1) {
        ion_close(buffershare->ion_open_fd);
    }
    FUNC_EXIT(err);
    pthread_mutex_unlock(&cmd_lock);
    return err;
}

fp_return_type_t QseeCa::sendHmacKeyToFpta() {
    fp_return_type_t err = FP_SUCCESS;
    int status = 0;
    FUNC_ENTER();
    fp_set_hmackey_t cmd;
    memset(&cmd, 0, sizeof(fp_set_hmackey_t));
    struct QSEECom_handle *keyMasterHandle = nullptr;
    KM_GET_AUTH_TOKEN_REQ_T config_buffer;
    memset(&config_buffer, 0, sizeof(KM_GET_AUTH_TOKEN_REQ_T));
    KM_GET_AUTH_TOKEN_RSP_T *hmackey_buffer = (KM_GET_AUTH_TOKEN_RSP_T *)cmd.config.hmackey_buffer;

    //start keymaster tzapp
    status = QSEECom_start_app(&keyMasterHandle, KEYMASTER_TA_PATH, KEYMASTER_TA_NAME, BASE_SHARED_BUFFER_LEN);
    CHECK_RESULT_SUCCESS((fp_return_type_t)status);
    LOG_D(LOG_TAG, "[%s] start keymaster ta succeeded, handle=%p", __func__, keyMasterHandle);

    //get hmackey
    config_buffer.cmd_id = KEYMASTER_GET_AUTH_TOKEN_KEY;
    config_buffer.auth_type = HW_AUTH_FINGERPRINT;
    status = QSEECom_send_cmd(keyMasterHandle, &config_buffer, sizeof(config_buffer), hmackey_buffer, QSEE_HMAC_KEY_MAX_LEN);
    CHECK_RESULT_SUCCESS((fp_return_type_t)status);
    LOG_D(LOG_TAG, "[%s] get hmackey from keymaster succeeded, key_offset:%d, key_len:%d", __func__,
        hmackey_buffer->key_offset, hmackey_buffer->key_len);

    //send hmackey to fp_ta
    cmd.header.module_id = FP_MODULE_FPCORE;
    cmd.header.cmd_id = FP_CMD_FPCORE_SET_HMACKEY;
    err = sendCommand(&cmd, sizeof(cmd));
    CHECK_RESULT_SUCCESS(err);

fp_out:
    //shutdown keymaster tzapp
    if (keyMasterHandle != nullptr) {
        status = QSEECom_shutdown_app(&keyMasterHandle);
    }
    FUNC_EXIT(err);
    return err;
}


}  // namespace android

#else

#include "QseeCa.h"

namespace android {

QseeCa::QseeCa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
}

QseeCa::~QseeCa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
}

fp_return_type_t QseeCa::startTa(fp_ta_name_t name) {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t QseeCa::closeTa() {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t QseeCa::sendCommand(void* cmd, uint32_t size) {
    LOG_I(LOG_TAG, "[%s] FP_ERR_NOT_REALIZATION ", __func__);
    return FP_ERR_NOT_REALIZATION;
}

fp_return_type_t QseeCa::sendHmacKeyToFpta() {
    return FP_SUCCESS;
}

}  // namespace android
#endif
