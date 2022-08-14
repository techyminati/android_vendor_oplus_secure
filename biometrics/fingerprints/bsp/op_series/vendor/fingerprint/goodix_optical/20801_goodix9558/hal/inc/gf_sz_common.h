/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_SZ_COMMON_H_
#define _GF_SZ_COMMON_H_

#define MAX_EXPOSURE_TIME 1000

#define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))

typedef struct GF_CFG {
    uint16_t addr;
    uint16_t val;
} gf_cfg_t;

typedef enum GF_W_UPDATE_FLAG {
    NO_NEED_UPDATE = 0,
    NEED_UPDATE = 1,
    UPDATE_BOOTCODE = 2,
    UPDATE_APP = 3,
    UPDATE_SUCCESS = 4,
} gf_fw_update_flag_t;

#define WAIT_RETRY_COUNT          100
#define RAW_DATA_PACKAGE_LEN      7168  // 7K for ech data area

#define ISP_DATA_MAX_LEN          (1024*4)
#define BOOTCODE_DATA_MAX_LEN     (1024*2)
#define APP_DATA_MAX_LEN          (1024*48)
typedef enum GF_FW_TYPE_VER {
    APP_VER = 0,
    BOOTCODE_VER = 1,
} gf_fw_type_ver_t;

typedef struct GF_FW_HEAD {
    uint8_t FW_ID;
    uint32_t fw_len;
    uint16_t flash_addr;
    uint8_t reserve;
} __attribute__((packed)) gf_fw_head_t;

typedef struct GF_FW_DATA {
    uint32_t total_len;
    uint16_t checksum;
    uint8_t BOOTCODE_VER[6];
    uint8_t APP_VER[14];
    uint8_t FW_NUM;
    uint8_t chip_type;
    uint8_t protocol_ver;
    uint8_t reserve[3];
    gf_fw_head_t isp_head;
    gf_fw_head_t bootcode_head;
    gf_fw_head_t app_head;
    uint8_t isp_code[ISP_DATA_MAX_LEN];
    uint8_t bootcode_code[BOOTCODE_DATA_MAX_LEN];
    uint8_t app_code[APP_DATA_MAX_LEN];
} gf_fw_data_t;

typedef struct GF_FW_LOGO_JUDGE {
    uint16_t package_len;
    uint16_t reg_num;
    uint16_t logo_x_start;
    uint16_t logo_y_start;
    uint16_t logo_row_len;
    uint16_t logo_col_len;
    uint16_t logo_max_judge_num;
    uint16_t check_sum;
}
__attribute__((packed)) gf_fw_logo_judge_t;

typedef struct GF_FW_IDS {
    uint32_t mcu_id;
    uint16_t pmic_id;
    uint16_t sensor_id;
    uint16_t sensor_version;
    uint32_t flash_id;
}
__attribute__((packed)) gf_fw_ids_t;

#endif  // _GF_SZ_FW_H_
