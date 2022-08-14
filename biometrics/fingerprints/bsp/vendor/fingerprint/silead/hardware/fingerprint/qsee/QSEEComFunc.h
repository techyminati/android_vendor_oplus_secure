/******************************************************************************
 * @file   QSEEComFunc.h
 * @brief  Contains QSEE communication functions header file.
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

#ifndef __QSEECOMFUNC_H__
#define __QSEECOMFUNC_H__

#include <dlfcn.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/ion.h>

#include "QSEEComAPI.h"

typedef struct qcom_km_ion_info {
    int32_t ion_fd;
    int32_t ifd_data_fd;
    struct ion_handle_data ion_alloc_handle;
    unsigned char *ion_sbuffer;
    uint32_t sbuf_len;
} qcom_km_ion_info_t;

typedef struct qsee_handle {
    void *_data;

    int (*QSEECom_start_app)(struct QSEECom_handle **handle, const char *path, const char *fname, uint32_t sb_size);
    int (*QSEECom_shutdown_app)(struct QSEECom_handle **handle);
    int (*QSEECom_load_external_elf)(struct QSEECom_handle **clnt_handle, const char *path, const char *fname);
    int (*QSEECom_unload_external_elf)(struct QSEECom_handle **handle);
    int (*QSEECom_register_listener)(struct QSEECom_handle **handle, uint32_t lstnr_id, uint32_t sb_length, uint32_t flags);
    int (*QSEECom_unregister_listener)(struct QSEECom_handle *handle);
    int (*QSEECom_send_cmd)(struct QSEECom_handle *handle, void *send_buf, uint32_t sbuf_len, void *rcv_buf, uint32_t rbuf_len);
    int (*QSEECom_send_modified_cmd_32)(struct QSEECom_handle *handle, void *send_buf, uint32_t sbuf_len,
                                        void *resp_buf, uint32_t rbuf_len, struct QSEECom_ion_fd_info *ifd_data);
    int (*QSEECom_receive_req)(struct QSEECom_handle *handle, void *buf, uint32_t len);
    int (*QSEECom_send_resp)(struct QSEECom_handle *handle, void *send_buf, uint32_t len);
    int (*QSEECom_set_bandwidth)(struct QSEECom_handle *handle, bool high);
    int (*QSEECom_app_load_query)(struct QSEECom_handle *handle, char *app_name);
    int (*QSEECom_get_app_info)(struct QSEECom_handle *handle, struct qseecom_app_info *info);
    int (*QSEECom_send_modified_cmd_64)(struct QSEECom_handle *handle, void *send_buf, uint32_t sbuf_len,
                                        void *resp_buf, uint32_t rbuf_len, struct QSEECom_ion_fd_info  *ifd_data);
    int (*QSEECom_start_app_V2)(struct QSEECom_handle **clnt_handle, const char *fname,
                                unsigned char *trustlet, uint32_t tlen, uint32_t sb_length);

    int32_t (*QCom_ion_free)(qcom_km_ion_info_t *handle);
    int32_t (*QCom_ion_alloc)(qcom_km_ion_info_t *handle, uint32_t size);
    int32_t (*QSEECom_load_trustlet)(struct qsee_handle *qsee_handle, struct QSEECom_handle **clnt_handle,
                                     const char *path, const char *fname, uint32_t sb_size);
    int (*QSEECom_send_modified_cmd)(struct QSEECom_handle *handle, void *send_buf, uint32_t sbuf_len,
                                     void *resp_buf, uint32_t rbuf_len, struct QSEECom_ion_fd_info *ifd_data);
} qsee_handle_t;

int32_t qsee_open_handle(qsee_handle_t **handle);
int32_t qsee_free_handle(qsee_handle_t **handle);
const char* qsee_error_strings(int32_t err);

#endif /* __QSEECOMFUNC_H__ */

