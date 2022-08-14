/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_SENSE_TOUCH_TYPES_H
#define FPC_HAL_SENSE_TOUCH_TYPES_H

#include <stdint.h>

#define FPC_SENSE_TOUCH_VERSION_1 1

typedef enum {
    FPC_SENSE_TOUCH_RAW = 0,
    FPC_SENSE_TOUCH_PRESS = 1,
    FPC_SENSE_TOUCH_AUTH_PRESS = 2,
} fpc_sense_touch_event_t;

typedef struct {
    uint32_t version;
    uint8_t ground;
    uint8_t trigger_threshold;
    uint8_t untrigger_threshold;
    uint8_t auth_enable_down_force;
    uint8_t auth_enable_up_force;
    uint32_t auth_button_timeout_ms;
} __attribute__((__packed__)) fpc_sense_touch_config_t;

#endif // FPC_HAL_SENSE_TOUCH_TYPES_H
