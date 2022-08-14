#ifndef _GF_CUSTOMIZED_SENSOR_TYPES_H_
#define _GF_CUSTOMIZED_SENSOR_TYPES_H_

#include <stdint.h>
#include "gf_sensor_types.h"
#include "gf_delmar_product_test_types.h"

#define SPMT_PASS_FLAG_FILE_NAME "gf_spmt_pass.so"
#define SPMT_PASS_FLAG_BAK_FILE_NAME "gf_spmt_pass_bak.so"
#define SPMT_PASS_FLAG_UNLOCK_FILE_NAME "gf_spmt_pass_unlock.so"

typedef enum CUSTOMIZED_SENSORCMD_ID {
    GF_CMD_SENSOR_GET_SPMT_PASS_FLAG = GF_CMD_SENSOR_MAX+ 1,
    GF_CMD_SENSOR_SET_EXPOSURE_TIME = GF_CMD_SENSOR_MAX+ 2,
} gf_customized_sensor_cmd_id_t;

enum {
    GF_CMD_TEST_SAVE_SPMT_PASS_FLAG = GF_CMD_TEST_MAX + 1
};

typedef struct {
    gf_cmd_header_t cmd_header;
} gf_delmar_save_pass_flag_cmd_t;

typedef struct GF_DELMAR_GET_SPMT_PASS_FLAG {
    gf_cmd_header_t header;
    uint32_t  spmt_pass;
} gf_delmar_get_spmt_pass_flag_t;

typedef struct GF_DELMAR_SET_EXPOSURE_TIME {
    gf_cmd_header_t header;
} gf_delmar_set_exposure_time_t;

#endif  // _GF_SENSOR_TYPES_H_
