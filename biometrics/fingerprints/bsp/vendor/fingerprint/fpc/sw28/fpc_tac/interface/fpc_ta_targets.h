/*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef __FPC_TA_TARGETS_H__
#define __FPC_TA_TARGETS_H__

#include <stdbool.h>
#include <stdint.h>
#include "fpc_log.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_sensor_interface.h"
#include "fpc_ta_bio_interface.h"
#include "fpc_ta_kpi_interface.h"
#ifdef FPC_CONFIG_NAVIGATION
#include "fpc_ta_navigation_interface.h"
#endif
#ifdef FPC_CONFIG_HW_AUTH
#include "fpc_ta_hw_auth_interface.h"
#endif
#ifdef FPC_CONFIG_ENGINEERING
#include "fpc_ta_engineering_interface.h"
#endif
#include "fpc_ta_fido_auth_interface.h"
#ifdef FPC_CONFIG_SENSORTEST
#include "fpc_ta_sensortest_interface.h"
#endif

//#define TARGET_FPC_TA 1 //depricated
//#define TARGET_FPC_TA_FS 2
#define TARGET_FPC_TA_HW_AUTH 3
#define TARGET_FPC_TA_DB_BLOB 4
#define TARGET_FPC_TA_SENSORTEST 5
#define TARGET_FPC_TA_ENGINEERING 6
#define TARGET_FPC_TA_FIDO_AUTH 7
#define TARGET_FPC_TA_NAVIGATION 8
#define TARGET_FPC_TA_KPI 9
#define TARGET_FPC_TA_SENSOR     10
#define TARGET_FPC_TA_BIO       11
#define TARGET_FPC_TA_COMMON       12

#ifdef SIDE_FPC_ENABLE
#define MAX_CHUNK ((1024*1024) - sizeof(fpc_ta_bio_command_t))
#endif

union _fpc_ta_target_commands
{
    fpc_ta_bio_command_t bio_command;
    fpc_ta_sensor_command_t sensor_command;
    fpc_ta_kpi_command_t kpi_command;
#ifdef FPC_CONFIG_NAVIGATION
    fpc_ta_navigation_command_t nav_command;
#endif
#ifdef FPC_CONFIG_HW_AUTH
    fpc_ta_hw_auth_command_t auth_command;
#endif
#ifdef FPC_CONFIG_ENGINEERING
    fpc_ta_engineering_command_t eng_command;
#endif
    fpc_ta_fido_auth_command_t fido_auth_command;

#ifdef FPC_CONFIG_SENSORTEST
    fpc_ta_sensortest_command_t sensortest_command;
#endif
};

#define MAX_COMMAND_SIZE ( sizeof (union _fpc_ta_target_commands) )

#define shared_cast_to(_type, _buffer, _size) \
    (sizeof(_type) <= _size ? (_type*) _buffer : NULL)

static inline bool byte_array_valid(fpc_ta_byte_array_msg_t* msg, uint32_t size_buffer) {
    uint32_t message_size = (uint32_t) sizeof(*msg);
    uint32_t payload_size = msg->size;
    uint32_t total_size   = message_size + payload_size;

    if (message_size > size_buffer) {
        // De-referencing msg->size would read outside of buffer
        LOGE("%s Insufficient command buffer buffer size", __func__);
        return false;
    }

    if (message_size > total_size ) {
        LOGE("%s Command buffer payload size caused arithmetic overflow", __func__);
        return false;
    }

    if (total_size > size_buffer) {
        LOGE("%s Insufficient command buffer size", __func__);
        return false;
    }
    return true;
}

static inline bool byte_array_string_valid(fpc_ta_byte_array_msg_t* msg,
                                             uint32_t size_buffer)
{

    if (!byte_array_valid(msg, size_buffer)) {
        return false;
    }

    char* string = (char*) msg->array;
    for (char* c = string; c != string + msg->size; ++c) {
        if (*c == '\0') {
            return true;
        }
    }

    return false;
}

#endif //__FPC_TA_TARGETS_H__
