/************************************************************************************
 ** File: - fpc_qsee_tac.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      fpc QSEECOMAPI (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,21/10/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 **
 ** --------------------------- Revision History: --------------------------------
 ** 	<author>	     <data>			<desc>
 **    Ziqing.guo      2017/10/13    create the file
 **    Ziqing.guo      2017/10/22    add wakelock for fpc_tac_transfer
 **    Ziqing.guo      2017/10/26    add for load respective TA according to fp vendor type
 **    Ziqing.guo      2017/11/30    add for 1270
 **    Ran.Chen        2018/01/29    modify for fp_id, Code refactoring
 **    Hongyu.lu       2019/04/18    modify for ion
 ************************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include "QSEEComAPI.h"
#include <fpc_tac.h>
#include <fpc_log.h>
#include <fpc_types.h>
#include <fpc_sysfs.h>
#include <ion/ion.h>
#include <linux/msm_ion.h>
#include <linux/dma-buf.h>
#include <fingerprint_type.h>
#define ION_DEVICE "/dev/ion"
#include "fpc_ta_interface.h"

struct fpc_tac {
    struct QSEECom_handle* qseecom_handle;
    uint32_t size_sbuffer;
    int sysfs_fd;
};

typedef struct {
    uint32_t size;
    uint64_t addr; // 64 bit makes enough room for both 32 bit and 64 bit trustzone
}__attribute__((packed)) fpc_qsee_msg_header_t;

typedef int32_t fpc_qsee_msg_response_t;

//#define ION_QSECOM_HEAP_ID 27
//#define ION_QSECOM_HEAP_ID 7
//#define ION_QSECOM_HEAP_MASK (1 << (ION_QSECOM_HEAP_ID))

#define AlignedPageSize(size) (((size) + 4095) & (~4095))
fpc_tac_t* fpc_tac_open() {
    int status;

    fpc_tac_t* tac = malloc(sizeof(fpc_tac_t));
    if (!tac) {
        return NULL;
    }

    memset(tac, 0, sizeof(fpc_tac_t));
    tac->sysfs_fd = -1;

    char path[PATH_MAX];

    if (!fpc_sysfs_path_by_attr(FPC_REE_DEVICE_ALIAS_FILE, FPC_REE_DEVICE_NAME, FPC_REE_DEVICE_PATH,
                path, PATH_MAX)) {
        goto err;
    }

    tac->sysfs_fd = open(path, O_RDONLY);

    if (tac->sysfs_fd == -1) {
        LOGE("%s open %s failed %i", __func__, path, errno);
        goto err;
    }

    tac->size_sbuffer = QSEECOM_ALIGN(sizeof(fpc_qsee_msg_header_t)) +
        QSEECOM_ALIGN(sizeof(fpc_qsee_msg_response_t));

#ifndef FPC_CONFIG_QSEE4
    status = fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "1");
    if (status) {
        goto err;
    }
#endif

    status = QSEECom_start_app(&tac->qseecom_handle,
                FP_TA_PATH,
                fp_config_info_init.ta_qsee_name,
                tac->size_sbuffer);

#ifndef FPC_CONFIG_QSEE4
    fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "0");
#endif

    if (status) {
        LOGE("%s start_app failed: %i", __func__, status);
        goto err;
    }
    LOGE("%s start_app success: %s", __func__, FP_TA_PATH);

    return tac;

err:
    if (tac->sysfs_fd != -1) {
        close(tac->sysfs_fd);
    }

    free(tac);
    return NULL;
}

void fpc_tac_release(fpc_tac_t* tac) {
    if (!tac) {
        return;
    }

    int status = QSEECom_shutdown_app(&tac->qseecom_handle);

    if (status) {
        LOGE("%s shutdown_app failed: %i", __func__, status);
    }

    if (tac->sysfs_fd != -1) {
        close(tac->sysfs_fd);
    }

    free(tac);
}

typedef struct {
    fpc_tac_shared_mem_t shared_mem;

    uint32_t size_mem;
    int ion_fd;
    int map_fd;
} qsee_ion_data_t;

fpc_tac_shared_mem_t* fpc_tac_alloc_shared(fpc_tac_t* tac, uint32_t size) {
    (void)tac; // Unused parameter
    int status;
    qsee_ion_data_t* ion_data = malloc(sizeof(qsee_ion_data_t));

    if (!ion_data) {
        return NULL;
    }

    unsigned int heap_id = ION_QSECOM_HEAP_ID;
    ion_data->ion_fd = -1;
    ion_data->map_fd = -1;
    ion_data->shared_mem.addr = MAP_FAILED;
    ion_data->size_mem = AlignedPageSize(size);
    ion_data->ion_fd = ion_open();

    if (ion_data->ion_fd == -1) {
        LOGE("%s open failed with error %i", __func__, -errno);
        goto err;
    }
#ifdef FP_CONFIG_SHMBRIDGE_ION_FUNCTION
    status = ion_alloc_fd(ion_data->ion_fd,
                        ion_data->size_mem,
                        0,
                        ION_HEAP(heap_id),
                        0, //ION_IOC_ALLOC
                        &ion_data->map_fd);
#else
    status = ion_alloc_fd(ion_data->ion_fd,
                        ion_data->size_mem,
                        0,
                        ION_HEAP(heap_id),
                        ION_IOC_ALLOC,
                        &ion_data->map_fd);
#endif
    if (status) {
        LOGE("%s IOC_ALLOC failed with error %i, size_mem=%u", __func__, -errno, ion_data->size_mem);
        goto err;
    }

    ion_data->shared_mem.addr = (void *) mmap(NULL,
                                              ion_data->size_mem,
                                              PROT_READ | PROT_WRITE,
                                              MAP_SHARED,
                                              ion_data->map_fd,
                                              0);

    if (ion_data->shared_mem.addr == MAP_FAILED) {
        LOGE("%s IOC_MAP failed with error %i", __func__, -errno);
        goto err;
    }

    return (fpc_tac_shared_mem_t*) ion_data;

err:
    fpc_tac_free_shared((fpc_tac_shared_mem_t*) ion_data);
    return NULL;
}

void fpc_tac_free_shared(fpc_tac_shared_mem_t* shared_buffer) {
    qsee_ion_data_t* ion_data = (qsee_ion_data_t*) shared_buffer;
    if (!shared_buffer) {
        return;
    }

    if (ion_data->shared_mem.addr != MAP_FAILED) {
        if (munmap(ion_data->shared_mem.addr, ion_data->size_mem)) {
            LOGE("%s munmap failed with error %i", __func__, -errno);
        }
    }

    if (ion_data->map_fd != -1) {
        close(ion_data->map_fd);
    }

    if (ion_data->ion_fd != -1) {
        ion_close(ion_data->ion_fd);
    }

    free(shared_buffer);
}

int fpc_tac_transfer(fpc_tac_t* tac, fpc_tac_shared_mem_t *shared_buffer) {
    int status;

    fpc_ta_cmd_header_t* ta_header = shared_buffer->addr;
    LOGD("%s ############%d, %d############", __func__, ta_header->target, ta_header->command);

#ifdef FPC_CONFIG_WAKE_LOCK
    fpc_sysfs_node_write(tac->sysfs_fd, "wakelock_enable", WAKELOCK_TIMEOUT_ENABLE);
#endif /*OPLUS_FEATURE_FINGERPRINT*/

    qsee_ion_data_t* ion_data = (qsee_ion_data_t*) shared_buffer;

    fpc_qsee_msg_header_t* header =
        (fpc_qsee_msg_header_t*) tac->qseecom_handle->ion_sbuffer;

    header->size = ion_data->size_mem;

    struct QSEECom_ion_fd_info ion_fd_info;
    memset(&ion_fd_info, 0, sizeof(ion_fd_info));
    ion_fd_info.data[0].fd = ion_data->map_fd;
    ion_fd_info.data[0].cmd_buf_offset = offsetof(fpc_qsee_msg_header_t, addr);

    fpc_qsee_msg_response_t* response = (fpc_qsee_msg_response_t*)
        (tac->qseecom_handle->ion_sbuffer +
        QSEECOM_ALIGN(sizeof(fpc_qsee_msg_header_t)));

#ifndef FPC_CONFIG_QSEE4
    status = fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "1");
    if (status) {
        return status;
    }
#endif

    status = QSEECom_send_modified_cmd(tac->qseecom_handle,
            header, QSEECOM_ALIGN(sizeof(fpc_qsee_msg_header_t)),
            response, QSEECOM_ALIGN(sizeof(fpc_qsee_msg_response_t)),
            &ion_fd_info);

#ifndef FPC_CONFIG_QSEE4
    fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "0");
#endif

#ifdef FPC_CONFIG_WAKE_LOCK
    fpc_sysfs_node_write(tac->sysfs_fd, "wakelock_enable", WAKELOCK_TIMEOUT_DISABLE);
#endif /*OPLUS_FEATURE_FINGERPRINT*/

    LOGD("%s status : %i, *response :%i", __func__, status, *response);

    if (status) {
        LOGE("%s send_cmd failed ,status : %i", __func__, status);
        return -FPC_ERROR_PARAMETER;
    }
    if (*response < 0) {
        LOGE("%s send_cmd failed ,*response :%i", __func__, *response);
        //return -FPC_ERROR_PARAMETER;
    }

    return *response;
}
