/************************************************************************************
 ** File: - fpc\fpc_tac\normal\src\fpc_tee_sensortest.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE SENSOR TEST for FPC (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/02/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>     <data>            <desc>
 **  Ziqing.guo   2017/10/21        create the file, modify for sensortest
 **  Ziqing.guo   2017/10/27        modify for wait finger down
 **  Ziqing.guo   2017/11/09        add the apis for engineering mode
 **  Ran.Chen    2018/03/15        add for SNR test
 ************************************************************************************/
#include <string.h>
#include "fpc_tac.h"
#include "fpc_log.h"

#include "fpc_tee_internal.h"
#include "fpc_tee_sensortest.h"

#include "fpc_ta_interface.h"
#include "fpc_ta_sensortest_interface.h"
#include "fpc_ta_targets.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_sensor_internal.h"
#include "fpc_tee.h"
#include "fingerprint_type.h"
#include "fpc_types.h"
#define SENSORTEST_STABILIZATION_MS 500
#define USECS_PER_MSEC              1000

#define _FPC_SELFTEST_IRQ_FAIL     6
#define _FPC_ERROR_SENSOR          7

int fpc_tee_sensortest_is_test_supported(fpc_tee_sensor_t *sensor,
                                         fpc_sensortest_test_t test,
                                         int32_t *is_supported) {
    LOGD("%s", __func__);

    int status = 0;
    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
    command->header.target = TARGET_FPC_TA_SENSORTEST;
    command->header.command = FPC_TA_SENSORTEST_IS_TEST_SUPPORTED;
    command->is_test_supported.test = test;

    *is_supported = 0;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (FAILED(status)) {
        goto out;
    }

    *is_supported = status;
    status = FPC_ERROR_NONE;

out:
    return status;
}

static int fpc_tee_sensortest_test_command(fpc_tee_sensor_t* sensor, int32_t command_id, uint32_t *result)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
    memset(command, 0, sizeof(*command));
    command->header.command   = command_id;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    command->test.result = *result; //send normal side irq-pin status to secure side
    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *result = command->test.result;
    return status;
}

int fpc_tee_checkerboardtest(fpc_tee_sensor_t** sensor, uint32_t *result) {
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;

    fpc_sensortest_test_t test = FPC_SENSORTEST_CHECKERBOARD_TEST;
    if (0 >= strcmp("F_1511", fp_config_info_init.fp_id_string)) {
        test = FPC_SENSORTEST_DEFECTIVE_PIXELS_TEST;
    } else {
        test = FPC_SENSORTEST_CHECKERBOARD_TEST;
    }
    int status = fpc_tee_sensortest_run_test(sensor,
                                             test,
                                             0,
                                             &params,
                                             &image_captured,
                                             result,
                                             &log_size);
    LOGD("%s: %d\n", __func__, *result);
    return status;
}

static int fpc_tee_sensortest_run_test_command(fpc_tee_sensor_t* sensor,
                                               fpc_sensortest_test_t test,
                                               fpc_ta_sensortest_test_params_t *params,
                                               uint32_t *image_captured,
                                               uint32_t *result,
                                               uint32_t *log_size,
                                               bool alive_check)

{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
    memset(command, 0, sizeof(*command));
    command->header.command   = FPC_TA_SENSORTEST_RUN_TEST;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    command->test.test        = test;
    command->test.params      = *params;
    command->test.alive_check = alive_check;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    *params         = command->test.params;
    *image_captured = command->test.image_captured;
    *result         = command->test.result;
    *log_size       = command->test.log_size;
    return status;
}
int fpc_tee_sensortest_force_defeat_pixel_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    fpc_ta_sensortest_test_params_t params;
    int status;
    memset(&params, 0, sizeof(params));
    uint32_t image_captured;
    uint32_t log_size;
    status = fpc_tee_sensortest_run_test_command(sensor,
                                     FPC_SENSORTEST_DEFECTIVE_PIXELS_TEST,
                                     &params,
                                     &image_captured,
                                     result,
                                     &log_size,
                                     true);
    return status;
}

int fpc_tee_sensortest_self_test(fpc_tee_sensor_t *sensor, uint32_t *result) {
    int status;
    uint32_t temp_result;

    LOGD("%s", __func__);

    status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_INIT, &temp_result);
    if (status) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_INIT failed! Status = %d", __func__, status);
        status = _FPC_ERROR_SENSOR;
        goto clean;
    }

    // Status is the value of irq pin, we expect 0 here after the init has been called
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ INITIAL -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status > 0) {
        LOGE("%s IRQ TEST READ INITIAL -> FPC_SELFTEST_IRQ_FAIL pin = %d should be 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_ERROR_NONE;
        goto clean;
    }

    // Do selftest including irq tests on secure-side
    status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST, result);

    // If failure on TA side skip REE side checks they will give nothing as we don't know IRQ status
    if (*result || status) {
        LOGE("%s Selftest failed status = %d result = %d", __func__, status, *result);
        goto clean;
    }

    // After selftest is executed on TA we expect an active IRQ -> pin high
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ AFTER -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status == 0) {
        LOGE("%s IRQ TEST READ AFTER -> FPC_SELFTEST_IRQ_FAIL pin = %d should be > 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_ERROR_NONE;
        goto clean;
    }

    if (fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_CLEANUP, &temp_result)) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_CLEANUP failed", __func__);
        status = _FPC_ERROR_SENSOR;
        goto out;
    }

    // After cleanup selftest is executed on TA we expect no active IRQ -> pin low
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ END -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status > 0) {
        LOGE("%s IRQ TEST READ END -> FPC_SELFTEST_IRQ_FAIL pin = %d should be 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_ERROR_NONE;
        goto clean;
    }

    goto out;

clean:
    if (fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_CLEANUP, &temp_result)) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_CLEANUP failed, ignoring", __func__);
    }

out:
    return status;
}

int fpc_tee_sensortest_run_test(fpc_tee_sensor_t **sensor,
                                fpc_sensortest_test_t test,
                                uint32_t stabilization_time_ms,
                                fpc_ta_sensortest_test_params_t *params,
                                uint32_t *image_captured,
                                uint32_t *result,
                                uint32_t *log_size)
{
    LOGD("%s test: %d", __func__, test);
    int status = 0;
    *log_size = 0;
    int finger_lost;

    if (stabilization_time_ms) {
        status = fpc_tee_wait_finger_down(sensor, SELFTEST_MODE, NULL);
        if (status) {
            LOGE("%s fpc_tee_wait_finger_down failed %d", __func__, status);
            goto out;
        }
        usleep(stabilization_time_ms * USECS_PER_MSEC);
    }

    switch (test) {
    case FPC_SENSORTEST_SELF_TEST:
        *image_captured = 0;
        status = fpc_tee_sensortest_self_test(*sensor, result);
        break;
    case FPC_SENSORTEST_CHECKERBOARD_TEST:
    case FPC_SENSORTEST_DEFECTIVE_PIXELS_TEST:
        finger_lost = fpc_tee_check_finger_lost(sensor);
        if (!finger_lost) {
            LOGE("%s: Please move off your finger from sensor!", __func__);
            status = -FPC_ERROR_TEST_FAILED;
            break;
        }
    default:
        status = fpc_tee_sensortest_run_test_command(*sensor,
                                                     test,
                                                     params,
                                                     image_captured,
                                                     result,
                                                     log_size, false);
        break;
    }

out:
    return status;
}

int fpc_tee_sensortest_capture_uncalibrated(fpc_tee_sensor_t *sensor)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
    command->header.command   = FPC_TA_SENSORTEST_CAPTURE_UNCALIBRATED;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

int fpc_tee_sensortest_get_log(fpc_tee_sensor_t *sensor, uint32_t *log_size, uint8_t *log_buffer)
{
    LOGD("%s", __func__);
    int status;

    if (!fpc_tee_engineering_enabled(sensor->tee)) {
        return -FPC_ERROR_NOT_SUPPORTED;
    }

    fpc_tee_t *tee = sensor->tee;
    size_t cmd_size = sizeof(fpc_ta_sensortest_command_t) + *log_size;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, cmd_size);
    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_sensortest_command_t *command = (fpc_ta_sensortest_command_t *) shared_buffer->addr;
    command->header.command   = FPC_TA_SENSORTEST_GET_LOG;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    command->get_log.size     = *log_size;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    if (log_buffer && (*log_size >= command->get_log.size)) {
        memcpy(log_buffer, command->get_log.array, command->get_log.size);
        *log_size = command->get_log.size;
    } else {
        status = -FPC_ERROR_PARAMETER;
        *log_size = 0;
    }

out:
    fpc_tac_free_shared(shared_buffer);

    return status;
}

int fpc_tee_run_mqt_test(fpc_tee_sensor_t **sensor, uint32_t *result, double *snr_value) {
    (void) sensor;
	(void) result;
	(void) snr_value;
	return 0;
#if 0
    LOGD("%s", __func__);
    int status = -FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED;
    uint32_t snr = 0;
    uint32_t snr_error = 0;
    int32_t is_supported = 0;

    status = fpc_tee_sensortest_is_test_supported(*sensor,
            FPC_TEE_SENSORTEST_MODULE_QUALITY_TEST,
            &is_supported);

    if(!is_supported) {
        LOGD("%s: FPC_TEE_SENSORTEST_MODULE_QUALITY_TEST not support", __func__);
        return FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED;
    }

    if (!status) {
        LOGD("%s: start FPC_TEE_SENSORTEST_MODULE_QUALITY_TEST", __func__);

        float snr_threshold = 0;
        uint32_t stablization_ms = 0;
        uint32_t snr_limit_preset = 0;
        uint32_t snr_cropping_left = 0;
        uint32_t snr_cropping_top = 0;
        uint32_t snr_cropping_right = 0;
        uint32_t snr_cropping_bottom = 0;
        if (1140 == fp_config_info_init.fp_ic_type) {
            snr_threshold = 3.0;
            stablization_ms = SENSORTEST_STABILIZATION_MS;
            snr_limit_preset = 0;
            snr_cropping_left = 0;
            snr_cropping_top = 0;
            snr_cropping_right = 0;
            snr_cropping_bottom = 0;
        } else {
            snr_threshold = 7.0;
            stablization_ms = SENSORTEST_STABILIZATION_MS;
            snr_limit_preset = 1;
            snr_cropping_left = 0;
            snr_cropping_top = 0;
            snr_cropping_right = 0;
            snr_cropping_bottom = 0;
        }
        status = fpc_tee_sensortest_run_module_quality_test(sensor,
                    stablization_ms,
                    snr_limit_preset,
                    snr_cropping_left,
                    snr_cropping_top,
                    snr_cropping_right,
                    snr_cropping_bottom,
                    result,
                    &snr,
                    &snr_error);

        if (!status) {
            *snr_value = (double) snr / 100;
            LOGD("%s: error:-%d snr: %f threshold: %f", __func__, snr_error, *snr_value, snr_threshold);

            if (*result == FPC_TA_SENSORTEST_TEST_OK) {
                LOGD("%s: fpc_tee_sensortest_run_test success !", __func__);
            } else {
                LOGD("%s: fpc_tee_sensortest_run_test error result : %d, result %d", __func__, status, *result);
                goto out;
            }
        } else {
            LOGD("%s: fpc_tee_sensortest_run_test error status : %d, result %d", __func__, status, *result);
        }
    }
out:
    LOGD("%s: finished with status: %d result: %d", __func__, status, *result);
    return status;
#endif
}

