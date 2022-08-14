/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_NAVIGATION_H
#define FPC_HAL_EXT_NAVIGATION_H

#include "fpc_nav_types.h"

typedef struct fpc_navigation fpc_navigation_t;

struct fpc_navigation
{
    void (*set_config)(fpc_navigation_t* self, const fpc_nav_config_t* config);
    void (*get_config)(fpc_navigation_t* self, fpc_nav_config_t* config);
    void (*set_enabled)(fpc_navigation_t* self, bool enabled);
    bool (*get_enabled)(fpc_navigation_t* self);
};

#endif // FPC_HAL_EXT_NAVIGATION_H
