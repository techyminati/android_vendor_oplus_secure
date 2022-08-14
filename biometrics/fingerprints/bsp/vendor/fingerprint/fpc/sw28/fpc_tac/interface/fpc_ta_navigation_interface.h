/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TA_NAVIGATION_INTERFACE_H
#define FPC_TA_NAVIGATION_INTERFACE_H

#include "fpc_ta_interface.h"
#include "fpc_nav_types.h"

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t nav_event;
    int32_t force;
    int32_t finger_down;
    int32_t request;
} fpc_ta_nav_poll_data_cmd_t;


typedef struct {
    fpc_ta_cmd_header_t header;
} fpc_ta_nav_void_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t            finger_up;
} fpc_ta_navigation_finger_det_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    fpc_nav_config_t config;
} fpc_ta_nav_config_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t arg_1;
} fpc_ta_nav_uint32_cmd_t;

typedef union {
    fpc_ta_cmd_header_t header;

    fpc_ta_nav_void_cmd_t init;
    fpc_ta_nav_void_cmd_t exit;
    fpc_ta_nav_poll_data_cmd_t poll_data;
    fpc_ta_nav_config_cmd_t set_config;
    fpc_ta_nav_config_cmd_t get_config;
	fpc_ta_nav_uint32_cmd_t get_debug_buffer_size;
    fpc_ta_byte_array_msg_t get_debug_buffer;
    fpc_ta_navigation_finger_det_cmd_t finger_detect;
} fpc_ta_navigation_command_t;


enum {
    FPC_TA_NAVIGATION_INIT = 1,
    FPC_TA_NAVIGATION_EXIT = 2,
    FPC_TA_NAVIGATION_POLL_DATA = 3,
    FPC_TA_NAVIGATION_SET_CONFIG = 4,
    FPC_TA_NAVIGATION_GET_CONFIG = 5,
    FPC_TA_NAVIGATION_GET_DEBUG_BUFFER_SIZE = 6,
    FPC_TA_NAVIGATION_GET_DEBUG_BUFFER = 7,
    FPC_TA_NAVIGATION_FINGER_DETECT = 8,
};

#endif //FPC_TA_NAVIGATION_INTERFACE_H
