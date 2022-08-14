/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#ifndef _GF_HAL_TEST_H_
#define _GF_HAL_TEST_H_

#include "gf_error.h"
#include "gf_fingerprint.h"
#include "gf_type_define.h"
#include "gf_common.h"

#define TEST_CAPTURE_VALID_IMAGE_QUALITY_THRESHOLD 15

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

extern uint8_t g_test_interrupt_pin_flag;
extern pthread_mutex_t g_test_interrupt_pin_mutex;
extern uint32_t g_test_spi_transfer_times;
extern uint32_t g_test_sensor_fine_count;
extern uint32_t g_sensor_broken_err;

extern void hal_notify_test_finger_event(gf_error_t err, uint32_t finger_event);
extern void hal_notify_test_performance(gf_error_t err,
                                        gf_test_performance_t *result);
extern void hal_notify_test_pixel_open(gf_error_t err, uint32_t bad_pixel_num, uint32_t local_bad_pixel_num);
extern void hal_notify_test_pixel_short_streak(gf_error_t err, uint32_t bad_pixel_short_streak_num);
extern void hal_notify_test_spi_performance(gf_error_t err,
                                            gf_test_spi_performance_t *result);
extern void hal_notify_test_spi_transfer(gf_error_t err, int32_t remainings);
extern void hal_notify_test_real_time_data(gf_error_t err,
                                           gf_test_real_time_data_t *result);
extern void hal_notify_test_bmp_data(gf_error_t err,
                                     gf_test_bmp_data_t *result);

extern void hal_notify_test_frr_far_record_calibration(gf_error_t err,
                                                       gf_test_calibration_t *result);
extern void hal_notify_test_frr_far_record_enroll(gf_error_t err,
                                                  gf_test_frr_far_t *result,
                                                  gf_test_frr_far_reserve_data_t *reserve_result);
extern void hal_notify_test_frr_far_record_authenticate(gf_error_t err,
                                                        gf_test_frr_far_t *result,
                                                        gf_test_frr_far_reserve_data_t *reserve_result);
extern void hal_notify_test_bio_hbd_calibration(gf_error_t err,
                                                gf_test_hbd_feature_t *hbd_feature,
                                                gf_cmd_test_id_t cmd_id);
extern gf_error_t hal_notify_test_bad_point(gf_error_t err,
                                      gf_bad_point_test_result_t *result);
extern void hal_notify_test_interrupt_pin(gf_error_t err);
extern void hal_notify_test_error(gf_error_t err, gf_cmd_test_id_t cmd_id);
extern void hal_notify_test_sensor_validity(gf_error_t err, int32_t status);
extern void hal_notify_test_sensor_fine(gf_error_t err, uint32_t average_pixel_diff);
extern void hal_notify_test_rawdata_saturated(gf_error_t err,
                                       gf_test_rawdata_saturated_t *result);
extern gf_error_t hal_test_sensor_fine();
extern gf_error_t hal_notify_test_calibration_para_retest(gf_error_t err, gf_test_calibration_para_t *result);
extern gf_error_t gf_hal_test_stable_get_touch_data(void *buffer, int32_t len);
extern gf_error_t gf_hal_test_stable_get_result(void);
extern gf_error_t hal_test_get_version(char *buf);

void gf_hal_free_snr_memmory();
extern gf_error_t gf_test_sensor_broken(void);

gf_error_t hal_test_sensor_broken(void);
gf_error_t hal_broken_check(uint8_t thread_block_mode);

gf_error_t gf_hal_test_cmd(void *dev, uint32_t cmd_id, const uint8_t *param,
                           uint32_t param_len);
gf_error_t gf_hal_test_cancel(void *dev);
gf_error_t gf_hal_test_prior_cancel(void *dev);
void hal_notify_test_fpc_detected(gf_error_t err, uint32_t  finger_event, uint32_t status);
void hal_notify_test_fpc_reset_fwcfg(gf_error_t err);
gf_error_t hal_test_sensor_validity(void);
gf_error_t hal_test_spi(void);
gf_error_t hal_test_synchronous_pixel_open(void);
gf_error_t hal_test_performance(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_HAL_TEST_H_
