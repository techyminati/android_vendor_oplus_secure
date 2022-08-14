/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description:
 * History:
 */

#ifndef _GF_BEANPOD_TYPE_DEFINE_H_
#define _GF_BEANPOD_TYPE_DEFINE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif    // #ifdef __cplusplus

// for beanpod tee
typedef struct
{
    uint32_t head1;
    uint32_t head2;
    uint32_t head3;
    uint32_t len;
} gf_beanpod_header_t;

typedef struct
{
    uint32_t cmd_id;
    uint32_t reverse[3];
} gf_beanpod_cmd_id_t;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus

#endif      // _GF_BEANPOD_TYPE_DEFINE_H_
