/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_INPUT_DEVICE_H
#define FPC_HAL_INPUT_DEVICE_H

#include <stdint.h>

#define FPC_HAL_INPUT_KEY_UP 0
#define FPC_HAL_INPUT_KEY_DOWN 1

/* This enum defines the internal event types can be used to trigger linux input device events.
   This enum in combination with a trigger_event is used to create a unique internal identifier
   which is used to generate a specific linux input event. See func report_input_event for usage.
   To add a new event type simply rename the next available placeholder. */
typedef enum {
    FPC_NAV_EVENT = 0x00010000,
    FPC_SENSE_TOUCH_EVENT = 0x00020000,
    FPC_HOMEKEY_EVENT     = 0x00040000,
#ifdef SIDE_FPC_ENABLE
    FPC_CAMERAKEY_EVENT = 0x00080000,
#else
    FPC_PLACEHOLDER_EVENT_D = 0x00080000,
#endif
    FPC_PLACEHOLDER_EVENT_E = 0x00100000,
    FPC_PLACEHOLDER_EVENT_F = 0x00200000,
    FPC_PLACEHOLDER_EVENT_G = 0x00400000,
    FPC_PLACEHOLDER_EVENT_H = 0x00800000,
    FPC_PLACEHOLDER_EVENT_I = 0x01000000,
    FPC_PLACEHOLDER_EVENT_J = 0x02000000,
    FPC_PLACEHOLDER_EVENT_K = 0x04000000,
    FPC_PLACEHOLDER_EVENT_L = 0x08000000,
    FPC_PLACEHOLDER_EVENT_M = 0x10000000,
    FPC_PLACEHOLDER_EVENT_N = 0x20000000,
    FPC_PLACEHOLDER_EVENT_O = 0x40000000,
    FPC_PLACEHOLDER_EVENT_P = 0x80000000,
} trigger_event_type_t;

/*
 * Creates a new input device that can genrate linux key events.
 *
 * @param[] void
 *
 * @return 0 on success else error code.
 */
int32_t create_input_device(void);

/*
 * Destroy a created input device.
 *
 * @param[] void
 *
 * @return 0 on success else error code.
 *         FPC_ERROR_INPUT
 *         FPC_ERROR_IO
 */
int32_t destroy_input_device(void);

/*
 * Report a linux device input event over the created input device.
 *
 * @param[in] trigger_event_type - Type of the triggering event, see trigger_event_type_t.
 * @param[in] trigger_event      - The interanl event triggering the input device to generate a
 *                                 linux event.
 * @param[in] event_value        - For key events FPC_HAL_INPUT_KEY_DOWN(1) or
 *                                 FPC_HAL_INPUT_KEY_UP(0) or actual value for relative or absolute
 *                                 movement events.
 *
 * @return 0 on success else error code.
 */
int32_t report_input_event(trigger_event_type_t trigger_event_type, uint32_t trigger_event,
                           int32_t event_value);

#endif // FPC_HAL_INPUT_DEVICE_H
