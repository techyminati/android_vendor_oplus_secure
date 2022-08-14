#ifndef _GF_CUSTOMIZED_SENSOR_TYPES_H_
#define _GF_CUSTOMIZED_SENSOR_TYPES_H_

#include <stdint.h>
#include "gf_sensor_types.h"
#include "gf_delmar_product_test_types.h"


typedef enum CUSTOMIZED_SENSORCMD_ID {
    GF_CMD_SENSOR_GET_QR_CODE = GF_CMD_SENSOR_MAX+ 1,
    GF_CMD_SENSOR_SET_EXPOSURE_TIME = GF_CMD_SENSOR_MAX+ 2,
} gf_customized_sensor_cmd_id_t;

typedef struct GF_DELMAR_SET_EXPOSURE_TIME {
    gf_cmd_header_t header;
} gf_delmar_set_exposure_time_t;

typedef struct GF_DELMAR_GET_QR_CODE {
    gf_cmd_header_t header;
    uint8_t  otp_qr_info[11];
} gf_delmar_get_qr_code_t;

#endif  // _GF_SENSOR_TYPES_H_
