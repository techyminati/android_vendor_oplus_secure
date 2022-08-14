/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_ENGINEERING_H
#define FPC_TEE_ENGINEERING_H

#include <stdint.h>
#include "fpc_tee.h"

#define FPC_TEE_ENGINEERING_TYPE_RAW             0
#define FPC_TEE_ENGINEERING_TYPE_ENHANCED_IMAGE  1

int fpc_tee_debug_get_raw_size(
    fpc_tee_t *tee,
    uint32_t  *raw_size);

int fpc_tee_debug_retrieve(
    fpc_tee_t* tee,
    uint32_t type,
    void *buffer,
    size_t buffer_size);

int fpc_tee_get_sensor_info(
    fpc_tee_t* tee,
    uint8_t *width,
    uint8_t *height);

int fpc_tee_debug_inject(fpc_tee_t* tee,
                               const void* buffer,
                               size_t buffer_size);

#endif //FPC_TEE_SENSORTEST_H
