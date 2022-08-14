/************************************************************************************
 ** File: - fpc\fpc_hal\tee_hal\sensortest\fpc_hal_ext_sensortest.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint SENSORTEST FOR FPC (SW23)
 **
 ** Version: 1.0
 ** Date created: 11:11:11,12/03/2018
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 ** 	<author>	    <data>			    <desc>
 **    Ziqing.guo     2018/03/12        create the file, add fix for problem for coverity 39426 (Uninitialized scalar variable (UNINIT))
 **    Yang.tan       2019/01/05        modify coverity error for sw28
 ************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _BSD_SOURCE
#include <bsd/string.h> //strlcpy on Ubuntu
#endif

#include "fpc_types.h"
#include "fpc_hal_ext_sensortest.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "container_of.h"

#include "fpc_ta_sensortest_interface.h"
#include "fpc_tee_sensortest.h"
#include "fpc_hal_ext_engineering.h"
#include "fpc_tee_hal.h"

#define STRLEN(s) (s == NULL ? 0 : strlen(s))
#define STRNCMP(s1, s2) strncmp(s1, s2, strlen(s2))
#define STABILIZATION_MS 500
#define USECS_PER_MSEC 1000

// Test cases
#define SELF_TEST_NAME "Self Test"
#define CHECKERBOARD_TEST_NAME "Checkerboard Test"
#define IMAGE_QUALITY_TEST_NAME "Image Quality Test"
#define RESET_PIXEL_TEST_NAME "Image Reset Pixel Test"
#define AFD_CALIBRATION_TEST_NAME "AFD Calibration Test"
#define AFD_CAL_TEST_NAME "AFD Cal Test"
#define MODULE_QUALITY_TEST_NAME "Module Quality Test"
#define OTP_VALIDATION_TEST_NAME "OTP Validation Test"
#define DEFECTIVE_PIXELS_TEST_NAME "Defective Pixels Test"
#define TXPULSE_CHECKERBOARD_TEST_NAME "TXPulse Checkerboard Test"


// Test description
#define SELF_TEST_DES "Tests the sensor response and interrupt signal"
#define CHECKERBOARD_TEST_DES "Takes an sensor internal image and analyze to find dead pixels"
#define IMAGE_QUALITY_TEST_DES "Takes an image with square pattern stamp and analyze to find coating issues"
#define RESET_PIXEL_TEST_DES "Takes an sensor internal image and analyze to find faulty pixels"
#define AFD_CALIBRATION_TEST_DES "Tests the calibration of AFD"
#define AFD_CAL_TEST_DES "Validates analog finger detection function of sensor module."
#define MODULE_QUALITY_TEST_DES "Takes an image with zebra pattern stamp and determine the SNR (signal to noise ratio) value"
#define OTP_VALIDATION_TEST_DES "Checks the OTP memory for manufacturing defects and the wafer contains relevant information"
#define DEFECTIVE_PIXELS_TEST_DES "Verifies that the number of pixel errors or dead pixels are within limits"
#define TXPULSE_CHECKERBOARD_TEST_DES "Takes an sensor internal image and analyze to find dead pixels (TXPULSE)"

// Stamp type

#define IMAGE_QUALITY_TEST_STAMP "Square pattern stamp"
#define MODULE_QUALITY_TEST_STAMP "Zebra pattern stamp"

typedef struct {
    fpc_hal_ext_sensortest_t sensortest;

    fpc_hal_ext_sensortest_test_t test;
    fpc_hal_ext_sensortest_test_input_t test_input;
    void *test_cb_ctx;
    fpc_test_result_cb_t test_result_cb;

    bool wait_for_finger;
    bool uncalibrated;
    void *capture_cb_ctx;
    fpc_capture_acquired_cb_t capture_acquired_cb;
    fpc_capture_error_cb_t capture_error_cb;

    fpc_hal_common_t *hal;
    fpc_engineering_t *engineering;
} sensortest_module_t;

static int get_mqt_config(sensortest_module_t *module,
                          fpc_ta_sensortest_test_params_t *params,
                          uint32_t *stabilization_time_ms);

typedef int (*get_test_config_t)(sensortest_module_t *module,
                                 fpc_ta_sensortest_test_params_t *params,
                                 uint32_t *stabilization_time_ms);

#define TEST_ENTRY(test) \
{ \
    FPC_SENSORTEST_ ## test, \
    test ## _NAME, \
    test ## _DES, \
    NULL, \
    false, \
    NULL, \
}

#define TEST_ENTRY_STAMP(test, get_config) \
{ \
    FPC_SENSORTEST_ ## test, \
    test ## _NAME, \
    test ## _DES, \
    test ## _STAMP, \
    true, \
    get_config, \
}

const struct {
    fpc_sensortest_test_t test;
    const char *name;
    const char *desc;
    const char *stamp;
    bool wait_for_finger;
    get_test_config_t get_test_config;
} fpc_sensortest_tests[] = {
    TEST_ENTRY(SELF_TEST),
    TEST_ENTRY(CHECKERBOARD_TEST),
    TEST_ENTRY_STAMP(IMAGE_QUALITY_TEST, NULL),
    TEST_ENTRY(RESET_PIXEL_TEST),
    TEST_ENTRY(AFD_CALIBRATION_TEST),
    TEST_ENTRY(AFD_CAL_TEST),
    TEST_ENTRY_STAMP(MODULE_QUALITY_TEST, get_mqt_config),
    TEST_ENTRY(DEFECTIVE_PIXELS_TEST),
    TEST_ENTRY(OTP_VALIDATION_TEST),
    TEST_ENTRY(TXPULSE_CHECKERBOARD_TEST),
    {FPC_SENSORTEST_INVALID_TEST, NULL, NULL, NULL, false, NULL }
};

static int get_sensor_info(fpc_hal_ext_sensortest_t *self, fpc_hw_module_info_t *info)
{
    LOGD("%s", __func__);
    sensortest_module_t *module = (sensortest_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    int status = fpc_tee_get_sensor_otp_info(module->hal->sensor, info);
    if (status) {
        LOGE("%s, Failed to fetch hw otp data code: %d\n", __func__, status);
    }

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
    return status;
}

static int add_sensor_test(fpc_hal_ext_sensortest_tests_t *sensor_tests,
                           const char *name,
                           const char *description,
                           bool wait_for_finger_down,
                           const char *rubber_stamp_type)
{
    LOGD("%s", __func__);
    int status = 0;
    int size = sensor_tests->size;
    if (size < FPC_HAL_EXT_SENSORTEST_TESTS_MAX) {
        if (name != NULL) {
            strlcpy(sensor_tests->tests[size].name, name, FPC_HAL_EXT_SENSORTEST_TEST_NAME_MAX);
        }

        if (description != NULL) {
            strlcpy(sensor_tests->tests[size].description, description,
                    FPC_HAL_EXT_SENSORTEST_TEST_DESCRIPTION_MAX);
        }

        sensor_tests->tests[size].wait_for_finger_down = wait_for_finger_down;

        if (rubber_stamp_type != NULL) {
            strlcpy(sensor_tests->tests[size].rubber_stamp_type, rubber_stamp_type,
                    FPC_HAL_EXT_SENSORTEST_TEST_STAMP_TYPE_MAX);
        }

        sensor_tests->size = size + 1;
    } else {
        LOGE("%s, size >= TESTS_MAX\n", __func__);
        status = -FPC_ERROR_PARAMETER;
    }
    return status;
}

static int get_sensor_tests(fpc_hal_ext_sensortest_t *self,
                            fpc_hal_ext_sensortest_tests_t *sensor_tests)
{
    LOGD("%s", __func__);
    int status = 0;
    int32_t is_supported = 0;
    sensortest_module_t *module = (sensortest_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    for (int i = 0; fpc_sensortest_tests[i].name != NULL; i++) {
        status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                      fpc_sensortest_tests[i].test,
                                                      &is_supported);
        if (status) {
            LOGE("%s Failed checking support for test %d", __func__, fpc_sensortest_tests[i].test);
            goto out;
        }
        if (is_supported) {
            if (add_sensor_test(sensor_tests,
                                fpc_sensortest_tests[i].name,
                                fpc_sensortest_tests[i].desc,
                                fpc_sensortest_tests[i].wait_for_finger,
                                fpc_sensortest_tests[i].stamp) != 0) {
                goto out;
            }
        }
    }

out:
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
    return status;
}

static int get_mqt_config(sensortest_module_t *module,
                          fpc_ta_sensortest_test_params_t *params,
                          uint32_t *stabilization_time_ms)
{
    int status = 0;
    fpc_module_quality_test_limits_t mqt_limits;
    mqt_limits.snr_cropping_bottom = 0;
    mqt_limits.snr_cropping_left = 0;
    mqt_limits.snr_cropping_right = 0;
    mqt_limits.snr_cropping_top = 0;
    mqt_limits.snr_limit_preset = 0;
    mqt_limits.snr_threshold = 0;

    status = fpc_tee_get_mqt_limits(module->hal->sensor, &mqt_limits);
    if (!status) {
        params->mqt.snr_limit = mqt_limits.snr_threshold;
        params->mqt.snr_limit_preset = mqt_limits.snr_limit_preset;
        params->mqt.snr_cropping_left = mqt_limits.snr_cropping_left;
        params->mqt.snr_cropping_top = mqt_limits.snr_cropping_top;
        params->mqt.snr_cropping_right = mqt_limits.snr_cropping_right;
        params->mqt.snr_cropping_bottom = mqt_limits.snr_cropping_bottom;
        *stabilization_time_ms = STABILIZATION_MS;

        // parse test limits
        char *key_context = NULL;
        char *key = strtok_r(module->test_input.test_limits_key_value_pair, ";", &key_context);
        char *value_context = NULL;
        char *value = NULL;
        for (int i = 0; (i < 4) && (key != NULL); i++) {
            if (!STRNCMP(key, "snr=")) {
                value = key + strlen("snr=");
                params->mqt.snr_limit = atof(value);
                LOGD("%s: override snr_threshold: %f", __func__, params->mqt.snr_limit);
            } else if (!STRNCMP(key, "snrlimitpreset=")) {
                value = key + strlen("snrlimitpreset=");
                params->mqt.snr_limit_preset = atoi(value);
                LOGD("%s: override snr_limit_preset: %d", __func__,
                     params->mqt.snr_limit_preset);
            } else if (!STRNCMP(key, "snrcropping=")) {
                value = strtok_r(key + strlen("snrcropping="), ",", &value_context);
                for (int j = 0; (j < 4) && (value != NULL); j++) {
                    if (j == 0) {
                        params->mqt.snr_cropping_left = atoi(value);
                    } else if (j == 1) {
                        params->mqt.snr_cropping_top = atoi(value);
                    } else if (j == 2) {
                        params->mqt.snr_cropping_right = atoi(value);
                    } else if (j == 3) {
                        params->mqt.snr_cropping_bottom = atoi(value);
                    }
                    value = strtok_r(NULL, ",", &value_context);
                }
                LOGD("%s override snr_cropping left:%d top:%d right:%d bottom:%d ",
                     __func__,
                     params->mqt.snr_cropping_left,
                     params->mqt.snr_cropping_top,
                     params->mqt.snr_cropping_right,
                     params->mqt.snr_cropping_bottom);
            } else if (!STRNCMP(key, "stabilizationms=")) {
                value = key + strlen("stabilizationms=");
                *stabilization_time_ms = atoi(value);
                 LOGD("%s: override stabilizationms: %d", __func__,
                     *stabilization_time_ms);
            }

             key = strtok_r(NULL, ";", &key_context);
        }
    }

    return status;
}

static fpc_sensortest_test_t fpc_sensortest_get_test(const char *test_title,
                                                     get_test_config_t *get_test_config,
                                                     bool *wait_for_finger)
{
    int i;
    for (i = 0; fpc_sensortest_tests[i].test != FPC_SENSORTEST_INVALID_TEST; i++) {
        if (STRNCMP(test_title, fpc_sensortest_tests[i].name) == 0) {
            break;
        }
    }
    *get_test_config = fpc_sensortest_tests[i].get_test_config;
    *wait_for_finger = fpc_sensortest_tests[i].wait_for_finger;
    return fpc_sensortest_tests[i].test;
}

static void do_run_sensor_test(void *self)
{
    LOGD("%s", __func__);
    uint32_t result = 0;
    sensortest_module_t *module = (sensortest_module_t *) self;
    fpc_hal_ext_sensortest_test_result_t *test_result = NULL;
    get_test_config_t get_test_config;
    uint32_t stabilization_time_ms = 0;
    bool wait_for_finger = false;
    uint32_t log_size = 0;

    LOGD("%s: %s start", __func__, module->test.name);

    fpc_sensortest_test_t test = fpc_sensortest_get_test(module->test.name,
                                                         &get_test_config,
                                                         &wait_for_finger);
    if (test == FPC_SENSORTEST_INVALID_TEST) {
        LOGE("%s Invalid test case %s", __func__, module->test.name);
        module->test_result_cb(module->test_cb_ctx, NULL);
        goto out;
    }

    if (wait_for_finger) {
        stabilization_time_ms = STABILIZATION_MS;
    }

    test_result =
        (fpc_hal_ext_sensortest_test_result_t *) malloc(sizeof(fpc_hal_ext_sensortest_test_result_t));

    if (!test_result) {
        LOGE("%s failed to allocate test_result", __func__);
        module->test_result_cb(module->test_cb_ctx, NULL);
        goto out;
    }

    memset(test_result, 0, sizeof(fpc_hal_ext_sensortest_test_result_t));

    uint32_t image_fetched;
    fpc_ta_sensortest_test_params_t params;
    memset(&params, 0, sizeof(params));

    int status = 0;
    if (get_test_config) {
        status = get_test_config(module, &params, &stabilization_time_ms);
    }

    if (status) {
        LOGE("%s Unable to get parameters", __func__);
        module->test_result_cb(module->test_cb_ctx, NULL);
        goto out;
    }

    status = fpc_tee_sensortest_run_test(&(module->hal->sensor),
                                         test,
                                         stabilization_time_ms,
                                         &params,
                                         &image_fetched,
                                         &result,
                                         &log_size);

    LOGD("%s: %s finished with status: %d result: %d", __func__, module->test.name, status, result);
    test_result->image_fetched = image_fetched ? true : false;
#ifdef FPC_CONFIG_ENGINEERING
    if (fpc_tee_engineering_enabled(module->hal->tee_handle) &&
        !status && (test == FPC_SENSORTEST_MODULE_QUALITY_TEST) &&
        result) {
        snprintf(test_result->error_string,
                 FPC_HAL_EXT_SENSORTEST_TEST_ERROR_MAX,
                 "SNR Error:-%d",
                 params.mqt.snr_error);
    }
#endif

    if (status) {
        switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
        case -FPC_ERROR_CANCELLED:
            test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_CANCELLED;
            break;
        case -FPC_ERROR_NOT_SUPPORTED:
            test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_NOT_SUPPORTED;
            break;
        default:
            test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_ERROR;
            break;
        }
        test_result->error_code = status;
    } else {
        if (result == FPC_ERROR_NONE) {
            test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_PASS;
        } else {
            test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_FAIL;
        }
    }

#ifdef FPC_CONFIG_ENGINEERING
        if (test_result->image_fetched) {
            fpc_hal_ext_get_raw_image(module->engineering, &test_result->image_result);
        }

        if (log_size > 0) {
            /* The size of the log includes zero termination, but add one extra
             * byte just to be sure.
             */
            test_result->log = calloc(log_size + 1, sizeof(char));
            if (test_result->log) {
                status = fpc_tee_sensortest_get_log(module->hal->sensor,
                                                    &log_size,
                                                    (uint8_t *)test_result->log);

                if (status) {
                    LOGE("%s failed getting log %d", __func__, status);
                    free(test_result->log);
                    test_result->log = NULL;
                }
            } else {
                LOGE("%s Failed to allocated log buffer (%d)", __func__, log_size);
            }
        }
#else
        test_result->image_fetched = false;
#endif

    module->test_result_cb(module->test_cb_ctx, test_result);

#ifdef FPC_CONFIG_ENGINEERING
    if (test_result->image_fetched) {
        fpc_hal_ext_free_image(&test_result->image_result);
    }
#endif
out:
    if (test_result && test_result->log) {
        free(test_result->log);
    }
    free(test_result);
    test_result = NULL;

    module->test_cb_ctx = NULL;
    module->test_result_cb = NULL;
}

static int run_sensor_test(fpc_hal_ext_sensortest_t *self, fpc_hal_ext_sensortest_test_t *test,
                           fpc_hal_ext_sensortest_test_input_t *input, void *ctx,
                           fpc_test_result_cb_t result_cb)
{
    int status = 0;
    sensortest_module_t *module = (sensortest_module_t *) self;

    if (test == NULL) {
        LOGE("%s failed test is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (ctx == NULL) {
        LOGE("%s failed ctx is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (result_cb == NULL) {
        LOGE("%s failed result_cb is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    LOGD("%s %s\n", __func__, test->name);

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    module->test = *test;
    if (input) {
        module->test_input = *input;
    }
    module->test_cb_ctx = ctx;
    module->test_result_cb = result_cb;

    fingerprint_hal_do_async_work(module->hal, do_run_sensor_test, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

out:
    return status;
}

static void cancel_sensor_test(fpc_hal_ext_sensortest_t *self)
{
    LOGD("%s", __func__);
    sensortest_module_t *module = (sensortest_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
}

static void do_capture(void *self)
{
    LOGD("%s", __func__);
    int status;
    sensortest_module_t *module = (sensortest_module_t *) self;

    if (module->uncalibrated) {
        status = fpc_tee_sensortest_capture_uncalibrated(module->hal->sensor);
        if (status) {
            goto out;
        }

    } else if (module->wait_for_finger) {
        for (;;) {
            status = fpc_tee_capture_image(&module->hal->sensor, true, true, SELFTEST_MODE, NULL);

            if (status == FPC_STATUS_BAD_QUALITY) {
                // This will happen after exceeded maximum attempts
                // of trying to capture an image.
                continue;
            } else if (status == FPC_STATUS_FINGER_LOST) {
                LOGD("%s fpc_tee_capture_image finger removed too fast.", __func__);
            } else {
                goto out;
            }
        }
    } else {
        status = fpc_tee_capture_image(&module->hal->sensor, false, false, SELFTEST_MODE, NULL);
        if (status) {
            LOGD("%s fpc_capture_image failed: %i \n", __func__, status);
            goto out;
        }
    }

out:
    // Send result callback
    switch (FPC_ERROR_GET_EXTERNAL_ERROR(status)) {
    case FPC_ERROR_NONE:
        module->capture_acquired_cb(module->capture_cb_ctx, HAL_COMPAT_ACQUIRED_GOOD);
        if (module->engineering) {
            module->engineering->handle_image_subscription(module->engineering);
        }
        break;
    case FPC_STATUS_BAD_QUALITY:
    case FPC_STATUS_WAIT_TIME:
        module->capture_acquired_cb(module->capture_cb_ctx, HAL_COMPAT_ACQUIRED_INSUFFICIENT);
        if (module->engineering) {
            module->engineering->handle_image_subscription(module->engineering);
        }
        break;
    case -FPC_ERROR_IO:
    case -FPC_ERROR_CANCELLED:
        module->capture_error_cb(module->capture_cb_ctx, HAL_COMPAT_ERROR_CANCELED);
        break;
    default:
        LOGD("%s status code = %d", __func__, status);
        break;
    }

    // Clear callback
    module->capture_cb_ctx = NULL;
    module->capture_acquired_cb = NULL;
    module->capture_error_cb = NULL;
}

static int capture(fpc_hal_ext_sensortest_t *self, bool wait_for_finger,
                   bool uncalibrated, void *ctx, fpc_capture_acquired_cb_t acquired_cb,
                   fpc_capture_error_cb_t error_cb)
{
    int status = 0;
    sensortest_module_t *module = (sensortest_module_t *) self;

    LOGD("%s wait_for_finger: %d, uncalibrated: %d\n", __func__, wait_for_finger, uncalibrated);

    if (ctx == NULL) {
        LOGE("%s failed ctx is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (acquired_cb == NULL) {
        LOGE("%s failed acquired_cb is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (error_cb == NULL) {
        LOGE("%s failed error_cb is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (wait_for_finger && uncalibrated) {
        LOGE("%s wait_for_finger can not be used together with uncalibrated\n", __func__);
        status = -EINVAL;
        goto out;
    }

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    module->wait_for_finger = wait_for_finger;
    module->uncalibrated = false;
    module->capture_cb_ctx = ctx;
    module->capture_acquired_cb = acquired_cb;
    module->capture_error_cb = error_cb;

    fingerprint_hal_do_async_work(module->hal, do_capture, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

out:
    return status;
}

static void cancel_capture(fpc_hal_ext_sensortest_t *self)
{
    LOGD("%s", __func__);
    sensortest_module_t *module = (sensortest_module_t *) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
}

fpc_hal_ext_sensortest_t *fpc_sensortest_new(fpc_hal_common_t *hal)
{
    sensortest_module_t *module = malloc(sizeof(sensortest_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(sensortest_module_t));
    module->hal = hal;
    module->sensortest.get_sensor_info = get_sensor_info;
    module->sensortest.get_sensor_tests = get_sensor_tests;
    module->sensortest.run_sensor_test = run_sensor_test;
    module->sensortest.cancel_sensor_test = cancel_sensor_test;
    module->sensortest.capture = capture;
    module->sensortest.cancel_capture = cancel_capture;
    module->engineering = hal->ext_engineering;

    return (fpc_hal_ext_sensortest_t *) module;
}

void fpc_sensortest_destroy(fpc_hal_ext_sensortest_t *self)
{
    free(self);
}
