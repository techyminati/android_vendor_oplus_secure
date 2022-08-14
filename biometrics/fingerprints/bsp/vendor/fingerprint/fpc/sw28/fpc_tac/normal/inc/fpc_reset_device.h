/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_RESET_DEVICE_H
#define FPC_RESET_DEVICE_H

#include <unistd.h>
#include "fpc_tee.h"

typedef struct fpc_reset fpc_reset_t;

void fpc_reset_release(fpc_reset_t* device);

fpc_reset_t* fpc_reset_init(void);

#ifdef FPC_CONFIG_NORMAL_SPI_RESET
int32_t fpc_reset_spi(fpc_reset_t* device);
#endif

#ifdef FPC_CONFIG_NORMAL_SENSOR_RESET
int32_t fpc_reset_sensor(fpc_reset_t* device);
#endif

#endif // FPC_RESET_DEVICE_H
