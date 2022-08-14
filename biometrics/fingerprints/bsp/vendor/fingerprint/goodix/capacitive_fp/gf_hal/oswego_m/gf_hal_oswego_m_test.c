/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include <endian.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "gf_dump_data.h"
#include "gf_hal.h"
#include "gf_hal_device.h"
#include "gf_hal_oswego_m.h"
#include "gf_type_define.h"
#include "gf_hal_log.h"
#include "gf_hal_common.h"
#include "gf_hal_test.h"
#include "gf_hal_dump.h"
#include "gf_ca_entry.h"
#include "gf_hal_timer.h"
#include "gf_queue.h"
#include "gf_hal_frr_database.h"

#define LOG_TAG "[GF_HAL][gf_hal_oswego_m]"

static void irq_image_test(gf_irq_t *cmd, gf_error_t *err_code);
static void irq_up_test(gf_irq_t *cmd);

static void irq_image_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    gf_error_t *err = err_code;
    VOID_FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] pre process error code, err=%s, errno=%d",
          __func__, gf_strerror(*err), *err);

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
            else if (GF_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS == *err)
            {
                gf_hal_notify_test_error_info(
                    GF_FINGERPRINT_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS);
            }
            else if (GF_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS == *err)
            {
                gf_hal_notify_test_error_info(
                    GF_FINGERPRINT_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS);
            }
            else if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == *err)
            {
                LOG_I(LOG_TAG, "[%s] crc check error, err=%s, errno=%d", __func__,
                      gf_strerror(*err), *err);
                gf_hal_notify_test_error_info(GF_FINGERPRINT_ERROR_SPI_COMMUNICATION);
            }
            else if (GF_ERROR_SENSOR_IS_BROKEN == *err)
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_PARTIAL);
            }
            else if (GF_ERROR_ACQUIRED_IMAGER_DIRTY == *err)
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
            }
            else if (GF_ERROR_SENSOR_BROKEN_CHECK_ALGO_ERROR == *err ||
                     GF_ERROR_ACQUIRED_PARTIAL == *err)
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_PARTIAL);
            }
            else if (GF_ERROR_DUPLICATE_AREA == *err)
            {
                gf_hal_notify_test_acquired_info(
                    GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA);
            }
            break;
        }

        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
        {
            if (GF_SUCCESS == *err)
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_test_authentication_succeeded(cmd->group_id, cmd->finger_id,
                                                            &cmd->auth_token);
                LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
                      cmd->finger_id);
                gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            }
            else if (GF_ERROR_NOT_MATCH == *err)
            {
                if (cmd->mistake_touch_flag > 0)
                {
                    LOG_I(LOG_TAG, "[%s] touch by mistake", __func__);
                    gf_hal_notify_test_acquired_info(
                        GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE);
                }
                else
                {
                    gf_hal_notify_test_authentication_failed();
                }

                gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
            }
            else if (GF_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS == *err)
            {
                gf_hal_notify_test_error_info(
                    GF_FINGERPRINT_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS);
            }
            else if (GF_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS == *err)
            {
                gf_hal_notify_test_error_info(
                    GF_FINGERPRINT_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS);
            }
            else if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == *err)
            {
                LOG_I(LOG_TAG, "[%s] crc check error, err=%s, errno=%d", __func__,
                      gf_strerror(*err), *err);
                gf_hal_notify_test_error_info(GF_FINGERPRINT_ERROR_SPI_COMMUNICATION);
            }
            break;
        }

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
                }

                pthread_mutex_unlock(&g_test_interrupt_pin_mutex);
            }
            break;
        }

        case OPERATION_TEST_PERFORMANCE:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_PERFORMANCE", __func__);
            cmd->test_performance.total_time = (uint32_t)(gf_hal_current_time_microsecond()
                                                          - g_down_irq_time);
            hal_notify_test_performance(*err, &cmd->test_performance);
            break;
        }

        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_PIXEL_OPEN_STEP1", __func__);

            if (*err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] OPERATION_TEST_PIXEL_OPEN_STEP1 failed, err=%s, errno=%d",
                      __func__, gf_strerror(*err), *err);
                hal_notify_test_error(*err, CMD_TEST_PIXEL_OPEN);
            }
            break;
        }

        case OPERATION_TEST_PIXEL_OPEN_STEP2:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_PIXEL_OPEN_STEP2", __func__);

            if (cmd->image_count == 2)
            {
                if (*err == GF_SUCCESS)
                {
                    LOG_D(LOG_TAG, "[%s] test chip success, bad_pixel_num=%u", __func__,
                          cmd->bad_pixel_num);
                }
                else
                {
                    LOG_D(LOG_TAG, "[%s] test chip failed, bad_pixel_num=%u", __func__,
                          cmd->bad_pixel_num);
                }

                hal_notify_test_pixel_open(*err, cmd->bad_pixel_num, 0);
                *err = gf_hal_test_invoke_command_ex(GF_CMD_TEST_PIXEL_OPEN_FINISH);
            }
            break;
        }

        case OPERATION_TEST_BAD_POINT:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT", __func__);

            if (*err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] OPERATION_TEST_BAD_POINT crc failed, err=%s, errno=%d",
                      __func__,
                      gf_strerror(*err), *err);
                hal_notify_test_error(*err, CMD_TEST_BAD_POINT);
            }

            if (cmd->test_bad_point.algo_processed_flag > 0)
            {
                hal_notify_test_bad_point(*err, &cmd->test_bad_point.result);
            }
            break;
        }

        case OPERATION_TEST_SPI_PERFORMANCE:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_SPI_PERFORMANCE", __func__);
            hal_notify_test_spi_performance(*err, &cmd->test_spi_performance);
            break;
        }

        case OPERATION_TEST_SPI_TRANSFER:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_SPI_TRANSFER", __func__);

            if ((gf_hal_current_time_microsecond() - g_irq_time) > 800000)
            {
                LOG_D(LOG_TAG, "[%s] GF_CMD_TEST_SPI_TRANSFER timeout", __func__);
                *err = GF_ERROR_TEST_SPI_TRANSFER_TIMEOUT;
            }
            else if (GF_SUCCESS != *err)
            {
                LOG_D(LOG_TAG, "[%s] GF_CMD_TEST_SPI_TRANSFER failed", __func__);
            }
            else if (0 == (--g_test_spi_transfer_times))
            {
                LOG_D(LOG_TAG, "[%s] GF_CMD_TEST_SPI_TRANSFER success", __func__);
            }

            hal_notify_test_spi_transfer(*err, g_test_spi_transfer_times);
            break;
        }

        case OPERATION_TEST_REAL_TIME_DATA:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_REAL_TIME_DATA", __func__);

            if (GF_SUCCESS == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_REAL_TIME_DATA success", __func__);
                hal_notify_test_real_time_data(*err, &(cmd->share.test_real_time_data));
            }
            break;
        }

        case OPERATION_TEST_BMP_DATA:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BMP_DATA", __func__);

            if (GF_SUCCESS == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_BMP_DATA success", __func__);
                hal_notify_test_bmp_data(*err, &(cmd->test_bmp_data));
            }
            break;
        }

        case OPERATION_TEST_DATA_NOISE:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_DATA_NOISE", __func__);
            if (GF_SUCCESS == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_DATA_NOISE success", __func__);
                // hal_notify_test_data_noise(*err, &(cmd->test_data_noise));
            }
            break;
        }

        case OPERATION_TEST_SENSOR_FINE_STEP2:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_SENSOR_FINE_STEP2, average_pixel_diff = %u",
                  __func__,
                  cmd->test_sensor_fine.average_pixel_diff);

            if (cmd->test_sensor_fine.average_pixel_diff >
                g_hal_config.average_pixel_diff_threshold
                || g_test_sensor_fine_count >= 2)
            {
                hal_notify_test_sensor_fine(*err, cmd->test_sensor_fine.average_pixel_diff);
                g_test_sensor_fine_count = 0;
                gf_hal_test_invoke_command_ex(GF_CMD_TEST_SENSOR_FINE_FINISH);
            }
            else
            {
                g_test_sensor_fine_count++;
                hal_test_sensor_fine();
            }
            break;
        }

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
                }
            }
            else
            {
                hal_notify_test_error(*err, CMD_TEST_FRR_FAR_RECORD_CALIBRATION);
            }
            break;
        }

        case OPERATION_TEST_FRR_FAR_RECORD_ENROLL:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_FRR_FAR_RECORD_ENROLL err=%u", __func__,
                  *err);

            if (GF_SUCCESS == *err || GF_ERROR_DUPLICATE_FINGER == *err)
            {
                LOG_D(LOG_TAG, "[%s] OPERATION_TEST_FRR_FAR_RECORD_ENROLL success", __func__);
                hal_notify_test_frr_far_record_enroll(*err, &(cmd->test_frr_far), &(cmd->share.test_frr_far_reserve_data));
            }
            else
            {
                hal_notify_test_error(*err, CMD_TEST_FRR_FAR_RECORD_ENROLL);
            }
            break;
        }

        case OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE:
        {
            LOG_D(LOG_TAG,
                  "[%s] OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE, err=%s, errno=%d",
                  __func__, gf_strerror(*err), *err);
            hal_notify_test_frr_far_record_authenticate(*err, &(cmd->test_frr_far), &(cmd->share.test_frr_far_reserve_data));
            break;
        }

        case OPERATION_TEST_RAWDATA_SATURATED:
        {
            LOG_D(LOG_TAG, "[%s] OPERATION_TEST_RAWDATA_SATURATED", __func__);
            hal_notify_test_rawdata_saturated(*err, &(cmd->test_rawdata_saturated));
            break;
        }

        default:
        {
            break;
        }
    }

    VOID_FUNC_EXIT();
}

static void irq_up_test(gf_irq_t *cmd)
{
    VOID_FUNC_ENTER();

    do
    {
        if (cmd->too_fast_flag > 0)
        {
            LOG_I(LOG_TAG, "[%s] press too fast", __func__);

            if (OPERATION_TEST_UNTRUSTED_ENROLL == cmd->operation
                || OPERATION_TEST_UNTRUSTED_AUTHENTICATE == cmd->operation)
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
            }
        }
        else if (cmd->mistake_touch_flag > 0)
        {
            LOG_I(LOG_TAG, "[%s] touch by mistake", __func__);

            if (OPERATION_TEST_UNTRUSTED_ENROLL == cmd->operation
                || OPERATION_TEST_UNTRUSTED_AUTHENTICATE == cmd->operation)
            {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE);
            }
        }
        else if (cmd->report_authenticate_fail_flag > 0)
        {
            if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] no finger match", __func__);
                gf_hal_notify_test_authentication_failed();
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

void hal_oswego_irq_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    if (NULL == cmd || NULL == err_code)
    {
        LOG_E(LOG_TAG, "[%s] cmd or err_code is null", __func__);
        return;
    }

    if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))
    {
        irq_image_test(cmd, err_code);
    }

    if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
    {
        irq_up_test(cmd);
    }

    VOID_FUNC_EXIT();
}
