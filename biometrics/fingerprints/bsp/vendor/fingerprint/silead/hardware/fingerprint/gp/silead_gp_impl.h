/******************************************************************************
 * @file   silead_gp_impl.h
 * @brief  Contains GP communication header file.
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
 * Daniel Ye   2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_GP_IMPL_H__
#define __SILEAD_GP_IMPL_H__

#include "silead_ca_impl.h"

int32_t silfp_ca_gp_register(ca_impl_handle_t *handle, const void *ta_name);

#endif /* __SILEAD_GP_IMPL_H__ */
