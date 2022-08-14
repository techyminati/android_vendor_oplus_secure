/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TA_ENGINEERING_INTERFACE_H
#define FPC_TA_ENGINEERING_INTERFACE_H

#include "fpc_ta_interface.h"

#define FPC_TA_ENGINEERING_TYPE_RAW            0
#define FPC_TA_ENGINEERING_TYPE_ENHANCED_IMAGE 1

typedef struct {
    fpc_ta_cmd_header_t header;
    uint8_t width;
    uint8_t height;
} fpc_ta_engineering_get_sensor_info_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t type;
    uint32_t size;
    uint8_t array[];
} fpc_ta_engineering_retrieve_t;

typedef union {
    fpc_ta_cmd_header_t                  header;
    fpc_ta_engineering_retrieve_t        retrieve;
    fpc_ta_byte_array_msg_t              inject;
    fpc_ta_engineering_get_sensor_info_t sensor_info;
    fpc_ta_size_msg_t                    raw_size;
} fpc_ta_engineering_command_t;

enum {
    FPC_TA_ENGINEERING_RETRIEVE         = 1,
    FPC_TA_ENGINEERING_GET_SENSOR_INFO  = 2,
    FPC_TA_ENGINEERING_INJECT_RAW       = 3,
    FPC_TA_ENGINEERING_GET_RAW_SIZE     = 4,
};
#endif // FPC_TA_ENGINEERING_INTERFACE_H
