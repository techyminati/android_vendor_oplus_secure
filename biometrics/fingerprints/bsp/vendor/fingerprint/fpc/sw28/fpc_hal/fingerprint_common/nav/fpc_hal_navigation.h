/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_NAVIGATION_H
#define FPC_HAL_NAVIGATION_H

#include "fpc_hal_ext_navigation.h"
#include "fpc_tee_hal.h"

fpc_navigation_t *fpc_navigation_new(fpc_hal_common_t *hal);
void fpc_navigation_destroy(fpc_navigation_t *self);

void fpc_navigation_resume(fpc_navigation_t *self);
void fpc_navigation_pause(fpc_navigation_t *self);

#endif // FPC_HAL_NAVIGATION_H

