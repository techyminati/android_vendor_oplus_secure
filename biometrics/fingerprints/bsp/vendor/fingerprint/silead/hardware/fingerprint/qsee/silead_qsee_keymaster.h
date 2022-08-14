/******************************************************************************
 * @file   silead_qsee_keymaster.h
 * @brief  Contains QSEE keymaster functions header file.
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
 * Willian Kin 2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_QSEE_KEYMASTER_H__
#define __SILEAD_QSEE_KEYMASTER_H__

int32_t silfp_qsee_keymaster_get(qsee_handle_t *qsee_handle, void **buffer);

#endif /* __SILEAD_QSEE_KEYMASTER_H__ */

