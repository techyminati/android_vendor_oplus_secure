/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_SENSE_TOUCH_H
#define FPC_HAL_EXT_SENSE_TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "fpc_tee_hal.h"
#include "fpc_hal_sense_touch_types.h"

typedef struct fpc_sense_touch fpc_sense_touch_t;

/* This struct defines the sense touch application API. */
struct fpc_sense_touch {
    /*
     * -
     *
     * @param[in] self -
     * @param[out] force -
     *
     * @return
     */
    int (*get_force)(fpc_sense_touch_t *self, uint8_t *force);

    /*
     * -
     *
     * @param[in] self -
     * @param[out] result -
     *
     * @return
     */
    int (*is_supported)(fpc_sense_touch_t *self, uint8_t *result);

    /*
     * -
     *
     * @param[in] ground -
     * @param[in] threshold -
     *
     * @return
     */
    int (*store_calibration_data)(uint8_t ground, uint8_t threshold);

    /*
      * -
      *
      * @param[in] self -
      * @param[in] ground -
      * @param[in] threshold -
      *
      * @return
      */
    int (*set_auth_mode)(bool enable_down_force, bool enable_up_force, uint32_t button_timeout_ms);

    /*
     * This function is used to read the sense touch config from the file system.
     *
     * @param[out] st_config - config containing the current sense touch configuration.
     *
     * @return 0 on success otherwise negated error code FPC_ERROR_NOENTITY.
     */
    int32_t (*read_config)(const fpc_sense_touch_config_t **st_config);
};

/*
 * Create a new sense touch extension module.
 *
 * @param[in] hal - hal module data structure.
 *
 * @return A sense touch extension handle on success else NULL.
 */
fpc_sense_touch_t *fpc_sense_touch_new(fpc_hal_common_t *hal);

/*
 * Destory an existing sense touch extension module.
 *
 * @param[in] self - Handle to the sense touch extension module.
 *
 */
void fpc_sense_touch_destroy(fpc_sense_touch_t *self);

#endif // FPC_HAL_EXT_SENSE_TOUCH_H
