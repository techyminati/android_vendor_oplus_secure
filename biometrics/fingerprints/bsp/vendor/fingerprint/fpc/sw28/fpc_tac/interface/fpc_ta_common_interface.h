/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_COMMON_INTERFACE
#define INCLUSION_GUARD_FPC_TA_COMMON_INTERFACE

#include "fpc_ta_interface.h"

#define MAX_BUILDINFO_SIZE 1024

typedef enum {
    FPC_TA_COMMON_ENGINEERING_ENABLED = 0,
    FPC_TA_COMMON_SENSORTEST_ENABLED,
    FPC_TA_COMMON_NAVIGATION_ENABLED,
    FPC_TA_COMMON_LOG_TO_FILE_ENABLED,
    FPC_TA_COMMON_FORCE_SENSOR_ENABLED,
    FPC_TA_COMMON_NAVIGATION_FORCE_SW_ENABLED,
} fpc_ta_common_feature_enabled_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t enabled_features;
    uint32_t log_to_file_buffer_size;
    uint32_t max_number_of_templates;
    uint32_t size;
    char array[MAX_BUILDINFO_SIZE];
} fpc_ta_common_build_info_msg_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t size;
    char array[128];
} fpc_ta_common_lib_version_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_byte_array_msg_t error_msg;
    fpc_ta_common_build_info_msg_t build_info;
    fpc_ta_common_lib_version_t lib_version;
} fpc_ta_common_command_t;

typedef enum {
    FPC_TA_COMMON_GET_LOG_CMD,
    FPC_TA_COMMON_GET_BUILD_INFO_CMD,
    FPC_TA_COMMON_GET_LIB_VERSION_CMD,
} fpc_ta_common_cmd_t;

#endif /* INCLUSION_GUARD_FPC_TA_COMMON_INTERFACE */
