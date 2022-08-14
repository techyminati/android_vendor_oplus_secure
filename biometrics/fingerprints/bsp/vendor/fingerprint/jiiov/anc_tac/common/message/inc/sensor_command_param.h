#ifndef __SENSOR_COMMAND_PARAM_H__
#define __SENSOR_COMMAND_PARAM_H__

#include "anc_type.h"

#define CHIP_PROTO_INFO_VERSION                  "V2_0"

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t image_size;
    uint32_t raw_bits;
    uint32_t image_serial_number;
    int32_t exposure_time;
    uint32_t exposure_reg_val;
    uint32_t env_light_value;
    uint32_t image_ready_time;
    int32_t capture_time;
    uint64_t timestamp;
} __attribute__((packed)) ANC_SENSOR_IMAGE_PARAM;

typedef struct {
    uint16_t addr;
    uint8_t data;
} __attribute__((packed)) ANC_SENSOR_REG_PARAM;

typedef struct {
    uint8_t retry0;
    uint8_t retry1;
    uint8_t retry2;
} __attribute__((packed)) ANC_SENSOR_FRAME_FUSION_PARAM;

typedef struct {
    uint8_t type;
    int32_t retry0;
    int32_t retry1;
    int32_t retry2;
} __attribute__((packed)) ANC_SENSOR_RETRY_EXPOSURE_TIME;

typedef struct {
    int32_t low_auto_expo_threshold;
    uint32_t low_env_light_threshold;
    uint32_t high_env_light_threshold;
} __attribute__((packed)) ANC_SENSOR_EXPO_CONFIG_THRESHOLD;

typedef struct {
    uint8_t x;
    uint8_t y;
} __attribute__((packed)) ANC_SENSOR_EFUSE_OPTICAL_CENTER;

typedef struct {
    uint8_t vendor_id;
    uint8_t materials_id[7];
    uint8_t date_year;
    uint8_t date_month;
    uint8_t date_day;
    uint8_t status_id;
    uint8_t serial_number[5];
    uint8_t assembly_line;
} __attribute__((packed)) ANC_SENSOR_EFUSE_CUSTOM_MODULE_INFO;

typedef struct {
    uint8_t module_integrator_id;
    uint8_t lens_id;
} __attribute__((packed)) ANC_SENSOR_EFUSE_MI_LENS_ID;

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
} __attribute__((packed)) ANC_SENSOR_EFUSE_MT_DATE;

typedef struct {
    ANC_SENSOR_EFUSE_MI_LENS_ID mi_lens_id;
    ANC_SENSOR_EFUSE_CUSTOM_MODULE_INFO custom_module_info;
    ANC_SENSOR_EFUSE_OPTICAL_CENTER optical_center;
    ANC_SENSOR_EFUSE_MT_DATE mt_date;
} __attribute__((packed)) ANC_SENSOR_EFUSE_MT_INFO;

typedef struct {
    uint32_t optical_center_x;
    uint32_t optical_center_y;
    uint32_t auto_exp_basesum;
} __attribute__((packed)) ANC_SENSOR_EFUSE_FT_INFO;

typedef struct {
    uint8_t chip_lot_id[4];
    uint8_t wafer_id;
    uint8_t die_x;
    uint8_t die_y;
    char chip_info[64]; //data format : LotNumber_WaferId_DieX_DieY
} __attribute__((packed)) ANC_SENSOR_EFUSE_CHIP_PROTO_INFO;

typedef struct {
    ANC_SENSOR_EFUSE_CHIP_PROTO_INFO chip_id;
} __attribute__((packed)) ANC_SENSOR_EFUSE_CP_INFO;

typedef struct {
    char product_id[16];
} __attribute__((packed)) ANC_SENSOR_PRODUCT_INFO;

typedef enum {
    ANC_SENSOR_NORMAL = 0,
    ANC_SENSOR_WAKEUP,
    ANC_SENSOR_SLEEP,
    ANC_SENSOR_POWER_ON,
    ANC_SENSOR_POWER_ON_INIT,
    ANC_SENSOR_POWER_OFF,
    ANC_SENSOR_HW_RESET,
    ANC_SENSOR_INIT,
    ANC_SENSOR_WAKEUP_RESET,
    ANC_SENSOR_HW_RESET_INIT,
    ANC_SENSOR_WAKEUP_SCAN,
}ANC_SENSOR_POWER_MODE;

typedef enum {
    ANC_SENSOR_CMD_SET_EXPOSURE_TIME = 0,
    ANC_SENSOR_CMD_GET_EXPOSURE_TIME,
    ANC_SENSOR_CMD_RESTORE_EXPOSURE_TIME,
    ANC_SENSOR_CMD_GET_EXPOSURE_REG_VAL,
    ANC_SENSOR_CMD_SET_IMAGE_SIZE,
    ANC_SENSOR_CMD_GET_IMAGE_SIZE,
    ANC_SENSOR_CMD_WRITE_REGISTER,
    ANC_SENSOR_CMD_READ_REGISTER,
    ANC_SENSOR_CMD_SET_FRAME_FUSION_NUM,
    ANC_SENSOR_CMD_VC_SETTING,
    ANC_SENSOR_CMD_SET_RETRY_EXPOSURE_TIME,
    ANC_SENSOR_CMD_SET_EXPO_CONFIG_THRESHOLD,
    ANC_SENSOR_CMD_GET_ENV_LIGHT_VAL,
    ANC_SENSOR_CMD_RESTORE_DEFAULT_IMAGE_SIZE,
    ANC_SENSOR_CMD_SET_CALIBRATION_MODE,
    ANC_SENSOR_CMD_SELF_TEST,
    ANC_SENSOR_CMD_GET_TOTAL_EXPOSURE_TIME,
    ANC_SENSOR_CMD_LOAD_CALIBRATION_DATA,
    ANC_SENSOR_CMD_SET_EXPO_TARGET_VALUE,
    ANC_SENSOR_CMD_GET_EXPO_TARGET_VALUE,
    ANC_SENSOR_CMD_GET_PRODUCT_ID,

    /* EFUSE Read/Write */
    ANC_SENSOR_CMD_EFUSE_READ_MT_OPTICAL_CENTER = 100,
    ANC_SENSOR_CMD_EFUSE_READ_MT_CUSTOM_MODULE_INFO,
    ANC_SENSOR_CMD_EFUSE_READ_MT_MI_LENS_ID,
    ANC_SENSOR_CMD_EFUSE_READ_FT_INFO,
    ANC_SENSOR_CMD_EFUSE_WRITE_FT_INFO,
    ANC_SENSOR_CMD_EFUSE_READ_CHIP_PROTO_INFO,
    ANC_SENSOR_CMD_EFUSE_CHECK,
    ANC_SENSOR_CMD_EFUSE_GET_MT1_OTP_DATA,
    ANC_SENSOR_CMD_EFUSE_GET_CP_OTP_DATA,

    ANC_SENSOR_CMD_MAX
}ANC_SENSOR_CMD_TYPE;

#define ANC_SENSOR_FIRST_EXPOSURE_BIT        0
#define ANC_SENSOR_FIRST_TRANSMIT_BIT        1
#define ANC_SENSOR_SECOND_EXPOSURE_BIT       2
#define ANC_SENSOR_FIXED_EXPO_FOR_RETRY_BIT  3
#define ANC_SENSOR_SKIP_SCAN_CONFIG_BIT      4

#define ANC_SENSOR_FIRST_EXPOSURE            (1U << ANC_SENSOR_FIRST_EXPOSURE_BIT)
#define ANC_SENSOR_FIRST_TRANSMIT            (1U << ANC_SENSOR_FIRST_TRANSMIT_BIT)
#define ANC_SENSOR_SECOND_EXPOSURE           (1U << ANC_SENSOR_SECOND_EXPOSURE_BIT)
#define ANC_SENSOR_FIXED_EXPO_FOR_RETRY      (1U << ANC_SENSOR_FIXED_EXPO_FOR_RETRY_BIT)
#define ANC_SENSOR_SKIP_SCAN_CONFIG          (1U << ANC_SENSOR_SKIP_SCAN_CONFIG_BIT)

typedef enum {
    ANC_SENSOR_NONE = 0,
    ANC_SENSOR_EXPOSURE = (ANC_SENSOR_FIRST_EXPOSURE),
    ANC_SENSOR_EXPOSURE_TRANSMIT = (ANC_SENSOR_FIRST_EXPOSURE | ANC_SENSOR_FIRST_TRANSMIT),
    ANC_SENSOR_EXPOSURE_TRANSMIT_EXPOSURE = (ANC_SENSOR_FIRST_EXPOSURE | ANC_SENSOR_FIRST_TRANSMIT | ANC_SENSOR_SECOND_EXPOSURE),
    ANC_SENSOR_TRANSMIT = (ANC_SENSOR_FIRST_TRANSMIT),
    ANC_SENSOR_TRANSMIT_EXPOSURE = (ANC_SENSOR_FIRST_TRANSMIT | ANC_SENSOR_SECOND_EXPOSURE),
}ANC_SENSOR_INNER_DATA_MODE;

typedef union {
    int32_t exposure_time;  // us
    ANC_SENSOR_IMAGE_PARAM image_param;
    ANC_SENSOR_REG_PARAM reg_param;
    ANC_SENSOR_FRAME_FUSION_PARAM fusion_param;
    ANC_SENSOR_RETRY_EXPOSURE_TIME retry_exposure_time;
    ANC_SENSOR_EFUSE_OPTICAL_CENTER optical_center;
    ANC_SENSOR_EFUSE_CUSTOM_MODULE_INFO custom_module_info;
    ANC_SENSOR_EFUSE_MI_LENS_ID mi_lens_id;
    ANC_SENSOR_EFUSE_FT_INFO ft_info;
    ANC_SENSOR_EFUSE_CHIP_PROTO_INFO chip_proto_info;
    ANC_SENSOR_EXPO_CONFIG_THRESHOLD expo_config_threshold;
    ANC_SENSOR_PRODUCT_INFO product_info;
    ANC_SENSOR_EFUSE_MT_INFO mt_info;
    ANC_SENSOR_EFUSE_CP_INFO cp_info;
    int vc_image_type;
    uint32_t env_light_value;
    uint8_t is_calibration_mode;
    uint8_t *p_calibration_data;
    uint32_t expo_target_value;
}ANC_SENSOR_CMD_PARAM;

typedef struct {
    ANC_SENSOR_CMD_TYPE type;
    ANC_SENSOR_CMD_PARAM data;
} __attribute__((packed)) AncSensorCommandParam;


#endif
