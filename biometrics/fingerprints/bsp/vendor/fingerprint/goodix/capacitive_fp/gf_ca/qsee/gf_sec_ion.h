/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_sec ion header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_SEC_ION_H_
#define _GF_SEC_ION_H_

#include <gf_error.h>
#include <gf_type_define.h>
#include <sys/mman.h>
#include <linux/msm_ion.h>


#ifdef __cplusplus
extern "C"
{
#endif  // end ifdef __cplusplus

typedef struct qsc_ion_info
{
    int32_t ion_fd;
    int32_t ifd_data_fd;
#if TARGET_ION_ABI_VERSION < 2
    struct ion_handle_data ion_alloc_handle;
#endif  // #if TARGET_ION_ABI_VERSION < 2
    unsigned char *ion_sbuffer;
    uint32_t sbuf_len;
    bool ion_local_flag;
} qsc_ion_info_t;


int32_t gf_sec_ion_malloc(struct qsc_ion_info *handle, int32_t data_size);
void gf_sec_ion_free(struct qsc_ion_info *handle);
void gf_sec_ion_destroy();
void gf_sec_ion_init();

int32_t qsc_ion_memalloc(struct qsc_ion_info *handle, uint32_t size);  // extern call for ion test

#ifdef __cplusplus
}
#endif  // end ifdef __cplusplus

#endif      // _GF_SEC_ION_H_
