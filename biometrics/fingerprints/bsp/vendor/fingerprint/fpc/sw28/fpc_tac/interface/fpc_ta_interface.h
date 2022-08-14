/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_INTERFACE
#define INCLUSION_GUARD_FPC_TA_INTERFACE

#include <stdint.h>

//max size for t-base
#ifndef SIDE_FPC_ENABLE
#define MAX_CHUNK ((1024*1024) - sizeof(fpc_ta_byte_array_msg_t))
#endif



typedef struct {
    int32_t target;
    int32_t command;
} fpc_ta_cmd_header_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t size;
    uint8_t array[];
} fpc_ta_byte_array_msg_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t size;
} fpc_ta_size_msg_t;

/**
 * @brief Used to specify capture mode for @ref fpc_tee_capture_image
 *
 */
typedef enum {
    FPC_TEE_CAPTURE_MODE_DEFAULT,       /**< Use CAC fasttap to capture image   **/
    FPC_TEE_CAPTURE_MODE_SWIPE_ENROLL,  /**< CAC fasttap with one image capture **/
} fpc_tee_capture_mode_t;

#endif // INCLUSION_GUARD_FPC_TA_INTERFACE
