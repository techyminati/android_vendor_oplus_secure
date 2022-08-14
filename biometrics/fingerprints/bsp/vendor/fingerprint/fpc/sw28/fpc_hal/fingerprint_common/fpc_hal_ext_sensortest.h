/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_SENSORTEST_H
#define FPC_HAL_EXT_SENSORTEST_H

#include <stdbool.h>

#include "fpc_tee_hal.h"
#include "fpc_hal_ext_engineering.h"
#include "fpc_hw_identification_types.h"

#define FPC_HAL_EXT_SENSORTEST_TEST_RESULT_MAX 128
#define FPC_HAL_EXT_SENSORTEST_TEST_ERROR_MAX 64
#define FPC_HAL_EXT_SENSORTEST_TEST_NAME_MAX 64
#define FPC_HAL_EXT_SENSORTEST_TEST_DESCRIPTION_MAX 128
#define FPC_HAL_EXT_SENSORTEST_TEST_STAMP_TYPE_MAX 64
#define FPC_HAL_EXT_SENSORTEST_TEST_INPUT_MAX 256
#define FPC_HAL_EXT_SENSORTEST_TESTS_MAX 16

typedef enum {
    FPC_HAL_EXT_SENSORTEST_TEST_PASS = 0,
    FPC_HAL_EXT_SENSORTEST_TEST_FAIL,
    FPC_HAL_EXT_SENSORTEST_TEST_CANCELLED,
    FPC_HAL_EXT_SENSORTEST_TEST_NOT_SUPPORTED,
    FPC_HAL_EXT_SENSORTEST_TEST_ERROR,
} fpc_hal_ext_sensortest_test_result_code_t;

typedef struct {
    int result_code;
    char result_string[FPC_HAL_EXT_SENSORTEST_TEST_RESULT_MAX];
    int error_code;
    char error_string[FPC_HAL_EXT_SENSORTEST_TEST_ERROR_MAX];
    bool image_fetched;
    fpc_hal_img_data_t image_result;
    char *log;
} fpc_hal_ext_sensortest_test_result_t;

typedef void (*fpc_test_result_cb_t)(void *ctx, fpc_hal_ext_sensortest_test_result_t *result);
typedef void (*fpc_capture_acquired_cb_t)(void *ctx, int acquired_info);
typedef void (*fpc_capture_error_cb_t)(void *ctx, int error);

typedef struct {
    char name[FPC_HAL_EXT_SENSORTEST_TEST_NAME_MAX];
    char description[FPC_HAL_EXT_SENSORTEST_TEST_DESCRIPTION_MAX];
    bool wait_for_finger_down;
    char rubber_stamp_type[FPC_HAL_EXT_SENSORTEST_TEST_STAMP_TYPE_MAX];
} fpc_hal_ext_sensortest_test_t;

typedef struct {
    char test_limits_key_value_pair[FPC_HAL_EXT_SENSORTEST_TEST_INPUT_MAX];
} fpc_hal_ext_sensortest_test_input_t;

typedef struct {
    fpc_hal_ext_sensortest_test_t tests[FPC_HAL_EXT_SENSORTEST_TESTS_MAX];
    uint32_t size;
} fpc_hal_ext_sensortest_tests_t;

typedef struct fpc_hal_ext_sensortest fpc_hal_ext_sensortest_t;

struct fpc_hal_ext_sensortest {
    int (*get_sensor_info)(fpc_hal_ext_sensortest_t *self, fpc_hw_module_info_t *info);
    int (*get_sensor_tests)(fpc_hal_ext_sensortest_t *self,
                            fpc_hal_ext_sensortest_tests_t *sensor_tests);
    int (*run_sensor_test)(fpc_hal_ext_sensortest_t *self, fpc_hal_ext_sensortest_test_t *test,
                           fpc_hal_ext_sensortest_test_input_t *input, void *ctx,
                           fpc_test_result_cb_t result_cb);
    void (*cancel_sensor_test)(fpc_hal_ext_sensortest_t *self);
    int (*capture)(fpc_hal_ext_sensortest_t *self, bool wait_for_finger,
                   bool uncalibrated, void *ctx,
                   fpc_capture_acquired_cb_t acquired_cb,
                   fpc_capture_error_cb_t error_cb);
    void (*cancel_capture)(fpc_hal_ext_sensortest_t *self);
};

fpc_hal_ext_sensortest_t *fpc_sensortest_new(fpc_hal_common_t *hal);
void fpc_sensortest_destroy(fpc_hal_ext_sensortest_t *self);

#endif // FPC_HAL_EXT_SENSORTEST_H
