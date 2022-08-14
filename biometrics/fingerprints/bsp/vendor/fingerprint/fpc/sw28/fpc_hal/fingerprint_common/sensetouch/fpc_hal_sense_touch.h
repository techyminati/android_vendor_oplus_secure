/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_SENSE_TOUCH_H
#define FPC_HAL_SENSE_TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "fpc_hal_sense_touch_types.h"

/*
 * Load the sense touch configuration from the file system.
 *
 * @param[]
 *
 * @return 0 on success else error code.
 */
int32_t fpc_sense_touch_load_config(void);

/*
 * This operation is used to fetch the active sense touch configuration.
 *
 * @param[]
 *
 * @return The active sense touch configuration on success else NULL.
 */
const fpc_sense_touch_config_t *fpc_sense_touch_get_config(void);

/*
 * This operation is used to set the current force mode for authentication.
 *
 * @param[in] enable_down_force -
 * @param[in] enable_up_force -
 * @param[in] button_timeout_ms -
 *
 */
void fpc_sense_touch_set_auth_mode(bool enable_down_force,
                                   bool enable_up_force,
                                   uint32_t button_timeout_ms);

#endif // FPC_HAL_SENSE_TOUCH_H
