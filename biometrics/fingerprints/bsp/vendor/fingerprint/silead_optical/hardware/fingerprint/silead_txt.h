/******************************************************************************
 * @file   silead_txt.h
 * @brief  Contains Bitmap operate functions header file.
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
 * Lyman Xue  2019/1/22   0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_DATATXT_H__
#define __SILEAD_DATATXT_H__

#include "silead_cmd_cust.h"

#ifndef SILEADFILE_INITIAL_CHAR
#define SILEADFILE_INITIAL_CHAR    255//initial char after open file
#endif
int32_t silfp_txt_get_save_path(char *path, uint32_t len, const char *dir, const char *prefix);
int32_t silfp_txt_save(const char *path, ft_info_t ft, uint32_t step, int32_t reserve);


#endif /* __SILEAD_DATATXT_H__ */


