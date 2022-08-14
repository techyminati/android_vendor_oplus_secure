/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_SENSORTEST_H
#define FPC_TEE_SENSORTEST_H

#include <stdint.h>
#include "fpc_tee_sensor.h"
#include "fpc_ta_sensortest_interface.h"

int fpc_tee_sensortest_self_test(fpc_tee_sensor_t *sensor, uint32_t *result);

int fpc_tee_sensortest_is_test_supported(fpc_tee_sensor_t *sensor,
                                         fpc_sensortest_test_t test,
                                         int32_t *is_supported);
/**
 * Run a sensortest in the TA
 *
 * @param[in]     sensor                Handle to sensor.
 * @param[in]     test                  The test to execute.
 * @param[in]     stabilization_time_ms If > 0 wait for a finger down and then sleep this amount
 *                                      of time before executing the actual test.
 * @param[in,out] params                Test parameters. Note that this can contain output from
 *                                      the test.
 * @param[out]    image_captured        Returns !=0 if an image was captured to the image buffer and
 *                                      can be fetched through the engineering interface.See
 *                                      fpc_tee_debug_retrieve().
 * @param[out] result                   Test result, 0 if test passed, otherwise an error code
 *                                      specific to the test case.
 * @param[out] log_size                 Size of the produced log. See fpc_tee_sensortest_get_log().
 *                                      currently logs are only generated for tests that execute
 *                                      through the CTL.
 *
 * @return                              Returns 0 if test was executed successfully, otherwise an
 *                                      error code. The output parameters are only valid if this
 *                                      return value is 0.
 */
int fpc_tee_sensortest_run_test(fpc_tee_sensor_t **sensor,
                                fpc_sensortest_test_t test,
                                uint32_t stabilization_time_ms,
                                fpc_ta_sensortest_test_params_t *params,
                                uint32_t *image_captured,
                                uint32_t *result,
                                uint32_t *log_size);

int fpc_tee_sensortest_capture_uncalibrated(fpc_tee_sensor_t *sensor);

int fpc_tee_checkerboardtest(fpc_tee_sensor_t** sensor, uint32_t *result);

int fpc_tee_run_mqt_test(fpc_tee_sensor_t **sensor, uint32_t *result, double *snr_value);

/**
 * Fetch the log of the last test that was executed. Currently a log is only generated if the test
 * was executed through the CTL. This function is only available in engineering builds of the TA.
 *
 * @param[in]     sensor     Handle to sensor.
 * @param[in,out] log_size   In: The size of the buffer, this must be at least as big as the
 *                               log_size parameter returned from fpc_tee_sensortest_run_test()
 *                           out: the number of bytes fetched
 * @param[out]    log_buffer Buffer to receive log data, must be allocated and freed by the caller.
 *
 * @return                   Returns 0 if log was fetched successfully, otherwise an error code.
 */
int fpc_tee_sensortest_get_log(fpc_tee_sensor_t *sensor, uint32_t *size, uint8_t *log_buffer);
int fpc_tee_sensortest_force_defeat_pixel_test(fpc_tee_sensor_t* sensor, uint32_t *result);

#endif //FPC_TEE_SENSORTEST_H
