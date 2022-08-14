/******************************************************************************
 * @file   silead_config_dump.h
 * @brief  Contains Chip config files dump to .h files header file.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered 
 * into with Silead Inc. This software may be subject to export or import 
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Martin Wu  2018/4/2    0.1.0      Init version
 * Martin Wu  2018/5/7    0.1.1      Support 6150 related configurations.
 *
 *****************************************************************************/

#ifndef __FP_CONFIG_DUMP_H__
#define __FP_CONFIG_DUMP_H__

void silfp_cfg_dump_data(const cf_set_t *pcfgs, const char *save_path, char *board_module, int32_t update);

#endif /* __FP_CONFIG_DUMP_H__ */

