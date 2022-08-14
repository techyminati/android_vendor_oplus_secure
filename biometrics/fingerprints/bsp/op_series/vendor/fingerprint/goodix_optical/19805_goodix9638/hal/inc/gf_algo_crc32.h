/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_ALGO_CRC32_H_
#define _GF_ALGO_CRC32_H_

#include <stdint.h>

typedef struct {
    uint32_t crc;
} gf_crc32_context_t;


void gf_algo_crc32_init(gf_crc32_context_t *ctx);

void gf_algo_crc32_update(gf_crc32_context_t *ctx, const unsigned char *data,
                          int32_t len);

void gf_algo_crc32_final(gf_crc32_context_t *ctx, unsigned char *md);


#endif      // _GF_ALGO_CRC32_H_
