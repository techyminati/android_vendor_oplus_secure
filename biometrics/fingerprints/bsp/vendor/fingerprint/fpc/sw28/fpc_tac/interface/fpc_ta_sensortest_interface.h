/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TA_SENSORTEST_INTERFACE_H
#define FPC_TA_SENSORTEST_INTERFACE_H

#include "fpc_ta_interface.h"

typedef enum {
    FPC_SENSORTEST_SELF_TEST,
    FPC_SENSORTEST_CHECKERBOARD_TEST,
    FPC_SENSORTEST_IMAGE_QUALITY_TEST,
    FPC_SENSORTEST_RESET_PIXEL_TEST,
    FPC_SENSORTEST_AFD_CALIBRATION_TEST,
    FPC_SENSORTEST_AFD_CAL_TEST,
    FPC_SENSORTEST_MODULE_QUALITY_TEST,
    FPC_SENSORTEST_DEFECTIVE_PIXELS_TEST,
    FPC_SENSORTEST_OTP_VALIDATION_TEST,
    FPC_SENSORTEST_TXPULSE_CHECKERBOARD_TEST,
    FPC_SENSORTEST_INVALID_TEST,
} fpc_sensortest_test_t;

typedef struct {
    uint32_t snr_limit_preset;
    float snr_limit;
    uint32_t snr_cropping_left;
    uint32_t snr_cropping_top;
    uint32_t snr_cropping_right;
    uint32_t snr_cropping_bottom;
    uint32_t snr_error;
} fpc_ta_sensortest_mqt_params_t;

typedef union {
    fpc_ta_sensortest_mqt_params_t mqt;
} fpc_ta_sensortest_test_params_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    fpc_sensortest_test_t test;
    fpc_ta_sensortest_test_params_t params;
    uint32_t image_captured;
    uint32_t result;
    uint32_t log_size;
    uint32_t alive_check;
} fpc_ta_sensortest_test_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    fpc_sensortest_test_t test;
} fpc_ta_sensortest_is_supported_cmd_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_sensortest_test_cmd_t test;
    fpc_ta_sensortest_is_supported_cmd_t is_test_supported;
    fpc_ta_cmd_header_t capture_uncalibrated;
    fpc_ta_byte_array_msg_t get_log;
} fpc_ta_sensortest_command_t;

typedef enum {
    FPC_TA_SENSORTEST_SELF_TEST_INIT,
    FPC_TA_SENSORTEST_SELF_TEST,
    FPC_TA_SENSORTEST_SELF_TEST_CLEANUP,
    FPC_TA_SENSORTEST_IS_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_RUN_TEST,
    FPC_TA_SENSORTEST_MODULE_QUALITY_TEST,
    FPC_TA_SENSORTEST_CAPTURE_UNCALIBRATED,
    FPC_TA_SENSORTEST_GET_LOG,
} fpc_ta_sensortest_cmd_t;

#endif // FPC_TA_SENSORTEST_INTERFACE_H

