#ifndef _GF_SENSOR_TYPES_H_
#define _GF_SENSOR_TYPES_H_

#include "gf_base_types.h"
#include "gf_config_type.h"

typedef enum {
    GF_CMD_SENSOR_PRE_DETECT,
    GF_CMD_SENSOR_DETECT_SENSOR,
    GF_CMD_SENSOR_INIT,
    GF_CMD_SENSOR_CAPTURE_IMG,
    GF_CMD_SENSOR_DATA,
    GF_CMD_SENSOR_GET_IRQ_TYPE,
    GF_CMD_SENSOR_SET_CONFIG,
    GF_CMD_SENSOR_GET_LOGO_TIMES,
    GF_CMD_SNESOR_SLEEP,
    GF_CMD_SNESOR_WAKEUP,
    GF_CMD_SENSOR_MAX
} gf_sensor_cmd_id_t;

typedef struct gf_sensor_ids {
    uint32_t mcu_id;
    uint32_t pmic_id;
    uint32_t sensor_id;
    uint32_t sensor_version;
    uint32_t flash_id;
} gf_sensor_ids_t;

typedef struct gf_sensor_info {
    gf_chip_series_t chip_series;
    uint32_t chip_type;
    uint32_t row;
    uint32_t col;
} gf_sensor_info_t;

typedef struct {
    gf_cmd_header_t header;
    gf_sensor_info_t o_sensor_info;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_detect_sensor_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    gf_config_t o_config;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_sensor_init_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_retry_count;
    int32_t i_operation;
    int8_t o_fast_up;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_capture_image_t;

typedef struct {
    gf_cmd_header_t header;
    int32_t o_irq_type;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_get_irq_type_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint8_t o_logo_times;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_get_logo_times_t;

typedef struct {
    gf_cmd_header_t header;
    uint8_t o_need_reset;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_sensor_wakeup_t;

typedef gf_cmd_header_t gf_sensor_preinit_t;
typedef gf_cmd_header_t gf_sensor_set_config_t;
typedef gf_cmd_header_t gf_sensor_sleep_t;

#endif  // _GF_SENSOR_TYPES_H_
