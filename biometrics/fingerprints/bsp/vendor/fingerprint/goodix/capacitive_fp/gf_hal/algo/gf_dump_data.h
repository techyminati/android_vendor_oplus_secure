/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: Dump data
 * History: None
 * Version: 1.0
 */
#ifndef _GF_DUMP_DATA_H_
#define _GF_DUMP_DATA_H_

#include <sys/time.h>
#include "gf_common.h"
#include "gf_user_type_define.h"
#include "gf_dump_data_encoder.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

void gf_dump_init(uint32_t row, uint32_t col, uint32_t nav_row,
                  uint32_t nav_col,
                  gf_chip_type_t chip_type, gf_chip_series_t chip_series);

/**
 * @brief : dump data to file for the given operation
 * @return :  0 means success, others means fail
 */
gf_error_t gf_dump_data_by_operation(dump_data_encoder_t* data_encoder,
                                     gf_dump_data_t *dump_data,
                                     gf_operation_type_t operation, struct timeval* tv,
                                     gf_error_t error_code, gf_chip_type_t chip_type);
/**
 * @brief : dump reserve data to file for the given operation
 * @return :  0 means success, others means fail
 */
gf_error_t gf_dump_data_by_operation_reserve(dump_data_encoder_t* data_encoder,
                                             gf_dump_reserve_t *dump_data,
                                             gf_operation_type_t operation, struct timeval* tv,
                                             gf_error_t error_code, gf_chip_type_t chip_type);

/**
 * @brief: dump template for the given user_cmd
 */
gf_error_t gf_dump_template(dump_data_encoder_t* data_encoder,
                            gf_dump_template_t* template, struct timeval* tv);

/**
 * @brief: dump ta mem_manager pool data
 *
 */
gf_error_t gf_dump_memmgr_pool(uint8_t *data, uint32_t len, void *time_str);


void gf_dump_prepare_for_test_cmd(uint32_t cmd_id);
void gf_dump_save_cur_auth_time(uint8_t index);
gf_error_t gf_dump_auth_retry_data(dump_data_encoder_t* data_encoder,
                                   gf_dump_data_t *dump_data,
                                   gf_operation_type_t operation,
                                   uint8_t index);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_DUMP_DATA_H_
