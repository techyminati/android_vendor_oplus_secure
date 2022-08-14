/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: HAL test for milan chip
 * History: None
 * Version: 1.0
 */

#include <endian.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "gf_dump_data.h"
#include "gf_hal.h"
#include "gf_hal_device.h"
#include "gf_hal_milan.h"
#include "gf_type_define.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_hal_test.h"
#include "gf_hal_dump.h"
#include "gf_ca_entry.h"
#include "gf_hal_timer.h"
#include "gf_hal_frr_database.h"
#include "gf_dump_bigdata.h"

#define LOG_TAG "[GF_HAL][gf_hal_milan_test]"

static void irq_image_test(gf_irq_t *cmd, gf_error_t *err_code);
static void test_authenticate_retry(gf_error_t *result, gf_irq_t *cmd);

/*
 * Function: test_authenticate_retry
 * Description: authenticate retry when failed
 * Input: command and error code
 * Output: error code
 * Return: void
 */
static void test_authenticate_retry(gf_error_t *result, gf_irq_t *cmd)
{
    uint32_t size = sizeof(gf_irq_t);
    VOID_FUNC_ENTER();

    do
    {
        // add for dump the front frame data
        if (cmd != NULL)
        {
            if ((cmd->irq_type & (GF_IRQ_IMAGE_MASK | GF_IRQ_NAV_MASK)) != 0)
            {
                gf_hal_dump_data_by_operation(cmd->operation, *result);
            }  // end if

            LOG_D(LOG_TAG, "[%s] start polling", __func__);
            cmd->step = GF_IRQ_STEP_POLLING;
            *result = gf_hal_invoke_command(GF_CMD_IRQ, cmd, size);

            if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))    // NOLINT(705)
            {
                LOG_D(LOG_TAG, "[%s] GF_IRQ_IMAGE_MASK", __func__);
                irq_image_test(cmd, result);
            }
            else
            {
                LOG_D(LOG_TAG, "[%s] polling image fail", __func__);
            }  // end if
        }  // end if
    }  // end do
    while (0);

    VOID_FUNC_EXIT();
}

/*
 * Function: test_image_test
 * Description: Test image processing after finger image is received
 * Input: IRQ command and error code
 * Output: error code
 * Return: void
 */
static void irq_image_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    gf_error_t *err = err_code;
    VOID_FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] pre process error code, err=%s, errno=%d",
          __func__, gf_strerror(*err), *err);

    // hal entry point of all test operation
    switch (cmd->operation)
    {
        case OPERATION_TEST_UNTRUSTED_ENROLL:
        {
            if (GF_SUCCESS == *err)
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_test_enrollment_progress(cmd->group_id, cmd->finger_id,
                                                       cmd->samples_remaining);
                LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u, samples_remaining=%u", __func__,
                            cmd->group_id, cmd->finger_id, cmd->samples_remaining);
                gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            }
            else if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == *err)
            {
                LOG_I(LOG_TAG, "[%s] crc check error, err=%s, errno=%d", __func__,
                            gf_strerror(*err), *err);
                gf_hal_notify_test_error_info(GF_FINGERPRINT_ERROR_SPI_COMMUNICATION);
            }  // end if

            break;
        }  // OPERATION_TEST_UNTRUSTED_ENROLL

        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
        {
            if (GF_SUCCESS == *err)  // err GF_SUCCESS
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_test_authentication_succeeded(cmd->group_id, cmd->finger_id,
                                                            &cmd->auth_token);
                LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
                            cmd->finger_id);
                gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            }
            else if (GF_ERROR_NOT_MATCH == *err)  // err GF_ERROR_NOT_MATCH
            {
                if (cmd->too_fast_flag > 0)  // not match for too fast
                {
                    gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
                }
                else if (cmd->mistake_touch_flag > 0)  // not match for mistake touch
                {
                    LOG_I(LOG_TAG, "[%s] touch by mistake", __func__);
                    gf_hal_notify_test_acquired_info(
                        GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE);
                }
                else
                {
                    gf_hal_notify_test_authentication_failed();
                }  // end if

                gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            }
            else if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == *err)  // err GF_ERROR_SPI_RAW_DATA_CRC_FAILED
            {
                LOG_I(LOG_TAG, "[%s] crc check error, err=%s, errno=%d", __func__,
                            gf_strerror(*err), *err);
                gf_hal_notify_test_error_info(GF_FINGERPRINT_ERROR_SPI_COMMUNICATION);
            }
            else if (GF_ERROR_MATCH_FAIL_AND_RETRY == *err)
            {
                test_authenticate_retry(err, cmd);
            }  // end if

            break;
        }  // case OPERATION_TEST_UNTRUSTED_AUTHENTICATE

        case OPERATION_TEST_INTERRUPT_PIN:
        {
            if (GF_SUCCESS == *err)
            {
                pthread_mutex_lock(&g_test_interrupt_pin_mutex);

                if (g_test_interrupt_pin_flag > 0)
                {
                    hal_notify_test_interrupt_pin(*err);
                    g_test_interrupt_pin_flag = 0;
                    gf_hal_destroy_timer(&g_irq_timer_id);
                }  // end if

                pthread_mutex_unlock(&g_test_interrupt_pin_mutex);
            }  // end if

            break;
        }  // case OPERATION_TEST_INTERRUPT_PIN

        case OPERATION_TEST_PERFORMANCE:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_PERFORMANCE", __func__);
            cmd->test_performance.total_time =
                (uint32_t)(gf_hal_current_time_microsecond() - g_down_irq_time);
            hal_notify_test_performance(*err, &cmd->test_performance);

            break;
        }  // case OPERATION_TEST_PERFORMANCE

        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_PIXEL_OPEN_STEP1", __func__);

            if (*err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] OPERATION_TEST_PIXEL_OPEN_STEP1 failed, err=%s, errno=%d",
                      __func__, gf_strerror(*err), *err);
                hal_notify_test_error(*err, CMD_TEST_PIXEL_OPEN);
            }  // end if

            break;
        }  // case OPERATION_TEST_PIXEL_OPEN_STEP1

        case OPERATION_TEST_PIXEL_OPEN_STEP2:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_PIXEL_OPEN_STEP2", __func__);

            if (*err == GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] test chip success, bad_pixel_num=%u", __func__,
                          cmd->bad_pixel_num);
            }
            else
            {
                LOG_D(LOG_TAG, "[%s] test chip failed, bad_pixel_num=%u", __func__,
                      cmd->bad_pixel_num);
            }  // end if

            hal_notify_test_pixel_open(*err, cmd->bad_pixel_num, cmd->local_bad_pixel_num);
            *err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PIXEL_OPEN_FINISH);

            break;
        }  // OPERATION_TEST_PIXEL_OPEN_STEP2

        case OPERATION_TEST_PIXEL_SHORT_STREAK:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_PIXEL_SHORT_STREAK", __func__);

            if (*err == GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] test chip success, bad_pixel_short_streak_num=%u", __func__,
                          cmd->bad_pixel_short_streak_num);
            }
            else
            {
                LOG_D(LOG_TAG, "[%s] test chip failed, bad_pixel_short_streak_num=%u", __func__,
                      cmd->bad_pixel_short_streak_num);
            }  // end if

            hal_notify_test_pixel_short_streak(*err, cmd->bad_pixel_short_streak_num);
            *err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PIXEL_OPEN_FINISH);

            break;
        }  // OPERATION_TEST_PIXEL_OPEN_STEP2

        case OPERATION_TEST_BAD_POINT:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT", __func__);

            if (*err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT crc failed, err=%s, errno=%d",
                      __func__,
                      gf_strerror(*err), *err);
                hal_notify_test_error(*err, CMD_TEST_BAD_POINT);
            }  // end if

            if (cmd->test_bad_point.algo_processed_flag > 0)
            {
                hal_notify_test_bad_point(*err, &cmd->test_bad_point.result);
            }  // end if

            break;
        }  // case OPERATION_TEST_BAD_POINT

        case OPERATION_TEST_CALIBRATION_PARA_RETEST:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_CALIBRATION_PARA_RETEST", __func__);

            if (*err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] OPERATION_TEST_CALIBRATION_PARA_RETEST crc failed, err=%s, errno=%d",
                      __func__,
                      gf_strerror(*err), *err);
                hal_notify_test_error(*err, CMD_TEST_CALIBRATION_PARA_RETEST);
            } else {
                hal_notify_test_calibration_para_retest(*err, &cmd->test_calibration_para_retest.cmd_data);
            }  // end if
            break;
        }  // case OPERATION_TEST_CALIBRATION_PARA_RETEST

        case OPERATION_TEST_SPI_PERFORMANCE:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_SPI_PERFORMANCE", __func__);
            hal_notify_test_spi_performance(*err, &cmd->test_spi_performance);

            break;
        }  // case OPERATION_TEST_SPI_PERFORMANCE

        case OPERATION_TEST_SPI_TRANSFER:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_SPI_TRANSFER", __func__);

            if ((gf_hal_current_time_microsecond() - g_irq_time_milan) > 800000)    // NOLINT(705)
            {
                LOG_D(LOG_TAG, "[%s] GF_CMD_TEST_SPI_TRANSFER timeout", __func__);
                *err = GF_ERROR_TEST_SPI_TRANSFER_TIMEOUT;
            }
            else if (GF_SUCCESS != *err)
            {
                LOG_D(LOG_TAG, "[%s] GF_CMD_TEST_SPI_TRANSFER failed", __func__);
            }
            else if (0 == (--g_test_spi_transfer_times))    // NOLINT(705)
            {
                LOG_D(LOG_TAG, "[%s] GF_CMD_TEST_SPI_TRANSFER success", __func__);
            }  // end if

            hal_notify_test_spi_transfer(*err, g_test_spi_transfer_times);

            break;
        }  // case OPERATION_TEST_SPI_TRANSFER

        case OPERATION_TEST_REAL_TIME_DATA:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_REAL_TIME_DATA", __func__);

            if (GF_SUCCESS == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_REAL_TIME_DATA success", __func__);
                hal_notify_test_real_time_data(*err, &(cmd->share.test_real_time_data));
            }  // end if

            break;
        }  // case OPERATION_TEST_REAL_TIME_DATA

        case OPERATION_TEST_BMP_DATA:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BMP_DATA", __func__);

            if (GF_SUCCESS == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BMP_DATA success", __func__);
                hal_notify_test_bmp_data(*err, &(cmd->test_bmp_data));
            }  // end if

            break;
        }  // case OPERATION_TEST_BMP_DATA

        case OPERATION_TEST_FRR_FAR_RECORD_CALIBRATION:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_FRR_FAR_RECORD_CALIBRATION", __func__);

            if (GF_SUCCESS == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_FRR_FAR_RECORD_CALIBRATION success",
                      __func__);
                LOG_D(LOG_TAG, "[%s]       frame_number=%d", __func__,
                      cmd->test_calibration.frame_num);
                LOG_D(LOG_TAG, "[%s]   calibration_type=%d", __func__,
                      cmd->test_calibration.calibration_type);

                if (cmd->test_calibration.calibration_type == 0)
                {
                    hal_notify_test_error(GF_ERROR_CALIBRATION_NOT_READY,
                                          CMD_TEST_FRR_FAR_RECORD_CALIBRATION);
                }
                else
                {
                    hal_notify_test_frr_far_record_calibration(*err, &(cmd->test_calibration));
                }  // end if
            }
            else
            {
                hal_notify_test_error(*err, CMD_TEST_FRR_FAR_RECORD_CALIBRATION);
            }  // end if

            break;
        }  // case OPERATION_TEST_FRR_FAR_RECORD_CALIBRATION

        case OPERATION_TEST_FRR_FAR_RECORD_ENROLL:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_FRR_FAR_RECORD_ENROLL err=%u", __func__,
                  *err);

            if (GF_SUCCESS == *err || GF_ERROR_DUPLICATE_FINGER == *err
                || GF_ERROR_DUPLICATE_AREA == *err || GF_ERROR_ACQUIRED_IMAGER_DIRTY == *err
                || GF_ERROR_ACQUIRED_PARTIAL == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_FRR_FAR_RECORD_ENROLL success", __func__);
                hal_notify_test_frr_far_record_enroll(*err, &(cmd->test_frr_far),
                        &(cmd->share.test_frr_far_reserve_data));
            }
            else
            {
                hal_notify_test_error(*err, CMD_TEST_FRR_FAR_RECORD_ENROLL);
            }  // end if

            break;
        }  // case OPERATION_TEST_FRR_FAR_RECORD_ENROLL

        case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE:
        {
            LOG_D(LOG_TAG,
                  "[%s] OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE, err=%s, errno=%d",
                  __func__, gf_strerror(*err), *err);
            hal_notify_test_frr_far_record_authenticate(*err, &(cmd->test_frr_far),
                    &(cmd->share.test_frr_far_reserve_data));

            break;
        }  // case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE

        default:
        {
            break;
        }  // case default
    }  // switch (cmd->operation)

    VOID_FUNC_EXIT();
}

/*
 * Function: gf_hal_milan_irq_test
 * Description: Test irq command for milan chip
 * Input: command and error code
 * Output: error code
 * Return: void
 */
void gf_hal_milan_irq_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    uint32_t size = sizeof(gf_irq_t);
    VOID_FUNC_ENTER();

    if (NULL == cmd || NULL == err_code)
    {
        LOG_E(LOG_TAG, "[%s] cmd or err_code is null", __func__);
        return;
    }

    // check sensor validity
    if (0 != (cmd->irq_type & GF_IRQ_RESET_MASK))    // NOLINT(705)
    {
        if (OPERATION_TEST_SENSOR_VALIDITY == cmd->operation)
        {
            hal_notify_test_sensor_validity(*err_code, cmd->test_sensor_validity.is_passed);
        }  // end if
    }  // end if

    // for down interupt
    if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))  // NOLINT(705)
    {
        LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_DOWN_MASK", __func__);
        g_down_irq_time = g_irq_time_milan;  // update finger down time

        if (GF_IRQ_STEP_POLLING == cmd->step)
        {
            // when get down irq, begin to get rawdata
            if (OPERATION_TEST_STABLE_FACTOR == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] snr start polling ", __func__);
                gf_hal_test_stable_get_touch_data(cmd, size);
            }  // end if
        }  // end if
    }  // end if

    // for image interupt
    if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))    // NOLINT(705)
    {
        irq_image_test(cmd, err_code);
    }  // end if

    // for up interupt
    if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))  // NOLINT(705)
    {
        LOG_D(LOG_TAG, "[%s] GF_IRQ_FINGER_UP_MASK", __func__);
        gf_hal_destroy_timer(&g_key_long_pressed_timer_id);
        // when get up irq, begin to get base
        if (OPERATION_TEST_STABLE_FACTOR == cmd->operation)
        {
            LOG_D(LOG_TAG, "[%s] gf_hal_test_snr_get_snr_result ", __func__);
            gf_hal_test_stable_get_result();
        }  // end if
    }  // end if

    // special case for INTERRUPT_PIN test happens over nav interupt
    if (cmd->irq_type & GF_IRQ_NAV_MASK
        && cmd->operation == OPERATION_TEST_INTERRUPT_PIN
        && GF_SUCCESS == *err_code)
    {
        pthread_mutex_lock(&g_test_interrupt_pin_mutex);

        if (g_test_interrupt_pin_flag > 0)
        {
            hal_notify_test_interrupt_pin(*err_code);
            g_test_interrupt_pin_flag = 0;
            gf_hal_destroy_timer(&g_irq_timer_id);
        }  // end if

        pthread_mutex_unlock(&g_test_interrupt_pin_mutex);
    }  // end if

    VOID_FUNC_EXIT();
}
