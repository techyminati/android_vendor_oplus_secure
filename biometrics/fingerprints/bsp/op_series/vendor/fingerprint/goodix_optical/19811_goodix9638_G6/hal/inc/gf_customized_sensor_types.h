#ifndef _GF_CUSTOMIZED_SENSOR_TYPES_H_
#define _GF_CUSTOMIZED_SENSOR_TYPES_H_

#include <stdint.h>
#include "gf_sensor_types.h"
#include "gf_delmar_product_test_types.h"


typedef enum CUSTOMIZED_SENSORCMD_ID {
    GF_CMD_SENSOR_SET_EXPOSURE_TIME = GF_CMD_SENSOR_MAX+ 2,
} gf_customized_sensor_cmd_id_t;

typedef struct GF_DELMAR_SET_EXPOSURE_TIME {
    gf_cmd_header_t header;
} gf_delmar_set_exposure_time_t;

#endif  // _GF_SENSOR_TYPES_H_
